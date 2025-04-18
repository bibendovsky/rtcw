/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#if defined RTCW_SP
// Copyright (C) 1999-2000 Id Software, Inc.
//
#endif // RTCW_XX

// g_public.h -- game module information visible to server

#define GAME_API_VERSION    8

// entity->svFlags
// the server does not know how to interpret most of the values
// in entityStates (level eType), so the game must explicitly flag
// special server behaviors
#define SVF_NOCLIENT            0x00000001  // don't send entity to clients, even if it has effects
#define SVF_VISDUMMY            0x00000004  // this ent is a "visibility dummy" and needs it's master to be sent to clients that can see it even if they can't see the master ent
#define SVF_BOT                 0x00000008

#if !defined RTCW_ET
// Wolfenstein
#define SVF_CASTAI              0x00000010
// done.
#endif // RTCW_XX

#if defined RTCW_ET
#define SVF_POW                 0x00000010  // Gordon: stole SVF_CASTAI as it's no longer used
#endif // RTCW_XX

#define SVF_BROADCAST           0x00000020  // send to all connected clients
#define SVF_PORTAL              0x00000040  // merge a second pvs at origin2 into snapshots

#if !defined RTCW_ET
#define SVF_USE_CURRENT_ORIGIN  0x00000080  // entity->r.currentOrigin instead of entity->s.origin
											// for link position (missiles and movers)
#endif // RTCW_XX

#if defined RTCW_ET
#define SVF_BLANK               0x00000080  // Gordon: removed SVF_USE_CURRENT_ORIGIN as it plain doesnt do anything
#endif // RTCW_XX

// Ridah
#define SVF_NOFOOTSTEPS         0x00000100
// done.
// MrE:
#define SVF_CAPSULE             0x00000200  // use capsule for collision detection

#define SVF_VISDUMMY_MULTIPLE   0x00000400  // so that one vis dummy can add to snapshot multiple speakers

// recent id changes
#define SVF_SINGLECLIENT        0x00000800  // only send to a single client (entityShared_t->singleClient)
#define SVF_NOSERVERINFO        0x00001000  // don't send CS_SERVERINFO updates to this client
											// so that it can be updated for ping tools without
											// lagging clients
#define SVF_NOTSINGLECLIENT     0x00002000  // send entity to everyone but one client
											// (entityShared_t->singleClient)

#if defined RTCW_ET
// Gordon:
#define SVF_IGNOREBMODELEXTENTS     0x00004000  // just use origin for in pvs check for snapshots, ignore the bmodel extents
#define SVF_SELF_PORTAL             0x00008000  // use self->origin2 as portal
#define SVF_SELF_PORTAL_EXCLUSIVE   0x00010000  // use self->origin2 as portal and DONT add self->origin PVS ents
#endif // RTCW_XX

//===============================================================

#if defined RTCW_ET
#define MAX_TEAM_LANDMINES  10

typedef qboolean ( *addToSnapshotCallback )( int entityNum, int clientNum );
#endif // RTCW_XX

typedef struct {

#if !defined RTCW_ET
	entityState_t s;                // communicated by server to clients
#else
//	entityState_t	s;				// communicated by server to clients
#endif // RTCW_XX

	qboolean linked;                // qfalse if not in any good cluster
	int linkcount;

	int svFlags;                    // SVF_NOCLIENT, SVF_BROADCAST, etc
	int singleClient;               // only send to this client when SVF_SINGLECLIENT is set

	qboolean bmodel;                // if false, assume an explicit mins / maxs bounding box
									// only set by trap_SetBrushModel
	vec3_t mins, maxs;
	int contents;                   // CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0

	vec3_t absmin, absmax;          // derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t currentOrigin;
	vec3_t currentAngles;

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->s.ownerNum = passEntityNum	(don't interact with your own missiles)
	// entity[ent->s.ownerNum].ownerNum = passEntityNum	(don't interact with other missiles from owner)
	int ownerNum;
	int eventTime;

#if !defined RTCW_SP
	int worldflags;             // DHM - Nerve
#endif // RTCW_XX

#if defined RTCW_ET
	qboolean snapshotCallback;
#endif // RTCW_XX

} entityShared_t;



// the server looks at a sharedEntity, which is the start of the game's gentity_t structure
typedef struct {
	entityState_t s;                // communicated by server to clients
	entityShared_t r;               // shared by both the server system and game
} sharedEntity_t;



//===============================================================

//
// system traps provided by the main engine
//
typedef enum {
	//============== general Quake services ==================

	G_PRINT,        // ( const char *string );
	// print message on the local console

	G_ERROR,        // ( const char *string );
	// abort the game

#if defined RTCW_SP
	G_ENDGAME,      // ( void );	//----(SA)	added
	// exit to main menu and start "endgame" menu
#endif // RTCW_XX

	G_MILLISECONDS, // ( void );
	// get current time for profiling reasons
	// this should NOT be used for any game related tasks,
	// because it is not journaled

	// console variable interaction
	G_CVAR_REGISTER,    // ( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
	G_CVAR_UPDATE,  // ( vmCvar_t *vmCvar );
	G_CVAR_SET,     // ( const char *var_name, const char *value );
	G_CVAR_VARIABLE_INTEGER_VALUE,  // ( const char *var_name );

	G_CVAR_VARIABLE_STRING_BUFFER,  // ( const char *var_name, char *buffer, int bufsize );

#if defined RTCW_ET
	G_CVAR_LATCHEDVARIABLESTRINGBUFFER,
#endif // RTCW_XX

	G_ARGC,         // ( void );
	// ClientCommand and ServerCommand parameter access

	G_ARGV,         // ( int n, char *buffer, int bufferLength );

	G_FS_FOPEN_FILE,    // ( const char *qpath, fileHandle_t *file, fsMode_t mode );
	G_FS_READ,      // ( void *buffer, int len, fileHandle_t f );
	G_FS_WRITE,     // ( const void *buffer, int len, fileHandle_t f );
	G_FS_RENAME,
	G_FS_FCLOSE_FILE,       // ( fileHandle_t f );

	G_SEND_CONSOLE_COMMAND, // ( const char *text );
	// add commands to the console as if they were typed in
	// for map changing, etc


	//=========== server specific functionality =============

	G_LOCATE_GAME_DATA,     // ( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
	//							playerState_t *clients, int sizeofGameClient );
	// the game needs to let the server system know where and how big the gentities
	// are, so it can look at them directly without going through an interface

	G_DROP_CLIENT,      // ( int clientNum, const char *reason );
	// kick a client off the server with a message

	G_SEND_SERVER_COMMAND,  // ( int clientNum, const char *fmt, ... );
	// reliably sends a command string to be interpreted by the given
	// client.  If clientNum is -1, it will be sent to all clients

	G_SET_CONFIGSTRING, // ( int num, const char *string );
	// config strings hold all the index strings, and various other information
	// that is reliably communicated to all clients
	// All of the current configstrings are sent to clients when
	// they connect, and changes are sent to all connected clients.
	// All confgstrings are cleared at each level start.

	G_GET_CONFIGSTRING, // ( int num, char *buffer, int bufferSize );

	G_GET_USERINFO,     // ( int num, char *buffer, int bufferSize );
	// userinfo strings are maintained by the server system, so they
	// are persistant across level loads, while all other game visible
	// data is completely reset

	G_SET_USERINFO,     // ( int num, const char *buffer );

	G_GET_SERVERINFO,   // ( char *buffer, int bufferSize );
	// the serverinfo info string has all the cvars visible to server browsers

	G_SET_BRUSH_MODEL,  // ( gentity_t *ent, const char *name );
	// sets mins and maxs based on the brushmodel name

	G_TRACE,    // ( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
	// collision detection against all linked entities

	G_POINT_CONTENTS,   // ( const vec3_t point, int passEntityNum );
	// point contents against all linked entities

	G_IN_PVS,           // ( const vec3_t p1, const vec3_t p2 );

	G_IN_PVS_IGNORE_PORTALS,    // ( const vec3_t p1, const vec3_t p2 );

	G_ADJUST_AREA_PORTAL_STATE, // ( gentity_t *ent, qboolean open );

	G_AREAS_CONNECTED,  // ( int area1, int area2 );

	G_LINKENTITY,       // ( gentity_t *ent );
	// an entity will never be sent to a client or used for collision
	// if it is not passed to linkentity.  If the size, position, or
	// solidity changes, it must be relinked.

	G_UNLINKENTITY,     // ( gentity_t *ent );
	// call before removing an interactive entity

	G_ENTITIES_IN_BOX,  // ( const vec3_t mins, const vec3_t maxs, gentity_t **list, int maxcount );
	// EntitiesInBox will return brush models based on their bounding box,
	// so exact determination must still be done with EntityContact

	G_ENTITY_CONTACT,   // ( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
	// perform an exact check against inline brush models of non-square shape

	// access for bots to get and free a server client (FIXME?)

#if !defined RTCW_ET
	G_BOT_ALLOCATE_CLIENT,  // ( void );
#else
	G_BOT_ALLOCATE_CLIENT,  // ( int clientNum );
#endif // RTCW_XX

	G_BOT_FREE_CLIENT,  // ( int clientNum );

	G_GET_USERCMD,  // ( int clientNum, usercmd_t *cmd )

	G_GET_ENTITY_TOKEN, // qboolean ( char *buffer, int bufferSize )
	// Retrieves the next string token from the entity spawn text, returning
	// false when all tokens have been parsed.
	// This should only be done at GAME_INIT time.

	G_FS_GETFILELIST,
	G_DEBUG_POLYGON_CREATE,
	G_DEBUG_POLYGON_DELETE,
	G_REAL_TIME,
	G_SNAPVECTOR,
// MrE:

	G_TRACECAPSULE, // ( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
	// collision detection using capsule against all linked entities

	G_ENTITY_CONTACTCAPSULE,    // ( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
	// perform an exact check against inline brush models of non-square shape
// done.

	G_GETTAG,

#if defined RTCW_ET
	G_REGISTERTAG,
	// Gordon: load a serverside tag

	G_REGISTERSOUND,    // xkan, 10/28/2002 - register the sound
	G_GET_SOUND_LENGTH, // xkan, 10/28/2002 - get the length of the sound
#endif // RTCW_XX

	BOTLIB_SETUP = 200,             // ( void );
	BOTLIB_SHUTDOWN,                // ( void );
	BOTLIB_LIBVAR_SET,
	BOTLIB_LIBVAR_GET,
	BOTLIB_PC_ADD_GLOBAL_DEFINE,
	BOTLIB_START_FRAME,
	BOTLIB_LOAD_MAP,
	BOTLIB_UPDATENTITY,
	BOTLIB_TEST,

	BOTLIB_GET_SNAPSHOT_ENTITY,     // ( int client, int ent );
	BOTLIB_GET_CONSOLE_MESSAGE,     // ( int client, char *message, int size );
	BOTLIB_USER_COMMAND,            // ( int client, usercmd_t *ucmd );

	BOTLIB_AAS_ENTITY_VISIBLE = 300,    //FIXME: remove
	BOTLIB_AAS_IN_FIELD_OF_VISION,      //FIXME: remove
	BOTLIB_AAS_VISIBLE_CLIENTS,         //FIXME: remove
	BOTLIB_AAS_ENTITY_INFO,

	BOTLIB_AAS_INITIALIZED,
	BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX,
	BOTLIB_AAS_TIME,

	// Ridah
	BOTLIB_AAS_SETCURRENTWORLD,
	// done.

	BOTLIB_AAS_POINT_AREA_NUM,
	BOTLIB_AAS_TRACE_AREAS,

#if defined RTCW_ET
	BOTLIB_AAS_BBOX_AREAS,
	BOTLIB_AAS_AREA_CENTER,
	BOTLIB_AAS_AREA_WAYPOINT,
#endif // RTCW_XX

	BOTLIB_AAS_POINT_CONTENTS,
	BOTLIB_AAS_NEXT_BSP_ENTITY,
	BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY,
	BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY,
	BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY,
	BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY,

	BOTLIB_AAS_AREA_REACHABILITY,

#if defined RTCW_ET
	BOTLIB_AAS_AREA_LADDER,
#endif // RTCW_XX

	BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA,

	BOTLIB_AAS_SWIMMING,
	BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT,

	// Ridah, route-tables
	BOTLIB_AAS_RT_SHOWROUTE,

#if !defined RTCW_ET
	BOTLIB_AAS_RT_GETHIDEPOS,
	BOTLIB_AAS_FINDATTACKSPOTWITHINRANGE,
#else
	//BOTLIB_AAS_RT_GETHIDEPOS,
	//BOTLIB_AAS_FINDATTACKSPOTWITHINRANGE,
#endif // RTCW_XX

#if defined RTCW_SP
	BOTLIB_AAS_GETROUTEFIRSTVISPOS,
#endif // RTCW_XX

#if defined RTCW_ET
	BOTLIB_AAS_NEARESTHIDEAREA,
	BOTLIB_AAS_LISTAREASINRANGE,
	BOTLIB_AAS_AVOIDDANGERAREA,
	BOTLIB_AAS_RETREAT,
	BOTLIB_AAS_ALTROUTEGOALS,
#endif // RTCW_XX

	BOTLIB_AAS_SETAASBLOCKINGENTITY,

#if defined RTCW_ET
	BOTLIB_AAS_RECORDTEAMDEATHAREA,
#endif // RTCW_XX

	// done.

	BOTLIB_EA_SAY = 400,
	BOTLIB_EA_SAY_TEAM,
	BOTLIB_EA_USE_ITEM,
	BOTLIB_EA_DROP_ITEM,
	BOTLIB_EA_USE_INV,
	BOTLIB_EA_DROP_INV,
	BOTLIB_EA_GESTURE,
	BOTLIB_EA_COMMAND,

	BOTLIB_EA_SELECT_WEAPON,
	BOTLIB_EA_TALK,
	BOTLIB_EA_ATTACK,
	BOTLIB_EA_RELOAD,
	BOTLIB_EA_USE,
	BOTLIB_EA_RESPAWN,
	BOTLIB_EA_JUMP,
	BOTLIB_EA_DELAYED_JUMP,
	BOTLIB_EA_CROUCH,

#if defined RTCW_ET
	BOTLIB_EA_WALK,
#endif // RTCW_XX

	BOTLIB_EA_MOVE_UP,
	BOTLIB_EA_MOVE_DOWN,
	BOTLIB_EA_MOVE_FORWARD,
	BOTLIB_EA_MOVE_BACK,
	BOTLIB_EA_MOVE_LEFT,
	BOTLIB_EA_MOVE_RIGHT,
	BOTLIB_EA_MOVE,
	BOTLIB_EA_VIEW,

#if defined RTCW_ET
	// START	xkan, 9/16/2002
	BOTLIB_EA_PRONE,
	// END		xkan, 9/16/2002
#endif // RTCW_XX

	BOTLIB_EA_END_REGULAR,
	BOTLIB_EA_GET_INPUT,
	BOTLIB_EA_RESET_INPUT,


	BOTLIB_AI_LOAD_CHARACTER = 500,
	BOTLIB_AI_FREE_CHARACTER,
	BOTLIB_AI_CHARACTERISTIC_FLOAT,
	BOTLIB_AI_CHARACTERISTIC_BFLOAT,
	BOTLIB_AI_CHARACTERISTIC_INTEGER,
	BOTLIB_AI_CHARACTERISTIC_BINTEGER,
	BOTLIB_AI_CHARACTERISTIC_STRING,

	BOTLIB_AI_ALLOC_CHAT_STATE,
	BOTLIB_AI_FREE_CHAT_STATE,
	BOTLIB_AI_QUEUE_CONSOLE_MESSAGE,
	BOTLIB_AI_REMOVE_CONSOLE_MESSAGE,
	BOTLIB_AI_NEXT_CONSOLE_MESSAGE,
	BOTLIB_AI_NUM_CONSOLE_MESSAGE,
	BOTLIB_AI_INITIAL_CHAT,
	BOTLIB_AI_REPLY_CHAT,
	BOTLIB_AI_CHAT_LENGTH,
	BOTLIB_AI_ENTER_CHAT,
	BOTLIB_AI_STRING_CONTAINS,
	BOTLIB_AI_FIND_MATCH,
	BOTLIB_AI_MATCH_VARIABLE,
	BOTLIB_AI_UNIFY_WHITE_SPACES,
	BOTLIB_AI_REPLACE_SYNONYMS,
	BOTLIB_AI_LOAD_CHAT_FILE,
	BOTLIB_AI_SET_CHAT_GENDER,
	BOTLIB_AI_SET_CHAT_NAME,

	BOTLIB_AI_RESET_GOAL_STATE,
	BOTLIB_AI_RESET_AVOID_GOALS,
	BOTLIB_AI_PUSH_GOAL,
	BOTLIB_AI_POP_GOAL,
	BOTLIB_AI_EMPTY_GOAL_STACK,
	BOTLIB_AI_DUMP_AVOID_GOALS,
	BOTLIB_AI_DUMP_GOAL_STACK,
	BOTLIB_AI_GOAL_NAME,
	BOTLIB_AI_GET_TOP_GOAL,
	BOTLIB_AI_GET_SECOND_GOAL,
	BOTLIB_AI_CHOOSE_LTG_ITEM,
	BOTLIB_AI_CHOOSE_NBG_ITEM,
	BOTLIB_AI_TOUCHING_GOAL,
	BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE,
	BOTLIB_AI_GET_LEVEL_ITEM_GOAL,
	BOTLIB_AI_AVOID_GOAL_TIME,
	BOTLIB_AI_INIT_LEVEL_ITEMS,
	BOTLIB_AI_UPDATE_ENTITY_ITEMS,
	BOTLIB_AI_LOAD_ITEM_WEIGHTS,
	BOTLIB_AI_FREE_ITEM_WEIGHTS,
	BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC,
	BOTLIB_AI_ALLOC_GOAL_STATE,
	BOTLIB_AI_FREE_GOAL_STATE,

	BOTLIB_AI_RESET_MOVE_STATE,
	BOTLIB_AI_MOVE_TO_GOAL,
	BOTLIB_AI_MOVE_IN_DIRECTION,
	BOTLIB_AI_RESET_AVOID_REACH,
	BOTLIB_AI_RESET_LAST_AVOID_REACH,
	BOTLIB_AI_REACHABILITY_AREA,
	BOTLIB_AI_MOVEMENT_VIEW_TARGET,
	BOTLIB_AI_ALLOC_MOVE_STATE,
	BOTLIB_AI_FREE_MOVE_STATE,
	BOTLIB_AI_INIT_MOVE_STATE,
	// Ridah
	BOTLIB_AI_INIT_AVOID_REACH,
	// done.

	BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON,
	BOTLIB_AI_GET_WEAPON_INFO,
	BOTLIB_AI_LOAD_WEAPON_WEIGHTS,
	BOTLIB_AI_ALLOC_WEAPON_STATE,
	BOTLIB_AI_FREE_WEAPON_STATE,
	BOTLIB_AI_RESET_WEAPON_STATE,

	BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION,
	BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC,
	BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC,
	BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL,
	BOTLIB_AI_GET_MAP_LOCATION_GOAL,
	BOTLIB_AI_NUM_INITIAL_CHATS,
	BOTLIB_AI_GET_CHAT_MESSAGE,
	BOTLIB_AI_REMOVE_FROM_AVOID_GOALS,
	BOTLIB_AI_PREDICT_VISIBLE_POSITION,

	BOTLIB_AI_SET_AVOID_GOAL_TIME,
	BOTLIB_AI_ADD_AVOID_SPOT,
	BOTLIB_AAS_ALTERNATIVE_ROUTE_GOAL,
	BOTLIB_AAS_PREDICT_ROUTE,
	BOTLIB_AAS_POINT_REACHABILITY_AREA_INDEX,

	BOTLIB_PC_LOAD_SOURCE,
	BOTLIB_PC_FREE_SOURCE,
	BOTLIB_PC_READ_TOKEN,

#if defined RTCW_SP
	BOTLIB_PC_SOURCE_FILE_AND_LINE,

	G_FS_COPY_FILE  //DAJ
#elif defined RTCW_MP
	BOTLIB_PC_SOURCE_FILE_AND_LINE
#else
	BOTLIB_PC_SOURCE_FILE_AND_LINE,
#endif // RTCW_XX

#if defined RTCW_ET
	BOTLIB_PC_UNREAD_TOKEN,

	PB_STAT_REPORT,

	// zinx
	G_SENDMESSAGE,
	G_MESSAGESTATUS,
	// -zinx
#endif // RTCW_XX

} gameImport_t;


//
// functions exported by the game subsystem
//
typedef enum {
	GAME_INIT,  // ( int levelTime, int randomSeed, int restart );
	// init and shutdown will be called every single level
	// The game should call G_GET_ENTITY_TOKEN to parse through all the
	// entity configuration text and spawn gentities.

	GAME_SHUTDOWN,  // (void);

	GAME_CLIENT_CONNECT,    // ( int clientNum, qboolean firstTime, qboolean isBot );
	// return NULL if the client is allowed to connect, otherwise return
	// a text string with the reason for denial

	GAME_CLIENT_BEGIN,              // ( int clientNum );

	GAME_CLIENT_USERINFO_CHANGED,   // ( int clientNum );

	GAME_CLIENT_DISCONNECT,         // ( int clientNum );

	GAME_CLIENT_COMMAND,            // ( int clientNum );

	GAME_CLIENT_THINK,              // ( int clientNum );

	GAME_RUN_FRAME,                 // ( int levelTime );

	GAME_CONSOLE_COMMAND,           // ( void );
	// ConsoleCommand will be called when a command has been issued
	// that is not recognized as a builtin function.
	// The game can issue trap_argc() / trap_argv() commands to get the command
	// and parameters.  Return qfalse if the game doesn't recognize it as a command.

#if defined RTCW_SP
	BOTAI_START_FRAME,              // ( int time );

	// Ridah, Cast AI
	AICAST_VISIBLEFROMPOS,
	AICAST_CHECKATTACKATPOS,
	// done.

	GAME_RETRIEVE_MOVESPEEDS_FROM_CLIENT,
	GAME_GETMODELINFO
#elif defined RTCW_MP
	BOTAI_START_FRAME               // ( int time );

	// Ridah, Cast AI
	,AICAST_VISIBLEFROMPOS
	,AICAST_CHECKATTACKATPOS
	// done.

	,GAME_RETRIEVE_MOVESPEEDS_FROM_CLIENT
#else
	GAME_SNAPSHOT_CALLBACK,         // ( int entityNum, int clientNum ); // return qfalse if you don't want it to be added

	BOTAI_START_FRAME,              // ( int time );

	// Ridah, Cast AI
	BOT_VISIBLEFROMPOS,
	BOT_CHECKATTACKATPOS,
	// done.

	// zinx
	GAME_MESSAGERECEIVED,           // ( int cno, const char *buf, int buflen, int commandTime );
	// -zinx
#endif // RTCW_XX

} gameExport_t;

