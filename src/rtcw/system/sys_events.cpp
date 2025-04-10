/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "sys_events.h"

#include "SDL_events.h"

#ifndef DEDICATED
#include "client.h"
#include "sys_input.h"
#endif // DEDICATED


extern Uint32 sys_main_window_id;


void sys_update_console();

void sys_handle_console_sdl_event(
	const SDL_Event& e);


#ifndef DEDICATED
void SNDDMA_Activate(bool value);


namespace {


void sys_handle_window_events(const SDL_WindowEvent& e)
{
	if (e.windowID != sys_main_window_id)
	{
		return;
	}

	switch (e.event) {
	case SDL_WINDOWEVENT_FOCUS_GAINED:
		SNDDMA_Activate(true);
		break;

	case SDL_WINDOWEVENT_FOCUS_LOST:
		SNDDMA_Activate(false);
		break;
	}
}


} // namespace
#endif // DEDICATED


void sys_poll_events()
{
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		sys_handle_console_sdl_event(e);

#ifndef DEDICATED
		switch (e.type) {
		case SDL_JOYAXISMOTION:
		case SDL_JOYBALLMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEMOTION:
		case SDL_MOUSEWHEEL:
		case SDL_TEXTEDITING:
		case SDL_TEXTINPUT:
			sys_input_handle_event(e);
			break;

		case SDL_WINDOWEVENT:
			sys_handle_window_events(e.window);
			break;

		case SDL_QUIT:
			Com_Quit_f();
			break;
		}
#endif // DEDICATED
	}

	sys_update_console();
}
