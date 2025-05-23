/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// server.h

#include "q_shared.h"
#include "qcommon.h"
#include "g_public.h"
#include "bg_public.h"

#if defined RTCW_ET
//bani
#ifdef __GNUC__
#define _attribute( x ) __attribute__( x )
#else
#define _attribute( x )
#endif
#endif // RTCW_XX

//=============================================================================

#define PERS_SCORE              0       // !!! MUST NOT CHANGE, SERVER AND
										// GAME BOTH REFERENCE !!!

#define MAX_ENT_CLUSTERS    16

#if !defined RTCW_SP
#define MAX_BPS_WINDOW      20          // NERVE - SMF - net debugging
#endif // RTCW_XX

typedef struct svEntity_s {
	struct worldSector_s *worldSector;
	struct svEntity_s *nextEntityInWorldSector;

	entityState_t baseline;         // for delta compression of initial sighting
	int numClusters;                // if -1, use headnode instead
	int clusternums[MAX_ENT_CLUSTERS];
	int lastCluster;                // if all the clusters don't fit in clusternums
	int areanum, areanum2;
	int snapshotCounter;            // used to prevent double adding from portal views

#if defined RTCW_ET
	int originCluster;              // Gordon: calced upon linking, for origin only bmodel vis checks
#endif // RTCW_XX

} svEntity_t;

typedef enum {
	SS_DEAD,            // no map loaded
	SS_LOADING,         // spawning level entities
	SS_GAME             // actively running
} serverState_t;

typedef struct {
	serverState_t state;
	qboolean restarting;                // if true, send configstring changes during SS_LOADING
	int serverId;                       // changes each server start
	int restartedServerId;              // serverId before a map_restart

#if defined RTCW_SP
	int checksumFeed;                   //
#else
	int checksumFeed;                   // the feed key that we use to compute the pure checksum strings
	// show_bug.cgi?id=475
	// the serverId associated with the current checksumFeed (always <= serverId)
	int checksumFeedServerId;
#endif // RTCW_XX

	int snapshotCounter;                // incremented for each snapshot built
	int timeResidual;                   // <= 1000 / sv_frame->value
	int nextFrameTime;                  // when time > nextFrameTime, process world
	struct cmodel_s *models[MAX_MODELS];
	char            *configstrings[MAX_CONFIGSTRINGS];

#if defined RTCW_ET
	qboolean configstringsmodified[MAX_CONFIGSTRINGS];
#endif // RTCW_XX

	svEntity_t svEntities[MAX_GENTITIES];

	char            *entityParsePoint;  // used during game VM init

	// the game virtual machine will update these on init and changes
	sharedEntity_t  *gentities;
	int gentitySize;
	int num_entities;                   // current number, <= MAX_GENTITIES

	playerState_t   *gameClients;
	int gameClientSize;                 // will be > sizeof(playerState_t) due to game private data

	int restartTime;

#if !defined RTCW_SP
	// NERVE - SMF - net debugging
	int bpsWindow[MAX_BPS_WINDOW];
	int bpsWindowSteps;
	int bpsTotalBytes;
	int bpsMaxBytes;

	int ubpsWindow[MAX_BPS_WINDOW];
	int ubpsTotalBytes;
	int ubpsMaxBytes;

	float ucompAve;
	int ucompNum;
	// -NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
	md3Tag_t tags[MAX_SERVER_TAGS];
	tagHeaderExt_t tagHeadersExt[MAX_TAG_FILES];

	int num_tagheaders;
	int num_tags;
#endif // RTCW_XX

} server_t;





typedef struct {
	int areabytes;
	byte areabits[MAX_MAP_AREA_BYTES];                  // portalarea visibility bits
	playerState_t ps;
	int num_entities;
	int first_entity;                   // into the circular sv_packet_entities[]
										// the entities MUST be in increasing state number
										// order, otherwise the delta compression will fail
	int messageSent;                    // time the message was transmitted
	int messageAcked;                   // time the message was acked
	int messageSize;                    // used to rate drop packets
} clientSnapshot_t;

typedef enum {
	CS_FREE,        // can be reused for a new connection
	CS_ZOMBIE,      // client has been disconnected, but don't reuse
					// connection for a couple seconds
	CS_CONNECTED,   // has been assigned to a client_t, but no gamestate yet
	CS_PRIMED,      // gamestate has been sent, but client hasn't sent a usercmd
	CS_ACTIVE       // client is fully in game
} clientState_t;

#if defined RTCW_SP
// RF, now using a global string buffer to hold all reliable commands
//#define	RELIABLE_COMMANDS_MULTI		128
//#define	RELIABLE_COMMANDS_SINGLE	256		// need more for loadgame situations

#define RELIABLE_COMMANDS_CHARS     384     // we can scale this down from the max of 1024, since not all commands are going to use that many chars

typedef struct {
	int bufSize;
	char    *buf;               // actual strings
	char    **commands;         // pointers to actual strings
	int     *commandLengths;    // lengths of actual strings
	//
	char    *rover;
} reliableCommands_t;
#else
typedef struct netchan_buffer_s {
	msg_t msg;
	byte msgBuffer[MAX_MSGLEN];

#if defined RTCW_ET
	char lastClientCommandString[MAX_STRING_CHARS];
#endif // RTCW_XX

	struct netchan_buffer_s *next;
} netchan_buffer_t;
#endif // RTCW_XX

typedef struct client_s {
	clientState_t state;
	char userinfo[MAX_INFO_STRING];                 // name, etc

#if defined RTCW_SP
	//char			reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];
	reliableCommands_t reliableCommands;
#else
	char reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];
#endif // RTCW_XX

	int reliableSequence;                   // last added reliable message, not necesarily sent or acknowledged yet
	int reliableAcknowledge;                // last acknowledged reliable message
	int reliableSent;                       // last sent reliable message, not necesarily acknowledged yet
	int messageAcknowledge;

#if defined RTCW_ET
	int binaryMessageLength;
	char binaryMessage[MAX_BINARY_MESSAGE];
	qboolean binaryMessageOverflowed;
#endif // RTCW_XX

	int gamestateMessageNum;                // netchan->outgoingSequence of gamestate
	int challenge;

	usercmd_t lastUsercmd;
	int lastMessageNum;                 // for delta compression
	int lastClientCommand;              // reliable client message sequence
	char lastClientCommandString[MAX_STRING_CHARS];
	sharedEntity_t  *gentity;           // SV_GentityNum(clientnum)
	char name[MAX_NAME_LENGTH];                     // extracted from userinfo, high bits masked

	// downloading
	char downloadName[MAX_QPATH];            // if not empty string, we are downloading
	fileHandle_t download;              // file being downloaded
	int downloadSize;                   // total bytes (can't use EOF because of paks)
	int downloadCount;                  // bytes sent
	int downloadClientBlock;                // last block we sent to the client, awaiting ack
	int downloadCurrentBlock;               // current block number
	int downloadXmitBlock;              // last block we xmited
	unsigned char   *downloadBlocks[MAX_DOWNLOAD_WINDOW];   // the buffers for the download blocks
	int downloadBlockSize[MAX_DOWNLOAD_WINDOW];
	qboolean downloadEOF;               // We have sent the EOF block
	int downloadSendTime;               // time we last got an ack from the client

#if defined RTCW_ET
	// www downloading
	qboolean bDlOK;    // passed from cl_wwwDownload CVAR_USERINFO, wether this client supports www dl
	char downloadURL[MAX_OSPATH];            // the URL we redirected the client to
	qboolean bWWWDl;    // we have a www download going
	qboolean bWWWing;    // the client is doing an ftp/http download
	qboolean bFallback;    // last www download attempt failed, fallback to regular download
	// note: this is one-shot, multiple downloads would cause a www download to be attempted again
#endif // RTCW_XX

	int deltaMessage;                   // frame last client usercmd message
	int nextReliableTime;               // svs.time when another reliable command will be allowed
	int lastPacketTime;                 // svs.time when packet was last received
	int lastConnectTime;                // svs.time when connection started
	int nextSnapshotTime;               // send another snapshot when svs.time >= nextSnapshotTime
	qboolean rateDelayed;               // true if nextSnapshotTime was set based on rate instead of snapshotMsec
	int timeoutCount;                   // must timeout a few frames in a row so debugging doesn't break
	clientSnapshot_t frames[PACKET_BACKUP];     // updates can be delta'd from here
	int ping;
	int rate;                           // bytes / second
	int snapshotMsec;                   // requests a snapshot every snapshotMsec unless rate choked
	int pureAuthentic;

#if !defined RTCW_SP
	qboolean gotCP;  // TTimo - additional flag to distinguish between a bad pure checksum, and no cp command at all
#endif // RTCW_XX

	netchan_t netchan;

#if !defined RTCW_SP
	// TTimo
	// queuing outgoing fragmented messages to send them properly, without udp packet bursts
	// in case large fragmented messages are stacking up
	// buffer them into this queue, and hand them out to netchan as needed
	netchan_buffer_t *netchan_start_queue;

#if !defined RTCW_ET
	netchan_buffer_t **netchan_end_queue;
#else
	//%	netchan_buffer_t **netchan_end_queue;
	netchan_buffer_t *netchan_end_queue;
#endif // RTCW_XX

#endif // RTCW_XX

#if defined RTCW_ET
	//bani
	int downloadnotify;
#endif // RTCW_XX

} client_t;

//=============================================================================


// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define MAX_CHALLENGES  1024

#define AUTHORIZE_TIMEOUT   5000

typedef struct {
	netadr_t adr;
	int challenge;
	int time;                       // time the last packet was sent to the autherize server
	int pingTime;                   // time the challenge response was sent to client
	int firstTime;                  // time the adr was first used, for authorize timeout checks

#if !defined RTCW_SP
	int firstPing;                  // Used for min and max ping checks
	qboolean connected;
#endif // RTCW_XX

} challenge_t;

#if defined RTCW_ET
typedef struct tempBan_s {
	netadr_t adr;
	int endtime;
} tempBan_t;
#endif // RTCW_XX

#define MAX_MASTERS 8               // max recipients for heartbeat packets

#if defined RTCW_ET
#define MAX_TEMPBAN_ADDRESSES               MAX_CLIENTS

#define SERVER_PERFORMANCECOUNTER_FRAMES    600
#define SERVER_PERFORMANCECOUNTER_SAMPLES   6
#endif // RTCW_XX

// this structure will be cleared only when the game dll changes
typedef struct {
	qboolean initialized;                   // sv_init has completed

	int time;                               // will be strictly increasing across level changes

	int snapFlagServerBit;                  // ^= SNAPFLAG_SERVERCOUNT every SV_SpawnServer()

	client_t    *clients;                   // [sv_maxclients->integer];
	int numSnapshotEntities;                // sv_maxclients->integer*PACKET_BACKUP*MAX_PACKET_ENTITIES
	int nextSnapshotEntities;               // next snapshotEntities to use
	entityState_t   *snapshotEntities;      // [numSnapshotEntities]
	int nextHeartbeatTime;
	challenge_t challenges[MAX_CHALLENGES]; // to prevent invalid IPs from connecting
	netadr_t redirectAddress;               // for rcon return messages

#if defined RTCW_ET
	tempBan_t tempBanAddresses[MAX_TEMPBAN_ADDRESSES];
#endif // RTCW_XX

#if !defined RTCW_ET || (defined RTCW_ET && defined AUTHORIZE_SUPPORT)
	netadr_t authorizeAddress;              // for rcon return messages
#endif // RTCW_XX

#if defined RTCW_ET
	int sampleTimes[SERVER_PERFORMANCECOUNTER_SAMPLES];
	int currentSampleIndex;
	int totalFrameTime;
	int currentFrameIndex;
	int serverLoad;
#endif // RTCW_XX

} serverStatic_t;

#if defined RTCW_MP
//================
// DHM - Nerve
#ifdef UPDATE_SERVER

typedef struct {
	char version[MAX_QPATH];
	char platform[MAX_QPATH];
	char installer[MAX_QPATH];
} versionMapping_t;


#define MAX_UPDATE_VERSIONS 128
extern versionMapping_t versionMap[MAX_UPDATE_VERSIONS];
extern int numVersions;
// Maps client version to appropriate installer

#endif
// DHM - Nerve
#endif // RTCW_XX

//=============================================================================

extern serverStatic_t svs;                  // persistant server info across maps
extern server_t sv;                         // cleared each map
extern vm_t            *gvm;                // game virtual machine


#define MAX_MASTER_SERVERS  5

extern cvar_t  *sv_fps;
extern cvar_t  *sv_timeout;
extern cvar_t  *sv_zombietime;
extern cvar_t  *sv_rconPassword;
extern cvar_t  *sv_privatePassword;
extern cvar_t  *sv_allowDownload;

#if !defined RTCW_SP
extern cvar_t  *sv_friendlyFire;        // NERVE - SMF
extern cvar_t  *sv_maxlives;            // NERVE - SMF

#if !defined RTCW_ET
extern cvar_t  *sv_tourney;             // NERVE - SMF
#endif // RTCW_XX

#endif // RTCW_XX

extern cvar_t  *sv_maxclients;

#if defined RTCW_ET
extern cvar_t  *sv_needpass;
#endif // RTCW_XX

extern cvar_t  *sv_privateClients;
extern cvar_t  *sv_hostname;
extern cvar_t  *sv_master[MAX_MASTER_SERVERS];
extern cvar_t  *sv_reconnectlimit;

#if defined RTCW_ET
extern cvar_t  *sv_tempbanmessage;
#endif // RTCW_XX

extern cvar_t  *sv_showloss;
extern cvar_t  *sv_padPackets;
extern cvar_t  *sv_killserver;
extern cvar_t  *sv_mapname;
extern cvar_t  *sv_mapChecksum;
extern cvar_t  *sv_serverid;
extern cvar_t  *sv_maxRate;
extern cvar_t  *sv_minPing;
extern cvar_t  *sv_maxPing;

#if !defined RTCW_ET
extern cvar_t  *sv_gametype;
#else
//extern	cvar_t	*sv_gametype;
#endif // RTCW_XX

extern cvar_t  *sv_pure;
extern cvar_t  *sv_floodProtect;
extern cvar_t  *sv_allowAnonymous;

#if !defined RTCW_SP
extern cvar_t  *sv_lanForceRate;
extern cvar_t  *sv_onlyVisibleClients;

extern cvar_t  *sv_showAverageBPS;          // NERVE - SMF - net debugging
#endif // RTCW_XX

#if defined RTCW_ET
extern cvar_t* g_gameType;
#endif // RTCW_XX

// Rafael gameskill

#if !defined RTCW_ET
extern cvar_t  *sv_gameskill;
#else
//extern	cvar_t	*sv_gameskill;
#endif // RTCW_XX

// done

#if !defined RTCW_MP
extern cvar_t  *sv_reloading;   //----(SA)	added
#endif // RTCW_XX

#if !defined RTCW_SP
// TTimo - autodl
extern cvar_t *sv_dl_maxRate;
#endif // RTCW_XX

#if defined RTCW_ET
// TTimo
extern cvar_t *sv_wwwDownload; // general flag to enable/disable www download redirects
extern cvar_t *sv_wwwBaseURL; // the base URL of all the files
// tell clients to perform their downloads while disconnected from the server
// this gets you a better throughput, but you loose the ability to control the download usage
extern cvar_t *sv_wwwDlDisconnected;
extern cvar_t *sv_wwwFallbackURL;

//bani
extern cvar_t *sv_cheats;
extern cvar_t *sv_packetloss;
extern cvar_t *sv_packetdelay;

//fretn
extern cvar_t *sv_fullmsg;
#endif // RTCW_XX

//===========================================================

//
// sv_main.c
//

#if !defined RTCW_ET
void SV_FinalMessage( const char *message );
#endif // RTCW_XX

#if defined RTCW_ET
void SV_FinalCommand( const char *cmd, qboolean disconnect ); // ydnar: added disconnect flag so map changes can use this function as well
#endif // RTCW_XX

void QDECL SV_SendServerCommand( client_t *cl, const char *fmt, ... );


void SV_AddOperatorCommands( void );
void SV_RemoveOperatorCommands( void );


#if defined RTCW_SP
void SV_MasterHeartbeat( void );
#else
void SV_MasterHeartbeat( const char *hbname );
#endif // RTCW_XX

void SV_MasterShutdown( void );


#if !defined RTCW_SP
void SV_MasterGameCompleteStatus();     // NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
//bani - bugtraq 12534
qboolean SV_VerifyChallenge( const char *challenge );
#endif // RTCW_XX


//
// sv_init.c
//

#if defined RTCW_ET
void SV_SetConfigstringNoUpdate( int index, const char *val );
#endif // RTCW_XX

void SV_SetConfigstring( int index, const char *val );

#if defined RTCW_ET
void SV_UpdateConfigStrings( void );
#endif // RTCW_XX

void SV_GetConfigstring( int index, char *buffer, int bufferSize );

void SV_SetUserinfo( int index, const char *val );
void SV_GetUserinfo( int index, char *buffer, int bufferSize );

#if defined RTCW_ET
void SV_CreateBaseline( void );
#endif // RTCW_XX

void SV_ChangeMaxClients( void );
void SV_SpawnServer( char *server, qboolean killBots );

#if defined RTCW_SP
//RF, reliable commands
const char *SV_GetReliableCommand( client_t *cl, int index );
void SV_FreeAcknowledgedReliableCommands( client_t *cl );
qboolean SV_AddReliableCommand( client_t *cl, int index, const char *cmd );
void SV_InitReliableCommandsForClient( client_t *cl, int commands );
void SV_FreeReliableCommandsForClient( client_t *cl );
#endif // RTCW_XX

//
// sv_client.c
//
void SV_GetChallenge( netadr_t from );

void SV_DirectConnect( netadr_t from );

void SV_AuthorizeIpPacket( netadr_t from );

void SV_ExecuteClientMessage( client_t *cl, msg_t *msg );
void SV_UserinfoChanged( client_t *cl );

void SV_ClientEnterWorld( client_t *client, usercmd_t *cmd );

#if defined RTCW_ET
void SV_FreeClientNetChan( client_t* client );
#endif // RTCW_XX

void SV_DropClient( client_t *drop, const char *reason );


#if !defined RTCW_ET
void SV_ExecuteClientCommand( client_t *cl, const char *s, qboolean clientOK );
#else
void SV_ExecuteClientCommand( client_t *cl, const char *s, qboolean clientOK, qboolean premaprestart );
#endif // RTCW_XX

void SV_ClientThink( client_t *cl, usercmd_t *cmd );

void SV_WriteDownloadToClient( client_t *cl, msg_t *msg );

//
// sv_ccmds.c
//
void SV_Heartbeat_f( void );

#if defined RTCW_ET
qboolean SV_TempBanIsBanned( netadr_t address );
void SV_TempBanNetAddress( netadr_t address, int length );
#endif // RTCW_XX

//
// sv_snapshot.c
//
void SV_AddServerCommand( client_t *client, const char *cmd );
void SV_UpdateServerCommandsToClient( client_t *client, msg_t *msg );
void SV_WriteFrameToClient( client_t *client, msg_t *msg );
void SV_SendMessageToClient( msg_t *msg, client_t *client );
void SV_SendClientMessages( void );
void SV_SendClientSnapshot( client_t *client );

#if defined RTCW_ET
//bani
void SV_SendClientIdle( client_t *client );
#endif // RTCW_XX


//
// sv_game.c
//
int SV_NumForGentity( sharedEntity_t *ent );

#if defined RTCW_ET
//#define SV_GentityNum( num ) ((sharedEntity_t *)((byte *)sv.gentities + sv.gentitySize*(num)))
//#define SV_GameClientNum( num ) ((playerState_t *)((byte *)sv.gameClients + sv.gameClientSize*(num)))
#endif // RTCW_XX

sharedEntity_t *SV_GentityNum( int num );
playerState_t *SV_GameClientNum( int num );
svEntity_t  *SV_SvEntityForGentity( sharedEntity_t *gEnt );
sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt );
void        SV_InitGameProgs( void );
void        SV_ShutdownGameProgs( void );
void        SV_RestartGameProgs( void );
qboolean    SV_inPVS( const vec3_t p1, const vec3_t p2 );

#if !defined RTCW_ET
qboolean SV_GetTag( int clientNum, char *tagname, orientation_t * orient );
#else
qboolean SV_GetTag( int clientNum, int tagFileNumber, char *tagname, orientation_t * orient );
#endif // RTCW_XX

#if defined RTCW_ET
int SV_LoadTag( const char* mod_name );
qboolean    SV_GameIsSinglePlayer( void );
qboolean    SV_GameIsCoop( void );
void        SV_GameBinaryMessageReceived( int cno, const char *buf, int buflen, int commandTime );
#endif // RTCW_XX

//
// sv_bot.c
//
void        SV_BotFrame( int time );

#if !defined RTCW_ET
int         SV_BotAllocateClient( void );
#else
int         SV_BotAllocateClient( int clientNum );
#endif // RTCW_XX

void        SV_BotFreeClient( int clientNum );

void        SV_BotInitCvars( void );
int         SV_BotLibSetup( void );
int         SV_BotLibShutdown( void );
int         SV_BotGetSnapshotEntity( int client, int ent );
int         SV_BotGetConsoleMessage( int client, char *buf, int size );

int BotImport_DebugPolygonCreate( int color, int numPoints, vec3_t *points );
void BotImport_DebugPolygonDelete( int id );

//============================================================
//
// high level object sorting to reduce interaction tests
//

void SV_ClearWorld( void );
// called after the world model has been loaded, before linking any entities

void SV_UnlinkEntity( sharedEntity_t *ent );
// call before removing an entity, and before trying to move one,
// so it doesn't clip against itself

void SV_LinkEntity( sharedEntity_t *ent );
// Needs to be called any time an entity changes origin, mins, maxs,
// or solid.  Automatically unlinks if needed.
// sets ent->v.absmin and ent->v.absmax
// sets ent->leafnums[] for pvs determination even if the entity
// is not solid


clipHandle_t SV_ClipHandleForEntity( const sharedEntity_t *ent );


void SV_SectorList_f( void );


int SV_AreaEntities( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
// fills in a table of entity numbers with entities that have bounding boxes
// that intersect the given area.  It is possible for a non-axial bmodel
// to be returned that doesn't actually intersect the area on an exact
// test.
// returns the number of pointers filled in
// The world entity is never returned in this list.


int SV_PointContents( const vec3_t p, int passEntityNum );
// returns the CONTENTS_* value from the world and all entities at the given point.


void SV_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, int capsule );
// mins and maxs are relative

// if the entire move stays in a solid volume, trace.allsolid will be set,
// trace.startsolid will be set, and trace.fraction will be 0

// if the starting point is in a solid, it will be allowed to move out
// to an open area

// passEntityNum is explicitly excluded from clipping checks (normally ENTITYNUM_NONE)


void SV_ClipToEntity( trace_t *trace, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int entityNum, int contentmask, int capsule );
// clip to a specific entity

//
// sv_net_chan.c
//
#if defined RTCW_SP
void SV_Netchan_Transmit( client_t *client, msg_t *msg );    //int length, const byte *data );
void SV_Netchan_TransmitNextFragment( netchan_t *chan );
#else
void SV_Netchan_Transmit( client_t *client, msg_t *msg );
void SV_Netchan_TransmitNextFragment( client_t *client );
#endif // RTCW_XX

qboolean SV_Netchan_Process( client_t *client, msg_t *msg );

#if defined RTCW_ET
//bani - cl->downloadnotify
#define DLNOTIFY_REDIRECT   0x00000001  // "Redirecting client ..."
#define DLNOTIFY_BEGIN      0x00000002  // "clientDownload: 4 : beginning ..."
#define DLNOTIFY_ALL        ( DLNOTIFY_REDIRECT | DLNOTIFY_BEGIN )
#endif // RTCW_XX

