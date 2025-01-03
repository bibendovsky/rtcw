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
// Very simple software renderer powered by SDL2 for Dear ImGui.
//
// Notes:
//    - Supports only SDL_Surface as texture id.
//    - Supports only ARGB8888 pixel format (see "create_texture_id").
//    - May produce visual artefacts for rounded/antialiased corners.
//


#ifndef IMGUI_SDL_INCLUDED
#define IMGUI_SDL_INCLUDED


#include <memory>
#include <string>
#include "SDL_events.h"


struct ImDrawData;


class ImGuiSdl
{
public:
	enum class PixelFormat
	{
		none,
		standard,
		imgui,
	}; // PixelFormat

	enum class WindowStatus
	{
		none,
		hidden,
		shown,
		minimized,
	}; // WindowStatus


	ImGuiSdl();

	ImGuiSdl(
		const ImGuiSdl& that) = delete;

	ImGuiSdl& operator=(
		const ImGuiSdl& that) = delete;

	ImGuiSdl(
		ImGuiSdl&& that);

	~ImGuiSdl();


	//
	// Gets a last error message.
	//
	// Returns:
	//    - An error message.
	//
	const std::string& get_error_message() const;

	//
	// Initializes the instance.
	//
	// Parameters:
	//    - sdl_window - SDL window to draw on.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	bool initialize(
		SDL_Window* sdl_window);

	//
	// Uninitializes the instance.
	//
	void uninitialize();


	//
	// Prepares a new frame.
	//
	// Notes:
	//    - Implicitly calls ImGui::NewFrame.
	//
	void new_frame();

	//
	// Handles SDL event.
	//
	// Parameters:
	//    - sdl_event - SDL event.
	//
	void handle_event(
		const SDL_Event& sdl_event);

	//
	// Handles ImGui's draw commands.
	//
	// Prameters:
	//    - draw_data - ImGui's draw data.
	//
	void draw(
		ImDrawData* draw_data);

	//
	// Renders drawn data.
	//
	// Notes:
	//    - Implicitly calls SDL_RenderPresent.
	//
	void present();

	//
	// Gets the window status.
	//
	// Returns:
	//    - A window status.
	//
	WindowStatus get_window_status() const;

	//
	// Checks if the user tried to close the SDL window.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	bool is_close_requested() const;

	//
	// Resets the flag to check if the user tried to close the SDL window.
	//
	void reset_is_close_requested();

	//
	// Creates a compatible texture id.
	//
	// Parameters:
	//    - width - a width of the texture.
	//    - height - a height of the texture.
	//    - src_pixel_format - a source pixel format.
	//    - raw_pixels - a source pixels (optional).
	//
	// Returns:
	//    - SDL surface as texture id.
	//
	SDL_Surface* create_texture_id(
		const int width,
		const int height,
		const PixelFormat src_pixel_format,
		const void* raw_pixels);


private:
	class Impl;

	using ImplUPtr = std::unique_ptr<Impl>;

	ImplUPtr pimpl_;
}; // ImGuiSdl


#endif // !IMGUI_SDL_INCLUDED
