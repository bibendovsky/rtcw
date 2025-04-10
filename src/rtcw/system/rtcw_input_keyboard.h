/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#ifndef RTCW_INPUT_KEYBOARD_INCLUDED
#define RTCW_INPUT_KEYBOARD_INCLUDED


#include "SDL_events.h"

#include "q_shared.h"


// Deprecated SP stuff.
extern cvar_t* k_language;


namespace rtcw
{
namespace input
{


class Keyboard
{
public:
	Keyboard();

	~Keyboard();

	bool initialize();

	void uninitialize(
		bool quiet = false);

	void handle_event(
		const SDL_Event& e);

	static void register_cvars();


private:
	bool is_initialized_;


	void handle_key_event(
		const SDL_KeyboardEvent& e);

	static char map_to_char(
		const SDL_KeyboardEvent& e);

	static int map_to_rtcw(
		SDL_Keycode key_code);
}; // Keyboard


} // input
} // rtcw


#endif // !RTCW_INPUT_KEYBOARD_INCLUDED
