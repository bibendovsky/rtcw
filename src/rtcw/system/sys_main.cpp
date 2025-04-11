/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#define SDL_MAIN_HANDLED

#include "SDL.h"

#include "client.h"
#include "qcommon.h"
#include "sys_local.h"
#include "rtcw_main.h"
#include "rtcw_string.h"


int totalMsec = 0;
int countMsec = 0;


int main(
	int argc,
	char* argv[])
{
	if (SDL_Init(0) != 0)
	{
		const char* sdl_error_message = SDL_GetError();

		if (sdl_error_message == NULL)
		{
			sdl_error_message = "";
		}

		const size_t sdl_error_message_size = rtcw::String::traits_type::length(sdl_error_message);

		rtcw::String error_message;
		error_message.reserve(sdl_error_message_size + 64);
		error_message += "Failed to initialize SDL (";
		error_message.append(sdl_error_message, sdl_error_message_size);
		error_message += ").";

		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", error_message.c_str());

		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"rtcw",
			error_message.c_str(),
			NULL);

		return 1;
	}


	const int command_line_reserve = 1024;

	rtcw::String command_line;
	command_line.reserve(command_line_reserve);

	for (int i = 1; i < argc; ++i)
	{
		if (!command_line.empty())
		{
			command_line += ' ';
		}

		command_line += argv[i];
	}


	// done before Com/Sys_Init since we need this for error output
	Sys_CreateConsole();

	// get the initial time base
	Sys_Milliseconds();

#if defined RTCW_SP
	// re-enabled CD checking for proper 'setup.exe' file on game cd
	// (SA) enable to do cd check for setup\setup.exe
	//#if 1
#if 0
	// if we find the CD, add a +set cddir xxx command line
	if (!Sys_ScanForCD())
	{
		Sys_Error("Game CD not in drive");
	}

#endif
#else
#if 0
	// if we find the CD, add a +set cddir xxx command line
	Sys_ScanForCD();
#endif
#endif // RTCW_XX

	Com_Init(command_line.data());
	NET_Init();

#if defined RTCW_ET
#ifndef DEDICATED
	IN_Init(); // fretn - directinput must be inited after video etc
#endif
#endif // RTCW_XX

	const char* cwd = Sys_Cwd();
	Com_Printf("Working directory: %s\n", cwd);

	// hide the early console since we've reached the point where we
	// have a working graphics subsystems
	if (!com_dedicated->integer && !com_viewlog->integer)
	{
		Sys_ShowConsole(0, qfalse);
	}

	// main game loop
	while (true)
	{
		// if not running as a game client, sleep a bit
		if (com_dedicated && com_dedicated->integer)
		{
			SDL_Delay(5);
		}

		const int startTime = Sys_Milliseconds();

		// make sure mouse and joystick are only called once a frame
		IN_Frame();

		// run the game
		Com_Frame();

		const int endTime = Sys_Milliseconds();

		totalMsec += endTime - startTime;
		countMsec += 1;
	}

	// never gets here
	return 0;
}
