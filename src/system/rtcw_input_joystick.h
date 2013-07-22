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


#ifndef RTCW_INPUT_JOYSTICK_H
#define RTCW_INPUT_JOYSTICK_H


#include <bitset>
#include <string>

#include "SDL.h"

#include "q_shared.h"


extern cvar_t* in_joystick;
extern cvar_t* in_joyBallScale;
extern cvar_t* in_debugJoystick;
extern cvar_t* joy_threshold;


namespace rtcw {
namespace input {


class Joystick {
public:
    Joystick();
    ~Joystick();

    bool initialize();
    void uninitialize(bool quiet = false);

    void handle_event(const SDL_Event& e);
    void reset_state();

    static void register_cvars();


private:
    static const size_t MAX_BUTTONS_STATES = 16;

    bool is_initialized_;
    SDL_JoystickID id_;
    SDL_Joystick* instance_;
    std::bitset<MAX_BUTTONS_STATES> buttons_states_;

    void handle_axis_event(const SDL_JoyAxisEvent& e);
    void handle_ball_event(const SDL_JoyBallEvent& e);
    void handle_hat_event(const SDL_JoyHatEvent& e);
    void handle_button_event(const SDL_JoyButtonEvent& e);
    void handle_device_event(const SDL_JoyDeviceEvent& e);

    static bool can_debug();
    static const char* pov_hat_value_to_string(uint8_t value);
    static const char* button_state_to_string(uint8_t value);
    static const char* device_state_to_string(uint32_t value);

    Joystick(const Joystick& that);
    Joystick& operator=(const Joystick& that);
}; // class Joystick


} // namespace input
} // namespace rtcw


#endif // RTCW_INPUT_JOYSTICK_H
