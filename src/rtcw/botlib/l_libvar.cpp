/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		l_libvar.c
 *
 * desc:		bot library variables
 *
 *
 *****************************************************************************/

#include "q_shared.h"
#include "l_memory.h"
#include "l_libvar.h"

//list with library variables
libvar_t *libvarlist;

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
float LibVarStringValue( char *string ) {
	int dotfound = 0;
	float value = 0;

	while ( *string )
	{
		if ( *string < '0' || *string > '9' ) {
			if ( dotfound || *string != '.' ) {
				return 0;
			} //end if
			else
			{
				dotfound = 10;
				string++;
			} //end if
		} //end if
		if ( dotfound ) {
			value = value + (float) ( *string - '0' ) / (float) dotfound;
			dotfound *= 10;
		} //end if
		else
		{
			value = value * 10.0 + (float) ( *string - '0' );
		} //end else
		string++;
	} //end while
	return value;
} //end of the function LibVarStringValue
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
libvar_t *LibVarAlloc( const char *var_name ) {
	libvar_t *v;

	v = (libvar_t *) GetMemory( sizeof( libvar_t ) + strlen( var_name ) + 1 );
	memset( v, 0, sizeof( libvar_t ) );
	v->name = (char *) v + sizeof( libvar_t );
	strcpy( v->name, var_name );
	//add the variable in the list
	v->next = libvarlist;
	libvarlist = v;
	return v;
} //end of the function LibVarAlloc
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void LibVarDeAlloc( libvar_t *v ) {
	if ( v->string ) {
		FreeMemory( v->string );
	}
	FreeMemory( v );
} //end of the function LibVarDeAlloc
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void LibVarDeAllocAll( void ) {
	libvar_t *v;

	for ( v = libvarlist; v; v = libvarlist )
	{
		libvarlist = libvarlist->next;
		LibVarDeAlloc( v );
	} //end for
	libvarlist = NULL;
} //end of the function LibVarDeAllocAll
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
libvar_t *LibVarGet( const char *var_name ) {
	libvar_t *v;

	for ( v = libvarlist; v; v = v->next )
	{
		if ( !Q_stricmp( v->name, var_name ) ) {
			return v;
		} //end if
	} //end for
	return NULL;
} //end of the function LibVarGet
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
const char *LibVarGetString( const char *var_name ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		return v->string;
	} //end if
	else
	{
		return "";
	} //end else
} //end of the function LibVarGetString
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
float LibVarGetValue( const char *var_name ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		return v->value;
	} //end if
	else
	{
		return 0;
	} //end else
} //end of the function LibVarGetValue
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
libvar_t *LibVar( const char *var_name, const char *value ) {
	libvar_t *v;
	v = LibVarGet( var_name );
	if ( v ) {
		return v;
	}
	//create new variable
	v = LibVarAlloc( var_name );
	//variable string
	v->string = (char *) GetMemory( strlen( value ) + 1 );
	strcpy( v->string, value );
	//the value
	v->value = LibVarStringValue( v->string );
	//variable is modified
	v->modified = qtrue;
	//
	return v;
} //end of the function LibVar
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *LibVarString( const char *var_name, const char *value ) {
	libvar_t *v;

	v = LibVar( var_name, value );
	return v->string;
} //end of the function LibVarString
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
float LibVarValue( const char *var_name, const char *value ) {
	libvar_t *v;

	v = LibVar( var_name, value );
	return v->value;
} //end of the function LibVarValue
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void LibVarSet( const char *var_name, const char *value ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		FreeMemory( v->string );
	} //end if
	else
	{
		v = LibVarAlloc( var_name );
	} //end else
	  //variable string
	v->string = (char *) GetMemory( strlen( value ) + 1 );
	strcpy( v->string, value );
	//the value
	v->value = LibVarStringValue( v->string );
	//variable is modified
	v->modified = qtrue;
} //end of the function LibVarSet
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean LibVarChanged( const char *var_name ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		return v->modified;
	} //end if
	else
	{
		return qfalse;
	} //end else
} //end of the function LibVarChanged
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void LibVarSetNotModified( const char *var_name ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		v->modified = qfalse;
	} //end if
} //end of the function LibVarSetNotModified
