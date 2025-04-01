/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

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


#include "SDL.h"

#include "client.h"
#include "qcommon.h"
#include "sys_local.h"
#include "rtcw_string.h"


int totalMsec = 0;
int countMsec = 0;


extern "C" int main(
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
