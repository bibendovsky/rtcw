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



#define CMD_BACKUP          64
#define CMD_MASK            ( CMD_BACKUP - 1 )
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP


#if !defined RTCW_ET
#define MAX_ENTITIES_IN_SNAPSHOT    256
#else
#define MAX_ENTITIES_IN_SNAPSHOT    512
#endif // RTCW_XX

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct {
	int snapFlags;                      // SNAPFLAG_RATE_DELAYED, etc
	int ping;

	int serverTime;                 // server time the message is valid for (in msec)

	byte areamask[MAX_MAP_AREA_BYTES];                  // portalarea visibility bits

	playerState_t ps;                       // complete information about the current player at this time

	int numEntities;                        // all of the entities that need to be presented
	entityState_t entities[MAX_ENTITIES_IN_SNAPSHOT];   // at the time of this snapshot

	int numServerCommands;                  // text based server commands to execute when this
	int serverCommandSequence;              // snapshot becomes current
} snapshot_t;

#if !defined RTCW_ET
enum {
	CGAME_EVENT_NONE,
	CGAME_EVENT_TEAMMENU,
	CGAME_EVENT_SCOREBOARD,
	CGAME_EVENT_EDITHUD
};
#else
typedef enum cgameEvent_e {
	CGAME_EVENT_NONE,
	CGAME_EVENT_GAMEVIEW,
	CGAME_EVENT_SPEAKEREDITOR,
	CGAME_EVENT_CAMPAIGNBREIFING,
	CGAME_EVENT_DEMO,               // OSP
	CGAME_EVENT_FIRETEAMMSG,
} cgameEvent_t;
#endif // RTCW_XX


/*
==================================================================

functions imported from the main executable

==================================================================
*/

#define CGAME_IMPORT_API_VERSION    3

typedef enum {
	CG_PRINT,
	CG_ERROR,
	CG_MILLISECONDS,
	CG_CVAR_REGISTER,
	CG_CVAR_UPDATE,
	CG_CVAR_SET,
	CG_CVAR_VARIABLESTRINGBUFFER,

#if defined RTCW_ET
	CG_CVAR_LATCHEDVARIABLESTRINGBUFFER,
#endif // RTCW_XX

	CG_ARGC,
	CG_ARGV,
	CG_ARGS,
	CG_FS_FOPENFILE,
	CG_FS_READ,
	CG_FS_WRITE,
	CG_FS_FCLOSEFILE,

#if defined RTCW_ET
	CG_FS_GETFILELIST,
	CG_FS_DELETEFILE,
#endif // RTCW_XX

	CG_SENDCONSOLECOMMAND,
	CG_ADDCOMMAND,
	CG_SENDCLIENTCOMMAND,
	CG_UPDATESCREEN,
	CG_CM_LOADMAP,
	CG_CM_NUMINLINEMODELS,
	CG_CM_INLINEMODEL,
	CG_CM_LOADMODEL,
	CG_CM_TEMPBOXMODEL,
	CG_CM_POINTCONTENTS,
	CG_CM_TRANSFORMEDPOINTCONTENTS,
	CG_CM_BOXTRACE,
	CG_CM_TRANSFORMEDBOXTRACE,
// MrE:
	CG_CM_CAPSULETRACE,
	CG_CM_TRANSFORMEDCAPSULETRACE,
	CG_CM_TEMPCAPSULEMODEL,
// done.
	CG_CM_MARKFRAGMENTS,

#if defined RTCW_ET
	CG_R_PROJECTDECAL,          // ydnar: projects a decal onto brush models
	CG_R_CLEARDECALS,           // ydnar: clears world/entity decals
#endif // RTCW_XX

	CG_S_STARTSOUND,
	CG_S_STARTSOUNDEX,  //----(SA)	added
	CG_S_STARTLOCALSOUND,
	CG_S_CLEARLOOPINGSOUNDS,

#if defined RTCW_ET
	CG_S_CLEARSOUNDS,
#endif // RTCW_XX

	CG_S_ADDLOOPINGSOUND,
	CG_S_UPDATEENTITYPOSITION,
// Ridah, talking animations
	CG_S_GETVOICEAMPLITUDE,
// done.
	CG_S_RESPATIALIZE,
	CG_S_REGISTERSOUND,
	CG_S_STARTBACKGROUNDTRACK,

#if !defined RTCW_MP
	CG_S_FADESTREAMINGSOUND,    //----(SA)	modified
	CG_S_FADEALLSOUNDS,         //----(SA)	added for fading out everything
#endif // RTCW_XX

	CG_S_STARTSTREAMINGSOUND,

#if defined RTCW_ET
	CG_S_GETSOUNDLENGTH,        // xkan - get the length (in milliseconds) of the sound
	CG_S_GETCURRENTSOUNDTIME,
#endif // RTCW_XX

	CG_R_LOADWORLDMAP,
	CG_R_REGISTERMODEL,
	CG_R_REGISTERSKIN,
	CG_R_REGISTERSHADER,

	CG_R_GETSKINMODEL,      // client allowed to view what the .skin loaded so they can set their model appropriately
	CG_R_GETMODELSHADER,    // client allowed the shader handle for given model/surface (for things like debris inheriting shader from explosive)

	CG_R_REGISTERFONT,
	CG_R_CLEARSCENE,
	CG_R_ADDREFENTITYTOSCENE,
	CG_GET_ENTITY_TOKEN,
	CG_R_ADDPOLYTOSCENE,
// Ridah
	CG_R_ADDPOLYSTOSCENE,

#if defined RTCW_SP
	CG_RB_ZOMBIEFXADDNEWHIT,
#endif // RTCW_XX

// done.

#if defined RTCW_ET
	CG_R_ADDPOLYBUFFERTOSCENE,
#endif // RTCW_XX

	CG_R_ADDLIGHTTOSCENE,

	CG_R_ADDCORONATOSCENE,
	CG_R_SETFOG,

#if defined RTCW_ET
	CG_R_SETGLOBALFOG,
#endif // RTCW_XX


	CG_R_RENDERSCENE,

#if defined RTCW_ET
	CG_R_SAVEVIEWPARMS,
	CG_R_RESTOREVIEWPARMS,
#endif // RTCW_XX

	CG_R_SETCOLOR,
	CG_R_DRAWSTRETCHPIC,
	CG_R_DRAWSTRETCHPIC_GRADIENT,   //----(SA)	added
	CG_R_MODELBOUNDS,
	CG_R_LERPTAG,
	CG_GETGLCONFIG,
	CG_GETGAMESTATE,
	CG_GETCURRENTSNAPSHOTNUMBER,
	CG_GETSNAPSHOT,
	CG_GETSERVERCOMMAND,
	CG_GETCURRENTCMDNUMBER,
	CG_GETUSERCMD,
	CG_SETUSERCMDVALUE,

#if !defined RTCW_SP
	CG_SETCLIENTLERPORIGIN,         // DHM - Nerve
#endif // RTCW_XX

	CG_R_REGISTERSHADERNOMIP,
	CG_MEMORY_REMAINING,

	CG_KEY_ISDOWN,
	CG_KEY_GETCATCHER,
	CG_KEY_SETCATCHER,
	CG_KEY_GETKEY,

#if defined RTCW_ET
	CG_KEY_GETOVERSTRIKEMODE,
	CG_KEY_SETOVERSTRIKEMODE,
#endif // RTCW_XX

	CG_PC_ADD_GLOBAL_DEFINE,
	CG_PC_LOAD_SOURCE,
	CG_PC_FREE_SOURCE,
	CG_PC_READ_TOKEN,
	CG_PC_SOURCE_FILE_AND_LINE,

#if defined RTCW_ET
	CG_PC_UNREAD_TOKEN,
#endif // RTCW_XX

	CG_S_STOPBACKGROUNDTRACK,
	CG_REAL_TIME,
	CG_SNAPVECTOR,
	CG_REMOVECOMMAND,

#if defined RTCW_SP
//	CG_R_LIGHTFORPOINT,	// not currently used (sorry, trying to keep CG_MEMSET @ 100)
#else
	CG_R_LIGHTFORPOINT,
#endif // RTCW_XX

#if !defined RTCW_ET
	CG_SENDMOVESPEEDSTOGAME,
#endif // RTCW_XX

	CG_CIN_PLAYCINEMATIC,
	CG_CIN_STOPCINEMATIC,
	CG_CIN_RUNCINEMATIC,
	CG_CIN_DRAWCINEMATIC,
	CG_CIN_SETEXTENTS,
	CG_R_REMAP_SHADER,

#if defined RTCW_SP
//	CG_S_ADDREALLOOPINGSOUND,	// not currently used (sorry, trying to keep CG_MEMSET @ 100)
#else
	CG_S_ADDREALLOOPINGSOUND,
#endif // RTCW_XX

#if !defined RTCW_ET
	CG_S_STOPLOOPINGSOUND,
#endif // RTCW_XX

#if !defined RTCW_MP
	CG_S_STOPSTREAMINGSOUND,    //----(SA)	added
#endif // RTCW_XX

	CG_LOADCAMERA,
	CG_STARTCAMERA,

#if !defined RTCW_MP
	CG_STOPCAMERA,  //----(SA)	added
#endif // RTCW_XX

	CG_GETCAMERAINFO,

#if defined RTCW_SP
	CG_MEMSET = 110,
#elif defined RTCW_MP
	CG_MEMSET = 100,
#else
	CG_MEMSET = 150,
#endif // RTCW_XX

	CG_MEMCPY,
	CG_STRNCPY,
	CG_SIN,
	CG_COS,
	CG_ATAN2,
	CG_SQRT,
	CG_FLOOR,
	CG_CEIL,

	CG_TESTPRINTINT,
	CG_TESTPRINTFLOAT,
	CG_ACOS,

	CG_INGAME_POPUP,        //----(SA)	added

#if defined RTCW_SP
	CG_INGAME_CLOSEPOPUP,   // NERVE - SMF
	CG_LIMBOCHAT,           // NERVE - SMF

	CG_GETMODELINFO
#else
	// NERVE - SMF
	CG_INGAME_CLOSEPOPUP,
#endif // RTCW_XX

#if defined RTCW_MP
	CG_LIMBOCHAT,
#endif // RTCW_XX

#if !defined RTCW_SP
	CG_R_DRAWROTATEDPIC,
#endif // RTCW_XX

#if defined RTCW_ET
	CG_R_DRAW2DPOLYS,
#endif // RTCW_XX

#if !defined RTCW_SP
	CG_KEY_GETBINDINGBUF,
	CG_KEY_SETBINDING,
	CG_KEY_KEYNUMTOSTRINGBUF,
#endif // RTCW_XX

#if defined RTCW_ET
	CG_KEY_BINDINGTOKEYS,
#endif // RTCW_XX


#if defined RTCW_MP
	CG_TRANSLATE_STRING
#elif defined RTCW_ET
	CG_TRANSLATE_STRING,
#endif // RTCW_XX

	// -NERVE - SMF

#if defined RTCW_ET
	CG_R_INPVS,
	CG_GETHUNKDATA,

	CG_PUMPEVENTLOOP,

	// zinx
	CG_SENDMESSAGE,
	CG_MESSAGESTATUS,
	// -zinx

	// bani
	CG_R_LOADDYNAMICSHADER,
	// -bani

	// fretn
	CG_R_RENDERTOTEXTURE,
	// -fretn
	// bani
	CG_R_GETTEXTUREID,
	// -bani
	// bani
	CG_R_FINISH,
	// -bani
#endif // RTCW_XX

} cgameImport_t;


/*
==================================================================

functions exported to the main executable

==================================================================
*/

typedef enum {
	CG_INIT,
//	void CG_Init( int serverMessageNum, int serverCommandSequence )
	// called when the level loads or when the renderer is restarted
	// all media should be registered at this time
	// cgame will display loading status by calling SCR_Update, which
	// will call CG_DrawInformation during the loading process
	// reliableCommandSequence will be 0 on fresh loads, but higher for
	// demos, tourney restarts, or vid_restarts

	CG_SHUTDOWN,
//	void (*CG_Shutdown)( void );
	// oportunity to flush and close any open files

	CG_CONSOLE_COMMAND,
//	qboolean (*CG_ConsoleCommand)( void );
	// a console command has been issued locally that is not recognized by the
	// main game system.
	// use Cmd_Argc() / Cmd_Argv() to read the command, return qfalse if the
	// command is not known to the game

	CG_DRAW_ACTIVE_FRAME,
//	void (*CG_DrawActiveFrame)( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );
	// Generates and draws a game scene and status information at the given time.
	// If demoPlayback is set, local movement prediction will not be enabled

	CG_CROSSHAIR_PLAYER,
//	int (*CG_CrosshairPlayer)( void );

	CG_LAST_ATTACKER,
//	int (*CG_LastAttacker)( void );

	CG_KEY_EVENT,
//	void	(*CG_KeyEvent)( int key, qboolean down );

	CG_MOUSE_EVENT,
//	void	(*CG_MouseEvent)( int dx, int dy );
	CG_EVENT_HANDLING,

#if !defined RTCW_ET
//	void (*CG_EventHandling)(int type);
#else
//	void (*CG_EventHandling)(int type, qboolean fForced);
#endif // RTCW_XX

	CG_GET_TAG,
//	qboolean CG_GetTag( int clientNum, char *tagname, orientation_t *or );

#if defined RTCW_SP
	MAX_CGAME_EXPORT
#endif // RTCW_XX

#if defined RTCW_MP
	CG_CHECKCENTERVIEW,
//	qboolean CG_CheckCenterView();
#endif // RTCW_XX

#if defined RTCW_ET
	CG_CHECKEXECKEY,

	CG_WANTSBINDKEYS,

	// zinx
	CG_MESSAGERECEIVED,
//	void (*CG_MessageReceived)( const char *buf, int buflen, int serverTime );
	// -zinx
#endif // RTCW_XX

} cgameExport_t;

//----------------------------------------------
