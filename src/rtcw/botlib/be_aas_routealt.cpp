/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_routealt.c
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

#include "q_shared.h"
#include "l_utils.h"
#include "l_memory.h"
#include "l_log.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_interface.h"
#include "be_aas_def.h"

#if !defined RTCW_ET
//#define ENABLE_ALTROUTING
#else
#define ENABLE_ALTROUTING
#endif // RTCW_XX

typedef struct midrangearea_s
{
	int valid;
	unsigned short starttime;
	unsigned short goaltime;
} midrangearea_t;

midrangearea_t *midrangeareas;
int *clusterareas;
int numclusterareas;

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_AltRoutingFloodCluster_r( int areanum ) {
	int i, otherareanum;
	aas_area_t *area;
	aas_face_t *face;

	//add the current area to the areas of the current cluster
	clusterareas[numclusterareas] = areanum;
	numclusterareas++;
	//remove the area from the mid range areas
	midrangeareas[areanum].valid = qfalse;
	//flood to other areas through the faces of this area
	area = &( *aasworld ).areas[areanum];
	for ( i = 0; i < area->numfaces; i++ )
	{
		face = &( *aasworld ).faces[c::abs( ( *aasworld ).faceindex[area->firstface + i] )];
		//get the area at the other side of the face
		if ( face->frontarea == areanum ) {
			otherareanum = face->backarea;
		} else { otherareanum = face->frontarea;}
		//if there is an area at the other side of this face
		if ( !otherareanum ) {
			continue;
		}
		//if the other area is not a midrange area
		if ( !midrangeareas[otherareanum].valid ) {
			continue;
		}
		//
		AAS_AltRoutingFloodCluster_r( otherareanum );
	} //end for
} //end of the function AAS_AltRoutingFloodCluster_r
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================

#if defined RTCW_ET
int AAS_AreaRouteToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags, int *traveltime, int *reachnum );
#endif // RTCW_XX

int AAS_AlternativeRouteGoals( vec3_t start, vec3_t goal, int travelflags,
							   aas_altroutegoal_t *altroutegoals, int maxaltroutegoals,
							   int color ) {
#ifndef ENABLE_ALTROUTING
	return 0;
#else
	int i, j, startareanum, goalareanum, bestareanum;
	int numaltroutegoals, nummidrangeareas;
	int starttime, goaltime, goaltraveltime;
	float dist, bestdist;
	vec3_t mid, dir;

#if !defined RTCW_ET
#ifdef DEBUG
	int startmillisecs;

	startmillisecs = Sys_MilliSeconds();
#endif
#else
	int reachnum, time;
	int a1, a2;
/*#ifdef DEBUG
	int startmillisecs;

	startmillisecs = Sys_MilliSeconds();
#endif*/
#endif // RTCW_XX

	startareanum = AAS_PointAreaNum( start );
	if ( !startareanum ) {
		return 0;
	}
	goalareanum = AAS_PointAreaNum( goal );
	if ( !goalareanum ) {

#if !defined RTCW_ET
		return 0;
#else
		VectorCopy( goal, dir );
		dir[2] += 30;
		goalareanum = AAS_PointAreaNum( dir );
		if ( !goalareanum ) {
			return 0;
		}
#endif // RTCW_XX

	}
	//travel time towards the goal area
	goaltraveltime = AAS_AreaTravelTimeToGoalArea( startareanum, start, goalareanum, travelflags );
	//clear the midrange areas
	memset( midrangeareas, 0, ( *aasworld ).numareas * sizeof( midrangearea_t ) );
	numaltroutegoals = 0;
	//
	nummidrangeareas = 0;
	//
	for ( i = 1; i < ( *aasworld ).numareas; i++ )
	{
		//

#if !defined RTCW_ET
		if ( !( ( *aasworld ).areasettings[i].contents & AREACONTENTS_ROUTEPORTAL ) ) {
#else
		if ( !( ( *aasworld ).areasettings[i].contents & AREACONTENTS_ROUTEPORTAL ) &&
			 !( ( *aasworld ).areasettings[i].contents & AREACONTENTS_CLUSTERPORTAL ) ) {
#endif // RTCW_XX

			continue;
		}
		//if the area has no reachabilities
		if ( !AAS_AreaReachability( i ) ) {
			continue;
		}
		//tavel time from the area to the start area
		starttime = AAS_AreaTravelTimeToGoalArea( startareanum, start, i, travelflags );
		if ( !starttime ) {
			continue;
		}
		//if the travel time from the start to the area is greater than the shortest goal travel time

#if !defined RTCW_ET
		if ( starttime > 1.5 * goaltraveltime ) {
#else
		if ( starttime > 500 + 3.0 * goaltraveltime ) {
#endif // RTCW_XX

			continue;
		}
		//travel time from the area to the goal area
		goaltime = AAS_AreaTravelTimeToGoalArea( i, NULL, goalareanum, travelflags );
		if ( !goaltime ) {
			continue;
		}
		//if the travel time from the area to the goal is greater than the shortest goal travel time

#if !defined RTCW_ET
		if ( goaltime > 1.5 * goaltraveltime ) {
#else
		if ( goaltime > 500 + 3.0 * goaltraveltime ) {
#endif // RTCW_XX

			continue;
		}
		//this is a mid range area
		midrangeareas[i].valid = qtrue;
		midrangeareas[i].starttime = starttime;
		midrangeareas[i].goaltime = goaltime;
		Log_Write( "%d midrange area %d", nummidrangeareas, i );
		nummidrangeareas++;
	} //end for
	  //
	for ( i = 1; i < ( *aasworld ).numareas; i++ )
	{
		if ( !midrangeareas[i].valid ) {
			continue;
		}
		//get the areas in one cluster
		numclusterareas = 0;
		AAS_AltRoutingFloodCluster_r( i );
		//now we've got a cluster with areas through which an alternative route could go
		//get the 'center' of the cluster
		VectorClear( mid );
		for ( j = 0; j < numclusterareas; j++ )
		{
			VectorAdd( mid, ( *aasworld ).areas[clusterareas[j]].center, mid );
		} //end for
		VectorScale( mid, 1.0 / numclusterareas, mid );
		//get the area closest to the center of the cluster
		bestdist = 999999;
		bestareanum = 0;
		for ( j = 0; j < numclusterareas; j++ )
		{
			VectorSubtract( mid, ( *aasworld ).areas[clusterareas[j]].center, dir );
			dist = VectorLength( dir );
			if ( dist < bestdist ) {
				bestdist = dist;
				bestareanum = clusterareas[j];
			} //end if
		} //end for

#if !defined RTCW_ET
		  //now we've got an area for an alternative route
		  //FIXME: add alternative goal origin
		VectorCopy( ( *aasworld ).areas[bestareanum].center, altroutegoals[numaltroutegoals].origin );
#else
		  // make sure the route to the destination isn't in the same direction as the route to the source
		if ( !AAS_AreaRouteToGoalArea( bestareanum, ( *aasworld ).areawaypoints[bestareanum], goalareanum, travelflags, &time, &reachnum ) ) {
			continue;
		}
		a1 = ( *aasworld ).reachability[reachnum].areanum;
		if ( !AAS_AreaRouteToGoalArea( bestareanum, ( *aasworld ).areawaypoints[bestareanum], startareanum, travelflags, &time, &reachnum ) ) {
			continue;
		}
		a2 = ( *aasworld ).reachability[reachnum].areanum;
		if ( a1 == a2 ) {
			continue;
		}
		//now we've got an area for an alternative route
		//FIXME: add alternative goal origin
		VectorCopy( ( *aasworld ).areawaypoints[bestareanum], altroutegoals[numaltroutegoals].origin );
#endif // RTCW_XX

		altroutegoals[numaltroutegoals].areanum = bestareanum;
		altroutegoals[numaltroutegoals].starttraveltime = midrangeareas[bestareanum].starttime;
		altroutegoals[numaltroutegoals].goaltraveltime = midrangeareas[bestareanum].goaltime;
		altroutegoals[numaltroutegoals].extratraveltime =
			( midrangeareas[bestareanum].starttime + midrangeareas[bestareanum].goaltime ) -
			goaltraveltime;
		numaltroutegoals++;
		//

#if !defined RTCW_ET
#ifdef DEBUG
		botimport.Print( PRT_MESSAGE, "alternative route goal area %d, numclusterareas = %d\n", bestareanum, numclusterareas );
		if ( color ) {
			AAS_DrawPermanentCross( ( *aasworld ).areas[bestareanum].center, 10, color );
		} //end if
		  //AAS_ShowArea(bestarea, qtrue);
#endif
#else
/*#ifdef DEBUG
		botimport.Print(PRT_MESSAGE, "alternative route goal area %d, numclusterareas = %d\n", bestareanum, numclusterareas);
		if (color)
		{
			AAS_DrawPermanentCross((*aasworld).areas[bestareanum].center, 10, color);
		} //end if
		//AAS_ShowArea(bestarea, qtrue);
#endif*/
#endif // RTCW_XX

		//don't return more than the maximum alternative route goals
		if ( numaltroutegoals >= maxaltroutegoals ) {
			break;
		}
	} //end for

#if !defined RTCW_ET
	botimport.Print( PRT_MESSAGE, "%d alternative route goals\n", numaltroutegoals );
#ifdef DEBUG
	botimport.Print( PRT_MESSAGE, "alternative route goals in %d msec\n", Sys_MilliSeconds() - startmillisecs );
#endif
#else
	  //botimport.Print(PRT_MESSAGE, "%d alternative route goals\n", numaltroutegoals);
#ifdef DEBUG
//	botimport.Print(PRT_MESSAGE, "alternative route goals in %d msec\n", Sys_MilliSeconds() - startmillisecs);
#endif
#endif // RTCW_XX

	return numaltroutegoals;
#endif
} //end of the function AAS_AlternativeRouteGoals
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_InitAlternativeRouting( void ) {
#ifdef ENABLE_ALTROUTING
	if ( midrangeareas ) {
		FreeMemory( midrangeareas );
	}
	midrangeareas = (midrangearea_t *) GetMemory( ( *aasworld ).numareas * sizeof( midrangearea_t ) );
	if ( clusterareas ) {
		FreeMemory( clusterareas );
	}

#if !defined RTCW_ET
	clusterareas = (int *) GetMemory( aasworld.numareas * sizeof( int ) );
#else
	clusterareas = (int *) GetMemory( ( *aasworld ).numareas * sizeof( int ) );
#endif // RTCW_XX

#endif
} //end of the function AAS_InitAlternativeRouting
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShutdownAlternativeRouting( void ) {
#ifdef ENABLE_ALTROUTING
	if ( midrangeareas ) {
		FreeMemory( midrangeareas );
	}
	midrangeareas = NULL;
	if ( clusterareas ) {
		FreeMemory( clusterareas );
	}
	clusterareas = NULL;
	numclusterareas = 0;
#endif
}
