/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "client.h"

cvar_t *cl_shownet;
// TTimo: win32 dedicated
cvar_t  *cl_language;

void CL_Shutdown( void ) {
}

void CL_Init( void ) {
	cl_shownet = Cvar_Get( "cl_shownet", "0", CVAR_TEMP );
	// TTimo: localisation, prolly not any use in dedicated / null client
	cl_language = Cvar_Get( "cl_language", "0", CVAR_ARCHIVE );
}

void CL_MouseEvent( int dx, int dy, int time ) {
}

void Key_WriteBindings( fileHandle_t f ) {
}

void CL_Frame( int msec ) {
}

void CL_PacketEvent( netadr_t from, msg_t *msg ) {
}

void CL_CharEvent( int key ) {
}

void CL_Disconnect( qboolean showMainMenu ) {
}

void CL_MapLoading( void ) {
}

qboolean CL_GameCommand( void ) {
	return qfalse; // bk001204 - non-void
}

void CL_KeyEvent( int key, qboolean down, unsigned time ) {
}

qboolean UI_GameCommand( void ) {
	return qfalse;
}

void CL_ForwardCommandToServer( const char *string ) {
}

void CL_ConsolePrint( char *txt ) {
}

void CL_JoystickEvent( int axis, int value, int time ) {
}

void CL_InitKeyCommands( void ) {
}

void CL_CDDialog( void ) {
}

void CL_FlushMemory( void ) {
}

void CL_StartHunkUsers( void ) {
}

// bk001119 - added new dummy for sv_init.c
void CL_ShutdownAll( void ) {};

// bk001208 - added new dummy (RC4)
qboolean CL_CDKeyValidate( const char *key, const char *checksum ) { return qtrue; }

// TTimo added for win32 dedicated
void Key_ClearStates( void ) {
}
