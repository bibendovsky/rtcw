/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

//===========================================================================
//
// Name:			ai_cast_debug.c
// Function:		Wolfenstein AI Character Routines
// Programmer:		Ridah
// Tab Size:		4 (real tabs)
//===========================================================================

#include "g_local.h"
#include "botlib.h"      //bot lib interface
#include "be_aas.h"
#include "be_ea.h"
#include "be_ai_gen.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"
#include "botai.h"          //bot ai interface

#include "ai_cast.h"

static int numaifuncs;
static const char     *aifuncs[MAX_AIFUNCS];

/*
==========
AICast_DBG_InitAIFuncs
==========
*/
void AICast_DBG_InitAIFuncs( void ) {
	numaifuncs = 0;
}

/*
==========
AICast_DBG_AddAIFunc
==========
*/
void AICast_DBG_AddAIFunc( cast_state_t *cs, const char *funcname ) {
	if ( aicast_debug.integer ) {
		if ( aicast_debug.integer != 2 || ( g_entities[cs->entityNum].aiName && !strcmp( aicast_debugname.string, g_entities[cs->entityNum].aiName ) ) ) {
			G_Printf( "%s: %s\n", g_entities[cs->entityNum].aiName, funcname );
		}
	}
	aifuncs[numaifuncs] = funcname;
	numaifuncs++;
}

/*
==========
AICast_DBG_ListAIFuncs
==========
*/
void AICast_DBG_ListAIFuncs( cast_state_t *cs, int numprint ) {
	int i;

	if ( aicast_debug.integer != 2 || ( g_entities[cs->entityNum].aiName && !strcmp( aicast_debugname.string, g_entities[cs->entityNum].aiName ) ) ) {
		AICast_Printf( AICAST_PRT_DEBUG, S_COLOR_RED "AICast_ProcessAIFunctions: executed more than %d AI funcs\n", MAX_AIFUNCS );
		for ( i = MAX_AIFUNCS - numprint; i < MAX_AIFUNCS; i++ )
			AICast_Printf( AICAST_PRT_DEBUG, "%s, ", aifuncs[i] );
		AICast_Printf( AICAST_PRT_DEBUG, "\n" );
	}
}

/*
==========
AICast_DebugFrame
==========
*/
void AICast_DebugFrame( cast_state_t *cs ) {
	gentity_t *ent;

	if ( aicast_debug.integer ) {
		ent = &g_entities[cs->entityNum];

		if ( cs->castScriptStatus.castScriptEventIndex >= 0 ) {
			ent->client->ps.eFlags |= EF_TALK;
		} else {
			ent->client->ps.eFlags &= ~EF_TALK;
		}
	}
}

/*
===========
AICast_DBG_RouteTable_f
===========
*/
void AICast_DBG_RouteTable_f( vec3_t org, char *param ) {
	static int srcarea = 0, dstarea = 0;
//	extern botlib_export_t botlib; // TTimo: unused

	if ( !param || strlen( param ) < 1 ) {
		trap_Printf( "You must specify 'src', 'dest' or 'show'\n" );
		return;
	}

	trap_AAS_SetCurrentWorld( 0 );  // use the default world, which should have a routetable

	if ( Q_stricmp( param, "toggle" ) == 0 ) {
		trap_AAS_RT_ShowRoute( vec3_origin, -666, -666 );   // stupid toggle hack
		return;
	}

	if ( Q_stricmp( param, "src" ) == 0 ) { // set the src
		srcarea = 1 + trap_AAS_PointAreaNum( org );
		return;
	} else if ( Q_stricmp( param, "dest" ) == 0 )        {
		dstarea = 1 + trap_AAS_PointAreaNum( org );
	}

	if ( srcarea && dstarea ) { // show the path
		trap_AAS_RT_ShowRoute( org, srcarea - 1, dstarea - 1 );
	} else
	{
		trap_Printf( "You must specify 'src' & 'dest' first\n" );
	}
}

/*
===============
AICast_DBG_Spawn_f
===============
*/
void AICast_DBG_Spawn_f( gclient_t *client, const char *cmd ) {
	extern qboolean G_CallSpawn( gentity_t *ent );
	gentity_t   *ent;
	vec3_t dir;

	ent = G_Spawn();
	ent->classname = static_cast<char*> (G_Alloc( strlen( cmd ) + 1 ));
	strcpy( ent->classname, cmd );
	AngleVectors( client->ps.viewangles, dir, NULL, NULL );
	VectorMA( client->ps.origin, 96, dir, ent->s.origin );

	if ( !G_CallSpawn( ent ) ) {
		G_Printf( "Error: unable to spawn \"%s\" entity\n", cmd );
	}
}

/*
===============
AICast_DBG_Cmd_f

  General entry point for all "aicast ..." commands
===============
*/
void AICast_DBG_Cmd_f( int clientNum ) {
	gentity_t *ent;
	char cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;     // not fully in game yet
	}

	// get the first word following "aicast"
	trap_Argv( 1, cmd, sizeof( cmd ) );

	if ( Q_stricmp( cmd, "dbg_routetable" ) == 0 ) {
		trap_Argv( 2, cmd, sizeof( cmd ) );
		AICast_DBG_RouteTable_f( ent->client->ps.origin, cmd );
		return;
	}
	if ( Q_stricmp( cmd, "spawn" ) == 0 ) {
		// spawn a given character
		trap_Argv( 2, cmd, sizeof( cmd ) );
		AICast_DBG_Spawn_f( ent->client, cmd );
		return;
	}
	if ( Q_stricmp( cmd, "getname" ) == 0 ) {
		// get name of character we're looking at
//		AICast_DBG_GetName_f(ent);
		return;
	}
	if ( Q_stricmp( cmd, "followme" ) == 0 ) {
		// tell character to follow us
		trap_Argv( 2, cmd, sizeof( cmd ) );
//		AICast_DBG_FollowMe_f(ent->client, cmd);
		return;
	}
}
/*
// Ridah, faster Win32 code
#ifdef _WIN32
#undef MAX_PATH		// this is an ugly hack, to temporarily ignore the current definition, since it's also defined in windows.h
#include <windows.h>
#undef MAX_PATH
#define MAX_PATH	MAX_QPATH
#endif

int Sys_MilliSeconds(void)
{
// Ridah, faster Win32 code
#ifdef _WIN32
	int			sys_curtime;
	static qboolean	initialized = qfalse;
	static int	sys_timeBase;

	if (!initialized) {
		sys_timeBase = timeGetTime();
		initialized = qtrue;
	}
	sys_curtime = timeGetTime() - sys_timeBase;

	return sys_curtime;
#else
	return clock() * 1000 / CLOCKS_PER_SEC;
#endif
} //end of the function Sys_MilliSeconds
*/
