/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "rtcw_input_mouse.h"

#include "SDL.h"

#include "client.h"
#include "keycodes.h"


extern SDL_Window* sys_gl_window;
extern Uint32 sys_main_window_id;


cvar_t* in_mouse = NULL;


void Sys_QueEvent(
	int time,
	sysEventType_t type,
	int value,
	int value2,
	int ptrLength,
	void* ptr);


namespace rtcw
{
namespace input
{


Mouse::Mouse()
	:
	is_initialized_(),
	is_activated_(),
	buttons_states_()
{}

Mouse::~Mouse()
{
	uninitialize(true);
}

bool Mouse::initialize()
{
	uninitialize(true);

	Com_Printf("Initializing mouse input...\n");

	in_mouse->modified = false;

	if (in_mouse->integer == 0)
	{
		Com_Printf(S_COLOR_YELLOW "  Ignored.\n");
		return false;
	}

	is_initialized_ = true;

	return true;
}

void Mouse::uninitialize(
	bool quiet)
{
	if (!quiet)
	{
		Com_Printf("Uninitializing mouse input...\n");
	}

	activate(false);

	is_initialized_ = false;
	is_activated_ = false;
	buttons_states_.clear();
}

void Mouse::handle_event(
	const SDL_Event& e)
{
	if (!is_initialized_)
	{
		return;
	}

	if (!is_activated_)
	{
		return;
	}

	switch (e.type)
	{
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			handle_button_event(e.button);
			break;

		case SDL_MOUSEMOTION:
			handle_motion_event(e.motion);
			break;

		case SDL_MOUSEWHEEL:
			handle_wheel_event(e.wheel);
			break;

		default:
			assert(!"Unsupported mouse event.");
			return;
	}
}

void Mouse::activate(
	bool value)
{
	if (!is_initialized_)
	{
		return;
	}

	if (is_activated_ == value)
	{
		return;
	}

	if (value)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	else
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);

		const Uint32 window_flags = SDL_GetWindowFlags(sys_gl_window);
		bool is_fullscreen = ((window_flags & SDL_WINDOW_FULLSCREEN) != 0);

		if (is_fullscreen)
		{
			SDL_ShowCursor(SDL_FALSE);
		}
	}

	is_activated_ = value;
}

void Mouse::update()
{
	if (!is_initialized_)
	{
		return;
	}

	bool activate_value = true;

	if (activate_value)
	{
		activate_value = ((cls.keyCatchers & KEYCATCH_CONSOLE) == 0);
	}

	if (activate_value)
	{
		const Uint32 window_flags = SDL_GetWindowFlags(sys_gl_window);
		activate_value = ((window_flags & SDL_WINDOW_INPUT_FOCUS) != 0);
	}

	activate(activate_value);
}

void Mouse::reset_state()
{
	if (buttons_states_.is_clear())
	{
		return;
	}

	const Uint32 timestamp = SDL_GetTicks();

	for (int i = 0; i < MAX_BUTTONS_COUNT; ++i)
	{
		const bool state = buttons_states_.is_set(i);

		if (!state)
		{
			continue;
		}

		const int game_button = get_game_button_by_state_index(i);

		Sys_QueEvent(
			timestamp,
			SE_KEY,
			game_button,
			false,
			0,
			NULL
		);
	}

	buttons_states_.clear();
}

void Mouse::register_cvars()
{
	in_mouse = Cvar_Get("in_mouse", "1", CVAR_ARCHIVE | CVAR_LATCH);
}

void Mouse::handle_button_event(
	const SDL_MouseButtonEvent& e)
{
	if (e.windowID != sys_main_window_id)
	{
		return;
	}

	const Uint8 button = e.button;
	const int state_index = get_state_index_by_sys_button(button);

	if (state_index < 0)
	{
		return;
	}

	const int game_button = get_game_button_by_sys_button(button);

	buttons_states_.toggle(state_index);
	const bool state = buttons_states_.is_set(state_index);

	Sys_QueEvent(
		e.timestamp,
		SE_KEY,
		game_button,
		state,
		0,
		NULL
	);
}

void Mouse::handle_motion_event(
	const SDL_MouseMotionEvent& e)
{
	if (e.windowID != sys_main_window_id)
	{
		return;
	}

	Sys_QueEvent(
		e.timestamp,
		SE_MOUSE,
		e.xrel,
		e.yrel,
		0,
		NULL
	);
}

void Mouse::handle_wheel_event(
	const SDL_MouseWheelEvent& e)
{
	if (e.windowID != sys_main_window_id)
	{
		return;
	}

	if (e.y == 0)
	{
		return;
	}

	const keyNum_t which = (e.y > 0 ? K_MWHEELUP : K_MWHEELDOWN);

	Sys_QueEvent(
		e.timestamp,
		SE_KEY,
		which,
		true,
		0,
		NULL
	);

	Sys_QueEvent(
		e.timestamp,
		SE_KEY,
		which,
		false,
		0,
		NULL
	);
}

int Mouse::get_game_button_by_sys_button(
	int sys_button)
{
	switch (sys_button)
	{
		case SDL_BUTTON_LEFT:
			return K_MOUSE1;

		case SDL_BUTTON_MIDDLE:
			return K_MOUSE3;

		case SDL_BUTTON_RIGHT:
			return K_MOUSE2;

		case SDL_BUTTON_X1:
			return K_MOUSE4;

		case SDL_BUTTON_X2:
			return K_MOUSE5;

		default:
			assert(!"Unsupported SDL mouse button.");
			return 0;
	}
}

int Mouse::get_state_index_by_sys_button(
	int sys_button)
{
	switch (sys_button)
	{
		case SDL_BUTTON_LEFT:
			return 0;

		case SDL_BUTTON_MIDDLE:
			return 1;

		case SDL_BUTTON_RIGHT:
			return 2;

		case SDL_BUTTON_X1:
			return 3;

		case SDL_BUTTON_X2:
			return 4;

		default:
			assert(!"Unsupported SDL mouse button.");
			return -1;
	}
}

int Mouse::get_game_button_by_state_index(
	int state_index)
{
	switch (state_index)
	{
		case 0:
			return K_MOUSE1;

		case 1:
			return K_MOUSE3;

		case 2:
			return K_MOUSE2;

		case 3:
			return K_MOUSE4;

		case 4:
			return K_MOUSE5;

		default:
			assert(!"Invalid state index.");
			return -1;
	}
}


} // input
} // rtcw
