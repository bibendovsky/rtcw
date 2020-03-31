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


#include "rtcw_input_joystick.h"
#include "SDL.h"
#include "qcommon.h"
#include "keycodes.h"


cvar_t* in_joystick = NULL;
cvar_t* in_joyBallScale = NULL;
cvar_t* in_debugJoystick = NULL;
cvar_t* joy_threshold = NULL;


void Sys_QueEvent(
    int time,
    sysEventType_t type,
    int value,
    int value2,
    int ptrLength,
    void* ptr);


namespace {


const int direction_keys[16] = {
    K_LEFTARROW, K_RIGHTARROW,
    K_UPARROW, K_DOWNARROW,
    K_JOY16, K_JOY17,
    K_JOY18, K_JOY19,
    K_JOY20, K_JOY21,
    K_JOY22, K_JOY23,

    K_JOY24, K_JOY25,
    K_JOY26, K_JOY27,
};


} // namespace


namespace rtcw {
namespace input {


Joystick::Joystick() :
    is_initialized_(false),
    id_(-1),
    instance_(NULL),
    buttons_states_()
{
}

Joystick::~Joystick()
{
    uninitialize(true);
}

bool Joystick::initialize()
{
    uninitialize();

    Com_Printf("Initializing joystick input...\n");

    in_joystick->modified = false;

    if (in_joystick->integer == 0) {
        Com_Printf(S_COLOR_YELLOW "  Ignored.\n");
        return false;
    }

    int sdl_result = 0;
    bool is_succeed = true;

    if (is_succeed) {
        if (SDL_WasInit(SDL_INIT_JOYSTICK) == 0) {
            sdl_result = SDL_InitSubSystem(SDL_INIT_JOYSTICK);

            if (sdl_result != 0) {
                is_succeed = false;
                Com_Printf(S_COLOR_RED "  %s\n", SDL_GetError());
            }
        }
    }

    if (is_succeed) {
        int joystick_count = SDL_NumJoysticks();

        if (joystick_count > 0) {
            Com_Printf("  Found %d joysticks.\n", joystick_count);
            Com_Printf("  Using first joystick.\n");
        } else {
            is_succeed = false;
            Com_Printf(S_COLOR_YELLOW "  Not found.\n");
        }
    }

    if (is_succeed) {
        instance_ = SDL_JoystickOpen(0);

        if (instance_ == NULL) {
            is_succeed = false;
            Com_Printf(S_COLOR_RED "  %s\n", SDL_GetError());
        }
    }

    if (is_succeed) {
        const char* name = SDL_JoystickName(instance_);

        if (name == NULL)
            name = "";

        SDL_JoystickGUID guid = SDL_JoystickGetGUID(instance_);

        char guid_string[33];
        guid_string[0] = '\0';
        SDL_JoystickGetGUIDString(guid, guid_string, 33);

        int axes_count = SDL_JoystickNumAxes(instance_);
        int trackballs_count = SDL_JoystickNumBalls(instance_);
        int buttons_count = SDL_JoystickNumButtons(instance_);
        int pov_hats_count = SDL_JoystickNumHats(instance_);

        Com_Printf("  Name: %s\n", name);
        Com_Printf("  GUID: %s\n", guid_string);
        Com_Printf("  Number of axes: %d\n", axes_count);
        Com_Printf("  Number of trackballs: %d\n", trackballs_count);
        Com_Printf("  Number of buttons: %d\n", buttons_count);
        Com_Printf("  Number of POV hats: %d\n", pov_hats_count);

        id_ = SDL_JoystickInstanceID(instance_);

        is_initialized_ = true;
    } else
        uninitialize(true);

    return is_succeed;
}

void Joystick::uninitialize(bool quiet)
{
    if (!quiet)
        Com_Printf("Uninitializing joystick input...\n");

    is_initialized_ = false;
    id_ = -1;

    if (instance_ != NULL) {
        SDL_JoystickClose(instance_);
        instance_ = NULL;
    }

    buttons_states_.reset();
}

void Joystick::handle_event(const SDL_Event& e)
{
    if (!is_initialized_)
        return;

    switch (e.type) {
    case SDL_JOYAXISMOTION:
        handle_axis_event(e.jaxis);
        break;

    case SDL_JOYBALLMOTION:
        handle_ball_event(e.jball);
        break;

    case SDL_JOYHATMOTION:
        handle_hat_event(e.jhat);
        break;

    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
        handle_button_event(e.jbutton);
        break;

    case SDL_JOYDEVICEADDED:
    case SDL_JOYDEVICEREMOVED:
        break;

    default:
        assert(!"Expected joystick event.");
        return;
    }
}

void Joystick::reset_state()
{
    if (!buttons_states_.any())
        return;

    Uint32 timestamp = SDL_GetTicks();

    for (int i = 0; i < MAX_BUTTONS_STATES; ++i) {
        bool state = buttons_states_.test(i);

        if (!state)
            continue;

        Sys_QueEvent(
            timestamp,
            SE_KEY,
            direction_keys[i],
            false,
            0,
            NULL);
    }

    buttons_states_.reset();
}

// (static)
void Joystick::register_cvars()
{
    in_joystick = Cvar_Get("in_joystick", "0", CVAR_ARCHIVE | CVAR_LATCH);
    in_joyBallScale = Cvar_Get("in_joyBallScale", "0.02", CVAR_ARCHIVE);
    in_debugJoystick = Cvar_Get("in_debugjoystick", "0", CVAR_TEMP);
    joy_threshold = Cvar_Get("joy_threshold", "0.15", CVAR_ARCHIVE);
}

void Joystick::handle_axis_event(const SDL_JoyAxisEvent& e)
{
    assert(e.type == SDL_JOYAXISMOTION);

    if (e.axis > 3)
        return;

    if (e.value == 0)
        return;

    float threshold = 0.15F;

    if (joy_threshold != NULL)
        threshold = joy_threshold->value;
    else
        Com_DPrintf(S_COLOR_YELLOW "Null cvar: %s\n", "joy_threshold");

    float value = e.value / 32768.0F;

    int index = (value > threshold ? 1 : 0);
    int key_index = index * e.axis;

    buttons_states_.flip(key_index);

    Sys_QueEvent(
        e.timestamp,
        SE_KEY,
        direction_keys[key_index],
        buttons_states_[key_index],
        0,
        NULL);

    if (can_debug()) {
        Com_Printf(
            "axis: %d; value: %d\n",
            e.axis,
            e.value);
    }
}

void Joystick::handle_ball_event(const SDL_JoyBallEvent& e)
{
    assert(e.type == SDL_JOYBALLMOTION);

    float scale = in_joyBallScale->value;

    int x = static_cast<int>(e.xrel * scale);
    int y = static_cast<int>(e.yrel * scale);

    if (x == 0 && y == 0)
        return;

    Sys_QueEvent(
        e.timestamp,
        SE_MOUSE,
        x,
        y,
        0,
        NULL);

    if (can_debug()) {
        Com_Printf(
            "ball: %d; dx: %d; dy: %d\n",
            e.ball,
            e.xrel,
            e.yrel);
    }
}

void Joystick::handle_hat_event(const SDL_JoyHatEvent& e)
{
    assert(e.type == SDL_JOYHATMOTION);

    int indices[2] = { -1, -1, };

    switch (e.value) {
    case SDL_HAT_UP:
        indices[0] = 12;
        break;

    case SDL_HAT_RIGHT:
        indices[0] = 14;
        break;

    case SDL_HAT_DOWN:
        indices[0] = 13;
        break;

    case SDL_HAT_LEFT:
        indices[0] = 15;
        break;

    case SDL_HAT_RIGHTUP:
        indices[0] = 14;
        indices[1] = 12;
        break;

    case SDL_HAT_RIGHTDOWN:
        indices[0] = 14;
        indices[1] = 13;
        break;

    case SDL_HAT_LEFTUP:
        indices[0] = 15;
        indices[1] = 12;
        break;

    case SDL_HAT_LEFTDOWN:
        indices[0] = 15;
        indices[1] = 13;
        break;
    }

    for (int i = 0; i < 2; ++i) {
        int index = indices[i];

        if (index < 0)
            continue;

        buttons_states_.flip(index);

        Sys_QueEvent(
            e.timestamp,
            SE_KEY,
            direction_keys[index],
            buttons_states_[index],
            0,
            NULL);
    }

    if (can_debug()) {
        Com_Printf(
            "hat: %d; dir: %s\n",
            e.hat,
            pov_hat_value_to_string(e.value));
    }
}

void Joystick::handle_button_event(const SDL_JoyButtonEvent& e)
{
    assert(e.type == SDL_JOYBUTTONDOWN || e.type == SDL_JOYBUTTONUP);

    if (e.which != id_)
        return;

    Sys_QueEvent(
        e.timestamp,
        SE_KEY,
        K_JOY1 + e.button,
        e.state == SDL_PRESSED,
        0,
        NULL);

    if (can_debug()) {
        Com_Printf(
            "button: %d; action: %s\n",
            e.button,
            button_state_to_string(e.state));
    }
}

void Joystick::handle_device_event(const SDL_JoyDeviceEvent& e)
{
    assert(e.type == SDL_JOYDEVICEADDED || e.type == SDL_JOYDEVICEREMOVED);

    if (e.type != SDL_JOYDEVICEREMOVED)
        return;

    if (e.which != id_)
        return;

    if (can_debug()) {
        Com_DPrintf(
            "id: %d; state: %s\n",
            e.which,
            device_state_to_string(e.type));
    }

    Com_Printf(S_COLOR_YELLOW "Joystick device removed.\n");
    Com_Printf(S_COLOR_YELLOW "Uninitializing joystick input.\n");

    uninitialize(true);
}

// (static)
bool Joystick::can_debug()
{
    if (in_debugJoystick != NULL)
        return in_debugJoystick->integer != 0;

    Com_DPrintf(S_COLOR_YELLOW "Null cvar: %s\n", "in_debugJoystick");
    return false;
}

// (static)
const char* Joystick::pov_hat_value_to_string(Uint8 value)
{
    switch (value) {
    case SDL_HAT_CENTERED:
        return "centered";

    case SDL_HAT_UP:
        return "up";

    case SDL_HAT_RIGHT:
        return "right";

    case SDL_HAT_DOWN:
        return "down";

    case SDL_HAT_LEFT:
        return "left";

    case SDL_HAT_RIGHTUP:
        return "right-up";

    case SDL_HAT_RIGHTDOWN:
        return "right-down";

    case SDL_HAT_LEFTUP:
        return "left-up";

    case SDL_HAT_LEFTDOWN:
        return "left-down";

    default:
        return "unknown";
    }
}

// (static)
const char* Joystick::button_state_to_string(Uint8 value)
{
    switch (value) {
    case SDL_PRESSED:
        return "pressed";

    case SDL_RELEASED:
        return "released";

    default:
        return "unknown";
    }
}

// (static)
const char* Joystick::device_state_to_string(uint32_t value)
{
    switch (value) {
    case SDL_JOYDEVICEADDED:
        return "added";

    case SDL_JOYDEVICEREMOVED:
        return "removed";

    default:
        return "unknown";
    }
}


} // namespace input
} // namespace rtcw
