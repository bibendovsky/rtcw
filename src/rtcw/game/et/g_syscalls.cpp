/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "g_local.h"

#include "rtcw_vm_args.h"


// this file is only included when building a dll
// g_syscalls.asm is included instead when building a qvm

#ifdef RTCW_VANILLA
static int ( QDECL * syscall )( int arg, ... ) = ( int ( QDECL * )( int, ... ) ) - 1;

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif
void dllEntry( int ( QDECL *syscallptr )( int arg,... ) ) {
	syscall = syscallptr;
}
#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif
#else // RTCW_VANILLA
static int ( QDECL * syscall )( intptr_t arg, ... ) = ( int ( QDECL * )( intptr_t, ... ) ) - 1;

extern "C" RTCW_DLLEXPORT void QDECL dllEntry( int ( QDECL *syscallptr )( intptr_t arg,... ) ) {
	syscall = syscallptr;
}
#endif // RTCW_VANILLA

#if FIXME
int PASSFLOAT( float x ) {
	float floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}
#endif // FIXME

void    trap_Printf( const char *fmt ) {
	syscall(
		G_PRINT,
		rtcw::to_vm_arg(fmt)
	);
}

void    trap_Error( const char *fmt ) {
	syscall(
		G_ERROR,
		rtcw::to_vm_arg(fmt)
	);
}

int     trap_Milliseconds( void ) {
	return syscall(G_MILLISECONDS);
}
int     trap_Argc( void ) {
	return syscall(G_ARGC);
}

void    trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall(
		G_ARGV,
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufferLength)
	);
}

int     trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall(
		G_FS_FOPEN_FILE,
		rtcw::to_vm_arg(qpath),
		rtcw::to_vm_arg(f),
		rtcw::to_vm_arg(mode)
	);
}

void    trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	syscall(
		G_FS_READ,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(len),
		rtcw::to_vm_arg(f)
	);
}

int     trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return syscall(
		G_FS_WRITE,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(len),
		rtcw::to_vm_arg(f)
	);
}

int     trap_FS_Rename( const char *from, const char *to ) {
	return syscall(
		G_FS_RENAME,
		rtcw::to_vm_arg(from),
		rtcw::to_vm_arg(to)
	);
}

void    trap_FS_FCloseFile( fileHandle_t f ) {
	syscall(
		G_FS_FCLOSE_FILE,
		rtcw::to_vm_arg(f)
	);
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall(
		G_FS_GETFILELIST,
		rtcw::to_vm_arg(path),
		rtcw::to_vm_arg(extension),
		rtcw::to_vm_arg(listbuf),
		rtcw::to_vm_arg(bufsize)
	);
}

void    trap_SendConsoleCommand( int exec_when, const char *text ) {
	syscall(
		G_SEND_CONSOLE_COMMAND,
		rtcw::to_vm_arg(exec_when),
		rtcw::to_vm_arg(text)
	);
}

void    trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	syscall(
		G_CVAR_REGISTER,
		rtcw::to_vm_arg(cvar),
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(value),
		rtcw::to_vm_arg(flags)
	);
}

void    trap_Cvar_Update( vmCvar_t *cvar ) {
	syscall(
		G_CVAR_UPDATE,
		rtcw::to_vm_arg(cvar)
	);
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall(
		G_CVAR_SET,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(value)
	);
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return syscall(
		G_CVAR_VARIABLE_INTEGER_VALUE,
		rtcw::to_vm_arg(var_name)
	);
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall(
		G_CVAR_VARIABLE_STRING_BUFFER,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufsize)
	);
}

void trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall(
		G_CVAR_LATCHEDVARIABLESTRINGBUFFER,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufsize)
	);
}

void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						  playerState_t *clients, int sizeofGClient ) {
	syscall(
		G_LOCATE_GAME_DATA,
		rtcw::to_vm_arg(gEnts),
		rtcw::to_vm_arg(numGEntities),
		rtcw::to_vm_arg(sizeofGEntity_t),
		rtcw::to_vm_arg(clients),
		rtcw::to_vm_arg(sizeofGClient)
	);
}

void trap_DropClient( int clientNum, const char *reason, int length ) {
	syscall(
		G_DROP_CLIENT,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(reason),
		rtcw::to_vm_arg(length)
	);
}

void trap_SendServerCommand( int clientNum, const char *text ) {
	// rain - #433 - commands over 1022 chars will crash the
	// client engine upon receipt, so ignore them
	if ( strlen( text ) > 1022 ) {
		G_LogPrintf( "%s: trap_SendServerCommand( %d, ... ) length exceeds 1022.\n", GAMEVERSION, clientNum );
		G_LogPrintf( "%s: text [%s]\n", GAMEVERSION, text );
		return;
	}
	syscall(
		G_SEND_SERVER_COMMAND,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(text)
	);
}

void trap_SetConfigstring( int num, const char *string ) {
	syscall(
		G_SET_CONFIGSTRING,
		rtcw::to_vm_arg(num),
		rtcw::to_vm_arg(string)
	);
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	syscall(
		G_GET_CONFIGSTRING,
		rtcw::to_vm_arg(num),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufferSize)
	);
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	syscall(
		G_GET_USERINFO,
		rtcw::to_vm_arg(num),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufferSize)
	);
}

void trap_SetUserinfo( int num, const char *buffer ) {
	syscall(
		G_SET_USERINFO,
		rtcw::to_vm_arg(num),
		rtcw::to_vm_arg(buffer)
	);
}

void trap_GetServerinfo( char *buffer, int bufferSize ) {
	syscall(
		G_GET_SERVERINFO,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufferSize)
	);
}

void trap_SetBrushModel( gentity_t *ent, const char *name ) {
	syscall(
		G_SET_BRUSH_MODEL,
		rtcw::to_vm_arg(ent),
		rtcw::to_vm_arg(name)
	);
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall(
		G_TRACE,
		rtcw::to_vm_arg(results),
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(end),
		rtcw::to_vm_arg(passEntityNum),
		rtcw::to_vm_arg(contentmask)
	);
}

void trap_TraceNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall(
		G_TRACE,
		rtcw::to_vm_arg(results),
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(end),
		rtcw::to_vm_arg(-2),
		rtcw::to_vm_arg(contentmask)
	);
}

void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall(
		G_TRACECAPSULE,
		rtcw::to_vm_arg(results),
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(end),
		rtcw::to_vm_arg(passEntityNum),
		rtcw::to_vm_arg(contentmask)
	);
}

void trap_TraceCapsuleNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall(
		G_TRACECAPSULE,
		rtcw::to_vm_arg(results),
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(end),
		rtcw::to_vm_arg(-2),
		rtcw::to_vm_arg(contentmask)
	);
}

int trap_PointContents( const vec3_t point, int passEntityNum ) {
	return syscall(
		G_POINT_CONTENTS,
		rtcw::to_vm_arg(point),
		rtcw::to_vm_arg(passEntityNum)
	);
}


qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 ) {
	return syscall(
		G_IN_PVS,
		rtcw::to_vm_arg(p1),
		rtcw::to_vm_arg(p2)
	);
}

qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	return syscall(
		G_IN_PVS_IGNORE_PORTALS,
		rtcw::to_vm_arg(p1),
		rtcw::to_vm_arg(p2)
	);
}

void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	syscall(
		G_ADJUST_AREA_PORTAL_STATE,
		rtcw::to_vm_arg(ent),
		rtcw::to_vm_arg(open)
	);
}

qboolean trap_AreasConnected( int area1, int area2 ) {
	return syscall(
		G_AREAS_CONNECTED,
		rtcw::to_vm_arg(area1),
		rtcw::to_vm_arg(area2)
	);
}

void trap_LinkEntity( gentity_t *ent ) {
	syscall(
		G_LINKENTITY,
		rtcw::to_vm_arg(ent)
	);
}

void trap_UnlinkEntity( gentity_t *ent ) {
	syscall(
		G_UNLINKENTITY,
		rtcw::to_vm_arg(ent)
	);
}


int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) {
	return syscall(
		G_ENTITIES_IN_BOX,
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(list),
		rtcw::to_vm_arg(maxcount)
	);
}

qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall(
		G_ENTITY_CONTACT,
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(ent)
	);
}

qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall(
		G_ENTITY_CONTACTCAPSULE,
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(ent)
	);
}

int trap_BotAllocateClient( int clientNum ) {
	return syscall(
		G_BOT_ALLOCATE_CLIENT,
		rtcw::to_vm_arg(clientNum)
	);
}

void trap_BotFreeClient( int clientNum ) {
	syscall(
		G_BOT_FREE_CLIENT,
		rtcw::to_vm_arg(clientNum)
	);
}

int trap_GetSoundLength( sfxHandle_t sfxHandle ) {
	return syscall(
		G_GET_SOUND_LENGTH,
		rtcw::to_vm_arg(sfxHandle)
	);
}

sfxHandle_t trap_RegisterSound( const char *sample, qboolean compressed ) {
	return syscall(
		G_REGISTERSOUND,
		rtcw::to_vm_arg(sample),
		rtcw::to_vm_arg(compressed)
	);
}

#ifdef DEBUG
//#define FAKELAG
#ifdef FAKELAG
#define MAX_USERCMD_BACKUP  256
#define MAX_USERCMD_MASK    ( MAX_USERCMD_BACKUP - 1 )

static usercmd_t cmds[MAX_CLIENTS][MAX_USERCMD_BACKUP];
static int cmdNumber[MAX_CLIENTS];
#endif // FAKELAG
#endif // DEBUG

void trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	syscall(
		G_GET_USERCMD,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(cmd)
	);

#ifdef FAKELAG
	{
		char s[MAX_STRING_CHARS];
		int fakeLag;

		trap_Cvar_VariableStringBuffer( "g_fakelag", s, sizeof( s ) );
		fakeLag = atoi( s );
		if ( fakeLag < 0 ) {
			fakeLag = 0;
		}

		if ( fakeLag ) {
			int i;
			int realcmdtime, thiscmdtime;

			// store our newest usercmd
			cmdNumber[clientNum]++;
			memcpy( &cmds[clientNum][cmdNumber[clientNum] & MAX_USERCMD_MASK], cmd, sizeof( usercmd_t ) );

			// find a usercmd that is fakeLag msec behind
			i = cmdNumber[clientNum] & MAX_USERCMD_MASK;
			realcmdtime = cmds[clientNum][i].serverTime;
			i--;
			do {
				thiscmdtime = cmds[clientNum][i & MAX_USERCMD_MASK].serverTime;

				if ( realcmdtime - thiscmdtime > fakeLag ) {
					// found the right one
					cmd = &cmds[clientNum][i & MAX_USERCMD_MASK];
					return;
				}

				i--;
			} while ( ( i & MAX_USERCMD_MASK ) != ( cmdNumber[clientNum] & MAX_USERCMD_MASK ) );

			// didn't find a proper one, just use the oldest one we have
			cmd = &cmds[clientNum][( cmdNumber[clientNum] - 1 ) & MAX_USERCMD_MASK];
			return;
		}
	}
#endif // FAKELAG
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return syscall(
		G_GET_ENTITY_TOKEN,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufferSize)
	);
}

int trap_DebugPolygonCreate( int color, int numPoints, vec3_t *points ) {
	return syscall(
		G_DEBUG_POLYGON_CREATE,
		rtcw::to_vm_arg(color),
		rtcw::to_vm_arg(numPoints),
		rtcw::to_vm_arg(points)
	);
}

void trap_DebugPolygonDelete( int id ) {
	syscall(
		G_DEBUG_POLYGON_DELETE,
		rtcw::to_vm_arg(id)
	);
}

int trap_RealTime( qtime_t *qtime ) {
	return syscall(
		G_REAL_TIME,
		rtcw::to_vm_arg(qtime)
	);
}

void trap_SnapVector( float *v ) {
	syscall(
		G_SNAPVECTOR,
		rtcw::to_vm_arg(v)
	);
	return;
}

qboolean trap_GetTag( int clientNum, int tagFileNumber, const char *tagName, orientation_t *orient ) {
	return syscall(
		G_GETTAG,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(tagFileNumber),
		rtcw::to_vm_arg(tagName),
		rtcw::to_vm_arg(orient)
	);
}

qboolean trap_LoadTag( const char* filename ) {
	return syscall(
		G_REGISTERTAG,
		rtcw::to_vm_arg(filename)
	);
}

// BotLib traps start here
int trap_BotLibSetup( void ) {
	return syscall(BOTLIB_SETUP);
}

int trap_BotLibShutdown( void ) {
	return syscall(BOTLIB_SHUTDOWN);
}

int trap_BotLibVarSet( const char *var_name, const char *value ) {
	return syscall(
		BOTLIB_LIBVAR_SET,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(value)
	);
}

int trap_BotLibVarGet( const char *var_name, char *value, int size ) {
	return syscall(
		BOTLIB_LIBVAR_GET,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(value),
		rtcw::to_vm_arg(size)
	);
}

int trap_BotLibDefine( const char *string ) {
	return syscall(
		BOTLIB_PC_ADD_GLOBAL_DEFINE,
		rtcw::to_vm_arg(string)
	);
}

int trap_PC_AddGlobalDefine( const char *define ) {
	return syscall(
		BOTLIB_PC_ADD_GLOBAL_DEFINE,
		rtcw::to_vm_arg(define)
	);
}

int trap_PC_LoadSource( const char *filename ) {
	return syscall(
		BOTLIB_PC_LOAD_SOURCE,
		rtcw::to_vm_arg(filename)
	);
}

int trap_PC_FreeSource( int handle ) {
	return syscall(
		BOTLIB_PC_FREE_SOURCE,
		rtcw::to_vm_arg(handle)
	);
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall(
		BOTLIB_PC_READ_TOKEN,
		rtcw::to_vm_arg(handle),
		rtcw::to_vm_arg(pc_token)
	);
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall(
		BOTLIB_PC_SOURCE_FILE_AND_LINE,
		rtcw::to_vm_arg(handle),
		rtcw::to_vm_arg(filename),
		rtcw::to_vm_arg(line)
	);
}

int trap_PC_UnReadToken( int handle ) {
	return syscall(
		BOTLIB_PC_UNREAD_TOKEN,
		rtcw::to_vm_arg(handle)
	);
}

int trap_BotLibStartFrame( float time ) {
	return syscall(
		BOTLIB_START_FRAME,
		rtcw::to_vm_arg(time)
	);
}

int trap_BotLibLoadMap( const char *mapname ) {
	return syscall(
		BOTLIB_LOAD_MAP,
		rtcw::to_vm_arg(mapname)
	);
}

int trap_BotLibUpdateEntity( int ent, void /* struct bot_updateentity_s */ *bue ) {
	return syscall(
		BOTLIB_UPDATENTITY,
		rtcw::to_vm_arg(ent),
		rtcw::to_vm_arg(bue)
	);
}

int trap_BotLibTest( int parm0, char *parm1, vec3_t parm2, vec3_t parm3 ) {
	return syscall(
		BOTLIB_TEST,
		rtcw::to_vm_arg(parm0),
		rtcw::to_vm_arg(parm1),
		rtcw::to_vm_arg(parm2),
		rtcw::to_vm_arg(parm3)
	);
}

int trap_BotGetSnapshotEntity( int clientNum, int sequence ) {
	return syscall(
		BOTLIB_GET_SNAPSHOT_ENTITY,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(sequence)
	);
}

int trap_BotGetServerCommand( int clientNum, char *message, int size ) {
	return syscall(
		BOTLIB_GET_CONSOLE_MESSAGE,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(message),
		rtcw::to_vm_arg(size)
	);
}

void trap_BotUserCommand( int clientNum, usercmd_t *ucmd ) {
	syscall(
		BOTLIB_USER_COMMAND,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(ucmd)
	);
}

void trap_AAS_EntityInfo( int entnum, void /* struct aas_entityinfo_s */ *info ) {
	syscall(
		BOTLIB_AAS_ENTITY_INFO,
		rtcw::to_vm_arg(entnum),
		rtcw::to_vm_arg(info)
	);
}

int trap_AAS_Initialized( void ) {
	return syscall(BOTLIB_AAS_INITIALIZED);
}

void trap_AAS_PresenceTypeBoundingBox( int presencetype, vec3_t mins, vec3_t maxs ) {
	syscall(
		BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX,
		rtcw::to_vm_arg(presencetype),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs)
	);
}

float trap_AAS_Time( void ) {
	int temp;
	temp = syscall(BOTLIB_AAS_TIME);
	return ( *(float*)&temp );
}

// Ridah, multiple AAS files
void trap_AAS_SetCurrentWorld( int index ) {
	// Gordon: stubbed out: we only use one aas
//	syscall( BOTLIB_AAS_SETCURRENTWORLD, index );
}
// done.

int trap_AAS_PointAreaNum( vec3_t point ) {
	return syscall(
		BOTLIB_AAS_POINT_AREA_NUM,
		rtcw::to_vm_arg(point)
	);
}

int trap_AAS_TraceAreas( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas ) {
	return syscall(
		BOTLIB_AAS_TRACE_AREAS,
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(end),
		rtcw::to_vm_arg(areas),
		rtcw::to_vm_arg(points),
		rtcw::to_vm_arg(maxareas)
	);
}

int trap_AAS_BBoxAreas( vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas ) {
	return syscall(
		BOTLIB_AAS_BBOX_AREAS,
		rtcw::to_vm_arg(absmins),
		rtcw::to_vm_arg(absmaxs),
		rtcw::to_vm_arg(areas),
		rtcw::to_vm_arg(maxareas)
	);
}

void trap_AAS_AreaCenter( int areanum, vec3_t center ) {
	syscall(
		BOTLIB_AAS_AREA_CENTER,
		rtcw::to_vm_arg(areanum),
		rtcw::to_vm_arg(center)
	);
}

qboolean trap_AAS_AreaWaypoint( int areanum, vec3_t center ) {
	return syscall(
		BOTLIB_AAS_AREA_WAYPOINT,
		rtcw::to_vm_arg(areanum),
		rtcw::to_vm_arg(center)
	);
}

int trap_AAS_PointContents( vec3_t point ) {
	return syscall(
		BOTLIB_AAS_POINT_CONTENTS,
		rtcw::to_vm_arg(point)
	);
}

int trap_AAS_NextBSPEntity( int ent ) {
	return syscall(
		BOTLIB_AAS_NEXT_BSP_ENTITY,
		rtcw::to_vm_arg(ent)
	);
}

int trap_AAS_ValueForBSPEpairKey( int ent, const char *key, char *value, int size ) {
	return syscall(
		BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY,
		rtcw::to_vm_arg(ent),
		rtcw::to_vm_arg(key),
		rtcw::to_vm_arg(value),
		rtcw::to_vm_arg(size)
	);
}

int trap_AAS_VectorForBSPEpairKey( int ent, const char *key, vec3_t v ) {
	return syscall(
		BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY,
		rtcw::to_vm_arg(ent),
		rtcw::to_vm_arg(key),
		rtcw::to_vm_arg(v)
	);
}

int trap_AAS_FloatForBSPEpairKey( int ent, const char *key, float *value ) {
	return syscall(
		BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY,
		rtcw::to_vm_arg(ent),
		rtcw::to_vm_arg(key),
		rtcw::to_vm_arg(value)
	);
}

int trap_AAS_IntForBSPEpairKey( int ent, const char *key, int *value ) {
	return syscall(
		BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY,
		rtcw::to_vm_arg(ent),
		rtcw::to_vm_arg(key),
		rtcw::to_vm_arg(value)
	);
}

int trap_AAS_AreaReachability( int areanum ) {
	return syscall(
		BOTLIB_AAS_AREA_REACHABILITY,
		rtcw::to_vm_arg(areanum)
	);
}

int trap_AAS_AreaLadder( int areanum ) {
	return syscall(
		BOTLIB_AAS_AREA_LADDER,
		rtcw::to_vm_arg(areanum)
	);
}

int trap_AAS_AreaTravelTimeToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags ) {
	return syscall(
		BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA,
		rtcw::to_vm_arg(areanum),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(goalareanum),
		rtcw::to_vm_arg(travelflags)
	);
}

int trap_AAS_Swimming( vec3_t origin ) {
	return syscall(
		BOTLIB_AAS_SWIMMING,
		rtcw::to_vm_arg(origin)
	);
}

int trap_AAS_PredictClientMovement( void /* struct aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize ) {
	return syscall(
		BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT,
		rtcw::to_vm_arg(move),
		rtcw::to_vm_arg(entnum),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(presencetype),
		rtcw::to_vm_arg(onground),
		rtcw::to_vm_arg(velocity),
		rtcw::to_vm_arg(cmdmove),
		rtcw::to_vm_arg(cmdframes),
		rtcw::to_vm_arg(maxframes),
		rtcw::to_vm_arg(frametime),
		rtcw::to_vm_arg(stopevent),
		rtcw::to_vm_arg(stopareanum),
		rtcw::to_vm_arg(visualize)
	);
}

// Ridah, route-tables
void trap_AAS_RT_ShowRoute( vec3_t srcpos, int srcnum, int destnum ) {
	syscall(
		BOTLIB_AAS_RT_SHOWROUTE,
		rtcw::to_vm_arg(srcpos),
		rtcw::to_vm_arg(srcnum),
		rtcw::to_vm_arg(destnum)
	);
}

//qboolean trap_AAS_RT_GetHidePos( vec3_t srcpos, int srcnum, int srcarea, vec3_t destpos, int destnum, int destarea, vec3_t returnPos ) {
//	return syscall( BOTLIB_AAS_RT_GETHIDEPOS, srcpos, srcnum, srcarea, destpos, destnum, destarea, returnPos );
//}

//int trap_AAS_FindAttackSpotWithinRange(int srcnum, int rangenum, int enemynum, float rangedist, int travelflags, float *outpos) {
//	return syscall( BOTLIB_AAS_FINDATTACKSPOTWITHINRANGE, srcnum, rangenum, enemynum, rtcw::to_vm_arg(rangedist), travelflags, outpos );
//}

int trap_AAS_NearestHideArea( int srcnum, vec3_t origin, int areanum, int enemynum, vec3_t enemyorigin, int enemyareanum, int travelflags, float maxdist, vec3_t distpos ) {
	return syscall(
		BOTLIB_AAS_NEARESTHIDEAREA,
		rtcw::to_vm_arg(srcnum),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(areanum),
		rtcw::to_vm_arg(enemynum),
		rtcw::to_vm_arg(enemyorigin),
		rtcw::to_vm_arg(enemyareanum),
		rtcw::to_vm_arg(travelflags),
		rtcw::to_vm_arg(maxdist),
		rtcw::to_vm_arg(distpos)
	);
}

int trap_AAS_ListAreasInRange( vec3_t srcpos, int srcarea, float range, int travelflags, float **outareas, int maxareas ) {
	return syscall(
		BOTLIB_AAS_LISTAREASINRANGE,
		rtcw::to_vm_arg(srcpos),
		rtcw::to_vm_arg(srcarea),
		rtcw::to_vm_arg(range),
		rtcw::to_vm_arg(travelflags),
		rtcw::to_vm_arg(outareas),
		rtcw::to_vm_arg(maxareas)
	);
}

int trap_AAS_AvoidDangerArea( vec3_t srcpos, int srcarea, vec3_t dangerpos, int dangerarea, float range, int travelflags ) {
	return syscall(
		BOTLIB_AAS_AVOIDDANGERAREA,
		rtcw::to_vm_arg(srcpos),
		rtcw::to_vm_arg(srcarea),
		rtcw::to_vm_arg(dangerpos),
		rtcw::to_vm_arg(dangerarea),
		rtcw::to_vm_arg(range),
		rtcw::to_vm_arg(travelflags)
	);
}

int trap_AAS_Retreat
(
	// Locations of the danger spots (AAS area numbers)
	int *dangerSpots,
	// The number of danger spots
	int dangerSpotCount,
	vec3_t srcpos,
	int srcarea,
	vec3_t dangerpos,
	int dangerarea,
	// Min range from startpos
	float range,
	// Min range from danger
	float dangerRange,
	int travelflags
) {
	return syscall(
		BOTLIB_AAS_RETREAT,
		rtcw::to_vm_arg(dangerSpots),
		rtcw::to_vm_arg(dangerSpotCount),
		rtcw::to_vm_arg(srcpos),
		rtcw::to_vm_arg(srcarea),
		rtcw::to_vm_arg(dangerpos),
		rtcw::to_vm_arg(dangerarea),
		rtcw::to_vm_arg(range),
		rtcw::to_vm_arg(dangerRange),
		rtcw::to_vm_arg(travelflags)
	);
}

int trap_AAS_AlternativeRouteGoals( vec3_t start, vec3_t goal, int travelflags,
									aas_altroutegoal_t *altroutegoals, int maxaltroutegoals,
									int color ) {
	return syscall(
		BOTLIB_AAS_ALTROUTEGOALS,
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(goal),
		rtcw::to_vm_arg(travelflags),
		rtcw::to_vm_arg(altroutegoals),
		rtcw::to_vm_arg(maxaltroutegoals),
		rtcw::to_vm_arg(color)
	);
}

void trap_AAS_SetAASBlockingEntity( vec3_t absmin, vec3_t absmax, int blocking ) {
	syscall(
		BOTLIB_AAS_SETAASBLOCKINGENTITY,
		rtcw::to_vm_arg(absmin),
		rtcw::to_vm_arg(absmax),
		rtcw::to_vm_arg(blocking)
	);
}

void trap_AAS_RecordTeamDeathArea( vec3_t srcpos, int srcarea, int team, int teamCount, int travelflags ) {
	syscall(
		BOTLIB_AAS_RECORDTEAMDEATHAREA,
		rtcw::to_vm_arg(srcpos),
		rtcw::to_vm_arg(srcarea),
		rtcw::to_vm_arg(team),
		rtcw::to_vm_arg(teamCount),
		rtcw::to_vm_arg(travelflags)
	);
}
// done.

void trap_EA_Say( int client, const char *str ) {
	syscall(
		BOTLIB_EA_SAY,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(str)
	);
}

void trap_EA_SayTeam( int client, const char *str ) {
	syscall(
		BOTLIB_EA_SAY_TEAM,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(str)
	);
}

void trap_EA_UseItem( int client, const char *it ) {
	syscall(
		BOTLIB_EA_USE_ITEM,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(it)
	);
}

void trap_EA_DropItem( int client, const char *it ) {
	syscall(
		BOTLIB_EA_DROP_ITEM,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(it)
	);
}

void trap_EA_UseInv( int client, const char *inv ) {
	syscall(
		BOTLIB_EA_USE_INV,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(inv)
	);
}

void trap_EA_DropInv( int client, const char *inv ) {
	syscall(
		BOTLIB_EA_DROP_INV,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(inv)
	);
}

void trap_EA_Gesture( int client ) {
	syscall(
		BOTLIB_EA_GESTURE,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_Command( int client, const char *command ) {
	syscall(
		BOTLIB_EA_COMMAND,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(command)
	);
}

void trap_EA_SelectWeapon( int client, int weapon ) {
	syscall(
		BOTLIB_EA_SELECT_WEAPON,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(weapon)
	);
}

void trap_EA_Talk( int client ) {
	syscall(
		BOTLIB_EA_TALK,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_Attack( int client ) {
	syscall(
		BOTLIB_EA_ATTACK,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_Reload( int client ) {
	syscall(
		BOTLIB_EA_RELOAD,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_Activate( int client ) {
	syscall(
		BOTLIB_EA_USE,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_Respawn( int client ) {
	syscall(
		BOTLIB_EA_RESPAWN,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_Jump( int client ) {
	syscall(
		BOTLIB_EA_JUMP,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_DelayedJump( int client ) {
	syscall(
		BOTLIB_EA_DELAYED_JUMP,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_Crouch( int client ) {
	syscall(
		BOTLIB_EA_CROUCH,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_Walk( int client ) {
	syscall(
		BOTLIB_EA_WALK,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_MoveUp( int client ) {
	syscall(
		BOTLIB_EA_MOVE_UP,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_MoveDown( int client ) {
	syscall(
		BOTLIB_EA_MOVE_DOWN,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_MoveForward( int client ) {
	syscall(
		BOTLIB_EA_MOVE_FORWARD,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_MoveBack( int client ) {
	syscall(
		BOTLIB_EA_MOVE_BACK,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_MoveLeft( int client ) {
	syscall(
		BOTLIB_EA_MOVE_LEFT,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_MoveRight( int client ) {
	syscall(
		BOTLIB_EA_MOVE_RIGHT,
		rtcw::to_vm_arg(client)
	);
}

void trap_EA_Move( int client, vec3_t dir, float speed ) {
	syscall(
		BOTLIB_EA_MOVE,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(dir),
		rtcw::to_vm_arg(speed)
	);
}

void trap_EA_View( int client, vec3_t viewangles ) {
	syscall(
		BOTLIB_EA_VIEW,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(viewangles)
	);
}

void trap_EA_EndRegular( int client, float thinktime ) {
	syscall(
		BOTLIB_EA_END_REGULAR,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(thinktime)
	);
}

void trap_EA_GetInput( int client, float thinktime, void /* struct bot_input_s */ *input ) {
	syscall(
		BOTLIB_EA_GET_INPUT,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(thinktime),
		rtcw::to_vm_arg(input)
	);
}

void trap_EA_ResetInput( int client, void *init ) {
	syscall(
		BOTLIB_EA_RESET_INPUT,
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(init)
	);
}

void trap_EA_Prone( int client ) {
	syscall(
		BOTLIB_EA_PRONE,
		rtcw::to_vm_arg(client)
	);
}

int trap_BotLoadCharacter( char *charfile, int skill ) {
	return syscall(
		BOTLIB_AI_LOAD_CHARACTER,
		rtcw::to_vm_arg(charfile),
		rtcw::to_vm_arg(skill)
	);
}

void trap_BotFreeCharacter( int character ) {
	syscall(
		BOTLIB_AI_FREE_CHARACTER,
		rtcw::to_vm_arg(character)
	);
}

float trap_Characteristic_Float( int character, int index ) {
	int temp;
	temp = syscall(
		BOTLIB_AI_CHARACTERISTIC_FLOAT,
		rtcw::to_vm_arg(character),
		rtcw::to_vm_arg(index)
	);
	return ( *(float*)&temp );
}

float trap_Characteristic_BFloat( int character, int index, float min, float max ) {
	int temp;
	temp = syscall(
		BOTLIB_AI_CHARACTERISTIC_BFLOAT,
		rtcw::to_vm_arg(character),
		rtcw::to_vm_arg(index),
		rtcw::to_vm_arg(min),
		rtcw::to_vm_arg(max)
	);
	return ( *(float*)&temp );
}

int trap_Characteristic_Integer( int character, int index ) {
	return syscall(
		BOTLIB_AI_CHARACTERISTIC_INTEGER,
		rtcw::to_vm_arg(character),
		rtcw::to_vm_arg(index)
	);
}

int trap_Characteristic_BInteger( int character, int index, int min, int max ) {
	return syscall(
		BOTLIB_AI_CHARACTERISTIC_BINTEGER,
		rtcw::to_vm_arg(character),
		rtcw::to_vm_arg(index),
		rtcw::to_vm_arg(min),
		rtcw::to_vm_arg(max)
	);
}

void trap_Characteristic_String( int character, int index, char *buf, int size ) {
	syscall(
		BOTLIB_AI_CHARACTERISTIC_STRING,
		rtcw::to_vm_arg(character),
		rtcw::to_vm_arg(index),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(size)
	);
}

int trap_BotAllocChatState( void ) {
	return syscall(BOTLIB_AI_ALLOC_CHAT_STATE);
}

void trap_BotFreeChatState( int handle ) {
	syscall(
		BOTLIB_AI_FREE_CHAT_STATE,
		rtcw::to_vm_arg(handle)
	);
}

void trap_BotQueueConsoleMessage( int chatstate, int type, char *message ) {
	syscall(
		BOTLIB_AI_QUEUE_CONSOLE_MESSAGE,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(type),
		rtcw::to_vm_arg(message)
	);
}

void trap_BotRemoveConsoleMessage( int chatstate, int handle ) {
	syscall(
		BOTLIB_AI_REMOVE_CONSOLE_MESSAGE,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(handle)
	);
}

int trap_BotNextConsoleMessage( int chatstate, void /* struct bot_consolemessage_s */ *cm ) {
	return syscall(
		BOTLIB_AI_NEXT_CONSOLE_MESSAGE,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(cm)
	);
}

int trap_BotNumConsoleMessages( int chatstate ) {
	return syscall(
		BOTLIB_AI_NUM_CONSOLE_MESSAGE,
		rtcw::to_vm_arg(chatstate)
	);
}

void trap_BotInitialChat( int chatstate, const char *type, int mcontext, const char *var0, const char *var1, const char *var2, const char *var3, const char *var4, const char *var5, const char *var6, const char *var7 ) {
	syscall(
		BOTLIB_AI_INITIAL_CHAT,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(type),
		rtcw::to_vm_arg(mcontext),
		rtcw::to_vm_arg(var0),
		rtcw::to_vm_arg(var1),
		rtcw::to_vm_arg(var2),
		rtcw::to_vm_arg(var3),
		rtcw::to_vm_arg(var4),
		rtcw::to_vm_arg(var5),
		rtcw::to_vm_arg(var6),
		rtcw::to_vm_arg(var7)
	);
}

int trap_BotNumInitialChats( int chatstate, char *type ) {
	return syscall(
		BOTLIB_AI_NUM_INITIAL_CHATS,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(type)
	);
}

int trap_BotReplyChat( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) {
	return syscall(
		BOTLIB_AI_REPLY_CHAT,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(message),
		rtcw::to_vm_arg(mcontext),
		rtcw::to_vm_arg(vcontext),
		rtcw::to_vm_arg(var0),
		rtcw::to_vm_arg(var1),
		rtcw::to_vm_arg(var2),
		rtcw::to_vm_arg(var3),
		rtcw::to_vm_arg(var4),
		rtcw::to_vm_arg(var5),
		rtcw::to_vm_arg(var6),
		rtcw::to_vm_arg(var7)
	);
}

int trap_BotChatLength( int chatstate ) {
	return syscall(
		BOTLIB_AI_CHAT_LENGTH,
		rtcw::to_vm_arg(chatstate)
	);
}

void trap_BotEnterChat( int chatstate, int client, int sendto ) {
	// RF, disabled
	return;
	syscall(
		BOTLIB_AI_ENTER_CHAT,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(client),
		rtcw::to_vm_arg(sendto)
	);
}

void trap_BotGetChatMessage( int chatstate, char *buf, int size ) {
	syscall(
		BOTLIB_AI_GET_CHAT_MESSAGE,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(size)
	);
}

int trap_StringContains( char *str1, char *str2, int casesensitive ) {
	return syscall(
		BOTLIB_AI_STRING_CONTAINS,
		rtcw::to_vm_arg(str1),
		rtcw::to_vm_arg(str2),
		rtcw::to_vm_arg(casesensitive)
	);
}

int trap_BotFindMatch( char *str, void /* struct bot_match_s */ *match, uint32_t context ) {
	return syscall(
		BOTLIB_AI_FIND_MATCH,
		rtcw::to_vm_arg(str),
		rtcw::to_vm_arg(match),
		rtcw::to_vm_arg(context)
	);
}

void trap_BotMatchVariable( void /* struct bot_match_s */ *match, int variable, char *buf, int size ) {
	syscall(
		BOTLIB_AI_MATCH_VARIABLE,
		rtcw::to_vm_arg(match),
		rtcw::to_vm_arg(variable),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(size)
	);
}

void trap_UnifyWhiteSpaces( char *string ) {
	syscall(
		BOTLIB_AI_UNIFY_WHITE_SPACES,
		rtcw::to_vm_arg(string)
	);
}

void trap_BotReplaceSynonyms( char *string, uint32_t context ) {
	syscall(
		BOTLIB_AI_REPLACE_SYNONYMS,
		rtcw::to_vm_arg(string),
		rtcw::to_vm_arg(context)
	);
}

int trap_BotLoadChatFile( int chatstate, char *chatfile, char *chatname ) {
	return syscall(
		BOTLIB_AI_LOAD_CHAT_FILE,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(chatfile),
		rtcw::to_vm_arg(chatname)
	);
}

void trap_BotSetChatGender( int chatstate, int gender ) {
	syscall(
		BOTLIB_AI_SET_CHAT_GENDER,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(gender)
	);
}

void trap_BotSetChatName( int chatstate, char *name ) {
	syscall(
		BOTLIB_AI_SET_CHAT_NAME,
		rtcw::to_vm_arg(chatstate),
		rtcw::to_vm_arg(name)
	);
}

void trap_BotResetGoalState( int goalstate ) {
	syscall(
		BOTLIB_AI_RESET_GOAL_STATE,
		rtcw::to_vm_arg(goalstate)
	);
}

void trap_BotResetAvoidGoals( int goalstate ) {
	syscall(
		BOTLIB_AI_RESET_AVOID_GOALS,
		rtcw::to_vm_arg(goalstate)
	);
}

void trap_BotRemoveFromAvoidGoals( int goalstate, int number ) {
	syscall(
		BOTLIB_AI_REMOVE_FROM_AVOID_GOALS,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(number)
	);
}

void trap_BotPushGoal( int goalstate, void /* struct bot_goal_s */ *goal ) {
	syscall(
		BOTLIB_AI_PUSH_GOAL,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(goal)
	);
}

void trap_BotPopGoal( int goalstate ) {
	syscall(
		BOTLIB_AI_POP_GOAL,
		rtcw::to_vm_arg(goalstate)
	);
}

void trap_BotEmptyGoalStack( int goalstate ) {
	syscall(
		BOTLIB_AI_EMPTY_GOAL_STACK,
		rtcw::to_vm_arg(goalstate)
	);
}

void trap_BotDumpAvoidGoals( int goalstate ) {
	syscall(
		BOTLIB_AI_DUMP_AVOID_GOALS,
		rtcw::to_vm_arg(goalstate)
	);
}

void trap_BotDumpGoalStack( int goalstate ) {
	syscall(
		BOTLIB_AI_DUMP_GOAL_STACK,
		rtcw::to_vm_arg(goalstate)
	);
}

void trap_BotGoalName( int number, char *name, int size ) {
	syscall(
		BOTLIB_AI_GOAL_NAME,
		rtcw::to_vm_arg(number),
		rtcw::to_vm_arg(name),
		rtcw::to_vm_arg(size)
	);
}

int trap_BotGetTopGoal( int goalstate, void /* struct bot_goal_s */ *goal ) {
	return syscall(
		BOTLIB_AI_GET_TOP_GOAL,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(goal)
	);
}

int trap_BotGetSecondGoal( int goalstate, void /* struct bot_goal_s */ *goal ) {
	return syscall(
		BOTLIB_AI_GET_SECOND_GOAL,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(goal)
	);
}

int trap_BotChooseLTGItem( int goalstate, vec3_t origin, int *inventory, int travelflags ) {
	return syscall(
		BOTLIB_AI_CHOOSE_LTG_ITEM,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(inventory),
		rtcw::to_vm_arg(travelflags)
	);
}

int trap_BotChooseNBGItem( int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime ) {
	return syscall(
		BOTLIB_AI_CHOOSE_NBG_ITEM,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(inventory),
		rtcw::to_vm_arg(travelflags),
		rtcw::to_vm_arg(ltg),
		rtcw::to_vm_arg(maxtime)
	);
}

int trap_BotTouchingGoal( vec3_t origin, void /* struct bot_goal_s */ *goal ) {
	return syscall(
		BOTLIB_AI_TOUCHING_GOAL,
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(goal)
	);
}

int trap_BotItemGoalInVisButNotVisible( int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal ) {
	return syscall(
		BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE,
		rtcw::to_vm_arg(viewer),
		rtcw::to_vm_arg(eye),
		rtcw::to_vm_arg(viewangles),
		rtcw::to_vm_arg(goal)
	);
}

int trap_BotGetLevelItemGoal( int index, const char *classname, void /* struct bot_goal_s */ *goal ) {
	return syscall(
		BOTLIB_AI_GET_LEVEL_ITEM_GOAL,
		rtcw::to_vm_arg(index),
		rtcw::to_vm_arg(classname),
		rtcw::to_vm_arg(goal)
	);
}

int trap_BotGetNextCampSpotGoal( int num, void /* struct bot_goal_s */ *goal ) {
	return syscall(
		BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL,
		rtcw::to_vm_arg(num),
		rtcw::to_vm_arg(goal)
	);
}

int trap_BotGetMapLocationGoal( char *name, void /* struct bot_goal_s */ *goal ) {
	return syscall(
		BOTLIB_AI_GET_MAP_LOCATION_GOAL,
		rtcw::to_vm_arg(name),
		rtcw::to_vm_arg(goal)
	);
}

float trap_BotAvoidGoalTime( int goalstate, int number ) {
	int temp;
	temp = syscall(
		BOTLIB_AI_AVOID_GOAL_TIME,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(number)
	);
	return ( *(float*)&temp );
}

void trap_BotInitLevelItems( void ) {
	syscall(BOTLIB_AI_INIT_LEVEL_ITEMS);
}

void trap_BotUpdateEntityItems( void ) {
	syscall(BOTLIB_AI_UPDATE_ENTITY_ITEMS);
}

int trap_BotLoadItemWeights( int goalstate, char *filename ) {
	return syscall(
		BOTLIB_AI_LOAD_ITEM_WEIGHTS,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(filename)
	);
}

void trap_BotFreeItemWeights( int goalstate ) {
	syscall(
		BOTLIB_AI_FREE_ITEM_WEIGHTS,
		rtcw::to_vm_arg(goalstate)
	);
}

void trap_BotInterbreedGoalFuzzyLogic( int parent1, int parent2, int child ) {
	syscall(
		BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC,
		rtcw::to_vm_arg(parent1),
		rtcw::to_vm_arg(parent2),
		rtcw::to_vm_arg(child)
	);
}

void trap_BotSaveGoalFuzzyLogic( int goalstate, char *filename ) {
	syscall(
		BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(filename)
	);
}

void trap_BotMutateGoalFuzzyLogic( int goalstate, float range ) {
	syscall(
		BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC,
		rtcw::to_vm_arg(goalstate),
		rtcw::to_vm_arg(range)
	);
}

int trap_BotAllocGoalState( int state ) {
	return syscall(
		BOTLIB_AI_ALLOC_GOAL_STATE,
		rtcw::to_vm_arg(state)
	);
}

void trap_BotFreeGoalState( int handle ) {
	syscall(
		BOTLIB_AI_FREE_GOAL_STATE,
		rtcw::to_vm_arg(handle)
	);
}

void trap_BotResetMoveState( int movestate ) {
	syscall(
		BOTLIB_AI_RESET_MOVE_STATE,
		rtcw::to_vm_arg(movestate)
	);
}

void trap_BotMoveToGoal( void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags ) {
	syscall(
		BOTLIB_AI_MOVE_TO_GOAL,
		rtcw::to_vm_arg(result),
		rtcw::to_vm_arg(movestate),
		rtcw::to_vm_arg(goal),
		rtcw::to_vm_arg(travelflags)
	);
}

int trap_BotMoveInDirection( int movestate, vec3_t dir, float speed, int type ) {
	return syscall(
		BOTLIB_AI_MOVE_IN_DIRECTION,
		rtcw::to_vm_arg(movestate),
		rtcw::to_vm_arg(dir),
		rtcw::to_vm_arg(speed),
		rtcw::to_vm_arg(type)
	);
}

void trap_BotResetAvoidReach( int movestate ) {
	syscall(
		BOTLIB_AI_RESET_AVOID_REACH,
		rtcw::to_vm_arg(movestate)
	);
}

void trap_BotResetLastAvoidReach( int movestate ) {
	syscall(
		BOTLIB_AI_RESET_LAST_AVOID_REACH,
		rtcw::to_vm_arg(movestate)
	);
}

int trap_BotReachabilityArea( vec3_t origin, int testground ) {
	return syscall(
		BOTLIB_AI_REACHABILITY_AREA,
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(testground)
	);
}

int trap_BotMovementViewTarget( int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target ) {
	return syscall(
		BOTLIB_AI_MOVEMENT_VIEW_TARGET,
		rtcw::to_vm_arg(movestate),
		rtcw::to_vm_arg(goal),
		rtcw::to_vm_arg(travelflags),
		rtcw::to_vm_arg(lookahead),
		rtcw::to_vm_arg(target)
	);
}

int trap_BotPredictVisiblePosition( vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target ) {
	return syscall(
		BOTLIB_AI_PREDICT_VISIBLE_POSITION,
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(areanum),
		rtcw::to_vm_arg(goal),
		rtcw::to_vm_arg(travelflags),
		rtcw::to_vm_arg(target)
	);
}

int trap_BotAllocMoveState( void ) {
	return syscall(BOTLIB_AI_ALLOC_MOVE_STATE);
}

void trap_BotFreeMoveState( int handle ) {
	syscall(
		BOTLIB_AI_FREE_MOVE_STATE,
		rtcw::to_vm_arg(handle)
	);
}

void trap_BotInitMoveState( int handle, void /* struct bot_initmove_s */ *initmove ) {
	syscall(
		BOTLIB_AI_INIT_MOVE_STATE,
		rtcw::to_vm_arg(handle),
		rtcw::to_vm_arg(initmove)
	);
}

// Ridah
void trap_BotInitAvoidReach( int handle ) {
	syscall(
		BOTLIB_AI_INIT_AVOID_REACH,
		rtcw::to_vm_arg(handle)
	);
}
// Done.

int trap_BotChooseBestFightWeapon( int weaponstate, int *inventory ) {
	return syscall(
		BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON,
		rtcw::to_vm_arg(weaponstate),
		rtcw::to_vm_arg(inventory)
	);
}

void trap_BotGetWeaponInfo( int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo ) {
	syscall(
		BOTLIB_AI_GET_WEAPON_INFO,
		rtcw::to_vm_arg(weaponstate),
		rtcw::to_vm_arg(weapon),
		rtcw::to_vm_arg(weaponinfo)
	);
}

int trap_BotLoadWeaponWeights( int weaponstate, char *filename ) {
	return syscall(
		BOTLIB_AI_LOAD_WEAPON_WEIGHTS,
		rtcw::to_vm_arg(weaponstate),
		rtcw::to_vm_arg(filename)
	);
}

int trap_BotAllocWeaponState( void ) {
	return syscall(BOTLIB_AI_ALLOC_WEAPON_STATE);
}

void trap_BotFreeWeaponState( int weaponstate ) {
	syscall(
		BOTLIB_AI_FREE_WEAPON_STATE,
		rtcw::to_vm_arg(weaponstate)
	);
}

void trap_BotResetWeaponState( int weaponstate ) {
	syscall(
		BOTLIB_AI_RESET_WEAPON_STATE,
		rtcw::to_vm_arg(weaponstate)
	);
}

int trap_GeneticParentsAndChildSelection( int numranks, float *ranks, int *parent1, int *parent2, int *child ) {
	return syscall(
		BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION,
		rtcw::to_vm_arg(numranks),
		rtcw::to_vm_arg(ranks),
		rtcw::to_vm_arg(parent1),
		rtcw::to_vm_arg(parent2),
		rtcw::to_vm_arg(child)
	);
}

void trap_PbStat( int clientNum, const char *category, const char *values ) {
	syscall(
		PB_STAT_REPORT,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(category),
		rtcw::to_vm_arg(values)
	);
}

void trap_SendMessage( int clientNum, char *buf, int buflen ) {
	syscall(
		G_SENDMESSAGE,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

messageStatus_t trap_MessageStatus( int clientNum ) {
	return static_cast<messageStatus_t>(syscall(
		G_MESSAGESTATUS,
		rtcw::to_vm_arg(clientNum)
	));
}
