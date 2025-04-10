/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
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
