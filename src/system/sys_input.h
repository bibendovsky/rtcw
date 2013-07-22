#ifndef RTCW_SYS_INPUT_H
#define RTCW_SYS_INPUT_H


#include "SDL.h"


void sys_input_handle_joystick_event(const SDL_Event& e);
void sys_input_handle_keyboard_event(const SDL_Event& e);
void sys_input_handle_mouse_event(const SDL_Event& e);


#endif // RTCW_SYS_INPUT_H
