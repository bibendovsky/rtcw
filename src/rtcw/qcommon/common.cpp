/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// common.c -- misc functions used in client and server

#include "q_shared.h"
#include "qcommon.h"
#include "SDL_events.h"
#include <setjmp.h>

#if defined RTCW_SP
#define MAXPRINTMSG 4096
#endif // RTCW_XX


#define MAX_NUM_ARGVS   50

#define MIN_DEDICATED_COMHUNKMEGS 1

#if defined RTCW_SP
#define MIN_COMHUNKMEGS 54      // RF, optimizing
#define DEF_COMHUNKMEGS "72"
#define DEF_COMZONEMEGS "30"
#else
#define MIN_COMHUNKMEGS 42 // JPW NERVE changed this to 42 for MP, was 56 for team arena and 75 for wolfSP
#define DEF_COMHUNKMEGS "56" // RF, increased this, some maps are exceeding 56mb // JPW NERVE changed this for multiplayer back to 42, 56 for depot/mp_cpdepot, 42 for everything else

#if !defined RTCW_ET
#define DEF_COMZONEMEGS "16" // JPW NERVE cut this back too was 30
#else
#define DEF_COMZONEMEGS "24" // RF, increased this from 16, to account for botlib/AAS
#endif // RTCW_XX

#endif // RTCW_XX

int com_argc;
char    *com_argv[MAX_NUM_ARGVS + 1];

#if defined RTCW_SP
extern char cl_cdkey[34];
#endif // RTCW_XX

::jmp_buf abortframe;     // an ERR_DROP occured, exit the entire frame

FILE *debuglogfile;
static fileHandle_t logfile;
fileHandle_t com_journalFile;               // events are written here
fileHandle_t com_journalDataFile;           // config files are written here

#if defined RTCW_ET
cvar_t  *com_crashed = NULL;        // ydnar: set in case of a crash, prevents CVAR_UNSAFE variables from being set from a cfg
//bani - explicit NULL to make win32 teh happy

cvar_t  *com_ignorecrash = NULL;    // bani - let experienced users ignore crashes, explicit NULL to make win32 teh happy
cvar_t  *com_pid;       // bani - process id
#endif // RTCW_XX

cvar_t  *com_viewlog;
cvar_t  *com_speeds;
cvar_t  *com_developer;
cvar_t  *com_dedicated;
cvar_t  *com_timescale;
cvar_t  *com_fixedtime;
cvar_t  *com_dropsim;       // 0.0 to 1.0, simulated packet drops
cvar_t  *com_journal;
cvar_t  *com_maxfps;
cvar_t  *com_timedemo;
cvar_t  *com_sv_running;
cvar_t  *com_cl_running;
cvar_t  *com_logfile;       // 1 = buffer log, 2 = flush after each print
cvar_t  *com_showtrace;
cvar_t  *com_version;

#if !defined RTCW_ET
cvar_t  *com_blood;
#else
//cvar_t	*com_blood;
#endif // RTCW_XX

cvar_t  *com_buildScript;   // for automated data building scripts

#if defined RTCW_ET
cvar_t  *con_drawnotify;
#endif // RTCW_XX

cvar_t  *com_introPlayed;

#if defined RTCW_ET
cvar_t  *com_logosPlaying;
#endif // RTCW_XX

cvar_t  *cl_paused;
cvar_t  *sv_paused;
cvar_t  *com_cameraMode;
#if _WIN32 && _DEBUG
cvar_t  *com_noErrorInterrupt;
#endif
cvar_t  *com_recommendedSet;

#if defined RTCW_ET
cvar_t  *com_watchdog;
cvar_t  *com_watchdog_cmd;
#endif // RTCW_XX

// Rafael Notebook
cvar_t  *cl_notebook;

cvar_t  *com_hunkused;      // Ridah

// com_speeds times
int time_game;
int time_frontend;          // renderer frontend time
int time_backend;           // renderer backend time

int com_frameTime;
int com_frameMsec;
int com_frameNumber;

#if defined RTCW_ET
int com_expectedhunkusage;
int com_hunkusedvalue;
#endif // RTCW_XX

qboolean com_errorEntered;
qboolean com_fullyInitialized;

char com_errorMessage[MAXPRINTMSG];

#ifndef RTCW_VANILLA
namespace {

int com_soft_error_code = 0;

} // namespace
#endif // RTCW_VANILLA

void Com_WriteConfig_f( void );
void CIN_CloseAllVideos();

//============================================================================

static char *rd_buffer;
static int rd_buffersize;
static void ( *rd_flush )( char *buffer );

void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char *) ) {
	if ( !buffer || !buffersize || !flush ) {
		return;
	}
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Com_EndRedirect( void ) {
	if ( rd_flush ) {
		rd_flush( rd_buffer );
	}

	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

/*
=============
Com_Printf

Both client and server can use this, and it will output
to the apropriate place.

A raw string should NEVER be passed as fmt, because of "%f" type crashers.
=============
*/
#if !defined RTCW_ET
void QDECL Com_Printf( const char *fmt, ... ) {
	va_list argptr;
#else
int QDECL Com_VPrintf( const char *fmt, va_list argptr ) {
#endif // RTCW_XX

	char msg[MAXPRINTMSG];
	static qboolean opening_qconsole = qfalse;

#if !defined RTCW_ET
	va_start( argptr,fmt );
#endif // RTCW_XX

#if defined RTCW_ET
	// FIXME TTimo
	// switched vsprintf -> vsnprintf
	// rcon could cause buffer overflow
	//
#endif // RTCW_XX

// BBi See #LBUG0001
//#if defined RTCW_SP
//	vsprintf( msg,fmt,argptr );
//#else
//	Q_vsnprintf( msg, sizeof( msg ), fmt, argptr );
//#endif // RTCW_XX

	msg[MAXPRINTMSG - 1] = '\0';

#if defined RTCW_SP
	vsnprintf (msg, MAXPRINTMSG - 1, fmt, argptr);
#else
	Q_vsnprintf (msg, MAXPRINTMSG - 1, fmt, argptr);
#endif // RTCW_XX
// BBi

#if !defined RTCW_ET
	va_end( argptr );
#endif // RTCW_XX

	if ( rd_buffer ) {
		if ( ( strlen( msg ) + strlen( rd_buffer ) ) > ( rd_buffersize - 1 ) ) {
			rd_flush( rd_buffer );
			*rd_buffer = 0;
		}
		Q_strcat( rd_buffer, rd_buffersize, msg );
		// show_bug.cgi?id=51
		// only flush the rcon buffer when it's necessary, avoid fragmenting
		//rd_flush(rd_buffer);
		//*rd_buffer = 0;

#if !defined RTCW_ET
		return;
#else
		return strlen( msg );
#endif // RTCW_XX

	}

	// echo to console if we're not a dedicated server
	if ( com_dedicated && !com_dedicated->integer ) {
		CL_ConsolePrint( msg );
	}

	// echo to dedicated console and early console
	Sys_Print( msg );

	// logfile
	if ( com_logfile && com_logfile->integer ) {
		// TTimo: only open the qconsole.log if the filesystem is in an initialized state
		//   also, avoid recursing in the qconsole.log opening (i.e. if fs_debug is on)
		if ( !logfile && FS_Initialized() && !opening_qconsole ) {
			struct tm *newtime;
			time_t aclock;

			opening_qconsole = qtrue;

			time( &aclock );
			newtime = localtime( &aclock );

#if defined RTCW_SP
			logfile = FS_FOpenFileWrite( "rtcwconsole.log" );    //----(SA)	changed name for Wolf
#elif defined RTCW_MP
			logfile = FS_FOpenFileWrite( "rtcwconsole.log" );
#else
			logfile = FS_FOpenFileWrite( "etconsole.log" );
#endif // RTCW_XX

			Com_Printf( "logfile opened on %s\n", asctime( newtime ) );
			if ( com_logfile->integer > 1 ) {
				// force it to not buffer so we get valid
				// data even if we are crashing
				FS_ForceFlush( logfile );
			}

			opening_qconsole = qfalse;
		}
		if ( logfile && FS_Initialized() ) {
			FS_Write( msg, strlen( msg ), logfile );
		}
	}

#if defined RTCW_ET
	return strlen( msg );
#endif // RTCW_XX

}

#if defined RTCW_ET
// BBi
//int QDECL Com_VPrintf( const char *fmt, va_list argptr ) _attribute( ( format( printf,1,0 ) ) );
// BBi

void QDECL Com_Printf( const char *fmt, ... ) {
	va_list argptr;

	va_start( argptr, fmt );
	Com_VPrintf( fmt, argptr );
	va_end( argptr );
}

// BBi
//void QDECL Com_Printf( const char *fmt, ... ) _attribute( ( format( printf,1,2 ) ) );
// BBi
#endif // RTCW_XX


/*
================
Com_DPrintf

A Com_Printf that only shows up if the "developer" cvar is set
================
*/
void QDECL Com_DPrintf( const char *fmt, ... ) {
	va_list argptr;
	char msg[MAXPRINTMSG];

#if !defined RTCW_ET
	if ( !com_developer || !com_developer->integer ) {
#else
	if ( !com_developer || com_developer->integer != 1 ) {
#endif // RTCW_XX

		return;         // don't confuse non-developers with techie stuff...
	}

	va_start( argptr,fmt );

// BBi See #LBUG0001
//#if defined RTCW_SP
//	vsprintf( msg,fmt,argptr );
//#else
//	Q_vsnprintf( msg, sizeof( msg ), fmt, argptr );
//#endif // RTCW_XX

	msg[MAXPRINTMSG - 1] = '\0';

#if defined RTCW_SP
	vsnprintf (msg, MAXPRINTMSG - 1, fmt, argptr);
#else
	Q_vsnprintf (msg, MAXPRINTMSG - 1, fmt, argptr);
#endif // RTCW_XX
// BBi

	va_end( argptr );

	Com_Printf( "%s", msg );
}

#if defined RTCW_ET
void QDECL Com_DPrintf( const char *fmt, ... ) _attribute( ( format( printf,1,2 ) ) );
#endif // RTCW_XX

/*
=============
Com_Error

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void QDECL Com_Error( int code, const char *fmt, ... ) {
	va_list argptr;
	static int lastErrorTime;
	static int errorCount;
	int currentTime;

	// when we are running automated scripts, make sure we
	// know if anything failed
	if ( com_buildScript && com_buildScript->integer ) {

#if defined RTCW_SP
		// ERR_ENDGAME is not really an error, don't die if building a script
		if ( code != ERR_ENDGAME ) {
			code = ERR_FATAL;
		}
#else
		code = ERR_FATAL;
#endif // RTCW_XX

	}

	// make sure we can get at our local stuff
	FS_PureServerSetLoadedPaks( "", "" );

	// if we are getting a solid stream of ERR_DROP, do an ERR_FATAL
	currentTime = Sys_Milliseconds();
	if ( currentTime - lastErrorTime < 100 ) {
		if ( ++errorCount > 3 ) {
			code = ERR_FATAL;
		}
	} else {
		errorCount = 0;
	}
	lastErrorTime = currentTime;

	if ( com_errorEntered ) {

#if !defined RTCW_ET
		Sys_Error( "recursive error after: %s", com_errorMessage );
#else
		char buf[4096];

		va_start( argptr,fmt );
		Q_vsnprintf( buf, sizeof( buf ), fmt, argptr );
		va_end( argptr );

		Sys_Error( "recursive error '%s' after: %s", buf, com_errorMessage );
#endif // RTCW_XX

	}
	com_errorEntered = qtrue;

	va_start( argptr,fmt );

#if defined RTCW_SP
	vsprintf( com_errorMessage,fmt,argptr );
#else
	Q_vsnprintf( com_errorMessage, sizeof( com_errorMessage ), fmt, argptr );
#endif // RTCW_XX

	va_end( argptr );

#if defined RTCW_SP
	if ( code != ERR_DISCONNECT && code != ERR_NEED_CD && code != ERR_ENDGAME ) {
#else
	if ( code != ERR_DISCONNECT && code != ERR_NEED_CD ) {
#endif // RTCW_XX

		Cvar_Set( "com_errorMessage", com_errorMessage );
	}

#ifdef RTCW_VANILLA
	if ( code == ERR_SERVERDISCONNECT ) {
		CL_Disconnect( qtrue );
		CL_FlushMemory();
		com_errorEntered = qfalse;
		longjmp( abortframe, -1 );

#if defined RTCW_SP
	} else if ( code == ERR_ENDGAME ) {  //----(SA)	added
		SV_Shutdown( "endgame" );
		if ( com_cl_running && com_cl_running->integer ) {
			CL_Disconnect( qtrue );
			CL_FlushMemory();
			com_errorEntered = qfalse;
			CL_EndgameMenu();
		}
		longjmp( abortframe, -1 );
#endif // RTCW_SP

	} else if ( code == ERR_DROP || code == ERR_DISCONNECT ) {
		Com_Printf( "********************\nERROR: %s\n********************\n", com_errorMessage );
		SV_Shutdown( va( "Server crashed: %s\n",  com_errorMessage ) );
		CL_Disconnect( qtrue );
		CL_FlushMemory();
		com_errorEntered = qfalse;
		longjmp( abortframe, -1 );
	} else if ( code == ERR_NEED_CD ) {
		SV_Shutdown( "Server didn't have CD\n" );
		if ( com_cl_running && com_cl_running->integer ) {
			CL_Disconnect( qtrue );
			CL_FlushMemory();
			com_errorEntered = qfalse;
			CL_CDDialog();
		} else {
			Com_Printf( "Server didn't have CD\n" );
		}
		longjmp( abortframe, -1 );

#if defined RTCW_ET
#ifndef DEDICATED
	} else if ( code == ERR_AUTOUPDATE ) {
		CL_Disconnect( qtrue );
		CL_FlushMemory();
		com_errorEntered = qfalse;
		if ( !Q_stricmpn( com_errorMessage, "Server is full", 14 ) && CL_NextUpdateServer() ) {
			CL_GetAutoUpdate();
		} else {
			longjmp( abortframe, -1 );
		}
#endif
#endif // RTCW_ET

	} else {
		CL_Shutdown();
		SV_Shutdown( va( "Server fatal crashed: %s\n", com_errorMessage ) );
	}

#else // RTCW_VANILLA

	if (
#ifdef RTCW_SP
		code == ERR_ENDGAME ||
#endif // RTCW_SP
		code == ERR_SERVERDISCONNECT ||
		code == ERR_DROP ||
		code == ERR_DISCONNECT ||
		code == ERR_NEED_CD)
	{
		com_soft_error_code = code;
		::longjmp( abortframe, -1 );
	}
#ifdef RTCW_ET
#ifndef DEDICATED
	else if (code == ERR_AUTOUPDATE)
	{
		if ( !Q_stricmpn( com_errorMessage, "Server is full", 14 ) && CL_NextUpdateServer() )
		{
			CL_Disconnect( qtrue );
			CL_FlushMemory();
			com_errorEntered = qfalse;
			CL_GetAutoUpdate();
		}
		else
		{
			com_soft_error_code = code;
			::longjmp( abortframe, -1 );
		}
	}
#endif // DEDICATED
#endif // RTCW_ET
	else
	{
		CL_Shutdown();
		SV_Shutdown( va( "Server fatal crashed: %s\n", com_errorMessage ) );
	}
#endif // RTCW_VANILLA

#if !defined RTCW_ET
	Com_Shutdown();
#else
	Com_Shutdown( code == ERR_VID_FATAL ? qtrue : qfalse );
#endif // RTCW_XX

	Sys_Error( "%s", com_errorMessage );
}

#if defined RTCW_ET
void QDECL Com_Error( int code, const char *fmt, ... ) _attribute( ( format( printf,2,3 ) ) );

//bani - moved
void CL_ShutdownCGame( void );
#endif // RTCW_XX

/*
=============
Com_Quit_f

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Quit_f( void ) {
	// don't try to shutdown if we are in a recursive error
	if ( !com_errorEntered ) {
		SV_Shutdown( "Server quit\n" );

#if defined RTCW_ET
//bani
#ifndef DEDICATED
		CL_ShutdownCGame();
#endif
#endif // RTCW_XX

		CL_Shutdown();

#if !defined RTCW_ET
		Com_Shutdown();
#else
		Com_Shutdown( qfalse );
#endif // RTCW_XX

		FS_Shutdown( qtrue );
	}
	Sys_Quit();
}



/*
============================================================================

COMMAND LINE FUNCTIONS

+ characters seperate the commandLine string into multiple console
command lines.

All of these are valid:

quake3 +set test blah +map test
quake3 set test blah+map test
quake3 set test blah + map test

============================================================================
*/

#define MAX_CONSOLE_LINES   32
int com_numConsoleLines;
char    *com_consoleLines[MAX_CONSOLE_LINES];

/*
==================
Com_ParseCommandLine

Break it up into multiple console lines
==================
*/
void Com_ParseCommandLine( char *commandLine ) {

#if defined RTCW_ET
	int inq = 0;
#endif // RTCW_XX

	com_consoleLines[0] = commandLine;
	com_numConsoleLines = 1;

	while ( *commandLine ) {

#if defined RTCW_ET
		if ( *commandLine == '"' ) {
			inq = !inq;
		}
#endif // RTCW_XX

		// look for a + seperating character
		// if commandLine came from a file, we might have real line seperators

#if !defined RTCW_ET
		if ( *commandLine == '+' || *commandLine == '\n' ) {
#else
		if ( *commandLine == '+' || *commandLine == '\n' || *commandLine == '\r' ) {
#endif // RTCW_XX

			if ( com_numConsoleLines == MAX_CONSOLE_LINES ) {
				return;
			}
			com_consoleLines[com_numConsoleLines] = commandLine + 1;
			com_numConsoleLines++;
			*commandLine = 0;
		}
		commandLine++;
	}
}


/*
===================
Com_SafeMode

Check for "safe" on the command line, which will
skip loading of wolfconfig.cfg
===================
*/
qboolean Com_SafeMode( void ) {
	int i;

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( !Q_stricmp( Cmd_Argv( 0 ), "safe" )
			 || !Q_stricmp( Cmd_Argv( 0 ), "cvar_restart" ) ) {
			com_consoleLines[i][0] = 0;
			return qtrue;
		}
	}
	return qfalse;
}


/*
===============
Com_StartupVariable

Searches for command line parameters that are set commands.
If match is not NULL, only that cvar will be looked for.
That is necessary because cddir and basedir need to be set
before the filesystem is started, but all other sets shouls
be after execing the config and default.
===============
*/
void Com_StartupVariable( const char *match ) {
	int i;
	const char    *s;
	cvar_t  *cv;

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( strcmp( Cmd_Argv( 0 ), "set" ) ) {
			continue;
		}

		s = Cmd_Argv( 1 );
		if ( !match || !strcmp( s, match ) ) {
			Cvar_Set( s, Cmd_Argv( 2 ) );
			cv = Cvar_Get( s, "", 0 );
			cv->flags |= CVAR_USER_CREATED;
//			com_consoleLines[i] = 0;
		}
	}
}


/*
=================
Com_AddStartupCommands

Adds command line parameters as script statements
Commands are seperated by + signs

Returns qtrue if any late commands were added, which
will keep the demoloop from immediately starting
=================
*/
qboolean Com_AddStartupCommands( void ) {
	int i;
	qboolean added;

	added = qfalse;
	// quote every token, so args with semicolons can work
	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		if ( !com_consoleLines[i] || !com_consoleLines[i][0] ) {
			continue;
		}

		// set commands won't override menu startup
		if ( Q_stricmpn( com_consoleLines[i], "set", 3 ) ) {
			added = qtrue;
		}
		Cbuf_AddText( com_consoleLines[i] );
		Cbuf_AddText( "\n" );
	}

	return added;
}


//============================================================================

void Info_Print( const char *s ) {
	char key[512];
	char value[512];
	char    *o;
	int l;

	if ( *s == '\\' ) {
		s++;
	}
	while ( *s )
	{
		o = key;
		while ( *s && *s != '\\' )
			*o++ = *s++;

		l = o - key;
		if ( l < 20 ) {
			memset( o, ' ', 20 - l );
			key[20] = 0;
		} else {
			*o = 0;
		}
		Com_Printf( "%s", key );

		if ( !*s ) {
			Com_Printf( "MISSING VALUE\n" );
			return;
		}

		o = value;
		s++;
		while ( *s && *s != '\\' )
			*o++ = *s++;
		*o = 0;

		if ( *s ) {
			s++;
		}
		Com_Printf( "%s\n", value );
	}
}

/*
============
Com_StringContains
============
*/
const char *Com_StringContains( const char *str1, const char *str2, int casesensitive ) {
	int len, i, j;

	len = strlen( str1 ) - strlen( str2 );
	for ( i = 0; i <= len; i++, str1++ ) {
		for ( j = 0; str2[j]; j++ ) {
			if ( casesensitive ) {
				if ( str1[j] != str2[j] ) {
					break;
				}
			} else {
				if ( toupper( str1[j] ) != toupper( str2[j] ) ) {
					break;
				}
			}
		}
		if ( !str2[j] ) {
			return str1;
		}
	}
	return NULL;
}

/*
============
Com_Filter
============
*/
int Com_Filter( const char *filter, const char *name, int casesensitive ) {
	char buf[MAX_TOKEN_CHARS];
	const char *ptr;
	int i, found;

	while ( *filter ) {
		if ( *filter == '*' ) {
			filter++;
			for ( i = 0; *filter; i++ ) {
				if ( *filter == '*' || *filter == '?' ) {
					break;
				}
				buf[i] = *filter;
				filter++;
			}
			buf[i] = '\0';
			if ( strlen( buf ) ) {
				ptr = Com_StringContains( name, buf, casesensitive );
				if ( !ptr ) {
					return qfalse;
				}
				name = ptr + strlen( buf );
			}
		} else if ( *filter == '?' )      {
			filter++;
			name++;
		} else if ( *filter == '[' && *( filter + 1 ) == '[' )           {
			filter++;
		} else if ( *filter == '[' )      {
			filter++;
			found = qfalse;
			while ( *filter && !found ) {
				if ( *filter == ']' && *( filter + 1 ) != ']' ) {
					break;
				}
				if ( *( filter + 1 ) == '-' && *( filter + 2 ) && ( *( filter + 2 ) != ']' || *( filter + 3 ) == ']' ) ) {
					if ( casesensitive ) {
						if ( *name >= *filter && *name <= *( filter + 2 ) ) {
							found = qtrue;
						}
					} else {
						if ( toupper( *name ) >= toupper( *filter ) &&
							 toupper( *name ) <= toupper( *( filter + 2 ) ) ) {
							found = qtrue;
						}
					}
					filter += 3;
				} else {
					if ( casesensitive ) {
						if ( *filter == *name ) {
							found = qtrue;
						}
					} else {
						if ( toupper( *filter ) == toupper( *name ) ) {
							found = qtrue;
						}
					}
					filter++;
				}
			}
			if ( !found ) {
				return qfalse;
			}
			while ( *filter ) {
				if ( *filter == ']' && *( filter + 1 ) != ']' ) {
					break;
				}
				filter++;
			}
			filter++;
			name++;
		} else {
			if ( casesensitive ) {
				if ( *filter != *name ) {
					return qfalse;
				}
			} else {
				if ( toupper( *filter ) != toupper( *name ) ) {
					return qfalse;
				}
			}
			filter++;
			name++;
		}
	}
	return qtrue;
}

/*
============
Com_FilterPath
============
*/
int Com_FilterPath( const char *filter, const char *name, int casesensitive ) {
	int i;
	char new_filter[MAX_QPATH];
	char new_name[MAX_QPATH];

	for ( i = 0; i < MAX_QPATH - 1 && filter[i]; i++ ) {
		if ( filter[i] == '\\' || filter[i] == ':' ) {
			new_filter[i] = '/';
		} else {
			new_filter[i] = filter[i];
		}
	}
	new_filter[i] = '\0';
	for ( i = 0; i < MAX_QPATH - 1 && name[i]; i++ ) {
		if ( name[i] == '\\' || name[i] == ':' ) {
			new_name[i] = '/';
		} else {
			new_name[i] = name[i];
		}
	}
	new_name[i] = '\0';
	return Com_Filter( new_filter, new_name, casesensitive );
}

/*
============
Com_HashKey
============
*/
int Com_HashKey( const char *string, int maxlen ) {
	int hash, i;

	hash = 0;
	for ( i = 0; i < maxlen && string[i] != '\0'; i++ ) {
		hash += string[i] * ( 119 + i );
	}
	hash = ( hash ^ ( hash >> 10 ) ^ ( hash >> 20 ) );
	return hash;
}

/*
================
Com_RealTime
================
*/
int Com_RealTime( qtime_t *qtime ) {
	time_t t;
	struct tm *tms;

	t = time( NULL );
	if ( !qtime ) {
		return t;
	}
	tms = localtime( &t );
	if ( tms ) {
		qtime->tm_sec = tms->tm_sec;
		qtime->tm_min = tms->tm_min;
		qtime->tm_hour = tms->tm_hour;
		qtime->tm_mday = tms->tm_mday;
		qtime->tm_mon = tms->tm_mon;
		qtime->tm_year = tms->tm_year;
		qtime->tm_wday = tms->tm_wday;
		qtime->tm_yday = tms->tm_yday;
		qtime->tm_isdst = tms->tm_isdst;
	}
	return t;
}


#if defined RTCW_SP
/*
==============================================================================

						ZONE MEMORY ALLOCATION

==============================================================================

  The old zone is gone, mallocs replaced it. To keep the widespread code changes down to a bare minimum
  Z_Malloc and Z_Free still work.
*/
#else
/*
==============================================================================

						ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/

#define ZONEID  0x1d4a11
#define MINFRAGMENT 64

typedef struct zonedebug_s {
	char *label;
	char *file;
	int line;
	int allocSize;
} zonedebug_t;

typedef struct memblock_s {
	int size;               // including the header and possibly tiny fragments
	int tag;                // a tag of 0 is a free block
	struct memblock_s       *next, *prev;
	int id;                 // should be ZONEID
#ifdef ZONE_DEBUG
	zonedebug_t d;
#endif
} memblock_t;

typedef struct {
	int size;               // total bytes malloced, including header
	int used;               // total bytes used
	memblock_t blocklist;   // start / end cap for linked list
	memblock_t  *rover;
} memzone_t;

// main zone for all "dynamic" memory allocation
memzone_t   *mainzone;
// we also have a small zone for small allocations that would only
// fragment the main zone (think of cvar and cmd strings)
memzone_t   *smallzone;


void Z_CheckHeap( void );

/*
========================
Z_ClearZone
========================
*/
void Z_ClearZone( memzone_t *zone, int size ) {
	memblock_t  *block;

	// set the entire zone to one free block

	zone->blocklist.next = zone->blocklist.prev = block =
													  ( memblock_t * )( (byte *)zone + sizeof( memzone_t ) );
	zone->blocklist.tag = 1;    // in use block
	zone->blocklist.id = 0;
	zone->blocklist.size = 0;
	zone->rover = block;
	zone->size = size;
	zone->used = 0;

	block->prev = block->next = &zone->blocklist;
	block->tag = 0;         // free block
	block->id = ZONEID;
	block->size = size - sizeof( memzone_t );
}
#endif // RTCW_XX


/*
========================
Z_Free
========================
*/
void Z_Free( void *ptr ) {

#if defined RTCW_SP
	free( ptr );
#else
	memblock_t  *block, *other;
	memzone_t *zone;

	if ( !ptr ) {
		Com_Error( ERR_DROP, "Z_Free: NULL pointer" );
	}

	block = ( memblock_t * )( (byte *)ptr - sizeof( memblock_t ) );
	if ( block->id != ZONEID ) {
		Com_Error( ERR_FATAL, "Z_Free: freed a pointer without ZONEID" );
	}
	if ( block->tag == 0 ) {
		Com_Error( ERR_FATAL, "Z_Free: freed a freed pointer" );
	}
	// if static memory
	if ( block->tag == TAG_STATIC ) {
		return;
	}

	// check the memory trash tester
	if ( *( int * )( (byte *)block + block->size - 4 ) != ZONEID ) {
		Com_Error( ERR_FATAL, "Z_Free: memory block wrote past end" );
	}

	if ( block->tag == TAG_SMALL ) {
		zone = smallzone;
	} else {
		zone = mainzone;
	}

	zone->used -= block->size;
	// set the block to something that should cause problems
	// if it is referenced...
	memset( ptr, 0xaa, block->size - sizeof( *block ) );

	block->tag = 0;     // mark as free

	other = block->prev;
	if ( !other->tag ) {
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if ( block == zone->rover ) {
			zone->rover = other;
		}
		block = other;
	}

	zone->rover = block;

	other = block->next;
	if ( !other->tag ) {
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		if ( other == zone->rover ) {
			zone->rover = block;
		}
	}
#endif // RTCW_XX

}


#if defined RTCW_SP
/*
================
Z_Malloc
================
*/
void *Z_Malloc( int size ) {
	void *buf = malloc( size );
	Com_Memset( buf, 0, size );
	return buf;
}

#if 0
/*
================
Z_TagMalloc
================
*/
void *Z_TagMalloc( int size, int tag ) {

	if ( tag != TAG_RENDERER ) {
		assert( 0 );
	}

	if ( g_numTaggedAllocs < MAX_TAG_ALLOCS ) {
		void *ptr = Z_Malloc( size );
		g_taggedAllocations[g_numTaggedAllocs++] = ptr;
		return ptr;
	} else {
		Com_Error( ERR_FATAL, "Z_TagMalloc: out of tagged allocation space\n" );
	}
	return NULL;
}

/*
================
Z_FreeTags
================
*/
void Z_FreeTags( int tag ) {
	int i;

	if ( tag != TAG_RENDERER ) {
		assert( 0 );
	}

	for ( i = 0; i < g_numTaggedAllocs; i++ ) {
		free( g_taggedAllocations[i] );
	}

	g_numTaggedAllocs = 0;
}

#endif
#else
/*
================
Z_FreeTags
================
*/
void Z_FreeTags( int tag ) {
	int count;
	memzone_t   *zone;

	if ( tag == TAG_SMALL ) {
		zone = smallzone;
	} else {
		zone = mainzone;
	}
	count = 0;
	// use the rover as our pointer, because
	// Z_Free automatically adjusts it
	zone->rover = zone->blocklist.next;
	do {
		if ( zone->rover->tag == tag ) {
			count++;
			Z_Free( ( void * )( zone->rover + 1 ) );
			continue;
		}
		zone->rover = zone->rover->next;
	} while ( zone->rover != &zone->blocklist );
}

/*
================
Z_TagMalloc
================
*/

memblock_t *debugblock; // RF, jusy so we can track a block to find out when it's getting trashed

#ifdef ZONE_DEBUG
void *Z_TagMallocDebug( int size, int tag, char *label, char *file, int line ) {
#else
void *Z_TagMalloc( int size, int tag ) {
#endif
	int extra, allocSize;
	memblock_t  *start, *rover, *new1, *base;
	memzone_t *zone;

	if ( !tag ) {
		Com_Error( ERR_FATAL, "Z_TagMalloc: tried to use a 0 tag" );
	}

	if ( tag == TAG_SMALL ) {
		zone = smallzone;
	} else {
		zone = mainzone;
	}

	allocSize = size;
	//
	// scan through the block list looking for the first free block
	// of sufficient size
	//
	size += sizeof( memblock_t ); // account for size of block header
	size += 4;                  // space for memory trash tester
	size = ( size + 3 ) & ~3;     // align to 32 bit boundary

	base = rover = zone->rover;
	start = base->prev;

	do {
		if ( rover == start ) {
#ifdef ZONE_DEBUG
			Z_LogHeap();
#endif
			// scaned all the way around the list
			Com_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone",
					   size, zone == smallzone ? "small" : "main" );
			return NULL;
		}
		if ( rover->tag ) {
			base = rover = rover->next;
		} else {
			rover = rover->next;
		}
	} while ( base->tag || base->size < size );

	//
	// found a block big enough
	//
	extra = base->size - size;
	if ( extra > MINFRAGMENT ) {
		// there will be a free fragment after the allocated block
		new1 = ( memblock_t * )( (byte *)base + size );
		new1->size = extra;
		new1->tag = 0;           // free block
		new1->prev = base;
		new1->id = ZONEID;
		new1->next = base->next;
		new1->next->prev = new1;
		base->next = new1;
		base->size = size;
	}

	base->tag = tag;            // no longer a free block

	zone->rover = base->next;   // next allocation will start looking here
	zone->used += base->size;   //

	base->id = ZONEID;

#ifdef ZONE_DEBUG
	base->d.label = label;
	base->d.file = file;
	base->d.line = line;
	base->d.allocSize = allocSize;
#endif

	// marker for memory trash testing
	*( int * )( (byte *)base + base->size - 4 ) = ZONEID;

	return ( void * )( (byte *)base + sizeof( memblock_t ) );
}

/*
========================
Z_Malloc
========================
*/
#ifdef ZONE_DEBUG
void *Z_MallocDebug( int size, char *label, char *file, int line ) {
#else
void *Z_Malloc( int size ) {
#endif
	void    *buf;

	//Z_CheckHeap ();	// DEBUG

#ifdef ZONE_DEBUG
	buf = Z_TagMallocDebug( size, TAG_GENERAL, label, file, line );
#else
	buf = Z_TagMalloc( size, TAG_GENERAL );
#endif
	Com_Memset( buf, 0, size );

	return buf;
}

#ifdef ZONE_DEBUG
void *S_MallocDebug( int size, char *label, char *file, int line ) {
	return Z_TagMallocDebug( size, TAG_SMALL, label, file, line );
}
#else
void *S_Malloc( int size ) {
	return Z_TagMalloc( size, TAG_SMALL );
}
#endif

/*
========================
Z_CheckHeap
========================
*/
void Z_CheckHeap( void ) {
	memblock_t  *block;

	for ( block = mainzone->blocklist.next ; ; block = block->next ) {
		if ( block->next == &mainzone->blocklist ) {
			break;          // all blocks have been hit
		}
		if ( (byte *)block + block->size != (byte *)block->next ) {
			Com_Error( ERR_FATAL, "Z_CheckHeap: block size does not touch the next block\n" );
		}
		if ( block->next->prev != block ) {
			Com_Error( ERR_FATAL, "Z_CheckHeap: next block doesn't have proper back link\n" );
		}
		if ( !block->tag && !block->next->tag ) {
			Com_Error( ERR_FATAL, "Z_CheckHeap: two consecutive free blocks\n" );
		}
	}
}

/*
========================
Z_LogZoneHeap
========================
*/
void Z_LogZoneHeap( memzone_t *zone, const char *name ) {
#ifdef ZONE_DEBUG
	char dump[32], *ptr;
	int i, j;
#endif
	memblock_t  *block;
	char buf[4096];
	int size, allocSize, numBlocks;

	if ( !logfile || !FS_Initialized() ) {
		return;
	}
	size = allocSize = numBlocks = 0;
	Com_sprintf( buf, sizeof( buf ), "\r\n================\r\n%s log\r\n================\r\n", name );
	FS_Write( buf, strlen( buf ), logfile );
	for ( block = zone->blocklist.next ; block->next != &zone->blocklist; block = block->next ) {
		if ( block->tag ) {
#ifdef ZONE_DEBUG
			ptr = ( (char *) block ) + sizeof( memblock_t );
			j = 0;
			for ( i = 0; i < 20 && i < block->d.allocSize; i++ ) {
				if ( ptr[i] >= 32 && ptr[i] < 127 ) {
					dump[j++] = ptr[i];
				} else {
					dump[j++] = '_';
				}
			}
			dump[j] = '\0';
			Com_sprintf( buf, sizeof( buf ), "size = %8d: %s, line: %d (%s) [%s]\r\n", block->d.allocSize, block->d.file, block->d.line, block->d.label, dump );
			FS_Write( buf, strlen( buf ), logfile );
			allocSize += block->d.allocSize;
#endif
			size += block->size;
			numBlocks++;
		}
	}
#ifdef ZONE_DEBUG
	// subtract debug memory
	size -= numBlocks * sizeof( zonedebug_t );
#else
	allocSize = numBlocks * sizeof( memblock_t ); // + 32 bit alignment
#endif
	Com_sprintf( buf, sizeof( buf ), "%d %s memory in %d blocks\r\n", size, name, numBlocks );
	FS_Write( buf, strlen( buf ), logfile );
	Com_sprintf( buf, sizeof( buf ), "%d %s memory overhead\r\n", size - allocSize, name );
	FS_Write( buf, strlen( buf ), logfile );
}

/*
========================
Z_LogHeap
========================
*/
void Z_LogHeap( void ) {
	Z_LogZoneHeap( mainzone, "MAIN" );
	Z_LogZoneHeap( smallzone, "SMALL" );
}

// static mem blocks to reduce a lot of small zone overhead
typedef struct memstatic_s {
	memblock_t b;
	byte mem[2];
} memstatic_t;

// bk001204 - initializer brackets
memstatic_t emptystring =
{ {( sizeof( memblock_t ) + 2 + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'\0', '\0'} };
memstatic_t numberstring[] = {
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'0', '\0'} },
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'1', '\0'} },
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'2', '\0'} },
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'3', '\0'} },
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'4', '\0'} },
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'5', '\0'} },
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'6', '\0'} },
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'7', '\0'} },
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'8', '\0'} },
	{ {( sizeof( memstatic_t ) + 3 ) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'9', '\0'} }
};
#endif // RTCW_XX

/*
========================
CopyString

 NOTE:	never write over the memory CopyString returns because
		memory from a memstatic_t might be returned
========================
*/
char *CopyString( const char *in ) {
	char    *out;

#if defined RTCW_SP
	out = static_cast<char*> (Z_Malloc( strlen( in ) + 1 ));
#else
	if ( !in[0] ) {
		return ( (char *)&emptystring ) + sizeof( memblock_t );
	} else if ( !in[1] )     {
		if ( in[0] >= '0' && in[0] <= '9' ) {
			return ( (char *)&numberstring[in[0] - '0'] ) + sizeof( memblock_t );
		}
	}
	out = static_cast<char*> (S_Malloc( strlen( in ) + 1 ));
#endif // RTCW_XX

	strcpy( out, in );
	return out;
}

/*
==============================================================================

Goals:
	reproducable without history effects -- no out of memory errors on weird map to map changes
	allow restarting of the client without fragmentation
	minimize total pages in use at run time
	minimize total pages needed during load time

  Single block of memory with stack allocators coming from both ends towards the middle.

  One side is designated the temporary memory allocator.

  Temporary memory can be allocated and freed in any order.

  A highwater mark is kept of the most in use at any time.

  When there is no temporary memory allocated, the permanent and temp sides
  can be switched, allowing the already touched temp memory to be used for
  permanent storage.

  Temp memory must never be allocated on two ends at once, or fragmentation
  could occur.

  If we have any in-use temp memory, additional temp allocations must come from
  that side.

  If not, we can choose to make either side the new temp side and push future
  permanent allocations to the other side.  Permanent allocations should be
  kept on the side that has the current greatest wasted highwater mark.

==============================================================================
*/


#define HUNK_MAGIC  0x89537892
#define HUNK_FREE_MAGIC 0x89537893

typedef struct {
	int magic;
	int size;
} hunkHeader_t;

typedef struct {
	int mark;
	int permanent;
	int temp;
	int tempHighwater;
} hunkUsed_t;

typedef struct hunkblock_s {
	int size;
	byte printed;
	struct hunkblock_s *next;
	char *label;
	char *file;
	int line;
} hunkblock_t;

static hunkblock_t *hunkblocks;

static hunkUsed_t hunk_low, hunk_high;
static hunkUsed_t  *hunk_permanent, *hunk_temp;

static byte    *s_hunkData = NULL;
static int s_hunkTotal;

static int s_zoneTotal;

#if defined RTCW_SP
//static	int		s_smallZoneTotal; // TTimo: unused
#else
static int s_smallZoneTotal;
#endif // RTCW_XX

/*
=================
Com_Meminfo_f
=================
*/
void Com_Meminfo_f( void ) {

#if !defined RTCW_SP
	memblock_t  *block;
	int zoneBytes, zoneBlocks;
	int smallZoneBytes, smallZoneBlocks;
	int botlibBytes, rendererBytes;
#endif // RTCW_XX

	int unused;

#if !defined RTCW_SP
	zoneBytes = 0;
	botlibBytes = 0;
	rendererBytes = 0;
	zoneBlocks = 0;
	for ( block = mainzone->blocklist.next ; ; block = block->next ) {
		if ( Cmd_Argc() != 1 ) {
			Com_Printf( "block:%p    size:%7i    tag:%3i\n",
						block, block->size, block->tag );
		}
		if ( block->tag ) {
			zoneBytes += block->size;
			zoneBlocks++;
			if ( block->tag == TAG_BOTLIB ) {
				botlibBytes += block->size;
			} else if ( block->tag == TAG_RENDERER ) {
				rendererBytes += block->size;
			}
		}

		if ( block->next == &mainzone->blocklist ) {
			break;          // all blocks have been hit
		}
		if ( (byte *)block + block->size != (byte *)block->next ) {
			Com_Printf( "ERROR: block size does not touch the next block\n" );
		}
		if ( block->next->prev != block ) {
			Com_Printf( "ERROR: next block doesn't have proper back link\n" );
		}
		if ( !block->tag && !block->next->tag ) {
			Com_Printf( "ERROR: two consecutive free blocks\n" );
		}
	}

	smallZoneBytes = 0;
	smallZoneBlocks = 0;
	for ( block = smallzone->blocklist.next ; ; block = block->next ) {
		if ( block->tag ) {
			smallZoneBytes += block->size;
			smallZoneBlocks++;
		}

		if ( block->next == &smallzone->blocklist ) {
			break;          // all blocks have been hit
		}
	}
#endif // RTCW_XX

#if !defined RTCW_ET
	Com_Printf( "%8i bytes total hunk\n", s_hunkTotal );
	Com_Printf( "%8i bytes total zone\n", s_zoneTotal );
	Com_Printf( "\n" );
	Com_Printf( "%8i low mark\n", hunk_low.mark );
	Com_Printf( "%8i low permanent\n", hunk_low.permanent );
	if ( hunk_low.temp != hunk_low.permanent ) {
		Com_Printf( "%8i low temp\n", hunk_low.temp );
	}
	Com_Printf( "%8i low tempHighwater\n", hunk_low.tempHighwater );
	Com_Printf( "\n" );
	Com_Printf( "%8i high mark\n", hunk_high.mark );
	Com_Printf( "%8i high permanent\n", hunk_high.permanent );
	if ( hunk_high.temp != hunk_high.permanent ) {
		Com_Printf( "%8i high temp\n", hunk_high.temp );
	}
	Com_Printf( "%8i high tempHighwater\n", hunk_high.tempHighwater );
	Com_Printf( "\n" );
	Com_Printf( "%8i total hunk in use\n", hunk_low.permanent + hunk_high.permanent );
#else
	Com_Printf( "%9i bytes (%6.2f MB) total hunk\n", s_hunkTotal, s_hunkTotal / Square( 1024.f ) );
	Com_Printf( "%9i bytes (%6.2f MB) total zone\n", s_zoneTotal, s_zoneTotal / Square( 1024.f ) );
	Com_Printf( "\n" );
	Com_Printf( "%9i bytes (%6.2f MB) low mark\n", hunk_low.mark, hunk_low.mark / Square( 1024.f ) );
	Com_Printf( "%9i bytes (%6.2f MB) low permanent\n", hunk_low.permanent, hunk_low.permanent / Square( 1024.f ) );
	if ( hunk_low.temp != hunk_low.permanent ) {
		Com_Printf( "%9i bytes (%6.2f MB) low temp\n", hunk_low.temp, hunk_low.temp / Square( 1024.f ) );
	}
	Com_Printf( "%9i bytes (%6.2f MB) low tempHighwater\n", hunk_low.tempHighwater, hunk_low.tempHighwater / Square( 1024.f ) );
	Com_Printf( "\n" );
	Com_Printf( "%9i bytes (%6.2f MB) high mark\n", hunk_high.mark, hunk_high.mark / Square( 1024.f ) );
	Com_Printf( "%9i bytes (%6.2f MB) high permanent\n", hunk_high.permanent, hunk_high.permanent / Square( 1024.f ) );
	if ( hunk_high.temp != hunk_high.permanent ) {
		Com_Printf( "%9i bytes (%6.2f MB) high temp\n", hunk_high.temp, hunk_high.temp / Square( 1024.f ) );
	}
	Com_Printf( "%9i bytes (%6.2f MB) high tempHighwater\n", hunk_high.tempHighwater, hunk_high.tempHighwater / Square( 1024.f ) );
	Com_Printf( "\n" );
	Com_Printf( "%9i bytes (%6.2f MB) total hunk in use\n", hunk_low.permanent + hunk_high.permanent, ( hunk_low.permanent + hunk_high.permanent ) / Square( 1024.f ) );
#endif // RTCW_XX

	unused = 0;
	if ( hunk_low.tempHighwater > hunk_low.permanent ) {
		unused += hunk_low.tempHighwater - hunk_low.permanent;
	}
	if ( hunk_high.tempHighwater > hunk_high.permanent ) {
		unused += hunk_high.tempHighwater - hunk_high.permanent;
	}

#if !defined RTCW_ET
	Com_Printf( "%8i unused highwater\n", unused );
#else
	Com_Printf( "%9i bytes (%6.2f MB) unused highwater\n", unused, unused / Square( 1024.f ) );
#endif // RTCW_XX

	Com_Printf( "\n" );

#if defined RTCW_SP
	//Com_Printf( "        %i number of tagged renderer allocations\n", g_numTaggedAllocs);
#elif defined RTCW_MP
	Com_Printf( "%8i bytes in %i zone blocks\n", zoneBytes, zoneBlocks    );
	Com_Printf( "        %8i bytes in dynamic botlib\n", botlibBytes );
	Com_Printf( "        %8i bytes in dynamic renderer\n", rendererBytes );
	Com_Printf( "        %8i bytes in dynamic other\n", zoneBytes - ( botlibBytes + rendererBytes ) );
	Com_Printf( "        %8i bytes in small Zone memory\n", smallZoneBytes );
#else
	Com_Printf( "%9i bytes (%6.2f MB) in %i zone blocks\n", zoneBytes, zoneBytes / Square( 1024.f ), zoneBlocks );
	Com_Printf( "        %9i bytes (%6.2f MB) in dynamic botlib\n", botlibBytes, botlibBytes / Square( 1024.f ) );
	Com_Printf( "        %9i bytes (%6.2f MB) in dynamic renderer\n", rendererBytes, rendererBytes / Square( 1024.f ) );
	Com_Printf( "        %9i bytes (%6.2f MB) in dynamic other\n", zoneBytes - ( botlibBytes + rendererBytes ), ( zoneBytes - ( botlibBytes + rendererBytes ) ) / Square( 1024.f ) );
	Com_Printf( "        %9i bytes (%6.2f MB) in small Zone memory\n", smallZoneBytes, smallZoneBytes / Square( 1024.f ) );
#endif // RTCW_XX

}

/*
===============
Com_TouchMemory

Touch all known used data to make sure it is paged in
===============
*/
void Com_TouchMemory( void ) {
	int start, end;
	int i, j;
	int sum;

#if !defined RTCW_SP
	memblock_t  *block;

	Z_CheckHeap();
#endif // RTCW_XX

	start = Sys_Milliseconds();

	sum = 0;

	j = hunk_low.permanent >> 2;
	for ( i = 0 ; i < j ; i += 64 ) {         // only need to touch each page
		sum += ( (int *)s_hunkData )[i];
	}

	i = ( s_hunkTotal - hunk_high.permanent ) >> 2;
	j = hunk_high.permanent >> 2;
	for (  ; i < j ; i += 64 ) {          // only need to touch each page
		sum += ( (int *)s_hunkData )[i];
	}

#if !defined RTCW_SP
	for ( block = mainzone->blocklist.next ; ; block = block->next ) {
		if ( block->tag ) {
			j = block->size >> 2;
			for ( i = 0 ; i < j ; i += 64 ) {             // only need to touch each page
				sum += ( (int *)block )[i];
			}
		}
		if ( block->next == &mainzone->blocklist ) {
			break;          // all blocks have been hit
		}
	}
#endif // RTCW_XX

	end = Sys_Milliseconds();

	Com_Printf( "Com_TouchMemory: %i msec\n", end - start );
}



#if !defined RTCW_SP
/*
=================
Com_InitZoneMemory
=================
*/
void Com_InitSmallZoneMemory( void ) {
	s_smallZoneTotal = 512 * 1024;
	// bk001205 - was malloc
	smallzone = static_cast<memzone_t*> (calloc( s_smallZoneTotal, 1 ));
	if ( !smallzone ) {
		Com_Error( ERR_FATAL, "Small zone data failed to allocate %1.1f megs", (float)s_smallZoneTotal / ( 1024 * 1024 ) );
	}
	Z_ClearZone( smallzone, s_smallZoneTotal );

	return;
}

/*
void Com_InitZoneMemory( void ) {
	cvar_t	*cv;
	s_smallZoneTotal = 512 * 1024;
	smallzone = malloc( s_smallZoneTotal );
	if ( !smallzone ) {
		Com_Error( ERR_FATAL, "Small zone data failed to allocate %1.1f megs", (float)s_smallZoneTotal / (1024*1024) );
	}
	Z_ClearZone( smallzone, s_smallZoneTotal );

	// allocate the random block zone
	cv = Cvar_Get( "com_zoneMegs", DEF_COMZONEMEGS, CVAR_LATCH | CVAR_ARCHIVE );

	if ( cv->integer < 16 ) {
		s_zoneTotal = 1024 * 1024 * 16;
	} else {
		s_zoneTotal = cv->integer * 1024 * 1024;
	}

	mainzone = malloc( s_zoneTotal );
	if ( !mainzone ) {
		Com_Error( ERR_FATAL, "Zone data failed to allocate %i megs", s_zoneTotal / (1024*1024) );
	}
	Z_ClearZone( mainzone, s_zoneTotal );
}
*/
#endif // RTCW_XX

void Com_InitZoneMemory( void ) {

#if defined RTCW_SP
	//memset(g_taggedAllocations, 0, sizeof(g_taggedAllocations));
	//g_numTaggedAllocs = 0;
#else
	cvar_t  *cv;
	// allocate the random block zone
	cv = Cvar_Get( "com_zoneMegs", DEF_COMZONEMEGS, CVAR_LATCH | CVAR_ARCHIVE );

	if ( cv->integer < 16 ) {
		s_zoneTotal = 1024 * 1024 * 16;
	} else
	{
		s_zoneTotal = cv->integer * 1024 * 1024;
	}

	// bk001205 - was malloc
	mainzone = static_cast<memzone_t*> (calloc( s_zoneTotal, 1 ));
	if ( !mainzone ) {
		Com_Error( ERR_FATAL, "Zone data failed to allocate %i megs", s_zoneTotal / ( 1024 * 1024 ) );
	}
	Z_ClearZone( mainzone, s_zoneTotal );
#endif // RTCW_XX

}

/*
=================
Hunk_Log
=================
*/
void Hunk_Log( void ) {
	hunkblock_t *block;
	char buf[4096];
	int size, numBlocks;

	if ( !logfile || !FS_Initialized() ) {
		return;
	}
	size = 0;
	numBlocks = 0;
	Com_sprintf( buf, sizeof( buf ), "\r\n================\r\nHunk log\r\n================\r\n" );
	FS_Write( buf, strlen( buf ), logfile );
	for ( block = hunkblocks ; block; block = block->next ) {
#ifdef HUNK_DEBUG
		Com_sprintf( buf, sizeof( buf ), "size = %8d: %s, line: %d (%s)\r\n", block->size, block->file, block->line, block->label );
		FS_Write( buf, strlen( buf ), logfile );
#endif
		size += block->size;
		numBlocks++;
	}
	Com_sprintf( buf, sizeof( buf ), "%d Hunk memory\r\n", size );
	FS_Write( buf, strlen( buf ), logfile );
	Com_sprintf( buf, sizeof( buf ), "%d hunk blocks\r\n", numBlocks );
	FS_Write( buf, strlen( buf ), logfile );
}

/*
=================
Hunk_SmallLog
=================
*/
void Hunk_SmallLog( void ) {
	hunkblock_t *block, *block2;
	char buf[4096];
	int size, locsize, numBlocks;

	if ( !logfile || !FS_Initialized() ) {
		return;
	}
	for ( block = hunkblocks ; block; block = block->next ) {
		block->printed = qfalse;
	}
	size = 0;
	numBlocks = 0;
	Com_sprintf( buf, sizeof( buf ), "\r\n================\r\nHunk Small log\r\n================\r\n" );
	FS_Write( buf, strlen( buf ), logfile );
	for ( block = hunkblocks; block; block = block->next ) {
		if ( block->printed ) {
			continue;
		}
		locsize = block->size;
		for ( block2 = block->next; block2; block2 = block2->next ) {
			if ( block->line != block2->line ) {
				continue;
			}
			if ( Q_stricmp( block->file, block2->file ) ) {
				continue;
			}
			size += block2->size;
			locsize += block2->size;
			block2->printed = qtrue;
		}
#ifdef HUNK_DEBUG

#if !defined RTCW_ET
		Com_sprintf( buf, sizeof( buf ), "size = %8d: %s, line: %d (%s)\r\n", locsize, block->file, block->line, block->label );
#else
		Com_sprintf( buf, sizeof( buf ), "size = %8d (%6.2f MB / %6.2f MB): %s, line: %d (%s)\r\n", locsize, locsize / Square( 1024.f ), ( size + block->size ) / Square( 1024.f ), block->file, block->line, block->label );
#endif // RTCW_XX

		FS_Write( buf, strlen( buf ), logfile );
#endif
		size += block->size;
		numBlocks++;
	}
	Com_sprintf( buf, sizeof( buf ), "%d Hunk memory\r\n", size );
	FS_Write( buf, strlen( buf ), logfile );
	Com_sprintf( buf, sizeof( buf ), "%d hunk blocks\r\n", numBlocks );
	FS_Write( buf, strlen( buf ), logfile );
}

/*
=================
Com_InitZoneMemory
=================
*/
void Com_InitHunkMemory( void ) {
	cvar_t  *cv;
	int nMinAlloc;
	const char *pMsg = NULL;

	// make sure the file system has allocated and "not" freed any temp blocks
	// this allows the config and product id files ( journal files too ) to be loaded
	// by the file system without redunant routines in the file system utilizing different
	// memory systems
	if ( FS_LoadStack() != 0 ) {
		Com_Error( ERR_FATAL, "Hunk initialization failed. File system load stack not zero" );
	}

	// allocate the stack based hunk allocator
	cv = Cvar_Get( "com_hunkMegs", DEF_COMHUNKMEGS, CVAR_LATCH | CVAR_ARCHIVE );

	// if we are not dedicated min allocation is 56, otherwise min is 1
	if ( com_dedicated && com_dedicated->integer ) {
		nMinAlloc = MIN_DEDICATED_COMHUNKMEGS;
		pMsg = "Minimum com_hunkMegs for a dedicated server is %i, allocating %i megs.\n";
	} else {
		nMinAlloc = MIN_COMHUNKMEGS;
		pMsg = "Minimum com_hunkMegs is %i, allocating %i megs.\n";
	}

	if ( cv->integer < nMinAlloc ) {
		s_hunkTotal = 1024 * 1024 * nMinAlloc;
		Com_Printf( pMsg, nMinAlloc, s_hunkTotal / ( 1024 * 1024 ) );
	} else {
		s_hunkTotal = cv->integer * 1024 * 1024;
	}

	s_hunkData = static_cast<byte*> (malloc( s_hunkTotal + 31 ));

	if ( !s_hunkData ) {
		Com_Error( ERR_FATAL, "Hunk data failed to allocate %i megs", s_hunkTotal / ( 1024 * 1024 ) );
	}

	// cacheline align
#ifdef RTCW_VANILLA
	s_hunkData = ( byte * )( ( (int)s_hunkData + 31 ) & ~31 );
#else // RTCW_VANILLA
	s_hunkData = reinterpret_cast<byte*>((reinterpret_cast<size_t>(s_hunkData) + 31) & ~(size_t(31)));
#endif // RTCW_VANILLA

	Hunk_Clear();

	Cmd_AddCommand( "meminfo", Com_Meminfo_f );

#if !defined RTCW_SP
#ifdef ZONE_DEBUG
	Cmd_AddCommand( "zonelog", Z_LogHeap );
#endif
#endif // RTCW_XX

#ifdef HUNK_DEBUG
	Cmd_AddCommand( "hunklog", Hunk_Log );
	Cmd_AddCommand( "hunksmalllog", Hunk_SmallLog );
#endif
}

/*
====================
Hunk_MemoryRemaining
====================
*/
int Hunk_MemoryRemaining( void ) {
	int low, high;

	low = hunk_low.permanent > hunk_low.temp ? hunk_low.permanent : hunk_low.temp;
	high = hunk_high.permanent > hunk_high.temp ? hunk_high.permanent : hunk_high.temp;

	return s_hunkTotal - ( low + high );
}

/*
===================
Hunk_SetMark

The server calls this after the level and game VM have been loaded
===================
*/
void Hunk_SetMark( void ) {
	hunk_low.mark = hunk_low.permanent;
	hunk_high.mark = hunk_high.permanent;
}

/*
=================
Hunk_ClearToMark

The client calls this before starting a vid_restart or snd_restart
=================
*/
void Hunk_ClearToMark( void ) {
	hunk_low.permanent = hunk_low.temp = hunk_low.mark;
	hunk_high.permanent = hunk_high.temp = hunk_high.mark;
}

/*
=================
Hunk_CheckMark
=================
*/
qboolean Hunk_CheckMark( void ) {
	if ( hunk_low.mark || hunk_high.mark ) {
		return qtrue;
	}
	return qfalse;
}

#if !defined RTCW_ET
void CL_ShutdownCGame( void );
#endif // RTCW_XX

void CL_ShutdownUI( void );
void SV_ShutdownGameProgs( void );

/*
=================
Hunk_Clear

The server calls this before shutting down or loading a new map
=================
*/
void Hunk_Clear( void ) {

#ifndef DEDICATED
	CL_ShutdownCGame();
	CL_ShutdownUI();
#endif
	SV_ShutdownGameProgs();
#ifndef DEDICATED
	CIN_CloseAllVideos();
#endif
	hunk_low.mark = 0;
	hunk_low.permanent = 0;
	hunk_low.temp = 0;
	hunk_low.tempHighwater = 0;

	hunk_high.mark = 0;
	hunk_high.permanent = 0;
	hunk_high.temp = 0;
	hunk_high.tempHighwater = 0;

	hunk_permanent = &hunk_low;
	hunk_temp = &hunk_high;

	Cvar_Set( "com_hunkused", va( "%i", hunk_low.permanent + hunk_high.permanent ) );

#if defined RTCW_ET
	com_hunkusedvalue = hunk_low.permanent + hunk_high.permanent;
#endif // RTCW_XX

	Com_Printf( "Hunk_Clear: reset the hunk ok\n" );
	VM_Clear(); // (SA) FIXME:TODO: was commented out in wolf
#ifdef HUNK_DEBUG
	hunkblocks = NULL;
#endif
}

static void Hunk_SwapBanks( void ) {
	hunkUsed_t  *swap;

	// can't swap banks if there is any temp already allocated
	if ( hunk_temp->temp != hunk_temp->permanent ) {
		return;
	}

	// if we have a larger highwater mark on this side, start making
	// our permanent allocations here and use the other side for temp
	if ( hunk_temp->tempHighwater - hunk_temp->permanent >
		 hunk_permanent->tempHighwater - hunk_permanent->permanent ) {
		swap = hunk_temp;
		hunk_temp = hunk_permanent;
		hunk_permanent = swap;
	}
}

/*
=================
Hunk_Alloc

Allocate permanent (until the hunk is cleared) memory
=================
*/
#ifdef HUNK_DEBUG
void *Hunk_AllocDebug( int size, ha_pref preference, char *label, char *file, int line ) {
#else
void *Hunk_Alloc( int size, ha_pref preference ) {
#endif
	void    *buf;

	if ( s_hunkData == NULL ) {
		Com_Error( ERR_FATAL, "Hunk_Alloc: Hunk memory system not initialized" );
	}

	Hunk_SwapBanks();

#ifdef HUNK_DEBUG
	size += sizeof( hunkblock_t );
#endif

	// round to cacheline
	size = ( size + 31 ) & ~31;

	if ( hunk_low.temp + hunk_high.temp + size > s_hunkTotal ) {
#ifdef HUNK_DEBUG
		Hunk_Log();
		Hunk_SmallLog();
#endif
		Com_Error( ERR_DROP, "Hunk_Alloc failed on %i", size );
	}

	if ( hunk_permanent == &hunk_low ) {
		buf = ( void * )( s_hunkData + hunk_permanent->permanent );
		hunk_permanent->permanent += size;
	} else {
		hunk_permanent->permanent += size;
		buf = ( void * )( s_hunkData + s_hunkTotal - hunk_permanent->permanent );
	}

	hunk_permanent->temp = hunk_permanent->permanent;

	memset( buf, 0, size );

#ifdef HUNK_DEBUG
	{
		hunkblock_t *block;

		block = (hunkblock_t *) buf;
		block->size = size - sizeof( hunkblock_t );
		block->file = file;
		block->label = label;
		block->line = line;
		block->next = hunkblocks;
		hunkblocks = block;
		buf = ( (byte *) buf ) + sizeof( hunkblock_t );
	}
#endif
	// Ridah, update the com_hunkused cvar in increments, so we don't update it too often, since this cvar call isn't very efficent

#if !defined RTCW_ET
	if ( ( hunk_low.permanent + hunk_high.permanent ) > com_hunkused->integer + 10000 ) {
#else
	if ( ( hunk_low.permanent + hunk_high.permanent ) > com_hunkused->integer + 2500 ) {
#endif // RTCW_XX

		Cvar_Set( "com_hunkused", va( "%i", hunk_low.permanent + hunk_high.permanent ) );
	}

#if defined RTCW_ET
	com_hunkusedvalue = hunk_low.permanent + hunk_high.permanent;
#endif // RTCW_XX

	return buf;
}

/*
=================
Hunk_AllocateTempMemory

This is used by the file loading system.
Multiple files can be loaded in temporary memory.
When the files-in-use count reaches zero, all temp memory will be deleted
=================
*/
void *Hunk_AllocateTempMemory( int size ) {
	void        *buf;
	hunkHeader_t    *hdr;

	// return a Z_Malloc'd block if the hunk has not been initialized
	// this allows the config and product id files ( journal files too ) to be loaded
	// by the file system without redunant routines in the file system utilizing different
	// memory systems
	if ( s_hunkData == NULL ) {
		return Z_Malloc( size );
	}

	Hunk_SwapBanks();

	size = ( ( size + 3 ) & ~3 ) + sizeof( hunkHeader_t );

	if ( hunk_temp->temp + hunk_permanent->permanent + size > s_hunkTotal ) {

#if defined RTCW_ET
#ifdef HUNK_DEBUG
		Hunk_Log();
		Hunk_SmallLog();
#endif
#endif // RCTW_XX

		Com_Error( ERR_DROP, "Hunk_AllocateTempMemory: failed on %i", size );
	}

	if ( hunk_temp == &hunk_low ) {
		buf = ( void * )( s_hunkData + hunk_temp->temp );
		hunk_temp->temp += size;
	} else {
		hunk_temp->temp += size;
		buf = ( void * )( s_hunkData + s_hunkTotal - hunk_temp->temp );
	}

	if ( hunk_temp->temp > hunk_temp->tempHighwater ) {
		hunk_temp->tempHighwater = hunk_temp->temp;
	}

	hdr = (hunkHeader_t *)buf;
	buf = ( void * )( hdr + 1 );

	hdr->magic = HUNK_MAGIC;
	hdr->size = size;

	// don't bother clearing, because we are going to load a file over it
	return buf;
}


/*
==================
Hunk_FreeTempMemory
==================
*/
void Hunk_FreeTempMemory( void *buf ) {
	hunkHeader_t    *hdr;

	// free with Z_Free if the hunk has not been initialized
	// this allows the config and product id files ( journal files too ) to be loaded
	// by the file system without redunant routines in the file system utilizing different
	// memory systems
	if ( s_hunkData == NULL ) {
		Z_Free( buf );
		return;
	}


	hdr = ( (hunkHeader_t *)buf ) - 1;
	if ( hdr->magic != HUNK_MAGIC ) {
		Com_Error( ERR_FATAL, "Hunk_FreeTempMemory: bad magic" );
	}

	hdr->magic = HUNK_FREE_MAGIC;

	// this only works if the files are freed in stack order,
	// otherwise the memory will stay around until Hunk_ClearTempMemory
	if ( hunk_temp == &hunk_low ) {
		if ( hdr == ( void * )( s_hunkData + hunk_temp->temp - hdr->size ) ) {
			hunk_temp->temp -= hdr->size;
		} else {
			Com_Printf( "Hunk_FreeTempMemory: not the final block\n" );
		}
	} else {
		if ( hdr == ( void * )( s_hunkData + s_hunkTotal - hunk_temp->temp ) ) {
			hunk_temp->temp -= hdr->size;
		} else {
			Com_Printf( "Hunk_FreeTempMemory: not the final block\n" );
		}
	}
}


/*
=================
Hunk_ClearTempMemory

The temp space is no longer needed.  If we have left more
touched but unused memory on this side, have future
permanent allocs use this side.
=================
*/
void Hunk_ClearTempMemory( void ) {
	if ( s_hunkData != NULL ) {
		hunk_temp->temp = hunk_temp->permanent;
	}
}

/*
=================
Hunk_Trash
=================
*/
#ifdef RTCW_VANILLA
void Hunk_Trash( void ) {

#if defined RTCW_ET
	return;
#endif // RTCW_XX

#if !defined RTCW_ET
	int length, i, rnd;
	char *buf, value;

	return;

	if ( s_hunkData == NULL ) {
		return;
	}

#ifdef _DEBUG
	Com_Error( ERR_DROP, "hunk trashed\n" );
	return;
#endif

	Cvar_Set( "com_jp", "1" );
	Hunk_SwapBanks();

	if ( hunk_permanent == &hunk_low ) {
		buf = reinterpret_cast<char*> ( s_hunkData + hunk_permanent->permanent );
	} else {
		buf = reinterpret_cast<char*> ( s_hunkData + s_hunkTotal - hunk_permanent->permanent );
	}
	length = hunk_permanent->permanent;

	if ( length > 0x7FFFF ) {
		//randomly trash data within buf
		rnd = random() * ( length - 0x7FFFF );
		value = 31;
		for ( i = 0; i < 0x7FFFF; i++ ) {
			value *= 109;
			buf[rnd + i] ^= value;
		}
	}
#else
#if 0
	int length, i, rnd;
	char *buf, value;

	if ( s_hunkData == NULL ) {
		return;
	}

#ifdef _DEBUG
	Com_Error( ERR_DROP, "hunk trashed\n" );
	return;
#endif

	Cvar_Set( "com_jp", "1" );
	Hunk_SwapBanks();

	if ( hunk_permanent == &hunk_low ) {
		buf = ( void * )( s_hunkData + hunk_permanent->permanent );
	} else {
		buf = ( void * )( s_hunkData + s_hunkTotal - hunk_permanent->permanent );
	}
	length = hunk_permanent->permanent;

	if ( length > 0x7FFFF ) {
		//randomly trash data within buf
		rnd = random() * ( length - 0x7FFFF );
		value = 31;
		for ( i = 0; i < 0x7FFFF; i++ ) {
			value *= 109;
			buf[rnd + i] ^= value;
		}
	}
#endif // 0
#endif // RTCW_XX

}
#endif // RTCW_VANILLA

/*
===================================================================

EVENTS AND JOURNALING

In addition to these events, .cfg files are also copied to the
journaled file
===================================================================
*/

// bk001129 - here we go again: upped from 64
#define MAX_PUSHED_EVENTS               256
// bk001129 - init, also static
static int com_pushedEventsHead = 0;
static int com_pushedEventsTail = 0;
// bk001129 - static
static sysEvent_t com_pushedEvents[MAX_PUSHED_EVENTS];

/*
=================
Com_InitJournaling
=================
*/
void Com_InitJournaling( void ) {
	Com_StartupVariable( "journal" );
	com_journal = Cvar_Get( "journal", "0", CVAR_INIT );
	if ( !com_journal->integer ) {
		return;
	}

	if ( com_journal->integer == 1 ) {

#if !defined RTCW_MP
		Com_Printf( "Journaling events\n" );
#endif // RTCW_XX

#if !defined RTCW_ET
#if defined RTCW_MP
		Com_Printf( "Journaling events\n" );
#endif // RTCW_XX
#endif // RTCW_XX

		com_journalFile = FS_FOpenFileWrite( "journal.dat" );
		com_journalDataFile = FS_FOpenFileWrite( "journaldata.dat" );
	} else if ( com_journal->integer == 2 ) {
		Com_Printf( "Replaying journaled events\n" );
		FS_FOpenFileRead( "journal.dat", &com_journalFile, qtrue );
		FS_FOpenFileRead( "journaldata.dat", &com_journalDataFile, qtrue );
	}

	if ( !com_journalFile || !com_journalDataFile ) {
		Cvar_Set( "com_journal", "0" );
		com_journalFile = 0;
		com_journalDataFile = 0;
		Com_Printf( "Couldn't open journal files\n" );
	}
}

/*
=================
Com_GetRealEvent
=================
*/
sysEvent_t  Com_GetRealEvent( void ) {
	int r;
	sysEvent_t ev;

	// either get an event from the system or the journal file
	if ( com_journal->integer == 2 ) {
		r = FS_Read( &ev, sizeof( ev ), com_journalFile );
		if ( r != sizeof( ev ) ) {
			Com_Error( ERR_FATAL, "Error reading from journal file" );
		}
		if ( ev.evPtrLength ) {
			ev.evPtr = Z_Malloc( ev.evPtrLength );
			r = FS_Read( ev.evPtr, ev.evPtrLength, com_journalFile );
			if ( r != ev.evPtrLength ) {
				Com_Error( ERR_FATAL, "Error reading from journal file" );
			}
		}
	} else {
		ev = Sys_GetEvent();

		// write the journal value out if needed
		if ( com_journal->integer == 1 ) {
			r = FS_Write( &ev, sizeof( ev ), com_journalFile );
			if ( r != sizeof( ev ) ) {
				Com_Error( ERR_FATAL, "Error writing to journal file" );
			}
			if ( ev.evPtrLength ) {
				r = FS_Write( ev.evPtr, ev.evPtrLength, com_journalFile );
				if ( r != ev.evPtrLength ) {
					Com_Error( ERR_FATAL, "Error writing to journal file" );
				}
			}
		}
	}

	return ev;
}


/*
=================
Com_InitPushEvent
=================
*/
// bk001129 - added
void Com_InitPushEvent( void ) {
	// clear the static buffer array
	// this requires SE_NONE to be accepted as a valid but NOP event
	memset( com_pushedEvents, 0, sizeof( com_pushedEvents ) );
	// reset counters while we are at it
	// beware: GetEvent might still return an SE_NONE from the buffer
	com_pushedEventsHead = 0;
	com_pushedEventsTail = 0;
}


/*
=================
Com_PushEvent
=================
*/
void Com_PushEvent( sysEvent_t *event ) {
	sysEvent_t      *ev;
	static int printedWarning = 0; // bk001129 - init, bk001204 - explicit int

	ev = &com_pushedEvents[ com_pushedEventsHead & ( MAX_PUSHED_EVENTS - 1 ) ];

	if ( com_pushedEventsHead - com_pushedEventsTail >= MAX_PUSHED_EVENTS ) {

		// don't print the warning constantly, or it can give time for more...
		if ( !printedWarning ) {
			printedWarning = qtrue;
			Com_Printf( "WARNING: Com_PushEvent overflow\n" );
		}

		if ( ev->evPtr ) {
			Z_Free( ev->evPtr );
		}
		com_pushedEventsTail++;
	} else {
		printedWarning = qfalse;
	}

	*ev = *event;
	com_pushedEventsHead++;
}

/*
=================
Com_GetEvent
=================
*/
sysEvent_t  Com_GetEvent( void ) {
	if ( com_pushedEventsHead > com_pushedEventsTail ) {
		com_pushedEventsTail++;
		return com_pushedEvents[ ( com_pushedEventsTail - 1 ) & ( MAX_PUSHED_EVENTS - 1 ) ];
	}
	return Com_GetRealEvent();
}

/*
=================
Com_RunAndTimeServerPacket
=================
*/
void Com_RunAndTimeServerPacket( netadr_t *evFrom, msg_t *buf ) {
	int t1, t2, msec;

	t1 = 0;

	if ( com_speeds->integer ) {
		t1 = Sys_Milliseconds();
	}

	SV_PacketEvent( *evFrom, buf );

	if ( com_speeds->integer ) {
		t2 = Sys_Milliseconds();
		msec = t2 - t1;
		if ( com_speeds->integer == 3 ) {
			Com_Printf( "SV_PacketEvent time: %i\n", msec );
		}
	}
}

/*
=================
Com_EventLoop

Returns last event time
=================
*/

#if defined RTCW_ET
#ifndef DEDICATED
extern qboolean consoleButtonWasPressed;
#endif
#endif // RTCW_XX

int Com_EventLoop( void ) {
	// BBi
	SDL_PumpEvents();
	// BBi

	sysEvent_t ev;
	netadr_t evFrom;
	byte bufData[MAX_MSGLEN];
	msg_t buf;

	MSG_Init( &buf, bufData, sizeof( bufData ) );

	while ( 1 ) {
		ev = Com_GetEvent();

		// if no more events are available
		if ( ev.evType == SE_NONE ) {
			// manually send packet events for the loopback channel
			while ( NET_GetLoopPacket( NS_CLIENT, &evFrom, &buf ) ) {
				CL_PacketEvent( evFrom, &buf );
			}

			while ( NET_GetLoopPacket( NS_SERVER, &evFrom, &buf ) ) {
				// if the server just shut down, flush the events
				if ( com_sv_running->integer ) {
					Com_RunAndTimeServerPacket( &evFrom, &buf );
				}
			}

			return ev.evTime;
		}


		switch ( ev.evType ) {
		default:
			// bk001129 - was ev.evTime
			Com_Error( ERR_FATAL, "Com_EventLoop: bad event type %i", ev.evType );
			break;
		case SE_NONE:
			break;
		case SE_KEY:
			CL_KeyEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_CHAR:

#if defined RTCW_ET
#ifndef DEDICATED
			// fretn
			// we just pressed the console button,
			// so ignore this event
			// this prevents chars appearing at console input
			// when you just opened it
			if ( consoleButtonWasPressed ) {
				consoleButtonWasPressed = qfalse;
				break;
			}
#endif
#endif // RTCW_XX

			CL_CharEvent( ev.evValue );
			break;
		case SE_MOUSE:
			CL_MouseEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_JOYSTICK_AXIS:
			CL_JoystickEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_CONSOLE:
			Cbuf_AddText( (char *)ev.evPtr );
			Cbuf_AddText( "\n" );
			break;
		case SE_PACKET:
			// this cvar allows simulation of connections that
			// drop a lot of packets.  Note that loopback connections
			// don't go through here at all.
			if ( com_dropsim->value > 0 ) {
				static int seed;

				if ( Q_random( &seed ) < com_dropsim->value ) {
					break;      // drop this packet
				}
			}

			evFrom = *(netadr_t *)ev.evPtr;
			buf.cursize = ev.evPtrLength - sizeof( evFrom );

			// we must copy the contents of the message out, because
			// the event buffers are only large enough to hold the
			// exact payload, but channel messages need to be large
			// enough to hold fragment reassembly
			if ( (unsigned)buf.cursize > buf.maxsize ) {
				Com_Printf( "Com_EventLoop: oversize packet\n" );
				continue;
			}
			memcpy( buf.data, ( byte * )( (netadr_t *)ev.evPtr + 1 ), buf.cursize );
			if ( com_sv_running->integer ) {
				Com_RunAndTimeServerPacket( &evFrom, &buf );
			} else {
				CL_PacketEvent( evFrom, &buf );
			}
			break;
		}

		// free any block data
		if ( ev.evPtr ) {
			Z_Free( ev.evPtr );
		}
	}

	return 0;   // never reached
}

/*
================
Com_Milliseconds

Can be used for profiling, but will be journaled accurately
================
*/
int Com_Milliseconds( void ) {
	sysEvent_t ev;

	// get events and push them until we get a null event with the current time
	do {

		ev = Com_GetRealEvent();
		if ( ev.evType != SE_NONE ) {
			Com_PushEvent( &ev );
		}
	} while ( ev.evType != SE_NONE );

	return ev.evTime;
}

//============================================================================

/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
static void Com_Error_f( void ) {
	if ( Cmd_Argc() > 1 ) {
		Com_Error( ERR_DROP, "Testing drop error" );
	} else {
		Com_Error( ERR_FATAL, "Testing fatal error" );
	}
}


/*
=============
Com_Freeze_f

Just freeze in place for a given number of seconds to test
error recovery
=============
*/
static void Com_Freeze_f( void ) {
	float s;
	int start, now;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "freeze <seconds>\n" );
		return;
	}
	s = atof( Cmd_Argv( 1 ) );

	start = Com_Milliseconds();

	while ( 1 ) {
		now = Com_Milliseconds();
		if ( ( now - start ) * 0.001 > s ) {
			break;
		}
	}
}

/*
=================
Com_Crash_f

A way to force a bus error for development reasons
=================
*/
static void Com_Crash_f( void ) {
#ifdef RTCW_VANILLA
	*( int * ) 0 = 0x12345678;
#else // RTCW_VANILLA
	*static_cast<size_t*>(NULL) = 0x12345678;
#endif // RTCW_VANILLA
}

#if defined RTCW_ET
void Com_CPUSpeed_f()
{
	Com_Printf(S_COLOR_YELLOW "Command %s deprecated.\n", "cpuspeed");
}
#endif // RTCW_XX

qboolean CL_CDKeyValidate( const char *key, const char *checksum );

#if !defined RTCW_SP
// TTimo: centralizing the cl_cdkey stuff after I discovered a buffer overflow problem with the dedicated server version
//   not sure it's necessary to have different defaults for regular and dedicated, but I don't want to take the risk
#ifndef DEDICATED
char cl_cdkey[34] = "                                ";
#else
char cl_cdkey[34] = "123456789";
#endif
#endif // RTCW_XX

/*
=================
Com_ReadCDKey
=================
*/
void Com_ReadCDKey( const char *filename ) {
	fileHandle_t f;
	char buffer[33];
	char fbuffer[MAX_OSPATH];

	sprintf( fbuffer, "%s/rtcwkey", filename );

	FS_SV_FOpenFileRead( fbuffer, &f );
	if ( !f ) {
		Q_strncpyz( cl_cdkey, "                ", 17 );
		return;
	}

	Com_Memset( buffer, 0, sizeof( buffer ) );

	FS_Read( buffer, 16, f );
	FS_FCloseFile( f );

	if ( CL_CDKeyValidate( buffer, NULL ) ) {
		Q_strncpyz( cl_cdkey, buffer, 17 );
	} else {
		Q_strncpyz( cl_cdkey, "                ", 17 );
	}
}

/*
=================
Com_ReadCDKey
=================
*/
void Com_AppendCDKey( const char *filename ) {
	fileHandle_t f;
	char buffer[33];
	char fbuffer[MAX_OSPATH];

	sprintf( fbuffer, "%s/rtcwkey", filename );

	FS_SV_FOpenFileRead( fbuffer, &f );
	if ( !f ) {
		Q_strncpyz( &cl_cdkey[16], "                ", 17 );
		return;
	}

	Com_Memset( buffer, 0, sizeof( buffer ) );

	FS_Read( buffer, 16, f );
	FS_FCloseFile( f );

	if ( CL_CDKeyValidate( buffer, NULL ) ) {
		strcat( &cl_cdkey[16], buffer );
	} else {
		Q_strncpyz( &cl_cdkey[16], "                ", 17 );
	}
}

#ifndef DEDICATED // bk001204
/*
=================
Com_WriteCDKey
=================
*/
static void Com_WriteCDKey( const char *filename, const char *ikey ) {
	fileHandle_t f;
	char fbuffer[MAX_OSPATH];
	char key[17];


	sprintf( fbuffer, "%s/rtcwkey", filename );


	Q_strncpyz( key, ikey, 17 );

	if ( !CL_CDKeyValidate( key, NULL ) ) {
		return;
	}

	f = FS_SV_FOpenFileWrite( fbuffer );
	if ( !f ) {
		Com_Printf( "Couldn't write %s.\n", filename );
		return;
	}

	FS_Write( key, 16, f );

#if !defined RTCW_ET
	FS_Printf( f, "\n// generated by RTCW, do not modify\r\n" );
#else
	FS_Printf( f, "\n// generated by ET, do not modify\r\n" );
#endif // RTCW_XX

	FS_Printf( f, "// Do not give this file to ANYONE.\r\n" );
	FS_Printf( f, "// id Software and Activision will NOT ask you to send this file to them.\r\n" );
	FS_FCloseFile( f );
}
#endif

void Com_SetRecommended(bool restart_video)
{
	cvar_t* r_highQualityVideo = NULL;

	r_highQualityVideo = Cvar_Get("r_highQualityVideo", "1", CVAR_ARCHIVE);

#if !defined RTCW_ET
	Cbuf_AddText("exec highVidhighCPU.cfg\n");

	// (SA) set the cvar so the menu will reflect this on first run
	Cvar_Set("ui_glCustom", "999"); // 'recommended'

#if defined RTCW_SP
	if (restart_video)
		Cbuf_AddText("vid_restart\n");
#endif // RTCW_XX
#else
	cvar_t* com_recommended = NULL;

	com_recommended = Cvar_Get("com_recommended", "-1", CVAR_ARCHIVE);

	Cbuf_AddText("exec preset_high.cfg\n");
	Cvar_Set("com_recommended", "0");
#endif // RTCW_XX
}


#if defined RTCW_ET
// Arnout: gameinfo, to let the engine know which gametypes are SP and if we should use profiles.
// This can't be dependant on gamecode as we sometimes need to know about it when no game-modules
// are loaded
gameInfo_t com_gameInfo;

void Com_GetGameInfo() {
	char    *f;
	const char* buf;
	char    *token;

	memset( &com_gameInfo, 0, sizeof( com_gameInfo ) );

	if ( FS_ReadFile( "gameinfo.dat", (void **)&f ) > 0 ) {

		buf = f;

		while ( ( token = COM_Parse( &buf ) ) != NULL && token[0] ) {
			if ( !Q_stricmp( token, "spEnabled" ) ) {
				com_gameInfo.spEnabled = qtrue;
			} else if ( !Q_stricmp( token, "spGameTypes" ) ) {
				while ( ( token = COM_ParseExt( &buf, qfalse ) ) != NULL && token[0] ) {
					com_gameInfo.spGameTypes |= ( 1 << atoi( token ) );
				}
			} else if ( !Q_stricmp( token, "defaultSPGameType" ) ) {
				if ( ( token = COM_ParseExt( &buf, qfalse ) ) != NULL && token[0] ) {
					com_gameInfo.defaultSPGameType = atoi( token );
				} else {
					FS_FreeFile( f );
					Com_Error( ERR_FATAL, "Com_GetGameInfo: bad syntax." );
				}
			} else if ( !Q_stricmp( token, "coopGameTypes" ) ) {

				while ( ( token = COM_ParseExt( &buf, qfalse ) ) != NULL && token[0] ) {
					com_gameInfo.coopGameTypes |= ( 1 << atoi( token ) );
				}
			} else if ( !Q_stricmp( token, "defaultCoopGameType" ) ) {
				if ( ( token = COM_ParseExt( &buf, qfalse ) ) != NULL && token[0] ) {
					com_gameInfo.defaultCoopGameType = atoi( token );
				} else {
					FS_FreeFile( f );
					Com_Error( ERR_FATAL, "Com_GetGameInfo: bad syntax." );
				}
			} else if ( !Q_stricmp( token, "defaultGameType" ) ) {
				if ( ( token = COM_ParseExt( &buf, qfalse ) ) != NULL && token[0] ) {
					com_gameInfo.defaultGameType = atoi( token );
				} else {
					FS_FreeFile( f );
					Com_Error( ERR_FATAL, "Com_GetGameInfo: bad syntax." );
				}
			} else if ( !Q_stricmp( token, "usesProfiles" ) ) {
				if ( ( token = COM_ParseExt( &buf, qfalse ) ) != NULL && token[0] ) {
					com_gameInfo.usesProfiles = atoi( token );
				} else {
					FS_FreeFile( f );
					Com_Error( ERR_FATAL, "Com_GetGameInfo: bad syntax." );
				}
			} else {
				FS_FreeFile( f );
				Com_Error( ERR_FATAL, "Com_GetGameInfo: bad syntax." );
			}
		}

		// all is good
		FS_FreeFile( f );
	}
}

// bani - checks if profile.pid is valid
// return qtrue if it is
// return qfalse if it isn't(!)
qboolean Com_CheckProfile( char *profile_path ) {
	fileHandle_t f;
	char f_data[32];
	int f_pid;

	//let user override this
	if ( com_ignorecrash->integer ) {
		return qtrue;
	}

	if ( FS_FOpenFileRead( profile_path, &f, qtrue ) < 0 ) {
		//no profile found, we're ok
		return qtrue;
	}

	if ( FS_Read( &f_data, sizeof( f_data ) - 1, f ) < 0 ) {
		//b0rk3d!
		FS_FCloseFile( f );
		//try to delete corrupted pid file
		FS_Delete( profile_path );
		return qfalse;
	}

	f_pid = atoi( f_data );
	if ( f_pid != com_pid->integer ) {
		//pid doesn't match
		FS_FCloseFile( f );
		return qfalse;
	}

	//we're all ok
	FS_FCloseFile( f );
	return qtrue;
}

//bani - from files.c
extern char fs_gamedir[MAX_OSPATH];
char last_fs_gamedir[MAX_OSPATH];
char last_profile_path[MAX_OSPATH];

//bani - track profile changes, delete old profile.pid if we change fs_game(dir)
//hackish, we fiddle with fs_gamedir to make FS_* calls work "right"
void Com_TrackProfile( char *profile_path ) {
	char temp_fs_gamedir[MAX_OSPATH];

//	Com_Printf( "Com_TrackProfile: Tracking profile [%s] [%s]\n", fs_gamedir, profile_path );
	//have we changed fs_game(dir)?
	if ( strcmp( last_fs_gamedir, fs_gamedir ) ) {
		if ( strlen( last_fs_gamedir ) && strlen( last_profile_path ) ) {
			//save current fs_gamedir
			Q_strncpyz( temp_fs_gamedir, fs_gamedir, sizeof( temp_fs_gamedir ) );
			//set fs_gamedir temporarily to make FS_* stuff work "right"
			Q_strncpyz( fs_gamedir, last_fs_gamedir, sizeof( fs_gamedir ) );
			if ( FS_FileExists( last_profile_path ) ) {
				Com_Printf( "Com_TrackProfile: Deleting old pid file [%s] [%s]\n", fs_gamedir, last_profile_path );
				FS_Delete( last_profile_path );
			}
			//restore current fs_gamedir
			Q_strncpyz( fs_gamedir, temp_fs_gamedir, sizeof( fs_gamedir ) );
		}
		//and save current vars for future reference
		Q_strncpyz( last_fs_gamedir, fs_gamedir, sizeof( last_fs_gamedir ) );
		Q_strncpyz( last_profile_path, profile_path, sizeof( last_profile_path ) );
	}
}

// bani - writes pid to profile
// returns qtrue if successful
// returns qfalse if not(!!)
qboolean Com_WriteProfile( char *profile_path ) {
	fileHandle_t f;

	if ( FS_FileExists( profile_path ) ) {
		FS_Delete( profile_path );
	}

	f = FS_FOpenFileWrite( profile_path );
	if ( f < 0 ) {
		Com_Printf( "Com_WriteProfile: Can't write %s.\n", profile_path );
		return qfalse;
	}

	FS_Printf( f, "%d", com_pid->integer );

	FS_FCloseFile( f );

	//track profile changes
	Com_TrackProfile( profile_path );

	return qtrue;
}
#endif // RTCW_XX

/*
=================
Com_Init
=================
*/
void Com_Init( char *commandLine ) {
	char    *s;

#if defined RTCW_ET
	int pid;
#endif // RTCW_XX

#if !defined RTCW_SP
	// TTimo gcc warning: variable `safeMode' might be clobbered by `longjmp' or `vfork'
	volatile qboolean safeMode = qtrue;
#endif // RTCW_XX

	// BBi
	//Com_Printf( "%s %s %s\n", Q3_VERSION, CPUSTRING, __DATE__ );
	Com_Printf("%s %s %s\n", RTCW_VERSION, SDL_GetPlatform(), __DATE__);
	// BBi

	if ( ::setjmp( abortframe ) ) {
		Sys_Error( "Error during initialization" );
	}

	// bk001129 - do this before anything else decides to push events
	Com_InitPushEvent();

#if !defined RTCW_SP
	Com_InitSmallZoneMemory();
#endif // RTCW_XX

	Cvar_Init();

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	Com_ParseCommandLine( commandLine );

	// BBi
	//Swap_Init();
	// BBi

	Cbuf_Init();

	Com_InitZoneMemory();
	Cmd_Init();

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

	// get the developer cvar set as early as possible
	Com_StartupVariable( "developer" );

#if defined RTCW_ET
	// bani: init this early
	Com_StartupVariable( "com_ignorecrash" );
	com_ignorecrash = Cvar_Get( "com_ignorecrash", "0", 0 );

	// ydnar: init crashed variable as early as possible
	com_crashed = Cvar_Get( "com_crashed", "0", CVAR_TEMP );

// Add appropriate system function?
#if FIXME
	// bani: init pid
#else
	pid = 0;
#endif // FIXME
	s = va( "%d", pid );
	com_pid = Cvar_Get( "com_pid", s, CVAR_ROM );
#endif // RTCW_XX

	// done early so bind command exists
	CL_InitKeyCommands();

	FS_InitFilesystem();

	Com_InitJournaling();

#if defined RTCW_ET
	Com_GetGameInfo();
#endif // RTCW_XX

#if defined RTCW_SP
	Cbuf_AddText( "exec default.cfg\n" );

	Cbuf_AddText( "exec language.cfg\n" ); //----(SA)	added

	// skip the q3config.cfg if "safe" is on the command line
	if ( !Com_SafeMode() ) {
		Cbuf_AddText( "exec wolfconfig.cfg\n" );
	}

	Cbuf_AddText( "exec autoexec.cfg\n" );
#elif defined RTCW_MP
	// DHM - Nerve
#ifndef UPDATE_SERVER
	Cbuf_AddText( "exec default.cfg\n" );
	Cbuf_AddText( "exec language.cfg\n" );     // NERVE - SMF

	// skip the q3config.cfg if "safe" is on the command line
	if ( !Com_SafeMode() ) {
		safeMode = qfalse;
		Cbuf_AddText( "exec wolfconfig_mp.cfg\n" );
	}

	Cbuf_AddText( "exec autoexec.cfg\n" );
#endif
#endif // RTCW_XX

#if defined RTCW_ET
	Cbuf_AddText( "exec default.cfg\n" );
	Cbuf_AddText( "exec language.cfg\n" );     // NERVE - SMF

	// skip the q3config.cfg if "safe" is on the command line
	if ( !Com_SafeMode() ) {
		const char *cl_profileStr = Cvar_VariableString( "cl_profile" );

		safeMode = qfalse;
		if ( com_gameInfo.usesProfiles ) {
			if ( !cl_profileStr[0] ) {
				char *defaultProfile = NULL;

				FS_ReadFile( "profiles/defaultprofile.dat", (void **)&defaultProfile );

				if ( defaultProfile ) {
					const char *text_p = defaultProfile;
					char *token = COM_Parse( &text_p );

					if ( token && *token ) {
						Cvar_Set( "cl_defaultProfile", token );
						Cvar_Set( "cl_profile", token );
					}

					FS_FreeFile( defaultProfile );

					cl_profileStr = Cvar_VariableString( "cl_defaultProfile" );
				}
			}

			if ( cl_profileStr[0] ) {
				// bani - check existing pid file and make sure it's ok
				if ( !Com_CheckProfile( va( "profiles/%s/profile.pid", cl_profileStr ) ) ) {
#ifndef _DEBUG
					Com_Printf( "^3WARNING: profile.pid found for profile '%s' - system settings will revert to defaults\n", cl_profileStr );
					// ydnar: set crashed state
					Cbuf_AddText( "set com_crashed 1\n" );
#endif
				}

				// bani - write a new one
				if ( !Com_WriteProfile( va( "profiles/%s/profile.pid", cl_profileStr ) ) ) {
					Com_Printf( "^3WARNING: couldn't write profiles/%s/profile.pid\n", cl_profileStr );
				}

				// exec the config
				Cbuf_AddText( va( "exec profiles/%s/%s\n", cl_profileStr, CONFIG_NAME ) );
			}
		} else
		{
			Cbuf_AddText( va( "exec %s\n", CONFIG_NAME ) );
		}
	}

	Cbuf_AddText( "exec autoexec.cfg\n" );

	// ydnar: reset crashed state
	Cbuf_AddText( "set com_crashed 0\n" );

	// execute the queued commands
#endif // RTCW_XX

	Cbuf_Execute();

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

	// get dedicated here for proper hunk megs initialization

#if defined RTCW_SP
#ifdef DEDICATED
	com_dedicated = Cvar_Get( "dedicated", "1", CVAR_ROM );
#else
	com_dedicated = Cvar_Get( "dedicated", "0", CVAR_LATCH );
#endif
#elif defined RTCW_MP
#ifdef UPDATE_SERVER
	com_dedicated = Cvar_Get( "dedicated", "1", CVAR_LATCH );
#elif DEDICATED
	// TTimo: default to internet dedicated, not LAN dedicated
	com_dedicated = Cvar_Get( "dedicated", "2", CVAR_ROM );
#else
	com_dedicated = Cvar_Get( "dedicated", "0", CVAR_LATCH );
#endif
#else
#if DEDICATED
	// TTimo: default to internet dedicated, not LAN dedicated
	com_dedicated = Cvar_Get( "dedicated", "2", CVAR_ROM );
#else
	com_dedicated = Cvar_Get( "dedicated", "0", CVAR_LATCH );
#endif
#endif // RTCW_XX

	// allocate the stack based hunk allocator
	Com_InitHunkMemory();

	// if any archived cvars are modified after this, we will trigger a writing
	// of the config file
	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

	//
	// init commands and vars
	//

#if defined RTCW_SP
	com_maxfps = Cvar_Get( "com_maxfps", "85", CVAR_ARCHIVE );
#elif defined RTCW_MP
	com_maxfps = Cvar_Get( "com_maxfps", "85", CVAR_ARCHIVE | CVAR_LATCH );
#else
	// Gordon: no need to latch this in ET, our recoil is framerate independant
	com_maxfps = Cvar_Get( "com_maxfps", "85", CVAR_ARCHIVE /*|CVAR_LATCH*/ );
#endif // RTCW_XX

#if !defined RTCW_ET
	com_blood = Cvar_Get( "com_blood", "1", CVAR_ARCHIVE );
#else
//	com_blood = Cvar_Get ("com_blood", "1", CVAR_ARCHIVE); // Gordon: no longer used?
#endif // RTCW_XX

	com_developer = Cvar_Get( "developer", "0", CVAR_TEMP );
	com_logfile = Cvar_Get( "logfile", "0", CVAR_TEMP );

	com_timescale = Cvar_Get( "timescale", "1", CVAR_CHEAT | CVAR_SYSTEMINFO );
	com_fixedtime = Cvar_Get( "fixedtime", "0", CVAR_CHEAT );
	com_showtrace = Cvar_Get( "com_showtrace", "0", CVAR_CHEAT );
	com_dropsim = Cvar_Get( "com_dropsim", "0", CVAR_CHEAT );
	com_viewlog = Cvar_Get( "viewlog", "0", CVAR_CHEAT );
	com_speeds = Cvar_Get( "com_speeds", "0", 0 );
	com_timedemo = Cvar_Get( "timedemo", "0", CVAR_CHEAT );
	com_cameraMode = Cvar_Get( "com_cameraMode", "0", CVAR_CHEAT );

#if defined RTCW_ET
	com_watchdog = Cvar_Get( "com_watchdog", "60", CVAR_ARCHIVE );
	com_watchdog_cmd = Cvar_Get( "com_watchdog_cmd", "", CVAR_ARCHIVE );
#endif // RTCW_XX

	cl_paused = Cvar_Get( "cl_paused", "0", CVAR_ROM );
	sv_paused = Cvar_Get( "sv_paused", "0", CVAR_ROM );
	com_sv_running = Cvar_Get( "sv_running", "0", CVAR_ROM );
	com_cl_running = Cvar_Get( "cl_running", "0", CVAR_ROM );
	com_buildScript = Cvar_Get( "com_buildScript", "0", 0 );

#if defined RTCW_ET
	con_drawnotify = Cvar_Get( "con_drawnotify", "0", CVAR_CHEAT );
#endif // RTCW_XX

	com_introPlayed = Cvar_Get( "com_introplayed", "0", CVAR_ARCHIVE );

#if defined RTCW_ET
	com_logosPlaying = Cvar_Get( "com_logosPlaying", "0", CVAR_ROM );
#endif // RTCW_XX

	com_recommendedSet = Cvar_Get( "com_recommendedSet", "0", CVAR_ARCHIVE );

#if !defined RTCW_MP
	Cvar_Get( "savegame_loading", "0", CVAR_ROM );
#endif // RTCW_XX

#if _WIN32 && _DEBUG
	com_noErrorInterrupt = Cvar_Get( "com_noErrorInterrupt", "0", 0 );
#endif

	com_hunkused = Cvar_Get( "com_hunkused", "0", 0 );

#if defined RTCW_ET
	com_hunkusedvalue = 0;
#endif // RTCW_XX

	if ( com_dedicated->integer ) {
		if ( !com_viewlog->integer ) {
			Cvar_Set( "viewlog", "1" );
		}
	}

	if ( com_developer && com_developer->integer ) {
		Cmd_AddCommand( "error", Com_Error_f );
		Cmd_AddCommand( "crash", Com_Crash_f );
		Cmd_AddCommand( "freeze", Com_Freeze_f );

#if defined RTCW_ET
		Cmd_AddCommand( "cpuspeed", Com_CPUSpeed_f );
#endif // RTCW_XX

	}
	Cmd_AddCommand( "quit", Com_Quit_f );
	Cmd_AddCommand( "changeVectors", MSG_ReportChangeVectors_f );
	Cmd_AddCommand( "writeconfig", Com_WriteConfig_f );

	// BBi
	//s = va( "%s %s %s", Q3_VERSION, CPUSTRING, __DATE__ );
	s = va("%s %s %s", RTCW_VERSION, SDL_GetPlatform(), __DATE__);
	// BBi

	com_version = Cvar_Get( "version", s, CVAR_ROM | CVAR_SERVERINFO );

	Sys_Init();
	Netchan_Init( Com_Milliseconds() & 0xffff );    // pick a port value that should be nice and random
	VM_Init();
	SV_Init();

	com_dedicated->modified = qfalse;
	if ( !com_dedicated->integer ) {
		CL_Init();
		Sys_ShowConsole( com_viewlog->integer, qfalse );
	}

	// set com_frameTime so that if a map is started on the
	// command line it will still be able to count on com_frameTime
	// being random enough for a serverid
	com_frameTime = Com_Milliseconds();

	// add + commands from command line
	if ( !Com_AddStartupCommands() ) {
		// if the user didn't give any commands, run default action
	}

	// start in full screen ui mode
	Cvar_Set( "r_uiFullScreen", "1" );

	CL_StartHunkUsers();

	// delay this so potential wicked3d dll can find a wolf window
	if ( !com_dedicated->integer ) {
		Sys_ShowConsole( com_viewlog->integer, qfalse );
	}

#if defined RTCW_SP
	if ( !com_recommendedSet->integer ) {
		Com_SetRecommended( qtrue );
		Cvar_Set( "com_recommendedSet", "1" );
	}

	if ( !com_dedicated->integer ) {
		//Cbuf_AddText ("cinematic gmlogo.RoQ\n");
		if ( !com_introPlayed->integer ) {
			//Cvar_Set( com_introPlayed->name, "1" );		//----(SA)	force this to get played every time (but leave cvar for override)
			Cbuf_AddText( "cinematic wolfintro.RoQ 3\n" );
			//Cvar_Set( "nextmap", "cinematic wolfintro.RoQ" );
		}
	}
#else
	// NERVE - SMF - force recommendedSet and don't do vid_restart if in safe mode
	if ( !com_recommendedSet->integer && !safeMode ) {
		Com_SetRecommended(false);
		Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
	}
	Cvar_Set( "com_recommendedSet", "1" );

	if ( !com_dedicated->integer ) {

#if !defined RTCW_ET
		Cbuf_AddText( "cinematic gmlogo.RoQ\n" );
		if ( !com_introPlayed->integer ) {
			Cvar_Set( com_introPlayed->name, "1" );
			Cvar_Set( "nextmap", "cinematic wolfintro.RoQ" );
		}
#else
		//Cvar_Set( "com_logosPlaying", "1" );
		Cbuf_AddText( "cinematic etintro.roq\n" );
		/*Cvar_Set( "nextmap", "cinematic avlogo.roq" );
		if( !com_introPlayed->integer ) {
			Cvar_Set( com_introPlayed->name, "1" );
			//Cvar_Set( "nextmap", "cinematic avlogo.roq" );
		}*/
#endif // RTCW_XX

	}
#endif // RTCW_XX

	com_fullyInitialized = qtrue;
	Com_Printf( "--- Common Initialization Complete ---\n" );
}

//==================================================================

void Com_WriteConfigToFile( const char *filename ) {
	fileHandle_t f;

	f = FS_FOpenFileWrite( filename );
	if ( !f ) {
		Com_Printf( "Couldn't write %s.\n", filename );
		return;
	}

#if !defined RTCW_ET
	FS_Printf( f, "// generated by RTCW, do not modify\n" );
#else
	FS_Printf( f, "// generated by ET, do not modify\n" );
#endif // RTCW_XX

	Key_WriteBindings( f );
	Cvar_WriteVariables( f );
	FS_FCloseFile( f );
}


/*
===============
Com_WriteConfiguration

Writes key bindings and archived cvars to config file if modified
===============
*/
void Com_WriteConfiguration( void ) {
#ifndef DEDICATED // bk001204
	cvar_t  *fs;
#endif

#if defined RTCW_ET
	const char *cl_profileStr = Cvar_VariableString( "cl_profile" );
#endif // RTCW_XX

	// if we are quiting without fully initializing, make sure
	// we don't write out anything
	if ( !com_fullyInitialized ) {
		return;
	}

	if ( !( cvar_modifiedFlags & CVAR_ARCHIVE ) ) {
		return;
	}
	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

#if defined RTCW_SP
	Com_WriteConfigToFile( "wolfconfig.cfg" );
#elif defined RTCW_MP
	Com_WriteConfigToFile( "wolfconfig_mp.cfg" );
#else
	if ( com_gameInfo.usesProfiles && cl_profileStr[0] ) {
		Com_WriteConfigToFile( va( "profiles/%s/%s", cl_profileStr, CONFIG_NAME ) );
	} else {
		Com_WriteConfigToFile( CONFIG_NAME );
	}
#endif // RTCW_XX

	// bk001119 - tentative "not needed for dedicated"
#ifndef DEDICATED
	fs = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
	if ( UI_usesUniqueCDKey() && fs && fs->string[0] != 0 ) {
		Com_WriteCDKey( fs->string, &cl_cdkey[16] );
	} else {

#if !defined RTCW_ET
		Com_WriteCDKey( "main", cl_cdkey );
#else
		Com_WriteCDKey( BASEGAME, cl_cdkey );
#endif // RTCW_XX

	}
#endif
}


/*
===============
Com_WriteConfig_f

Write the config file to a specific name
===============
*/
void Com_WriteConfig_f( void ) {
	char filename[MAX_QPATH];

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: writeconfig <filename>\n" );
		return;
	}

	Q_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".cfg" );
	Com_Printf( "Writing %s.\n", filename );
	Com_WriteConfigToFile( filename );
}

/*
================
Com_ModifyMsec
================
*/
int Com_ModifyMsec( int msec ) {
	int clampTime;

	//
	// modify time for debugging values
	//
	if ( com_fixedtime->integer ) {
		msec = com_fixedtime->integer;
	} else if ( com_timescale->value ) {
		msec *= com_timescale->value;
//	} else if (com_cameraMode->integer) {
//		msec *= com_timescale->value;
	}

	// don't let it scale below 1 msec
	if ( msec < 1 && com_timescale->value ) {
		msec = 1;
	}

	if ( com_dedicated->integer ) {
		// dedicated servers don't want to clamp for a much longer
		// period, because it would mess up all the client's views
		// of time.

#if defined RTCW_SP
		if ( msec > 500 ) {
#else
		if ( msec > 500 && msec < 500000 ) {
#endif // RTCW_XX

			Com_Printf( "Hitch warning: %i msec frame time\n", msec );
		}
		clampTime = 5000;
	} else
	if ( !com_sv_running->integer ) {
		// clients of remote servers do not want to clamp time, because
		// it would skew their view of the server's time temporarily
		clampTime = 5000;
	} else {
		// for local single player gaming
		// we may want to clamp the time to prevent players from
		// flying off edges when something hitches.
		clampTime = 200;
	}

	if ( msec > clampTime ) {
		msec = clampTime;
	}

	return msec;
}

#ifndef RTCW_VANILLA
namespace {

void com_handle_soft_error()
{
	switch (com_soft_error_code)
	{
		case ERR_SERVERDISCONNECT:
			CL_Disconnect( qtrue );
			CL_FlushMemory();
			com_errorEntered = qfalse;
			break;

#ifdef RTCW_SP
		case ERR_ENDGAME:
			//----(SA)	added
			SV_Shutdown( "endgame" );
			if ( com_cl_running && com_cl_running->integer ) {
				CL_Disconnect( qtrue );
				CL_FlushMemory();
				com_errorEntered = qfalse;
				CL_EndgameMenu();
			}
			break;
#endif // RTCW_SP

		case ERR_DROP:
		case ERR_DISCONNECT:
			Com_Printf( "********************\nERROR: %s\n********************\n", com_errorMessage );
			SV_Shutdown( va( "Server crashed: %s\n",  com_errorMessage ) );
			CL_Disconnect( qtrue );
			CL_FlushMemory();
			com_errorEntered = qfalse;
			break;

		case ERR_NEED_CD:
			SV_Shutdown( "Server didn't have CD\n" );
			if ( com_cl_running && com_cl_running->integer ) {
				CL_Disconnect( qtrue );
				CL_FlushMemory();
				com_errorEntered = qfalse;
				CL_CDDialog();
			} else {
				Com_Printf( "Server didn't have CD\n" );
			}
			break;

#ifdef RTCW_ET
#ifndef DEDICATED
		case ERR_AUTOUPDATE:
			CL_Disconnect( qtrue );
			CL_FlushMemory();
			com_errorEntered = qfalse;
			break;
#endif // DEDICATED
#endif // RTCW_ET
	}
}

} // namespace
#endif // RTCW_VANILLA

/*
=================
Com_Frame
=================
*/
void Com_Frame( void ) {

	int msec, minMsec;
	static int lastTime;
	int key;

	int timeBeforeFirstEvents;
	int timeBeforeServer;
	int timeBeforeEvents;
	int timeBeforeClient;
	int timeAfter;

#if defined RTCW_ET
	static int watchdogTime = 0;
	static qboolean watchWarn = qfalse;
#endif // RTCW_XX

#ifndef RTCW_VANILLA
	com_soft_error_code = 0;
#endif // RTCW_VANILLA

	if ( ::setjmp( abortframe ) ) {
#ifndef RTCW_VANILLA
		com_handle_soft_error();
#endif // RTCW_VANILLA
		return;         // an ERR_DROP was thrown
	}

	// bk001204 - init to zero.
	//  also:  might be clobbered by `longjmp' or `vfork'
	timeBeforeFirstEvents = 0;
	timeBeforeServer = 0;
	timeBeforeEvents = 0;
	timeBeforeClient = 0;
	timeAfter = 0;


	// old net chan encryption key
	key = 0x87243987;

#if !defined RTCW_MP
	// write config file if anything changed
	Com_WriteConfiguration();
#else
	// DHM - Nerve :: Don't write config on Update Server
#ifndef UPDATE_SERVER
	// write config file if anything changed
	Com_WriteConfiguration();
#endif
#endif // RTCW_XX

	// if "viewlog" has been modified, show or hide the log console
	if ( com_viewlog->modified ) {
#ifdef DEDICATED
		Sys_ShowConsole(com_viewlog->integer, qfalse);
#else
		if ( !com_dedicated->value ) {
			Sys_ShowConsole( com_viewlog->integer, qfalse );
		}
#endif // DEDICATED
		com_viewlog->modified = qfalse;
	}

	//
	// main event loop
	//
	if ( com_speeds->integer ) {
		timeBeforeFirstEvents = Sys_Milliseconds();
	}

	// we may want to spin here if things are going too fast
	if ( !com_dedicated->integer && com_maxfps->integer > 0 && !com_timedemo->integer ) {
		minMsec = 1000 / com_maxfps->integer;
	} else {
		minMsec = 1;
	}
	do {
		com_frameTime = Com_EventLoop();
		if ( lastTime > com_frameTime ) {
			lastTime = com_frameTime;       // possible on first frame
		}
		msec = com_frameTime - lastTime;
	} while ( msec < minMsec );
	Cbuf_Execute();

	lastTime = com_frameTime;

	// mess with msec if needed
	com_frameMsec = msec;
	msec = Com_ModifyMsec( msec );

	//
	// server side
	//
	if ( com_speeds->integer ) {
		timeBeforeServer = Sys_Milliseconds();
	}

	SV_Frame( msec );

	// if "dedicated" has been modified, start up
	// or shut down the client system.
	// Do this after the server may have started,
	// but before the client tries to auto-connect
	if ( com_dedicated->modified ) {
		// get the latched value
		Cvar_Get( "dedicated", "0", 0 );
		com_dedicated->modified = qfalse;
		if ( !com_dedicated->integer ) {
			CL_Init();
			Sys_ShowConsole( com_viewlog->integer, qfalse );
		} else {
			CL_Shutdown();
			Sys_ShowConsole( 1, qtrue );
		}
	}

	//
	// client system
	//
	if ( !com_dedicated->integer ) {
		//
		// run event loop a second time to get server to client packets
		// without a frame of latency
		//
		if ( com_speeds->integer ) {
			timeBeforeEvents = Sys_Milliseconds();
		}
		Com_EventLoop();
		Cbuf_Execute();


		//
		// client side
		//
		if ( com_speeds->integer ) {
			timeBeforeClient = Sys_Milliseconds();
		}

		CL_Frame( msec );

		if ( com_speeds->integer ) {
			timeAfter = Sys_Milliseconds();
		}

#if defined RTCW_ET
	} else {
		timeAfter = Sys_Milliseconds();
	}

	//
	// watchdog
	//
	if ( com_dedicated->integer && !com_sv_running->integer && com_watchdog->integer ) {
		if ( watchdogTime == 0 ) {
			watchdogTime = Sys_Milliseconds();
		} else {
			if ( !watchWarn && Sys_Milliseconds() - watchdogTime > ( com_watchdog->integer - 4 ) * 1000 ) {
				Com_Printf( "WARNING: watchdog will trigger in 4 seconds\n" );
				watchWarn = qtrue;
			} else if ( Sys_Milliseconds() - watchdogTime > com_watchdog->integer * 1000 ) {
				Com_Printf( "Idle Server with no map - triggering watchdog\n" );
				watchdogTime = 0;
				watchWarn = qfalse;
				if ( com_watchdog_cmd->string[0] == '\0' ) {
					Cbuf_AddText( "quit\n" );
				} else {
					Cbuf_AddText( va( "%s\n", com_watchdog_cmd->string ) );
				}
			}
		}
#endif // RTCW_XX

	}

	//
	// report timing information
	//
	if ( com_speeds->integer ) {

#if !defined RTCW_ET
		int all, sv, ev, cl;
#else
		int all, sv, sev, cev, cl;
#endif // RTCW_XX

		all = timeAfter - timeBeforeServer;
		sv = timeBeforeEvents - timeBeforeServer;

#if !defined RTCW_ET
		ev = timeBeforeServer - timeBeforeFirstEvents + timeBeforeClient - timeBeforeEvents;
#else
		sev = timeBeforeServer - timeBeforeFirstEvents;
		cev = timeBeforeClient - timeBeforeEvents;
#endif // RTCW_XX

		cl = timeAfter - timeBeforeClient;
		sv -= time_game;
		cl -= time_frontend + time_backend;


#if !defined RTCW_ET
		Com_Printf( "frame:%i all:%3i sv:%3i ev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n",
					com_frameNumber, all, sv, ev, cl, time_game, time_frontend, time_backend );
#else
		Com_Printf( "frame:%i all:%3i sv:%3i sev:%3i cev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n",
					com_frameNumber, all, sv, sev, cev, cl, time_game, time_frontend, time_backend );
#endif // RTCW_XX

	}

	//
	// trace optimization tracking
	//
	if ( com_showtrace->integer ) {

		extern int c_traces, c_brush_traces, c_patch_traces;
		extern int c_pointcontents;

		Com_Printf( "%4i traces  (%ib %ip) %4i points\n", c_traces,
					c_brush_traces, c_patch_traces, c_pointcontents );
		c_traces = 0;
		c_brush_traces = 0;
		c_patch_traces = 0;
		c_pointcontents = 0;
	}

	// old net chan encryption key
	key = lastTime * 0x87243987;

	com_frameNumber++;
}

/*
=================
Com_Shutdown
=================
*/

#if !defined RTCW_ET
void Com_Shutdown( void ) {
#else
void Com_Shutdown( qboolean badProfile ) {
#endif // RTCW_XX


#if defined RTCW_ET
	const char *cl_profileStr = Cvar_VariableString( "cl_profile" );


	// delete pid file
	if ( com_gameInfo.usesProfiles && cl_profileStr[0] && !badProfile ) {
		if ( FS_FileExists( va( "profiles/%s/profile.pid", cl_profileStr ) ) ) {
			FS_Delete( va( "profiles/%s/profile.pid", cl_profileStr ) );
		}
	}
#endif // RTCW_XX

	if ( logfile ) {
		FS_FCloseFile( logfile );
		logfile = 0;
	}

	if ( com_journalFile ) {
		FS_FCloseFile( com_journalFile );
		com_journalFile = 0;
	}

}

void Com_Memcpy( void* dest, const void* src, const size_t count ) {
	memcpy( dest, src, count );
}

void Com_Memset( void* dest, const int val, const size_t count ) {
	memset( dest, val, count );
}

//------------------------------------------------------------------------


/*
=====================
Q_acos

the msvc acos doesn't always return a value between -PI and PI:

int i;
i = 1065353246;
c::acos(*(float*) &i) == -1.#IND0

	This should go in q_math but it is too late to add new traps
	to game and ui
=====================
*/
float Q_acos( float c ) {
	float angle;

	angle = c::acos( c );

	if ( angle > M_PI ) {
		return (float)M_PI;
	}
	if ( angle < -M_PI ) {
		return (float)M_PI;
	}
	return angle;
}

/*
===========================================
command line completion
===========================================
*/

/*
==================
Field_Clear
==================
*/
void Field_Clear( field_t *edit ) {
	memset( edit->buffer, 0, MAX_EDIT_LINE );
	edit->cursor = 0;
	edit->scroll = 0;
}

static const char *completionString;
static char shortestMatch[MAX_TOKEN_CHARS];
static int matchCount;
// field we are working on, passed to Field_CompleteCommand (&g_consoleCommand for instance)
static field_t *completionField;

/*
===============
FindMatches

===============
*/
static void FindMatches( const char *s ) {
	int i;

	if ( Q_stricmpn( s, completionString, strlen( completionString ) ) ) {
		return;
	}
	matchCount++;
	if ( matchCount == 1 ) {
		Q_strncpyz( shortestMatch, s, sizeof( shortestMatch ) );
		return;
	}

	// cut shortestMatch to the amount common with s
	for ( i = 0 ; s[i] ; i++ ) {
		if ( tolower( shortestMatch[i] ) != tolower( s[i] ) ) {
			shortestMatch[i] = 0;
		}
	}

#if defined RTCW_ET
	shortestMatch[i] = 0;
#endif // RTCW_XX

}

/*
===============
PrintMatches

===============
*/
static void PrintMatches( const char *s ) {
	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_Printf( "    %s\n", s );
	}
}

static void keyConcatArgs( void ) {
	int i;
	const char    *arg;

	for ( i = 1 ; i < Cmd_Argc() ; i++ ) {
		Q_strcat( completionField->buffer, sizeof( completionField->buffer ), " " );
		arg = Cmd_Argv( i );
		while ( *arg ) {
			if ( *arg == ' ' ) {
				Q_strcat( completionField->buffer, sizeof( completionField->buffer ),  "\"" );
				break;
			}
			arg++;
		}
		Q_strcat( completionField->buffer, sizeof( completionField->buffer ),  Cmd_Argv( i ) );
		if ( *arg == ' ' ) {
			Q_strcat( completionField->buffer, sizeof( completionField->buffer ),  "\"" );
		}
	}
}

static void ConcatRemaining( const char *src, const char *start ) {
	char *str;

	str = const_cast<char*> (strstr( src, start ));
	if ( !str ) {
		keyConcatArgs();
		return;
	}

	str += strlen( start );
	Q_strcat( completionField->buffer, sizeof( completionField->buffer ), str );
}

/*
===============
Field_CompleteCommand

perform Tab expansion
NOTE TTimo this was originally client code only
  moved to common code when writing tty console for *nix dedicated server
===============
*/
void Field_CompleteCommand( field_t *field ) {
	field_t temp;
	completionField = field;

	// only look at the first token for completion purposes
	Cmd_TokenizeString( completionField->buffer );

	completionString = Cmd_Argv( 0 );
	if ( completionString[0] == '\\' || completionString[0] == '/' ) {
		completionString++;
	}
	matchCount = 0;
	shortestMatch[0] = 0;

	if ( strlen( completionString ) == 0 ) {
		return;
	}

	Cmd_CommandCompletion( FindMatches );
	Cvar_CommandCompletion( FindMatches );

	if ( matchCount == 0 ) {
		return; // no matches
	}

	Com_Memcpy( &temp, completionField, sizeof( field_t ) );

	if ( matchCount == 1 ) {
		Com_sprintf( completionField->buffer, sizeof( completionField->buffer ), "\\%s", shortestMatch );
		if ( Cmd_Argc() == 1 ) {
			Q_strcat( completionField->buffer, sizeof( completionField->buffer ), " " );
		} else {
			ConcatRemaining( temp.buffer, completionString );
		}
		completionField->cursor = strlen( completionField->buffer );
		return;
	}

	// multiple matches, complete to shortest
	Com_sprintf( completionField->buffer, sizeof( completionField->buffer ), "\\%s", shortestMatch );
	completionField->cursor = strlen( completionField->buffer );
	ConcatRemaining( temp.buffer, completionString );

	Com_Printf( "]%s\n", completionField->buffer );

	// run through again, printing matches
	Cmd_CommandCompletion( PrintMatches );
	Cvar_CommandCompletion( PrintMatches );
}

#if defined RTCW_ET
void Com_GetHunkInfo( int* hunkused, int* hunkexpected ) {
	*hunkused =     com_hunkusedvalue;
	*hunkexpected = com_expectedhunkusage;
}

/*
============
Q_vsnprintf

vsnprintf portability:

C99 standard: vsnprintf returns the number of characters (excluding the trailing
'\0') which would have been written to the final string if enough space had been available
snprintf and vsnprintf do not write more than size bytes (including the trailing '\0')

win32: _vsnprintf returns the number of characters written, not including the terminating null character,
or a negative value if an output error occurs. If the number of characters to write exceeds count,
then count characters are written and -1 is returned and no trailing '\0' is added.

Q_vsnPrintf: always append a trailing '\0', returns number of characters written or
returns -1 on failure or if the buffer would be overflowed.
============
*/
int Q_vsnprintf( char *dest, int size, const char *fmt, va_list argptr ) {
	int ret;

#ifdef _WIN32
#undef _vsnprintf
	ret = _vsnprintf( dest, size - 1, fmt, argptr );
#define _vsnprintf  use_idStr_vsnPrintf
#else
#undef vsnprintf
	ret = vsnprintf( dest, size, fmt, argptr );
#define vsnprintf   use_idStr_vsnPrintf
#endif
	dest[size - 1] = '\0';
	if ( ret < 0 || ret >= size ) {
		return -1;
	}
	return ret;
}
#endif // RTCW_XX

