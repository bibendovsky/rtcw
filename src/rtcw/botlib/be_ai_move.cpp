/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_ai_move.c
 *
 * desc:		bot movement AI
 *
 *
 *****************************************************************************/

#include "q_shared.h"
#include "l_memory.h"
#include "l_libvar.h"
#include "l_utils.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_interface.h"

#include "be_ea.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"


//#define DEBUG_AI_MOVE
//#define DEBUG_ELEVATOR
//#define DEBUG_GRAPPLE
//movement state

//NOTE: the moveflags MFL_ONGROUND, MFL_TELEPORTED and MFL_WATERJUMP must be set outside the movement code
typedef struct bot_movestate_s
{
	//input vars (all set outside the movement code)
	vec3_t origin;                              //origin of the bot
	vec3_t velocity;                            //velocity of the bot
	vec3_t viewoffset;                          //view offset
	int entitynum;                              //entity number of the bot
	int client;                                 //client number of the bot
	float thinktime;                            //time the bot thinks
	int presencetype;                           //presencetype of the bot
	vec3_t viewangles;                          //view angles of the bot
	//state vars
	int areanum;                                //area the bot is in
	int lastareanum;                            //last area the bot was in
	int lastgoalareanum;                        //last goal area number
	int lastreachnum;                           //last reachability number
	vec3_t lastorigin;                          //origin previous cycle
	float lasttime;
	int reachareanum;                           //area number of the reachabilty
	int moveflags;                              //movement flags
	int jumpreach;                              //set when jumped
	float grapplevisible_time;                  //last time the grapple was visible
	float lastgrappledist;                      //last distance to the grapple end
	float reachability_time;                    //time to use current reachability
	int avoidreach[MAX_AVOIDREACH];             //reachabilities to avoid
	float avoidreachtimes[MAX_AVOIDREACH];      //times to avoid the reachabilities
	int avoidreachtries[MAX_AVOIDREACH];        //number of tries before avoiding
} bot_movestate_t;

//used to avoid reachability links for some time after being used

#if !defined RTCW_ET
// Ridah, disabled this to prevent wierd navigational behaviour (mostly by Zombie, since it's so slow)
//#define AVOIDREACH
#define AVOIDREACH_TIME         6       //avoid links for 6 seconds after use
#else
#define AVOIDREACH
#define AVOIDREACH_TIME         4       //avoid links for 6 seconds after use
#endif // RTCW_XX

#define AVOIDREACH_TRIES        4
//prediction times
#define PREDICTIONTIME_JUMP 3       //in seconds
#define PREDICTIONTIME_MOVE 2       //in seconds
//hook commands
#define CMD_HOOKOFF             "hookoff"
#define CMD_HOOKON              "hookon"
//weapon indexes for weapon jumping
#define WEAPONINDEX_ROCKET_LAUNCHER     5
#define WEAPONINDEX_BFG                 9

#define MODELTYPE_FUNC_PLAT     1
#define MODELTYPE_FUNC_BOB      2

float sv_maxstep;
float sv_maxbarrier;
float sv_gravity;
//type of model, func_plat or func_bobbing
int modeltypes[MAX_MODELS];

bot_movestate_t *botmovestates[MAX_CLIENTS + 1];

//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
int BotAllocMoveState( void ) {
	int i;

	for ( i = 1; i <= MAX_CLIENTS; i++ )
	{
		if ( !botmovestates[i] ) {
			botmovestates[i] = static_cast<bot_movestate_t*> (GetClearedMemory( sizeof( bot_movestate_t ) ));
			return i;
		} //end if
	} //end for
	return 0;
} //end of the function BotAllocMoveState
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void BotFreeMoveState( int handle ) {
	if ( handle <= 0 || handle > MAX_CLIENTS ) {
		botimport.Print( PRT_FATAL, "move state handle %d out of range\n", handle );
		return;
	} //end if
	if ( !botmovestates[handle] ) {
		botimport.Print( PRT_FATAL, "invalid move state %d\n", handle );
		return;
	} //end if
	FreeMemory( botmovestates[handle] );
	botmovestates[handle] = NULL;
} //end of the function BotFreeMoveState
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
bot_movestate_t *BotMoveStateFromHandle( int handle ) {
	if ( handle <= 0 || handle > MAX_CLIENTS ) {
		botimport.Print( PRT_FATAL, "move state handle %d out of range\n", handle );
		return NULL;
	} //end if
	if ( !botmovestates[handle] ) {
		botimport.Print( PRT_FATAL, "invalid move state %d\n", handle );
		return NULL;
	} //end if
	return botmovestates[handle];
} //end of the function BotMoveStateFromHandle

// Ridah, provide a means of resetting the avoidreach, so if a bot stops moving, they don't avoid the area they were heading for
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void BotInitAvoidReach( int handle ) {
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle( handle );
	if ( !ms ) {
		return;
	}

	memset( ms->avoidreach, 0, sizeof( ms->avoidreach ) );
	memset( ms->avoidreachtries, 0, sizeof( ms->avoidreachtries ) );
	memset( ms->avoidreachtimes, 0, sizeof( ms->avoidreachtimes ) );
}
// done.

//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void BotInitMoveState( int handle, bot_initmove_t *initmove ) {
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle( handle );
	if ( !ms ) {
		return;
	}
	VectorCopy( initmove->origin, ms->origin );
	VectorCopy( initmove->velocity, ms->velocity );
	VectorCopy( initmove->viewoffset, ms->viewoffset );
	ms->entitynum = initmove->entitynum;
	ms->client = initmove->client;
	ms->thinktime = initmove->thinktime;
	ms->presencetype = initmove->presencetype;

#if defined RTCW_ET
	ms->areanum = initmove->areanum;
#endif // RTCW_XX

	VectorCopy( initmove->viewangles, ms->viewangles );
	//
	ms->moveflags &= ~MFL_ONGROUND;
	if ( initmove->or_moveflags & MFL_ONGROUND ) {
		ms->moveflags |= MFL_ONGROUND;
	}
	ms->moveflags &= ~MFL_TELEPORTED;
	if ( initmove->or_moveflags & MFL_TELEPORTED ) {
		ms->moveflags |= MFL_TELEPORTED;
	}
	ms->moveflags &= ~MFL_WATERJUMP;
	if ( initmove->or_moveflags & MFL_WATERJUMP ) {
		ms->moveflags |= MFL_WATERJUMP;
	}
	ms->moveflags &= ~MFL_WALK;
	if ( initmove->or_moveflags & MFL_WALK ) {
		ms->moveflags |= MFL_WALK;
	}
} //end of the function BotInitMoveState
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
float AngleDiff( float ang1, float ang2 ) {
	float diff;

	diff = ang1 - ang2;
	if ( ang1 > ang2 ) {
		if ( diff > 180.0 ) {
			diff -= 360.0;
		}
	} //end if
	else
	{
		if ( diff < -180.0 ) {
			diff += 360.0;
		}
	} //end else
	return diff;
} //end of the function AngleDiff

#if defined RTCW_ET
/*
==================
BotFirstReachabilityArea
==================
*/
int BotFirstReachabilityArea( vec3_t origin, int *areas, int numareas, qboolean distCheck ) {
	int i, best = 0;
	vec3_t center;
	float bestDist, dist;
	bsp_trace_t tr;
	//
	bestDist = 999999;
	for ( i = 0; i < numareas; i++ ) {
		if ( AAS_AreaReachability( areas[i] ) ) {
			// make sure this area is visible
			if ( !AAS_AreaWaypoint( areas[i], center ) ) {
				AAS_AreaCenter( areas[i], center );
			}
			if ( distCheck ) {
				dist = VectorDistance( center, origin );
				if ( center[2] > origin[2] ) {
					dist += 32 * ( center[2] - origin[2] );
				}
				if ( dist < bestDist ) {
					tr = AAS_Trace( origin, NULL, NULL, center, -1, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
					if ( tr.fraction == 1.0 && !tr.startsolid && !tr.allsolid ) {
						best = areas[i];
						bestDist = dist;
						//if (dist < 128) {
						//	return best;
						//}
					}
				}
			} else {
				tr = AAS_Trace( origin, NULL, NULL, center, -1, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
				if ( tr.fraction == 1.0 && !tr.startsolid && !tr.allsolid ) {
					best = areas[i];
					break;
				}
			}
		}
	}
	//
	return best;
}
#endif // RTCW_XX

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotFuzzyPointReachabilityArea( vec3_t origin ) {

#if !defined RTCW_ET
	int firstareanum, j, x, y, z;
	int areas[10], numareas, areanum, bestareanum;
	float dist, bestdist;
	vec3_t points[10], v, end;

	firstareanum = 0;
	areanum = AAS_PointAreaNum( origin );
	if ( areanum ) {
		firstareanum = areanum;
		if ( AAS_AreaReachability( areanum ) ) {
			return areanum;
		}
	} //end if
	VectorCopy( origin, end );
	end[2] += 4;
	numareas = AAS_TraceAreas( origin, end, areas, points, 10 );
	for ( j = 0; j < numareas; j++ )
	{
		if ( AAS_AreaReachability( areas[j] ) ) {
			return areas[j];
		}
	} //end for
	bestdist = 999999;
	bestareanum = 0;

#if defined RTCW_SP
	//z = 0;
#endif // RTCW_XX

	for ( z = 1; z >= -1; z -= 1 )
	{

#if defined RTCW_SP
		for ( x = 1; x >= -1; x -= 2 )
		{
			for ( y = 1; y >= -1; y -= 2 )
#elif defined RTCW_MP
		for ( x = 1; x >= -1; x -= 1 )
		{
			for ( y = 1; y >= -1; y -= 1 )
#endif // RTCW_XX

			{
				VectorCopy( origin, end );
				// Ridah, increased this for Wolf larger bounding boxes

#if defined RTCW_SP
				end[0] += x * 256; //8;
				end[1] += y * 256; //8;
				end[2] += z * 200; //12;
#elif defined RTCW_MP
				end[0] += x * 16; //8;
				end[1] += y * 16; //8;
				end[2] += z * 24; //12;
#endif // RTCW_XX

				numareas = AAS_TraceAreas( origin, end, areas, points, 10 );
				for ( j = 0; j < numareas; j++ )
				{
					if ( AAS_AreaReachability( areas[j] ) ) {
						VectorSubtract( points[j], origin, v );
						dist = VectorLength( v );
						if ( dist < bestdist ) {
							bestareanum = areas[j];
							bestdist = dist;
						} //end if
					} //end if
					if ( !firstareanum ) {
						firstareanum = areas[j];
					}
				} //end for
			} //end for
		} //end for
		if ( bestareanum ) {
			return bestareanum;
		}
	} //end for
	return firstareanum;
#else
	int areanum, numareas, areas[100], bestarea = 0; //, i;
	vec3_t end, start /*, ofs*/, mins, maxs;
	//float f;
	#define BOTAREA_JIGGLE_DIST     256 // 32	// OUCH, hack for MP_BEACH which has lots of voids (mostly in water)

	areanum = AAS_PointAreaNum( origin );
	if ( !AAS_AreaReachability( areanum ) ) {
		areanum = 0;
	}
	if ( areanum ) {
		return areanum;
	}

	// try a line trace from beneath to above
	VectorCopy( origin, start );
	VectorCopy( origin, end );
	start[2] -= 30;
	end[2] += 40;
	numareas = AAS_TraceAreas( start, end, areas, NULL, 100 );
	if ( numareas > 0 ) {
		bestarea = BotFirstReachabilityArea( origin, areas, numareas, qfalse );
	}
	if ( bestarea ) {
		return bestarea;
	}

	// try a small box around the origin
	maxs[0] = 4;
	maxs[1] = 4;
	maxs[2] = 4;
	VectorSubtract( origin, maxs, mins );
	VectorAdd( origin, maxs, maxs );
	numareas = AAS_BBoxAreas( mins, maxs, areas, 100 );
	if ( numareas > 0 ) {
		bestarea = BotFirstReachabilityArea( origin, areas, numareas, qtrue );
	}
	if ( bestarea ) {
		return bestarea;
	}

	AAS_PresenceTypeBoundingBox( PRESENCE_NORMAL, mins, maxs );
	VectorAdd( mins, origin, mins );
	VectorAdd( maxs, origin, maxs );
	numareas = AAS_BBoxAreas( mins, maxs, areas, 100 );
	if ( numareas > 0 ) {
		bestarea = BotFirstReachabilityArea( origin, areas, numareas, qtrue );
	}
	if ( bestarea ) {
		return bestarea;
	}
/*
	// try half size first
	for (f=0.1; f<=1.0; f+=0.45) {
		VectorCopy( origin, end );
		end[2]+=80;
		VectorCopy( origin, ofs );
		ofs[2]-=60;
		for (i=0;i<2;i++) end[i]+=BOTAREA_JIGGLE_DIST*f;
		for (i=0;i<2;i++) ofs[i]-=BOTAREA_JIGGLE_DIST*f;

		numareas = AAS_BBoxAreas(ofs, end, areas, 100);
		if (numareas > 0) bestarea = BotFirstReachabilityArea(origin, areas, numareas);
		if (bestarea) return bestarea;
	}
*/
	return 0;
/*
	int firstareanum, j, x, y, z;
	int areas[10], numareas, areanum, bestareanum;
	float dist, bestdist;
	vec3_t points[10], v, end;

	firstareanum = 0;
	areanum = AAS_PointAreaNum(origin);
	if (areanum)
	{
		firstareanum = areanum;
		if (AAS_AreaReachability(areanum)) return areanum;
	} //end if
	VectorCopy(origin, end);
	end[2] += 4;
	numareas = AAS_TraceAreas(origin, end, areas, points, 10);
	for (j = 0; j < numareas; j++)
	{
		if (AAS_AreaReachability(areas[j])) return areas[j];
	} //end for
	bestdist = 999999;
	bestareanum = 0;
	for (z = 1; z >= -1; z -= 1)
	{
		for (x = 1; x >= -1; x -= 1)
		{
			for (y = 1; y >= -1; y -= 1)
			{
				VectorCopy(origin, end);
				// Ridah, increased this for Wolf larger bounding boxes
				end[0] += x * 16;//8;
				end[1] += y * 16;//8;
				end[2] += z * 24;//12;
				numareas = AAS_TraceAreas(origin, end, areas, points, 10);
				for (j = 0; j < numareas; j++)
				{
					if (AAS_AreaReachability(areas[j]))
					{
						VectorSubtract(points[j], origin, v);
						dist = VectorLength(v);
						if (dist < bestdist)
						{
							bestareanum = areas[j];
							bestdist = dist;
						} //end if
					} //end if
					if (!firstareanum) firstareanum = areas[j];
				} //end for
			} //end for
		} //end for
		if (bestareanum) return bestareanum;
	} //end for
	return firstareanum;
*/
#endif // RTCW_XX

} //end of the function BotFuzzyPointReachabilityArea
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotReachabilityArea( vec3_t origin, int client ) {
	int modelnum, modeltype, reachnum, areanum;
	aas_reachability_t reach;
	vec3_t org, end, mins, maxs, up = {0, 0, 1};
	bsp_trace_t bsptrace;
	aas_trace_t trace;

	//check if the bot is standing on something
	AAS_PresenceTypeBoundingBox( PRESENCE_CROUCH, mins, maxs );
	VectorMA( origin, -3, up, end );
	bsptrace = AAS_Trace( origin, mins, maxs, end, client, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
	if ( !bsptrace.startsolid && bsptrace.fraction < 1 && bsptrace.ent != ENTITYNUM_NONE ) {
		//if standing on the world the bot should be in a valid area
		if ( bsptrace.ent == ENTITYNUM_WORLD ) {
			return BotFuzzyPointReachabilityArea( origin );
		} //end if

		modelnum = AAS_EntityModelindex( bsptrace.ent );
		modeltype = modeltypes[modelnum];

		//if standing on a func_plat or func_bobbing then the bot is assumed to be
		//in the area the reachability points to
		if ( modeltype == MODELTYPE_FUNC_PLAT || modeltype == MODELTYPE_FUNC_BOB ) {
			reachnum = AAS_NextModelReachability( 0, modelnum );
			if ( reachnum ) {
				AAS_ReachabilityFromNum( reachnum, &reach );
				return reach.areanum;
			} //end if
		} //end else if

		//if the bot is swimming the bot should be in a valid area
		if ( AAS_Swimming( origin ) ) {
			return BotFuzzyPointReachabilityArea( origin );
		} //end if
		  //
		areanum = BotFuzzyPointReachabilityArea( origin );
		//if the bot is in an area with reachabilities
		if ( areanum && AAS_AreaReachability( areanum ) ) {
			return areanum;
		}
		//trace down till the ground is hit because the bot is standing on some other entity
		VectorCopy( origin, org );
		VectorCopy( org, end );
		end[2] -= 800;
		trace = AAS_TraceClientBBox( org, end, PRESENCE_CROUCH, -1 );
		if ( !trace.startsolid ) {
			VectorCopy( trace.endpos, org );
		} //end if
		  //
		return BotFuzzyPointReachabilityArea( org );
	} //end if
	  //
	return BotFuzzyPointReachabilityArea( origin );
} //end of the function BotReachabilityArea
//===========================================================================
// returns the reachability area the bot is in
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
int BotReachabilityArea(vec3_t origin, int testground)
{
	int firstareanum, i, j, x, y, z;
	int areas[10], numareas, areanum, bestareanum;
	float dist, bestdist;
	vec3_t org, end, points[10], v;
	aas_trace_t trace;

	firstareanum = 0;
	for (i = 0; i < 2; i++)
	{
		VectorCopy(origin, org);
		//if test at the ground (used when bot is standing on an entity)
		if (i > 0)
		{
			VectorCopy(origin, end);
			end[2] -= 800;
			trace = AAS_TraceClientBBox(origin, end, PRESENCE_CROUCH, -1);
			if (!trace.startsolid)
			{
				VectorCopy(trace.endpos, org);
			} //end if
		} //end if

		firstareanum = 0;
		areanum = AAS_PointAreaNum(org);
		if (areanum)
		{
			firstareanum = areanum;
			if (AAS_AreaReachability(areanum)) return areanum;
		} //end if
		bestdist = 999999;
		bestareanum = 0;
		for (z = 1; z >= -1; z -= 1)
		{
			for (x = 1; x >= -1; x -= 1)
			{
				for (y = 1; y >= -1; y -= 1)
				{
					VectorCopy(org, end);
					end[0] += x * 8;
					end[1] += y * 8;
					end[2] += z * 12;
					numareas = AAS_TraceAreas(org, end, areas, points, 10);
					for (j = 0; j < numareas; j++)
					{
						if (AAS_AreaReachability(areas[j]))
						{
							VectorSubtract(points[j], org, v);
							dist = VectorLength(v);
							if (dist < bestdist)
							{
								bestareanum = areas[j];
								bestdist = dist;
							} //end if
						} //end if
					} //end for
				} //end for
			} //end for
			if (bestareanum) return bestareanum;
		} //end for
		if (!testground) break;
	} //end for
//#ifdef DEBUG
	//botimport.Print(PRT_MESSAGE, "no reachability area\n");
//#endif //DEBUG
	return firstareanum;
} //end of the function BotReachabilityArea*/
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotOnMover( vec3_t origin, int entnum, aas_reachability_t *reach ) {
	int i, modelnum;
	vec3_t mins, maxs, modelorigin, org, end;
	vec3_t angles = {0, 0, 0};
	vec3_t boxmins = {-16, -16, -8}, boxmaxs = {16, 16, 8};
	bsp_trace_t trace;

	modelnum = reach->facenum & 0x0000FFFF;
	//get some bsp model info
	AAS_BSPModelMinsMaxsOrigin( modelnum, angles, mins, maxs, NULL );
	//
	if ( !AAS_OriginOfEntityWithModelNum( modelnum, modelorigin ) ) {
		botimport.Print( PRT_MESSAGE, "no entity with model %d\n", modelnum );
		return qfalse;
	} //end if
	  //
	for ( i = 0; i < 2; i++ )
	{
		if ( origin[i] > modelorigin[i] + maxs[i] + 16 ) {
			return qfalse;
		}
		if ( origin[i] < modelorigin[i] + mins[i] - 16 ) {
			return qfalse;
		}
	} //end for
	  //
	VectorCopy( origin, org );
	org[2] += 24;
	VectorCopy( origin, end );
	end[2] -= 48;
	//
	trace = AAS_Trace( org, boxmins, boxmaxs, end, entnum, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
	if ( !trace.startsolid && !trace.allsolid ) {
		//NOTE: the reachability face number is the model number of the elevator
		if ( trace.ent != ENTITYNUM_NONE && AAS_EntityModelNum( trace.ent ) == modelnum ) {
			return qtrue;
		} //end if
	} //end if
	return qfalse;
} //end of the function BotOnMover
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int MoverDown( aas_reachability_t *reach ) {
	int modelnum;
	vec3_t mins, maxs, origin;
	vec3_t angles = {0, 0, 0};

	modelnum = reach->facenum & 0x0000FFFF;
	//get some bsp model info
	AAS_BSPModelMinsMaxsOrigin( modelnum, angles, mins, maxs, origin );
	//
	if ( !AAS_OriginOfEntityWithModelNum( modelnum, origin ) ) {
		botimport.Print( PRT_MESSAGE, "no entity with model %d\n", modelnum );
		return qfalse;
	} //end if
	  //if the top of the plat is below the reachability start point
	if ( origin[2] + maxs[2] < reach->start[2] ) {
		return qtrue;
	}
	return qfalse;
} //end of the function MoverDown
//========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//========================================================================
void BotSetBrushModelTypes( void ) {
	int ent, modelnum;
	char classname[MAX_EPAIRKEY], model[MAX_EPAIRKEY];

	memset( modeltypes, 0, MAX_MODELS * sizeof( int ) );
	//
	for ( ent = AAS_NextBSPEntity( 0 ); ent; ent = AAS_NextBSPEntity( ent ) )
	{
		if ( !AAS_ValueForBSPEpairKey( ent, "classname", classname, MAX_EPAIRKEY ) ) {
			continue;
		}
		if ( !AAS_ValueForBSPEpairKey( ent, "model", model, MAX_EPAIRKEY ) ) {
			continue;
		}
		if ( model[0] ) {
			modelnum = atoi( model + 1 );
		} else { modelnum = 0;}

		if ( modelnum < 0 || modelnum > MAX_MODELS ) {
			botimport.Print( PRT_MESSAGE, "entity %s model number out of range\n", classname );
			continue;
		} //end if

		if ( !strcmp( classname, "func_bobbing" ) ) {
			modeltypes[modelnum] = MODELTYPE_FUNC_BOB;
		} else if ( !strcmp( classname, "func_plat" ) ) {
			modeltypes[modelnum] = MODELTYPE_FUNC_PLAT;
		}
	} //end for
} //end of the function BotSetBrushModelTypes
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotOnTopOfEntity( bot_movestate_t *ms ) {
	vec3_t mins, maxs, end, up = {0, 0, 1};
	bsp_trace_t trace;

	AAS_PresenceTypeBoundingBox( ms->presencetype, mins, maxs );
	VectorMA( ms->origin, -3, up, end );
	trace = AAS_Trace( ms->origin, mins, maxs, end, ms->entitynum, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
	if ( !trace.startsolid && ( trace.ent != ENTITYNUM_WORLD && trace.ent != ENTITYNUM_NONE ) ) {
		return trace.ent;
	} //end if
	return -1;
} //end of the function BotOnTopOfEntity
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotValidTravel( vec3_t origin, aas_reachability_t *reach, int travelflags ) {
	//if the reachability uses an unwanted travel type
	if ( AAS_TravelFlagForType( reach->traveltype ) & ~travelflags ) {
		return qfalse;
	}
	//don't go into areas with bad travel types
	if ( AAS_AreaContentsTravelFlag( reach->areanum ) & ~travelflags ) {
		return qfalse;
	}
	return qtrue;
} //end of the function BotValidTravel
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotAddToAvoidReach( bot_movestate_t *ms, int number, float avoidtime ) {
	int i;

	for ( i = 0; i < MAX_AVOIDREACH; i++ )
	{
		if ( ms->avoidreach[i] == number ) {
			if ( ms->avoidreachtimes[i] > AAS_Time() ) {
				ms->avoidreachtries[i]++;
			} else { ms->avoidreachtries[i] = 1;}
			ms->avoidreachtimes[i] = AAS_Time() + avoidtime;
			return;
		} //end if
	} //end for
	  //add the reachability to the reachabilities to avoid for a while
	for ( i = 0; i < MAX_AVOIDREACH; i++ )
	{
		if ( ms->avoidreachtimes[i] < AAS_Time() ) {
			ms->avoidreach[i] = number;
			ms->avoidreachtimes[i] = AAS_Time() + avoidtime;
			ms->avoidreachtries[i] = 1;
			return;
		} //end if
	} //end for
} //end of the function BotAddToAvoidReach
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
//__inline int AAS_AreaContentsTravelFlag(int areanum);

int BotGetReachabilityToGoal( vec3_t origin, int areanum, int entnum,
							  int lastgoalareanum, int lastareanum,
							  int *avoidreach, float *avoidreachtimes, int *avoidreachtries,
							  bot_goal_t *goal, int travelflags, int movetravelflags ) {
	int t, besttime, bestreachnum, reachnum;
	aas_reachability_t reach;

#if defined RTCW_ET
	qboolean useAvoidPass = qfalse;

again:
#endif // RTCW_XX


	//if not in a valid area
	if ( !areanum ) {
		return 0;
	}
	//
	if ( AAS_AreaDoNotEnter( areanum ) || AAS_AreaDoNotEnter( goal->areanum ) ) {
		travelflags |= TFL_DONOTENTER;
		movetravelflags |= TFL_DONOTENTER;
	} //end if
	if ( AAS_AreaDoNotEnterLarge( areanum ) || AAS_AreaDoNotEnterLarge( goal->areanum ) ) {
		travelflags |= TFL_DONOTENTER_LARGE;
		movetravelflags |= TFL_DONOTENTER_LARGE;
	} //end if
	  //use the routing to find the next area to go to
	besttime = 0;
	bestreachnum = 0;
	//
	for ( reachnum = AAS_NextAreaReachability( areanum, 0 ); reachnum;
		  reachnum = AAS_NextAreaReachability( areanum, reachnum ) )
	{
#ifdef AVOIDREACH
		int i;
		//check if it isn't an reachability to avoid
		for ( i = 0; i < MAX_AVOIDREACH; i++ )
		{
			if ( avoidreach[i] == reachnum && avoidreachtimes[i] >= AAS_Time() ) {
				break;
			}
		} //end for

#if !defined RTCW_ET
		if ( i != MAX_AVOIDREACH && avoidreachtries[i] > AVOIDREACH_TRIES ) {
#else
		  // RF, if this is a "useAvoidPass" then we should only accept avoidreach reachabilities
		if ( ( !useAvoidPass && i != MAX_AVOIDREACH && avoidreachtries[i] > AVOIDREACH_TRIES )
			 || ( useAvoidPass && !( i != MAX_AVOIDREACH && avoidreachtries[i] > AVOIDREACH_TRIES ) ) ) {
#endif // RTCW_XX

#ifdef DEBUG
			if ( bot_developer ) {
				botimport.Print( PRT_MESSAGE, "avoiding reachability %d\n", avoidreach[i] );
			} //end if
#endif //DEBUG
			continue;
		} //end if
#endif //AVOIDREACH
	   //get the reachability from the number
		AAS_ReachabilityFromNum( reachnum, &reach );
		//NOTE: do not go back to the previous area if the goal didn't change
		//NOTE: is this actually avoidance of local routing minima between two areas???
		if ( lastgoalareanum == goal->areanum && reach.areanum == lastareanum ) {
			continue;
		}
		//if (AAS_AreaContentsTravelFlag(reach.areanum) & ~travelflags) continue;
		//if the travel isn't valid
		if ( !BotValidTravel( origin, &reach, movetravelflags ) ) {
			continue;
		}

#if defined RTCW_SP
		//RF, ignore disabled areas
		if ( !AAS_AreaReachability( reach.areanum ) ) {
			continue;
		}
		//get the travel time (ignore routes that leads us back our current area)
		t = AAS_AreaTravelTimeToGoalAreaCheckLoop( reach.areanum, reach.end, goal->areanum, travelflags, areanum );
#elif defined RTCW_MP
		//get the travel time
		t = AAS_AreaTravelTimeToGoalArea( reach.areanum, reach.end, goal->areanum, travelflags );
#else
		// if the area is disabled
		if ( !AAS_AreaReachability( reach.areanum ) ) {
			continue;
		}
		//get the travel time
		t = AAS_AreaTravelTimeToGoalArea( reach.areanum, reach.end, goal->areanum, travelflags );
#endif // RTCW_XX

		//if the goal area isn't reachable from the reachable area
		if ( !t ) {
			continue;
		}

		// Ridah, if this sends us to a looped route, ignore it

#if defined RTCW_SP
		//if (AAS_AreaTravelTimeToGoalArea(areanum, reach.start, goal->areanum, travelflags) + reach.traveltime < t)
#else
//		if (AAS_AreaTravelTimeToGoalArea(areanum, reach.start, goal->areanum, travelflags) + reach.traveltime == t)
//			continue;
#endif // RTCW_XX

#if !defined RTCW_SP
		//	continue;
#endif // RTCW_XX

		//add the travel time towards the area
		// Ridah, not sure why this was disabled, but it causes looped links in the route-cache

#if defined RTCW_SP
		// RF, update.. seems to work better like this....
		t += reach.traveltime; // + AAS_AreaTravelTime(areanum, origin, reach.start);
		//t += reach.traveltime + AAS_AreaTravelTime(areanum, origin, reach.start);
#else
		//t += reach.traveltime;// + AAS_AreaTravelTime(areanum, origin, reach.start);
		t += reach.traveltime + AAS_AreaTravelTime( areanum, origin, reach.start );
#endif // RTCW_XX

		// Ridah, if there exists other entities in this area, avoid it
//		if (reach.areanum != goal->areanum && AAS_IsEntityInArea( entnum, goal->entitynum, reach.areanum )) {
//			t += 50;
//		}

		//if the travel time is better than the ones already found
		if ( !besttime || t < besttime ) {
			besttime = t;
			bestreachnum = reachnum;
		} //end if
	} //end for
	  //

#if defined RTCW_ET
	  // RF, if we didnt find a route, then try again only looking through avoidreach reachabilities
	if ( !bestreachnum && !useAvoidPass ) {
		useAvoidPass = qtrue;
		goto again;
	}
	//
#endif // RTCW_XX

	return bestreachnum;
} //end of the function BotGetReachabilityToGoal
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int BotAddToTarget( vec3_t start, vec3_t end, float maxdist, float *dist, vec3_t target ) {
	vec3_t dir;
	float curdist;

	VectorSubtract( end, start, dir );
	curdist = VectorNormalize( dir );
	if ( *dist + curdist < maxdist ) {
		VectorCopy( end, target );
		*dist += curdist;
		return qfalse;
	} //end if
	else
	{
		VectorMA( start, maxdist - *dist, dir, target );
		*dist = maxdist;
		return qtrue;
	} //end else
} //end of the function BotAddToTarget

int BotMovementViewTarget( int movestate, bot_goal_t *goal, int travelflags, float lookahead, vec3_t target ) {
	aas_reachability_t reach;
	int reachnum, lastareanum;
	bot_movestate_t *ms;
	vec3_t end;
	float dist;

	ms = BotMoveStateFromHandle( movestate );
	if ( !ms ) {
		return qfalse;
	}
	reachnum = 0;
	//if the bot has no goal or no last reachability
	if ( !ms->lastreachnum || !goal ) {
		return qfalse;
	}

	reachnum = ms->lastreachnum;
	VectorCopy( ms->origin, end );
	lastareanum = ms->lastareanum;
	dist = 0;
	while ( reachnum && dist < lookahead )
	{
		AAS_ReachabilityFromNum( reachnum, &reach );
		if ( BotAddToTarget( end, reach.start, lookahead, &dist, target ) ) {
			return qtrue;
		}
		//never look beyond teleporters
		if ( reach.traveltype == TRAVEL_TELEPORT ) {
			return qtrue;
		}
		//don't add jump pad distances
		if ( reach.traveltype != TRAVEL_JUMPPAD &&
			 reach.traveltype != TRAVEL_ELEVATOR &&
			 reach.traveltype != TRAVEL_FUNCBOB ) {
			if ( BotAddToTarget( reach.start, reach.end, lookahead, &dist, target ) ) {
				return qtrue;
			}
		} //end if
		reachnum = BotGetReachabilityToGoal( reach.end, reach.areanum, -1,
											 ms->lastgoalareanum, lastareanum,
											 ms->avoidreach, ms->avoidreachtimes, ms->avoidreachtries,
											 goal, travelflags, travelflags );
		VectorCopy( reach.end, end );
		lastareanum = reach.areanum;
		if ( lastareanum == goal->areanum ) {
			BotAddToTarget( reach.end, goal->origin, lookahead, &dist, target );
			return qtrue;
		} //end if
	} //end while
	  //
	return qfalse;
} //end of the function BotMovementViewTarget
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotVisible( int ent, vec3_t eye, vec3_t target ) {
	bsp_trace_t trace;

	trace = AAS_Trace( eye, NULL, NULL, target, ent, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
	if ( trace.fraction >= 1 ) {
		return qtrue;
	}
	return qfalse;
} //end of the function BotVisible
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotPredictVisiblePosition( vec3_t origin, int areanum, bot_goal_t *goal, int travelflags, vec3_t target ) {
	aas_reachability_t reach;
	int reachnum, lastgoalareanum, lastareanum, i;
	int avoidreach[MAX_AVOIDREACH];
	float avoidreachtimes[MAX_AVOIDREACH];
	int avoidreachtries[MAX_AVOIDREACH];
	vec3_t end;

	//if the bot has no goal or no last reachability
	if ( !goal ) {
		return qfalse;
	}
	//if the areanum is not valid
	if ( !areanum ) {
		return qfalse;
	}
	//if the goal areanum is not valid
	if ( !goal->areanum ) {
		return qfalse;
	}

	memset( avoidreach, 0, MAX_AVOIDREACH * sizeof( int ) );
	lastgoalareanum = goal->areanum;
	lastareanum = areanum;
	VectorCopy( origin, end );
	//only do 20 hops
	for ( i = 0; i < 20 && ( areanum != goal->areanum ); i++ )
	{
		//
		reachnum = BotGetReachabilityToGoal( end, areanum, -1,
											 lastgoalareanum, lastareanum,
											 avoidreach, avoidreachtimes, avoidreachtries,
											 goal, travelflags, travelflags );
		if ( !reachnum ) {
			return qfalse;
		}
		AAS_ReachabilityFromNum( reachnum, &reach );
		//
		if ( BotVisible( goal->entitynum, goal->origin, reach.start ) ) {
			VectorCopy( reach.start, target );
			return qtrue;
		} //end if
		  //
		if ( BotVisible( goal->entitynum, goal->origin, reach.end ) ) {
			VectorCopy( reach.end, target );
			return qtrue;
		} //end if
		  //
		if ( reach.areanum == goal->areanum ) {
			VectorCopy( reach.end, target );
			return qtrue;
		} //end if
		  //
		lastareanum = areanum;
		areanum = reach.areanum;
		VectorCopy( reach.end, end );
		//
	} //end while
	  //
	return qfalse;
} //end of the function BotPredictVisiblePosition
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void MoverBottomCenter( aas_reachability_t *reach, vec3_t bottomcenter ) {
	int modelnum;
	vec3_t mins, maxs, origin, mids;
	vec3_t angles = {0, 0, 0};

	modelnum = reach->facenum & 0x0000FFFF;
	//get some bsp model info
	AAS_BSPModelMinsMaxsOrigin( modelnum, angles, mins, maxs, origin );
	//
	if ( !AAS_OriginOfEntityWithModelNum( modelnum, origin ) ) {
		botimport.Print( PRT_MESSAGE, "no entity with model %d\n", modelnum );
	} //end if
	  //get a point just above the plat in the bottom position
	VectorAdd( mins, maxs, mids );
	VectorMA( origin, 0.5, mids, bottomcenter );
	bottomcenter[2] = reach->start[2];
} //end of the function MoverBottomCenter
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
float BotGapDistance( vec3_t origin, vec3_t hordir, int entnum ) {
	float dist, startz;
	vec3_t start, end;
	aas_trace_t trace;

	//do gap checking
	startz = origin[2];
	//this enables walking down stairs more fluidly
	{
		VectorCopy( origin, start );
		VectorCopy( origin, end );
		end[2] -= 60;
		trace = AAS_TraceClientBBox( start, end, PRESENCE_CROUCH, entnum );
		if ( trace.fraction >= 1 ) {
			return 1;
		}
		startz = trace.endpos[2] + 1;
	}
	//
	for ( dist = 8; dist <= 100; dist += 8 )
	{
		VectorMA( origin, dist, hordir, start );
		start[2] = startz + 24;
		VectorCopy( start, end );
		end[2] -= 48 + sv_maxbarrier;
		trace = AAS_TraceClientBBox( start, end, PRESENCE_CROUCH, entnum );
		//if solid is found the bot can't walk any further and fall into a gap
		if ( !trace.startsolid ) {
			//if it is a gap
			if ( trace.endpos[2] < startz - sv_maxstep - 8 ) {
				VectorCopy( trace.endpos, end );
				end[2] -= 20;
				if ( AAS_PointContents( end ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) {
					break;                                                              //----(SA)	modified since slime is no longer deadly
				}
//				if (AAS_PointContents(end) & CONTENTS_WATER) break;
				//if a gap is found slow down
				//botimport.Print(PRT_MESSAGE, "gap at %f\n", dist);
				return dist;
			} //end if
			startz = trace.endpos[2];
		} //end if
	} //end for
	return 0;
} //end of the function BotGapDistance
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotCheckBarrierJump( bot_movestate_t *ms, vec3_t dir, float speed ) {
	vec3_t start, hordir, end;
	aas_trace_t trace;

	VectorCopy( ms->origin, end );
	end[2] += sv_maxbarrier;
	//trace right up
	trace = AAS_TraceClientBBox( ms->origin, end, PRESENCE_NORMAL, ms->entitynum );
	//this shouldn't happen... but we check anyway
	if ( trace.startsolid ) {
		return qfalse;
	}
	//if very low ceiling it isn't possible to jump up to a barrier
	if ( trace.endpos[2] - ms->origin[2] < sv_maxstep ) {
		return qfalse;
	}
	//
	hordir[0] = dir[0];
	hordir[1] = dir[1];
	hordir[2] = 0;
	VectorNormalize( hordir );
	VectorMA( ms->origin, ms->thinktime * speed * 0.5, hordir, end );
	VectorCopy( trace.endpos, start );
	end[2] = trace.endpos[2];
	//trace from previous trace end pos horizontally in the move direction
	trace = AAS_TraceClientBBox( start, end, PRESENCE_NORMAL, ms->entitynum );
	//again this shouldn't happen
	if ( trace.startsolid ) {
		return qfalse;
	}
	//
	VectorCopy( trace.endpos, start );
	VectorCopy( trace.endpos, end );
	end[2] = ms->origin[2];
	//trace down from the previous trace end pos
	trace = AAS_TraceClientBBox( start, end, PRESENCE_NORMAL, ms->entitynum );
	//if solid
	if ( trace.startsolid ) {
		return qfalse;
	}
	//if no obstacle at all
	if ( trace.fraction >= 1.0 ) {
		return qfalse;
	}
	//if less than the maximum step height
	if ( trace.endpos[2] - ms->origin[2] < sv_maxstep ) {
		return qfalse;
	}
	//
	EA_Jump( ms->client );
	EA_Move( ms->client, hordir, speed );
	ms->moveflags |= MFL_BARRIERJUMP;
	//there is a barrier
	return qtrue;
} //end of the function BotCheckBarrierJump
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotSwimInDirection( bot_movestate_t *ms, vec3_t dir, float speed, int type ) {
	vec3_t normdir;

	VectorCopy( dir, normdir );
	VectorNormalize( normdir );
	EA_Move( ms->client, normdir, speed );
	return qtrue;
} //end of the function BotSwimInDirection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotWalkInDirection( bot_movestate_t *ms, vec3_t dir, float speed, int type ) {
	vec3_t hordir, cmdmove, velocity, tmpdir, origin;
	int presencetype, maxframes, cmdframes, stopevent;
	aas_clientmove_t move;
	float dist;

	//if the bot is on the ground
	if ( ms->moveflags & MFL_ONGROUND ) {
		//if there is a barrier the bot can jump on
		if ( BotCheckBarrierJump( ms, dir, speed ) ) {
			return qtrue;
		}
		//remove barrier jump flag
		ms->moveflags &= ~MFL_BARRIERJUMP;
		//get the presence type for the movement
		if ( ( type & MOVE_CROUCH ) && !( type & MOVE_JUMP ) ) {
			presencetype = PRESENCE_CROUCH;
		} else { presencetype = PRESENCE_NORMAL;}
		//horizontal direction
		hordir[0] = dir[0];
		hordir[1] = dir[1];
		hordir[2] = 0;
		VectorNormalize( hordir );
		//if the bot is not supposed to jump
		if ( !( type & MOVE_JUMP ) ) {
			//if there is a gap, try to jump over it
			if ( BotGapDistance( ms->origin, hordir, ms->entitynum ) > 0 ) {
				type |= MOVE_JUMP;
			}
		} //end if
		  //get command movement
		VectorScale( hordir, speed, cmdmove );
		VectorCopy( ms->velocity, velocity );
		//
		if ( type & MOVE_JUMP ) {
			//botimport.Print(PRT_MESSAGE, "trying jump\n");
			cmdmove[2] = 400;
			maxframes = PREDICTIONTIME_JUMP / 0.1;
			cmdframes = 1;
			stopevent = SE_HITGROUND | SE_HITGROUNDDAMAGE |
						SE_ENTERWATER | SE_ENTERSLIME | SE_ENTERLAVA;
		} //end if
		else
		{
			maxframes = 2;
			cmdframes = 2;
			stopevent = SE_HITGROUNDDAMAGE |
						SE_ENTERWATER | SE_ENTERSLIME | SE_ENTERLAVA;
		} //end else
		  //AAS_ClearShownDebugLines();
		  //
		VectorCopy( ms->origin, origin );
		origin[2] += 0.5;
		AAS_PredictClientMovement( &move, ms->entitynum, origin, presencetype, qtrue,
								   velocity, cmdmove, cmdframes, maxframes, 0.1,
								   stopevent, 0, qfalse ); //qtrue);
		//if prediction time wasn't enough to fully predict the movement
		if ( move.frames >= maxframes && ( type & MOVE_JUMP ) ) {
			//botimport.Print(PRT_MESSAGE, "client %d: max prediction frames\n", ms->client);
			return qfalse;
		} //end if
		  //don't enter slime or lava and don't fall from too high
		if ( move.stopevent & ( SE_ENTERLAVA | SE_HITGROUNDDAMAGE ) ) {   //----(SA)	modified since slime is no longer deadly
//		if (move.stopevent & (SE_ENTERSLIME|SE_ENTERLAVA|SE_HITGROUNDDAMAGE))
			//botimport.Print(PRT_MESSAGE, "client %d: would be hurt ", ms->client);
			//if (move.stopevent & SE_ENTERSLIME) botimport.Print(PRT_MESSAGE, "slime\n");
			//if (move.stopevent & SE_ENTERLAVA) botimport.Print(PRT_MESSAGE, "lava\n");
			//if (move.stopevent & SE_HITGROUNDDAMAGE) botimport.Print(PRT_MESSAGE, "hitground\n");
			return qfalse;
		} //end if
		  //if ground was hit
		if ( move.stopevent & SE_HITGROUND ) {
			//check for nearby gap
			VectorNormalize2( move.velocity, tmpdir );
			dist = BotGapDistance( move.endpos, tmpdir, ms->entitynum );
			if ( dist > 0 ) {
				return qfalse;
			}
			//
			dist = BotGapDistance( move.endpos, hordir, ms->entitynum );
			if ( dist > 0 ) {
				return qfalse;
			}
		} //end if
		  //get horizontal movement
		tmpdir[0] = move.endpos[0] - ms->origin[0];
		tmpdir[1] = move.endpos[1] - ms->origin[1];
		tmpdir[2] = 0;
		//
		//AAS_DrawCross(move.endpos, 4, LINECOLOR_BLUE);
		//the bot is blocked by something
		if ( VectorLength( tmpdir ) < speed * ms->thinktime * 0.5 ) {
			return qfalse;
		}
		//perform the movement
		if ( type & MOVE_JUMP ) {
			EA_Jump( ms->client );
		}
		if ( type & MOVE_CROUCH ) {
			EA_Crouch( ms->client );
		}
		EA_Move( ms->client, hordir, speed );
		//movement was succesfull
		return qtrue;
	} //end if
	else
	{
		if ( ms->moveflags & MFL_BARRIERJUMP ) {
			//if near the top or going down
			if ( ms->velocity[2] < 50 ) {
				EA_Move( ms->client, dir, speed );
			} //end if
		} //end if
		  //FIXME: do air control to avoid hazards
		return qtrue;
	} //end else
} //end of the function BotWalkInDirection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotMoveInDirection( int movestate, vec3_t dir, float speed, int type ) {
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle( movestate );
	if ( !ms ) {
		return qfalse;
	}
	//if swimming
	if ( AAS_Swimming( ms->origin ) ) {
		return BotSwimInDirection( ms, dir, speed, type );
	} //end if
	else
	{
		return BotWalkInDirection( ms, dir, speed, type );
	} //end else
} //end of the function BotMoveInDirection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int Intersection( vec2_t p1, vec2_t p2, vec2_t p3, vec2_t p4, vec2_t out ) {
	float x1, dx1, dy1, x2, dx2, dy2, d;

	dx1 = p2[0] - p1[0];
	dy1 = p2[1] - p1[1];
	dx2 = p4[0] - p3[0];
	dy2 = p4[1] - p3[1];

	d = dy1 * dx2 - dx1 * dy2;
	if ( d != 0 ) {
		x1 = p1[1] * dx1 - p1[0] * dy1;
		x2 = p3[1] * dx2 - p3[0] * dy2;
		out[0] = (int) ( ( dx1 * x2 - dx2 * x1 ) / d );
		out[1] = (int) ( ( dy1 * x2 - dy2 * x1 ) / d );
		return qtrue;
	} //end if
	else
	{
		return qfalse;
	} //end else
} //end of the function Intersection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef RTCW_SP
void BotCheckBlocked(bot_movestate_t*, vec3_t, int, bot_moveresult_t*)
{
	// RF, not required for Wolf AI
}
#else // RTCW_SP
void BotCheckBlocked( bot_movestate_t *ms, vec3_t dir, int checkbottom, bot_moveresult_t *result ) {
	vec3_t mins, maxs, end, up = {0, 0, 1};
	bsp_trace_t trace;

	//test for entities obstructing the bot's path
	AAS_PresenceTypeBoundingBox( ms->presencetype, mins, maxs );
	//

#if !defined RTCW_ET
	if ( c::fabs( DotProduct( dir, up ) ) < 0.7 ) {
#else
	if ( Q_fabs( DotProduct( dir, up ) ) < 0.7 ) {
#endif // RTCW_XX

		mins[2] += sv_maxstep; //if the bot can step on
		maxs[2] -= 10; //a little lower to avoid low ceiling
	} //end if

#if !defined RTCW_ET
	VectorMA( ms->origin, 3, dir, end );
#else
	VectorMA( ms->origin, 16, dir, end );
#endif // RTCW_XX

	trace = AAS_Trace( ms->origin, mins, maxs, end, ms->entitynum, CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY );
	//if not started in solid and not hitting the world entity
	if ( !trace.startsolid && ( trace.ent != ENTITYNUM_WORLD && trace.ent != ENTITYNUM_NONE ) ) {
		result->blocked = qtrue;
		result->blockentity = trace.ent;
#ifdef DEBUG
		//botimport.Print(PRT_MESSAGE, "%d: BotCheckBlocked: I'm blocked\n", ms->client);
#endif //DEBUG
	} //end if
	  //if not in an area with reachability
	else if ( checkbottom && !AAS_AreaReachability( ms->areanum ) ) {
		//check if the bot is standing on something
		AAS_PresenceTypeBoundingBox( ms->presencetype, mins, maxs );
		VectorMA( ms->origin, -3, up, end );
		trace = AAS_Trace( ms->origin, mins, maxs, end, ms->entitynum, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
		if ( !trace.startsolid && ( trace.ent != ENTITYNUM_WORLD && trace.ent != ENTITYNUM_NONE ) ) {
			result->blocked = qtrue;
			result->blockentity = trace.ent;
			result->flags |= MOVERESULT_ONTOPOFOBSTACLE;
#ifdef DEBUG
			//botimport.Print(PRT_MESSAGE, "%d: BotCheckBlocked: I'm blocked\n", ms->client);
#endif //DEBUG
		} //end if
	} //end else
} //end of the function BotCheckBlocked
#endif // RTCW_SP
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotClearMoveResult( bot_moveresult_t *moveresult ) {
	moveresult->failure = qfalse;
	moveresult->type = 0;
	moveresult->blocked = qfalse;

#if !defined RTCW_ET
	moveresult->blockentity = 0;
#else
	moveresult->blockentity = -1;
#endif // RTCW_XX

	moveresult->traveltype = 0;
	moveresult->flags = 0;
} //end of the function BotClearMoveResult

#if defined RTCW_ET
char    *vtosf( const vec3_t v ) {
	static int index;
	static char str[8][64];
	char    *s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = ( index + 1 ) & 7;

	Com_sprintf( s, 64, "(%f %f %f)", v[0], v[1], v[2] );

	return s;
}
#endif // RTCW_XX

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Walk( bot_movestate_t *ms, aas_reachability_t *reach ) {
	float dist, speed;

#if !defined RTCW_ET
	vec3_t hordir;
#else
	vec3_t hordir; //, v1, v2, p;
#endif // RTCW_XX

	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//first walk straight to the reachability start
	hordir[0] = reach->start[0] - ms->origin[0];
	hordir[1] = reach->start[1] - ms->origin[1];
	hordir[2] = 0;
	dist = VectorNormalize( hordir );
	//
	BotCheckBlocked( ms, hordir, qtrue, &result );
	//
	// Ridah, tweaked this
//	if (dist < 10)
	if ( dist < 32 ) {
		//walk straight to the reachability end
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		hordir[2] = 0;
		dist = VectorNormalize( hordir );

#if defined RTCW_ET
/*
		// if we are really close to the line, then move towards the center of the reach area
		VectorCopy( reach->start, v1 );
		VectorCopy( reach->end, v2 );
		VectorCopy( ms->origin, p );
		v1[2] = 0;
		v2[2] = 0;
		p[2] = 0;
		if (DistanceFromVectorSquared( p, v1, v2 ) < 4) {
			if (!AAS_AreaWaypoint( reach->areanum, p ))
				AAS_AreaCenter( reach->areanum, p );
			if (VectorDistance( ms->origin, p ) > 32) {
			VectorSubtract( p, ms->origin, hordir );
			hordir[2] = 0;
			dist = VectorNormalize(hordir);
		}
		}
*/
	} else {
//botimport.Print(PRT_MESSAGE, "BotTravel_Walk: NOT in range of reach (%i)\n", reach->areanum);
#endif // RTCW_XX

	} //end if
	  //if going towards a crouch area

	// Ridah, some areas have a 0 presence (!?!)
//	if (!(AAS_AreaPresenceType(reach->areanum) & PRESENCE_NORMAL))
	if (     ( AAS_AreaPresenceType( reach->areanum ) & PRESENCE_CROUCH ) &&
			 !( AAS_AreaPresenceType( reach->areanum ) & PRESENCE_NORMAL ) ) {
		//if pretty close to the reachable area
		if ( dist < 20 ) {
			EA_Crouch( ms->client );
		}
	} //end if
	  //
	dist = BotGapDistance( ms->origin, hordir, ms->entitynum );
	//
	if ( ms->moveflags & MFL_WALK ) {
		if ( dist > 0 ) {
			speed = 200 - ( 180 - 1 * dist );
		} else { speed = 200;}
		EA_Walk( ms->client );
	} //end if
	else
	{
		if ( dist > 0 ) {
			speed = 400 - ( 360 - 2 * dist );
		} else { speed = 400;}
	} //end else
	  //elemantary action move in direction
	EA_Move( ms->client, hordir, speed );
	VectorCopy( hordir, result.movedir );

#if !defined RTCW_ET
	//
#else
//
//botimport.Print(PRT_MESSAGE, "\nBotTravel_Walk: srcarea %i, rcharea %i, org %s, reachorg %s\n", ms->areanum, reach->areanum, vtosf(ms->origin), vtosf(reach->start) );
	//
#endif // RTCW_XX

	return result;
} //end of the function BotTravel_Walk
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_Walk( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t hordir;
	float dist, speed;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//if not on the ground and changed areas... don't walk back!!
	//(doesn't seem to help)
	/*
	ms->areanum = BotFuzzyPointReachabilityArea(ms->origin);
	if (ms->areanum == reach->areanum)
	{
#ifdef DEBUG
		botimport.Print(PRT_MESSAGE, "BotFinishTravel_Walk: already in reach area\n");
#endif //DEBUG
		return result;
	} //end if*/
	//go straight to the reachability end
	hordir[0] = reach->end[0] - ms->origin[0];
	hordir[1] = reach->end[1] - ms->origin[1];
	hordir[2] = 0;
	dist = VectorNormalize( hordir );
	//
	if ( dist > 100 ) {
		dist = 100;
	}
	speed = 400 - ( 400 - 3 * dist );
	//
	EA_Move( ms->client, hordir, speed );
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotFinishTravel_Walk
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Crouch( bot_movestate_t *ms, aas_reachability_t *reach ) {
	float speed;
	vec3_t hordir;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//
	speed = 400;
	//walk straight to reachability end
	hordir[0] = reach->end[0] - ms->origin[0];
	hordir[1] = reach->end[1] - ms->origin[1];
	hordir[2] = 0;
	VectorNormalize( hordir );
	//
	BotCheckBlocked( ms, hordir, qtrue, &result );
	//elemantary actions
	EA_Crouch( ms->client );
	EA_Move( ms->client, hordir, speed );
	//
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotTravel_Crouch
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_BarrierJump( bot_movestate_t *ms, aas_reachability_t *reach ) {
	float dist, speed;
	vec3_t hordir;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//walk straight to reachability start
	hordir[0] = reach->start[0] - ms->origin[0];
	hordir[1] = reach->start[1] - ms->origin[1];
	hordir[2] = 0;
	dist = VectorNormalize( hordir );
	//
	BotCheckBlocked( ms, hordir, qtrue, &result );
	//if pretty close to the barrier

#if !defined RTCW_ET
	if ( dist < 9 ) {
#else
	if ( dist < 12 ) {
#endif // RTCW_XX

		EA_Jump( ms->client );

		// Ridah, do the movement also, so we have momentum to get onto the barrier
		hordir[0] = reach->end[0] - reach->start[0];
		hordir[1] = reach->end[1] - reach->start[1];
		hordir[2] = 0;
		VectorNormalize( hordir );

#if !defined RTCW_ET
		dist = 60;
		speed = 360 - ( 360 - 6 * dist );
#else
		dist = 90;
		speed = 400 - ( 360 - 4 * dist );
#endif // RTCW_XX

		EA_Move( ms->client, hordir, speed );
		// done.
	} //end if
	else
	{

#if !defined RTCW_ET
		if ( dist > 60 ) {
			dist = 60;
		}
		speed = 360 - ( 360 - 6 * dist );
#else
		if ( dist > 90 ) {
			dist = 90;
		}
		speed = 400 - ( 360 - 4 * dist );
#endif // RTCW_XX

		EA_Move( ms->client, hordir, speed );
	} //end else
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotTravel_BarrierJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_BarrierJump( bot_movestate_t *ms, aas_reachability_t *reach ) {
	float dist, speed;
	vec3_t hordir;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//if near the top or going down
	if ( ms->velocity[2] < 250 ) {
		// Ridah, extend the end point a bit, so we strive to get over the ledge more
		vec3_t end;

		VectorSubtract( reach->end, reach->start, end );
		end[2] = 0;
		VectorNormalize( end );
		VectorMA( reach->end, 32, end, end );
		hordir[0] = end[0] - ms->origin[0];
		hordir[1] = end[1] - ms->origin[1];
//		hordir[0] = reach->end[0] - ms->origin[0];
//		hordir[1] = reach->end[1] - ms->origin[1];
		// done.
		hordir[2] = 0;
		dist = VectorNormalize( hordir );
		//
		BotCheckBlocked( ms, hordir, qtrue, &result );
		//
		if ( dist > 60 ) {
			dist = 60;
		}
		speed = 400 - ( 400 - 6 * dist );
		EA_Move( ms->client, hordir, speed );
		VectorCopy( hordir, result.movedir );

#if !defined RTCW_ET
	} //end if
	  //
#else
	} else {
		// hold crouch in case we are going for a crouch area
		if ( AAS_AreaPresenceType( reach->areanum ) & PRESENCE_CROUCH ) {
			EA_Crouch( ms->client );
		}
	}
	//
#endif // RTCW_XX

	return result;
} //end of the function BotFinishTravel_BarrierJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Swim( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t dir;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//swim straight to reachability end
	VectorSubtract( reach->start, ms->origin, dir );
	VectorNormalize( dir );
	//
	BotCheckBlocked( ms, dir, qtrue, &result );
	//elemantary actions
	EA_Move( ms->client, dir, 400 );
	//
	VectorCopy( dir, result.movedir );
	Vector2Angles( dir, result.ideal_viewangles );
	result.flags |= MOVERESULT_SWIMVIEW;
	//
	return result;
} //end of the function BotTravel_Swim
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_WaterJump( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t dir, hordir;
	float dist;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//swim straight to reachability end
	VectorSubtract( reach->end, ms->origin, dir );
	VectorCopy( dir, hordir );
	hordir[2] = 0;
	dir[2] += 15 + crandom() * 40;
	//botimport.Print(PRT_MESSAGE, "BotTravel_WaterJump: dir[2] = %f\n", dir[2]);
	VectorNormalize( dir );
	dist = VectorNormalize( hordir );
	//elemantary actions
	//EA_Move(ms->client, dir, 400);
	EA_MoveForward( ms->client );
	//move up if close to the actual out of water jump spot
	if ( dist < 40 ) {
		EA_MoveUp( ms->client );
	}
	//set the ideal view angles
	Vector2Angles( dir, result.ideal_viewangles );
	result.flags |= MOVERESULT_MOVEMENTVIEW;
	//
	VectorCopy( dir, result.movedir );
	//
	return result;
} //end of the function BotTravel_WaterJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_WaterJump( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t dir, pnt;
	float dist;
	bot_moveresult_t result;

	//botimport.Print(PRT_MESSAGE, "BotFinishTravel_WaterJump\n");
	BotClearMoveResult( &result );
	//if waterjumping there's nothing to do
	if ( ms->moveflags & MFL_WATERJUMP ) {
		return result;
	}
	//if not touching any water anymore don't do anything
	//otherwise the bot sometimes keeps jumping?
	VectorCopy( ms->origin, pnt );
	pnt[2] -= 32;   //extra for q2dm4 near red armor/mega health
	if ( !( AAS_PointContents( pnt ) & ( CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER ) ) ) {
		return result;
	}
	//swim straight to reachability end
	VectorSubtract( reach->end, ms->origin, dir );
	dir[0] += crandom() * 10;
	dir[1] += crandom() * 10;
	dir[2] += 70 + crandom() * 10;
	dist = VectorNormalize( dir );
	//elemantary actions
	EA_Move( ms->client, dir, 400 );
	//set the ideal view angles
	Vector2Angles( dir, result.ideal_viewangles );
	result.flags |= MOVERESULT_MOVEMENTVIEW;
	//
	VectorCopy( dir, result.movedir );
	//
	return result;
} //end of the function BotFinishTravel_WaterJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_WalkOffLedge( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t hordir, dir;
	float dist, speed, reachhordist;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//check if the bot is blocked by anything
	VectorSubtract( reach->start, ms->origin, dir );
	VectorNormalize( dir );
	BotCheckBlocked( ms, dir, qtrue, &result );
	//if the reachability start and end are practially above each other
	VectorSubtract( reach->end, reach->start, dir );
	dir[2] = 0;
	reachhordist = VectorLength( dir );
	//walk straight to the reachability start
	hordir[0] = reach->start[0] - ms->origin[0];
	hordir[1] = reach->start[1] - ms->origin[1];
	hordir[2] = 0;
	dist = VectorNormalize( hordir );
	//if pretty close to the start focus on the reachability end

	// Ridah, tweaked this
#if 0
	if ( dist < 48 ) {
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		hordir[2] = 0;
#else
	if ( ( dist < 72 ) && ( DotProduct( dir, hordir ) < 0 ) ) { // walk in the direction of start -> end
															   //hordir[0] = reach->end[0] - ms->origin[0];
															   //hordir[1] = reach->end[1] - ms->origin[1];
															   //VectorNormalize( dir );
															   //VectorMA( hordir, 48, dir, hordir );
															   //hordir[2] = 0;

		VectorCopy( dir, hordir );
#endif
		VectorNormalize( hordir );
		//
		if ( reachhordist < 20 ) {

#if defined RTCW_SP
			//speed = 100;
			speed = 200;    // RF, increased this to speed up travel speed down steps
#else
			speed = 100;
#endif // RTCW_XX

		} //end if
		else if ( !AAS_HorizontalVelocityForJump( 0, reach->start, reach->end, &speed ) ) {
			speed = 400;
		} //end if
		  // looks better crouching off a ledge
		EA_Crouch( ms->client );
	} //end if
	else
	{
		if ( reachhordist < 20 ) {
			if ( dist > 64 ) {
				dist = 64;
			}

#if defined RTCW_SP
			//speed = 400 - (256 - 4 * dist);
			speed = 400 - ( 256 - 4 * dist ) * 0.5;   // RF changed this for steps
#else
			speed = 400 - ( 256 - 4 * dist );
#endif // RTCW_XX

		} //end if
		else
		{
			speed = 400;
			// Ridah, tweaked this
			if ( dist < 128 ) {

#if defined RTCW_SP
				speed *= 0.5 + 0.5 * ( dist / 128 );
#else
				speed *= ( dist / 128 );
#endif // RTCW_XX

			}
		} //end else
	} //end else
	  //
	BotCheckBlocked( ms, hordir, qtrue, &result );
	//elemantary action
	EA_Move( ms->client, hordir, speed );
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotTravel_WalkOffLedge
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotAirControl( vec3_t origin, vec3_t velocity, vec3_t goal, vec3_t dir, float *speed ) {
	vec3_t org, vel;
	float dist;
	int i;

	VectorCopy( origin, org );
	VectorScale( velocity, 0.1, vel );
	for ( i = 0; i < 50; i++ )
	{
		vel[2] -= sv_gravity * 0.01;
		//if going down and next position would be below the goal
		if ( vel[2] < 0 && org[2] + vel[2] < goal[2] ) {
			VectorScale( vel, ( goal[2] - org[2] ) / vel[2], vel );
			VectorAdd( org, vel, org );
			VectorSubtract( goal, org, dir );
			dist = VectorNormalize( dir );
			if ( dist > 32 ) {
				dist = 32;
			}
			*speed = 400 - ( 400 - 13 * dist );
			return qtrue;
		} //end if
		else
		{
			VectorAdd( org, vel, org );
		} //end else
	} //end for
	VectorSet( dir, 0, 0, 0 );
	*speed = 400;
	return qfalse;
} //end of the function BotAirControl
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_WalkOffLedge( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t dir, hordir, end, v;
	float dist, speed;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//
	VectorSubtract( reach->end, ms->origin, dir );
	BotCheckBlocked( ms, dir, qtrue, &result );
	//
	VectorSubtract( reach->end, ms->origin, v );
	v[2] = 0;
	dist = VectorNormalize( v );
	if ( dist > 16 ) {
		VectorMA( reach->end, 16, v, end );
	} else { VectorCopy( reach->end, end );}
	//
	if ( !BotAirControl( ms->origin, ms->velocity, end, hordir, &speed ) ) {
		//go straight to the reachability end
		VectorCopy( dir, hordir );
		hordir[2] = 0;
		//
		dist = VectorNormalize( hordir );
		speed = 400;
	} //end if
	  //
	  // looks better crouching off a ledge
	EA_Crouch( ms->client );
	EA_Move( ms->client, hordir, speed );
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotFinishTravel_WalkOffLedge
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
bot_moveresult_t BotTravel_Jump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	vec3_t hordir;
	float dist, gapdist, speed, horspeed, sv_jumpvel;
	bot_moveresult_t result;

	BotClearMoveResult(&result);
	//
	sv_jumpvel = botlibglobals.sv_jumpvel->value;
	//walk straight to the reachability start
	hordir[0] = reach->start[0] - ms->origin[0];
	hordir[1] = reach->start[1] - ms->origin[1];
	hordir[2] = 0;
	dist = VectorNormalize(hordir);
	//
	speed = 350;
	//
	gapdist = BotGapDistance(ms, hordir, ms->entitynum);
	//if pretty close to the start focus on the reachability end
	if (dist < 50 || (gapdist && gapdist < 50))
	{
		//NOTE: using max speed (400) works best
		//if (AAS_HorizontalVelocityForJump(sv_jumpvel, ms->origin, reach->end, &horspeed))
		//{
		//	speed = horspeed * 400 / botlibglobals.sv_maxwalkvelocity->value;
		//} //end if
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		VectorNormalize(hordir);
		//elemantary action jump
		EA_Jump(ms->client);
		//
		ms->jumpreach = ms->lastreachnum;
		speed = 600;
	} //end if
	else
	{
		if (AAS_HorizontalVelocityForJump(sv_jumpvel, reach->start, reach->end, &horspeed))
		{
			speed = horspeed * 400 / botlibglobals.sv_maxwalkvelocity->value;
		} //end if
	} //end else
	//elemantary action
	EA_Move(ms->client, hordir, speed);
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_Jump*/
/*
bot_moveresult_t BotTravel_Jump(bot_movestate_t *ms, aas_reachability_t *reach)
{
	vec3_t hordir, dir1, dir2, mins, maxs, start, end;
	float dist1, dist2, speed;
	bot_moveresult_t result;
	bsp_trace_t trace;

	BotClearMoveResult(&result);
	//
	hordir[0] = reach->start[0] - reach->end[0];
	hordir[1] = reach->start[1] - reach->end[1];
	hordir[2] = 0;
	VectorNormalize(hordir);
	//
	VectorCopy(reach->start, start);
	start[2] += 1;
	//minus back the bouding box size plus 16
	VectorMA(reach->start, 80, hordir, end);
	//
	AAS_PresenceTypeBoundingBox(PRESENCE_NORMAL, mins, maxs);
	//check for solids
	trace = AAS_Trace(start, mins, maxs, end, ms->entitynum, MASK_PLAYERSOLID);
	if (trace.startsolid) VectorCopy(start, trace.endpos);
	//check for a gap
	for (dist1 = 0; dist1 < 80; dist1 += 10)
	{
		VectorMA(start, dist1+10, hordir, end);
		end[2] += 1;
		if (AAS_PointAreaNum(end) != ms->reachareanum) break;
	} //end for
	if (dist1 < 80) VectorMA(reach->start, dist1, hordir, trace.endpos);
//	dist1 = BotGapDistance(start, hordir, ms->entitynum);
//	if (dist1 && dist1 <= trace.fraction * 80) VectorMA(reach->start, dist1-20, hordir, trace.endpos);
	//
	VectorSubtract(ms->origin, reach->start, dir1);
	dir1[2] = 0;
	dist1 = VectorNormalize(dir1);
	VectorSubtract(ms->origin, trace.endpos, dir2);
	dir2[2] = 0;
	dist2 = VectorNormalize(dir2);
	//if just before the reachability start
	if (DotProduct(dir1, dir2) < -0.8 || dist2 < 5)
	{
		//botimport.Print(PRT_MESSAGE, "between jump start and run to point\n");
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize(hordir);
		//elemantary action jump
		if (dist1 < 24) EA_Jump(ms->client);
		else if (dist1 < 32) EA_DelayedJump(ms->client);
		EA_Move(ms->client, hordir, 600);
		//
		ms->jumpreach = ms->lastreachnum;
	} //end if
	else
	{
		//botimport.Print(PRT_MESSAGE, "going towards run to point\n");
		hordir[0] = trace.endpos[0] - ms->origin[0];
		hordir[1] = trace.endpos[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize(hordir);
		//
		if (dist2 > 80) dist2 = 80;
		speed = 400 - (400 - 5 * dist2);
		EA_Move(ms->client, hordir, speed);
	} //end else
	VectorCopy(hordir, result.movedir);
	//
	return result;
} //end of the function BotTravel_Jump*/
//*
bot_moveresult_t BotTravel_Jump( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t hordir, dir1, dir2, start, end, runstart;
//	vec3_t runstart, dir1, dir2, hordir;
	float dist1, dist2, speed;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//
	AAS_JumpReachRunStart( reach, runstart );
	//*
	hordir[0] = runstart[0] - reach->start[0];
	hordir[1] = runstart[1] - reach->start[1];
	hordir[2] = 0;
	VectorNormalize( hordir );
	//
	VectorCopy( reach->start, start );
	start[2] += 1;
	VectorMA( reach->start, 80, hordir, runstart );
	//check for a gap
	for ( dist1 = 0; dist1 < 80; dist1 += 10 )
	{
		VectorMA( start, dist1 + 10, hordir, end );
		end[2] += 1;
		if ( AAS_PointAreaNum( end ) != ms->reachareanum ) {
			break;
		}
	} //end for
	if ( dist1 < 80 ) {
		VectorMA( reach->start, dist1, hordir, runstart );
	}
	//
	VectorSubtract( ms->origin, reach->start, dir1 );
	dir1[2] = 0;
	dist1 = VectorNormalize( dir1 );
	VectorSubtract( ms->origin, runstart, dir2 );
	dir2[2] = 0;
	dist2 = VectorNormalize( dir2 );
	//if just before the reachability start

#if !defined RTCW_ET
	if ( DotProduct( dir1, dir2 ) < -0.8 || dist2 < 5 ) {
#else
	if ( DotProduct( dir1, dir2 ) < -0.8 || dist2 < 12 ) {
#endif // RTCW_XX

//		botimport.Print(PRT_MESSAGE, "between jump start and run start point\n");
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize( hordir );
		//elemantary action jump
		if ( dist1 < 24 ) {
			EA_Jump( ms->client );
		} else if ( dist1 < 32 ) {
			EA_DelayedJump( ms->client );
		}
		EA_Move( ms->client, hordir, 600 );
		//
		ms->jumpreach = ms->lastreachnum;
	} //end if
	else
	{
//		botimport.Print(PRT_MESSAGE, "going towards run start point\n");
		hordir[0] = runstart[0] - ms->origin[0];
		hordir[1] = runstart[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize( hordir );
		//
		if ( dist2 > 80 ) {
			dist2 = 80;
		}
		speed = 400 - ( 400 - 5 * dist2 );
		EA_Move( ms->client, hordir, speed );
	} //end else
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotTravel_Jump*/
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_Jump( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t hordir, hordir2;
	float speed, dist;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//if not jumped yet
	if ( !ms->jumpreach ) {
		return result;
	}
	//go straight to the reachability end
	hordir[0] = reach->end[0] - ms->origin[0];
	hordir[1] = reach->end[1] - ms->origin[1];
	hordir[2] = 0;
	dist = VectorNormalize( hordir );
	//
	hordir2[0] = reach->end[0] - reach->start[0];
	hordir2[1] = reach->end[1] - reach->start[1];
	hordir2[2] = 0;
	VectorNormalize( hordir2 );
	//
	if ( DotProduct( hordir, hordir2 ) < -0.5 && dist < 24 ) {
		return result;
	}
	//always use max speed when traveling through the air
	speed = 800;
	//
	EA_Move( ms->client, hordir, speed );
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotFinishTravel_Jump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Ladder( bot_movestate_t *ms, aas_reachability_t *reach ) {
	//float dist, speed;

#if !defined RTCW_MP
	vec3_t dir, viewdir, hordir, pos, p, v1, v2, vec, right;
#else
	vec3_t dir, viewdir, hordir, pos;
#endif // RTCW_XX

	vec3_t origin = {0, 0, 0};
//	vec3_t up = {0, 0, 1};
	bot_moveresult_t result;
	float dist, speed;

	// RF, heavily modified, wolf has different ladder movement

	BotClearMoveResult( &result );
	//

#if defined RTCW_ET
	// if it's a descend reachability
	if ( reach->start[2] > reach->end[2] ) {
		if ( !( ms->moveflags & MFL_ONGROUND ) ) {
			//botimport.Print(PRT_MESSAGE, "not on ground\n");
			// RF, wolf has different ladder movement
			VectorSubtract( reach->end, reach->start, dir );
			dir[2] = 0;
			dist = VectorNormalize( dir );
			//set the ideal view angles, facing the ladder up or down
			viewdir[0] = dir[0];
			viewdir[1] = dir[1];
			viewdir[2] = 0; // straight forward goes up
			if ( dist >= 3.1 ) {      // vertical ladder -> ladder reachability has length 3, dont inverse in this case
				VectorInverse( viewdir );
			}
			VectorNormalize( viewdir );
			Vector2Angles( viewdir, result.ideal_viewangles );
			//elemantary action
			EA_Move( ms->client, origin, 0 );
			// RF, disabled this check, if we're not on ground, then it shouldn't be a problem, and the ladder flag is unreliable
			//if (ms->moveflags & MFL_AGAINSTLADDER) {
			//botimport.Print(PRT_MESSAGE, "against ladder, descending\n");
			EA_MoveBack( ms->client );      // only move back if we are touching the ladder brush
			// check for sideways adjustments to stay on the center of the ladder
			VectorMA( ms->origin, 18, viewdir, p );
			VectorCopy( reach->start, v1 );
			v1[2] = ms->origin[2];
			VectorCopy( reach->end, v2 );
			v2[2] = ms->origin[2];
			VectorSubtract( v2, v1, vec );
			VectorNormalize( vec );
			VectorMA( v1, -32, vec, v1 );
			VectorMA( v2,  32, vec, v2 );
			ProjectPointOntoVector( p, v1, v2, pos );
			VectorSubtract( pos, p, vec );
			if ( VectorLength( vec ) > 2 ) {
				AngleVectors( result.ideal_viewangles, NULL, right, NULL );
				if ( DotProduct( vec, right ) > 0 ) {
					EA_MoveRight( ms->client );
				} else {
					EA_MoveLeft( ms->client );
				}
			}
			//}
			//set movement view flag so the AI can see the view is focussed
			result.flags |= MOVERESULT_MOVEMENTVIEW;
		} //end if
		else
		{
			//botimport.Print(PRT_MESSAGE, "moving towards ladder top\n");
			// find a postion back away from the edge of the ladder
			VectorSubtract( reach->end, reach->start, hordir );
			hordir[2] = 0;
			VectorNormalize( hordir );
			VectorMA( reach->start, -24, hordir, pos );
			VectorCopy( reach->end, v1 );
			v1[2] = pos[2];
			// project our position onto the vector
			ProjectPointOntoVectorBounded( ms->origin, pos, v1, p );
			VectorSubtract( p, ms->origin, dir );
			//make sure the horizontal movement is large anough
			VectorCopy( dir, hordir );
			hordir[2] = 0;
			dist = VectorNormalize( hordir );
			if ( dist < 64 ) {    // within range, go for the end
				//botimport.Print(PRT_MESSAGE, "found top, moving towards ladder edge\n");
				VectorSubtract( reach->end, reach->start, dir );
				//make sure the horizontal movement is large anough
				dir[2] = 0;
				VectorNormalize( dir );
				//set the ideal view angles, facing the ladder up or down
				viewdir[0] = dir[0];
				viewdir[1] = dir[1];
				viewdir[2] = 0;
				VectorInverse( viewdir );
				VectorNormalize( viewdir );
				Vector2Angles( viewdir, result.ideal_viewangles );
				result.flags |= MOVERESULT_MOVEMENTVIEW;
				// if we are still on ground, then start moving backwards until we are in air
				if ( ( dist < 4 ) && ( ms->moveflags & MFL_ONGROUND ) ) {
					//botimport.Print(PRT_MESSAGE, "close to edge, backing in slowly..\n");
					VectorSubtract( reach->end, ms->origin, vec );
					vec[2] = 0;
					VectorNormalize( vec );
					EA_Move( ms->client, vec, 100 );
					VectorCopy( vec, result.movedir );
					result.ideal_viewangles[PITCH] = 45;    // face downwards
					return result;
				}
			}
			//
			dir[0] = hordir[0];
			dir[1] = hordir[1];
			dir[2] = 0;
			if ( dist > 150 ) {
				dist = 150;
			}
			speed = 400 - ( 300 - 2 * dist );
			//botimport.Print(PRT_MESSAGE, "speed = %.0f\n", speed);
			EA_Move( ms->client, dir, speed );
		} //end else
	} else {
#endif // RTCW_XX

	if ( ( ms->moveflags & MFL_AGAINSTLADDER )
		 //NOTE: not a good idea for ladders starting in water
		 || !( ms->moveflags & MFL_ONGROUND ) ) {
		//botimport.Print(PRT_MESSAGE, "against ladder or not on ground\n");
		// RF, wolf has different ladder movement
		VectorSubtract( reach->end, reach->start, dir );
		VectorNormalize( dir );
		//set the ideal view angles, facing the ladder up or down
		viewdir[0] = dir[0];
		viewdir[1] = dir[1];
		viewdir[2] = dir[2];

#if !defined RTCW_MP
		if ( dir[2] < 0 ) {   // going down, so face the other way (towards ladder)
#else
		if ( dir[2] < 0 ) {   // going down, so face the other way
#endif // RTCW_XX

			VectorInverse( viewdir );

#if !defined RTCW_MP
		}
		viewdir[2] = 0; // straight forward goes up
		VectorNormalize( viewdir );
#else
		} else {
			viewdir[2] = 0; // straight forward goes up
		}
#endif // RTCW_XX

		Vector2Angles( viewdir, result.ideal_viewangles );
		//elemantary action
		EA_Move( ms->client, origin, 0 );

#if !defined RTCW_MP
		if ( dir[2] < 0 ) {   // going down, so face the other way
			EA_MoveBack( ms->client );
		} else {
			EA_MoveForward( ms->client );
		}

#if defined RTCW_ET
			// RF, against ladder code isn't completely accurate
			//if (ms->moveflags & MFL_AGAINSTLADDER) {
#endif // RTCW_XX

#else
		EA_MoveForward( ms->client );
#endif // RTCW_XX

#if !defined RTCW_MP
		// check for sideways adjustments to stay on the center of the ladder
		VectorMA( ms->origin, 18, viewdir, p );
		VectorCopy( reach->start, v1 );
		v1[2] = ms->origin[2];
		VectorCopy( reach->end, v2 );
		v2[2] = ms->origin[2];
		VectorSubtract( v2, v1, vec );
		VectorNormalize( vec );
		VectorMA( v1, -32, vec, v1 );
		VectorMA( v2,  32, vec, v2 );

#if !defined RTCW_ET
		ProjectPointOntoVector( p, v1, v2, pos );
#else
			ProjectPointOntoVectorBounded( p, v1, v2, pos );
#endif // RTCW_XX

		VectorSubtract( pos, p, vec );
		if ( VectorLength( vec ) > 2 ) {
			AngleVectors( result.ideal_viewangles, NULL, right, NULL );
			if ( DotProduct( vec, right ) > 0 ) {
				EA_MoveRight( ms->client );
			} else {
				EA_MoveLeft( ms->client );
			}
		}
#endif // RTCW_XX

		//set movement view flag so the AI can see the view is focussed
		result.flags |= MOVERESULT_MOVEMENTVIEW;
	} //end if
	else
	{
		//botimport.Print(PRT_MESSAGE, "moving towards ladder base\n");
		// find a postion back away from the base of the ladder
		VectorSubtract( reach->end, reach->start, hordir );
		hordir[2] = 0;
		VectorNormalize( hordir );
		VectorMA( reach->start, -24, hordir, pos );

#if defined RTCW_ET
			VectorCopy( reach->end, v1 );
			v1[2] = pos[2];
#endif // RTCW_XX

#if !defined RTCW_MP
		// project our position onto the vector

#if !defined RTCW_ET
		ProjectPointOntoVector( ms->origin, pos, reach->start, p );
#else
			ProjectPointOntoVectorBounded( ms->origin, pos, v1, p );
#endif // RTCW_XX

		VectorSubtract( p, ms->origin, dir );
#else
		VectorSubtract( pos, ms->origin, dir );
#endif // RTCW_XX

		//make sure the horizontal movement is large anough
		VectorCopy( dir, hordir );
		hordir[2] = 0;
		dist = VectorNormalize( hordir );

#if defined RTCW_SP
		if ( dist < 8 ) { // within range, go for the end
#elif defined RTCW_MP
		if ( dist < 32 ) {    // within range, go for the end
#else
			if ( dist < 16 ) {      // within range, go for the end
#endif // RTCW_XX

			//botimport.Print(PRT_MESSAGE, "found base, moving towards ladder top\n");
			VectorSubtract( reach->end, ms->origin, dir );
			//make sure the horizontal movement is large anough
			VectorCopy( dir, hordir );
			hordir[2] = 0;
			dist = VectorNormalize( hordir );

#if !defined RTCW_MP
			//set the ideal view angles, facing the ladder up or down
			viewdir[0] = dir[0];
			viewdir[1] = dir[1];

#if !defined RTCW_ET
			viewdir[2] = dir[2];
			if ( dir[2] < 0 ) {   // going down, so face the other way
				VectorInverse( viewdir );
			}
#endif // RTCW_XX

			viewdir[2] = 0;
			VectorNormalize( viewdir );
			Vector2Angles( viewdir, result.ideal_viewangles );
			result.flags |= MOVERESULT_MOVEMENTVIEW;
#endif // RTCW_XX

		}
		//
		dir[0] = hordir[0];
		dir[1] = hordir[1];
//		if (dist < 48) {
//			if (dir[2] > 0) dir[2] = 1;
//			else dir[2] = -1;
//		} else {
		dir[2] = 0;
//		}
		if ( dist > 50 ) {
			dist = 50;
		}

#if !defined RTCW_MP
		speed = 400 - ( 300 - 6 * dist );
#else
		speed = 400 - ( 200 - 4 * dist );
#endif // RTCW_XX

		EA_Move( ms->client, dir, speed );
	} //end else

#if defined RTCW_ET
	}
#endif // RTCW_XX

	  //save the movement direction
	VectorCopy( dir, result.movedir );
	//
	return result;
} //end of the function BotTravel_Ladder
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Teleport( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t hordir;
	float dist;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//if the bot is being teleported
	if ( ms->moveflags & MFL_TELEPORTED ) {
		return result;
	}

	//walk straight to center of the teleporter
	VectorSubtract( reach->start, ms->origin, hordir );
	if ( !( ms->moveflags & MFL_SWIMMING ) ) {
		hordir[2] = 0;
	}
	dist = VectorNormalize( hordir );
	//
	BotCheckBlocked( ms, hordir, qtrue, &result );

	if ( dist < 30 ) {
		EA_Move( ms->client, hordir, 200 );
	} else { EA_Move( ms->client, hordir, 400 );}

	if ( ms->moveflags & MFL_SWIMMING ) {
		result.flags |= MOVERESULT_SWIMVIEW;
	}

	VectorCopy( hordir, result.movedir );
	return result;
} //end of the function BotTravel_Teleport
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Elevator( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t dir, dir1, dir2, hordir, bottomcenter;
	float dist, dist1, dist2, speed;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//if standing on the plat
	if ( BotOnMover( ms->origin, ms->entitynum, reach ) ) {
#ifdef DEBUG_ELEVATOR
		botimport.Print( PRT_MESSAGE, "bot on elevator\n" );
#endif //DEBUG_ELEVATOR
	   //if vertically not too far from the end point
		if ( c::abs( ms->origin[2] - reach->end[2] ) < sv_maxbarrier ) {
#ifdef DEBUG_ELEVATOR
			botimport.Print( PRT_MESSAGE, "bot moving to end\n" );
#endif //DEBUG_ELEVATOR
			//move to the end point
			VectorSubtract( reach->end, ms->origin, hordir );
			hordir[2] = 0;
			VectorNormalize( hordir );
			if ( !BotCheckBarrierJump( ms, hordir, 100 ) ) {
				EA_Move( ms->client, hordir, 400 );
			} //end if
			VectorCopy( hordir, result.movedir );
		} //end else
		  //if not really close to the center of the elevator
		else
		{
			MoverBottomCenter( reach, bottomcenter );
			VectorSubtract( bottomcenter, ms->origin, hordir );
			hordir[2] = 0;
			dist = VectorNormalize( hordir );
			//
			if ( dist > 10 ) {
#ifdef DEBUG_ELEVATOR
				botimport.Print( PRT_MESSAGE, "bot moving to center\n" );
#endif //DEBUG_ELEVATOR
				//move to the center of the plat
				if ( dist > 100 ) {
					dist = 100;
				}
				speed = 400 - ( 400 - 4 * dist );
				//
				EA_Move( ms->client, hordir, speed );
				VectorCopy( hordir, result.movedir );
			} //end if
		} //end else
	} //end if
	else
	{
#ifdef DEBUG_ELEVATOR
		botimport.Print( PRT_MESSAGE, "bot not on elevator\n" );
#endif //DEBUG_ELEVATOR
	   //if very near the reachability end
		VectorSubtract( reach->end, ms->origin, dir );
		dist = VectorLength( dir );
		if ( dist < 64 ) {
			if ( dist > 60 ) {
				dist = 60;
			}
			speed = 360 - ( 360 - 6 * dist );
			//
			if ( ( ms->moveflags & MFL_SWIMMING ) || !BotCheckBarrierJump( ms, dir, 50 ) ) {
				if ( speed > 5 ) {
					EA_Move( ms->client, dir, speed );
				}
			} //end if
			VectorCopy( dir, result.movedir );
			//
			if ( ms->moveflags & MFL_SWIMMING ) {
				result.flags |= MOVERESULT_SWIMVIEW;
			}
			//stop using this reachability
			ms->reachability_time = 0;
			return result;
		} //end if
		  //get direction and distance to reachability start
		VectorSubtract( reach->start, ms->origin, dir1 );
		if ( !( ms->moveflags & MFL_SWIMMING ) ) {
			dir1[2] = 0;
		}
		dist1 = VectorNormalize( dir1 );
		//if the elevator isn't down
		if ( !MoverDown( reach ) ) {
#ifdef DEBUG_ELEVATOR
			botimport.Print( PRT_MESSAGE, "elevator not down\n" );
#endif //DEBUG_ELEVATOR
			dist = dist1;
			VectorCopy( dir1, dir );
			//
			BotCheckBlocked( ms, dir, qfalse, &result );
			//
			if ( dist > 60 ) {
				dist = 60;
			}
			speed = 360 - ( 360 - 6 * dist );
			//
			if ( !( ms->moveflags & MFL_SWIMMING ) && !BotCheckBarrierJump( ms, dir, 50 ) ) {
				if ( speed > 5 ) {
					EA_Move( ms->client, dir, speed );
				}
			} //end if
			VectorCopy( dir, result.movedir );
			//
			if ( ms->moveflags & MFL_SWIMMING ) {
				result.flags |= MOVERESULT_SWIMVIEW;
			}
			//this isn't a failure... just wait till the elevator comes down
			//result.failure = qtrue;
			result.type = RESULTTYPE_ELEVATORUP;
			result.flags |= MOVERESULT_WAITING;
			return result;
		} //end if
		  //get direction and distance to elevator bottom center
		MoverBottomCenter( reach, bottomcenter );
		VectorSubtract( bottomcenter, ms->origin, dir2 );
		if ( !( ms->moveflags & MFL_SWIMMING ) ) {
			dir2[2] = 0;
		}
		dist2 = VectorNormalize( dir2 );
		//if very close to the reachability start or
		//closer to the elevator center or
		//between reachability start and elevator center
		if ( dist1 < 20 || dist2 < dist1 || DotProduct( dir1, dir2 ) < 0 ) {
#ifdef DEBUG_ELEVATOR
			botimport.Print( PRT_MESSAGE, "bot moving to center\n" );
#endif //DEBUG_ELEVATOR
			dist = dist2;
			VectorCopy( dir2, dir );
		} //end if
		else //closer to the reachability start
		{
#ifdef DEBUG_ELEVATOR
			botimport.Print( PRT_MESSAGE, "bot moving to start\n" );
#endif //DEBUG_ELEVATOR
			dist = dist1;
			VectorCopy( dir1, dir );
		} //end else
		  //
		BotCheckBlocked( ms, dir, qfalse, &result );
		//
		if ( dist > 60 ) {
			dist = 60;
		}
		speed = 400 - ( 400 - 6 * dist );
		//
		if ( !( ms->moveflags & MFL_SWIMMING ) && !BotCheckBarrierJump( ms, dir, 50 ) ) {
			EA_Move( ms->client, dir, speed );
		} //end if
		VectorCopy( dir, result.movedir );
		//
		if ( ms->moveflags & MFL_SWIMMING ) {
			result.flags |= MOVERESULT_SWIMVIEW;
		}
	} //end else
	return result;
} //end of the function BotTravel_Elevator
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_Elevator( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t bottomcenter, bottomdir, topdir;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//
	MoverBottomCenter( reach, bottomcenter );
	VectorSubtract( bottomcenter, ms->origin, bottomdir );
	//
	VectorSubtract( reach->end, ms->origin, topdir );
	//

#if !defined RTCW_ET
	if ( c::fabs( bottomdir[2] ) < c::fabs( topdir[2] ) ) {
#else
	if ( Q_fabs( bottomdir[2] ) < Q_fabs( topdir[2] ) ) {
#endif // RTCW_XX

		VectorNormalize( bottomdir );
		EA_Move( ms->client, bottomdir, 300 );
	} //end if
	else
	{
		VectorNormalize( topdir );
		EA_Move( ms->client, topdir, 300 );
	} //end else
	return result;
} //end of the function BotFinishTravel_Elevator
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotFuncBobStartEnd( aas_reachability_t *reach, vec3_t start, vec3_t end, vec3_t origin ) {
	int spawnflags, modelnum;
	vec3_t mins, maxs, mid, angles = {0, 0, 0};
	int num0, num1;

	modelnum = reach->facenum & 0x0000FFFF;
	if ( !AAS_OriginOfEntityWithModelNum( modelnum, origin ) ) {
		botimport.Print( PRT_MESSAGE, "BotFuncBobStartEnd: no entity with model %d\n", modelnum );
		VectorSet( start, 0, 0, 0 );
		VectorSet( end, 0, 0, 0 );
		return;
	} //end if
	AAS_BSPModelMinsMaxsOrigin( modelnum, angles, mins, maxs, NULL );
	VectorAdd( mins, maxs, mid );
	VectorScale( mid, 0.5, mid );
	VectorCopy( mid, start );
	VectorCopy( mid, end );
	spawnflags = reach->facenum >> 16;
	num0 = reach->edgenum >> 16;
	if ( num0 > 0x00007FFF ) {
		num0 |= 0xFFFF0000;
	}
	num1 = reach->edgenum & 0x0000FFFF;
	if ( num1 > 0x00007FFF ) {
		num1 |= 0xFFFF0000;
	}
	if ( spawnflags & 1 ) {
		start[0] = num0;
		end[0] = num1;
		//
		origin[0] += mid[0];
		origin[1] = mid[1];
		origin[2] = mid[2];
	} //end if
	else if ( spawnflags & 2 ) {
		start[1] = num0;
		end[1] = num1;
		//
		origin[0] = mid[0];
		origin[1] += mid[1];
		origin[2] = mid[2];
	} //end else if
	else
	{
		start[2] = num0;
		end[2] = num1;
		//
		origin[0] = mid[0];
		origin[1] = mid[1];
		origin[2] += mid[2];
	} //end else
} //end of the function BotFuncBobStartEnd
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_FuncBobbing( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t dir, dir1, dir2, hordir, bottomcenter, bob_start, bob_end, bob_origin;
	float dist, dist1, dist2, speed;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//
	BotFuncBobStartEnd( reach, bob_start, bob_end, bob_origin );
	//if standing ontop of the func_bobbing
	if ( BotOnMover( ms->origin, ms->entitynum, reach ) ) {
#ifdef DEBUG_FUNCBOB
		botimport.Print( PRT_MESSAGE, "bot on func_bobbing\n" );
#endif
		//if near end point of reachability
		VectorSubtract( bob_origin, bob_end, dir );
		if ( VectorLength( dir ) < 24 ) {
#ifdef DEBUG_FUNCBOB
			botimport.Print( PRT_MESSAGE, "bot moving to reachability end\n" );
#endif
			//move to the end point
			VectorSubtract( reach->end, ms->origin, hordir );
			hordir[2] = 0;
			VectorNormalize( hordir );
			if ( !BotCheckBarrierJump( ms, hordir, 100 ) ) {
				EA_Move( ms->client, hordir, 400 );
			} //end if
			VectorCopy( hordir, result.movedir );
		} //end else
		  //if not really close to the center of the elevator
		else
		{
			MoverBottomCenter( reach, bottomcenter );
			VectorSubtract( bottomcenter, ms->origin, hordir );
			hordir[2] = 0;
			dist = VectorNormalize( hordir );
			//
			if ( dist > 10 ) {
#ifdef DEBUG_FUNCBOB
				botimport.Print( PRT_MESSAGE, "bot moving to func_bobbing center\n" );
#endif
				//move to the center of the plat
				if ( dist > 100 ) {
					dist = 100;
				}
				speed = 400 - ( 400 - 4 * dist );
				//
				EA_Move( ms->client, hordir, speed );
				VectorCopy( hordir, result.movedir );
			} //end if
		} //end else
	} //end if
	else
	{
#ifdef DEBUG_FUNCBOB
		botimport.Print( PRT_MESSAGE, "bot not ontop of func_bobbing\n" );
#endif
		//if very near the reachability end
		VectorSubtract( reach->end, ms->origin, dir );
		dist = VectorLength( dir );
		if ( dist < 64 ) {
#ifdef DEBUG_FUNCBOB
			botimport.Print( PRT_MESSAGE, "bot moving to end\n" );
#endif
			if ( dist > 60 ) {
				dist = 60;
			}
			speed = 360 - ( 360 - 6 * dist );
			//if swimming or no barrier jump
			if ( ( ms->moveflags & MFL_SWIMMING ) || !BotCheckBarrierJump( ms, dir, 50 ) ) {
				if ( speed > 5 ) {
					EA_Move( ms->client, dir, speed );
				}
			} //end if
			VectorCopy( dir, result.movedir );
			//
			if ( ms->moveflags & MFL_SWIMMING ) {
				result.flags |= MOVERESULT_SWIMVIEW;
			}
			//stop using this reachability
			ms->reachability_time = 0;
			return result;
		} //end if
		  //get direction and distance to reachability start
		VectorSubtract( reach->start, ms->origin, dir1 );
		if ( !( ms->moveflags & MFL_SWIMMING ) ) {
			dir1[2] = 0;
		}
		dist1 = VectorNormalize( dir1 );
		//if func_bobbing is Not it's start position
		VectorSubtract( bob_origin, bob_start, dir );
		if ( VectorLength( dir ) > 16 ) {
#ifdef DEBUG_FUNCBOB
			botimport.Print( PRT_MESSAGE, "func_bobbing not at start\n" );
#endif
			dist = dist1;
			VectorCopy( dir1, dir );
			//
			BotCheckBlocked( ms, dir, qfalse, &result );
			//
			if ( dist > 60 ) {
				dist = 60;
			}
			speed = 360 - ( 360 - 6 * dist );
			//
			if ( !( ms->moveflags & MFL_SWIMMING ) && !BotCheckBarrierJump( ms, dir, 50 ) ) {
				if ( speed > 5 ) {
					EA_Move( ms->client, dir, speed );
				}
			} //end if
			VectorCopy( dir, result.movedir );
			//
			if ( ms->moveflags & MFL_SWIMMING ) {
				result.flags |= MOVERESULT_SWIMVIEW;
			}
			//this isn't a failure... just wait till the func_bobbing arrives
			result.type = RESULTTYPE_ELEVATORUP;
			result.flags |= MOVERESULT_WAITING;
			return result;
		} //end if
		  //get direction and distance to func_bob bottom center
		MoverBottomCenter( reach, bottomcenter );
		VectorSubtract( bottomcenter, ms->origin, dir2 );
		if ( !( ms->moveflags & MFL_SWIMMING ) ) {
			dir2[2] = 0;
		}
		dist2 = VectorNormalize( dir2 );
		//if very close to the reachability start or
		//closer to the elevator center or
		//between reachability start and func_bobbing center
		if ( dist1 < 20 || dist2 < dist1 || DotProduct( dir1, dir2 ) < 0 ) {
#ifdef DEBUG_FUNCBOB
			botimport.Print( PRT_MESSAGE, "bot moving to func_bobbing center\n" );
#endif
			dist = dist2;
			VectorCopy( dir2, dir );
		} //end if
		else //closer to the reachability start
		{
#ifdef DEBUG_FUNCBOB
			botimport.Print( PRT_MESSAGE, "bot moving to reachability start\n" );
#endif
			dist = dist1;
			VectorCopy( dir1, dir );
		} //end else
		  //
		BotCheckBlocked( ms, dir, qfalse, &result );
		//
		if ( dist > 60 ) {
			dist = 60;
		}
		speed = 400 - ( 400 - 6 * dist );
		//
		if ( !( ms->moveflags & MFL_SWIMMING ) && !BotCheckBarrierJump( ms, dir, 50 ) ) {
			EA_Move( ms->client, dir, speed );
		} //end if
		VectorCopy( dir, result.movedir );
		//
		if ( ms->moveflags & MFL_SWIMMING ) {
			result.flags |= MOVERESULT_SWIMVIEW;
		}
	} //end else
	return result;
} //end of the function BotTravel_FuncBobbing
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_FuncBobbing( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t bob_origin, bob_start, bob_end, dir, hordir, bottomcenter;
	bot_moveresult_t result;
	float dist, speed;

	BotClearMoveResult( &result );
	//
	BotFuncBobStartEnd( reach, bob_start, bob_end, bob_origin );
	//
	VectorSubtract( bob_origin, bob_end, dir );
	dist = VectorLength( dir );
	//if the func_bobbing is near the end
	if ( dist < 16 ) {
		VectorSubtract( reach->end, ms->origin, hordir );
		if ( !( ms->moveflags & MFL_SWIMMING ) ) {
			hordir[2] = 0;
		}
		dist = VectorNormalize( hordir );
		//
		if ( dist > 60 ) {
			dist = 60;
		}
		speed = 360 - ( 360 - 6 * dist );
		//
		if ( speed > 5 ) {
			EA_Move( ms->client, dir, speed );
		}
		VectorCopy( dir, result.movedir );
		//
		if ( ms->moveflags & MFL_SWIMMING ) {
			result.flags |= MOVERESULT_SWIMVIEW;
		}
	} //end if
	else
	{
		MoverBottomCenter( reach, bottomcenter );
		VectorSubtract( bottomcenter, ms->origin, hordir );
		if ( !( ms->moveflags & MFL_SWIMMING ) ) {
			hordir[2] = 0;
		}
		dist = VectorNormalize( hordir );
		//
		if ( dist > 5 ) {
			//move to the center of the plat
			if ( dist > 100 ) {
				dist = 100;
			}
			speed = 400 - ( 400 - 4 * dist );
			//
			EA_Move( ms->client, hordir, speed );
			VectorCopy( hordir, result.movedir );
		} //end if
	} //end else
	return result;
} //end of the function BotFinishTravel_FuncBobbing
//===========================================================================
// 0  no valid grapple hook visible
// 1  the grapple hook is still flying
// 2  the grapple hooked into a wall
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
int GrappleState(bot_movestate_t *ms, aas_reachability_t *reach)
{
	static int grapplemodelindex;
	int i;
	vec3_t dir;
	aas_entityinfo_t entinfo;

	if (!grapplemodelindex)
	{
		grapplemodelindex = AAS_IndexFromModel("models/weapons/grapple/hook/tris.md2");
	} //end if
	for (i = AAS_NextEntity(0); i; i = AAS_NextEntity(i))
	{
		if (AAS_EntityModelindex(i) == grapplemodelindex)
		{
			AAS_EntityInfo(i, &entinfo);
			if (VectorCompare(entinfo.origin, entinfo.old_origin))
			{
				VectorSubtract(entinfo.origin, reach->end, dir);
				//if hooked near the reachability end
				if (VectorLength(dir) < 32) return 2;
			} //end if
			else
			{
				//still shooting hook
				return 1;
			} //end else
		} //end if
	} //end if
	//no valid grapple at all
	return 0;
} //end of the function GrappleState*/
//===========================================================================
// 0  no valid grapple hook visible
// 1  the grapple hook is still flying
// 2  the grapple hooked into a wall
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int GrappleState( bot_movestate_t *ms, aas_reachability_t *reach ) {
	static int grapplemodelindex;
	static libvar_t *laserhook;
	int i;
	vec3_t dir;
	aas_entityinfo_t entinfo;

	if ( !laserhook ) {
		laserhook = LibVar( "laserhook", "0" );
	}
	if ( !laserhook->value && !grapplemodelindex ) {
		grapplemodelindex = AAS_IndexFromModel( "models/weapons/grapple/hook/tris.md2" );
	} //end if
	for ( i = AAS_NextEntity( 0 ); i; i = AAS_NextEntity( i ) )
	{
		if ( ( !laserhook->value && AAS_EntityModelindex( i ) == grapplemodelindex )
//			|| (laserhook->value && (AAS_EntityRenderFX(i) & RF_BEAM))
			 ) {
			AAS_EntityInfo( i, &entinfo );
			//if the origin is equal to the last visible origin
			if ( VectorCompare( entinfo.origin, entinfo.lastvisorigin ) ) {
				VectorSubtract( entinfo.origin, reach->end, dir );
				//if hooked near the reachability end
				if ( VectorLength( dir ) < 32 ) {
					return 2;
				}
			} //end if
			else
			{
				//still shooting hook
				return 1;
			} //end else
		} //end if
	} //end for
	  //no valid grapple at all
	return 0;
} //end of the function GrappleState
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotResetGrapple( bot_movestate_t *ms ) {
	aas_reachability_t reach;

	AAS_ReachabilityFromNum( ms->lastreachnum, &reach );
	//if not using the grapple hook reachability anymore
	if ( reach.traveltype != TRAVEL_GRAPPLEHOOK ) {
		if ( ( ms->moveflags & MFL_ACTIVEGRAPPLE ) || ms->grapplevisible_time ) {
			EA_Command( ms->client, CMD_HOOKOFF );
			ms->moveflags &= ~MFL_ACTIVEGRAPPLE;
			ms->grapplevisible_time = 0;
#ifdef DEBUG_GRAPPLE
			botimport.Print( PRT_MESSAGE, "reset grapple\n" );
#endif //DEBUG_GRAPPLE
		} //end if
	} //end if
} //end of the function BotResetGrapple
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_Grapple( bot_movestate_t *ms, aas_reachability_t *reach ) {
	bot_moveresult_t result;
	float dist, speed;
	vec3_t dir, viewdir, org;
	int state, areanum;

#ifdef DEBUG_GRAPPLE
	static int debugline;
	if ( !debugline ) {
		debugline = botimport.DebugLineCreate();
	}
	botimport.DebugLineShow( debugline, reach->start, reach->end, LINECOLOR_BLUE );
#endif //DEBUG_GRAPPLE

	BotClearMoveResult( &result );
	//
	if ( ms->moveflags & MFL_GRAPPLERESET ) {
		EA_Command( ms->client, CMD_HOOKOFF );
		ms->moveflags &= ~MFL_ACTIVEGRAPPLE;
		return result;
	} //end if
	  //
	if ( ms->moveflags & MFL_ACTIVEGRAPPLE ) {
#ifdef DEBUG_GRAPPLE
		botimport.Print( PRT_MESSAGE, "BotTravel_Grapple: active grapple\n" );
#endif //DEBUG_GRAPPLE
	   //
		state = GrappleState( ms, reach );
		//
		VectorSubtract( reach->end, ms->origin, dir );
		dir[2] = 0;
		dist = VectorLength( dir );
		//if very close to the grapple end or
		//the grappled is hooked and the bot doesn't get any closer
		if ( state && dist < 48 ) {
			if ( ms->lastgrappledist - dist < 1 ) {
				EA_Command( ms->client, CMD_HOOKOFF );
				ms->moveflags &= ~MFL_ACTIVEGRAPPLE;
				ms->moveflags |= MFL_GRAPPLERESET;
				ms->reachability_time = 0;  //end the reachability
#ifdef DEBUG_GRAPPLE
				botimport.Print( PRT_ERROR, "grapple normal end\n" );
#endif //DEBUG_GRAPPLE
			} //end if
		} //end if
		  //if no valid grapple at all, or the grapple hooked and the bot
		  //isn't moving anymore
		else if ( !state || ( state == 2 && dist > ms->lastgrappledist - 2 ) ) {
			if ( ms->grapplevisible_time < AAS_Time() - 0.4 ) {
#ifdef DEBUG_GRAPPLE
				botimport.Print( PRT_ERROR, "grapple not visible\n" );
#endif //DEBUG_GRAPPLE
				EA_Command( ms->client, CMD_HOOKOFF );
				ms->moveflags &= ~MFL_ACTIVEGRAPPLE;
				ms->moveflags |= MFL_GRAPPLERESET;
				ms->reachability_time = 0;  //end the reachability
				//result.failure = qtrue;
				//result.type = RESULTTYPE_INVISIBLEGRAPPLE;
				return result;
			} //end if
		} //end if
		else
		{
			ms->grapplevisible_time = AAS_Time();
		} //end else
		  //remember the current grapple distance
		ms->lastgrappledist = dist;
	} //end if
	else
	{
#ifdef DEBUG_GRAPPLE
		botimport.Print( PRT_MESSAGE, "BotTravel_Grapple: inactive grapple\n" );
#endif //DEBUG_GRAPPLE
	   //
		ms->grapplevisible_time = AAS_Time();
		//
		VectorSubtract( reach->start, ms->origin, dir );
		if ( !( ms->moveflags & MFL_SWIMMING ) ) {
			dir[2] = 0;
		}
		VectorAdd( ms->origin, ms->viewoffset, org );
		VectorSubtract( reach->end, org, viewdir );
		//
		dist = VectorNormalize( dir );
		Vector2Angles( viewdir, result.ideal_viewangles );
		result.flags |= MOVERESULT_MOVEMENTVIEW;
		//
		if ( dist < 5 &&

#if !defined RTCW_ET
			 c::fabs( AngleDiff( result.ideal_viewangles[0], ms->viewangles[0] ) ) < 2 &&
			 c::fabs( AngleDiff( result.ideal_viewangles[1], ms->viewangles[1] ) ) < 2 ) {
#else
			 Q_fabs( AngleDiff( result.ideal_viewangles[0], ms->viewangles[0] ) ) < 2 &&
			 Q_fabs( AngleDiff( result.ideal_viewangles[1], ms->viewangles[1] ) ) < 2 ) {
#endif // RTCW_XX

#ifdef DEBUG_GRAPPLE
			botimport.Print( PRT_MESSAGE, "BotTravel_Grapple: activating grapple\n" );
#endif //DEBUG_GRAPPLE
			EA_Command( ms->client, CMD_HOOKON );
			ms->moveflags |= MFL_ACTIVEGRAPPLE;
			ms->lastgrappledist = 999999;
		} //end if
		else
		{
			if ( dist < 70 ) {
				speed = 300 - ( 300 - 4 * dist );
			} else { speed = 400;}
			//
			BotCheckBlocked( ms, dir, qtrue, &result );
			//elemantary action move in direction
			EA_Move( ms->client, dir, speed );
			VectorCopy( dir, result.movedir );
		} //end else
		  //if in another area before actually grappling
		areanum = AAS_PointAreaNum( ms->origin );
		if ( areanum && areanum != ms->reachareanum ) {
			ms->reachability_time = 0;
		}
	} //end else
	return result;
} //end of the function BotTravel_Grapple
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_RocketJump( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t hordir;
	float dist, speed;
	bot_moveresult_t result;

	//botimport.Print(PRT_MESSAGE, "BotTravel_RocketJump: bah\n");
	BotClearMoveResult( &result );
	//
	hordir[0] = reach->start[0] - ms->origin[0];
	hordir[1] = reach->start[1] - ms->origin[1];
	hordir[2] = 0;
	//
	dist = VectorNormalize( hordir );
	//
	if ( dist < 5 ) {
//		botimport.Print(PRT_MESSAGE, "between jump start and run start point\n");
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize( hordir );
		//elemantary action jump
		EA_Jump( ms->client );
		EA_Attack( ms->client );
		EA_Move( ms->client, hordir, 800 );
		//
		ms->jumpreach = ms->lastreachnum;
	} //end if
	else
	{
		if ( dist > 80 ) {
			dist = 80;
		}
		speed = 400 - ( 400 - 5 * dist );
		EA_Move( ms->client, hordir, speed );
	} //end else
	  //
/*
	vec3_t hordir, dir1, dir2, start, end, runstart;
	float dist1, dist2, speed;
	bot_moveresult_t result;

	botimport.Print(PRT_MESSAGE, "BotTravel_RocketJump: bah\n");
	BotClearMoveResult(&result);
	AAS_JumpReachRunStart(reach, runstart);
	//
	hordir[0] = runstart[0] - reach->start[0];
	hordir[1] = runstart[1] - reach->start[1];
	hordir[2] = 0;
	VectorNormalize(hordir);
	//
	VectorCopy(reach->start, start);
	start[2] += 1;
	VectorMA(reach->start, 80, hordir, runstart);
	//check for a gap
	for (dist1 = 0; dist1 < 80; dist1 += 10)
	{
		VectorMA(start, dist1+10, hordir, end);
		end[2] += 1;
		if (AAS_PointAreaNum(end) != ms->reachareanum) break;
	} //end for
	if (dist1 < 80) VectorMA(reach->start, dist1, hordir, runstart);
	//
	VectorSubtract(ms->origin, reach->start, dir1);
	dir1[2] = 0;
	dist1 = VectorNormalize(dir1);
	VectorSubtract(ms->origin, runstart, dir2);
	dir2[2] = 0;
	dist2 = VectorNormalize(dir2);
	//if just before the reachability start
	if (DotProduct(dir1, dir2) < -0.8 || dist2 < 5)
	{
//		botimport.Print(PRT_MESSAGE, "between jump start and run start point\n");
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize(hordir);
		//elemantary action jump
		if (dist1 < 24) EA_Jump(ms->client);
		else if (dist1 < 32) EA_DelayedJump(ms->client);
		EA_Attack(ms->client);
		EA_Move(ms->client, hordir, 800);
		//
		ms->jumpreach = ms->lastreachnum;
	} //end if
	else
	{
//		botimport.Print(PRT_MESSAGE, "going towards run start point\n");
		hordir[0] = runstart[0] - ms->origin[0];
		hordir[1] = runstart[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize(hordir);
		//
		if (dist2 > 80) dist2 = 80;
		speed = 400 - (400 - 5 * dist2);
		EA_Move(ms->client, hordir, speed);
	} //end else
  */
	//look in the movement direction
	Vector2Angles( hordir, result.ideal_viewangles );
	//look straight down
	result.ideal_viewangles[PITCH] = 90;
	//set the view angles directly
	EA_View( ms->client, result.ideal_viewangles );
	//view is important for the movment
	result.flags |= MOVERESULT_MOVEMENTVIEWSET;
	//select the rocket launcher
	EA_SelectWeapon( ms->client, WEAPONINDEX_ROCKET_LAUNCHER );
	//weapon is used for movement
	result.weapon = WEAPONINDEX_ROCKET_LAUNCHER;
	result.flags |= MOVERESULT_MOVEMENTWEAPON;
	//
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotTravel_RocketJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_BFGJump( bot_movestate_t *ms, aas_reachability_t *reach ) {
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//
	return result;
} //end of the function BotTravel_BFGJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_WeaponJump( bot_movestate_t *ms, aas_reachability_t *reach ) {
	vec3_t hordir;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//if not jumped yet
	if ( !ms->jumpreach ) {
		return result;
	}
	//go straight to the reachability end
	hordir[0] = reach->end[0] - ms->origin[0];
	hordir[1] = reach->end[1] - ms->origin[1];
	hordir[2] = 0;
	VectorNormalize( hordir );
	//always use max speed when traveling through the air
	EA_Move( ms->client, hordir, 800 );
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotFinishTravel_WeaponJump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotTravel_JumpPad( bot_movestate_t *ms, aas_reachability_t *reach ) {
	float dist, speed;
	vec3_t hordir;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	//first walk straight to the reachability start
	hordir[0] = reach->start[0] - ms->origin[0];
	hordir[1] = reach->start[1] - ms->origin[1];
	hordir[2] = 0;
	dist = VectorNormalize( hordir );
	//
	BotCheckBlocked( ms, hordir, qtrue, &result );
	speed = 400;
	//elemantary action move in direction
	EA_Move( ms->client, hordir, speed );
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotTravel_JumpPad
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotFinishTravel_JumpPad( bot_movestate_t *ms, aas_reachability_t *reach ) {
	float speed;
	vec3_t hordir;
	bot_moveresult_t result;

	BotClearMoveResult( &result );
	if ( !BotAirControl( ms->origin, ms->velocity, reach->end, hordir, &speed ) ) {
		hordir[0] = reach->end[0] - ms->origin[0];
		hordir[1] = reach->end[1] - ms->origin[1];
		hordir[2] = 0;
		VectorNormalize( hordir );
		speed = 400;
	} //end if
	BotCheckBlocked( ms, hordir, qtrue, &result );
	//elemantary action move in direction
	EA_Move( ms->client, hordir, speed );
	VectorCopy( hordir, result.movedir );
	//
	return result;
} //end of the function BotFinishTravel_JumpPad
//===========================================================================
// time before the reachability times out
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int BotReachabilityTime( aas_reachability_t *reach ) {
	switch ( reach->traveltype )
	{
	case TRAVEL_WALK: return 5;
	case TRAVEL_CROUCH: return 5;
	case TRAVEL_BARRIERJUMP: return 5;
	case TRAVEL_LADDER: return 6;
	case TRAVEL_WALKOFFLEDGE: return 5;
	case TRAVEL_JUMP: return 5;
	case TRAVEL_SWIM: return 5;
	case TRAVEL_WATERJUMP: return 5;
	case TRAVEL_TELEPORT: return 5;
	case TRAVEL_ELEVATOR: return 10;
	case TRAVEL_GRAPPLEHOOK: return 8;
	case TRAVEL_ROCKETJUMP: return 6;
		//case TRAVEL_BFGJUMP: return 6;
	case TRAVEL_JUMPPAD: return 10;
	case TRAVEL_FUNCBOB: return 10;
	default:
	{
		botimport.Print( PRT_ERROR, "travel type %d not implemented yet\n", reach->traveltype );
		return 8;
	}     //end case
	} //end switch
} //end of the function BotReachabilityTime
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_moveresult_t BotMoveInGoalArea( bot_movestate_t *ms, bot_goal_t *goal ) {
	bot_moveresult_t result;
	vec3_t dir;
	float dist, speed;

#ifdef DEBUG
	//botimport.Print(PRT_MESSAGE, "%s: moving straight to goal\n", ClientName(ms->entitynum-1));
	//AAS_ClearShownDebugLines();
	//AAS_DebugLine(ms->origin, goal->origin, LINECOLOR_RED);
#endif //DEBUG
	BotClearMoveResult( &result );
	//walk straight to the goal origin
	dir[0] = goal->origin[0] - ms->origin[0];
	dir[1] = goal->origin[1] - ms->origin[1];
	if ( ms->moveflags & MFL_SWIMMING ) {
		dir[2] = goal->origin[2] - ms->origin[2];
		result.traveltype = TRAVEL_SWIM;
	} //end if
	else
	{
		dir[2] = 0;
		result.traveltype = TRAVEL_WALK;
	} //endif
	  //
	dist = VectorNormalize( dir );
	if ( dist > 100 || ( goal->flags & GFL_NOSLOWAPPROACH ) ) {
		dist = 100;
	}
	speed = 400 - ( 400 - 4 * dist );
	if ( speed < 10 ) {
		speed = 0;
	}
	//
	BotCheckBlocked( ms, dir, qtrue, &result );
	//elemantary action move in direction
	EA_Move( ms->client, dir, speed );
	VectorCopy( dir, result.movedir );
	//
	if ( ms->moveflags & MFL_SWIMMING ) {
		Vector2Angles( dir, result.ideal_viewangles );
		result.flags |= MOVERESULT_SWIMVIEW;
	} //end if
	  //if (!debugline) debugline = botimport.DebugLineCreate();
	  //botimport.DebugLineShow(debugline, ms->origin, goal->origin, LINECOLOR_BLUE);
	  //
	ms->lastreachnum = 0;
	ms->lastareanum = 0;
	ms->lastgoalareanum = goal->areanum;
	VectorCopy( ms->origin, ms->lastorigin );
	ms->lasttime = AAS_Time();
	//
	return result;
} //end of the function BotMoveInGoalArea
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_AreaRouteToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags, int *traveltime, int *reachnum );
extern float VectorDistance( vec3_t v1, vec3_t v2 );
void BotMoveToGoal( bot_moveresult_t *result, int movestate, bot_goal_t *goal, int travelflags ) {

#if defined RTCW_SP
	int reachnum = 0, lastreachnum, foundjumppad, ent; // TTimo: init
#else
	int reachnum = 0; // TTimo (might be used uninitialized in this function)
	int lastreachnum, foundjumppad, ent;
#endif // RTCW_XX

	aas_reachability_t reach, lastreach;
	bot_movestate_t *ms;
	//vec3_t mins, maxs, up = {0, 0, 1};
	//bsp_trace_t trace;
	//static int debugline;

	BotClearMoveResult( result );
	//
	ms = BotMoveStateFromHandle( movestate );
	if ( !ms ) {
		return;
	}
	//reset the grapple before testing if the bot has a valid goal
	//because the bot could loose all it's goals when stuck to a wall
	BotResetGrapple( ms );
	//
	if ( !goal ) {
#ifdef DEBUG
		botimport.Print( PRT_MESSAGE, "client %d: movetogoal -> no goal\n", ms->client );
#endif //DEBUG
		result->failure = qtrue;
		return;
	} //end if
	  //botimport.Print(PRT_MESSAGE, "numavoidreach = %d\n", ms->numavoidreach);
	  //remove some of the move flags
	ms->moveflags &= ~( MFL_SWIMMING | MFL_AGAINSTLADDER );
	//set some of the move flags
	//NOTE: the MFL_ONGROUND flag is also set in the higher AI
	if ( AAS_OnGround( ms->origin, ms->presencetype, ms->entitynum ) ) {
		ms->moveflags |= MFL_ONGROUND;
	}
	//

	if ( ms->moveflags & MFL_ONGROUND ) {
		int modeltype, modelnum;

		ent = BotOnTopOfEntity( ms );

		if ( ent != -1 ) {
			modelnum = AAS_EntityModelindex( ent );
			if ( modelnum >= 0 && modelnum < MAX_MODELS ) {
				modeltype = modeltypes[modelnum];

				if ( modeltype == MODELTYPE_FUNC_PLAT ) {
					AAS_ReachabilityFromNum( ms->lastreachnum, &reach );
					//if the bot is Not using the elevator
					if ( reach.traveltype != TRAVEL_ELEVATOR ||
						 //NOTE: the face number is the plat model number
						 ( reach.facenum & 0x0000FFFF ) != modelnum ) {
						reachnum = AAS_NextModelReachability( 0, modelnum );
						if ( reachnum ) {
							//botimport.Print(PRT_MESSAGE, "client %d: accidentally ended up on func_plat\n", ms->client);
							AAS_ReachabilityFromNum( reachnum, &reach );
							ms->lastreachnum = reachnum;
							ms->reachability_time = AAS_Time() + BotReachabilityTime( &reach );
						} //end if
						else
						{
							if ( bot_developer ) {
								botimport.Print( PRT_MESSAGE, "client %d: on func_plat without reachability\n", ms->client );
							} //end if
							result->blocked = qtrue;
							result->blockentity = ent;
							result->flags |= MOVERESULT_ONTOPOFOBSTACLE;
							return;
						} //end else
					} //end if
					result->flags |= MOVERESULT_ONTOPOF_ELEVATOR;
				} //end if
				else if ( modeltype == MODELTYPE_FUNC_BOB ) {
					AAS_ReachabilityFromNum( ms->lastreachnum, &reach );
					//if the bot is Not using the func bobbing
					if ( reach.traveltype != TRAVEL_FUNCBOB ||
						 //NOTE: the face number is the func_bobbing model number
						 ( reach.facenum & 0x0000FFFF ) != modelnum ) {
						reachnum = AAS_NextModelReachability( 0, modelnum );
						if ( reachnum ) {
							//botimport.Print(PRT_MESSAGE, "client %d: accidentally ended up on func_bobbing\n", ms->client);
							AAS_ReachabilityFromNum( reachnum, &reach );
							ms->lastreachnum = reachnum;
							ms->reachability_time = AAS_Time() + BotReachabilityTime( &reach );
						} //end if
						else
						{
							if ( bot_developer ) {
								botimport.Print( PRT_MESSAGE, "client %d: on func_bobbing without reachability\n", ms->client );
							} //end if
							result->blocked = qtrue;
							result->blockentity = ent;
							result->flags |= MOVERESULT_ONTOPOFOBSTACLE;
							return;
						} //end else
					} //end if
					result->flags |= MOVERESULT_ONTOPOF_FUNCBOB;
				} //end if
				  /* Ridah, disabled this, or standing on little fragments causes problems
				  else
				  {
					  result->blocked = qtrue;
					  result->blockentity = ent;
					  result->flags |= MOVERESULT_ONTOPOFOBSTACLE;
					  return;
				  } //end else
				  */
			} //end if
		} //end if
	} //end if
	  //if swimming
	if ( AAS_Swimming( ms->origin ) ) {
		ms->moveflags |= MFL_SWIMMING;
	}
	//if against a ladder
	if ( AAS_AgainstLadder( ms->origin, ms->areanum ) ) {
		ms->moveflags |= MFL_AGAINSTLADDER;
	}
	//if the bot is on the ground, swimming or against a ladder
	if ( ms->moveflags & ( MFL_ONGROUND | MFL_SWIMMING | MFL_AGAINSTLADDER ) ) {
		//botimport.Print(PRT_MESSAGE, "%s: onground, swimming or against ladder\n", ClientName(ms->entitynum-1));
		//
		AAS_ReachabilityFromNum( ms->lastreachnum, &lastreach );
		//reachability area the bot is in
		//ms->areanum = BotReachabilityArea(ms->origin, (lastreach.traveltype != TRAVEL_ELEVATOR));

#if !defined RTCW_ET
		ms->areanum = BotFuzzyPointReachabilityArea( ms->origin );
#else
		if ( !ms->areanum ) {
			result->failure = qtrue;
			return;
		}
		//ms->areanum = BotFuzzyPointReachabilityArea(ms->origin);
#endif // RTCW_XX

		//if the bot is in the goal area
		if ( ms->areanum == goal->areanum ) {
			*result = BotMoveInGoalArea( ms, goal );
			return;
		} //end if
		  //assume we can use the reachability from the last frame
		reachnum = ms->lastreachnum;
		//if there is a last reachability

#if !defined RTCW_ET
// Ridah, best calc it each frame, especially for Zombie, which moves so slow that we can't afford to take a long route
//		reachnum = 0;
// done.
#endif // RTCW_XX

		if ( reachnum ) {
			AAS_ReachabilityFromNum( reachnum, &reach );
			//check if the reachability is still valid
			if ( !( AAS_TravelFlagForType( reach.traveltype ) & travelflags ) ) {
				reachnum = 0;
			} //end if
			  //special grapple hook case
			else if ( reach.traveltype == TRAVEL_GRAPPLEHOOK ) {
				if ( ms->reachability_time < AAS_Time() ||
					 ( ms->moveflags & MFL_GRAPPLERESET ) ) {
					reachnum = 0;
				} //end if
			} //end if
			  //special elevator case
			else if ( reach.traveltype == TRAVEL_ELEVATOR || reach.traveltype == TRAVEL_FUNCBOB ) {
				if ( ( result->flags & MOVERESULT_ONTOPOF_FUNCBOB ) ||
					 ( result->flags & MOVERESULT_ONTOPOF_FUNCBOB ) ) {
					ms->reachability_time = AAS_Time() + 5;
				} //end if
				  //if the bot was going for an elevator and reached the reachability area
				if ( ms->areanum == reach.areanum ||
					 ms->reachability_time < AAS_Time() ) {
					reachnum = 0;
				} //end if
			} //end if
			else
			{

#if defined RTCW_ET
				if ( ms->reachability_time < AAS_Time() ) {
					// the reachability timed out, add it to the ignore list
#ifdef AVOIDREACH
					BotAddToAvoidReach( ms, reachnum, AVOIDREACH_TIME + 4 );
#endif //AVOIDREACH
				}
#endif // RTCW_XX

#ifdef DEBUG
				if ( bot_developer ) {
					if ( ms->reachability_time < AAS_Time() ) {
						botimport.Print( PRT_MESSAGE, "client %d: reachability timeout in ", ms->client );
						AAS_PrintTravelType( reach.traveltype );
						botimport.Print( PRT_MESSAGE, "\n" );
					} //end if
					  /*
					  if (ms->lastareanum != ms->areanum)
					  {
						  botimport.Print(PRT_MESSAGE, "changed from area %d to %d\n", ms->lastareanum, ms->areanum);
					  } //end if*/
				} //end if
#endif //DEBUG
				//if the goal area changed or the reachability timed out
				//or the area changed
				if ( ms->lastgoalareanum != goal->areanum ||
					 ms->reachability_time < AAS_Time() ||
					 ms->lastareanum != ms->areanum ||

#if !defined RTCW_ET
					 ( ( ms->lasttime > ( AAS_Time() - 0.5 ) ) && ( VectorDistance( ms->origin, ms->lastorigin ) < 20 * ( AAS_Time() - ms->lasttime ) ) ) ) {
#else
					 //@TODO.  The hardcoded distance here should actually be tied to speed.  As it was, 20 was too big for
					 // slow moving walking Nazis.
//						((ms->lasttime > (AAS_Time()-0.5)) && (VectorDistance(ms->origin, ms->lastorigin) < 20*(AAS_Time()-ms->lasttime))))
					 ( ( ms->lasttime > ( AAS_Time() - 0.5 ) ) && ( VectorDistance( ms->origin, ms->lastorigin ) < 5 * ( AAS_Time() - ms->lasttime ) ) ) ) {
#endif // RTCW_XX

					reachnum = 0;
					//botimport.Print(PRT_MESSAGE, "area change or timeout\n");
				} //end else if
			} //end else
		} //end if
		  //if the bot needs a new reachability
		if ( !reachnum ) {
			//if the area has no reachability links
			if ( !AAS_AreaReachability( ms->areanum ) ) {
#ifdef DEBUG
				if ( bot_developer ) {
					botimport.Print( PRT_MESSAGE, "area %d no reachability\n", ms->areanum );
				} //end if
#endif //DEBUG
			} //end if
			  //get a new reachability leading towards the goal
			reachnum = BotGetReachabilityToGoal( ms->origin, ms->areanum, ms->entitynum,
												 ms->lastgoalareanum, ms->lastareanum,
												 ms->avoidreach, ms->avoidreachtimes, ms->avoidreachtries,
												 goal, travelflags, travelflags );
			//the area number the reachability starts in
			ms->reachareanum = ms->areanum;
			//reset some state variables
			ms->jumpreach = 0;                      //for TRAVEL_JUMP
			ms->moveflags &= ~MFL_GRAPPLERESET; //for TRAVEL_GRAPPLEHOOK
			//if there is a reachability to the goal
			if ( reachnum ) {
				AAS_ReachabilityFromNum( reachnum, &reach );
				//set a timeout for this reachability

#if !defined RTCW_ET
				ms->reachability_time = AAS_Time() + BotReachabilityTime( &reach );
#else
				ms->reachability_time = AAS_Time() + (float)( 0.01 * ( AAS_AreaTravelTime( ms->areanum, ms->origin, reach.start ) * 2 ) + BotReachabilityTime( &reach ) + 100 );
				if ( reach.traveltype == TRAVEL_LADDER ) {
					ms->reachability_time += 4.0;   // allow more time to navigate ladders
				}
#endif // RTCW_XX

				//
#ifdef AVOIDREACH

#if !defined RTCW_ET
				//add the reachability to the reachabilities to avoid for a while
				BotAddToAvoidReach( ms, reachnum, AVOIDREACH_TIME );
#else
				if ( reach.traveltype != TRAVEL_LADDER ) {    // don't avoid ladders unless we were unable to reach them in time
					BotAddToAvoidReach( ms, reachnum, 3 );    // add a short avoid reach time
				}
#endif // RTCW_XX

#endif //AVOIDREACH
			} //end if
#ifdef DEBUG

			else if ( bot_developer ) {
				botimport.Print( PRT_MESSAGE, "goal not reachable\n" );
				memset( &reach, 0, sizeof( aas_reachability_t ) ); //make compiler happy
			} //end else
			if ( bot_developer ) {
				//if still going for the same goal
				if ( ms->lastgoalareanum == goal->areanum ) {
					if ( ms->lastareanum == reach.areanum ) {
						botimport.Print( PRT_MESSAGE, "same goal, going back to previous area\n" );
					} //end if
				} //end if
			} //end if
#endif //DEBUG
		} //end else
		  //
		ms->lastreachnum = reachnum;
		ms->lastgoalareanum = goal->areanum;
		ms->lastareanum = ms->areanum;
		//if the bot has a reachability
		if ( reachnum ) {
			//get the reachability from the number
			AAS_ReachabilityFromNum( reachnum, &reach );
			result->traveltype = reach.traveltype;
			//

#if !defined RTCW_ET
#ifdef DEBUG_AI_MOVE
			AAS_ClearShownDebugLines();
			AAS_PrintTravelType( reach.traveltype );
			AAS_ShowReachability( &reach );
#endif //DEBUG_AI_MOVE
#else
			if ( goal->flags & GFL_DEBUGPATH ) {
				AAS_ClearShownPolygons();
				AAS_ClearShownDebugLines();
				AAS_PrintTravelType( reach.traveltype );
				// src area
				AAS_ShowAreaPolygons( ms->areanum, 1, qtrue );
				// dest area
				AAS_ShowAreaPolygons( goal->areanum, 3, qtrue );
				// reachability
				AAS_ShowReachability( &reach );
				AAS_ShowAreaPolygons( reach.areanum, 2, qtrue );
			}
#endif // RTCW_XX

			//
#ifdef DEBUG
			//botimport.Print(PRT_MESSAGE, "client %d: ", ms->client);
			//AAS_PrintTravelType(reach.traveltype);
			//botimport.Print(PRT_MESSAGE, "\n");
#endif //DEBUG
			switch ( reach.traveltype )
			{
			case TRAVEL_WALK: *result = BotTravel_Walk( ms, &reach ); break;
			case TRAVEL_CROUCH: *result = BotTravel_Crouch( ms, &reach ); break;
			case TRAVEL_BARRIERJUMP: *result = BotTravel_BarrierJump( ms, &reach ); break;
			case TRAVEL_LADDER: *result = BotTravel_Ladder( ms, &reach ); break;
			case TRAVEL_WALKOFFLEDGE: *result = BotTravel_WalkOffLedge( ms, &reach ); break;
			case TRAVEL_JUMP: *result = BotTravel_Jump( ms, &reach ); break;
			case TRAVEL_SWIM: *result = BotTravel_Swim( ms, &reach ); break;
			case TRAVEL_WATERJUMP: *result = BotTravel_WaterJump( ms, &reach ); break;
			case TRAVEL_TELEPORT: *result = BotTravel_Teleport( ms, &reach ); break;
			case TRAVEL_ELEVATOR: *result = BotTravel_Elevator( ms, &reach ); break;
			case TRAVEL_GRAPPLEHOOK: *result = BotTravel_Grapple( ms, &reach ); break;
			case TRAVEL_ROCKETJUMP: *result = BotTravel_RocketJump( ms, &reach ); break;
				//case TRAVEL_BFGJUMP:
			case TRAVEL_JUMPPAD: *result = BotTravel_JumpPad( ms, &reach ); break;
			case TRAVEL_FUNCBOB: *result = BotTravel_FuncBobbing( ms, &reach ); break;
			default:
			{
				botimport.Print( PRT_FATAL, "travel type %d not implemented yet\n", reach.traveltype );
				break;
			}     //end case
			} //end switch
		} //end if
		else
		{
			result->failure = qtrue;
			memset( &reach, 0, sizeof( aas_reachability_t ) );
		} //end else
#ifdef DEBUG
		if ( bot_developer ) {
			if ( result->failure ) {
				botimport.Print( PRT_MESSAGE, "client %d: movement failure in ", ms->client );
				AAS_PrintTravelType( reach.traveltype );
				botimport.Print( PRT_MESSAGE, "\n" );
			} //end if
		} //end if
#endif //DEBUG
	} //end if
	else
	{
		int i, numareas, areas[16];
		vec3_t end;

		//special handling of jump pads when the bot uses a jump pad without knowing it
		foundjumppad = qfalse;
		VectorMA( ms->origin, -2 * ms->thinktime, ms->velocity, end );
		numareas = AAS_TraceAreas( ms->origin, end, areas, NULL, 16 );
		for ( i = numareas - 1; i >= 0; i-- )
		{
			if ( AAS_AreaJumpPad( areas[i] ) ) {
				//botimport.Print(PRT_MESSAGE, "client %d used a jumppad without knowing, area %d\n", ms->client, areas[i]);
				foundjumppad = qtrue;
				lastreachnum = BotGetReachabilityToGoal( end, areas[i], ms->entitynum,
														 ms->lastgoalareanum, ms->lastareanum,
														 ms->avoidreach, ms->avoidreachtimes, ms->avoidreachtries,
														 goal, travelflags, TFL_JUMPPAD );
				if ( lastreachnum ) {
					ms->lastreachnum = lastreachnum;
					ms->lastareanum = areas[i];
					//botimport.Print(PRT_MESSAGE, "found jumppad reachability\n");
					break;
				} //end if
				else
				{
					for ( lastreachnum = AAS_NextAreaReachability( areas[i], 0 ); lastreachnum;
						  lastreachnum = AAS_NextAreaReachability( areas[i], lastreachnum ) )
					{
						//get the reachability from the number
						AAS_ReachabilityFromNum( lastreachnum, &reach );
						if ( reach.traveltype == TRAVEL_JUMPPAD ) {
							ms->lastreachnum = lastreachnum;
							ms->lastareanum = areas[i];
							//botimport.Print(PRT_MESSAGE, "found jumppad reachability hard!!\n");
							break;
						} //end if
					} //end for
					if ( lastreachnum ) {
						break;
					}
				} //end else
			} //end if
		} //end for
		if ( bot_developer ) {
			//if a jumppad is found with the trace but no reachability is found
			if ( foundjumppad && !ms->lastreachnum ) {
				botimport.Print( PRT_MESSAGE, "client %d didn't find jumppad reachability\n", ms->client );
			} //end if
		} //end if
		  //
		if ( ms->lastreachnum ) {
			//botimport.Print(PRT_MESSAGE, "%s: NOT onground, swimming or against ladder\n", ClientName(ms->entitynum-1));
			AAS_ReachabilityFromNum( ms->lastreachnum, &reach );
			result->traveltype = reach.traveltype;
#ifdef DEBUG
			//botimport.Print(PRT_MESSAGE, "client %d finish: ", ms->client);
			//AAS_PrintTravelType(reach.traveltype);
			//botimport.Print(PRT_MESSAGE, "\n");
#endif //DEBUG
			//
			switch ( reach.traveltype )
			{
			case TRAVEL_WALK: *result = BotTravel_Walk( ms, &reach ); break;  //BotFinishTravel_Walk(ms, &reach); break;
			case TRAVEL_CROUCH: /*do nothing*/ break;
			case TRAVEL_BARRIERJUMP: *result = BotFinishTravel_BarrierJump( ms, &reach ); break;
			case TRAVEL_LADDER: *result = BotTravel_Ladder( ms, &reach ); break;
			case TRAVEL_WALKOFFLEDGE: *result = BotFinishTravel_WalkOffLedge( ms, &reach ); break;
			case TRAVEL_JUMP: *result = BotFinishTravel_Jump( ms, &reach ); break;
			case TRAVEL_SWIM: *result = BotTravel_Swim( ms, &reach ); break;
			case TRAVEL_WATERJUMP: *result = BotFinishTravel_WaterJump( ms, &reach ); break;
			case TRAVEL_TELEPORT: /*do nothing*/ break;
			case TRAVEL_ELEVATOR: *result = BotFinishTravel_Elevator( ms, &reach ); break;
			case TRAVEL_GRAPPLEHOOK: *result = BotTravel_Grapple( ms, &reach ); break;
			case TRAVEL_ROCKETJUMP: *result = BotFinishTravel_WeaponJump( ms, &reach ); break;
				//case TRAVEL_BFGJUMP:
			case TRAVEL_JUMPPAD: *result = BotFinishTravel_JumpPad( ms, &reach ); break;
			case TRAVEL_FUNCBOB: *result = BotFinishTravel_FuncBobbing( ms, &reach ); break;
			default:
			{
				botimport.Print( PRT_FATAL, "(last) travel type %d not implemented yet\n", reach.traveltype );
				break;
			}     //end case
			} //end switch
#ifdef DEBUG
			if ( bot_developer ) {
				if ( result->failure ) {
					botimport.Print( PRT_MESSAGE, "client %d: movement failure in finish ", ms->client );
					AAS_PrintTravelType( reach.traveltype );
					botimport.Print( PRT_MESSAGE, "\n" );
				} //end if
			} //end if
#endif //DEBUG
		} //end if
	} //end else
	  //FIXME: is it right to do this here?
	if ( result->blocked ) {
		ms->reachability_time -= 10 * ms->thinktime;
	}
	//copy the last origin
	VectorCopy( ms->origin, ms->lastorigin );
	ms->lasttime = AAS_Time();

#if !defined RTCW_ET
	// RF, try to look in the direction we will be moving ahead of time
	if ( reachnum > 0 && !( result->flags & ( MOVERESULT_MOVEMENTVIEW | MOVERESULT_SWIMVIEW ) ) ) {
		vec3_t dir;

#if defined RTCW_SP
		int ftraveltime, freachnum, straveltime, ltraveltime;
#elif defined RTCW_MP
		int ftraveltime, freachnum;
#endif // RTCW_XX

		AAS_ReachabilityFromNum( reachnum, &reach );
		if ( reach.areanum != goal->areanum ) {

#if defined RTCW_SP
			if ( AAS_AreaRouteToGoalArea( reach.areanum, reach.end, goal->areanum, travelflags, &straveltime, &freachnum ) ) {
#elif defined RTCW_MP
			if ( AAS_AreaRouteToGoalArea( reach.areanum, reach.end, goal->areanum, travelflags, &ftraveltime, &freachnum ) ) {
				AAS_ReachabilityFromNum( freachnum, &reach );
#endif // RTCW_XX

#if defined RTCW_SP
				ltraveltime = 999999;
				while ( AAS_AreaRouteToGoalArea( reach.areanum, reach.end, goal->areanum, travelflags, &ftraveltime, &freachnum ) ) {
					// make sure we are not in a loop
					if ( ftraveltime > ltraveltime ) {
						break;
					}
					ltraveltime = ftraveltime;
					//
					AAS_ReachabilityFromNum( freachnum, &reach );
					if ( reach.areanum == goal->areanum ) {
#elif defined RTCW_MP
				VectorSubtract( reach.end, ms->origin, dir );
				VectorNormalize( dir );
				vectoangles( dir, result->ideal_viewangles );
				result->flags |= MOVERESULT_FUTUREVIEW;
			}
		} else {
#endif // RTCW_XX

#if defined RTCW_SP
						VectorSubtract( goal->origin, ms->origin, dir );
						VectorNormalize( dir );
						vectoangles( dir, result->ideal_viewangles );
						result->flags |= MOVERESULT_FUTUREVIEW;
						break;
					}
					if ( straveltime - ftraveltime > 120 ) {
						VectorSubtract( reach.end, ms->origin, dir );
						VectorNormalize( dir );
						vectoangles( dir, result->ideal_viewangles );
						result->flags |= MOVERESULT_FUTUREVIEW;
						break;
					}
				}
			}
		} else {
#endif // RTCW_XX

			VectorSubtract( goal->origin, ms->origin, dir );
			VectorNormalize( dir );
			vectoangles( dir, result->ideal_viewangles );
			result->flags |= MOVERESULT_FUTUREVIEW;
		}
	}

#else
/*
	// RF, try to look in the direction we will be moving ahead of time
	if (reachnum > 0 && !(result->flags & (MOVERESULT_MOVEMENTVIEW|MOVERESULT_SWIMVIEW))) {
		vec3_t dir;
		int ftraveltime, freachnum;

		AAS_ReachabilityFromNum( reachnum, &reach);
		if (reach.areanum != goal->areanum) {
			if (AAS_AreaRouteToGoalArea( reach.areanum, reach.end, goal->areanum, travelflags, &ftraveltime, &freachnum )) {
					AAS_ReachabilityFromNum( freachnum, &reach);
						VectorSubtract( reach.end, ms->origin, dir );
						VectorNormalize( dir );
						vectoangles( dir, result->ideal_viewangles );
						result->flags |= MOVERESULT_FUTUREVIEW;
					}
		} else {
			VectorSubtract( goal->origin, ms->origin, dir );
			VectorNormalize( dir );
			vectoangles( dir, result->ideal_viewangles );
			result->flags |= MOVERESULT_FUTUREVIEW;
		}
	}
*/
#endif // RTCW_XX

	//return the movement result
	return;
} //end of the function BotMoveToGoal
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotResetAvoidReach( int movestate ) {
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle( movestate );
	if ( !ms ) {
		return;
	}
	memset( ms->avoidreach, 0, MAX_AVOIDREACH * sizeof( int ) );
	memset( ms->avoidreachtimes, 0, MAX_AVOIDREACH * sizeof( float ) );
	memset( ms->avoidreachtries, 0, MAX_AVOIDREACH * sizeof( int ) );

	// RF, also clear movestate stuff
	ms->lastareanum = 0;
	ms->lastgoalareanum = 0;
	ms->lastreachnum = 0;
} //end of the function BotResetAvoidReach
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotResetLastAvoidReach( int movestate ) {
	int i, latest;
	float latesttime;
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle( movestate );
	if ( !ms ) {
		return;
	}
	latesttime = 0;
	latest = 0;
	for ( i = 0; i < MAX_AVOIDREACH; i++ )
	{
		if ( ms->avoidreachtimes[i] > latesttime ) {
			latesttime = ms->avoidreachtimes[i];
			latest = i;
		} //end if
	} //end for
	if ( latesttime ) {
		ms->avoidreachtimes[latest] = 0;
		if ( ms->avoidreachtries[i] > 0 ) {
			ms->avoidreachtries[latest]--;
		}
	} //end if
} //end of the function BotResetLastAvoidReach
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotResetMoveState( int movestate ) {
	bot_movestate_t *ms;

	ms = BotMoveStateFromHandle( movestate );
	if ( !ms ) {
		return;
	}
	memset( ms, 0, sizeof( bot_movestate_t ) );
} //end of the function BotResetMoveState
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int BotSetupMoveAI( void ) {
	BotSetBrushModelTypes();
	sv_maxstep = LibVarValue( "sv_step", "18" );
	sv_maxbarrier = LibVarValue( "sv_maxbarrier", "32" );
	sv_gravity = LibVarValue( "sv_gravity", "800" );
	return BLERR_NOERROR;
} //end of the function BotSetupMoveAI
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotShutdownMoveAI( void ) {
	int i;

	for ( i = 1; i <= MAX_CLIENTS; i++ )
	{
		if ( botmovestates[i] ) {
			FreeMemory( botmovestates[i] );
			botmovestates[i] = NULL;
		} //end if
	} //end for
} //end of the function BotShutdownMoveAI
