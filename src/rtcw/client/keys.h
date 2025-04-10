/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "keycodes.h"

#define MAX_KEYS        256

typedef struct {
	qboolean down;
	int repeats;                // if > 1, it is autorepeating
	char        *binding;

#if defined RTCW_ET
	int hash;
#endif // RTCW_XX

} qkey_t;

extern qboolean key_overstrikeMode;
extern qkey_t keys[MAX_KEYS];

// NOTE TTimo the declaration of field_t and Field_Clear is now in qcommon/qcommon.h

void Field_KeyDownEvent( field_t *edit, int key );
void Field_CharEvent( field_t *edit, int ch );
void Field_Draw( field_t *edit, int x, int y, int width, qboolean showCursor );
void Field_BigDraw( field_t *edit, int x, int y, int width, qboolean showCursor );

#define     COMMAND_HISTORY     32
extern field_t historyEditLines[COMMAND_HISTORY];

extern field_t g_consoleField;
extern field_t chatField;
extern qboolean anykeydown;
extern qboolean chat_team;

#if !defined RTCW_ET
extern qboolean chat_limbo;             // NERVE - SMF
extern int chat_playerNum;
#endif // RTCW_XX

#if defined RTCW_ET
extern qboolean chat_buddy;
#endif // RTCW_XX

void Key_WriteBindings( fileHandle_t f );
void Key_SetBinding( int keynum, const char *binding );

#if defined RTCW_ET
void Key_GetBindingByString( const char* binding, int* key1, int* key2 );
#endif // RTCW_XX

const char *Key_GetBinding( int keynum );
qboolean Key_IsDown( int keynum );
qboolean Key_GetOverstrikeMode( void );
void Key_SetOverstrikeMode( qboolean state );
void Key_ClearStates( void );
int Key_GetKey( const char *binding );
