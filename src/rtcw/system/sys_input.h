/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_SYS_INPUT_H
#define RTCW_SYS_INPUT_H


union SDL_Event;


void sys_input_handle_event(const SDL_Event& e);


#endif // RTCW_SYS_INPUT_H
