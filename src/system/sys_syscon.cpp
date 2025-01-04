/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/


#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#endif // !IMGUI_DISABLE_OBSOLETE_FUNCTIONS


#include <deque>
#include <string>
#include <vector>
#include "SDL.h"
#include "imgui.h"
#include "client.h"
#include "imgui_sdl.h"

#if defined RTCW_SP
#include "rtcw_sp_resource.h"
#elif defined RTCW_MP
#include "rtcw_mp_resource.h"
#elif defined RTCW_ET
#include "rtcw_et_resource.h"
#endif // RTCW_XX


namespace
{


const int screen_width = 540;
const int screen_height = 450;

const int max_log_lines = 4096;
const int line_min_reserved_size = 1024;

const int max_edit_buffer_size = 128;

const char* window_title =
#if defined(RTCW_SP)
	"RTCW Single-player Console"
#elif defined(RTCW_MP)
#ifdef UPDATE_SERVER
	"RTCW Multiplayer Update Server Console"
#else
	"RTCW Multiplayer Console"
#endif // UPDATE_SERVER
#elif defined(RTCW_ET)
	"RTCW Enemy Territory Console"
#else
	"RTCW Console"
#endif // RTCW_SP
;


using Log = std::deque<std::string>;
using EditBuffer = std::vector<char>;


std::string error_message_;

Uint32 sdl_window_id_ = 0;
SDL_Window* sdl_window_ = NULL;
SDL_Renderer* sdl_renderer_ = NULL;
SDL_Surface* sdl_font_surface_ = NULL;

Log log_;
std::string error_text_;
std::string returned_commands_;
std::string entered_commands_;
std::string append_buffer_;
ImGuiSdl::WindowStatus window_status_ = ImGuiSdl::WindowStatus::none;
ImGuiSdl imgui_sdl_;
ImGuiContext* imgui_context_ = NULL;

EditBuffer edit_buffer_;

bool is_initialized_ = false;
bool quit_on_close_ = false;

bool is_log_scrolled_ = false;
bool is_edit_activated_ = false;

bool is_enter_pressed_ = false;
bool is_tab_entered_ = false;
bool is_quit_ = false;


int edit_callback(
	ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag != ImGuiInputTextFlags_CallbackCharFilter)
	{
		return 0;
	}

	// Allow only ASCII characters.
	if (data->EventChar > 0x7F)
	{
		return 1;
	}

	if (data->EventChar == '\t')
	{
		is_tab_entered_ = true;
		return 1;
	}

	return 0;
}

void add_line_to_log(
	const char* const line,
	const int length)
{
	if (!line)
	{
		return;
	}

	while (log_.size() >= max_log_lines)
	{
		log_.pop_front();
	}

	if (length < 0)
	{
		log_.emplace_back(line);
	}
	else
	{
		log_.emplace_back(line, length);
	}
}

void add_line_to_log(
	const char* const line)
{
	if (!line || *line == '\0')
	{
		return;
	}

	add_line_to_log(line, static_cast<int>(std::string::traits_type::length(line)));
}

void add_line_to_log(
	const std::string& line)
{
	if (line.empty())
	{
		return;
	}

	add_line_to_log(line.data(), static_cast<int>(line.size()));
}

void append_text_to_log(
	const char* const text)
{
	if (!text || *text == '\0')
	{
		return;
	}

	int i = 0;
	append_buffer_.clear();

	while (true)
	{
		const char ch = text[i];

		if (ch == '\0')
		{
			if (!append_buffer_.empty())
			{
				add_line_to_log(append_buffer_);
			}

			break;
		}
		else
		{
			const char next_ch = text[i + 1];

			if (ch == '\r' && next_ch == '\n')
			{
				add_line_to_log(append_buffer_);
				append_buffer_.clear();

				i += 2;
			}
			else if (ch == '\r')
			{
				add_line_to_log(append_buffer_);
				append_buffer_.clear();

				i += 1;
			}
			else if (ch == '\n')
			{
				add_line_to_log(append_buffer_);
				append_buffer_.clear();

				i += 1;
			}
			else if (ch == Q_COLOR_ESCAPE)
			{
				if (next_ch == Q_COLOR_ESCAPE)
				{
					append_buffer_ += Q_COLOR_ESCAPE;
				}

				i += 2;
			}
			else
			{
				append_buffer_ += ch;

				i += 1;
			}
		}
	}

	is_log_scrolled_ = false;
}

void uninitialize()
{
	sdl_window_id_ = {};

	imgui_sdl_.uninitialize();


	if (imgui_context_)
	{
		ImGui::DestroyContext(imgui_context_);
		imgui_context_ = {};
	}

	if (sdl_font_surface_)
	{
		SDL_FreeSurface(sdl_font_surface_);
		sdl_font_surface_ = {};
	}

	if (sdl_renderer_)
	{
		SDL_DestroyRenderer(sdl_renderer_);
		sdl_renderer_ = {};
	}

	if (sdl_window_)
	{
		SDL_DestroyWindow(sdl_window_);
		sdl_window_ = {};
	}

	log_.clear();
	window_status_ = {};
	returned_commands_.clear();
	entered_commands_.clear();
	append_buffer_.clear();

	quit_on_close_ = {};
	is_initialized_ = {};

	edit_buffer_.clear();
}

bool initialize_font_atlas()
{
	ImGuiIO& imgui_io = ImGui::GetIO();

	unsigned char* out_pixels = NULL;
	int out_width = 0;
	int out_height = 0;
	int out_bytes_per_pixel = 0;

	imgui_io.Fonts->GetTexDataAsRGBA32(&out_pixels, &out_width, &out_height, &out_bytes_per_pixel);

	if (!out_pixels || out_width <= 0 || out_height <= 0 || out_bytes_per_pixel != 4)
	{
		error_message_ = "DImGui: Failed to get font's texture data.";
		return false;
	}

	sdl_font_surface_ = imgui_sdl_.create_texture_id(out_width, out_height, ImGuiSdl::PixelFormat::imgui, out_pixels);

	if (!sdl_font_surface_)
	{
		error_message_ = "DImGuiSdl: " + imgui_sdl_.get_error_message();
		return false;
	}

	imgui_io.Fonts->TexID = sdl_font_surface_;

	return true;
}

void setup_imgui()
{
	ImGuiIO& imgui_io = ImGui::GetIO();

	imgui_io.IniFilename = NULL;
	imgui_io.DisplaySize.x = static_cast<float>(screen_width);
	imgui_io.DisplaySize.y = static_cast<float>(screen_height);

	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0F;
	style.AntiAliasedFill = false;
	style.AntiAliasedLines = false;
	style.ChildRounding = 0;
	style.FrameBorderSize = 0;
	style.FramePadding = {};
	style.FrameRounding = 0;
	style.GrabRounding = 0;
	style.ItemInnerSpacing = {};
	style.ItemSpacing = {};
	style.PopupRounding = 0;
	style.ScrollbarRounding = 0;
	style.TouchExtraPadding = {};
	style.WindowBorderSize = 0;
	style.WindowPadding = {};
	style.WindowRounding = 0;
}

bool initialize_internal()
{
	error_message_.clear();

	int sdl_result = 0;

	if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
	{
		sdl_result = SDL_InitSubSystem(SDL_INIT_VIDEO);

		if (sdl_result != 0)
		{
			error_message_ = "SDL: Failed to initialize video subsystem.";
			return false;
		}
	}

	sdl_result = SDL_CreateWindowAndRenderer(
		screen_width,
		screen_height,
		SDL_WINDOW_HIDDEN,
		&sdl_window_,
		&sdl_renderer_);

	if (sdl_result != 0)
	{
		error_message_ = "SDL: Failed to create a window with a renderer.";
		return false;
	}

	SDL_SetWindowTitle(sdl_window_, window_title);

	imgui_context_ = ImGui::CreateContext();

	if (!imgui_context_)
	{
		error_message_ = "DImGui: Failed to create a context.";
		return false;
	}

	if (!imgui_sdl_.initialize(sdl_window_))
	{
		error_message_ = "DImGuiSdl: " + imgui_sdl_.get_error_message();
		return false;
	}

	if (!initialize_font_atlas())
	{
		return false;
	}


	window_status_ = ImGuiSdl::WindowStatus::hidden;
	is_initialized_ = true;
	append_buffer_.reserve(line_min_reserved_size);
	edit_buffer_.resize(max_edit_buffer_size);

	setup_imgui();

	return true;
}

void initialize()
{
	uninitialize();

	if (!initialize_internal())
	{
		uninitialize();

		static_cast<void>(SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"RTCW",
			error_message_.c_str(),
			NULL
		));
	}
}

ImGuiSdl::WindowStatus get_window_status_by_visibility_level(
	const int visibility_level)
{
	switch (visibility_level)
	{
	case 0:
		return ImGuiSdl::WindowStatus::hidden;

	case 1:
		return ImGuiSdl::WindowStatus::shown;

	case 2:
		return ImGuiSdl::WindowStatus::minimized;

	default:
		return ImGuiSdl::WindowStatus::none;
	}
}

void show(
	const int visibility_level,
	const bool quit_on_close)
{
	quit_on_close_ = quit_on_close;

	const ImGuiSdl::WindowStatus new_window_status = get_window_status_by_visibility_level(visibility_level);

	if (new_window_status == window_status_)
	{
		return;
	}

	window_status_ = new_window_status;

	switch (window_status_)
	{
	case ImGuiSdl::WindowStatus::hidden:
	case ImGuiSdl::WindowStatus::none:
		SDL_HideWindow(sdl_window_);
		break;

	case ImGuiSdl::WindowStatus::shown:
		SDL_ShowWindow(sdl_window_);
		break;

	case ImGuiSdl::WindowStatus::minimized:
		SDL_MinimizeWindow(sdl_window_);
		break;

	default:
		break;
	}
}

void set_error_text(
	const char* const error_text)
{
	if (!error_text || *error_text == '\0')
	{
		error_text_ = "Generic failure.";
	}
	else
	{
		error_text_ = error_text;
	}
}

char* get_commands()
{
	if (entered_commands_.empty())
	{
		return {};
	}

	returned_commands_ = entered_commands_;

	entered_commands_.clear();

	return &returned_commands_[0];
}

void clear_log()
{
	log_.clear();
}

void imgui_draw()
{
	imgui_sdl_.new_frame();


	const bool is_show_error = (!error_text_.empty());

	ImVec2 error_control_position = {5.0F, 5.0F};
	ImVec2 error_control_size = {530.0F, 30.0F};

	ImVec2 log_control_position = {5.0F, 40.0F};
	ImVec2 log_control_size = {530.0F, 355.0F};

	ImVec2 command_control_position = {5.0F, 400.0F};
	ImVec2 command_control_size = {530.0F, 20.0F};

	ImVec2 copy_button_control_position = {5.0F, 425.0F};
	ImVec2 clear_button_control_position = {85.0F, 425.0F};
	ImVec2 quit_button_control_position = {465.0F, 425.0F};

	ImVec2 button_control_size = {70.0F, 20.0F};

	if (is_show_error)
	{
		log_control_size.y = 380.0F;
	}
	else
	{
		log_control_position.y = 5.0F;
		log_control_size.y = 390.0F;
	}

	// Main window (begin)
	//
	const int window_flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings |
		0;

	ImGui::Begin("main", NULL, window_flags);
	ImGui::SetWindowSize({static_cast<float>(screen_width), static_cast<float>(screen_height)}, ImGuiCond_Always);
	ImGui::SetWindowPos({}, ImGuiCond_Always);


	// Error message
	//
	if (is_show_error)
	{
		const int error_flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings |
			0;

		ImGui::SetCursorPos(error_control_position);
		ImGui::PushStyleColor(ImGuiCol_Text, {1.0F, 0.0F, 0.0F, 1.0F});
		ImGui::BeginChildFrame(1, error_control_size, error_flags);
		ImGui::TextWrapped("%s", error_text_.c_str());
		ImGui::EndChildFrame();
		ImGui::PopStyleColor();
	}

	// Log
	//
	const int log_flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings |
			0;

	ImGui::SetCursorPos(log_control_position);

	ImGui::BeginChildFrame(20, log_control_size, log_flags);

	if (!log_.empty())
	{
		ImGuiListClipper log_im_clipper(static_cast<int>(log_.size()));

		while (log_im_clipper.Step())
		{
			for (int i = log_im_clipper.DisplayStart; i < log_im_clipper.DisplayEnd; ++i)
			{
				const std::string& line = log_[i];
				ImGui::TextUnformatted(line.data(), line.data() + line.length());
			}
		}

		if (!is_log_scrolled_)
		{
			ImGui::SetScrollHereY(0.5F);

			const float y = ImGui::GetScrollY();
			const float max_y = ImGui::GetScrollMaxY();

			if (max_y > 0.0F && y >= max_y)
			{
				is_log_scrolled_ = true;
			}
		}
	}

	ImGui::EndChildFrame();


	// Input text
	//
	bool is_command_enter_pressed = false;

	if (!is_show_error)
	{
		const int edit_flags =
			ImGuiInputTextFlags_CallbackCharFilter |
			ImGuiInputTextFlags_EnterReturnsTrue |
			0;

		ImGui::SetCursorPos(command_control_position);
		ImGui::PushItemWidth(command_control_size.x);

		is_command_enter_pressed = ImGui::InputText(
			"##cmd",
			edit_buffer_.data(),
			max_edit_buffer_size,
			edit_flags,
			edit_callback);

		ImGui::PopItemWidth();

		if (!is_edit_activated_)
		{
			ImGui::SetKeyboardFocusHere();

			if (ImGui::IsItemActive())
			{
				is_edit_activated_ = true;
			}
		}
	}


	// Button "copy"
	//
	ImGui::SetCursorPos(copy_button_control_position);
	const bool is_copy_clicked = ImGui::Button("Copy", button_control_size);


	// Button "clear"
	//
	bool is_clear_clicked = false;

	if (!is_show_error)
	{
		ImGui::SetCursorPos(clear_button_control_position);
		is_clear_clicked = ImGui::Button("Clear", button_control_size);
	}


	// Button "quit"
	//
	ImGui::SetCursorPos(quit_button_control_position);
	const bool is_quit_clicked = ImGui::Button("Quit", button_control_size);

	// Main window (end)
	// 
	ImGui::End();

	if (is_tab_entered_)
	{
		is_tab_entered_ = false;

		// TODO Command completion.
	}

	if (is_command_enter_pressed)
	{
		is_enter_pressed_ = true;
	}

	if (is_copy_clicked)
	{
		std::string clipboard_buffer;

		bool is_first_line = true;
		const size_t line_count = log_.size();

		for (size_t i_line = 0; i_line < line_count; ++i_line)
		{
			const std::string& line = log_[i_line];

			if (is_first_line)
			{
				is_first_line = false;
			}
			else
			{
				clipboard_buffer += '\n';
			}

			clipboard_buffer += line;
		}

		static_cast<void>(SDL_SetClipboardText(clipboard_buffer.c_str()));
	}

	if (is_clear_clicked)
	{
		log_.clear();
	}

	if (is_quit_clicked)
	{
		is_quit_ = true;
	}

	ImGui::Render();
	ImDrawData* imgui_draw_data = ImGui::GetDrawData();

	imgui_sdl_.draw(imgui_draw_data);
	imgui_sdl_.present();
}

void update(
	bool& is_close_requested)
{
	imgui_draw();

	if (imgui_sdl_.is_close_requested())
	{
		window_status_ = ImGuiSdl::WindowStatus::hidden;
		SDL_HideWindow(sdl_window_);
		imgui_sdl_.reset_is_close_requested();

		is_close_requested = true;
	}

	if (is_quit_)
	{
		if (!quit_on_close_)
		{
			Com_Quit_f();
		}

		is_close_requested = true;
	}

	if (is_enter_pressed_)
	{
		is_enter_pressed_ = false;
		is_log_scrolled_ = false;
		is_edit_activated_ = false;

		entered_commands_ += edit_buffer_.data();
		entered_commands_ += '\n';

		Sys_Print(va("]%s\n", edit_buffer_.data()));
		edit_buffer_[0] = '\0';
	}
}

void update_window_status(
	const ImGuiSdl::WindowStatus window_status)
{
	if (window_status == window_status_)
	{
		return;
	}

	window_status_ = window_status;

	switch (window_status_)
	{
	case ImGuiSdl::WindowStatus::hidden:
		SDL_HideWindow(sdl_window_);
		break;

	case ImGuiSdl::WindowStatus::minimized:
		SDL_MinimizeWindow(sdl_window_);
		break;

	case ImGuiSdl::WindowStatus::shown:
		is_log_scrolled_ = false;
		SDL_ShowWindow(sdl_window_);
		break;

	default:
		break;
	}
}


} // namespace


void Sys_CreateConsole()
{
	initialize();
}

void Sys_DestroyConsole()
{
	uninitialize();
}

void Sys_ShowConsole(
	const int visibility_level,
	const qboolean quit_on_close)
{
	show(visibility_level, quit_on_close);
}

char* Sys_ConsoleInput()
{
	return get_commands();
}

void Conbuf_AppendText(
	const char* text)
{
	append_text_to_log(text);
}

void Sys_SetErrorText(
	const char* const text)
{
	set_error_text(text);
}

void Sys_ClearViewlog_f()
{
	log_.clear();
}

void sys_run_console()
{
	SDL_Event e = {};

	bool is_quit = false;

	while (!is_quit)
	{
		while (SDL_PollEvent(&e))
		{
			imgui_sdl_.handle_event(e);
		}

		const ImGuiSdl::WindowStatus window_status = imgui_sdl_.get_window_status();

		update_window_status(window_status);

		if (window_status == ImGuiSdl::WindowStatus::shown)
		{
			bool is_close_requested = false;

			update(is_close_requested);

			if (is_close_requested)
			{
				is_quit = true;
			}
		}

		SDL_Delay(10);
	}
}

void sys_handle_console_sdl_event(
	const SDL_Event& e)
{
	if (!is_initialized_)
	{
		return;
	}

	imgui_sdl_.handle_event(e);
}

void sys_update_console()
{
	if (!is_initialized_)
	{
		return;
	}

	const ImGuiSdl::WindowStatus window_status = imgui_sdl_.get_window_status();

	update_window_status(window_status);

	if (window_status != ImGuiSdl::WindowStatus::shown)
	{
		return;
	}

	bool is_close_requested = false;

	update(is_close_requested);
}
