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


#include "rtcw_input_keyboard.h"

#include "SDL.h"

#include "tr_local.h"
#include "keycodes.h"


cvar_t* k_language = nullptr;
extern Uint32 sys_main_window_id;


void Sys_QueEvent(
	int time,
	sysEventType_t type,
	int value,
	int value2,
	int ptrLength,
	void* ptr);

bool GLimp_SetFullscreen(
	bool value);


namespace rtcw
{
namespace input
{


Keyboard::Keyboard() = default;

Keyboard::~Keyboard()
{
	uninitialize(true);
}

bool Keyboard::initialize()
{
	uninitialize(true);


	Com_Printf("Initializing keyboard input...\n");

	int sdl_result = 0;

	if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
	{
		sdl_result = SDL_InitSubSystem(SDL_INIT_VIDEO);

		if (sdl_result != 0)
		{
			Com_Printf(S_COLOR_RED "  %s\n", SDL_GetError());
			return false;
		}
	}

	is_initialized_ = true;

	return true;
}

void Keyboard::uninitialize(
	bool quiet)
{
	if (!quiet)
	{
		Com_Printf("Uninitializing keyboard input...\n");
	}

	is_initialized_ = false;
}

void Keyboard::handle_event(
	const SDL_Event& e)
{
	if (!is_initialized_)
	{
		return;
	}

	switch (e.type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			handle_key_event(e.key);
			break;

		case SDL_TEXTEDITING:
		case SDL_TEXTINPUT:
			break;

		default:
			assert(!"Unsupported keyboard event.");
			return;
	}
}

void Keyboard::handle_key_event(
	const SDL_KeyboardEvent& e)
{
	if (e.windowID != sys_main_window_id)
	{
		return;
	}

	const SDL_Keycode key_code = e.keysym.sym;
	const int key = map_to_rtcw(key_code);

	if (key == 0)
	{
		return;
	}

	const Uint16 key_mod = e.keysym.mod;

	bool is_alt_pressed = ((key_mod & KMOD_ALT) != 0);

	bool is_enter_pressed = (e.state == SDL_PRESSED && key_code == SDLK_RETURN);

	if (is_alt_pressed && is_enter_pressed)
	{
		if (r_fullscreen != nullptr)
		{
			Cvar_SetValue("r_fullscreen", !r_fullscreen->integer);
			const bool is_fullscreen = (r_fullscreen->integer != 0);

			if (GLimp_SetFullscreen(is_fullscreen))
			{
				glConfig.isFullscreen = is_fullscreen;
			}
			else
			{
				Cbuf_AddText("vid_restart\n");
			}

			return;
		}
	}

	Sys_QueEvent(
		e.timestamp,
		SE_KEY,
		key,
		e.state == SDL_PRESSED,
		0,
		nullptr
	);

	if (e.state == SDL_RELEASED)
	{
		return;
	}

	const char key_char = map_to_char(e);

	if (key_char != 0)
	{
		Sys_QueEvent(
			e.timestamp,
			SE_CHAR,
			key_char,
			0,
			0,
			nullptr
		);
	}
}

void Keyboard::register_cvars()
{
	k_language = Cvar_Get("k_language", "american", CVAR_ARCHIVE | CVAR_NORESTART);
}

char Keyboard::map_to_char(
	const SDL_KeyboardEvent& e)
{
	const Uint16 flags = e.keysym.mod;

	if ((flags & (
		KMOD_LCTRL |
		KMOD_RCTRL |
		KMOD_LALT |
		KMOD_RALT |
		KMOD_LGUI |
		KMOD_RGUI |
		KMOD_MODE)) != 0)
	{
		return 0;
	}

	bool is_caps = false;

	if ((flags & KMOD_CAPS) != 0)
	{
		is_caps = !is_caps;
	}

	if ((flags & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0)
	{
		is_caps = !is_caps;
	}

	const SDL_Keycode key_code = e.keysym.sym;

	switch (key_code)
	{
		case SDLK_BACKSPACE:
		case SDLK_SPACE:
			return key_code;
	}

	if (!is_caps)
	{
		switch (key_code)
		{
			case SDLK_1:
			case SDLK_2:
			case SDLK_3:
			case SDLK_4:
			case SDLK_5:
			case SDLK_6:
			case SDLK_7:
			case SDLK_8:
			case SDLK_9:
			case SDLK_0:
			case SDLK_MINUS:
			case SDLK_EQUALS:
			case SDLK_LEFTBRACKET:
			case SDLK_RIGHTBRACKET:
			case SDLK_SEMICOLON:
			case SDLK_QUOTE:
			case SDLK_BACKQUOTE:
			case SDLK_BACKSLASH:
			case SDLK_COMMA:
			case SDLK_PERIOD:
			case SDLK_SLASH:
			case SDLK_a:
			case SDLK_b:
			case SDLK_c:
			case SDLK_d:
			case SDLK_e:
			case SDLK_f:
			case SDLK_g:
			case SDLK_h:
			case SDLK_i:
			case SDLK_j:
			case SDLK_k:
			case SDLK_l:
			case SDLK_m:
			case SDLK_n:
			case SDLK_o:
			case SDLK_p:
			case SDLK_q:
			case SDLK_r:
			case SDLK_s:
			case SDLK_t:
			case SDLK_u:
			case SDLK_v:
			case SDLK_w:
			case SDLK_x:
			case SDLK_y:
			case SDLK_z:
				return key_code;
		}
	}
	else
	{
		switch (key_code)
		{
			case SDLK_1:
				return '!';

			case SDLK_2:
				return '@';

			case SDLK_3:
				return '#';

			case SDLK_4:
				return '$';

			case SDLK_5:
				return '%';

			case SDLK_6:
				return '^';

			case SDLK_7:
				return '&';

			case SDLK_8:
				return '*';

			case SDLK_9:
				return '(';

			case SDLK_0:
				return ')';

			case SDLK_MINUS:
				return '_';

			case SDLK_EQUALS:
				return '+';

			case SDLK_LEFTBRACKET:
				return '{';

			case SDLK_RIGHTBRACKET:
				return '}';

			case SDLK_SEMICOLON:
				return ':';

			case SDLK_QUOTE:
				return '"';

			case SDLK_BACKQUOTE:
				return '~';

			case SDLK_BACKSLASH:
				return '|';

			case SDLK_COMMA:
				return '<';

			case SDLK_PERIOD:
				return '>';

			case SDLK_SLASH:
				return '?';

			case SDLK_a:
			case SDLK_b:
			case SDLK_c:
			case SDLK_d:
			case SDLK_e:
			case SDLK_f:
			case SDLK_g:
			case SDLK_h:
			case SDLK_i:
			case SDLK_j:
			case SDLK_k:
			case SDLK_l:
			case SDLK_m:
			case SDLK_n:
			case SDLK_o:
			case SDLK_p:
			case SDLK_q:
			case SDLK_r:
			case SDLK_s:
			case SDLK_t:
			case SDLK_u:
			case SDLK_v:
			case SDLK_w:
			case SDLK_x:
			case SDLK_y:
			case SDLK_z:
				return SDL_toupper(key_code);
		}
	}

	return 0;
}

int Keyboard::map_to_rtcw(
	SDL_Keycode key_code)
{
	switch (key_code)
	{
		case SDLK_TAB:
			return K_TAB;

		case SDLK_RETURN:
			return K_ENTER;

		case SDLK_ESCAPE:
			return K_ESCAPE;

		case SDLK_SPACE:
			return K_SPACE;

		case SDLK_BACKSPACE:
		case SDLK_KP_BACKSPACE:
			return K_BACKSPACE;

		//TODO: K_COMMAND

		case SDLK_CAPSLOCK:
			return K_CAPSLOCK;

		case SDLK_POWER:
		case SDLK_KP_POWER:
			return K_POWER;

		case SDLK_PAUSE:
			return K_PAUSE;

		case SDLK_UP:
			return K_UPARROW;

		case SDLK_DOWN:
			return K_DOWNARROW;

		case SDLK_LEFT:
			return K_LEFTARROW;

		case SDLK_RIGHT:
			return K_RIGHTARROW;

		case SDLK_LALT:
		case SDLK_RALT:
			return K_ALT;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
			return K_CTRL;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			return K_SHIFT;

		case SDLK_INSERT:
			return K_INS;

		case SDLK_PAGEDOWN:
			return K_PGDN;

		case SDLK_PAGEUP:
			return K_PGUP;

		case SDLK_HOME:
			return K_HOME;

		case SDLK_END:
			return K_END;

		case SDLK_F1:
			return K_F1;

		case SDLK_F2:
			return K_F2;

		case SDLK_F3:
			return K_F3;

		case SDLK_F4:
			return K_F4;

		case SDLK_F5:
			return K_F5;

		case SDLK_F6:
			return K_F6;

		case SDLK_F7:
			return K_F7;

		case SDLK_F8:
			return K_F8;

		case SDLK_F9:
			return K_F9;

		case SDLK_F10:
			return K_F10;

		case SDLK_F11:
			return K_F11;

		case SDLK_F12:
			return K_F12;

		case SDLK_F13:
			return K_F13;

		case SDLK_F14:
			return K_F14;

		case SDLK_F15:
			return K_F15;

		case SDLK_KP_7:
			return K_KP_HOME;

		case SDLK_KP_8:
			return K_KP_UPARROW;

		case SDLK_KP_9:
			return K_KP_PGUP;

		case SDLK_KP_4:
			return K_KP_LEFTARROW;

		case SDLK_KP_5:
			return K_KP_5;

		case SDLK_KP_6:
			return K_KP_RIGHTARROW;

		case SDLK_KP_1:
			return K_KP_END;

		case SDLK_KP_2:
			return K_KP_DOWNARROW;

		case SDLK_KP_3:
			return K_KP_PGDN;

		case SDLK_KP_ENTER:
			return K_KP_ENTER;

		case SDLK_KP_0:
			return K_KP_INS;

		case SDLK_KP_PERIOD:
			return K_KP_DEL;

		case SDLK_KP_DIVIDE:
			return K_KP_SLASH;

		case SDLK_KP_MINUS:
			return K_KP_MINUS;

		case SDLK_KP_PLUS:
			return K_KP_PLUS;

		case SDLK_NUMLOCKCLEAR:
			return K_KP_NUMLOCK;

		case SDLK_KP_MULTIPLY:
			return K_KP_STAR;

		case SDLK_KP_EQUALS:
			return K_KP_EQUALS;

		case SDLK_1:
		case SDLK_2:
		case SDLK_3:
		case SDLK_4:
		case SDLK_5:
		case SDLK_6:
		case SDLK_7:
		case SDLK_8:
		case SDLK_9:
		case SDLK_0:
		case SDLK_MINUS:
		case SDLK_EQUALS:
		case SDLK_q:
		case SDLK_w:
		case SDLK_e:
		case SDLK_r:
		case SDLK_t:
		case SDLK_y:
		case SDLK_u:
		case SDLK_i:
		case SDLK_o:
		case SDLK_p:
		case SDLK_LEFTBRACKET:
		case SDLK_RIGHTBRACKET:
		case SDLK_a:
		case SDLK_s:
		case SDLK_d:
		case SDLK_f:
		case SDLK_g:
		case SDLK_h:
		case SDLK_j:
		case SDLK_k:
		case SDLK_l:
		case SDLK_SEMICOLON:
		case SDLK_QUOTE:
		case SDLK_BACKQUOTE:
		case SDLK_BACKSLASH:
		case SDLK_z:
		case SDLK_x:
		case SDLK_c:
		case SDLK_v:
		case SDLK_b:
		case SDLK_n:
		case SDLK_m:
		case SDLK_COMMA:
		case SDLK_PERIOD:
		case SDLK_SLASH:
			return key_code;

		default:
			return 0;
	}
}


} // input
} // rtcw
