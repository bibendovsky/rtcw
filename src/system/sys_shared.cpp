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


// BBi
// Former win_shared.c
// BBi


#include "SDL_timer.h"
#include "q_shared.h"
#include "qcommon.h"
#include "win_local.h"


int Sys_Milliseconds()
{
    static int time_base = 0;
    static bool is_initialized = false;

    if (!is_initialized) {
        time_base = static_cast<int>(::SDL_GetTicks());
        is_initialized = true;
    }

    int sys_curtime = static_cast<int>(::SDL_GetTicks()) - time_base;
    return sys_curtime;
}

void Sys_SnapVector(float* v)
{
    v[0] = static_cast<float>(static_cast<int>(v[0]));
    v[1] = static_cast<float>(static_cast<int>(v[1]));
    v[2] = static_cast<float>(static_cast<int>(v[2]));
}

int Sys_GetProcessorId()
{
    return CPUID_GENERIC;
}

const char* Sys_GetCurrentUser()
{
    return "player";
}

const char* Sys_DefaultHomePath()
{
    return NULL;
}

const char* Sys_DefaultInstallPath()
{
    return Sys_Cwd();
}
