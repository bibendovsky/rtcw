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


#include "sys_events.h"

#include "SDL.h"

#include "client.h"
#include "sys_input.h"


void SNDDMA_Activate(bool value);


namespace {


void sys_handle_window_events(const SDL_WindowEvent& e)
{
    switch (e.event) {
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        ::SNDDMA_Activate(true);
        break;

    case SDL_WINDOWEVENT_FOCUS_LOST:
        ::SNDDMA_Activate(false);
        break;
    }
}


} // namespace


void sys_poll_events()
{
    SDL_Event e;

    while (::SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_TEXTEDITING:
        case SDL_TEXTINPUT:
            ::sys_input_handle_keyboard_event(e);
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEWHEEL:
            ::sys_input_handle_mouse_event(e);
            break;

        case SDL_JOYAXISMOTION:
        case SDL_JOYBALLMOTION:
        case SDL_JOYHATMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
            ::sys_input_handle_joystick_event(e);
            break;

        case SDL_WINDOWEVENT:
            sys_handle_window_events(e.window);
            break;

        case SDL_QUIT:
            ::Com_Quit_f();
            break;
        }
    }
}
