/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
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
