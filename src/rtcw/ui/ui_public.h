/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef __UI_PUBLIC_H__
#define __UI_PUBLIC_H__

#define UI_API_VERSION  4

typedef struct {
	connstate_t connState;
	int connectPacketCount;
	int clientNum;
	char servername[MAX_STRING_CHARS];
	char updateInfoString[MAX_STRING_CHARS];
	char messageString[MAX_STRING_CHARS];
} uiClientState_t;

typedef enum {
	UI_ERROR,
	UI_PRINT,
	UI_MILLISECONDS,
	UI_CVAR_SET,
	UI_CVAR_VARIABLEVALUE,
	UI_CVAR_VARIABLESTRINGBUFFER,

#if defined RTCW_ET
	UI_CVAR_LATCHEDVARIABLESTRINGBUFFER,
#endif // RTCW_XX

	UI_CVAR_SETVALUE,
	UI_CVAR_RESET,
	UI_CVAR_CREATE,
	UI_CVAR_INFOSTRINGBUFFER,
	UI_ARGC,
	UI_ARGV,
	UI_CMD_EXECUTETEXT,

#if defined RTCW_ET
	UI_ADDCOMMAND,
#endif // RTCW_XX

	UI_FS_FOPENFILE,
	UI_FS_READ,

#if defined RTCW_SP
	UI_FS_SEEK, //----(SA)	added
#endif // RTCW_XX

	UI_FS_WRITE,
	UI_FS_FCLOSEFILE,
	UI_FS_GETFILELIST,
	UI_FS_DELETEFILE,

#if defined RTCW_ET
	UI_FS_COPYFILE,
#endif // RTCW_XX

	UI_R_REGISTERMODEL,
	UI_R_REGISTERSKIN,
	UI_R_REGISTERSHADERNOMIP,
	UI_R_CLEARSCENE,
	UI_R_ADDREFENTITYTOSCENE,
	UI_R_ADDPOLYTOSCENE,
	UI_R_ADDPOLYSTOSCENE,
	// JOSEPH 12-6-99
	UI_R_ADDLIGHTTOSCENE,
	// END JOSEPH
	//----(SA)
	UI_R_ADDCORONATOSCENE,
	//----(SA)
	UI_R_RENDERSCENE,
	UI_R_SETCOLOR,

#if defined RTCW_ET
	UI_R_DRAW2DPOLYS,
#endif // RTCW_XX

	UI_R_DRAWSTRETCHPIC,

#if defined RTCW_ET
	UI_R_DRAWROTATEDPIC,
#endif // RTCW_XX

	UI_UPDATESCREEN,        // 30
	UI_CM_LERPTAG,
	UI_CM_LOADMODEL,
	UI_S_REGISTERSOUND,
	UI_S_STARTLOCALSOUND,

#if !defined RTCW_MP
	UI_S_FADESTREAMINGSOUND,    //----(SA)	added
	UI_S_FADEALLSOUNDS,         //----(SA)	added
#endif // RTCW_XX

	UI_KEY_KEYNUMTOSTRINGBUF,
	UI_KEY_GETBINDINGBUF,
	UI_KEY_SETBINDING,

#if defined RTCW_ET
	UI_KEY_BINDINGTOKEYS,
#endif // RTCW_XX

	UI_KEY_ISDOWN,
	UI_KEY_GETOVERSTRIKEMODE,
	UI_KEY_SETOVERSTRIKEMODE,
	UI_KEY_CLEARSTATES,
	UI_KEY_GETCATCHER,
	UI_KEY_SETCATCHER,
	UI_GETCLIPBOARDDATA,
	UI_GETGLCONFIG,
	UI_GETCLIENTSTATE,
	UI_GETCONFIGSTRING,
	UI_LAN_GETLOCALSERVERCOUNT,
	UI_LAN_GETLOCALSERVERADDRESSSTRING,
	UI_LAN_GETGLOBALSERVERCOUNT,        // 50
	UI_LAN_GETGLOBALSERVERADDRESSSTRING,
	UI_LAN_GETPINGQUEUECOUNT,
	UI_LAN_CLEARPING,
	UI_LAN_GETPING,
	UI_LAN_GETPINGINFO,
	UI_CVAR_REGISTER,
	UI_CVAR_UPDATE,
	UI_MEMORY_REMAINING,

	UI_GET_CDKEY,
	UI_SET_CDKEY,
	UI_R_REGISTERFONT,
	UI_R_MODELBOUNDS,
	UI_PC_ADD_GLOBAL_DEFINE,

#if defined RTCW_ET
	UI_PC_REMOVE_ALL_GLOBAL_DEFINES,
#endif // RTCW_XX

	UI_PC_LOAD_SOURCE,
	UI_PC_FREE_SOURCE,
	UI_PC_READ_TOKEN,
	UI_PC_SOURCE_FILE_AND_LINE,

#if defined RTCW_ET
	UI_PC_UNREAD_TOKEN,
#endif // RTCW_XX

	UI_S_STOPBACKGROUNDTRACK,
	UI_S_STARTBACKGROUNDTRACK,
	UI_REAL_TIME,
	UI_LAN_GETSERVERCOUNT,
	UI_LAN_GETSERVERADDRESSSTRING,
	UI_LAN_GETSERVERINFO,
	UI_LAN_MARKSERVERVISIBLE,
	UI_LAN_UPDATEVISIBLEPINGS,
	UI_LAN_RESETPINGS,
	UI_LAN_LOADCACHEDSERVERS,
	UI_LAN_SAVECACHEDSERVERS,
	UI_LAN_ADDSERVER,
	UI_LAN_REMOVESERVER,
	UI_CIN_PLAYCINEMATIC,
	UI_CIN_STOPCINEMATIC,
	UI_CIN_RUNCINEMATIC,
	UI_CIN_DRAWCINEMATIC,
	UI_CIN_SETEXTENTS,
	UI_R_REMAP_SHADER,
	UI_VERIFY_CDKEY,
	UI_LAN_SERVERSTATUS,
	UI_LAN_GETSERVERPING,
	UI_LAN_SERVERISVISIBLE,
	UI_LAN_COMPARESERVERS,

#if defined RTCW_ET
	UI_LAN_SERVERISINFAVORITELIST,
#endif // RTCW_XX

	UI_CL_GETLIMBOSTRING,           // NERVE - SMF

#if !defined RTCW_SP
	UI_SET_PBCLSTATUS,              // DHM - Nerve
	UI_CHECKAUTOUPDATE,             // DHM - Nerve
	UI_GET_AUTOUPDATE,              // DHM - Nerve
	UI_CL_TRANSLATE_STRING,
	UI_OPENURL,
	UI_SET_PBSVSTATUS,              // TTimo
#endif // RTCW_XX

#if !defined RTCW_ET
	UI_MEMSET = 100,
#else
	UI_MEMSET = 200,
#endif // RTCW_XX

	UI_MEMCPY,
	UI_STRNCPY,
	UI_SIN,
	UI_COS,
	UI_ATAN2,
	UI_SQRT,
	UI_FLOOR,

#if !defined RTCW_ET
	UI_CEIL
#else
	UI_CEIL,
#endif // RTCW_XX

#if defined RTCW_ET
	UI_GETHUNKDATA
#endif // RTCW_XX

} uiImport_t;

#if !defined RTCW_ET
typedef enum {
	UIMENU_NONE,
	UIMENU_MAIN,
	UIMENU_INGAME,
	UIMENU_NEED_CD,

#if defined RTCW_SP
	UIMENU_ENDGAME, //----(SA)	added
#endif // RTCW_XX

	UIMENU_BAD_CD_KEY,
	UIMENU_TEAM,

#if defined RTCW_SP
	UIMENU_PREGAME, //----(SA)	added
#endif // RTCW_XX

	UIMENU_POSTGAME,
	UIMENU_NOTEBOOK,
	UIMENU_CLIPBOARD,
	UIMENU_HELP,
	UIMENU_BOOK1,           //----(SA)	added
	UIMENU_BOOK2,           //----(SA)	added
	UIMENU_BOOK3,           //----(SA)	added
	UIMENU_WM_PICKTEAM,     // NERVE - SMF - for multiplayer only
	UIMENU_WM_PICKPLAYER,   // NERVE - SMF - for multiplayer only
	UIMENU_WM_QUICKMESSAGE, // NERVE - SMF

#if defined RTCW_MP
	UIMENU_WM_QUICKMESSAGEALT,  // NERVE - SMF
#endif // RTCW_XX

	UIMENU_WM_LIMBO,        // NERVE - SMF

#if defined RTCW_SP
	UIMENU_BRIEFING         //----(SA)	added
#endif // RTCW_XX

#if defined RTCW_MP
	UIMENU_WM_AUTOUPDATE        // NERVE - DHM
#endif // RTCW_XX

} uiMenuCommand_t;
#endif // RTCW_XX

#define SORT_HOST           0
#define SORT_MAP            1
#define SORT_CLIENTS        2
#define SORT_GAME           3
#define SORT_PING           4

#if defined RTCW_MP
#define SORT_PUNKBUSTER     5
#endif // RTCW_XX

#if defined RTCW_SP
#define SORT_SAVENAME       0
#define SORT_SAVETIME       1
#endif // RTCW_XX

#if defined RTCW_ET
#define SORT_FILTERS        5
#define SORT_FAVOURITES     6
#endif // RTCW_XX

typedef enum {
	UI_GETAPIVERSION = 0,   // system reserved

	UI_INIT,
//	void	UI_Init( void );

	UI_SHUTDOWN,
//	void	UI_Shutdown( void );

	UI_KEY_EVENT,
//	void	UI_KeyEvent( int key );

	UI_MOUSE_EVENT,
//	void	UI_MouseEvent( int dx, int dy );

	UI_REFRESH,
//	void	UI_Refresh( int time );

	UI_IS_FULLSCREEN,
//	qboolean UI_IsFullscreen( void );

	UI_SET_ACTIVE_MENU,
//	void	UI_SetActiveMenu( uiMenuCommand_t menu );

	UI_GET_ACTIVE_MENU,
//	void	UI_GetActiveMenu( void );

	UI_CONSOLE_COMMAND,
//	qboolean UI_ConsoleCommand( void );

	UI_DRAW_CONNECT_SCREEN,
//	void	UI_DrawConnectScreen( qboolean overlay );

#if defined RTCW_SP
	UI_HASUNIQUECDKEY
#else
	UI_HASUNIQUECDKEY,
#endif // RTCW_XX

// if !overlay, the background will be drawn, otherwise it will be
// overlayed over whatever the cgame has drawn.
// a GetClientState syscall will be made to get the current strings

#if defined RTCW_MP
	UI_CHECKEXECKEY     // NERVE - SMF
#elif defined RTCW_ET
	UI_CHECKEXECKEY,        // NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
	UI_WANTSBINDKEYS,
#endif // RTCW_XX


} uiExport_t;

#endif
