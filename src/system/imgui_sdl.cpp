// By Emil Ernerfeldt 2018
// LICENSE:
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
// WHAT:
//   This is a software renderer for Dear ImGui.
//   It is decently fast, but has a lot of room for optimization.
//   The goal was to get something fast and decently accurate in not too many lines of code.
// LIMITATIONS:
//   * It is not pixel-perfect, but it is good enough for must use cases.


/*

Software renderer powered by SDL2 for Dear ImGui.
Based on implementation by Emil Ernerfeldt.
https://github.com/emilk/imgui_software_renderer

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
// Software renderer powered by SDL2 for Dear ImGui.
//
// Notes:
//    - Supports only SDL_Surface as texture id.
//    - Supports only ARGB8888 pixel format (see "create_texture_id").
//


#include "imgui_sdl.h"
#include <array>
#include <bitset>
#include <limits>
#include <utility>
#include <vector>
#include "imgui.h"
#include "SDL_clipboard.h"
#include "SDL_render.h"


bool operator==(
	const SDL_Point& lhs,
	const SDL_Point& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

// ----------------------------------------------------------------------------
// Useful operators on ImGui vectors:

ImVec2 operator*(
	const float f,
	const ImVec2& v)
{
	return ImVec2{f * v.x, f * v.y};
}

ImVec2 operator+(
	const ImVec2& a,
	const ImVec2& b)
{
	return ImVec2{a.x + b.x, a.y + b.y};
}

ImVec2 operator-(
	const ImVec2& a,
	const ImVec2& b)
{
	return ImVec2{a.x - b.x, a.y - b.y};
}

bool operator!=(
	const ImVec2& a,
	const ImVec2& b)
{
	return a.x != b.x || a.y != b.y;
}

ImVec4 operator*(
	const float f,
	const ImVec4& v)
{
	return ImVec4{f * v.x, f * v.y, f * v.z, f * v.w};
}

ImVec4 operator+(
	const ImVec4& a,
	const ImVec4& b)
{
	return ImVec4{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}


// ----------------------------------------------------------------------------
// Used for interpolating vertex attributes (color and texture coordinates) in a triangle.

struct Barycentric
{
	float w0;
	float w1;
	float w2;
}; // Barycentric

Barycentric operator*(
	const float f,
	const Barycentric& va)
{
	return {f * va.w0, f * va.w1, f * va.w2};
}

void operator+=(
	Barycentric& a,
	const Barycentric& b)
{
	a.w0 += b.w0;
	a.w1 += b.w1;
	a.w2 += b.w2;
}

Barycentric operator+(
	const Barycentric& a,
	const Barycentric& b)
{
	return Barycentric{a.w0 + b.w0, a.w1 + b.w1, a.w2 + b.w2};
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
		mouse_buttons_state_{}
	{
	}

	Impl(
		const Impl& that) = delete;

	Impl& operator=(
		const Impl& that) = delete;

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

		const auto list_count = draw_data->CmdListsCount;

		if (list_count <= 0)
		{
			return;
		}

		auto& im_io = ImGui::GetIO();

		for (int i = 0; i < list_count; ++i)
		{
			paint_draw_list(draw_data->CmdLists[i]);
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

		auto sdl_surface = ::SDL_CreateRGBSurfaceFrom(
			const_cast<void*>(raw_pixels),
			width,
			height,
			32,
			width * 4,
			0x00FF0000,
			0x0000FF00,
			0x000000FF,
			0xFF000000);

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
	using FrameBuffer = std::vector<Color>;


	static constexpr auto max_mouse_button_count = 3;
	using MouseButtonsState = std::bitset<max_mouse_button_count>;


	struct PaintTarget
	{
		std::uint32_t* pixels;
		int width;
		int height;
		ImVec2 scale; // Multiply ImGui (point) coordinates with this to get pixel coordinates.
	};

	struct ColorInt
	{
		std::uint32_t a;
		std::uint32_t b;
		std::uint32_t g;
		std::uint32_t r;

		ColorInt() = default;

		explicit ColorInt(
			const std::uint32_t x)
		{
			a = (x >> IM_COL32_A_SHIFT) & 0xFF;
			b = (x >> IM_COL32_B_SHIFT) & 0xFF;
			g = (x >> IM_COL32_G_SHIFT) & 0xFF;
			r = (x >> IM_COL32_R_SHIFT) & 0xFF;
		}

		std::uint32_t to_uint32() const
		{
			return (a << 24) | (r << 16) | (g << 8) | b;
		}

		static ColorInt modulate(
			const ColorInt& lhs,
			const ColorInt& rhs)
		{
			ColorInt result;

			result.a = (lhs.a * rhs.a) / 0xFF;
			result.b = (lhs.b * rhs.b) / 0xFF;
			result.g = (lhs.g * rhs.g) / 0xFF;
			result.r = (lhs.r * rhs.r) / 0xFF;

			return result;
		}
	};

	struct Point
	{
		std::int64_t x;
		std::int64_t y;
	}; // Point


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

	static ColorInt blend(
		const ColorInt& target,
		const ColorInt& source)
	{
		const auto one_minus_src_a = 0xFF - source.a;

		ColorInt result;

		result.a = (source.a + (one_minus_src_a * target.a)) / 0xFF;
		result.b = ((source.b * source.a) + (target.b * one_minus_src_a)) / 0xFF;
		result.g = ((source.g * source.a) + (target.g * one_minus_src_a)) / 0xFF;
		result.r = ((source.r * source.a) + (target.r * one_minus_src_a)) / 0xFF;

		return result;
	}

	// ----------------------------------------------------------------------------
	// Copies of functions in ImGui, inlined for speed:

	static ImVec4 color_convert_u32_to_float4(
		const ImU32 in)
	{
		const auto s = 1.0F / 255.0F;

		return ImVec4
		{
			((in >> IM_COL32_R_SHIFT) & 0xFF) * s,
			((in >> IM_COL32_G_SHIFT) & 0xFF) * s,
			((in >> IM_COL32_B_SHIFT) & 0xFF) * s,
			((in >> IM_COL32_A_SHIFT) & 0xFF) * s,
		};
	}

	static ImU32 color_convert_float4_to_u32(
		const ImVec4& in)
	{
		ImU32 out;

		out = static_cast<std::uint32_t>((in.x * 255.0F) + 0.5F) << 16;
		out |= static_cast<std::uint32_t>((in.y * 255.0F) + 0.5F) << 8;
		out |= static_cast<std::uint32_t>((in.z * 255.0F) + 0.5F) << 0;
		out |= static_cast<std::uint32_t>((in.w * 255.0F) + 0.5F) << 24;

		return out;
	}

	// ----------------------------------------------------------------------------
	// For fast and subpixel-perfect triangle rendering we used fixed point arithmetic.
	// To keep the code simple we use 64 bits to avoid overflows.

	static const std::int64_t fixed_bias = 256;

	static std::int64_t orient2d(
		const Point& a,
		const Point& b,
		const Point& c)
	{
		return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	}

	static std::int64_t as_int(
		const float v)
	{
		return static_cast<std::int64_t>(std::floor(v * fixed_bias));
	}

	static Point as_point(
		const ImVec2& v)
	{
		return Point{as_int(v.x), as_int(v.y)};
	}

	static float min3(
		const float a,
		const float b,
		const float c)
	{
		if (a < b && a < c)
		{
			return a;
		}

		return b < c ? b : c;
	}

	static float max3(
		const float a,
		const float b,
		const float c)
	{
		if (a > b && a > c)
		{
			return a;
		}

		return b > c ? b : c;
	}

	static float barycentric(
		const ImVec2& a,
		const ImVec2& b,
		const ImVec2& point)
	{
		return ((b.x - a.x) * (point.y - a.y)) - ((b.y - a.y) * (point.x - a.x));
	}

	static std::uint32_t sample_texture(
		const SDL_Surface& texture,
		const ImVec2& uv)
	{
		auto tx = static_cast<int>(uv.x * (texture.w - 1.0F) + 0.5F);
		auto ty = static_cast<int>(uv.y * (texture.h - 1.0F) + 0.5F);

		// Clamp to inside of texture:
		tx = std::max(tx, 0);
		tx = std::min(tx, texture.w - 1);
		ty = std::max(ty, 0);
		ty = std::min(ty, texture.h - 1);

		auto row = reinterpret_cast<const std::uint32_t*>(static_cast<const std::uint8_t*>(texture.pixels) + (ty * texture.pitch));

		const auto argb_color = row[tx];

		const auto im_color =
			(((argb_color >> 24) & 0xFF) << IM_COL32_A_SHIFT) |
			(((argb_color >> 16) & 0xFF) << IM_COL32_R_SHIFT) |
			(((argb_color >> 8) & 0xFF) << IM_COL32_G_SHIFT) |
			(((argb_color >> 0) & 0xFF) << IM_COL32_B_SHIFT);

		return im_color;
	}

	void paint_uniform_rectangle(
		const ImVec2& min_f,
		const ImVec2& max_f,
		const ColorInt& color)
	{
		// Integer bounding box [min, max):
		auto min_x_i = static_cast<int>(min_f.x + 0.5F);
		auto min_y_i = static_cast<int>(min_f.y + 0.5F);
		auto max_x_i = static_cast<int>(max_f.x + 0.5F);
		auto max_y_i = static_cast<int>(max_f.y + 0.5F);

		// Clamp to render target:
		min_x_i = std::max(min_x_i, 0);
		min_y_i = std::max(min_y_i, 0);
		max_x_i = std::min(max_x_i, screen_width_);
		max_y_i = std::min(max_y_i, screen_height_);

		// We often blend the same colors over and over again, so optimize for this (saves 25% total cpu):
		auto last_target_pixel = frame_buffer_[(min_y_i * screen_width_) + min_x_i];
		auto last_output = blend(ColorInt{last_target_pixel}, color).to_uint32();

		for (int y = min_y_i; y < max_y_i; ++y)
		{
			for (int x = min_x_i; x < max_x_i; ++x)
			{
				auto& target_pixel = frame_buffer_[(y * screen_width_) + x];

				if (target_pixel == last_target_pixel)
				{
					target_pixel = last_output;
					continue;
				}

				last_target_pixel = target_pixel;
				target_pixel = blend(ColorInt{target_pixel}, color).to_uint32();
				last_output = target_pixel;
			}
		}
	}

	void paint_uniform_textured_rectangle(
		const SDL_Surface& texture,
		const ImVec4& clip_rect,
		const ImDrawVert& min_v,
		const ImDrawVert& max_v)
	{
		const auto& min_p = ImVec2{min_v.pos.x, min_v.pos.y};
		const auto& max_p = ImVec2{max_v.pos.x, max_v.pos.y};

		// Find bounding box:
		auto min_x_f = min_p.x;
		auto min_y_f = min_p.y;
		auto max_x_f = max_p.x;
		auto max_y_f = max_p.y;

		// Clip against clip_rect:
		min_x_f = std::max(min_x_f, clip_rect.x);
		min_y_f = std::max(min_y_f, clip_rect.y);
		max_x_f = std::min(max_x_f, clip_rect.z - 0.5F);
		max_y_f = std::min(max_y_f, clip_rect.w - 0.5F);

		// Integer bounding box [min, max):
		auto min_x_i = static_cast<int>(min_x_f);
		auto min_y_i = static_cast<int>(min_y_f);
		auto max_x_i = static_cast<int>(max_x_f + 1.0F);
		auto max_y_i = static_cast<int>(max_y_f + 1.0F);

		// Clip against render target:
		min_x_i = std::max(min_x_i, 0);
		min_y_i = std::max(min_y_i, 0);
		max_x_i = std::min(max_x_i, screen_width_);
		max_y_i = std::min(max_y_i, screen_height_);

		const auto top_left = ImVec2(min_x_i + 0.5f, min_y_i + 0.5f);

		const auto delta_uv_per_pixel = ImVec2
		{
			(max_v.uv.x - min_v.uv.x) / (max_p.x - min_p.x),
			(max_v.uv.y - min_v.uv.y) / (max_p.y - min_p.y),
		};

		const auto uv_topleft = ImVec2
		{
			min_v.uv.x + ((top_left.x - min_v.pos.x) * delta_uv_per_pixel.x),
			min_v.uv.y + ((top_left.y - min_v.pos.y) * delta_uv_per_pixel.y),
		};

		auto current_uv = uv_topleft;

		for (int y = min_y_i; y < max_y_i; ++y, current_uv.y += delta_uv_per_pixel.y)
		{
			current_uv.x = uv_topleft.x;

			for (int x = min_x_i; x < max_x_i; ++x, current_uv.x += delta_uv_per_pixel.x)
			{
				const auto texel = sample_texture(texture, current_uv);

				// The font texture is all black or all white, so optimize for this:
				if ((texel & IM_COL32_A_MASK) == 0)
				{
					continue;
				}

				auto& target_pixel = frame_buffer_[y * screen_width_ + x];

				if (texel == 0xFFFFFFFF)
				{
					target_pixel = ColorInt(min_v.col).to_uint32();
					continue;
				}

				// Other textured rectangles
				const auto& source_color = ColorInt::modulate(ColorInt{min_v.col}, ColorInt{texel});
				target_pixel = blend(ColorInt{target_pixel}, source_color).to_uint32();
			}
		}
	}

	// When two triangles share an edge, we want to draw the pixels on that edge exactly once.
	// The edge will be the same, but the direction will be the opposite
	// (assuming the two triangles have the same winding order).
	// Which edge wins? This functions decides.
	static bool is_dominant_edge(
		const ImVec2& edge)
	{
		return edge.y > 0 || (edge.y == 0 && edge.x < 0);
	}

	// Handles triangles in any winding order (CW/CCW)
	void paint_triangle(
		const SDL_Surface* texture,
		const ImVec4& clip_rect,
		const ImDrawVert& v0,
		const ImDrawVert& v1,
		const ImDrawVert& v2)
	{
		const auto& p0 = ImVec2{v0.pos.x, v0.pos.y};
		const auto& p1 = ImVec2{v1.pos.x, v1.pos.y};
		const auto& p2 = ImVec2{v2.pos.x, v2.pos.y};

		// Can be positive or negative depending on winding order
		const auto rect_area = barycentric(p0, p1, p2);

		if (rect_area == 0.0F)
		{
			return;
		}

		// Find bounding box:
		auto min_x_f = min3(p0.x, p1.x, p2.x);
		auto min_y_f = min3(p0.y, p1.y, p2.y);
		auto max_x_f = max3(p0.x, p1.x, p2.x);
		auto max_y_f = max3(p0.y, p1.y, p2.y);

		// Clip against clip_rect:
		min_x_f = std::max(min_x_f, clip_rect.x);
		min_y_f = std::max(min_y_f, clip_rect.y);
		max_x_f = std::min(max_x_f, clip_rect.z - 0.5F);
		max_y_f = std::min(max_y_f, clip_rect.w - 0.5F);

		// Integer bounding box [min, max):
		auto min_x_i = static_cast<int>(min_x_f);
		auto min_y_i = static_cast<int>(min_y_f);
		auto max_x_i = static_cast<int>(max_x_f + 1.0F);
		auto max_y_i = static_cast<int>(max_y_f + 1.0F);

		// Clip against render target:
		min_x_i = std::max(min_x_i, 0);
		min_y_i = std::max(min_y_i, 0);
		max_x_i = std::min(max_x_i, screen_width_);
		max_y_i = std::min(max_y_i, screen_height_);

		// ------------------------------------------------------------------------
		// Set up interpolation of barycentric coordinates:

		const auto& topleft = ImVec2{static_cast<float>(min_x_i), static_cast<float>(min_y_i)};
		const auto& dx = ImVec2{1, 0};
		const auto& dy = ImVec2{0, 1};

		const auto w0_topleft = barycentric(p1, p2, topleft);
		const auto w1_topleft = barycentric(p2, p0, topleft);
		const auto w2_topleft = barycentric(p0, p1, topleft);

		const auto w0_dx = barycentric(p1, p2, topleft + dx) - w0_topleft;
		const auto w1_dx = barycentric(p2, p0, topleft + dx) - w1_topleft;
		const auto w2_dx = barycentric(p0, p1, topleft + dx) - w2_topleft;

		const auto w0_dy = barycentric(p1, p2, topleft + dy) - w0_topleft;
		const auto w1_dy = barycentric(p2, p0, topleft + dy) - w1_topleft;
		const auto w2_dy = barycentric(p0, p1, topleft + dy) - w2_topleft;

		const auto& bary_0 = Barycentric{1, 0, 0};
		const auto& bary_1 = Barycentric{0, 1, 0};
		const auto& bary_2 = Barycentric{0, 0, 1};

		const auto inv_area = 1.0F / rect_area;

		const auto& bary_topleft = inv_area * ((w0_topleft * bary_0) + (w1_topleft * bary_1) + (w2_topleft * bary_2));
		const auto& bary_dx = inv_area * ((w0_dx * bary_0) + (w1_dx * bary_1) + (w2_dx * bary_2));
		const auto& bary_dy = inv_area * ((w0_dy * bary_0) + (w1_dy * bary_1) + (w2_dy * bary_2));

		auto bary_current_row = bary_topleft;

		// ------------------------------------------------------------------------
		// For pixel-perfect inside/outside testing:

		const auto sign = (rect_area > 0 ? 1 : -1); // winding order?

		const auto bias0i = (is_dominant_edge(p2 - p1) ? 0 : -1);
		const auto bias1i = (is_dominant_edge(p0 - p2) ? 0 : -1);
		const auto bias2i = (is_dominant_edge(p1 - p0) ? 0 : -1);

		const auto p0i = as_point(p0);
		const auto p1i = as_point(p1);
		const auto p2i = as_point(p2);

		// ------------------------------------------------------------------------

		const bool has_uniform_color = (v0.col == v1.col && v0.col == v2.col);

		const auto& c0 = color_convert_u32_to_float4(v0.col);
		const auto& c1 = color_convert_u32_to_float4(v1.col);
		const auto& c2 = color_convert_u32_to_float4(v2.col);

		// We often blend the same colors over and over again, so optimize for this (saves 10% total cpu):
		auto last_target_pixel = std::uint32_t{};
		auto last_output = blend(ColorInt{last_target_pixel}, ColorInt{v0.col}).to_uint32();

		for (int y = min_y_i; y < max_y_i; ++y)
		{
			auto bary = bary_current_row;

			bool has_been_inside_this_row = false;

			for (int x = min_x_i; x < max_x_i; ++x)
			{
				const auto w0 = bary.w0;
				const auto w1 = bary.w1;
				const auto w2 = bary.w2;

				bary += bary_dx;

				{
					// Inside/outside test:
					const auto p = Point{(fixed_bias * x) + (fixed_bias / 2), (fixed_bias * y) + (fixed_bias / 2)};
					const auto w0i = (sign * orient2d(p1i, p2i, p)) + bias0i;
					const auto w1i = (sign * orient2d(p2i, p0i, p)) + bias1i;
					const auto w2i = (sign * orient2d(p0i, p1i, p)) + bias2i;

					if (w0i < 0 || w1i < 0 || w2i < 0)
					{
						if (has_been_inside_this_row)
						{
							// Gives a nice 10% speedup
							break;
						}
						else
						{
							continue;
						}
					}
				}

				has_been_inside_this_row = true;

				auto& target_pixel = frame_buffer_[(y * screen_width_) + x];

				if (has_uniform_color && !texture)
				{
					if (target_pixel == last_target_pixel)
					{
						target_pixel = last_output;
						continue;
					}

					last_target_pixel = target_pixel;
					target_pixel = blend(ColorInt{target_pixel}, ColorInt{v0.col}).to_uint32();
					last_output = target_pixel;

					continue;
				}

				ImVec4 src_color;

				if (has_uniform_color)
				{
					src_color = c0;
				}
				else
				{
					src_color = (w0 * c0) + (w1 * c1) + (w2 * c2);
				}

				if (texture)
				{
					const ImVec2& uv = (w0 * v0.uv) + (w1 * v1.uv) + (w2 * v2.uv);
					const auto alpha = (sample_texture(*texture, uv) >> IM_COL32_A_SHIFT) & 0xFF;

					src_color.w *= alpha / 255.0F;
				}

				if (src_color.w <= 0.0F)
				{
					// Transparent.
					continue;
				}

				if (src_color.w >= 1.0F)
				{
					// Opaque, no blending needed:
					target_pixel = color_convert_float4_to_u32(src_color);
					continue;
				}

				const auto& target_color = color_convert_u32_to_float4(target_pixel);
				const auto blended_color = (src_color.w * src_color) + ((1.0F - src_color.w) * target_color);
				target_pixel = color_convert_float4_to_u32(blended_color);
			}

			bary_current_row += bary_dy;
		}
	}

	void paint_draw_cmd(
		const ImDrawVert* vertices,
		const ImDrawIdx* idx_buffer,
		const ImDrawCmd& pcmd)
	{
		const auto texture = reinterpret_cast<const SDL_Surface*>(pcmd.TextureId);
		assert(texture);

		// ImGui uses the first pixel for "white".
		const auto& white_uv = ImVec2{0.5F / texture->w, 0.5f / texture->h};

		for (unsigned int i = 0; i + 3 <= pcmd.ElemCount; )
		{
			const auto& v0 = vertices[idx_buffer[i + 0]];
			const auto& v1 = vertices[idx_buffer[i + 1]];
			const auto& v2 = vertices[idx_buffer[i + 2]];

			// Text is common, and is made of textured rectangles. So let's optimize for it.
			// This assumes the ImGui way to layout text does not change.
			if ((i + 6) <= pcmd.ElemCount &&
				idx_buffer[i + 3] == idx_buffer[i + 0] &&
				idx_buffer[i + 4] == idx_buffer[i + 2])
			{
				const auto& v3 = vertices[idx_buffer[i + 5]];

				if (v0.pos.x == v3.pos.x &&
					v1.pos.x == v2.pos.x &&
					v0.pos.y == v1.pos.y &&
					v2.pos.y == v3.pos.y &&
					v0.uv.x == v3.uv.x &&
					v1.uv.x == v2.uv.x &&
					v0.uv.y == v1.uv.y &&
					v2.uv.y == v3.uv.y)
				{
					const bool has_uniform_color =
						(v0.col == v1.col && v0.col == v2.col && v0.col == v3.col);

					const bool has_texture = (
						v0.uv != white_uv ||
						v1.uv != white_uv ||
						v2.uv != white_uv ||
						v3.uv != white_uv);

					if (has_uniform_color && has_texture)
					{
						paint_uniform_textured_rectangle(*texture, pcmd.ClipRect, v0, v2);
						i += 6;
						continue;
					}
				}
			}

			// A lot of the big stuff are uniformly colored rectangles,
			// so we can save a lot of CPU by detecting them:
			if ((i + 6) <= pcmd.ElemCount)
			{
				const auto& v3 = vertices[idx_buffer[i + 3]];
				const auto& v4 = vertices[idx_buffer[i + 4]];
				const auto& v5 = vertices[idx_buffer[i + 5]];

				ImVec2 min;

				min.x = min3(v0.pos.x, v1.pos.x, v2.pos.x);
				min.y = min3(v0.pos.y, v1.pos.y, v2.pos.y);

				ImVec2 max;

				max.x = max3(v0.pos.x, v1.pos.x, v2.pos.x);
				max.y = max3(v0.pos.y, v1.pos.y, v2.pos.y);

				// Not the prettiest way to do this, but it catches all cases
				// of a rectangle split into two triangles.
				// TODO: Stop it from also assuming duplicate triangles is one rectangle.
				if ((v0.pos.x == min.x || v0.pos.x == max.x) &&
					(v0.pos.y == min.y || v0.pos.y == max.y) &&
					(v1.pos.x == min.x || v1.pos.x == max.x) &&
					(v1.pos.y == min.y || v1.pos.y == max.y) &&
					(v2.pos.x == min.x || v2.pos.x == max.x) &&
					(v2.pos.y == min.y || v2.pos.y == max.y) &&
					(v3.pos.x == min.x || v3.pos.x == max.x) &&
					(v3.pos.y == min.y || v3.pos.y == max.y) &&
					(v4.pos.x == min.x || v4.pos.x == max.x) &&
					(v4.pos.y == min.y || v4.pos.y == max.y) &&
					(v5.pos.x == min.x || v5.pos.x == max.x) &&
					(v5.pos.y == min.y || v5.pos.y == max.y))
				{
					const bool has_uniform_color = (
						v0.col == v1.col &&
						v0.col == v2.col &&
						v0.col == v3.col &&
						v0.col == v4.col &&
						v0.col == v5.col);

					const bool has_texture = (
						v0.uv != white_uv ||
						v1.uv != white_uv ||
						v2.uv != white_uv ||
						v3.uv != white_uv ||
						v4.uv != white_uv ||
						v5.uv != white_uv);

					min.x = std::max(min.x, pcmd.ClipRect.x);
					min.y = std::max(min.y, pcmd.ClipRect.y);
					max.x = std::min(max.x, pcmd.ClipRect.z - 0.5F);
					max.y = std::min(max.y, pcmd.ClipRect.w - 0.5F);

					if (max.x < min.x || max.y < min.y)
					{
						// Completely clipped

						i += 6;
						continue;
					}

					const auto num_pixels = (max.x - min.x) * (max.y - min.y);

					if (has_uniform_color)
					{
						if (has_texture)
						{
						}
						else
						{
							paint_uniform_rectangle(min, max, ColorInt{v0.col});
							i += 6;
							continue;
						}
					}
					else
					{
						if (has_texture)
						{
							// I have never encountered these.
						}
						else
						{
							// Color picker. TODO: Optimize
						}
					}
				}
			}

			const bool has_texture = (v0.uv != white_uv || v1.uv != white_uv || v2.uv != white_uv);
			paint_triangle(has_texture ? texture : nullptr, pcmd.ClipRect, v0, v1, v2);
			i += 3;
		}
	}

	void paint_draw_list(
		const ImDrawList* cmd_list)
	{
		auto idx_buffer = &cmd_list->IdxBuffer[0];
		const auto vertices = cmd_list->VtxBuffer.Data;

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); ++cmd_i)
		{
			const ImDrawCmd& pcmd = cmd_list->CmdBuffer[cmd_i];

			if (pcmd.UserCallback)
			{
				pcmd.UserCallback(cmd_list, &pcmd);
			}
			else
			{
				paint_draw_cmd(vertices, idx_buffer, pcmd);
			}

			idx_buffer += pcmd.ElemCount;
		}
	}

	void clear_frame_buffer()
	{
		std::uninitialized_fill_n(frame_buffer_.data(), frame_buffer_.size(), Color{});
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
