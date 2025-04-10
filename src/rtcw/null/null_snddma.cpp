/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


// snddma_null.c
// all other sound mixing is portable

#include "client.h"

qboolean SNDDMA_Init( void ) {
	return qfalse;
}

int SNDDMA_GetDMAPos( void ) {
	return 0;
}

void SNDDMA_Shutdown( void ) {
}

void SNDDMA_BeginPainting( void ) {
}

void SNDDMA_Submit( void ) {
}

// bk001119 - added boolean flag, match client/snd_public.h
sfxHandle_t S_RegisterSound( const char *name, qboolean compressed ) {
	return 0;
}

#if defined RTCW_MP
void S_StartLocalSound( sfxHandle_t sfxHandle, int channelNum ) {
}
#elif defined RTCW_ET
void S_StartLocalSound( sfxHandle_t sfxHandle, int channelNum, int volume ) {
}
#endif

#if defined RTCW_MP
void S_ClearSoundBuffer( void ) {
}
#elif defined RTCW_ET
void S_ClearSoundBuffer( qboolean killStreaming ) {
}
#endif

// TTimo: added for win32 dedicated

// BBi
//void SNDDMA_Activate( void ) {
//}
void SNDDMA_Activate (
	bool isActive)
{
}
// BBi

#if defined RTCW_ET
// show_bug.cgi?id=574
int S_GetSoundLength( sfxHandle_t sfxHandle ) {
	Com_Error( ERR_DROP, "null_snddma.c: S_GetSoundLength\n" );
	return 0;
}

void S_UpdateThread( void ) {
}

void S_AddLoopSounds( void ) {
}
#endif