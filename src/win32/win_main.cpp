/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

// win_main.h

#include "client.h"
#include "qcommon.h"
#include "win_local.h"

#if defined RTCW_SP
	#include "rtcw_sp_resource.h"
#elif defined RTCW_MP
	#include "rtcw_mp_resource.h"
#elif defined RTCW_ET
	#include "rtcw_et_resource.h"
#endif // RTCW_XX

#include <direct.h>
#include <io.h>

// BBi
#include "SDL.h"

#include "sys_events.h"
// BBi

#if defined RTCW_SP
//#define	CD_BASEDIR	"wolf"
#define CD_BASEDIR  ""
//#define	CD_EXE		"wolf.exe"
#define CD_EXE      "setup\\setup.exe"
//#define	CD_BASEDIR_LINUX	"bin\\x86\\glibc-2.1"
#define CD_BASEDIR_LINUX    "bin\\x86\\glibc-2.1"
//#define	CD_EXE_LINUX "wolf"
#define CD_EXE_LINUX "setup\\setup"

#elif defined RTCW_MP
#define CD_BASEDIR  "wolf"
#define CD_EXE      "wolf.exe"
#define CD_BASEDIR_LINUX    "bin\\x86\\glibc-2.1"
#define CD_EXE_LINUX "wolf"
#else
#define CD_BASEDIR  "et"
#define CD_EXE      "et.exe"
#define CD_BASEDIR_LINUX    "bin\\x86\\glibc-2.1"
#define CD_EXE_LINUX "et"
#endif // RTCW_XX

#define MEM_THRESHOLD 96 * 1024 * 1024

static char sys_cmdline[MAX_STRING_CHARS];

/*
==================
Sys_LowPhysicalMemory()
==================
*/

qboolean Sys_LowPhysicalMemory() {
	MEMORYSTATUS stat;
	GlobalMemoryStatus( &stat );
	return ( stat.dwTotalPhys <= MEM_THRESHOLD ) ? qtrue : qfalse;
}

#if defined RTCW_SP
//NOTE TTimo: heavily NON PORTABLE, PLZ DON'T USE
//  show_bug.cgi?id=447
#if 0
//----(SA) added
/*
==============
Sys_ShellExecute

-	Windows only

	Performs an operation on a specified file.

	See info on ShellExecute() for details

==============
*/
int Sys_ShellExecute( char *op, char *file, qboolean doexit, char *params, char *dir ) {
	unsigned int retval;
	char *se_op;

	// set default operation to "open"
	if ( op ) {
		se_op = op;
	} else { se_op = "open";}


	// probably need to protect this some in the future so people have
	// less chance of system invasion with this powerful interface
	// (okay, not so invasive, but could be annoying/rude)


	retval = (UINT)ShellExecute( NULL, se_op, file, params, dir, SW_NORMAL ); // only option forced by game is 'sw_normal'

	if ( retval <= 32 ) { // ERROR
		Com_DPrintf( "Sys_ShellExecuteERROR: %d\n", retval );
		return retval;
	}

	if ( doexit ) {
		// (SA) this works better for exiting cleanly...
		Cbuf_ExecuteText( EXEC_APPEND, "quit" );
	}

	return 999; // success
}
//----(SA) end
#endif

//----(SA)	from NERVE MP codebase (10/15/01)  (checkins at time of this file should be related)
#endif // RTCW_XX

#if defined RTCW_SP
/*
==================
Sys_StartProcess
==================
*/
void Sys_StartProcess( const char *exeName, qboolean doexit ) {           // NERVE - SMF
#else
/*
==================
Sys_StartProcess

NERVE - SMF
==================
*/
void Sys_StartProcess( const char *exeName, qboolean doexit ) {
#endif // RTCW_XX

	TCHAR szPathOrig[_MAX_PATH];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof( si );

	GetCurrentDirectory( _MAX_PATH, szPathOrig );

#if !defined RTCW_SP
	// JPW NERVE swiped from Sherman's SP code
#endif // RTCW_XX

	if ( !CreateProcess( NULL, va( "%s\\%s", szPathOrig, exeName ), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		// couldn't start it, popup error box
		Com_Error( ERR_DROP, "Could not start process: '%s\\%s' ", szPathOrig, exeName  );
		return;
	}

#if !defined RTCW_SP
	// jpw
#endif // RTCW_XX

	// TTimo: similar way of exiting as used in Sys_OpenURL below
	if ( doexit ) {

#if defined RTCW_SP
		Cbuf_ExecuteText( EXEC_APPEND, "quit" );
#else
		Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
#endif // RTCW_XX

	}
}


#if defined RTCW_SP
/*
==================
Sys_OpenURL
==================
*/
void Sys_OpenURL( const char *url, qboolean doexit ) {                // NERVE - SMF
#else
/*
==================
Sys_OpenURL

NERVE - SMF
==================
*/
void Sys_OpenURL( const char *url, qboolean doexit ) {
#endif // RTCW_XX

	HWND wnd;

#if defined RTCW_ET
	static qboolean doexit_spamguard = qfalse;

	if ( doexit_spamguard ) {
		Com_DPrintf( "Sys_OpenURL: already in a doexit sequence, ignoring %s\n", url );
		return;
	}

	Com_Printf( "Open URL: %s\n", url );
#endif // RTCW_XX

	if ( !ShellExecute( NULL, "open", url, NULL, NULL, SW_RESTORE ) ) {
		// couldn't start it, popup error box
		Com_Error( ERR_DROP, "Could not open url: '%s' ", url );
		return;
	}

	wnd = GetForegroundWindow();

	if ( wnd ) {
		ShowWindow( wnd, SW_MAXIMIZE );
	}

	if ( doexit ) {

#if defined RTCW_ET
		// show_bug.cgi?id=612
		doexit_spamguard = qtrue;
#endif // RTCW_XX

#if defined RTCW_SP
		Cbuf_ExecuteText( EXEC_APPEND, "quit" );
#else
		Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
#endif // RTCW_XX

	}
}

#if defined RTCW_SP
//----(SA)	end
#endif // RTCW_XX

/*
==================
Sys_BeginProfiling
==================
*/
void Sys_BeginProfiling( void ) {
	// this is just used on the mac build
}

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void QDECL Sys_Error( const char *error, ... ) {
	va_list argptr;
	char text[4096];
	MSG msg;

	va_start( argptr, error );

#if !defined RTCW_ET
	vsprintf( text, error, argptr );
#else
	Q_vsnprintf( text, sizeof( text ), error, argptr );
#endif // RTCW_XX

	va_end( argptr );

	Conbuf_AppendText( text );
	Conbuf_AppendText( "\n" );

	Sys_SetErrorText( text );
	Sys_ShowConsole( 1, qtrue );

	// BBi
	//timeEndPeriod( 1 );
	// BBi

	IN_Shutdown();

#if 0
	// wait for the user to quit
	while ( 1 ) {
		if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
			Com_Quit_f();
		}
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
#else
	sys_run_console();
	Com_Quit_f();
#endif

	Sys_DestroyConsole();

	exit( 1 );
}

/*
==============
Sys_Quit
==============
*/
void Sys_Quit( void ) {
	// BBi
	//timeEndPeriod( 1 );
	// BBi

	IN_Shutdown();
	Sys_DestroyConsole();

	exit( 0 );
}

/*
==============
Sys_Print
==============
*/
void Sys_Print( const char *msg ) {
	Conbuf_AppendText( msg );
}


/*
==============
Sys_Mkdir
==============
*/
void Sys_Mkdir( const char *path ) {
	_mkdir( path );
}

/*
==============
Sys_Cwd
==============
*/
char *Sys_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	_getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH - 1] = 0;

	return cwd;
}

/*
==============
Sys_DefaultCDPath
==============
*/
const char *Sys_DefaultCDPath( void ) {
	return "";
}

/*
==============
Sys_DefaultBasePath
==============
*/
const char *Sys_DefaultBasePath( void ) {
	return Sys_Cwd();
}

/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

#define MAX_FOUND_FILES 0x1000

void Sys_ListFilteredFiles( const char *basedir, const char *subdirs, const char *filter, char **list, int *numfiles ) {
	char search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char filename[MAX_OSPATH];
	int findhandle;
	struct _finddata_t findinfo;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if ( strlen( subdirs ) ) {
		Com_sprintf( search, sizeof( search ), "%s\\%s\\*", basedir, subdirs );
	} else {
		Com_sprintf( search, sizeof( search ), "%s\\*", basedir );
	}

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		return;
	}

	do {
		if ( findinfo.attrib & _A_SUBDIR ) {
			if ( Q_stricmp( findinfo.name, "." ) && Q_stricmp( findinfo.name, ".." ) ) {
				if ( strlen( subdirs ) ) {
					Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s\\%s", subdirs, findinfo.name );
				} else {
					Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s", findinfo.name );
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof( filename ), "%s\\%s", subdirs, findinfo.name );
		if ( !Com_FilterPath( filter, filename, qfalse ) ) {
			continue;
		}
		list[ *numfiles ] = CopyString( filename );
		( *numfiles )++;
	} while ( _findnext( findhandle, &findinfo ) != -1 );

	_findclose( findhandle );
}

static qboolean strgtr( const char *s0, const char *s1 ) {
	int l0, l1, i;

	l0 = strlen( s0 );
	l1 = strlen( s1 );

	if ( l1 < l0 ) {
		l0 = l1;
	}

	for ( i = 0; i < l0; i++ ) {
		if ( s1[i] > s0[i] ) {
			return qtrue;
		}
		if ( s1[i] < s0[i] ) {
			return qfalse;
		}
	}
	return qfalse;
}

char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, int *numfiles, qboolean wantsubs ) {
	char search[MAX_OSPATH];
	int nfiles;
	char        **listCopy;
	char        *list[MAX_FOUND_FILES];
	struct _finddata_t findinfo;
	intptr_t findhandle;
	int flag;
	int i;

	if ( filter ) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = 0;
		*numfiles = nfiles;

		if ( !nfiles ) {
			return NULL;
		}

		listCopy = static_cast<char**> (Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) ));
		for ( i = 0 ; i < nfiles ; i++ ) {
			listCopy[i] = list[i];
		}
		listCopy[i] = NULL;

		return listCopy;
	}

	if ( !extension ) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	Com_sprintf( search, sizeof( search ), "%s\\*%s", directory, extension );

	// search
	nfiles = 0;

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		*numfiles = 0;
		return NULL;
	}

	do {
		if ( ( !wantsubs && flag ^ ( findinfo.attrib & _A_SUBDIR ) ) || ( wantsubs && findinfo.attrib & _A_SUBDIR ) ) {
			if ( nfiles == MAX_FOUND_FILES - 1 ) {
				break;
			}
			list[ nfiles ] = CopyString( findinfo.name );
			nfiles++;
		}
	} while ( _findnext( findhandle, &findinfo ) != -1 );

	list[ nfiles ] = 0;

	_findclose( findhandle );

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = static_cast<char**> (Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) ));
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	do {
		flag = 0;
		for ( i = 1; i < nfiles; i++ ) {
			if ( strgtr( listCopy[i - 1], listCopy[i] ) ) {
				char *temp = listCopy[i];
				listCopy[i] = listCopy[i - 1];
				listCopy[i - 1] = temp;
				flag = 1;
			}
		}
	} while ( flag );

	return listCopy;
}

void    Sys_FreeFileList( char **list ) {
	int i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}

//========================================================


/*
================
Sys_ScanForCD

Search all the drives to see if there is a valid CD to grab
the cddir from
================
*/
qboolean Sys_ScanForCD( void ) {
	static char cddir[MAX_OSPATH];
	char drive[4];
	FILE        *f;
	char test[MAX_OSPATH];
#if 0
	// don't override a cdpath on the command line
	if ( strstr( sys_cmdline, "cdpath" ) ) {
		return;
	}
#endif

	drive[0] = 'c';
	drive[1] = ':';
	drive[2] = '\\';
	drive[3] = 0;

	// scan the drives
	for ( drive[0] = 'c' ; drive[0] <= 'z' ; drive[0]++ ) {
		if ( GetDriveType( drive ) != DRIVE_CDROM ) {
			continue;
		}

		sprintf( cddir, "%s%s", drive, CD_BASEDIR );
		sprintf( test, "%s\\%s", cddir, CD_EXE );
		f = fopen( test, "r" );
		if ( f ) {
			fclose( f );
			return qtrue;
		} else {
			sprintf( cddir, "%s%s", drive, CD_BASEDIR_LINUX );
			sprintf( test, "%s\\%s", cddir, CD_EXE_LINUX );
			f = fopen( test, "r" );
			if ( f ) {
				fclose( f );
				return qtrue;
			}
		}
	}

	return qfalse;
}

/*
================
Sys_CheckCD

Return true if the proper CD is in the drive
================
*/
qboolean    Sys_CheckCD( void ) {
	// FIXME: mission pack
	return qtrue;

#if !defined RTCW_MP
//	return Sys_ScanForCD();
#else
	//return Sys_ScanForCD();
#endif // RTCW_XX

}

char* Sys_GetClipboardData()
{
	if (SDL_HasClipboardText())
		return SDL_GetClipboardText();

	return NULL;
}



/*
========================================================================

LOAD/UNLOAD DLL

========================================================================
*/

/*
=================
Sys_UnloadDll

=================
*/

void Sys_UnloadDll(void* dllHandle)
{
	SDL_UnloadObject(dllHandle);
}


/*
=================
Sys_LoadDll

Used to load a development dll instead of a virtual machine
=================
*/

extern char* FS_BuildOSPath(
	const char* base,
	const char* game,
	const char* qpath);

#if defined RTCW_MP
#ifdef UPDATE_SERVER
int cl_connectedToPureServer;
#else
extern int cl_connectedToPureServer;
#endif
#elif defined RTCW_ET
extern int cl_connectedToPureServer;
#endif // RTCW_XX

#if 0
#if !defined RTCW_SP
char* Sys_GetDLLName( const char *name ) {
// BBi FIXME
	//return va( "%s_mp_x86.dll", name );
#ifdef _WIN64
	return va ("%s_mp_x64.dll", name);
#else
	return va ("%s_mp_x86.dll", name);
#endif
// BBi
}
#endif // RTCW_XX
#endif // 0

const char* Sys_GetDLLName(const char* name)
{
	const std::string bits =
#if defined RTCW_32
		"x86"
#elif defined RTCW_64
		"x64"
#else
	#error Unknown CPU architecture
#endif
		;

	const std::string game =
#ifdef RTCW_SP
		""
#else
		"_mp_"
#endif
		;

	const std::string is_demo =
#ifndef WOLF_SP_DEMO
		""
#else
		"_d"
#endif
		;

	const std::string ext =
#ifdef __WIN32__
		".dll"
#else
		".so"
#endif
		;

	static std::string buffer;

	buffer = name + game + bits + is_demo + ext;

	return buffer.c_str();
}

// fqpath param added 2/15/02 by T.Ray - Sys_LoadDll is only called in vm.c at this time
// fqpath will be empty if dll not loaded, otherwise will hold fully qualified path of dll module loaded
// fqpath buffersize must be at least MAX_QPATH+1 bytes long
void* QDECL Sys_LoadDll(
	const char* name,
	char* fqpath,
	DllEntryPoint* entryPoint,
	DllEntryPoint systemcalls)
{
	typedef void (QDECL* DllEntry)(DllEntryPoint);

	*fqpath = '\0'; // added 2/15/02 by T.Ray

	const char* basepath = Cvar_VariableString("fs_basepath");
	const char* cdpath = Cvar_VariableString("fs_cdpath");
	const char* gamedir = Cvar_VariableString("fs_game");

	std::string filename = Sys_GetDLLName(name);

	// try gamepath first
	char* fn = FS_BuildOSPath(basepath, gamedir, filename.c_str());

#if !defined RTCW_SP
	// TTimo - this is only relevant for full client
	// if a full client runs a dedicated server, it's not affected by this
#if !defined DEDICATED
	// NERVE - SMF - extract dlls from pak file for security
	// we have to handle the game dll a little differently
	// TTimo - passing the exact path to check against
	//   (compatibility with other OSes loading procedure)
	if (cl_connectedToPureServer && Q_strncmp(name, "qagame", 6) != 0) {
		if (!::FS_CL_ExtractFromPakFile(fn, gamedir, filename.c_str(), NULL)) {
			Com_Error(
				ERR_DROP,
				"Game code(%s) failed Pure Server check",
				filename.c_str());
		}
	}
#endif
#endif // RTCW_XX

	void* libHandle = SDL_LoadObject(fn);

	if (libHandle == NULL) {
		if (cdpath[0] != '\0') {
			fn = FS_BuildOSPath(cdpath, gamedir, filename.c_str());
			libHandle = SDL_LoadObject(fn);
		}
	}

	if (libHandle == NULL) {
		fn = FS_BuildOSPath(basepath, BASEGAME, filename.c_str());
		libHandle = SDL_LoadObject(fn);
	}

	if (libHandle == NULL) {
		strcpy(fn, filename.c_str());
		libHandle = SDL_LoadObject(fn);
	}

	if (libHandle == NULL)
		return NULL;


	Q_strncpyz(fqpath, fn, MAX_QPATH); // added 2/15/02 by T.Ray

	DllEntry dllEntry = reinterpret_cast<DllEntry>(
		SDL_LoadFunction(libHandle, "dllEntry"));

	*entryPoint = reinterpret_cast<DllEntryPoint>(
		SDL_LoadFunction(libHandle, "vmMain"));

	if (*entryPoint == NULL || dllEntry == NULL) {
		SDL_UnloadObject(libHandle);
		return NULL;
	}

	dllEntry(systemcalls);

	return libHandle;
}


/*
========================================================================

EVENT LOOP

========================================================================
*/

#define MAX_QUED_EVENTS     256
#define MASK_QUED_EVENTS    ( MAX_QUED_EVENTS - 1 )

sysEvent_t eventQue[MAX_QUED_EVENTS];
int eventHead, eventTail;
byte sys_packetReceived[MAX_MSGLEN];

/*
================
Sys_QueEvent

A time of 0 will get the current time
Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr ) {
	sysEvent_t  *ev;

	ev = &eventQue[ eventHead & MASK_QUED_EVENTS ];
	if ( eventHead - eventTail >= MAX_QUED_EVENTS ) {
		Com_Printf( "Sys_QueEvent: overflow\n" );
		// we are discarding an event, but don't leak memory
		if ( ev->evPtr ) {
			Z_Free( ev->evPtr );
		}
		eventTail++;
	}

	eventHead++;

	if ( time == 0 ) {
		time = Sys_Milliseconds();
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

/*
================
Sys_GetEvent

================
*/
sysEvent_t Sys_GetEvent( void ) {
	MSG msg;
	sysEvent_t ev;
	char        *s;
	msg_t netmsg;
	netadr_t adr;

	// return if we have data
	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
	}

	// pump the message loop

// BBi
	//while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
	//	if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
	//		Com_Quit_f();
	//	}

	//	// save the msg time, because wndprocs don't have access to the timestamp
	//	g_wv.sysMsgTime = msg.time;

	//	TranslateMessage( &msg );
	//	DispatchMessage( &msg );
	//}
	sys_poll_events();
// BBi

	// check for console commands
	s = Sys_ConsoleInput();
	if ( s ) {
		char    *b;
		int len;

		len = strlen( s ) + 1;
		b = static_cast<char*> (Z_Malloc( len ));
		Q_strncpyz( b, s, len - 1 );
		Sys_QueEvent( 0, SE_CONSOLE, 0, 0, len, b );
	}

	// check for network packets
	MSG_Init( &netmsg, sys_packetReceived, sizeof( sys_packetReceived ) );
	if ( Sys_GetPacket( &adr, &netmsg ) ) {
		netadr_t        *buf;
		int len;

		// copy out to a seperate buffer for qeueing
		// the readcount stepahead is for SOCKS support
		len = sizeof( netadr_t ) + netmsg.cursize - netmsg.readcount;
		buf = static_cast<netadr_t*> (Z_Malloc( len ));
		*buf = adr;
		memcpy( buf + 1, &netmsg.data[netmsg.readcount], netmsg.cursize - netmsg.readcount );
		Sys_QueEvent( 0, SE_PACKET, 0, 0, len, buf );
	}

	// return if we have data
	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
	}

	// create an empty event to return

	memset( &ev, 0, sizeof( ev ) );
	ev.evTime = SDL_GetTicks();

	return ev;
}

//================================================================

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( void ) {
	IN_Shutdown();
	IN_Init();
}


/*
=================
Sys_Net_Restart_f

Restart the network subsystem
=================
*/
void Sys_Net_Restart_f( void ) {
	NET_Restart();
}


/*
================
Sys_Init

Called after the common systems (cvars, files, etc)
are initialized
================
*/
extern void Sys_ClearViewlog_f( void ); // fretn

void Sys_Init( void ) {
	Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
	Cmd_AddCommand( "net_restart", Sys_Net_Restart_f );
	Cmd_AddCommand( "clearviewlog", Sys_ClearViewlog_f );

	// FIXME
	Cvar_Set("arch", SDL_GetPlatform());

	int cpuid = CPUID_GENERIC;
	Cvar_Get("sys_cpustring", "detect", 0);

	const char* cpu_string = Cvar_VariableString("sys_cpustring");

	if (Q_stricmp(cpu_string, "detect") == 0)
		Cvar_Set("sys_cpustring", "generic");

	Cvar_SetValue("sys_cpuid", cpuid);

	cpu_string = Cvar_VariableString("sys_cpustring");

	Cvar_Set("username", Sys_GetCurrentUser());

#if !defined RTCW_ET
	IN_Init(); // FIXME: not in dedicated?
#endif // RTCW_XX
}


//=======================================================================

int totalMsec, countMsec;

/*
==================
WinMain

==================
*/

// BBi
//int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
extern "C" int main(int argc, char* argv[])
{
// BBi

	if (SDL_Init(0) != 0) {
		// TODO Print out some message.
		return 1;
	}

	char cwd[MAX_OSPATH];
	int startTime, endTime;

// BBi
	//Q_strncpyz( sys_cmdline, lpCmdLine, sizeof( sys_cmdline ) );

	const size_t MAX_CMD_LINE_LENGTH = sizeof(sys_cmdline) - 1;
	std::string cmd_line;
	cmd_line.reserve(MAX_CMD_LINE_LENGTH);

	for (int i = 1; i < argc; ++i) {
		cmd_line += ' ';
		cmd_line += argv[i];
	}

	if (cmd_line.size() > MAX_CMD_LINE_LENGTH)
		cmd_line.resize(MAX_CMD_LINE_LENGTH);

	std::string::traits_type::copy(
		sys_cmdline,
		cmd_line.c_str(),
		cmd_line.size());
// BBi

	// done before Com/Sys_Init since we need this for error output
	Sys_CreateConsole();

	// no abort/retry/fail errors
	SetErrorMode( SEM_FAILCRITICALERRORS );

	// get the initial time base
	Sys_Milliseconds();

#if defined RTCW_SP
// re-enabled CD checking for proper 'setup.exe' file on game cd
// (SA) enable to do cd check for setup\setup.exe
//#if 1
#if 0
	// if we find the CD, add a +set cddir xxx command line
	if ( !Sys_ScanForCD() ) {
		Sys_Error( "Game CD not in drive" );
	}

#endif
#else
#if 0
	// if we find the CD, add a +set cddir xxx command line
	Sys_ScanForCD();
#endif
#endif // RTCW_XX

	Com_Init( sys_cmdline );
	NET_Init();

#if defined RTCW_ET
#ifndef DEDICATED
	IN_Init(); // fretn - directinput must be inited after video etc
#endif
#endif // RTCW_XX

	_getcwd( cwd, sizeof( cwd ) );
	Com_Printf( "Working directory: %s\n", cwd );

	// hide the early console since we've reached the point where we
	// have a working graphics subsystems
	if ( !com_dedicated->integer && !com_viewlog->integer ) {
		Sys_ShowConsole( 0, qfalse );
	}

	// main game loop
	while ( 1 ) {
		// if not running as a game client, sleep a bit
		if (com_dedicated && com_dedicated->integer) {
			Sleep( 5 );
		}

		// set low precision every frame, because some system calls
		// reset it arbitrarily
//		_controlfp( _PC_24, _MCW_PC );
//    _controlfp( -1, _MCW_EM  ); // no exceptions, even if some crappy
		// syscall turns them back on!

		startTime = Sys_Milliseconds();

		// make sure mouse and joystick are only called once a frame
		IN_Frame();

		// run the game
		Com_Frame();

		endTime = Sys_Milliseconds();
		totalMsec += endTime - startTime;
		countMsec++;
	}

	// never gets here
	return 0;
}

#if defined RTCW_ET
bool Sys_IsNumLockDown()
{
	SDL_Keymod state = SDL_GetModState();
	return (state & KMOD_NUM) != 0;
}
#endif // RTCW_XX

