/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		l_log.h
 *
 * desc:		log file
 *
 *
 *****************************************************************************/

//open a log file
void Log_Open( const char *filename );
//
void Log_AlwaysOpen( const char *filename );
//close the current log file
void Log_Close( void );
//close log file if present
void Log_Shutdown( void );
//write to the current opened log file
void QDECL Log_Write( const char *fmt, ... );
//write to the current opened log file with a time stamp
void QDECL Log_WriteTimeStamped( const char *fmt, ... );
//returns a pointer to the log file
FILE *Log_FilePointer( void );
//flush log file
void Log_Flush( void );

