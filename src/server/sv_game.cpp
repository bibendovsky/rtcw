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

// sv_game.c -- interface to the game dll

#include "server.h"

#include "botlib.h"

botlib_export_t *botlib_export;

void SV_GameError( const char *string ) {
	Com_Error( ERR_DROP, "%s", string );
}

void SV_GamePrint( const char *string ) {
	Com_Printf( "%s", string );
}

// these functions must be used instead of pointer arithmetic, because
// the game allocates gentities with private information after the server shared part
int SV_NumForGentity( sharedEntity_t *ent ) {
	int num;

	num = ( (byte *)ent - (byte *)sv.gentities ) / sv.gentitySize;

	return num;
}

sharedEntity_t *SV_GentityNum( int num ) {
	sharedEntity_t *ent;

	ent = ( sharedEntity_t * )( (byte *)sv.gentities + sv.gentitySize * ( num ) );

	return ent;
}

playerState_t *SV_GameClientNum( int num ) {
	playerState_t   *ps;

	ps = ( playerState_t * )( (byte *)sv.gameClients + sv.gameClientSize * ( num ) );

	return ps;
}

svEntity_t  *SV_SvEntityForGentity( sharedEntity_t *gEnt ) {
	if ( !gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	return &sv.svEntities[ gEnt->s.number ];
}

sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt ) {
	int num;

	num = svEnt - sv.svEntities;
	return SV_GentityNum( num );
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( int clientNum, const char *text ) {
	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, "%s", text );
	} else {
		if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
			return;
		}
		SV_SendServerCommand( svs.clients + clientNum, "%s", text );
	}
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/

#if !defined RTCW_ET
void SV_GameDropClient( int clientNum, const char *reason ) {
#else
void SV_GameDropClient( int clientNum, const char *reason, int length ) {
#endif // RTCW_XX

	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( svs.clients + clientNum, reason );

#if defined RTCW_ET
	if ( length ) {
		SV_TempBanNetAddress( svs.clients[ clientNum ].netchan.remoteAddress, length );
	}
#endif // RTCW_XX

}


/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( sharedEntity_t *ent, const char *name ) {
	clipHandle_t h;
	vec3_t mins, maxs;

	if ( !name ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if ( name[0] != '*' ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}


	ent->s.modelindex = atoi( name + 1 );

	h = CM_InlineModel( ent->s.modelindex );
	CM_ModelBounds( h, mins, maxs );
	VectorCopy( mins, ent->r.mins );
	VectorCopy( maxs, ent->r.maxs );
	ent->r.bmodel = qtrue;

	ent->r.contents = -1;       // we don't know exactly what is in the brushes

	SV_LinkEntity( ent );       // FIXME: remove
}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
qboolean SV_inPVS( const vec3_t p1, const vec3_t p2 ) {
	int leafnum;
	int cluster;
	int area1, area2;
	byte    *mask;

	leafnum = CM_PointLeafnum( p1 );
	cluster = CM_LeafCluster( leafnum );
	area1 = CM_LeafArea( leafnum );
	mask = CM_ClusterPVS( cluster );

	leafnum = CM_PointLeafnum( p2 );
	cluster = CM_LeafCluster( leafnum );
	area2 = CM_LeafArea( leafnum );
	if ( mask && ( !( mask[cluster >> 3] & ( 1 << ( cluster & 7 ) ) ) ) ) {
		return qfalse;
	}
	if ( !CM_AreasConnected( area1, area2 ) ) {
		return qfalse;      // a door blocks sight
	}
	return qtrue;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
qboolean SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	int leafnum;
	int cluster;
	int area1, area2;
	byte    *mask;

	leafnum = CM_PointLeafnum( p1 );
	cluster = CM_LeafCluster( leafnum );
	area1 = CM_LeafArea( leafnum );
	mask = CM_ClusterPVS( cluster );

	leafnum = CM_PointLeafnum( p2 );
	cluster = CM_LeafCluster( leafnum );
	area2 = CM_LeafArea( leafnum );

	if ( mask && ( !( mask[cluster >> 3] & ( 1 << ( cluster & 7 ) ) ) ) ) {
		return qfalse;
	}

	return qtrue;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( sharedEntity_t *ent, qboolean open ) {
	svEntity_t  *svEnt;

	svEnt = SV_SvEntityForGentity( ent );
	if ( svEnt->areanum2 == -1 ) {
		return;
	}
	CM_AdjustAreaPortalState( svEnt->areanum, svEnt->areanum2, open );
}


/*
==================
SV_GameAreaEntities
==================
*/
qboolean    SV_EntityContact( const vec3_t mins, const vec3_t maxs, const sharedEntity_t *gEnt, const int capsule ) {
	const float *origin, *angles;
	clipHandle_t ch;
	trace_t trace;

	// check for exact collision
	origin = gEnt->r.currentOrigin;
	angles = gEnt->r.currentAngles;

	ch = SV_ClipHandleForEntity( gEnt );
	CM_TransformedBoxTrace( &trace, vec3_origin, vec3_origin, mins, maxs,
							ch, -1, origin, angles, capsule );

	return trace.startsolid;
}


/*
===============
SV_GetServerinfo

===============
*/
void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}

#if !defined RTCW_ET
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
#else
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO | CVAR_SERVERINFO_NOUPDATE ), bufferSize );
#endif // RTCW_XX

}

/*
===============
SV_LocateGameData

===============
*/
void SV_LocateGameData( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						playerState_t *clients, int sizeofGameClient ) {
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity_t;
	sv.num_entities = numGEntities;

	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;
}


/*
===============
SV_GetUsercmd

===============
*/
void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}

#if defined RTCW_ET
/*
====================
SV_SendBinaryMessage
====================
*/
static void SV_SendBinaryMessage( int cno, char *buf, int buflen ) {
	if ( cno < 0 || cno >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_SendBinaryMessage: bad client %i", cno );
		return;
	}

	if ( buflen < 0 || buflen > MAX_BINARY_MESSAGE ) {
		Com_Error( ERR_DROP, "SV_SendBinaryMessage: bad length %i", buflen );
		svs.clients[cno].binaryMessageLength = 0;
		return;
	}

	svs.clients[cno].binaryMessageLength = buflen;
	memcpy( svs.clients[cno].binaryMessage, buf, buflen );
}

/*
====================
SV_BinaryMessageStatus
====================
*/
static int SV_BinaryMessageStatus( int cno ) {
	if ( cno < 0 || cno >= sv_maxclients->integer ) {
		return qfalse;
	}

	if ( svs.clients[cno].binaryMessageLength == 0 ) {
		return MESSAGE_EMPTY;
	}

	if ( svs.clients[cno].binaryMessageOverflowed ) {
		return MESSAGE_WAITING_OVERFLOW;
	}

	return MESSAGE_WAITING;
}

/*
====================
SV_GameBinaryMessageReceived
====================
*/
void SV_GameBinaryMessageReceived( int cno, const char *buf, int buflen, int commandTime ) {
	VM_Call( gvm, GAME_MESSAGERECEIVED, cno, buf, buflen, commandTime );
}
#endif // RTCW_XX

//==============================================

//BBi
//static int  FloatAsInt( float f ) {
//	int temp;
//
//	*(float *)&temp = f;
//
//	return temp;
//}

static intptr_t FloatAsInt (
    float x)
{
    intptr_t result;

    *reinterpret_cast<float*> (&result) = x;

    return result;
}
//BBi

/*
====================
SV_GameSystemCalls

The module is making a system call
====================
*/
//rcg010207 - see my comments in VM_DllSyscall(), in qcommon/vm.c ...
#if ( ( defined __linux__ ) && ( defined __powerpc__ ) ) || ( defined MACOS_X )
#define VMA( x ) ( (void *) args[x] )
#else
#define VMA( x ) VM_ArgPtr( args[x] )
#endif

//BBi
//#define VMF( x )  ( (float *)args )[x]
#define VMF(x) (*reinterpret_cast<const float*> (args + x))
//BBi

#if defined RTCW_ET
// show_bug.cgi?id=574
extern int S_RegisterSound( const char *name, qboolean compressed );
extern int S_GetSoundLength( sfxHandle_t sfxHandle );
#endif // RTCW_XX

//BBi
//int SV_GameSystemCalls( int *args ) {
intptr_t SV_GameSystemCalls (
    intptr_t* args)
{
//BBi

	switch ( args[0] ) {
	case G_PRINT:

#if !defined RTCW_ET
		Com_Printf( "%s", VMA( 1 ) );
#else
		Com_Printf( "%s", (char *)VMA( 1 ) );
#endif // RTCW_XX

		return 0;
	case G_ERROR:

#if !defined RTCW_ET
		Com_Error( ERR_DROP, "%s", VMA( 1 ) );
#else
		Com_Error( ERR_DROP, "%s", (char *)VMA( 1 ) );
#endif // RTCW_XX

		return 0;

#if defined RTCW_SP
	case G_ENDGAME:
		Com_Error( ERR_ENDGAME, "endgame" );  // no message, no error print
		return 0;
#endif // RTCW_XX

	case G_MILLISECONDS:
		return Sys_Milliseconds();
	case G_CVAR_REGISTER:
		Cvar_Register( static_cast<vmCvar_t*> (VMA( 1 )), static_cast<const char*> (VMA( 2 )), static_cast<const char*> (VMA( 3 )), args[4] );
		return 0;
	case G_CVAR_UPDATE:
		Cvar_Update( static_cast<vmCvar_t*> (VMA( 1 )) );
		return 0;
	case G_CVAR_SET:
		Cvar_Set( static_cast<const char*> (VMA( 1 )), static_cast<const char*> (VMA( 2 )) );
		return 0;
	case G_CVAR_VARIABLE_INTEGER_VALUE:
		return Cvar_VariableIntegerValue( static_cast<const char*> (VMA( 1 )) );
	case G_CVAR_VARIABLE_STRING_BUFFER:
		Cvar_VariableStringBuffer( static_cast<const char*> (VMA( 1 )), static_cast<char*> (VMA( 2 )), args[3] );
		return 0;

#if defined RTCW_ET
	case G_CVAR_LATCHEDVARIABLESTRINGBUFFER:
		Cvar_LatchedVariableStringBuffer( static_cast<const char*> (VMA( 1 )), static_cast<char*> (VMA( 2 )), args[3] );
		return 0;
#endif // RTCW_XX

	case G_ARGC:
		return Cmd_Argc();
	case G_ARGV:
		Cmd_ArgvBuffer( args[1], static_cast<char*> (VMA( 2 )), args[3] );
		return 0;
	case G_SEND_CONSOLE_COMMAND:
		Cbuf_ExecuteText( args[1], static_cast<const char*> (VMA( 2 )) );
		return 0;

	case G_FS_FOPEN_FILE:
		return FS_FOpenFileByMode( static_cast<const char*> (VMA( 1 )), static_cast<fileHandle_t*> (VMA( 2 )), fsMode_t (args[3]) );
	case G_FS_READ:
		FS_Read( VMA( 1 ), args[2], args[3] );
		return 0;
	case G_FS_WRITE:
		return FS_Write( VMA( 1 ), args[2], args[3] );
	case G_FS_RENAME:
		FS_Rename( static_cast<const char*> (VMA( 1 )), static_cast<const char*> (VMA( 2 )) );
		return 0;
	case G_FS_FCLOSE_FILE:
		FS_FCloseFile( args[1] );
		return 0;

#if defined RTCW_SP
	case G_FS_COPY_FILE:
		FS_CopyFileOS( static_cast<char*> (VMA( 1 )), static_cast<char*> (VMA( 2 )) );    //DAJ
		return 0;
#endif // RTCW_XX

	case G_FS_GETFILELIST:
		return FS_GetFileList( static_cast<const char*> (VMA( 1 )), static_cast<const char*> (VMA( 2 )), static_cast<char*> (VMA( 3 )), args[4] );

	case G_LOCATE_GAME_DATA:
		SV_LocateGameData( static_cast<sharedEntity_t*> (VMA( 1 )), args[2], args[3], static_cast<playerState_t*> (VMA( 4 )), args[5] );
		return 0;
	case G_DROP_CLIENT:

#if !defined RTCW_ET
		SV_GameDropClient( args[1], static_cast<const char*> (VMA( 2 )) );
#else
		SV_GameDropClient( args[1], static_cast<const char*> (VMA( 2 )), args[3] );
#endif // RTCW_XX

		return 0;
	case G_SEND_SERVER_COMMAND:
		SV_GameSendServerCommand( args[1], static_cast<const char*> (VMA( 2 )) );
		return 0;
	case G_LINKENTITY:
		SV_LinkEntity( static_cast<sharedEntity_t*> (VMA( 1 )) );
		return 0;
	case G_UNLINKENTITY:
		SV_UnlinkEntity( static_cast<sharedEntity_t*> (VMA( 1 )) );
		return 0;
	case G_ENTITIES_IN_BOX:
		return SV_AreaEntities( static_cast<const vec_t*> (VMA( 1 )), static_cast<const vec_t*> (VMA( 2 )), static_cast<int*> (VMA( 3 )), args[4] );
	case G_ENTITY_CONTACT:
		return SV_EntityContact( static_cast<const vec_t*> (VMA( 1 )), static_cast<const vec_t*> (VMA( 2 )), static_cast<const sharedEntity_t*> (VMA( 3 )), /* int capsule */ qfalse );
	case G_ENTITY_CONTACTCAPSULE:
		return SV_EntityContact( static_cast<const vec_t*> (VMA( 1 )), static_cast<const vec_t*> (VMA( 2 )), static_cast<const sharedEntity_t*> (VMA( 3 )), /* int capsule */ qtrue );
	case G_TRACE:
		SV_Trace( static_cast<trace_t*> (VMA( 1 )), static_cast<const vec_t*> (VMA( 2 )), static_cast<const vec_t*> (VMA( 3 )), static_cast<const vec_t*> (VMA( 4 )), static_cast<const vec_t*> (VMA( 5 )), args[6], args[7], /* int capsule */ qfalse );
		return 0;
	case G_TRACECAPSULE:
		SV_Trace( static_cast<trace_t*> (VMA( 1 )), static_cast<const vec_t*> (VMA( 2 )), static_cast<const vec_t*> (VMA( 3 )), static_cast<const vec_t*> (VMA( 4 )), static_cast<const vec_t*> (VMA( 5 )), args[6], args[7], /* int capsule */ qtrue );
		return 0;
	case G_POINT_CONTENTS:
		return SV_PointContents( static_cast<const vec_t*> (VMA( 1 )), args[2] );
	case G_SET_BRUSH_MODEL:
		SV_SetBrushModel( static_cast<sharedEntity_t*> (VMA( 1 )), static_cast<const char*> (VMA( 2 )) );
		return 0;
	case G_IN_PVS:
		return SV_inPVS( static_cast<const vec_t*> (VMA( 1 )), static_cast<const vec_t*> (VMA( 2 )) );
	case G_IN_PVS_IGNORE_PORTALS:
		return SV_inPVSIgnorePortals( static_cast<const vec_t*> (VMA( 1 )), static_cast<const vec_t*> (VMA( 2 )) );

	case G_SET_CONFIGSTRING:
		SV_SetConfigstring( args[1], static_cast<const char*> (VMA( 2 )) );
		return 0;
	case G_GET_CONFIGSTRING:
		SV_GetConfigstring( args[1], static_cast<char*> (VMA( 2 )), args[3] );
		return 0;
	case G_SET_USERINFO:
		SV_SetUserinfo( args[1], static_cast<const char*> (VMA( 2 )) );
		return 0;
	case G_GET_USERINFO:
		SV_GetUserinfo( args[1], static_cast<char*> (VMA( 2 )), args[3] );
		return 0;
	case G_GET_SERVERINFO:
		SV_GetServerinfo( static_cast<char*> (VMA( 1 )), args[2] );
		return 0;
	case G_ADJUST_AREA_PORTAL_STATE:
		SV_AdjustAreaPortalState( static_cast<sharedEntity_t*> (VMA( 1 )), args[2] );
		return 0;
	case G_AREAS_CONNECTED:
		return CM_AreasConnected( args[1], args[2] );

	case G_BOT_ALLOCATE_CLIENT:

#if !defined RTCW_ET
		return SV_BotAllocateClient();
#else
		return SV_BotAllocateClient( args[1] );
#endif // RTCW_XX

	case G_BOT_FREE_CLIENT:
		SV_BotFreeClient( args[1] );
		return 0;

	case G_GET_USERCMD:
		SV_GetUsercmd( args[1], static_cast<usercmd_t*> (VMA( 2 )) );
		return 0;
	case G_GET_ENTITY_TOKEN:
	{
		const char  *s;

		s = COM_Parse( const_cast<const char**>(&sv.entityParsePoint) );
		Q_strncpyz( static_cast<char*> (VMA( 1 )), s, args[2] );
		if ( !sv.entityParsePoint && !s[0] ) {
			return qfalse;
		} else {
			return qtrue;
		}
	}

	case G_DEBUG_POLYGON_CREATE:
		return BotImport_DebugPolygonCreate( args[1], args[2], static_cast<vec3_t*> (VMA( 3 )) );
	case G_DEBUG_POLYGON_DELETE:
		BotImport_DebugPolygonDelete( args[1] );
		return 0;
	case G_REAL_TIME:
		return Com_RealTime( static_cast<qtime_t*> (VMA( 1 )) );
	case G_SNAPVECTOR:
		Sys_SnapVector( static_cast<float*> (VMA( 1 )) );
		return 0;
	case G_GETTAG:

#if !defined RTCW_ET
		return SV_GetTag( args[1], static_cast<char*> (VMA( 2 )), static_cast<orientation_t*> (VMA( 3 )) );
#else
		return SV_GetTag( args[1], args[2], static_cast<char*> (VMA( 3 )), static_cast<orientation_t*> (VMA( 4 )) );
#endif // RTCW_XX

#if defined RTCW_ET
	case G_REGISTERTAG:
		return SV_LoadTag( static_cast<const char*> (VMA( 1 )) );

		// START	xkan, 10/28/2002
	case G_REGISTERSOUND:
#ifdef DOOMSOUND    ///// (SA) DOOMSOUND
		return S_RegisterSound( VMA( 1 ) );
#else
		return S_RegisterSound( static_cast<const char*> (VMA( 1 )), args[2] );
#endif  ///// (SA) DOOMSOUND
	case G_GET_SOUND_LENGTH:
		return S_GetSoundLength( args[1] );
		// END		xkan, 10/28/2002
#endif // RTCW_XX

		//====================================

	case BOTLIB_SETUP:
		return SV_BotLibSetup();
	case BOTLIB_SHUTDOWN:
		return SV_BotLibShutdown();
	case BOTLIB_LIBVAR_SET:
		return botlib_export->BotLibVarSet( static_cast<char*> (VMA( 1 )), static_cast<char*> (VMA( 2 )) );
	case BOTLIB_LIBVAR_GET:
		return botlib_export->BotLibVarGet( static_cast<char*> (VMA( 1 )), static_cast<char*> (VMA( 2 )), args[3] );

	case BOTLIB_PC_ADD_GLOBAL_DEFINE:
		return botlib_export->PC_AddGlobalDefine( static_cast<char*> (VMA( 1 )) );
	case BOTLIB_PC_LOAD_SOURCE:
		return botlib_export->PC_LoadSourceHandle( static_cast<const char*> (VMA( 1 )) );
	case BOTLIB_PC_FREE_SOURCE:
		return botlib_export->PC_FreeSourceHandle( args[1] );
	case BOTLIB_PC_READ_TOKEN:
		return botlib_export->PC_ReadTokenHandle( args[1], static_cast<pc_token_t*> (VMA( 2 )) );
	case BOTLIB_PC_SOURCE_FILE_AND_LINE:
		return botlib_export->PC_SourceFileAndLine( args[1], static_cast<char*> (VMA( 2 )), static_cast<int*> (VMA( 3 )) );

#if defined RTCW_ET
	case BOTLIB_PC_UNREAD_TOKEN:
		botlib_export->PC_UnreadLastTokenHandle( args[1] );
		return 0;
#endif // RTCW_XX

	case BOTLIB_START_FRAME:
		return botlib_export->BotLibStartFrame( VMF( 1 ) );
	case BOTLIB_LOAD_MAP:
		return botlib_export->BotLibLoadMap( static_cast<const char*> (VMA( 1 )) );
	case BOTLIB_UPDATENTITY:
		return botlib_export->BotLibUpdateEntity( args[1], static_cast<bot_entitystate_t*> (VMA( 2 )) );
	case BOTLIB_TEST:
		return botlib_export->Test( args[1], static_cast<char*> (VMA( 2 )), static_cast<vec_t*> (VMA( 3 )), static_cast<vec_t*> (VMA( 4 )) );

	case BOTLIB_GET_SNAPSHOT_ENTITY:
		return SV_BotGetSnapshotEntity( args[1], args[2] );
	case BOTLIB_GET_CONSOLE_MESSAGE:
		return SV_BotGetConsoleMessage( args[1], static_cast<char*> (VMA( 2 )), args[3] );
	case BOTLIB_USER_COMMAND:
		SV_ClientThink( &svs.clients[args[1]], static_cast<usercmd_t*> (VMA( 2 )) );
		return 0;

	case BOTLIB_AAS_ENTITY_INFO:
		botlib_export->aas.AAS_EntityInfo( args[1], static_cast<aas_entityinfo_s*> (VMA( 2 )) );
		return 0;

	case BOTLIB_AAS_INITIALIZED:
		return botlib_export->aas.AAS_Initialized();
	case BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX:
		botlib_export->aas.AAS_PresenceTypeBoundingBox( args[1], static_cast<vec_t*> (VMA( 2 )), static_cast<vec_t*> (VMA( 3 )) );
		return 0;
	case BOTLIB_AAS_TIME:
		return FloatAsInt( botlib_export->aas.AAS_Time() );

		// Ridah
	case BOTLIB_AAS_SETCURRENTWORLD:
		botlib_export->aas.AAS_SetCurrentWorld( args[1] );
		return 0;
		// done.

	case BOTLIB_AAS_POINT_AREA_NUM:
		return botlib_export->aas.AAS_PointAreaNum( static_cast<vec_t*> (VMA( 1 )) );
	case BOTLIB_AAS_TRACE_AREAS:
		return botlib_export->aas.AAS_TraceAreas( static_cast<vec_t*> (VMA( 1 )), static_cast<vec_t*> (VMA( 2 )), static_cast<int*> (VMA( 3 )), static_cast<vec3_t*> (VMA( 4 )), args[5] );

#if defined RTCW_ET
	case BOTLIB_AAS_BBOX_AREAS:
		return botlib_export->aas.AAS_BBoxAreas( static_cast<vec_t*> (VMA( 1 )), static_cast<vec_t*> (VMA( 2 )), static_cast<int*> (VMA( 3 )), args[4] );
	case BOTLIB_AAS_AREA_CENTER:
		botlib_export->aas.AAS_AreaCenter( args[1], static_cast<vec_t*> (VMA( 2 )) );
		return 0;
	case BOTLIB_AAS_AREA_WAYPOINT:
		return botlib_export->aas.AAS_AreaWaypoint( args[1], static_cast<vec_t*> (VMA( 2 )) );
#endif // RTCW_XX

	case BOTLIB_AAS_POINT_CONTENTS:
		return botlib_export->aas.AAS_PointContents( static_cast<vec_t*> (VMA( 1 )) );
	case BOTLIB_AAS_NEXT_BSP_ENTITY:
		return botlib_export->aas.AAS_NextBSPEntity( args[1] );
	case BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY:
		return botlib_export->aas.AAS_ValueForBSPEpairKey( args[1],static_cast<char*> ( VMA( 2 )), static_cast<char*> (VMA( 3 )), args[4] );
	case BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY:
		return botlib_export->aas.AAS_VectorForBSPEpairKey( args[1], static_cast<char*> (VMA( 2 )), static_cast<vec_t*> (VMA( 3 )) );
	case BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY:
		return botlib_export->aas.AAS_FloatForBSPEpairKey( args[1], static_cast<char*> (VMA( 2 )), static_cast<float*> (VMA( 3 )) );
	case BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY:
		return botlib_export->aas.AAS_IntForBSPEpairKey( args[1], static_cast<char*> (VMA( 2 )), static_cast<int*> (VMA( 3 )) );

	case BOTLIB_AAS_AREA_REACHABILITY:
		return botlib_export->aas.AAS_AreaReachability( args[1] );

#if defined RTCW_ET
	case BOTLIB_AAS_AREA_LADDER:
		return botlib_export->aas.AAS_AreaLadder( args[1] );
#endif // RTCW_XX

	case BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA:
		return botlib_export->aas.AAS_AreaTravelTimeToGoalArea( args[1], static_cast<vec_t*> (VMA( 2 )), args[3], args[4] );

	case BOTLIB_AAS_SWIMMING:
		return botlib_export->aas.AAS_Swimming( static_cast<vec_t*> (VMA( 1 )) );
	case BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT:
		return botlib_export->aas.AAS_PredictClientMovement( static_cast<aas_clientmove_s*> (VMA( 1 )), args[2], static_cast<vec_t*> (VMA( 3 )), args[4], args[5],
															 static_cast<vec_t*> (VMA( 6 )), static_cast<vec_t*> (VMA( 7 )), args[8], args[9], VMF( 10 ), args[11], args[12], args[13] );

		// Ridah, route-tables
	case BOTLIB_AAS_RT_SHOWROUTE:
		botlib_export->aas.AAS_RT_ShowRoute( static_cast<vec_t*> (VMA( 1 )), args[2], args[3] );
		return 0;

#if !defined RTCW_ET
	case BOTLIB_AAS_RT_GETHIDEPOS:
		return botlib_export->aas.AAS_RT_GetHidePos( static_cast<vec_t*> (VMA( 1 )), args[2], args[3], static_cast<vec_t*> (VMA( 4 )), args[5], args[6], static_cast<vec_t*> (VMA( 7 )) );

	case BOTLIB_AAS_FINDATTACKSPOTWITHINRANGE:
		return botlib_export->aas.AAS_FindAttackSpotWithinRange( args[1], args[2], args[3], VMF( 4 ), args[5], static_cast<float*> (VMA( 6 )) );
#else
		//case BOTLIB_AAS_RT_GETHIDEPOS:
		//	return botlib_export->aas.AAS_RT_GetHidePos( VMA(1), args[2], args[3], VMA(4), args[5], args[6], VMA(7) );

		//case BOTLIB_AAS_FINDATTACKSPOTWITHINRANGE:
		//	return botlib_export->aas.AAS_FindAttackSpotWithinRange( args[1], args[2], args[3], VMF(4), args[5], VMA(6) );
#endif // RTCW_XX

#if defined RTCW_SP
	case BOTLIB_AAS_GETROUTEFIRSTVISPOS:
		return botlib_export->aas.AAS_GetRouteFirstVisPos( static_cast<vec_t*> (VMA( 1 )), static_cast<vec_t*> (VMA( 2 )), args[3], static_cast<vec_t*> (VMA( 4 )) );
#endif // RTCW_XX

#if defined RTCW_ET
	case BOTLIB_AAS_NEARESTHIDEAREA:
		return botlib_export->aas.AAS_NearestHideArea( args[1], static_cast<vec_t*> (VMA( 2 )), args[3], args[4], static_cast<vec_t*> (VMA( 5 )), args[6], args[7], VMF( 8 ), static_cast<vec_t*> (VMA( 9 )) );

	case BOTLIB_AAS_LISTAREASINRANGE:
		return botlib_export->aas.AAS_ListAreasInRange( static_cast<vec_t*> (VMA( 1 )), args[2], VMF( 3 ), args[4], static_cast<vec3_t*> (VMA( 5 )), args[6] );

	case BOTLIB_AAS_AVOIDDANGERAREA:
		return botlib_export->aas.AAS_AvoidDangerArea( static_cast<vec_t*> (VMA( 1 )), args[2], static_cast<vec_t*> (VMA( 3 )), args[4], VMF( 5 ), args[6] );

	case BOTLIB_AAS_RETREAT:
		return botlib_export->aas.AAS_Retreat( static_cast<int*> (VMA( 1 )), args[2], static_cast<vec_t*> (VMA( 3 )), args[4], static_cast<vec_t*> (VMA( 5 )), args[6], VMF( 7 ), VMF( 8 ), args[9] );

	case BOTLIB_AAS_ALTROUTEGOALS:
		return botlib_export->aas.AAS_AlternativeRouteGoals( static_cast<vec_t*> (VMA( 1 )), static_cast<vec_t*> (VMA( 2 )), args[3], static_cast<aas_altroutegoal_t*> (VMA( 4 )), args[5], args[6] );
#endif // RTCW_XX

	case BOTLIB_AAS_SETAASBLOCKINGENTITY:
		botlib_export->aas.AAS_SetAASBlockingEntity( static_cast<vec_t*> (VMA( 1 )), static_cast<vec_t*> (VMA( 2 )), args[3] );
		return 0;

#if defined RTCW_ET
	case BOTLIB_AAS_RECORDTEAMDEATHAREA:
		botlib_export->aas.AAS_RecordTeamDeathArea( static_cast<vec_t*> (VMA( 1 )), args[2], args[3], args[4], args[5] );
		return 0;
#endif // RTCW_XX

		// done.

	case BOTLIB_EA_SAY:
		botlib_export->ea.EA_Say( args[1], static_cast<char*> (VMA( 2 )) );
		return 0;
	case BOTLIB_EA_SAY_TEAM:
		botlib_export->ea.EA_SayTeam( args[1], static_cast<char*> (VMA( 2 )) );
		return 0;
	case BOTLIB_EA_USE_ITEM:
		botlib_export->ea.EA_UseItem( args[1], static_cast<char*> (VMA( 2 )) );
		return 0;
	case BOTLIB_EA_DROP_ITEM:
		botlib_export->ea.EA_DropItem( args[1], static_cast<char*> (VMA( 2 )) );
		return 0;
	case BOTLIB_EA_USE_INV:
		botlib_export->ea.EA_UseInv( args[1], static_cast<char*> (VMA( 2 )) );
		return 0;
	case BOTLIB_EA_DROP_INV:
		botlib_export->ea.EA_DropInv( args[1], static_cast<char*> (VMA( 2 )) );
		return 0;
	case BOTLIB_EA_GESTURE:
		botlib_export->ea.EA_Gesture( args[1] );
		return 0;
	case BOTLIB_EA_COMMAND:
		botlib_export->ea.EA_Command( args[1], static_cast<char*> (VMA( 2 )) );
		return 0;

	case BOTLIB_EA_SELECT_WEAPON:
		botlib_export->ea.EA_SelectWeapon( args[1], args[2] );
		return 0;
	case BOTLIB_EA_TALK:
		botlib_export->ea.EA_Talk( args[1] );
		return 0;
	case BOTLIB_EA_ATTACK:
		botlib_export->ea.EA_Attack( args[1] );
		return 0;
	case BOTLIB_EA_RELOAD:
		botlib_export->ea.EA_Reload( args[1] );
		return 0;
	case BOTLIB_EA_USE:
		botlib_export->ea.EA_Use( args[1] );
		return 0;
	case BOTLIB_EA_RESPAWN:
		botlib_export->ea.EA_Respawn( args[1] );
		return 0;
	case BOTLIB_EA_JUMP:
		botlib_export->ea.EA_Jump( args[1] );
		return 0;
	case BOTLIB_EA_DELAYED_JUMP:
		botlib_export->ea.EA_DelayedJump( args[1] );
		return 0;
	case BOTLIB_EA_CROUCH:
		botlib_export->ea.EA_Crouch( args[1] );
		return 0;

#if defined RTCW_ET
	case BOTLIB_EA_WALK:
		botlib_export->ea.EA_Walk( args[1] );
		return 0;
#endif // RTCW_XX

	case BOTLIB_EA_MOVE_UP:
		botlib_export->ea.EA_MoveUp( args[1] );
		return 0;
	case BOTLIB_EA_MOVE_DOWN:
		botlib_export->ea.EA_MoveDown( args[1] );
		return 0;
	case BOTLIB_EA_MOVE_FORWARD:
		botlib_export->ea.EA_MoveForward( args[1] );
		return 0;
	case BOTLIB_EA_MOVE_BACK:
		botlib_export->ea.EA_MoveBack( args[1] );
		return 0;
	case BOTLIB_EA_MOVE_LEFT:
		botlib_export->ea.EA_MoveLeft( args[1] );
		return 0;
	case BOTLIB_EA_MOVE_RIGHT:
		botlib_export->ea.EA_MoveRight( args[1] );
		return 0;
	case BOTLIB_EA_MOVE:
		botlib_export->ea.EA_Move( args[1], static_cast<vec_t*> (VMA( 2 )), VMF( 3 ) );
		return 0;
	case BOTLIB_EA_VIEW:
		botlib_export->ea.EA_View( args[1], static_cast<vec_t*> (VMA( 2 )) );
		return 0;

#if defined RTCW_ET
	case BOTLIB_EA_PRONE:
		botlib_export->ea.EA_Prone( args[1] );
		return 0;
#endif // RTCW_XX

	case BOTLIB_EA_END_REGULAR:
		botlib_export->ea.EA_EndRegular( args[1], VMF( 2 ) );
		return 0;
	case BOTLIB_EA_GET_INPUT:
		botlib_export->ea.EA_GetInput( args[1], VMF( 2 ), static_cast<bot_input_t*> (VMA( 3 )) );
		return 0;
	case BOTLIB_EA_RESET_INPUT:
		botlib_export->ea.EA_ResetInput( args[1], static_cast<bot_input_t*> (VMA( 2 )) );
		return 0;

	case BOTLIB_AI_LOAD_CHARACTER:
		return botlib_export->ai.BotLoadCharacter( static_cast<char*> (VMA( 1 )), args[2] );
	case BOTLIB_AI_FREE_CHARACTER:
		botlib_export->ai.BotFreeCharacter( args[1] );
		return 0;
	case BOTLIB_AI_CHARACTERISTIC_FLOAT:
		return FloatAsInt( botlib_export->ai.Characteristic_Float( args[1], args[2] ) );
	case BOTLIB_AI_CHARACTERISTIC_BFLOAT:
		return FloatAsInt( botlib_export->ai.Characteristic_BFloat( args[1], args[2], VMF( 3 ), VMF( 4 ) ) );
	case BOTLIB_AI_CHARACTERISTIC_INTEGER:
		return botlib_export->ai.Characteristic_Integer( args[1], args[2] );
	case BOTLIB_AI_CHARACTERISTIC_BINTEGER:
		return botlib_export->ai.Characteristic_BInteger( args[1], args[2], args[3], args[4] );
	case BOTLIB_AI_CHARACTERISTIC_STRING:
		botlib_export->ai.Characteristic_String( args[1], args[2], static_cast<char*> (VMA( 3 )), args[4] );
		return 0;

	case BOTLIB_AI_ALLOC_CHAT_STATE:
		return botlib_export->ai.BotAllocChatState();
	case BOTLIB_AI_FREE_CHAT_STATE:
		botlib_export->ai.BotFreeChatState( args[1] );
		return 0;
	case BOTLIB_AI_QUEUE_CONSOLE_MESSAGE:
		botlib_export->ai.BotQueueConsoleMessage( args[1], args[2], static_cast<char*> (VMA( 3 )) );
		return 0;
	case BOTLIB_AI_REMOVE_CONSOLE_MESSAGE:
		botlib_export->ai.BotRemoveConsoleMessage( args[1], args[2] );
		return 0;
	case BOTLIB_AI_NEXT_CONSOLE_MESSAGE:
		return botlib_export->ai.BotNextConsoleMessage( args[1], static_cast<bot_consolemessage_s*> (VMA( 2 )) );
	case BOTLIB_AI_NUM_CONSOLE_MESSAGE:
		return botlib_export->ai.BotNumConsoleMessages( args[1] );
	case BOTLIB_AI_INITIAL_CHAT:
		botlib_export->ai.BotInitialChat( args[1], static_cast<char*> (VMA( 2 )), args[3], static_cast<char*> (VMA( 4 )), static_cast<char*> (VMA( 5 )), static_cast<char*> (VMA( 6 )), static_cast<char*> (VMA( 7 )), static_cast<char*> (VMA( 8 )), static_cast<char*> (VMA( 9 )), static_cast<char*> (VMA( 10 )), static_cast<char*> (VMA( 11 )) );
		return 0;
	case BOTLIB_AI_NUM_INITIAL_CHATS:
		return botlib_export->ai.BotNumInitialChats( args[1], static_cast<char*> (VMA( 2 )) );
	case BOTLIB_AI_REPLY_CHAT:
		return botlib_export->ai.BotReplyChat( args[1], static_cast<char*> (VMA( 2 )), args[3], args[4], static_cast<char*> (VMA( 5 )), static_cast<char*> (VMA( 6 )), static_cast<char*> (VMA( 7 )), static_cast<char*> (VMA( 8 )), static_cast<char*> (VMA( 9 )), static_cast<char*> (VMA( 10 )), static_cast<char*> (VMA( 11 )), static_cast<char*> (VMA( 12 )) );
	case BOTLIB_AI_CHAT_LENGTH:
		return botlib_export->ai.BotChatLength( args[1] );
	case BOTLIB_AI_ENTER_CHAT:
		botlib_export->ai.BotEnterChat( args[1], args[2], args[3] );
		return 0;
	case BOTLIB_AI_GET_CHAT_MESSAGE:
		botlib_export->ai.BotGetChatMessage( args[1], static_cast<char*> (VMA( 2 )), args[3] );
		return 0;
	case BOTLIB_AI_STRING_CONTAINS:
		return botlib_export->ai.StringContains( static_cast<char*> (VMA( 1 )), static_cast<char*> (VMA( 2 )), args[3] );
	case BOTLIB_AI_FIND_MATCH:
		return botlib_export->ai.BotFindMatch( static_cast<char*> (VMA( 1 )), static_cast<bot_match_s*> (VMA( 2 )), args[3] );
	case BOTLIB_AI_MATCH_VARIABLE:
		botlib_export->ai.BotMatchVariable( static_cast<bot_match_s*> (VMA( 1 )), args[2], static_cast<char*> (VMA( 3 )), args[4] );
		return 0;
	case BOTLIB_AI_UNIFY_WHITE_SPACES:
		botlib_export->ai.UnifyWhiteSpaces( static_cast<char*> (VMA( 1 )) );
		return 0;
	case BOTLIB_AI_REPLACE_SYNONYMS:
		botlib_export->ai.BotReplaceSynonyms( static_cast<char*> (VMA( 1 )), args[2] );
		return 0;
	case BOTLIB_AI_LOAD_CHAT_FILE:
		return botlib_export->ai.BotLoadChatFile( args[1], static_cast<char*> (VMA( 2 )), static_cast<char*> (VMA( 3 )) );
	case BOTLIB_AI_SET_CHAT_GENDER:
		botlib_export->ai.BotSetChatGender( args[1], args[2] );
		return 0;
	case BOTLIB_AI_SET_CHAT_NAME:
		botlib_export->ai.BotSetChatName( args[1], static_cast<char*> (VMA( 2 )) );
		return 0;

	case BOTLIB_AI_RESET_GOAL_STATE:
		botlib_export->ai.BotResetGoalState( args[1] );
		return 0;
	case BOTLIB_AI_RESET_AVOID_GOALS:
		botlib_export->ai.BotResetAvoidGoals( args[1] );
		return 0;
	case BOTLIB_AI_REMOVE_FROM_AVOID_GOALS:
		botlib_export->ai.BotRemoveFromAvoidGoals( args[1], args[2] );
		return 0;
	case BOTLIB_AI_PUSH_GOAL:
		botlib_export->ai.BotPushGoal( args[1], static_cast<bot_goal_s*> (VMA( 2 )) );
		return 0;
	case BOTLIB_AI_POP_GOAL:
		botlib_export->ai.BotPopGoal( args[1] );
		return 0;
	case BOTLIB_AI_EMPTY_GOAL_STACK:
		botlib_export->ai.BotEmptyGoalStack( args[1] );
		return 0;
	case BOTLIB_AI_DUMP_AVOID_GOALS:
		botlib_export->ai.BotDumpAvoidGoals( args[1] );
		return 0;
	case BOTLIB_AI_DUMP_GOAL_STACK:
		botlib_export->ai.BotDumpGoalStack( args[1] );
		return 0;
	case BOTLIB_AI_GOAL_NAME:
		botlib_export->ai.BotGoalName( args[1], static_cast<char*> (VMA( 2 )), args[3] );
		return 0;
	case BOTLIB_AI_GET_TOP_GOAL:
		return botlib_export->ai.BotGetTopGoal( args[1], static_cast<bot_goal_s*> (VMA( 2 )) );
	case BOTLIB_AI_GET_SECOND_GOAL:
		return botlib_export->ai.BotGetSecondGoal( args[1], static_cast<bot_goal_s*> (VMA( 2 )) );
	case BOTLIB_AI_CHOOSE_LTG_ITEM:
		return botlib_export->ai.BotChooseLTGItem( args[1], static_cast<vec_t*> (VMA( 2 )), static_cast<int*> (VMA( 3 )), args[4] );
	case BOTLIB_AI_CHOOSE_NBG_ITEM:
		return botlib_export->ai.BotChooseNBGItem( args[1], static_cast<vec_t*> (VMA( 2 )), static_cast<int*> (VMA( 3 )), args[4], static_cast<bot_goal_s*> (VMA( 5 )), VMF( 6 ) );
	case BOTLIB_AI_TOUCHING_GOAL:
		return botlib_export->ai.BotTouchingGoal( static_cast<vec_t*> (VMA( 1 )), static_cast<bot_goal_s*> (VMA( 2 )) );
	case BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE:
		return botlib_export->ai.BotItemGoalInVisButNotVisible( args[1], static_cast<vec_t*> (VMA( 2 )), static_cast<vec_t*> (VMA( 3 )), static_cast<bot_goal_s*> (VMA( 4 )) );
	case BOTLIB_AI_GET_LEVEL_ITEM_GOAL:
		return botlib_export->ai.BotGetLevelItemGoal( args[1], static_cast<char*> (VMA( 2 )), static_cast<bot_goal_s*> (VMA( 3 )) );
	case BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL:
		return botlib_export->ai.BotGetNextCampSpotGoal( args[1], static_cast<bot_goal_s*> (VMA( 2 )) );
	case BOTLIB_AI_GET_MAP_LOCATION_GOAL:
		return botlib_export->ai.BotGetMapLocationGoal( static_cast<char*> (VMA( 1 )), static_cast<bot_goal_s*> (VMA( 2 )) );
	case BOTLIB_AI_AVOID_GOAL_TIME:
		return FloatAsInt( botlib_export->ai.BotAvoidGoalTime( args[1], args[2] ) );
	case BOTLIB_AI_INIT_LEVEL_ITEMS:
		botlib_export->ai.BotInitLevelItems();
		return 0;
	case BOTLIB_AI_UPDATE_ENTITY_ITEMS:
		botlib_export->ai.BotUpdateEntityItems();
		return 0;
	case BOTLIB_AI_LOAD_ITEM_WEIGHTS:
		return botlib_export->ai.BotLoadItemWeights( args[1], static_cast<char*> (VMA( 2 )) );
	case BOTLIB_AI_FREE_ITEM_WEIGHTS:
		botlib_export->ai.BotFreeItemWeights( args[1] );
		return 0;
	case BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC:
		botlib_export->ai.BotInterbreedGoalFuzzyLogic( args[1], args[2], args[3] );
		return 0;
	case BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC:
		botlib_export->ai.BotSaveGoalFuzzyLogic( args[1], static_cast<char*> (VMA( 2 )) );
		return 0;
	case BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC:
		botlib_export->ai.BotMutateGoalFuzzyLogic( args[1], VMF( 2 ) );
		return 0;
	case BOTLIB_AI_ALLOC_GOAL_STATE:
		return botlib_export->ai.BotAllocGoalState( args[1] );
	case BOTLIB_AI_FREE_GOAL_STATE:
		botlib_export->ai.BotFreeGoalState( args[1] );
		return 0;

	case BOTLIB_AI_RESET_MOVE_STATE:
		botlib_export->ai.BotResetMoveState( args[1] );
		return 0;
	case BOTLIB_AI_MOVE_TO_GOAL:
		botlib_export->ai.BotMoveToGoal( static_cast<bot_moveresult_s*> (VMA( 1 )), args[2], static_cast<bot_goal_s*> (VMA( 3 )), args[4] );
		return 0;
	case BOTLIB_AI_MOVE_IN_DIRECTION:
		return botlib_export->ai.BotMoveInDirection( args[1], static_cast<vec_t*> (VMA( 2 )), VMF( 3 ), args[4] );
	case BOTLIB_AI_RESET_AVOID_REACH:
		botlib_export->ai.BotResetAvoidReach( args[1] );
		return 0;
	case BOTLIB_AI_RESET_LAST_AVOID_REACH:
		botlib_export->ai.BotResetLastAvoidReach( args[1] );
		return 0;
	case BOTLIB_AI_REACHABILITY_AREA:
		return botlib_export->ai.BotReachabilityArea( static_cast<vec_t*> (VMA( 1 )), args[2] );
	case BOTLIB_AI_MOVEMENT_VIEW_TARGET:
		return botlib_export->ai.BotMovementViewTarget( args[1], static_cast<bot_goal_s*> (VMA( 2 )), args[3], VMF( 4 ), static_cast<vec_t*> (VMA( 5 )) );
	case BOTLIB_AI_PREDICT_VISIBLE_POSITION:
		return botlib_export->ai.BotPredictVisiblePosition( static_cast<vec_t*> (VMA( 1 )), args[2], static_cast<bot_goal_s*> (VMA( 3 )), args[4], static_cast<vec_t*> (VMA( 5 )) );
	case BOTLIB_AI_ALLOC_MOVE_STATE:
		return botlib_export->ai.BotAllocMoveState();
	case BOTLIB_AI_FREE_MOVE_STATE:
		botlib_export->ai.BotFreeMoveState( args[1] );
		return 0;
	case BOTLIB_AI_INIT_MOVE_STATE:
		botlib_export->ai.BotInitMoveState( args[1], static_cast<bot_initmove_s*> (VMA( 2 )) );
		return 0;
		// Ridah
	case BOTLIB_AI_INIT_AVOID_REACH:
		botlib_export->ai.BotInitAvoidReach( args[1] );
		return 0;
		// done.

	case BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON:
		return botlib_export->ai.BotChooseBestFightWeapon( args[1], static_cast<int*> (VMA( 2 )) );
	case BOTLIB_AI_GET_WEAPON_INFO:
		botlib_export->ai.BotGetWeaponInfo( args[1], args[2], static_cast<weaponinfo_s*> (VMA( 3 )) );
		return 0;
	case BOTLIB_AI_LOAD_WEAPON_WEIGHTS:
		return botlib_export->ai.BotLoadWeaponWeights( args[1], static_cast<char*> (VMA( 2 )) );
	case BOTLIB_AI_ALLOC_WEAPON_STATE:
		return botlib_export->ai.BotAllocWeaponState();
	case BOTLIB_AI_FREE_WEAPON_STATE:
		botlib_export->ai.BotFreeWeaponState( args[1] );
		return 0;
	case BOTLIB_AI_RESET_WEAPON_STATE:
		botlib_export->ai.BotResetWeaponState( args[1] );
		return 0;

	case BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION:
		return botlib_export->ai.GeneticParentsAndChildSelection( args[1], static_cast<float*> (VMA( 2 )), static_cast<int*> (VMA( 3 )), static_cast<int*> (VMA( 4 )), static_cast<int*> (VMA( 5 )) );

	case TRAP_MEMSET:
		memset( VMA( 1 ), args[2], args[3] );
		return 0;

	case TRAP_MEMCPY:
		memcpy( VMA( 1 ), VMA( 2 ), args[3] );
		return 0;

	case TRAP_STRNCPY:
		return (int)strncpy( static_cast<char*> (VMA( 1 )), static_cast<const char*> (VMA( 2 )), args[3] );

	case TRAP_SIN:
		return FloatAsInt( c::sin( VMF( 1 ) ) );

	case TRAP_COS:
		return FloatAsInt( c::cos( VMF( 1 ) ) );

	case TRAP_ATAN2:
		return FloatAsInt( c::atan2( VMF( 1 ), VMF( 2 ) ) );

	case TRAP_SQRT:
		return FloatAsInt( c::sqrt( VMF( 1 ) ) );

	case TRAP_MATRIXMULTIPLY:
		MatrixMultiply( static_cast<float(*)[3]> (VMA( 1 )), static_cast<float(*)[3]> (VMA( 2 )), static_cast<float(*)[3]> (VMA( 3 )) );
		return 0;

	case TRAP_ANGLEVECTORS:
		AngleVectors( static_cast<const vec_t*> (VMA( 1 )), static_cast<vec_t*> (VMA( 2 )), static_cast<vec_t*> (VMA( 3 )), static_cast<vec_t*> (VMA( 4 )) );
		return 0;

	case TRAP_PERPENDICULARVECTOR:
		PerpendicularVector( static_cast<vec_t*> (VMA( 1 )), static_cast<const vec_t*> (VMA( 2 )) );
		return 0;

	case TRAP_FLOOR:
		return FloatAsInt( c::floor( VMF( 1 ) ) );

	case TRAP_CEIL:
		return FloatAsInt( c::ceil( VMF( 1 ) ) );


#if defined RTCW_ET
	case PB_STAT_REPORT:
		return 0 ;

	case G_SENDMESSAGE:
		SV_SendBinaryMessage( args[1], static_cast<char*> (VMA( 2 )), args[3] );
		return 0;
	case G_MESSAGESTATUS:
		return SV_BinaryMessageStatus( args[1] );
#endif // RTCW_XX

	default:
		Com_Error( ERR_DROP, "Bad game system trap: %i", args[0] );
	}
	return -1;
}

/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs( void ) {
	if ( !gvm ) {
		return;
	}
	VM_Call( gvm, GAME_SHUTDOWN, qfalse );
	VM_Free( gvm );
	gvm = NULL;
}

/*
==================
SV_InitGameVM

Called for both a full init and a restart
==================
*/
static void SV_InitGameVM( qboolean restart ) {
	int i;

	// start the entity parsing at the beginning
	sv.entityParsePoint = CM_EntityString();

#if defined RTCW_SP
	// use the current msec count for a random seed
	// init for this gamestate
	VM_Call( gvm, GAME_INIT, svs.time, Com_Milliseconds(), restart );
#endif // RTCW_XX

	// clear all gentity pointers that might still be set from
	// a previous level
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		svs.clients[i].gentity = NULL;
	}

#if !defined RTCW_SP
	// use the current msec count for a random seed
	// init for this gamestate
	VM_Call( gvm, GAME_INIT, svs.time, Com_Milliseconds(), restart );
#endif // RTCW_XX

}



/*
===================
SV_RestartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void SV_RestartGameProgs( void ) {
	if ( !gvm ) {
		return;
	}
	VM_Call( gvm, GAME_SHUTDOWN, qtrue );

	// do a restart instead of a free
	gvm = VM_Restart( gvm );
	if ( !gvm ) { // bk001212 - as done below
		Com_Error( ERR_FATAL, "VM_Restart on game failed" );
	}

	SV_InitGameVM( qtrue );
}


/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs( void ) {

#if defined RTCW_ET
	sv.num_tagheaders = 0;
	sv.num_tags = 0;
#endif // RTCW_XX

#if defined RTCW_SP
	cvar_t  *var;
	//FIXME these are temp while I make bots run in vm
	extern int bot_enable;

	var = Cvar_Get( "bot_enable", "1", CVAR_LATCH );
	if ( var ) {
		bot_enable = var->integer;
	} else {
		bot_enable = 0;
	}
#endif // RTCW_XX

//BBi
//#if defined RTCW_SP
//	// load the dll or bytecode
//	gvm = VM_Create( "qagame", SV_GameSystemCalls, vmInterpret_t (int (Cvar_VariableValue( "vm_game" ))) );
//#else
//	// load the dll
//	gvm = VM_Create( "qagame", SV_GameSystemCalls, VMI_NATIVE );
//#endif // RTCW_XX

    gvm = ::VM_Create ("qagame", SV_GameSystemCalls);
//BBi

	if ( !gvm ) {
		Com_Error( ERR_FATAL, "VM_Create on game failed" );
	}

	SV_InitGameVM( qfalse );
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
qboolean SV_GameCommand( void ) {
	if ( sv.state != SS_GAME ) {
		return qfalse;
	}

	return VM_Call( gvm, GAME_CONSOLE_COMMAND );
}

#if !defined RTCW_ET
/*
====================
SV_SendMoveSpeedsToGame
====================
*/
void SV_SendMoveSpeedsToGame( int entnum, char *text ) {
	if ( !gvm ) {
		return;
	}
	VM_Call( gvm, GAME_RETRIEVE_MOVESPEEDS_FROM_CLIENT, entnum, text );
}
#endif // RTCW_XX

#if defined RTCW_ET
/*
====================
SV_GameIsSinglePlayer
====================
*/
qboolean SV_GameIsSinglePlayer( void ) {
	return( com_gameInfo.spGameTypes & ( 1 << g_gameType->integer ) );
}

/*
====================
SV_GameIsCoop

	This is a modified SinglePlayer, no savegame capability for example
====================
*/
qboolean SV_GameIsCoop( void ) {
	return( com_gameInfo.coopGameTypes & ( 1 << g_gameType->integer ) );
}
#endif // RTCW_XX

/*
====================
SV_GetTag

  return qfalse if unable to retrieve tag information for this client
====================
*/

#if defined RTCW_SP
extern qboolean CL_GetTag( int clientNum, char *tagname, orientation_t *orient );
#else
extern qboolean CL_GetTag( int clientNum, char *tagname, orientation_t * orient );
#endif // RTCW_XX

#if !defined RTCW_ET
qboolean SV_GetTag( int clientNum, char *tagname, orientation_t *orient ) {
#else
qboolean SV_GetTag( int clientNum, int tagFileNumber, char *tagname, orientation_t *orient ) {
	int i;
#endif // RTCW_XX

#if defined RTCW_ET
	if ( tagFileNumber > 0 && tagFileNumber <= sv.num_tagheaders ) {
		for ( i = sv.tagHeadersExt[tagFileNumber - 1].start; i < sv.tagHeadersExt[tagFileNumber - 1].start + sv.tagHeadersExt[tagFileNumber - 1].count; i++ ) {
			if ( !Q_stricmp( sv.tags[i].name, tagname ) ) {
				VectorCopy( sv.tags[i].origin, or->origin );
				VectorCopy( sv.tags[i].axis[0], or->axis[0] );
				VectorCopy( sv.tags[i].axis[1], or->axis[1] );
				VectorCopy( sv.tags[i].axis[2], or->axis[2] );
				return qtrue;
			}
		}
	}
#endif // RTCW_XX

#if defined RTCW_ET
	// Gordon: lets try and remove the inconsitancy between ded/non-ded servers...
	// Gordon: bleh, some code in clientthink_real really relies on this working on player models...
#endif // RTCW_XX

#if defined RTCW_SP || (!defined RTCW_SP && !defined DEDICATED)
// TTimo: dedicated only binary defines DEDICATED
	if ( com_dedicated->integer ) {
		return qfalse;
	}

	return CL_GetTag( clientNum, tagname, orient );
#else
	return qfalse;
#endif // RTCW_XX

}

#if defined RTCW_SP
/*
===================
SV_GetModelInfo

  request this modelinfo from the game
===================
*/
qboolean SV_GetModelInfo( int clientNum, char *modelName, animModelInfo_t **modelInfo ) {
	return VM_Call( gvm, GAME_GETMODELINFO, clientNum, modelName, modelInfo );
}
#endif // RTCW_XX

