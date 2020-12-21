/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (?RTCW SP Source Code?).

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


#ifndef RTCW_INPUT_MOUSE_INCLUDED
#define RTCW_INPUT_MOUSE_INCLUDED


#include <bitset>

#include "SDL_events.h"

#include "q_shared.h"


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
	static constexpr auto MAX_BUTTONS_COUNT = 5;


	using ButtonsStates = std::bitset<MAX_BUTTONS_COUNT>;


	bool is_initialized_{};
	bool is_activated_{};
	ButtonsStates buttons_states_{};


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
