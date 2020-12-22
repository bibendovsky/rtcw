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

// win_local.h: Win32-specific Quake3 header file

#ifndef SYS_LOCAL_INCLUDED
#define SYS_LOCAL_INCLUDED


void IN_MouseEvent(int mstate);

void Sys_QueEvent(int time, sysEventType_t type, int value, int value2, int ptrLength, void* ptr);

void Sys_CreateConsole(void);
void Sys_DestroyConsole(void);

char* Sys_ConsoleInput(void);

bool Sys_GetPacket(netadr_t* net_from, msg_t* net_message);

// Input subsystem

void IN_Init();
void IN_Shutdown();
void IN_Activate(qboolean active);
void IN_Frame();

void Conbuf_AppendText(const char* msg);

// BBi
//void SNDDMA_Activate( void );
void SNDDMA_Activate(
	bool isActive);
// BBi

// BBi
#ifndef DEDICATED
void GLimp_Activate(
	bool isActivated,
	bool isMinimized);
#endif // DEDICATED
// BBi


#endif // !SYS_LOCAL_INCLUDED
