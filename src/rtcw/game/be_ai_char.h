/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_ai_char.h
 *
 * desc:		bot characters
 *
 *
 *****************************************************************************/

//loads a bot character from a file
int BotLoadCharacter( char *charfile, int skill );
//frees a bot character
void BotFreeCharacter( int character );
//float characteristic
float Characteristic_Float( int character, int index );
//bounded float characteristic
float Characteristic_BFloat( int character, int index, float min, float max );
//integer characteristic
int Characteristic_Integer( int character, int index );
//bounded integer characteristic
int Characteristic_BInteger( int character, int index, int min, int max );
//string characteristic
void Characteristic_String( int character, int index, char *buf, int size );
//free cached bot characters
void BotShutdownCharacters( void );
