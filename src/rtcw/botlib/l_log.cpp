/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		l_log.c
 *
 * desc:		log file
 *
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "q_shared.h"
#include "botlib.h"
#include "be_interface.h"            //for botimport.Print
#include "l_libvar.h"

#define MAX_LOGFILENAMESIZE     1024

typedef struct logfile_s
{
	char filename[MAX_LOGFILENAMESIZE];
	FILE *fp;
	int numwrites;
} logfile_t;

static logfile_t logfile;

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Log_AlwaysOpen( const char *filename ) {
	if ( !filename || !strlen( filename ) ) {
		botimport.Print( PRT_MESSAGE, "openlog <filename>\n" );
		return;
	} //end if
	if ( logfile.fp ) {
		botimport.Print( PRT_ERROR, "log file %s is already opened\n", logfile.filename );
		return;
	} //end if
	logfile.fp = fopen( filename, "wb" );
	if ( !logfile.fp ) {
		botimport.Print( PRT_ERROR, "can't open the log file %s\n", filename );
		return;
	} //end if
	strncpy( logfile.filename, filename, MAX_LOGFILENAMESIZE );
	botimport.Print( PRT_MESSAGE, "Opened log %s\n", logfile.filename );
} //end of the function Log_Create
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Log_Open( const char *filename ) {
	if ( !LibVarValue( "log", "0" ) ) {
		return;
	}
	Log_AlwaysOpen( filename );

}
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Log_Close( void ) {
	if ( !logfile.fp ) {
		return;
	}
	if ( fclose( logfile.fp ) ) {
		botimport.Print( PRT_ERROR, "can't close log file %s\n", logfile.filename );
		return;
	} //end if
	logfile.fp = NULL;
	botimport.Print( PRT_MESSAGE, "Closed log %s\n", logfile.filename );
} //end of the function Log_Close
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Log_Shutdown( void ) {
	if ( logfile.fp ) {
		Log_Close();
	}
} //end of the function Log_Shutdown
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void QDECL Log_Write( const char *fmt, ... ) {
	va_list ap;

	if ( !logfile.fp ) {
		return;
	}
	va_start( ap, fmt );
	vfprintf( logfile.fp, fmt, ap );
	va_end( ap );
	//fprintf(logfile.fp, "\r\n");
	fflush( logfile.fp );
} //end of the function Log_Write
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void QDECL Log_WriteTimeStamped( const char *fmt, ... ) {
	va_list ap;

	if ( !logfile.fp ) {
		return;
	}
	fprintf( logfile.fp, "%d   %02d:%02d:%02d:%02d   ",
			 logfile.numwrites,
			 (int) ( botlibglobals.time / 60 / 60 ),
			 (int) ( botlibglobals.time / 60 ),
			 (int) ( botlibglobals.time ),
			 (int) ( (int) ( botlibglobals.time * 100 ) ) -
			 ( (int) botlibglobals.time ) * 100 );
	va_start( ap, fmt );
	vfprintf( logfile.fp, fmt, ap );
	va_end( ap );
	fprintf( logfile.fp, "\r\n" );
	logfile.numwrites++;
	fflush( logfile.fp );
} //end of the function Log_Write
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
FILE *Log_FilePointer( void ) {
	return logfile.fp;
} //end of the function Log_FilePointer
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Log_Flush( void ) {
	if ( logfile.fp ) {
		fflush( logfile.fp );
	}
} //end of the function Log_Flush

