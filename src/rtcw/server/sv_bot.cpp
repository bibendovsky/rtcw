/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// sv_bot.c


#include "server.h"
#include "botlib.h"
#include "botai.h"

#include "rtcw_vm_args.h"


#if !defined RTCW_ET
#define MAX_DEBUGPOLYS      128

typedef struct bot_debugpoly_s
{
	int inuse;
	int color;
	int numPoints;
	vec3_t points[128];
} bot_debugpoly_t;
#endif // RTCW_XX

static bot_debugpoly_t debugpolygons[MAX_DEBUGPOLYS];

extern botlib_export_t  *botlib_export;
int bot_enable;

/*
==================
SV_BotAllocateClient
==================
*/
#if !defined RTCW_ET
int SV_BotAllocateClient( void ) {
#else
int SV_BotAllocateClient( int clientNum ) {
#endif // RTCW_XX

	int i;
	client_t    *cl;

#if defined RTCW_ET
	// Arnout: added possibility to request a clientnum
	if ( clientNum > 0 ) {
		if ( clientNum >= sv_maxclients->integer ) {
			return -1;
		}

		cl = &svs.clients[clientNum];
		if ( cl->state != CS_FREE ) {
			return -1;
		} else {
			i = clientNum;
		}
	} else {
#endif // RTCW_XX

	// find a client slot
	for ( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ ) {
		// Wolfenstein, never use the first slot, otherwise if a bot connects before the first client on a listen server, game won't start
		if ( i < 1 ) {
			continue;
		}
		// done.
		if ( cl->state == CS_FREE ) {
			break;
		}
	}

#if defined RTCW_ET
	}
#endif // RTCW_XX

	if ( i == sv_maxclients->integer ) {
		return -1;
	}

	cl->gentity = SV_GentityNum( i );
	cl->gentity->s.number = i;
	cl->state = CS_ACTIVE;
	cl->lastPacketTime = svs.time;
	cl->netchan.remoteAddress.type = NA_BOT;
	cl->rate = 16384;

	return i;
}

/*
==================
SV_BotFreeClient
==================
*/
void SV_BotFreeClient( int clientNum ) {
	client_t    *cl;

	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_BotFreeClient: bad clientNum: %i", clientNum );
	}
	cl = &svs.clients[clientNum];
	cl->state = CS_FREE;
	cl->name[0] = 0;
	if ( cl->gentity ) {
		cl->gentity->r.svFlags &= ~SVF_BOT;
	}
}

/*
==================
BotDrawDebugPolygons
==================
*/

#if !defined RTCW_ET
void BotDrawDebugPolygons( void ( *drawPoly )( int color, int numPoints, float *points ), int value ) {
	static cvar_t *bot_debug, *bot_groundonly, *bot_reachability, *bot_highlightarea;
#else
void BotDrawDebugPolygons( BotPolyFunc drawPoly, int value ) {
	static cvar_t *bot_debug, *bot_groundonly, *bot_reachability, *bot_highlightarea;
#endif // RTCW_XX

#if defined RTCW_SP
	static cvar_t *bot_testhidepos, *bot_testroutevispos;
#else
	static cvar_t *bot_testhidepos;
#endif // RTCW_XX

	bot_debugpoly_t *poly;
	int i, parm0;

#if defined RTCW_ET
	static cvar_t   *debugSurface;
#endif // RTCW_XX

#if !defined RTCW_SP
#ifdef PRE_RELEASE_DEMO
	return;
#endif
#endif // RTCW_XX

	if ( !bot_enable ) {
		return;
	}
	//bot debugging
	if ( !bot_debug ) {
		bot_debug = Cvar_Get( "bot_debug", "0", 0 );
	}
	//show reachabilities
	if ( !bot_reachability ) {
		bot_reachability = Cvar_Get( "bot_reachability", "0", 0 );
	}
	//show ground faces only
	if ( !bot_groundonly ) {
		bot_groundonly = Cvar_Get( "bot_groundonly", "1", 0 );
	}
	//get the hightlight area
	if ( !bot_highlightarea ) {
		bot_highlightarea = Cvar_Get( "bot_highlightarea", "0", 0 );
	}
	//
	if ( !bot_testhidepos ) {
		bot_testhidepos = Cvar_Get( "bot_testhidepos", "0", 0 );
	}
	//

#if defined RTCW_SP
	if ( !bot_testroutevispos ) {
		bot_testroutevispos = Cvar_Get( "bot_testroutevispos", "0", 0 );
	}

	//
#endif // RTCW_XX

#if defined RTCW_ET
	if ( !debugSurface ) {
		debugSurface = Cvar_Get( "r_debugSurface", "0", 0 );
	}
#endif // RTCW_XX

#if !defined RTCW_ET
	if ( bot_debug->integer ) {
#else
	//
	if ( bot_debug->integer == 1 || bot_debug->integer == 9 ) {
#endif // RTCW_XX

		parm0 = 0;
		if ( svs.clients[0].lastUsercmd.buttons & BUTTON_ATTACK ) {
			parm0 |= 1;
		}
		if ( bot_reachability->integer ) {
			parm0 |= 2;
		}
		if ( bot_groundonly->integer ) {
			parm0 |= 4;
		}

#if defined RTCW_ET
		if ( debugSurface->integer == 3 ) {
			parm0 |= 8;
		} else if ( debugSurface->integer == 4 ) {
			parm0 |= 4 | 8;
		} else if ( debugSurface->integer == 5 ) {
			parm0 |= 8 | 16;
		}
#endif // RTCW_XX

		botlib_export->BotLibVarSet( "bot_highlightarea", bot_highlightarea->string );
		botlib_export->BotLibVarSet( "bot_testhidepos", bot_testhidepos->string );

#if defined RTCW_SP
		botlib_export->BotLibVarSet( "bot_testroutevispos", bot_testroutevispos->string );
#endif // RTCW_XX

#if defined RTCW_ET
		botlib_export->BotLibVarSet( "bot_debug", bot_debug->string );
#endif // RTCW_XX

		botlib_export->Test( parm0, NULL, svs.clients[0].gentity->r.currentOrigin,
							 svs.clients[0].gentity->r.currentAngles );
	} //end if
	for ( i = 0; i < MAX_DEBUGPOLYS; i++ ) {
		poly = &debugpolygons[i];
		if ( !poly->inuse ) {
			continue;
		}
		drawPoly( poly->color, poly->numPoints, (float *) poly->points );
		//Com_Printf("poly %i, numpoints = %d\n", i, poly->numPoints);
	}
}

/*
==================
BotImport_Print
==================
*/
void QDECL BotImport_Print( int type, const char *fmt, ... ) {
	char str[2048];
	va_list ap;

	va_start( ap, fmt );

#if defined RTCW_SP
	vsprintf( str, fmt, ap );
#else
	Q_vsnprintf( str, sizeof( str ), fmt, ap );
#endif // RTCW_XX

	va_end( ap );

	switch ( type ) {
	case PRT_MESSAGE: {
		Com_Printf( "%s", str );
		break;
	}
	case PRT_WARNING: {
		Com_Printf( S_COLOR_YELLOW "Warning: %s", str );
		break;
	}
	case PRT_ERROR: {
		Com_Printf( S_COLOR_RED "Error: %s", str );
		break;
	}
	case PRT_FATAL: {
		Com_Printf( S_COLOR_RED "Fatal: %s", str );
		break;
	}
	case PRT_EXIT: {
		Com_Error( ERR_DROP, S_COLOR_RED "Exit: %s", str );
		break;
	}
	default: {
		Com_Printf( "unknown print type\n" );
		break;
	}
	}
}

/*
==================
BotImport_Trace
==================
*/
void BotImport_Trace( bsp_trace_t *bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passent, int contentmask ) {
	trace_t trace;

	// always use bounding box for bot stuff ?
	SV_Trace( &trace, start, mins, maxs, end, passent, contentmask, qfalse );
	//copy the trace information
	bsptrace->allsolid = trace.allsolid;
	bsptrace->startsolid = trace.startsolid;
	bsptrace->fraction = trace.fraction;
	VectorCopy( trace.endpos, bsptrace->endpos );
	bsptrace->plane.dist = trace.plane.dist;
	VectorCopy( trace.plane.normal, bsptrace->plane.normal );
	bsptrace->plane.signbits = trace.plane.signbits;
	bsptrace->plane.type = trace.plane.type;
	bsptrace->surface.value = trace.surfaceFlags;
	bsptrace->ent = trace.entityNum;
	bsptrace->exp_dist = 0;
	bsptrace->sidenum = 0;
	bsptrace->contents = 0;
}

/*
==================
BotImport_EntityTrace
==================
*/
void BotImport_EntityTrace( bsp_trace_t *bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int entnum, int contentmask ) {
	trace_t trace;

	// always use bounding box for bot stuff ?
	SV_ClipToEntity( &trace, start, mins, maxs, end, entnum, contentmask, qfalse );
	//copy the trace information
	bsptrace->allsolid = trace.allsolid;
	bsptrace->startsolid = trace.startsolid;
	bsptrace->fraction = trace.fraction;
	VectorCopy( trace.endpos, bsptrace->endpos );
	bsptrace->plane.dist = trace.plane.dist;
	VectorCopy( trace.plane.normal, bsptrace->plane.normal );
	bsptrace->plane.signbits = trace.plane.signbits;
	bsptrace->plane.type = trace.plane.type;
	bsptrace->surface.value = trace.surfaceFlags;
	bsptrace->ent = trace.entityNum;
	bsptrace->exp_dist = 0;
	bsptrace->sidenum = 0;
	bsptrace->contents = 0;
}


/*
==================
BotImport_PointContents
==================
*/
int BotImport_PointContents( vec3_t point ) {
	return SV_PointContents( point, -1 );
}

/*
==================
BotImport_inPVS
==================
*/
int BotImport_inPVS( vec3_t p1, vec3_t p2 ) {
	return SV_inPVS( p1, p2 );
}

/*
==================
BotImport_BSPEntityData
==================
*/
char *BotImport_BSPEntityData( void ) {
	return CM_EntityString();
}

/*
==================
BotImport_BSPModelMinsMaxsOrigin
==================
*/
void BotImport_BSPModelMinsMaxsOrigin( int modelnum, vec3_t angles, vec3_t outmins, vec3_t outmaxs, vec3_t origin ) {
	clipHandle_t h;
	vec3_t mins, maxs;
	float max;
	int i;

	h = CM_InlineModel( modelnum );
	CM_ModelBounds( h, mins, maxs );
	//if the model is rotated
	if ( ( angles[0] || angles[1] || angles[2] ) ) {
		// expand for rotation

		max = RadiusFromBounds( mins, maxs );
		for ( i = 0; i < 3; i++ ) {
			mins[i] = -max;
			maxs[i] = max;
		}
	}
	if ( outmins ) {
		VectorCopy( mins, outmins );
	}
	if ( outmaxs ) {
		VectorCopy( maxs, outmaxs );
	}
	if ( origin ) {
		VectorClear( origin );
	}
}

/*
==================
BotImport_GetMemory
==================
*/
void *BotImport_GetMemory( int size ) {

#if defined RTCW_SP
	return malloc( size );
#else
	void *ptr;

	ptr = Z_TagMalloc( size, TAG_BOTLIB );
	return ptr;
#endif // RTCW_XX

}

/*
==================
BotImport_FreeMemory
==================
*/
void BotImport_FreeMemory( void *ptr ) {

#if defined RTCW_SP
	free( ptr );
#else
	Z_Free( ptr );
#endif // RTCW_XX

}

#if !defined RTCW_SP
/*
==================
BotImport_FreeZoneMemory
==================
*/
void BotImport_FreeZoneMemory( void ) {
	Z_FreeTags( TAG_BOTLIB );
}
#endif // RTCW_XX

/*
=================
BotImport_HunkAlloc
=================
*/
void *BotImport_HunkAlloc( int size ) {
	if ( Hunk_CheckMark() ) {
		Com_Error( ERR_DROP, "SV_Bot_HunkAlloc: Alloc with marks already set\n" );
	}
	return Hunk_Alloc( size, h_high );
}

/*
==================
BotImport_DebugPolygonCreate
==================
*/
int BotImport_DebugPolygonCreate( int color, int numPoints, vec3_t *points ) {
	bot_debugpoly_t *poly;
	int i;

	for ( i = 1; i < MAX_DEBUGPOLYS; i++ )    {
		if ( !debugpolygons[i].inuse ) {
			break;
		}
	}
	if ( i >= MAX_DEBUGPOLYS ) {
		return 0;
	}
	poly = &debugpolygons[i];
	poly->inuse = qtrue;
	poly->color = color;
	poly->numPoints = numPoints;
	memcpy( poly->points, points, numPoints * sizeof( vec3_t ) );
	//
	return i;
}

#if defined RTCW_ET
/*
==================
BotImport_DebugPolygonCreate
==================
*/
bot_debugpoly_t* BotImport_GetFreeDebugPolygon( void ) {
	int i;

	for ( i = 1; i < MAX_DEBUGPOLYS; i++ )    {
		if ( !debugpolygons[i].inuse ) {
			return &debugpolygons[i];
		}
	}
	return NULL;
}
#endif // RTCW_XX

/*
==================
BotImport_DebugPolygonShow
==================
*/
void BotImport_DebugPolygonShow( int id, int color, int numPoints, vec3_t *points ) {
	bot_debugpoly_t *poly;

	poly = &debugpolygons[id];
	poly->inuse = qtrue;
	poly->color = color;
	poly->numPoints = numPoints;
	memcpy( poly->points, points, numPoints * sizeof( vec3_t ) );
}

/*
==================
BotImport_DebugPolygonDelete
==================
*/
void BotImport_DebugPolygonDelete( int id ) {
	debugpolygons[id].inuse = qfalse;
}

#if defined RTCW_ET
/*
==================
BotImport_DebugPolygonDeletePointer
==================
*/
void BotImport_DebugPolygonDeletePointer( bot_debugpoly_t* pPoly ) {
	pPoly->inuse = qfalse;
}
#endif // RTCW_XX

/*
==================
BotImport_DebugLineCreate
==================
*/
int BotImport_DebugLineCreate( void ) {
	vec3_t points[1];
	return BotImport_DebugPolygonCreate( 0, 0, points );
}

/*
==================
BotImport_DebugLineDelete
==================
*/
void BotImport_DebugLineDelete( int line ) {
	BotImport_DebugPolygonDelete( line );
}

/*
==================
BotImport_DebugLineShow
==================
*/
void BotImport_DebugLineShow( int line, vec3_t start, vec3_t end, int color ) {
	vec3_t points[4], dir, cross, up = {0, 0, 1};
	float dot;

	VectorCopy( start, points[0] );
	VectorCopy( start, points[1] );
	//points[1][2] -= 2;
	VectorCopy( end, points[2] );
	//points[2][2] -= 2;
	VectorCopy( end, points[3] );


	VectorSubtract( end, start, dir );
	VectorNormalize( dir );
	dot = DotProduct( dir, up );
	if ( dot > 0.99 || dot < -0.99 ) {
		VectorSet( cross, 1, 0, 0 );
	} else { CrossProduct( dir, up, cross );}

	VectorNormalize( cross );

	VectorMA( points[0], 2, cross, points[0] );
	VectorMA( points[1], -2, cross, points[1] );
	VectorMA( points[2], -2, cross, points[2] );
	VectorMA( points[3], 2, cross, points[3] );

	BotImport_DebugPolygonShow( line, color, 4, points );
}

#if defined RTCW_ET
/*
==================
BotImport_BotVisibleFromPos
==================
*/
qboolean BotImport_BotVisibleFromPos( vec3_t srcorigin, int srcnum, vec3_t destorigin, int destent, qboolean dummy ) {
	return VM_Call(
		gvm,
		BOT_VISIBLEFROMPOS,
		rtcw::to_vm_arg(srcorigin),
		rtcw::to_vm_arg(srcnum),
		rtcw::to_vm_arg(destorigin),
		rtcw::to_vm_arg(destent),
		rtcw::to_vm_arg(dummy)
	);
}

/*
==================
BotImport_BotCheckAttackAtPos
==================
*/
qboolean BotImport_BotCheckAttackAtPos( int entnum, int enemy, vec3_t pos, qboolean ducking, qboolean allowHitWorld ) {
	return VM_Call(
		gvm,
		BOT_CHECKATTACKATPOS,
		rtcw::to_vm_arg(entnum),
		rtcw::to_vm_arg(enemy),
		rtcw::to_vm_arg(pos),
		rtcw::to_vm_arg(ducking),
		rtcw::to_vm_arg(allowHitWorld)
	);
}
#endif // RTCW_XX

/*
==================
SV_BotClientCommand
==================
*/
void BotClientCommand( int client, const char *command ) {

#if !defined RTCW_ET
	SV_ExecuteClientCommand( &svs.clients[client], command, qtrue );
#else
	SV_ExecuteClientCommand( &svs.clients[client], command, qtrue, qfalse );
#endif // RTCW_XX

}

/*
==================
SV_BotFrame
==================
*/
void SV_BotFrame( int time ) {

#if !defined RTCW_SP
#ifdef PRE_RELEASE_DEMO
	return;
#endif
#endif // RTCW_XX

	if ( !bot_enable ) {
		return;
	}
	//NOTE: maybe the game is already shutdown
	if ( !gvm ) {
		return;
	}
	VM_Call(gvm, BOTAI_START_FRAME, rtcw::to_vm_arg(time));
}

/*
===============
SV_BotLibSetup
===============
*/
int SV_BotLibSetup( void ) {

#if defined RTCW_ET
	static cvar_t *bot_norcd;
	static cvar_t *bot_frameroutingupdates;
#endif // RTCW_XX

#if !defined RTCW_SP
#ifdef PRE_RELEASE_DEMO
	return 0;
#endif
#endif // RTCW_XX

	if ( !bot_enable ) {
		return 0;
	}

	if ( !botlib_export ) {
		Com_Printf( S_COLOR_RED "Error: SV_BotLibSetup without SV_BotInitBotLib\n" );
		return -1;
	}

#if !defined RTCW_ET
	return botlib_export->BotLibSetup();
#else
	// RF, set RCD calculation status
	bot_norcd = Cvar_Get( "bot_norcd", "0", 0 );
	botlib_export->BotLibVarSet( "bot_norcd", bot_norcd->string );

	// RF, set AAS routing max per frame
	if ( SV_GameIsSinglePlayer() ) {
		bot_frameroutingupdates = Cvar_Get( "bot_frameroutingupdates", "9999999", 0 );
	} else {    // more restrictive in multiplayer
		bot_frameroutingupdates = Cvar_Get( "bot_frameroutingupdates", "1000", 0 );
	}
	botlib_export->BotLibVarSet( "bot_frameroutingupdates", bot_frameroutingupdates->string );

// START	Arnout changes, 28-08-2002.
// added single player
	return botlib_export->BotLibSetup( ( SV_GameIsSinglePlayer() || SV_GameIsCoop() ) );
// END	Arnout changes, 28-08-2002.
#endif // RTCW_XX

}

/*
===============
SV_ShutdownBotLib

Called when either the entire server is being killed, or
it is changing to a different game directory.
===============
*/
int SV_BotLibShutdown( void ) {

	if ( !botlib_export ) {
		return -1;
	}

	return botlib_export->BotLibShutdown();
}

/*
==================
SV_BotInitCvars
==================
*/
void SV_BotInitCvars( void ) {

#if defined RTCW_SP
	Cvar_Get( "bot_enable", "1", 0 );               //enable the bot
#else
	// DHM - Nerve :: bot_enable defaults to 0
	Cvar_Get( "bot_enable", "0", 0 );               //enable the bot
#endif // RTCW_XX

	Cvar_Get( "bot_developer", "0", 0 );            //bot developer mode
	Cvar_Get( "bot_debug", "0", 0 );                //enable bot debugging
	Cvar_Get( "bot_groundonly", "1", 0 );           //only show ground faces of areas
	Cvar_Get( "bot_reachability", "0", 0 );     //show all reachabilities to other areas

#if !defined RTCW_ET
	Cvar_Get( "bot_thinktime", "100", 0 );      //msec the bots thinks
#else
	Cvar_Get( "bot_thinktime", "50", 0 );       //msec the bots thinks
#endif // RTCW_XX

	Cvar_Get( "bot_reloadcharacters", "0", 0 ); //reload the bot characters each time
	Cvar_Get( "bot_testichat", "0", 0 );            //test ichats
	Cvar_Get( "bot_testrchat", "0", 0 );            //test rchats
	Cvar_Get( "bot_fastchat", "0", 0 );         //fast chatting bots

#if !defined RTCW_ET
	Cvar_Get( "bot_nochat", "0", 0 );               //disable chats
#else
	Cvar_Get( "bot_nochat", "1", 0 );               //disable chats
#endif // RTCW_XX

	Cvar_Get( "bot_grapple", "0", 0 );          //enable grapple

#if !defined RTCW_ET
	Cvar_Get( "bot_rocketjump", "1", 0 );           //enable rocket jumping
#else
	Cvar_Get( "bot_rocketjump", "0", 0 );           //enable rocket jumping
#endif // RTCW_XX

#if !defined RTCW_ET
	Cvar_Get( "bot_miniplayers", "0", 0 );      //minimum players in a team or the game
#endif // RTCW_XX

#if defined RTCW_ET
	Cvar_Get( "bot_norcd", "0", 0 );                //enable creation of RCD file

	bot_enable = Cvar_VariableIntegerValue( "bot_enable" );
#endif // RTCW_XX

}

#if !defined RTCW_ET
// Ridah, Cast AI
/*
===============
BotImport_AICast_VisibleFromPos
===============
*/
qboolean BotImport_AICast_VisibleFromPos(   vec3_t srcpos, int srcnum,
											vec3_t destpos, int destnum, qboolean updateVisPos ) {
	return VM_Call(
		gvm,
		AICAST_VISIBLEFROMPOS,
		rtcw::to_vm_arg(srcpos),
		rtcw::to_vm_arg(srcnum),
		rtcw::to_vm_arg(destpos),
		rtcw::to_vm_arg(destnum),
		rtcw::to_vm_arg(updateVisPos)
	);
}

/*
===============
BotImport_AICast_CheckAttackAtPos
===============
*/
qboolean BotImport_AICast_CheckAttackAtPos( int entnum, int enemy, vec3_t pos, qboolean ducking, qboolean allowHitWorld ) {
	return VM_Call(
		gvm,
		AICAST_CHECKATTACKATPOS,
		rtcw::to_vm_arg(entnum),
		rtcw::to_vm_arg(enemy),
		rtcw::to_vm_arg(pos),
		rtcw::to_vm_arg(ducking),
		rtcw::to_vm_arg(allowHitWorld)
	);
}
// done.
#endif // RTCW_XX

#if defined RTCW_ET
#ifndef DEDICATED
void BotImport_DrawPolygon( int color, int numpoints, float* points );
#else
/*
==================
BotImport_DrawPolygon
==================
*/
void BotImport_DrawPolygon( int color, int numpoints, float* points ) {
	Com_DPrintf( "BotImport_DrawPolygon stub\n" );
}
#endif
#endif // RTCW_XX

/*
==================
SV_BotInitBotLib
==================
*/
void SV_BotInitBotLib( void ) {
	botlib_import_t botlib_import;

#if COPY_PROTECT
	if ( !Cvar_VariableValue( "fs_restrict" ) && !Sys_CheckCD() ) {
		Com_Error( ERR_NEED_CD, "Game CD not in drive" );
	}
#else
	Com_Printf( "Bypassing CD checks\n" );
#endif

#if defined RTCW_SP
	/*
	if ( botlib_export ) {
		SV_BotLibShutdown();
	}*/
#endif // RTCW_XX

	botlib_import.Print = BotImport_Print;
	botlib_import.Trace = BotImport_Trace;
	botlib_import.EntityTrace = BotImport_EntityTrace;
	botlib_import.PointContents = BotImport_PointContents;
	botlib_import.inPVS = BotImport_inPVS;
	botlib_import.BSPEntityData = BotImport_BSPEntityData;
	botlib_import.BSPModelMinsMaxsOrigin = BotImport_BSPModelMinsMaxsOrigin;
	botlib_import.BotClientCommand = BotClientCommand;

	//memory management
	botlib_import.GetMemory = BotImport_GetMemory;
	botlib_import.FreeMemory = BotImport_FreeMemory;

#if !defined RTCW_SP
	botlib_import.FreeZoneMemory = BotImport_FreeZoneMemory;
#endif // RTCW_XX

	botlib_import.HunkAlloc = BotImport_HunkAlloc;

	// file system acess
	botlib_import.FS_FOpenFile = FS_FOpenFileByMode;
	botlib_import.FS_Read = FS_Read;
	botlib_import.FS_Write = FS_Write;
	botlib_import.FS_FCloseFile = FS_FCloseFile;
	botlib_import.FS_Seek = FS_Seek;

	//debug lines
	botlib_import.DebugLineCreate = BotImport_DebugLineCreate;
	botlib_import.DebugLineDelete = BotImport_DebugLineDelete;
	botlib_import.DebugLineShow = BotImport_DebugLineShow;

	//debug polygons
	botlib_import.DebugPolygonCreate = BotImport_DebugPolygonCreate;

#if defined RTCW_ET
	botlib_import.DebugPolygonGetFree =         BotImport_GetFreeDebugPolygon;
#endif // RTCW_XX

	botlib_import.DebugPolygonDelete = BotImport_DebugPolygonDelete;

#if defined RTCW_ET
	botlib_import.DebugPolygonDeletePointer =   BotImport_DebugPolygonDeletePointer;
#endif // RTCW_XX

#if !defined RTCW_ET
	// Ridah, Cast AI
	botlib_import.AICast_VisibleFromPos = BotImport_AICast_VisibleFromPos;
	botlib_import.AICast_CheckAttackAtPos = BotImport_AICast_CheckAttackAtPos;
	// done.
#endif // RTCW_XX

#if defined RTCW_ET
	//bot routines
	botlib_import.BotVisibleFromPos =   BotImport_BotVisibleFromPos;
	botlib_import.BotCheckAttackAtPos = BotImport_BotCheckAttackAtPos;

	botlib_import.BotDrawPolygon =      BotImport_DrawPolygon;

	// singleplayer check
	// Arnout: no need for this
	//botlib_import.BotGameIsSinglePlayer = SV_GameIsSinglePlayer;
#endif // RTCW_XX

	botlib_export = (botlib_export_t *)GetBotLibAPI( BOTLIB_API_VERSION, &botlib_import );
}


//
//  * * * BOT AI CODE IS BELOW THIS POINT * * *
//

/*
==================
SV_BotGetConsoleMessage
==================
*/
int SV_BotGetConsoleMessage( int client, char *buf, int size ) {
	client_t    *cl;
	int index;

#if defined RTCW_SP
	const char        *msg;
#endif // RTCW_XX

	cl = &svs.clients[client];
	cl->lastPacketTime = svs.time;

	if ( cl->reliableAcknowledge == cl->reliableSequence ) {
		return qfalse;
	}

	cl->reliableAcknowledge++;
	index = cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 );

#if defined RTCW_SP
	//if ( !cl->reliableCommands[index][0] ) {
	if ( !( msg = SV_GetReliableCommand( cl, index ) ) || !msg[0] ) {
#else
	if ( !cl->reliableCommands[index][0] ) {
#endif // RTCW_XX

		return qfalse;
	}

#if !defined RTCW_MP
	//Q_strncpyz( buf, cl->reliableCommands[index], size );

#if !defined RTCW_ET
	Q_strncpyz( buf, msg, size );
#endif // RTCW_XX

#else
	Q_strncpyz( buf, cl->reliableCommands[index], size );
#endif // RTCW_XX

	return qtrue;
}

#if 0
/*
==================
EntityInPVS
==================
*/
int EntityInPVS( int client, int entityNum ) {
	client_t            *cl;
	clientSnapshot_t    *frame;
	int i;

	cl = &svs.clients[client];
	frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
	for ( i = 0; i < frame->num_entities; i++ ) {
		if ( svs.snapshotEntities[( frame->first_entity + i ) % svs.numSnapshotEntities].number == entityNum ) {
			return qtrue;
		}
	}
	return qfalse;
}
#endif

/*
==================
SV_BotGetSnapshotEntity
==================
*/
int SV_BotGetSnapshotEntity( int client, int sequence ) {
	client_t            *cl;
	clientSnapshot_t    *frame;

	cl = &svs.clients[client];
	frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
	if ( sequence < 0 || sequence >= frame->num_entities ) {
		return -1;
	}
	return svs.snapshotEntities[( frame->first_entity + sequence ) % svs.numSnapshotEntities].number;
}

