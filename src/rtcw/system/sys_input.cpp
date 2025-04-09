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


#include "client.h"

#include "rtcw_input_joystick.h"
#include "rtcw_input_keyboard.h"
#include "rtcw_input_mouse.h"


namespace {


rtcw::input::Joystick joystick;
rtcw::input::Keyboard keyboard;
rtcw::input::Mouse mouse;


} // namespace


cvar_t* in_midi = NULL;
cvar_t* in_midiport = NULL;
cvar_t* in_midichannel = NULL;
cvar_t* in_mididevice = NULL;


extern glconfig_t glConfig;
extern SDL_Window* sys_gl_window;


/*
============================================================

MOUSE CONTROL

============================================================
*/

void IN_MouseEvent(int buttons)
{
}


/*
=========================================================================

=========================================================================
*/

/*
===========
IN_Startup
===========
*/
void IN_Startup()
{
	joystick.initialize();
	keyboard.initialize();
	mouse.initialize();
}

void IN_Shutdown()
{
	Com_Printf("%s input subsystems...\n", "Uninitializing");

	joystick.uninitialize();
	keyboard.uninitialize();
	mouse.uninitialize();

	Cmd_RemoveCommand("midiinfo");
}

static void cmd_midi_info()
{
	Com_Printf("MIDI control not supported.\n");
}

void IN_Init()
{
	Com_Printf("%s input subsystems...\n", "Initializing");

	// MIDI input controler variables
	in_midi = Cvar_Get("in_midi", "0", CVAR_ARCHIVE);
	in_midiport = Cvar_Get("in_midiport", "1", CVAR_ARCHIVE );
	in_midichannel = Cvar_Get("in_midichannel", "1", CVAR_ARCHIVE );
	in_mididevice = Cvar_Get("in_mididevice", "0", CVAR_ARCHIVE );

	Cmd_AddCommand("midiinfo", cmd_midi_info);

	rtcw::input::Joystick::register_cvars();
	rtcw::input::Keyboard::register_cvars();
	rtcw::input::Mouse::register_cvars();

	IN_Startup();
}

// Called every frame, even if not generating commands
void IN_Frame()
{
	mouse.update();
}

void IN_ClearStates()
{
	joystick.reset_state();
	mouse.reset_state();
}

void sys_input_handle_event(const SDL_Event& e)
{
	switch (e.type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	case SDL_TEXTEDITING:
	case SDL_TEXTINPUT:
		keyboard.handle_event(e);
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
	case SDL_MOUSEWHEEL:
		mouse.handle_event(e);
		break;

	case SDL_JOYAXISMOTION:
	case SDL_JOYBALLMOTION:
	case SDL_JOYHATMOTION:
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
	case SDL_JOYDEVICEADDED:
	case SDL_JOYDEVICEREMOVED:
		joystick.handle_event(e);
		break;

	default:
		assert(!"Expected input event.");
		break;
	}
}
