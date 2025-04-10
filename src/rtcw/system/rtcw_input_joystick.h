/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#ifndef RTCW_INPUT_JOYSTICK_INCLUDED
#define RTCW_INPUT_JOYSTICK_INCLUDED


#include "SDL_events.h"

#include "q_shared.h"
#include "rtcw_bit_array_single_unit.h"


extern cvar_t* in_joystick;
extern cvar_t* in_joyBallScale;
extern cvar_t* in_debugJoystick;
extern cvar_t* joy_threshold;


namespace rtcw
{
namespace input
{


class Joystick
{
public:
	Joystick();

	~Joystick();

	bool initialize();

	void uninitialize(
		bool quiet = false);

	void handle_event(
		const SDL_Event& e);

	void reset_state();

	static void register_cvars();


private:
	static const int MAX_BUTTONS_STATES = 16;


	typedef BitArraySingleUnit<MAX_BUTTONS_STATES> ButtonsStates;


	bool is_initialized_;
	SDL_JoystickID id_;
	SDL_Joystick* instance_;
	ButtonsStates buttons_states_;


	void handle_axis_event(
		const SDL_JoyAxisEvent& e);

	void handle_ball_event(
		const SDL_JoyBallEvent& e);

	void handle_hat_event(
		const SDL_JoyHatEvent& e);

	void handle_button_event(
		const SDL_JoyButtonEvent& e);

	void handle_device_event(
		const SDL_JoyDeviceEvent& e);

	static bool can_debug();

	static const char* pov_hat_value_to_string(
		uint8_t value);

	static const char* button_state_to_string(
		uint8_t value);

	static const char* device_state_to_string(
		uint32_t value);
}; // Joystick


} // input
} // rtcw


#endif // !RTCW_INPUT_JOYSTICK_INCLUDED
