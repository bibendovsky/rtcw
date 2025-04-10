/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		l_libvar.h
 *
 * desc:		botlib vars
 *
 *
 *****************************************************************************/

//library variable
typedef struct libvar_s
{
	char        *name;
	char        *string;
	int flags;
	qboolean modified;      // set each time the cvar is changed
	float value;
	struct  libvar_s *next;
} libvar_t;

//removes all library variables
void LibVarDeAllocAll( void );
//gets the library variable with the given name
libvar_t *LibVarGet( const char *var_name );
//gets the string of the library variable with the given name
const char *LibVarGetString( const char *var_name );
//gets the value of the library variable with the given name
float LibVarGetValue( const char *var_name );
//creates the library variable if not existing already and returns it
libvar_t *LibVar( const char *var_name, const char *value );
//creates the library variable if not existing already and returns the value
float LibVarValue( const char *var_name, const char *value );
//creates the library variable if not existing already and returns the value string
char *LibVarString( const char *var_name, const char *value );
//sets the library variable
void LibVarSet( const char *var_name, const char *value );
//returns true if the library variable has been modified
qboolean LibVarChanged( const char *var_name );
//sets the library variable to unmodified
void LibVarSetNotModified( const char *var_name );

