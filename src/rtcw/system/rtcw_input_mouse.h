/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#ifndef RTCW_INPUT_MOUSE_INCLUDED
#define RTCW_INPUT_MOUSE_INCLUDED


#include "SDL_events.h"

#include "q_shared.h"
#include "rtcw_bit_array_single_unit.h"


extern cvar_t* in_mouse;


namespace rtcw
{
namespace input
{


class Mouse
{
public:
	Mouse();

	~Mouse();


	bool initialize();

	void uninitialize(
		bool quiet = false);

	void handle_event(
		const SDL_Event& e);

	void activate(
		bool value);

	void update();

	void reset_state();

	static void register_cvars();


private:
	static const int MAX_BUTTONS_COUNT = 5;


	typedef BitArraySingleUnit<MAX_BUTTONS_COUNT> ButtonsStates;


	bool is_initialized_;
	bool is_activated_;
	ButtonsStates buttons_states_;


	void handle_button_event(
		const SDL_MouseButtonEvent& e);

	void handle_motion_event(
		const SDL_MouseMotionEvent& e);

	void handle_wheel_event(
		const SDL_MouseWheelEvent& e);

	static int get_game_button_by_sys_button(
		int sys_button);

	static int get_state_index_by_sys_button(
		int sys_button);

	static int get_game_button_by_state_index(
		int state_index);
}; // Mouse


} // input
} // rtcw


#endif // !RTCW_INPUT_MOUSE_INCLUDED
