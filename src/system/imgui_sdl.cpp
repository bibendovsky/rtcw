/*

Very simple software renderer powered by SDL2 for Dear ImGui

Copyright (c) 2018 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

*/


//
// Very simple software renderer powered by SDL2 for Dear ImGui.
//
// Notes:
//    - Supports only SDL_Surface as texture id.
//    - Supports only ARGB8888 pixel format (see "create_texture_id").
//    - May produce visual artefacts for rounded/antialiased corners.
//


#include "imgui_sdl.h"
#include <array>
#include <bitset>
#include <limits>
#include <utility>
#include <vector>
#include "imgui.h"
#include "SDL.h"


bool operator==(
	const SDL_Point& lhs,
	const SDL_Point& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}


// ==========================================================================
// ImGuiSdl::Impl
//

class ImGuiSdl::Impl
{
public:
	Impl()
		:
		error_message_{},
		clipboard_buffer_{},
		sdl_window_{},
		sdl_window_id_{},
		sdl_renderer_{},
		sdl_framebuffer_texture_{},
		screen_width_{},
		screen_height_{},
		frame_buffer_{},
		is_close_requested_{},
		mouse_buttons_state_{},
		has_common_edge_{},
		has_common_edge_prev_triangle_{},
		common_edge_prev_triangle_{},
		common_edge_tag_{},
		common_edge_map_{}
	{
	}

	Impl(
		const Impl& that) = delete;

	Impl& operator=(
		const Impl& that) = delete;

	Impl(
		Impl&& that)
		:
		error_message_{std::move(that.error_message_)},
		clipboard_buffer_{std::move(that.clipboard_buffer_)},
		sdl_window_{std::move(that.sdl_window_)},
		sdl_window_id_{std::move(that.sdl_window_id_)},
		sdl_renderer_{std::move(that.sdl_renderer_)},
		sdl_framebuffer_texture_{std::move(that.sdl_framebuffer_texture_)},
		screen_width_{std::move(that.screen_width_)},
		screen_height_{std::move(that.screen_height_)},
		frame_buffer_{std::move(that.frame_buffer_)},
		is_close_requested_{std::move(that.is_close_requested_)},
		mouse_buttons_state_{std::move(that.mouse_buttons_state_)},
		has_common_edge_{std::move(that.has_common_edge_)},
		has_common_edge_prev_triangle_{std::move(that.has_common_edge_prev_triangle_)},
		common_edge_prev_triangle_{std::move(that.common_edge_prev_triangle_)},
		common_edge_tag_{std::move(that.common_edge_tag_)},
		common_edge_map_{std::move(that.common_edge_map_)}
	{
		that.sdl_framebuffer_texture_ = nullptr;
	}

	~Impl()
	{
		api_uninitialize();
	}


	// ======================================================================
	// API
	//

	const std::string& get_error_message() const
	{
		return error_message_;
	}

	bool api_initialize(
		SDL_Window* sdl_window)
	{
		error_message_.clear();

		api_uninitialize();

		auto imgui_context_ptr = ImGui::GetCurrentContext();

		if (!imgui_context_ptr)
		{
			error_message_ = "No Dear ImGui context.";
			return false;
		}


		auto sdl_result = 0;

		sdl_window_ = sdl_window;

		if (!sdl_window_)
		{
			error_message_ = "No SDL window.";
			return false;
		}

		sdl_renderer_ = ::SDL_GetRenderer(sdl_window_);

		if (!sdl_renderer_)
		{
			error_message_ = "No SDL renderer.";
			return false;
		}

		sdl_result = ::SDL_GetRendererOutputSize(sdl_renderer_, &screen_width_, &screen_height_);

		if (sdl_result != 0)
		{
			error_message_ = "Failed to get SDL renderer's output size.";
			return false;
		}

		sdl_framebuffer_texture_ = ::SDL_CreateTexture(
			sdl_renderer_,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			screen_width_,
			screen_height_);

		if (!sdl_framebuffer_texture_)
		{
			error_message_ = "Failed to create SDL texture for framebuffer.";
			return false;
		}

		frame_buffer_.clear();
		frame_buffer_.resize(screen_width_ * screen_height_);

		initialize_common_edge_map();
		initialize_imgui_io();

		sdl_window_id_ = ::SDL_GetWindowID(sdl_window_);

		const auto window_flags = ::SDL_GetWindowFlags(sdl_window_);

		if ((window_flags & (SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED)) != 0)
		{
			window_status_ = WindowStatus::shown;
		}
		else if ((window_flags & SDL_WINDOW_HIDDEN) != 0)
		{
			window_status_ = WindowStatus::hidden;
		}
		else if ((window_flags & SDL_WINDOW_MINIMIZED) != 0)
		{
			window_status_ = WindowStatus::minimized;
		}
		else
		{
			window_status_ = WindowStatus::none;
		}

		return true;
	}

	void api_uninitialize()
	{
		sdl_window_ = {};
		sdl_window_id_ = {};
		sdl_renderer_ = {};

		if (sdl_framebuffer_texture_)
		{
			::SDL_DestroyTexture(sdl_framebuffer_texture_);
			sdl_framebuffer_texture_ = {};
		}

		screen_width_ = {};
		screen_height_ = {};
		frame_buffer_.clear();

		mouse_buttons_state_.reset();

		has_common_edge_ = {};
		has_common_edge_prev_triangle_ = {};
		common_edge_prev_triangle_ = {};
		common_edge_tag_ = {};
		common_edge_map_.clear();
	}

	void api_handle_new_frame()
	{
		auto& imgui_io = ImGui::GetIO();


		auto mouse_x = 0;
		auto mouse_y = 0;

		const auto sdl_mouse_buttons = ::SDL_GetMouseState(&mouse_x, &mouse_y);

		imgui_io.MouseDown[0] = mouse_buttons_state_.test(0) || (sdl_mouse_buttons & SDL_BUTTON_LMASK) != 0;
		imgui_io.MouseDown[1] = mouse_buttons_state_.test(1) || (sdl_mouse_buttons & SDL_BUTTON_RMASK) != 0;
		imgui_io.MouseDown[2] = mouse_buttons_state_.test(2) || (sdl_mouse_buttons & SDL_BUTTON_MMASK) != 0;

		mouse_buttons_state_.reset();

		ImGui::NewFrame();
	}

	void api_handle_event(
		const SDL_Event& sdl_event)
	{
		switch (sdl_event.type)
		{
		case SDL_QUIT:
			is_close_requested_ = true;
			break;

		case SDL_WINDOWEVENT:
			handle_window_event(sdl_event.window);
			break;

		case SDL_MOUSEMOTION:
			handle_mouse_motion_event(sdl_event.motion);
			break;

		case SDL_MOUSEWHEEL:
			handle_mouse_wheel(sdl_event.wheel);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			handle_mouse_button(sdl_event.button);
			break;

		case SDL_TEXTINPUT:
			handle_text_input_event(sdl_event.text);
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			handle_keyboard_event(sdl_event.key);
			break;

		default:
			break;
		}
	}

	void api_handle_draw_data(
		ImDrawData* draw_data)
	{
		clear_frame_buffer();

		auto clip_rect = Rect{};
		auto parameters = DrawTriangleParameters{};

		for (auto i_cmd_list = 0; i_cmd_list < draw_data->CmdListsCount; ++i_cmd_list)
		{
			const auto cmd_list_ptr = draw_data->CmdLists[i_cmd_list];
			const auto vertex_buffer_ptr = cmd_list_ptr->VtxBuffer.Data;
			auto index_buffer_ptr = cmd_list_ptr->IdxBuffer.Data;

			for (auto i_cmd = 0; i_cmd < cmd_list_ptr->CmdBuffer.Size; ++i_cmd)
			{
				const auto pcmd = &cmd_list_ptr->CmdBuffer[i_cmd];

				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list_ptr, pcmd);
				}
				else
				{
					clip_rect.x = static_cast<int>(pcmd->ClipRect.x);
					clip_rect.y = static_cast<int>(pcmd->ClipRect.y);
					clip_rect.w = static_cast<int>(pcmd->ClipRect.z - pcmd->ClipRect.x);
					clip_rect.h = static_cast<int>(pcmd->ClipRect.w - pcmd->ClipRect.y);

					const auto sdl_surface = static_cast<SDL_Surface*>(pcmd->TextureId);
					const auto has_sdl_surface = (sdl_surface != nullptr);

					auto tx_width_f = 0.0F;
					auto tx_height_f = 0.0F;

					if (has_sdl_surface)
					{
						tx_width_f = static_cast<float>(sdl_surface->w - 1);
						tx_height_f = static_cast<float>(sdl_surface->h - 1);
					}

					parameters.clip_rect_ = clip_rect;
					parameters.sdl_texture_surface_ = sdl_surface;

					const auto tri_count = pcmd->ElemCount / 3;

					for (auto i_tri = ImDrawIdx{}; i_tri < tri_count; ++i_tri)
					{
						const auto index = 3 * i_tri;

						const ImDrawVert* vertex_ptrs[3] = {
							&vertex_buffer_ptr[index_buffer_ptr[index + 0]],
							&vertex_buffer_ptr[index_buffer_ptr[index + 1]],
							&vertex_buffer_ptr[index_buffer_ptr[index + 2]],
						};

						auto& points = parameters.points_;
						auto& tx_coords = parameters.tx_coords_;

						for (auto i_vertex = 0; i_vertex < 3; ++i_vertex)
						{
							const auto& vertex = *vertex_ptrs[i_vertex];

							auto& point = points[i_vertex];
							point.x = static_cast<int>(vertex.pos.x);
							point.y = static_cast<int>(vertex.pos.y);

							parameters.colors_[i_vertex] = color_from_imgui(vertex.col);

							auto& tx_coord = tx_coords[i_vertex];
							tx_coord.x = static_cast<int>(vertex.uv.x * tx_width_f);
							tx_coord.y = static_cast<int>(vertex.uv.y * tx_height_f);
						}

						auto has_texturing = false;

						if (sdl_surface)
						{
							if (tx_coords[0] == tx_coords[1] && tx_coords[0] == tx_coords[2])
							{
								has_texturing = false;
							}
							else
							{
								has_texturing = true;
							}
						}

						parameters.has_texturing_ = has_texturing;

						draw_triangle(parameters);
					}
				}

				index_buffer_ptr += pcmd->ElemCount;
			}
		}
	}

	void api_present()
	{
		void* pixels = nullptr;
		auto pitch = 0;

		auto sdl_result = 0;

		sdl_result = ::SDL_LockTexture(sdl_framebuffer_texture_, nullptr, &pixels, &pitch);

		if (sdl_result == 0)
		{
			auto dst_pixels = static_cast<Color*>(pixels);

			std::uninitialized_copy_n(frame_buffer_.data(), frame_buffer_.size(), dst_pixels);

			::SDL_UnlockTexture(sdl_framebuffer_texture_);
		}

		sdl_result = ::SDL_RenderCopy(sdl_renderer_, sdl_framebuffer_texture_, nullptr, nullptr);

		::SDL_RenderPresent(sdl_renderer_);
	}

	static SDL_Surface* api_create_sdl_rgba_surface(
		const int width,
		const int height,
		const PixelFormat src_pixel_format,
		const void* raw_pixels)
	{
		if (width <= 0 || height <= 0)
		{
			return nullptr;
		}

		auto use_converter = false;

		if (raw_pixels)
		{
			switch (src_pixel_format)
			{
			case PixelFormat::standard:
				break;

			case PixelFormat::imgui:
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
				use_converter = true;
#endif // IMGUI_USE_BGRA_PACKED_COLOR
				break;

			default:
				return nullptr;
			}
		}

		SDL_Surface* sdl_surface;

		if (!raw_pixels || use_converter)
		{
			sdl_surface = ::SDL_CreateRGBSurface(
				0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		}
		else
		{
			sdl_surface = ::SDL_CreateRGBSurfaceFrom(
				const_cast<void*>(raw_pixels),
				width,
				height,
				32,
				width * 4,
				0x00FF0000,
				0x0000FF00,
				0x000000FF,
				0xFF000000);
		}

		if (!sdl_surface)
		{
			return nullptr;
		}

		if (!use_converter)
		{
			return sdl_surface;
		}

		const auto dst_line_width = sdl_surface->pitch / 4;

		auto src_pixels = static_cast<const Color*>(raw_pixels);
		auto dst_pixels = static_cast<Color*>(sdl_surface->pixels);

		for (auto i = 0; i < height; ++i)
		{
			const auto area = width * height;

			std::transform(
				src_pixels + 0,
				src_pixels + width,
				dst_pixels,
				color_from_imgui);

			src_pixels += width;
			dst_pixels += dst_line_width;
		}

		return sdl_surface;
	}

	WindowStatus api_get_window_status() const
	{
		return window_status_;
	}

	bool api_is_close_requested() const
	{
		return is_close_requested_;
	}

	void api_reset_is_close_requested()
	{
		is_close_requested_ = false;
	}

	//
	// API
	// ======================================================================


private:
	using Color = std::uint32_t; // A8R8G8B8
	using Point = SDL_Point;
	using Rect = SDL_Rect;
	using Triangle = std::array<Point, 3>;
	using Colors = std::array<Color, 3>;
	using TxCoords = std::array<Point, 3>;
	using BarycentricNumerators = std::array<int, 3>;
	using FrameBuffer = std::vector<Color>;

	static constexpr auto max_mouse_button_count = 3;
	using MouseButtonsState = std::bitset<max_mouse_button_count>;

	using CommonEdgeTag = std::uint16_t;
	using CommonEdgeMap = std::vector<CommonEdgeTag>;

	struct CommonEdge
	{
		int index0_;
		int index1_;
	}; // CommonEdge

	static constexpr auto common_edge_max_tag = std::numeric_limits<CommonEdgeTag>::max();


	struct DrawTriangleParameters
	{
		bool has_texturing_;

		Rect clip_rect_;
		Triangle points_;
		Colors colors_;
		TxCoords tx_coords_;
		SDL_Surface* sdl_texture_surface_;
	}; // DrawTriangleParameters


	std::string error_message_;
	std::string clipboard_buffer_;
	SDL_Window* sdl_window_;
	Uint32 sdl_window_id_;
	SDL_Renderer* sdl_renderer_;
	SDL_Texture* sdl_framebuffer_texture_;

	int screen_width_;
	int screen_height_;
	FrameBuffer frame_buffer_;

	bool is_close_requested_;
	WindowStatus window_status_;
	MouseButtonsState mouse_buttons_state_;

	bool has_common_edge_;
	bool has_common_edge_prev_triangle_;
	Triangle common_edge_prev_triangle_;
	CommonEdgeTag common_edge_tag_;
	CommonEdgeMap common_edge_map_;


	template<typename T>
	static constexpr T min(
		const T& lhs,
		const T& rhs)
	{
		return std::min(lhs, rhs);
	}

	template<typename T, typename... TArgs>
	static constexpr T min(
		const T& v0,
		const T& v1,
		TArgs&&... args)
	{
		return min(min(v0, v1), std::forward<TArgs>(args)...);
	}


	template<typename T>
	static constexpr T max(
		const T& lhs,
		const T& rhs)
	{
		return std::max(lhs, rhs);
	}

	template<typename T, typename... TArgs>
	static constexpr T max(
		const T& v0,
		const T& v1,
		TArgs&&... args)
	{
		return max(max(v0, v1), std::forward<TArgs>(args)...);
	}


	template<typename T>
	static constexpr void clamp_i(
		T& value,
		const T& min_value,
		const T& max_value)
	{
		value = std::min(std::max(value, min_value), max_value);
	}

	static Color color_from_imgui(
		const Color imgui_color)
	{
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
		return imgui_color;
#else
		const auto a = (imgui_color >> IM_COL32_A_SHIFT) & 0xFF;
		const auto r = (imgui_color >> IM_COL32_R_SHIFT) & 0xFF;
		const auto g = (imgui_color >> IM_COL32_G_SHIFT) & 0xFF;
		const auto b = (imgui_color >> IM_COL32_B_SHIFT) & 0xFF;

		return (a << 24) | (r << 16) | (g << 8) | b;
#endif // IMGUI_USE_BGRA_PACKED_COLOR
	}

	static bool is_color_transparent(
		const Color color)
	{
		return (color & 0xFF000000) == 0;
	}

	static bool is_color_opaque(
		const Color color)
	{
		return (color & 0xFF000000) == 0xFF000000;
	}

	static Color modulate_color(
		const Color lhs,
		const Color rhs)
	{
		const auto lhs_a = lhs >> 24;
		const auto lhs_r = (lhs >> 16) & 0xFF;
		const auto lhs_g = (lhs >> 8) & 0xFF;
		const auto lhs_b = lhs & 0xFF;

		const auto rhs_a = rhs >> 24;
		const auto rhs_r = (rhs >> 16) & 0xFF;
		const auto rhs_g = (rhs >> 8) & 0xFF;
		const auto rhs_b = rhs & 0xFF;

		const auto a = (lhs_a * rhs_a) / 0xFF;
		const auto r = (lhs_r * rhs_r) / 0xFF;
		const auto g = (lhs_g * rhs_g) / 0xFF;
		const auto b = (lhs_b * rhs_b) / 0xFF;

		return (a << 24) | (r << 16) | (g << 8) | b;
	}

	static void blend_color(
		const Color src,
		Color& dst)
	{
		const auto src_a = src >> 24;
		const auto src_one_minus_a = Color{0xFF} - src_a;

		const auto src_r = (src >> 16) & 0xFF;
		const auto src_g = (src >> 8) & 0xFF;
		const auto src_b = src & 0xFF;

		const auto dst_a = dst >> 24;
		const auto dst_r = (dst >> 16) & 0xFF;
		const auto dst_g = (dst >> 8) & 0xFF;
		const auto dst_b = dst & 0xFF;

		const auto a = (src_a + (src_one_minus_a * dst_a)) / 0xFF;
		const auto r = ((src_a * src_r) + (src_one_minus_a * dst_r)) / 0xFF;
		const auto g = ((src_a * src_g) + (src_one_minus_a * dst_g)) / 0xFF;
		const auto b = ((src_a * src_b) + (src_one_minus_a * dst_b)) / 0xFF;

		dst = (a << 24) | (r << 16) | (g << 8) | b;
	}


	//
	// Resolves an intersection of segments.
	//
	// Parameters:
	//    - p1, p2 - points of the first segment.
	//    - p3, p4 - points of the second segment.
	//    - p - point of intersection.
	//
	// Returns:
	//    - "true" if segments are intersects.
	//    - "false" otherwise.
	//
	// References:
	//    - https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	//
	static bool get_line_with_line_intersection(
		const Point& p1,
		const Point& p2,
		const Point& p3,
		const Point& p4,
		Point& p)
	{
		const auto x12 = p1.x - p2.x;
		const auto y12 = p1.y - p2.y;

		const auto x34 = p3.x - p4.x;
		const auto y34 = p3.y - p4.y;

		const auto den = (x12 * y34) - (y12 * x34);

		if (den == 0)
		{
			// Lines are parallel.
			return false;
		}

		const auto nom1 = (p1.x * p2.y) - (p1.y * p2.x);
		const auto nom2 = (p3.x * p4.y) - (p3.y * p4.x);

		p.x = (nom1 * x34) - (nom2 * x12);
		p.y = (nom1 * y34) - (nom2 * y12);

		if (den == 1)
		{
			return true;
		}
		else if (den == -1)
		{
			p.x = -p.x;
			p.y = -p.y;
		}
		else
		{
			p.x /= den;
			p.y /= den;
		}

		return true;
	}

	//
	// References:
	//    - https://en.wikipedia.org/wiki/Barycentric_coordinate_system
	//
	int get_barycentric_denominator(
		const Triangle& triangle)
	{
		return
			((triangle[1].y - triangle[2].y) * (triangle[0].x - triangle[2].x)) +
			((triangle[2].x - triangle[1].x) * (triangle[0].y - triangle[2].y));
	}

	//
	// References:
	//    - https://en.wikipedia.org/wiki/Barycentric_coordinate_system
	//
	void get_barycentric_numerators(
		const int denominator,
		const Point& point,
		const Triangle& triangle,
		BarycentricNumerators& numerators)
	{
		numerators[0] =
			((triangle[1].y - triangle[2].y) * (point.x - triangle[2].x)) +
			((triangle[2].x - triangle[1].x) * (point.y - triangle[2].y));

		numerators[1] =
			((triangle[2].y - triangle[0].y) * (point.x - triangle[2].x)) +
			((triangle[0].x - triangle[2].x) * (point.y - triangle[2].y));

		numerators[2] = denominator - numerators[0] - numerators[1];
	}

	//
	// References:
	//    - https://en.wikipedia.org/wiki/Barycentric_coordinate_system
	//
	void barycentric_interpolate_color(
		const BarycentricNumerators& barycentric_numerators,
		const int barycentric_denominator,
		const Colors& src_colors,
		Color& dst_color)
	{
		const auto a = static_cast<Color>(((
			(static_cast<int>(src_colors[0] >> 24) * barycentric_numerators[0]) +
			(static_cast<int>(src_colors[1] >> 24) * barycentric_numerators[1]) +
			(static_cast<int>(src_colors[2] >> 24) * barycentric_numerators[2])) /
				barycentric_denominator) & 0xFF);

		const auto r = static_cast<Color>(((
			(static_cast<int>((src_colors[0] >> 16) & 0xFF) * barycentric_numerators[0]) +
			(static_cast<int>((src_colors[1] >> 16) & 0xFF) * barycentric_numerators[1]) +
			(static_cast<int>((src_colors[2] >> 16) & 0xFF) * barycentric_numerators[2])) /
				barycentric_denominator) & 0xFF);

		const auto g = static_cast<Color>(((
			(static_cast<int>((src_colors[0] >> 8) & 0xFF) * barycentric_numerators[0]) +
			(static_cast<int>((src_colors[1] >> 8) & 0xFF) * barycentric_numerators[1]) +
			(static_cast<int>((src_colors[2] >> 8) & 0xFF) * barycentric_numerators[2])) /
				barycentric_denominator) & 0xFF);

		const auto b = static_cast<Color>(((
			(static_cast<int>(src_colors[0] & 0xFF) * barycentric_numerators[0]) +
			(static_cast<int>(src_colors[1] & 0xFF) * barycentric_numerators[1]) +
			(static_cast<int>(src_colors[2] & 0xFF) * barycentric_numerators[2])) /
				barycentric_denominator) & 0xFF);

		dst_color = (a << 24) | (r << 16) | (g << 8) | b;
	}

	void clear_frame_buffer()
	{
		std::uninitialized_fill_n(frame_buffer_.data(), frame_buffer_.size(), Color{});
	}

	void fill_common_edge_tags(
		const CommonEdgeTag tag)
	{
		std::uninitialized_fill(common_edge_map_.begin(), common_edge_map_.end(), tag);
	}

	void increase_common_edge_tag()
	{
		if (common_edge_tag_ < (common_edge_max_tag - 1))
		{
			common_edge_tag_ += 1;
			return;
		}

		common_edge_tag_ = {};
		fill_common_edge_tags(common_edge_max_tag);
	}

	void initialize_common_edge_map()
	{
		has_common_edge_ = {};
		common_edge_tag_ = {};

		common_edge_map_.resize(screen_width_ * screen_height_, common_edge_max_tag);
	}

	//
	// Checks if the last triangle and the provided one has a common edge.
	//
	bool has_common_edge(
		const Triangle& dst_tri)
	{
		for (auto i = 0; i < 3; ++i)
		{
			const auto& src_point_1 = common_edge_prev_triangle_[i];
			const auto& src_point_2 = common_edge_prev_triangle_[(i + 1) % 3];

			for (auto j = 0; j < 3; ++j)
			{
				const auto& dst_point_1 = dst_tri[j];
				const auto& dst_point_2 = dst_tri[(j + 1) % 3];

				if ((src_point_1 == dst_point_1 && src_point_2 == dst_point_2) ||
					(src_point_1 == dst_point_2 && src_point_2 == dst_point_1))
				{
					return true;
				}
			}
		}

		return false;
	}

	void handle_common_edge(
		const Triangle& dst_tri)
	{
		if (!has_common_edge_prev_triangle_)
		{
			has_common_edge_prev_triangle_ = true;
			common_edge_prev_triangle_ = dst_tri;
			has_common_edge_ = true;
			return;
		}

		const auto new_has_common_edge = has_common_edge(dst_tri);

		if (has_common_edge_ && !new_has_common_edge)
		{
			increase_common_edge_tag();
		}

		has_common_edge_ = new_has_common_edge;

		common_edge_prev_triangle_ = dst_tri;
	}

	void draw_triangle(
		const DrawTriangleParameters& parameters)
	{
		const auto& points = parameters.points_;

		handle_common_edge(points);

		if (screen_width_ <= 0 || screen_height_ <= 0)
		{
			return;
		}

		const auto& clip_rect = parameters.clip_rect_;

		const auto has_clip_rect = (
			clip_rect.w > 0 && clip_rect.h > 0 &&
			clip_rect.w != screen_width_ && clip_rect.h != screen_height_);

		const auto& colors = parameters.colors_;
		const auto are_diffuse_colors_same = (colors[0] == colors[1] && colors[0] == colors[2]);

		constexpr auto default_min = std::numeric_limits<int>::max();
		constexpr auto default_max = std::numeric_limits<int>::min();

		auto min_x = min(points[0].x, points[1].x, points[2].x);
		auto min_y = min(points[0].y, points[1].y, points[2].y);

		auto max_x = max(points[0].x, points[1].x, points[2].x);
		auto max_y = max(points[0].y, points[1].y, points[2].y);

		// Trim Y by a screen rectangle.
		//
		clamp_i(min_y, 0, screen_height_ - 1);
		clamp_i(max_y, 0, screen_height_ - 1);

		// Trim Y by a clip rect.
		//
		if (has_clip_rect)
		{
			clamp_i(min_y, clip_rect.y, clip_rect.y + clip_rect.h - 1);
			clamp_i(max_y, clip_rect.y, clip_rect.y + clip_rect.h - 1);
		}

		if (min_y > max_y)
		{
			return;
		}

		auto barycentric_denominator = 0;

		if (!are_diffuse_colors_same || parameters.has_texturing_)
		{
			barycentric_denominator = get_barycentric_denominator(points);
		}

		for (auto y = min_y; y <= max_y; ++y)
		{
			const auto scan_p1 = Point{min_x, y};
			const auto scan_p2 = Point{max_x, y};

			auto is_ip_found = false;
			auto ip_min_x = 0;
			auto ip_max_x = 0;

			// Find intersection points.
			//
			for (auto i = 0; i < 3; ++i)
			{
				auto ip = Point{};

				const auto& p1 = points[i];
				const auto& p2 = points[(i + 1) % 3];

				const auto ip_result = get_line_with_line_intersection(p1, p2, scan_p1, scan_p2, ip);

				if (ip_result &&
					ip.y == y &&
					!((p1.y < y && p2.y < y) || (p1.y > y && p2.y > y)))
				{
					if (is_ip_found)
					{
						ip_min_x = std::min(ip_min_x, ip.x);
						ip_max_x = std::max(ip_max_x, ip.x);
					}
					else
					{
						ip_min_x = ip.x;
						ip_max_x = ip.x;

						is_ip_found = true;
					}
				}
			}

			if (!is_ip_found)
			{
				// No intersection.
				continue;
			}

			// Trim X by a screen rectangle.
			//
			clamp_i(ip_min_x, 0, screen_width_ - 1);
			clamp_i(ip_max_x, 0, screen_width_ - 1);

			// Trim X by a clip rect.
			//
			if (has_clip_rect)
			{
				clamp_i(ip_min_x, clip_rect.x, clip_rect.x + clip_rect.w - 1);
				clamp_i(ip_max_x, clip_rect.x, clip_rect.x + clip_rect.w - 1);
			}

			if (ip_min_x > ip_max_x)
			{
				continue;
			}

			if (parameters.has_texturing_)
			{
				auto barycentric_numerators = BarycentricNumerators{};

				for (auto ip_x = ip_min_x; ip_x <= ip_max_x; ++ip_x)
				{
					const auto pixel_index = (y * screen_width_) + ip_x;

					if (has_common_edge_ && common_edge_map_[pixel_index] == common_edge_tag_)
					{
						continue;
					}

					if (barycentric_denominator != 0)
					{
						const auto current_point = Point{ip_x, y};

						get_barycentric_numerators(
							barycentric_denominator,
							current_point,
							points,
							barycentric_numerators);
					}

					auto mod_color = Color{};

					if (are_diffuse_colors_same || barycentric_denominator == 0)
					{
						mod_color = colors[0];
					}
					else
					{
						barycentric_interpolate_color(
							barycentric_numerators,
							barycentric_denominator,
							colors,
							mod_color);
					}

					if (is_color_transparent(mod_color))
					{
						continue;
					}

					auto tx_color = Color{0xFFFFFFFF};

					if (barycentric_denominator != 0)
					{
						const auto& tx_coords = parameters.tx_coords_;

						const auto u = (
							(tx_coords[0].x * barycentric_numerators[0]) +
							(tx_coords[1].x * barycentric_numerators[1]) +
							(tx_coords[2].x * barycentric_numerators[2])) / barycentric_denominator;

						const auto v = (
							(tx_coords[0].y * barycentric_numerators[0]) +
							(tx_coords[1].y * barycentric_numerators[1]) +
							(tx_coords[2].y * barycentric_numerators[2])) / barycentric_denominator;

						const auto sdl_surface = parameters.sdl_texture_surface_;

						if (u >= 0 && u < sdl_surface->w && v >= 0 && v < sdl_surface->h)
						{
							tx_color = (static_cast<const Color*>(sdl_surface->pixels))[(v * (sdl_surface->pitch / 4)) + u];
						}
					}

					if (is_color_transparent(tx_color))
					{
						continue;
					}

					const auto color = modulate_color(mod_color, tx_color);

					blend_color(color, frame_buffer_[pixel_index]);

					common_edge_map_[pixel_index] = common_edge_tag_;
				}
			}
			else
			{
				if (are_diffuse_colors_same)
				{
					const auto same_color = colors[0];
					const auto is_opaque = is_color_opaque(same_color);

					for (auto ip_x = ip_min_x; ip_x <= ip_max_x; ++ip_x)
					{
						const auto pixel_index = (y * screen_width_) + ip_x;

						if (has_common_edge_ && common_edge_map_[pixel_index] == common_edge_tag_)
						{
							continue;
						}

						blend_color(same_color, frame_buffer_[pixel_index]);

						common_edge_map_[pixel_index] = common_edge_tag_;
					}
				}
				else if (barycentric_denominator != 0)
				{
					for (auto ip_x = ip_min_x; ip_x <= ip_max_x; ++ip_x)
					{
						const auto pixel_index = (y * screen_width_) + ip_x;

						if (has_common_edge_ && common_edge_map_[pixel_index] == common_edge_tag_)
						{
							continue;
						}

						const auto current_point = Point{ip_x, y};

						auto barycentric_numerators = BarycentricNumerators{};

						get_barycentric_numerators(
							barycentric_denominator,
							current_point,
							points,
							barycentric_numerators);

						auto color = Color{};

						barycentric_interpolate_color(
							barycentric_numerators,
							barycentric_denominator,
							colors,
							color);

						if (is_color_transparent(color))
						{
							continue;
						}

						blend_color(color, frame_buffer_[pixel_index]);

						common_edge_map_[pixel_index] = common_edge_tag_;
					}
				}
			}
		}
	}

	void handle_window_event(
		const SDL_WindowEvent& sdl_event)
	{
		if (sdl_event.windowID != sdl_window_id_)
		{
			return;
		}

		switch (sdl_event.event)
		{
		case SDL_WINDOWEVENT_EXPOSED:
		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESTORED:
		case SDL_WINDOWEVENT_SHOWN:
			window_status_ = WindowStatus::shown;
			break;

		case SDL_WINDOWEVENT_HIDDEN:
			window_status_ = WindowStatus::hidden;
			break;

		case SDL_WINDOWEVENT_MINIMIZED:
			window_status_ = WindowStatus::minimized;
			break;

		case SDL_WINDOWEVENT_CLOSE:
			is_close_requested_ = true;
			break;
		}
	}

	void handle_mouse_motion_event(
		const SDL_MouseMotionEvent& sdl_event)
	{
		if (sdl_event.windowID != sdl_window_id_)
		{
			return;
		}

		auto& imgui_io = ImGui::GetIO();
		imgui_io.MousePos = {static_cast<float>(sdl_event.x), static_cast<float>(sdl_event.y)};
	}

	void handle_mouse_wheel(
		const SDL_MouseWheelEvent& sdl_event)
	{
		if (sdl_event.windowID != sdl_window_id_)
		{
			return;
		}

		auto& imgui_io = ImGui::GetIO();

		if (sdl_event.x > 0)
		{
			imgui_io.MouseWheelH += 1;
		}

		if (sdl_event.x < 0)
		{
			imgui_io.MouseWheelH -= 1;
		}

		if (sdl_event.y > 0)
		{
			imgui_io.MouseWheel += 1;
		}

		if (sdl_event.y < 0)
		{
			imgui_io.MouseWheel -= 1;
		}
	}

	void handle_mouse_button(
		const SDL_MouseButtonEvent& sdl_event)
	{
		if (sdl_event.windowID != sdl_window_id_ || sdl_event.type != SDL_MOUSEBUTTONDOWN)
		{
			return;
		}

		auto& imgui_io = ImGui::GetIO();

		auto mouse_index = -1;

		switch (sdl_event.button)
		{
		case SDL_BUTTON_LEFT:
			mouse_index = 0;
			break;

		case SDL_BUTTON_MIDDLE:
			mouse_index = 2;
			break;

		case SDL_BUTTON_RIGHT:
			mouse_index = 1;
			break;

		default:
			return;
		}

		mouse_buttons_state_.set(mouse_index);
	}

	void handle_text_input_event(
		const SDL_TextInputEvent& sdl_event)
	{
		if (sdl_event.windowID != sdl_window_id_)
		{
			return;
		}

		auto& imgui_io = ImGui::GetIO();

		imgui_io.AddInputCharactersUTF8(sdl_event.text);
	}

	void handle_keyboard_event(
		const SDL_KeyboardEvent& sdl_event)
	{
		if (sdl_event.windowID != sdl_window_id_)
		{
			return;
		}

		auto& imgui_io = ImGui::GetIO();

		const auto key = sdl_event.keysym.scancode;
		imgui_io.KeysDown[key] = (sdl_event.type == SDL_KEYDOWN);

		imgui_io.KeyShift = ((sdl_event.keysym.mod & KMOD_SHIFT) != 0);
		imgui_io.KeyCtrl = ((sdl_event.keysym.mod & KMOD_CTRL) != 0);
		imgui_io.KeyAlt = ((sdl_event.keysym.mod & KMOD_ALT) != 0);
		imgui_io.KeySuper = ((sdl_event.keysym.mod & KMOD_GUI) != 0);
	}

	void initialize_imgui_key_map()
	{
		auto& imgui_io = ImGui::GetIO();

		imgui_io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
		imgui_io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
		imgui_io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
		imgui_io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
		imgui_io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
		imgui_io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
		imgui_io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
		imgui_io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
		imgui_io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
		imgui_io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
		imgui_io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
		imgui_io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
		imgui_io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
		imgui_io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
		imgui_io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
		imgui_io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
		imgui_io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
		imgui_io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
		imgui_io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
		imgui_io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
		imgui_io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;
	}

	void initialize_imgui_clipboard()
	{
		auto& imgui_io = ImGui::GetIO();

		clipboard_buffer_.clear();

		imgui_io.ClipboardUserData = this;
		imgui_io.GetClipboardTextFn = get_clipboard_text_fn_proxy;
		imgui_io.SetClipboardTextFn = set_clipboard_text_fn_proxy;
	}

	void initialize_imgui_io()
	{
		initialize_imgui_key_map();
		initialize_imgui_clipboard();
	}

	const char* get_clipboard_text_fn()
	{
		const auto clipboard_text = ::SDL_GetClipboardText();

		if (clipboard_text)
		{
			clipboard_buffer_ = clipboard_text;
			::SDL_free(clipboard_text);
		}
		else
		{
			clipboard_buffer_.clear();
		}

		return clipboard_buffer_.c_str();
	}

	void set_clipboard_text_fn(
		const char* text)
	{
		static_cast<void>(::SDL_SetClipboardText(text));
	}

	static const char* get_clipboard_text_fn_proxy(
		void* user_data)
	{
		return static_cast<Impl*>(user_data)->get_clipboard_text_fn();
	}

	static void set_clipboard_text_fn_proxy(
		void* user_data,
		const char* text)
	{
		static_cast<Impl*>(user_data)->set_clipboard_text_fn(text);
	}
}; // ImGuiSdl::Impl

//
// ImGuiSdl::Impl
// ==========================================================================

// ==========================================================================
// ImGuiSdl
//

ImGuiSdl::ImGuiSdl()
	:
	pimpl_{new Impl{}}
{
}

ImGuiSdl::ImGuiSdl(
	ImGuiSdl&& that)
	:
	pimpl_{std::move(that.pimpl_)}
{
}

ImGuiSdl::~ImGuiSdl()
{
}

const std::string& ImGuiSdl::get_error_message() const
{
	return pimpl_->get_error_message();
}

bool ImGuiSdl::initialize(
	SDL_Window* sdl_window)
{
	return pimpl_->api_initialize(sdl_window);
}

void ImGuiSdl::uninitialize()
{
	pimpl_->api_uninitialize();
}

void ImGuiSdl::new_frame()
{
	pimpl_->api_handle_new_frame();
}

void ImGuiSdl::handle_event(
	const SDL_Event& sdl_event)
{
	pimpl_->api_handle_event(sdl_event);
}

void ImGuiSdl::draw(
	ImDrawData* imgui_draw_data)
{
	pimpl_->api_handle_draw_data(imgui_draw_data);
}

void ImGuiSdl::present()
{
	pimpl_->api_present();
}

SDL_Surface* ImGuiSdl::create_texture_id(
	const int width,
	const int height,
	const PixelFormat src_pixel_format,
	const void* raw_pixels)
{
	return pimpl_->api_create_sdl_rgba_surface(width, height, src_pixel_format, raw_pixels);
}

ImGuiSdl::WindowStatus ImGuiSdl::get_window_status() const
{
	return pimpl_->api_get_window_status();
}

bool ImGuiSdl::is_close_requested() const
{
	return pimpl_->api_is_close_requested();
}

void ImGuiSdl::reset_is_close_requested()
{
	pimpl_->api_reset_is_close_requested();
}

//
// ImGuiSdl
// ==========================================================================
