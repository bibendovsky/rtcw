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


/*****************************************************************************
 * name:		be_aas_route.c
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

//BBi
//BBi FIXME Take in account endianess
//BBi

//BBi
#include <algorithm>
#include <memory>
#include <vector>

#include "rtcw_endian.h"
//BBi

#include "q_shared.h"
#include "l_utils.h"
#include "l_memory.h"
#include "l_log.h"
#include "l_crc.h"
#include "l_libvar.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_interface.h"
#include "be_aas_def.h"

//BBi
namespace {


    const bool aasIs32 = (sizeof (size_t) == 4);
    const size_t AAS_RCS_SIZE = sizeof (aas_routingcache_t);
    const size_t AAS_RCS32_SIZE = sizeof (aas_routingcache_t::Struct32);
    const size_t AAS_RCS_DIFF = AAS_RCS_SIZE - AAS_RCS32_SIZE;
    const size_t AAS_RCS_TT_SIZE = AAS_RCS_SIZE - reinterpret_cast<size_t> (
        &static_cast<const aas_routingcache_t*> (0)->traveltimes);
    const size_t AAS_RCS_TT32_SIZE = AAS_RCS32_SIZE - reinterpret_cast<size_t> (
        &static_cast<const aas_routingcache_t::Struct32*> (0)->traveltimes);
    const size_t AAS_RCS_TT_DIFF = AAS_RCS_TT_SIZE - AAS_RCS_TT32_SIZE;

    std::vector<byte> aas_rcs_buffer;

    void aasWriteRcStruct (
        aas_routingcache_t* rcStruct,
        fileHandle_t fileHandle)
    {
        if (aasIs32)
            botimport.FS_Write (rcStruct, rcStruct->size, fileHandle);
        else {
            aas_rcs_buffer.resize (rcStruct->size);
            aas_routingcache_t::Struct32* struct32 =
                reinterpret_cast<aas_routingcache_t::Struct32*> (&aas_rcs_buffer[0]);
            rcStruct->convertTo32 (struct32);

            botimport.FS_Write (struct32, struct32->size, fileHandle);
        }
    }

} // namespace


// (static)
aas_routingcache_t::Struct* aas_routingcache_t::convertFrom32 (
    const Struct32* struct32)
{
    int newSize = struct32->size + (AAS_RCS_DIFF - AAS_RCS_TT_DIFF);

    Struct* result = static_cast<Struct*> (GetClearedMemory (newSize));

    const Struct32& src = *struct32;
    Struct& dst = *result;

    dst.size = newSize;
    dst.time = rtcw::Endian::le(src.time);
    dst.cluster = rtcw::Endian::le(src.cluster);
    dst.areanum = rtcw::Endian::le(src.areanum);
    rtcw::Endian::le(src.origin, dst.origin);
    dst.starttraveltime = rtcw::Endian::le(src.starttraveltime);
    dst.travelflags = rtcw::Endian::le(src.travelflags);
    dst.prev = 0;
    dst.next = 0;

    dst.reachabilities = reinterpret_cast<byte*> (result) +
        AAS_RCS_SIZE - AAS_RCS_TT_DIFF + (((src.size - AAS_RCS32_SIZE) / 3) * 2);

    int extraSize = src.size - AAS_RCS32_SIZE + AAS_RCS_TT32_SIZE;

    std::uninitialized_copy(
        reinterpret_cast<const byte*> (&src.traveltimes),
        reinterpret_cast<const byte*> (&src.traveltimes) + extraSize,
        reinterpret_cast<byte*> (&dst.traveltimes));

    if (!rtcw::Endian::is_little()) {
        int tt_count = (dst.reachabilities - reinterpret_cast<const unsigned char*>(&dst.traveltimes)) / 2;

        rtcw::Endian::le(src.traveltimes, tt_count, dst.traveltimes);
    }

    return result;
}

void aas_routingcache_t::convertTo32 (
    aas_routingcache_t::Struct32* struct32) const
{
    int oldSize = size - AAS_RCS_DIFF;

    const Struct& src = *this;
    Struct32& dst = *struct32;

    dst.size = rtcw::Endian::le(oldSize);
    dst.time = rtcw::Endian::le(src.time);
    dst.cluster = rtcw::Endian::le(src.cluster);
    dst.areanum = rtcw::Endian::le(src.areanum);
    rtcw::Endian::le(src.origin, dst.origin);
    dst.starttraveltime = rtcw::Endian::le(src.starttraveltime);
    dst.travelflags = rtcw::Endian::le(src.travelflags);
    dst.prev = 0;
    dst.next = 0;
    dst.reachabilities = 0;

    int extraSize = oldSize - AAS_RCS32_SIZE + AAS_RCS_TT32_SIZE;

    std::uninitialized_copy(
        reinterpret_cast<const byte*> (&src.traveltimes),
        reinterpret_cast<const byte*> (&src.traveltimes) + extraSize,
        reinterpret_cast<byte*> (&dst.traveltimes));

    if (!rtcw::Endian::is_little()) {
        int tt_count = (src.reachabilities - reinterpret_cast<const unsigned char*>(&src.traveltimes)) / 2;

        rtcw::Endian::le(src.traveltimes, tt_count, dst.traveltimes);
    }
}
//BBi

#define ROUTING_DEBUG

//travel time in hundreths of a second = distance * 100 / speed
#define DISTANCEFACTOR_CROUCH       1.3     //crouch speed = 100
#define DISTANCEFACTOR_SWIM         1       //should be 0.66, swim speed = 150
#define DISTANCEFACTOR_WALK         0.33    //walk speed = 300

// Ridah, scale traveltimes with ground steepness of area

#if !defined RTCW_ET
#define GROUNDSTEEPNESS_TIMESCALE   20  // this is the maximum scale, 1 being the usual for a flat ground
#else
#define GROUNDSTEEPNESS_TIMESCALE   1   // this is the maximum scale, 1 being the usual for a flat ground
#endif // RTCW_XX

//cache refresh time
#define CACHE_REFRESHTIME       15.0    //15 seconds refresh time

#if !defined RTCW_ET
//maximum number of routing updates each frame
#define MAX_FRAMEROUTINGUPDATES     100
#else
#define DEFAULT_MAX_ROUTINGCACHESIZE        "16384"
#endif // RTCW_XX

#if !defined RTCW_MP
extern aas_t aasworlds[MAX_AAS_WORLDS];
#endif // RTCW_XX


#if !defined RTCW_ET
/*

  area routing cache:
  stores the distances within one cluster to a specific goal area
  this goal area is in this same cluster and could be a cluster portal
  for every cluster there's a list with routing cache for every area
  in that cluster (including the portals of that cluster)
  area cache stores (*aasworld).clusters[?].numreachabilityareas travel times

  portal routing cache:
  stores the distances of all portals to a specific goal area
  this goal area could be in any cluster and could also be a cluster portal
  for every area ((*aasworld).numareas) the portal cache stores
  (*aasworld).numportals travel times

*/
#else
/*

  area routing cache:
  stores the distances within one cluster to a specific goal area
  this goal area is in this same cluster and could be a cluster portal
  for every cluster there's a list with routing cache for every area
  in that cluster (including the portals of that cluster)
  area cache stores aasworld->clusters[?].numreachabilityareas travel times

  portal routing cache:
  stores the distances of all portals to a specific goal area
  this goal area could be in any cluster and could also be a cluster portal
  for every area (aasworld->numareas) the portal cache stores
  aasworld->numportals travel times

*/
#endif // RTCW_XX

#ifdef ROUTING_DEBUG
int numareacacheupdates;
int numportalcacheupdates;
#endif //ROUTING_DEBUG

int routingcachesize;
int max_routingcachesize;

#if defined RTCW_ET
int max_frameroutingupdates;
#endif // RTCW_XX

// Ridah, routing memory calls go here, so we can change between Hunk/Zone easily
void *AAS_RoutingGetMemory( int size ) {
	return GetClearedMemory( size );
}

void AAS_RoutingFreeMemory( void *ptr ) {
	FreeMemory( ptr );
}
// done.

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef ROUTING_DEBUG
void AAS_RoutingInfo( void ) {
	botimport.Print( PRT_MESSAGE, "%d area cache updates\n", numareacacheupdates );
	botimport.Print( PRT_MESSAGE, "%d portal cache updates\n", numportalcacheupdates );
	botimport.Print( PRT_MESSAGE, "%d bytes routing cache\n", routingcachesize );
} //end of the function AAS_RoutingInfo
#endif //ROUTING_DEBUG
//===========================================================================
// returns the number of the area in the cluster
// assumes the given area is in the given cluster or a portal of the cluster
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_ClusterAreaNum( int cluster, int areanum ) {
	int side, areacluster;

#if !defined RTCW_ET
	areacluster = ( *aasworld ).areasettings[areanum].cluster;
	if ( areacluster > 0 ) {
		return ( *aasworld ).areasettings[areanum].clusterareanum;
	} else
	{
/*#ifdef ROUTING_DEBUG
		if ((*aasworld).portals[-areacluster].frontcluster != cluster &&
				(*aasworld).portals[-areacluster].backcluster != cluster)
		{
			botimport.Print(PRT_ERROR, "portal %d: does not belong to cluster %d\n"
											, -areacluster, cluster);
		} //end if
#endif //ROUTING_DEBUG*/
		side = ( *aasworld ).portals[-areacluster].frontcluster != cluster;
		return ( *aasworld ).portals[-areacluster].clusterareanum[side];
#else
	areacluster = aasworld->areasettings[areanum].cluster;
	if ( areacluster > 0 ) {
		return aasworld->areasettings[areanum].clusterareanum;
	} else
	{
/*#ifdef ROUTING_DEBUG
		if (aasworld->portals[-areacluster].frontcluster != cluster &&
				aasworld->portals[-areacluster].backcluster != cluster)
		{
			botimport.Print(PRT_ERROR, "portal %d: does not belong to cluster %d\n"
											, -areacluster, cluster);
		} //end if
#endif //ROUTING_DEBUG*/
		side = aasworld->portals[-areacluster].frontcluster != cluster;
		return aasworld->portals[-areacluster].clusterareanum[side];
#endif // RTCW_XX

	} //end else
} //end of the function AAS_ClusterAreaNum
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_InitTravelFlagFromType( void ) {
	int i;

	for ( i = 0; i < MAX_TRAVELTYPES; i++ )
	{

#if !defined RTCW_ET
		( *aasworld ).travelflagfortype[i] = TFL_INVALID;
	} //end for
	( *aasworld ).travelflagfortype[TRAVEL_INVALID] = TFL_INVALID;
	( *aasworld ).travelflagfortype[TRAVEL_WALK] = TFL_WALK;
	( *aasworld ).travelflagfortype[TRAVEL_CROUCH] = TFL_CROUCH;
	( *aasworld ).travelflagfortype[TRAVEL_BARRIERJUMP] = TFL_BARRIERJUMP;
	( *aasworld ).travelflagfortype[TRAVEL_JUMP] = TFL_JUMP;
	( *aasworld ).travelflagfortype[TRAVEL_LADDER] = TFL_LADDER;
	( *aasworld ).travelflagfortype[TRAVEL_WALKOFFLEDGE] = TFL_WALKOFFLEDGE;
	( *aasworld ).travelflagfortype[TRAVEL_SWIM] = TFL_SWIM;
	( *aasworld ).travelflagfortype[TRAVEL_WATERJUMP] = TFL_WATERJUMP;
	( *aasworld ).travelflagfortype[TRAVEL_TELEPORT] = TFL_TELEPORT;
	( *aasworld ).travelflagfortype[TRAVEL_ELEVATOR] = TFL_ELEVATOR;
	( *aasworld ).travelflagfortype[TRAVEL_ROCKETJUMP] = TFL_ROCKETJUMP;
	( *aasworld ).travelflagfortype[TRAVEL_BFGJUMP] = TFL_BFGJUMP;
	( *aasworld ).travelflagfortype[TRAVEL_GRAPPLEHOOK] = TFL_GRAPPLEHOOK;
	( *aasworld ).travelflagfortype[TRAVEL_DOUBLEJUMP] = TFL_DOUBLEJUMP;
	( *aasworld ).travelflagfortype[TRAVEL_RAMPJUMP] = TFL_RAMPJUMP;
	( *aasworld ).travelflagfortype[TRAVEL_STRAFEJUMP] = TFL_STRAFEJUMP;
	( *aasworld ).travelflagfortype[TRAVEL_JUMPPAD] = TFL_JUMPPAD;
	( *aasworld ).travelflagfortype[TRAVEL_FUNCBOB] = TFL_FUNCBOB;
#else
		aasworld->travelflagfortype[i] = TFL_INVALID;
	} //end for
	aasworld->travelflagfortype[TRAVEL_INVALID] = TFL_INVALID;
	aasworld->travelflagfortype[TRAVEL_WALK] = TFL_WALK;
	aasworld->travelflagfortype[TRAVEL_CROUCH] = TFL_CROUCH;
	aasworld->travelflagfortype[TRAVEL_BARRIERJUMP] = TFL_BARRIERJUMP;
	aasworld->travelflagfortype[TRAVEL_JUMP] = TFL_JUMP;
	aasworld->travelflagfortype[TRAVEL_LADDER] = TFL_LADDER;
	aasworld->travelflagfortype[TRAVEL_WALKOFFLEDGE] = TFL_WALKOFFLEDGE;
	aasworld->travelflagfortype[TRAVEL_SWIM] = TFL_SWIM;
	aasworld->travelflagfortype[TRAVEL_WATERJUMP] = TFL_WATERJUMP;
	aasworld->travelflagfortype[TRAVEL_TELEPORT] = TFL_TELEPORT;
	aasworld->travelflagfortype[TRAVEL_ELEVATOR] = TFL_ELEVATOR;
	aasworld->travelflagfortype[TRAVEL_ROCKETJUMP] = TFL_ROCKETJUMP;
	aasworld->travelflagfortype[TRAVEL_BFGJUMP] = TFL_BFGJUMP;
	aasworld->travelflagfortype[TRAVEL_GRAPPLEHOOK] = TFL_GRAPPLEHOOK;
	aasworld->travelflagfortype[TRAVEL_DOUBLEJUMP] = TFL_DOUBLEJUMP;
	aasworld->travelflagfortype[TRAVEL_RAMPJUMP] = TFL_RAMPJUMP;
	aasworld->travelflagfortype[TRAVEL_STRAFEJUMP] = TFL_STRAFEJUMP;
	aasworld->travelflagfortype[TRAVEL_JUMPPAD] = TFL_JUMPPAD;
	aasworld->travelflagfortype[TRAVEL_FUNCBOB] = TFL_FUNCBOB;
#endif // RTCW_XX

} //end of the function AAS_InitTravelFlagFromType
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_TravelFlagForType( int traveltype ) {
	if ( traveltype < 0 || traveltype >= MAX_TRAVELTYPES ) {
		return TFL_INVALID;
	}

#if !defined RTCW_ET
	return ( *aasworld ).travelflagfortype[traveltype];
#else
	return aasworld->travelflagfortype[traveltype];
#endif // RTCW_XX

} //end of the function AAS_TravelFlagForType
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
float AAS_RoutingTime( void ) {
	return AAS_Time();
} //end of the function AAS_RoutingTime
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_FreeRoutingCache( aas_routingcache_t *cache ) {
	routingcachesize -= cache->size;
	AAS_RoutingFreeMemory( cache );
} //end of the function AAS_FreeRoutingCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_RemoveRoutingCacheInCluster( int clusternum ) {
	int i;
	aas_routingcache_t *cache, *nextcache;
	aas_cluster_t *cluster;

#if !defined RTCW_ET
	if ( !( *aasworld ).clusterareacache ) {
		return;
	}
	cluster = &( *aasworld ).clusters[clusternum];
	for ( i = 0; i < cluster->numareas; i++ )
	{
		for ( cache = ( *aasworld ).clusterareacache[clusternum][i]; cache; cache = nextcache )
		{
			nextcache = cache->next;
			AAS_FreeRoutingCache( cache );
		} //end for
		( *aasworld ).clusterareacache[clusternum][i] = NULL;
#else
	if ( !aasworld->clusterareacache ) {
		return;
	}
	cluster = &aasworld->clusters[clusternum];
	for ( i = 0; i < cluster->numareas; i++ )
	{
		for ( cache = aasworld->clusterareacache[clusternum][i]; cache; cache = nextcache )
		{
			nextcache = cache->next;
			AAS_FreeRoutingCache( cache );
		} //end for
		aasworld->clusterareacache[clusternum][i] = NULL;
#endif // RTCW_XX

	} //end for
} //end of the function AAS_RemoveRoutingCacheInCluster
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_RemoveRoutingCacheUsingArea( int areanum ) {
	int i, clusternum;
	aas_routingcache_t *cache, *nextcache;

#if !defined RTCW_ET
	clusternum = ( *aasworld ).areasettings[areanum].cluster;
#else
	clusternum = aasworld->areasettings[areanum].cluster;
#endif // RTCW_XX

	if ( clusternum > 0 ) {
		//remove all the cache in the cluster the area is in
		AAS_RemoveRoutingCacheInCluster( clusternum );
	} //end if
	else
	{
		// if this is a portal remove all cache in both the front and back cluster

#if !defined RTCW_ET
		AAS_RemoveRoutingCacheInCluster( ( *aasworld ).portals[-clusternum].frontcluster );
		AAS_RemoveRoutingCacheInCluster( ( *aasworld ).portals[-clusternum].backcluster );
	} //end else
	  // remove all portal cache
	if ( ( *aasworld ).portalcache ) {
		for ( i = 0; i < ( *aasworld ).numareas; i++ )
		{
			//refresh portal cache
			for ( cache = ( *aasworld ).portalcache[i]; cache; cache = nextcache )
			{
				nextcache = cache->next;
				AAS_FreeRoutingCache( cache );
			} //end for
			( *aasworld ).portalcache[i] = NULL;
#else
		AAS_RemoveRoutingCacheInCluster( aasworld->portals[-clusternum].frontcluster );
		AAS_RemoveRoutingCacheInCluster( aasworld->portals[-clusternum].backcluster );
	} //end else
	  // remove all portal cache
	if ( aasworld->portalcache ) {
		for ( i = 0; i < aasworld->numareas; i++ )
		{
			//refresh portal cache
			for ( cache = aasworld->portalcache[i]; cache; cache = nextcache )
			{
				nextcache = cache->next;
				AAS_FreeRoutingCache( cache );
			} //end for
			aasworld->portalcache[i] = NULL;
#endif // RTCW_XX

		} //end for
	}
} //end of the function AAS_RemoveRoutingCacheUsingArea

#if defined RTCW_ET
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_TeamTravelFlagsForAreaFlags( int areaflags ) {
	int travelflags = 0;
	//
	if ( areaflags & AREA_TEAM_FLAGS ) {
		if ( areaflags & AREA_TEAM_AXIS ) {
			travelflags |= TFL_TEAM_AXIS;
		}
		if ( areaflags & AREA_TEAM_ALLIES ) {
			travelflags |= TFL_TEAM_ALLIES;
		}
		if ( areaflags & AREA_TEAM_AXIS_DISGUISED ) {
			travelflags |= TFL_TEAM_AXIS_DISGUISED;
		}
		if ( areaflags & AREA_TEAM_ALLIES_DISGUISED ) {
			travelflags |= TFL_TEAM_AXIS_DISGUISED;
		}
		if ( areaflags & AREA_AVOID_AXIS ) {
			travelflags |= TFL_TEAM_AXIS;
		}
		if ( areaflags & AREA_AVOID_ALLIES ) {
			travelflags |= TFL_TEAM_ALLIES;
		}
	}
	//
	return travelflags;
}
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_ClearClusterTeamFlags( int areanum ) {
	int clusternum;
	//
	clusternum = aasworld->areasettings[areanum].cluster;
	if ( clusternum > 0 ) {
		aasworld->clusterTeamTravelFlags[clusternum] = -1;  // recalculate
	}
}
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_CalculateClusterTeamFlags( int clusternum ) {
	int i;
	//
	if ( clusternum < 0 ) {
		return;
	}
	//
	aasworld->clusterTeamTravelFlags[clusternum] = 0;
	for ( i = 1; i < aasworld->numareas; i++ ) {
		if ( aasworld->areasettings[i].cluster == clusternum ) {
			aasworld->clusterTeamTravelFlags[clusternum] |= AAS_TeamTravelFlagsForAreaFlags( aasworld->areasettings[i].areaflags );
		}
	}
}
#endif // RTCW_XX

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_EnableRoutingArea( int areanum, int enable ) {
	int flags;

#if !defined RTCW_ET

	if ( areanum <= 0 || areanum >= ( *aasworld ).numareas ) {
		if ( bot_developer ) {
			botimport.Print( PRT_ERROR, "AAS_EnableRoutingArea: areanum %d out of range\n", areanum );
		} //end if
		return 0;
	} //end if
	flags = ( *aasworld ).areasettings[areanum].areaflags & AREA_DISABLED;
	if ( enable < 0 ) {
		return !flags;
	}

	if ( enable ) {
		( *aasworld ).areasettings[areanum].areaflags &= ~AREA_DISABLED;
	} else {
		( *aasworld ).areasettings[areanum].areaflags |= AREA_DISABLED;
	}
	// if the status of the area changed
	if ( ( flags & AREA_DISABLED ) != ( ( *aasworld ).areasettings[areanum].areaflags & AREA_DISABLED ) ) {
		//remove all routing cache involving this area
		AAS_RemoveRoutingCacheUsingArea( areanum );
#else
	int bitflag;    // flag to set or clear

	if ( areanum <= 0 || areanum >= aasworld->numareas ) {
		if ( bot_developer ) {
			botimport.Print( PRT_ERROR, "AAS_EnableRoutingArea: areanum %d out of range\n", areanum );
		} //end if
		return 0;
	} //end if

	if ( ( enable & 1 ) || ( enable < 0 ) ) {
		// clear all flags
		bitflag = AREA_AVOID | AREA_DISABLED | AREA_TEAM_AXIS | AREA_TEAM_ALLIES | AREA_TEAM_AXIS_DISGUISED | AREA_TEAM_ALLIES_DISGUISED;
	} else if ( enable & 0x10 ) {
		bitflag = AREA_AVOID;
	} else if ( enable & 0x20 ) {
		bitflag = AREA_TEAM_AXIS;
	} else if ( enable & 0x40 ) {
		bitflag = AREA_TEAM_ALLIES;
	} else if ( enable & 0x80 ) {
		bitflag = AREA_TEAM_AXIS_DISGUISED;
	} else if ( enable & 0x100 ) {
		bitflag = AREA_TEAM_ALLIES_DISGUISED;
	} else {
		bitflag = AREA_DISABLED;
	}

	// remove avoidance flag
	enable &= 1;

	flags = aasworld->areasettings[areanum].areaflags & bitflag;
	if ( enable < 0 ) {
		return !flags;
	}

	if ( enable ) {
		aasworld->areasettings[areanum].areaflags &= ~bitflag;
	} else {
		aasworld->areasettings[areanum].areaflags |= bitflag;
	}

	// if the status of the area changed
	if ( ( flags & bitflag ) != ( aasworld->areasettings[areanum].areaflags & bitflag ) ) {
		//remove all routing cache involving this area
		AAS_RemoveRoutingCacheUsingArea( areanum );
		// recalculate the team flags that are used in this cluster
		AAS_ClearClusterTeamFlags( areanum );
#endif // RTCW_X

	} //end if
	return !flags;
} //end of the function AAS_EnableRoutingArea
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================

#if defined RTCW_ET
void AAS_EnableAllAreas( void ) {
	int i;
	for ( i = 0; i < ( *aasworld ).numareas; i++ ) {
		if ( ( *aasworld ).areasettings[i].areaflags & AREA_DISABLED ) {
			AAS_EnableRoutingArea( i, qtrue );
		}
	}
}
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
#endif // RTCW_XX

void AAS_CreateReversedReachability( void ) {
	int i, n;
	aas_reversedlink_t *revlink;
	aas_reachability_t *reach;
	aas_areasettings_t *settings;
	char *ptr;
#ifdef DEBUG
	int starttime;

	starttime = Sys_MilliSeconds();
#endif
	//free reversed links that have already been created

#if !defined RTCW_ET
	if ( ( *aasworld ).reversedreachability ) {
		AAS_RoutingFreeMemory( ( *aasworld ).reversedreachability );
	}
	//allocate memory for the reversed reachability links
	ptr = (char *) AAS_RoutingGetMemory( ( *aasworld ).numareas * sizeof( aas_reversedreachability_t ) +
										 ( *aasworld ).reachabilitysize * sizeof( aas_reversedlink_t ) );
	//
	( *aasworld ).reversedreachability = (aas_reversedreachability_t *) ptr;
	//pointer to the memory for the reversed links
	ptr += ( *aasworld ).numareas * sizeof( aas_reversedreachability_t );
	//check all other areas for reachability links to the area
	for ( i = 1; i < ( *aasworld ).numareas; i++ )
	{
		//settings of the area
		settings = &( *aasworld ).areasettings[i];
		//check the reachability links
		for ( n = 0; n < settings->numreachableareas; n++ )
		{
			//reachability link
			reach = &( *aasworld ).reachability[settings->firstreachablearea + n];
			//
			revlink = (aas_reversedlink_t *) ptr;
			ptr += sizeof( aas_reversedlink_t );
			//
			revlink->areanum = i;
			revlink->linknum = settings->firstreachablearea + n;
			revlink->next = ( *aasworld ).reversedreachability[reach->areanum].first;
			( *aasworld ).reversedreachability[reach->areanum].first = revlink;
			( *aasworld ).reversedreachability[reach->areanum].numlinks++;
#else
	if ( aasworld->reversedreachability ) {
		AAS_RoutingFreeMemory( aasworld->reversedreachability );
	}
	//allocate memory for the reversed reachability links
	ptr = (char *) AAS_RoutingGetMemory( aasworld->numareas * sizeof( aas_reversedreachability_t ) + aasworld->reachabilitysize * sizeof( aas_reversedlink_t ) );

	aasworld->reversedreachability = (aas_reversedreachability_t *) ptr;
	//pointer to the memory for the reversed links
	ptr += aasworld->numareas * sizeof( aas_reversedreachability_t );
	//check all other areas for reachability links to the area
	for ( i = 1; i < aasworld->numareas; i++ )
	{
		//settings of the area
		settings = &( aasworld->areasettings[i] );
		//check the reachability links
		for ( n = 0; n < settings->numreachableareas; n++ )
		{
			// Gordon: Temp hack for b0rked last area in
			if ( settings->firstreachablearea < 0 || settings->firstreachablearea >= ( *aasworld ).reachabilitysize ) {
				Com_Printf( "^1WARNING: settings->firstreachablearea out of range\n" );
				continue;
			}

			//reachability link
			reach = &aasworld->reachability[settings->firstreachablearea + n];

			if ( ( reach->areanum < 0 || ( reach->areanum >= aasworld->reachabilitysize ) ) ) {
				Com_Printf( "^1WARNING: reach->areanum out of range\n" );
				continue;
			}

			revlink = (aas_reversedlink_t *) ptr;
			ptr += sizeof( aas_reversedlink_t );
			//
			revlink->areanum = i;
			revlink->linknum = settings->firstreachablearea + n;
			revlink->next = aasworld->reversedreachability[reach->areanum].first;
			aasworld->reversedreachability[reach->areanum].first = revlink;
			aasworld->reversedreachability[reach->areanum].numlinks++;
#endif // RTCW_XX

		} //end for
	} //end for
#ifdef DEBUG
	botimport.Print( PRT_MESSAGE, "reversed reachability %d msec\n", Sys_MilliSeconds() - starttime );
#endif //DEBUG
} //end of the function AAS_CreateReversedReachability
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================

#if defined RTCW_ET
// Gordon: always returns 1, so er?...
#endif // RTCW_XX
float AAS_AreaGroundSteepnessScale( int areanum ) {

#if !defined RTCW_ET
	return ( 1.0 + ( *aasworld ).areasettings[areanum].groundsteepness * (float)( GROUNDSTEEPNESS_TIMESCALE - 1 ) );
#else
	return ( 1.0 + aasworld->areasettings[areanum].groundsteepness * (float)( GROUNDSTEEPNESS_TIMESCALE - 1 ) );
#endif // RTCW_XX

}
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
unsigned short int AAS_AreaTravelTime( int areanum, vec3_t start, vec3_t end ) {
	int intdist;
	float dist;

#if !defined RTCW_ET
	vec3_t dir;

	VectorSubtract( start, end, dir );
	dist = VectorLength( dir );
	// Ridah, factor in the groundsteepness now
	dist *= AAS_AreaGroundSteepnessScale( areanum );

	//if crouch only area
	if ( AAS_AreaCrouch( areanum ) ) {
		dist *= DISTANCEFACTOR_CROUCH;
	}
	//if swim area
	else if ( AAS_AreaSwim( areanum ) ) {
		dist *= DISTANCEFACTOR_SWIM;
	}
	//normal walk area
	else {dist *= DISTANCEFACTOR_WALK;}
	//

#if defined RTCW_SP
	intdist = (int) c::ceil( dist );
#elif defined RTCW_MP
	intdist = (int) dist;
#endif // RTCW_XX

	//make sure the distance isn't zero
	if ( intdist <= 0 ) {
		intdist = 1;
	}
#else
	// Ridah, factor in the groundsteepness now
	dist = VectorDistance( start, end ); // * AAS_AreaGroundSteepnessScale(areanum); // Gordon: useless as it returns 1 all the time...

	if ( AAS_AreaCrouch( areanum ) ) {
		dist *= DISTANCEFACTOR_CROUCH; //if crouch only area
/*	} else if( AAS_AreaSwim(areanum)) { // Gordon: again, uselss as it's a multiply by 1
		dist *= DISTANCEFACTOR_SWIM; //if swim area */
	} else {
		dist *= DISTANCEFACTOR_WALK; //normal walk area
	}

	intdist = myftol( dist );

	//make sure the distance isn't zero
	if ( intdist <= 0 ) {
		return 1;
	}
#endif // RTCW_XX

	return intdist;
} //end of the function AAS_AreaTravelTime
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_CalculateAreaTravelTimes( void ) {
	int i, l, n, size;
	char *ptr;
	vec3_t end;
	aas_reversedreachability_t *revreach;
	aas_reversedlink_t *revlink;
	aas_reachability_t *reach;
	aas_areasettings_t *settings;
	int starttime;

	starttime = Sys_MilliSeconds();
	//if there are still area travel times, free the memory

#if !defined RTCW_ET
	if ( ( *aasworld ).areatraveltimes ) {
		AAS_RoutingFreeMemory( ( *aasworld ).areatraveltimes );
	}
	//get the total size of all the area travel times
	size = ( *aasworld ).numareas * sizeof( unsigned short ** );
	for ( i = 0; i < ( *aasworld ).numareas; i++ )
	{
		revreach = &( *aasworld ).reversedreachability[i];
		//settings of the area
		settings = &( *aasworld ).areasettings[i];
		//
		size += settings->numreachableareas * sizeof( unsigned short * );
		//
		size += settings->numreachableareas * revreach->numlinks * sizeof( unsigned short );
	} //end for
	  //allocate memory for the area travel times
	ptr = (char *) AAS_RoutingGetMemory( size );
	( *aasworld ).areatraveltimes = (unsigned short ***) ptr;
	ptr += ( *aasworld ).numareas * sizeof( unsigned short ** );
	//calcluate the travel times for all the areas
	for ( i = 0; i < ( *aasworld ).numareas; i++ )
	{
		//reversed reachabilities of this area
		revreach = &( *aasworld ).reversedreachability[i];
		//settings of the area
		settings = &( *aasworld ).areasettings[i];
		//
		( *aasworld ).areatraveltimes[i] = (unsigned short **) ptr;
		ptr += settings->numreachableareas * sizeof( unsigned short * );
		//
		reach = &( *aasworld ).reachability[settings->firstreachablearea];
		for ( l = 0; l < settings->numreachableareas; l++, reach++ )
		{
			( *aasworld ).areatraveltimes[i][l] = (unsigned short *) ptr;
			ptr += revreach->numlinks * sizeof( unsigned short );
			//reachability link
			//
			for ( n = 0, revlink = revreach->first; revlink; revlink = revlink->next, n++ )
			{
				VectorCopy( ( *aasworld ).reachability[revlink->linknum].end, end );
				//
				( *aasworld ).areatraveltimes[i][l][n] = AAS_AreaTravelTime( i, end, reach->start );
			} //end for
		} //end for
#else
	if ( aasworld->areatraveltimes ) {
		AAS_RoutingFreeMemory( aasworld->areatraveltimes );
	}
	//get the total size of all the area travel times
	size = aasworld->numareas * sizeof( unsigned short ** );
	for ( i = 0; i < aasworld->numareas; i++ )
	{
		revreach = &aasworld->reversedreachability[i];
		//settings of the area
		settings = &aasworld->areasettings[i];
		//
		size += settings->numreachableareas * sizeof( unsigned short * );
		//
		size += settings->numreachableareas * revreach->numlinks * sizeof( unsigned short );
	} //end for
	  //allocate memory for the area travel times
	ptr = (char *) AAS_RoutingGetMemory( size );
	aasworld->areatraveltimes = (unsigned short ***) ptr;
	ptr += aasworld->numareas * sizeof( unsigned short ** );
	//calcluate the travel times for all the areas
	for ( i = 0; i < aasworld->numareas; i++ )
	{
		//reversed reachabilities of this area
		revreach = &aasworld->reversedreachability[i];
		//settings of the area
		settings = &aasworld->areasettings[i];
		//
		if ( settings->numreachableareas ) {
			aasworld->areatraveltimes[i] = (unsigned short **) ptr;
			ptr += settings->numreachableareas * sizeof( unsigned short * );
			//
			reach = &aasworld->reachability[settings->firstreachablearea];
			for ( l = 0; l < settings->numreachableareas; l++, reach++ )
			{
				aasworld->areatraveltimes[i][l] = (unsigned short *) ptr;
				ptr += revreach->numlinks * sizeof( unsigned short );
				//reachability link
				//
				for ( n = 0, revlink = revreach->first; revlink; revlink = revlink->next, n++ )
				{
					VectorCopy( aasworld->reachability[revlink->linknum].end, end );
					//
					aasworld->areatraveltimes[i][l][n] = AAS_AreaTravelTime( i, end, reach->start );
				} //end for
			} //end for
		}
#endif // RTCW_XX

	} //end for
#ifdef DEBUG
	botimport.Print( PRT_MESSAGE, "area travel times %d msec\n", Sys_MilliSeconds() - starttime );
#endif //DEBUG
} //end of the function AAS_CalculateAreaTravelTimes
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_PortalMaxTravelTime( int portalnum ) {
	int l, n, t, maxt;
	aas_portal_t *portal;
	aas_reversedreachability_t *revreach;
	aas_reversedlink_t *revlink;
	aas_areasettings_t *settings;

#if !defined RTCW_ET
	portal = &( *aasworld ).portals[portalnum];
	//reversed reachabilities of this portal area
	revreach = &( *aasworld ).reversedreachability[portal->areanum];
	//settings of the portal area
	settings = &( *aasworld ).areasettings[portal->areanum];
#else
	portal = &aasworld->portals[portalnum];
	//reversed reachabilities of this portal area
	revreach = &aasworld->reversedreachability[portal->areanum];
	//settings of the portal area
	settings = &aasworld->areasettings[portal->areanum];
#endif // RTCW_XX

	//
	maxt = 0;
	for ( l = 0; l < settings->numreachableareas; l++ )
	{
		for ( n = 0, revlink = revreach->first; revlink; revlink = revlink->next, n++ )
		{

#if !defined RTCW_ET
			t = ( *aasworld ).areatraveltimes[portal->areanum][l][n];
#else
			t = aasworld->areatraveltimes[portal->areanum][l][n];
#endif // RTCW_XX

			if ( t > maxt ) {
				maxt = t;
			} //end if
		} //end for
	} //end for
	return maxt;
} //end of the function AAS_PortalMaxTravelTime
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_InitPortalMaxTravelTimes( void ) {
	int i;

#if !defined RTCW_ET
	if ( ( *aasworld ).portalmaxtraveltimes ) {
		AAS_RoutingFreeMemory( ( *aasworld ).portalmaxtraveltimes );
	}

	( *aasworld ).portalmaxtraveltimes = (int *) AAS_RoutingGetMemory( ( *aasworld ).numportals * sizeof( int ) );

	for ( i = 0; i < ( *aasworld ).numportals; i++ )
	{
		( *aasworld ).portalmaxtraveltimes[i] = AAS_PortalMaxTravelTime( i );
		//botimport.Print(PRT_MESSAGE, "portal %d max tt = %d\n", i, (*aasworld).portalmaxtraveltimes[i]);
#else
	if ( aasworld->portalmaxtraveltimes ) {
		AAS_RoutingFreeMemory( aasworld->portalmaxtraveltimes );
	}

	aasworld->portalmaxtraveltimes = (int *) AAS_RoutingGetMemory( aasworld->numportals * sizeof( int ) );

	for ( i = 0; i < aasworld->numportals; i++ )
	{
		aasworld->portalmaxtraveltimes[i] = AAS_PortalMaxTravelTime( i );
		//botimport.Print(PRT_MESSAGE, "portal %d max tt = %d\n", i, aasworld->portalmaxtraveltimes[i]);
#endif // RTCW_XX

	} //end for
} //end of the function AAS_InitPortalMaxTravelTimes
/*
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_UnlinkCache(aas_routingcache_t *cache)
{
	if (cache->time_next) cache->time_next->time_prev = cache->time_prev;
	else newestcache = cache->time_prev;
	if (cache->time_prev) cache->time_prev->time_next = cache->time_next;
	else oldestcache = cache->time_next;
	cache->time_next = NULL;
	cache->time_prev = NULL;
} //end of the function AAS_UnlinkCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_LinkCache(aas_routingcache_t *cache)
{
	if (newestcache)
	{
		newestcache->time_next = cache;
		cache->time_prev = cache;
	} //end if
	else
	{
		oldestcache = cache;
		cache->time_prev = NULL;
	} //end else
	cache->time_next = NULL;
	newestcache = cache;
} //end of the function AAS_LinkCache*/
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_FreeOldestCache( void ) {
	int i, j, bestcluster, bestarea, freed;
	float besttime;
	aas_routingcache_t *cache, *bestcache;

	freed = qfalse;
	besttime = 999999999;
	bestcache = NULL;
	bestcluster = 0;
	bestarea = 0;
	//refresh cluster cache

#if !defined RTCW_ET
	for ( i = 0; i < ( *aasworld ).numclusters; i++ )
	{
		for ( j = 0; j < ( *aasworld ).clusters[i].numareas; j++ )
		{
			for ( cache = ( *aasworld ).clusterareacache[i][j]; cache; cache = cache->next )
			{
				//never remove cache leading towards a portal
				if ( ( *aasworld ).areasettings[cache->areanum].cluster < 0 ) {
#else
	for ( i = 0; i < aasworld->numclusters; i++ )
	{
		for ( j = 0; j < aasworld->clusters[i].numareas; j++ )
		{
			for ( cache = aasworld->clusterareacache[i][j]; cache; cache = cache->next )
			{
				//never remove cache leading towards a portal
				if ( aasworld->areasettings[cache->areanum].cluster < 0 ) {
#endif // RTCW_XX

					continue;
				}
				//if this cache is older than the cache we found so far
				if ( cache->time < besttime ) {
					bestcache = cache;
					bestcluster = i;
					bestarea = j;
					besttime = cache->time;
				} //end if
			} //end for
		} //end for
	} //end for
	if ( bestcache ) {
		cache = bestcache;
		if ( cache->prev ) {
			cache->prev->next = cache->next;

#if !defined RTCW_ET
		} else { ( *aasworld ).clusterareacache[bestcluster][bestarea] = cache->next;}
#else
		} else { aasworld->clusterareacache[bestcluster][bestarea] = cache->next;}
#endif // RTCW_XX

		if ( cache->next ) {
			cache->next->prev = cache->prev;
		}
		AAS_FreeRoutingCache( cache );
		freed = qtrue;
	} //end if
	besttime = 999999999;
	bestcache = NULL;
	bestarea = 0;

#if !defined RTCW_ET
	for ( i = 0; i < ( *aasworld ).numareas; i++ )
#else
	for ( i = 0; i < aasworld->numareas; i++ )
#endif // RTCW_XX

	{
		//refresh portal cache

#if !defined RTCW_ET
		for ( cache = ( *aasworld ).portalcache[i]; cache; cache = cache->next )
#else
		for ( cache = aasworld->portalcache[i]; cache; cache = cache->next )
#endif // RTCW_XX

		{
			if ( cache->time < besttime ) {
				bestcache = cache;
				bestarea = i;
				besttime = cache->time;
			} //end if
		} //end for
	} //end for
	if ( bestcache ) {
		cache = bestcache;
		if ( cache->prev ) {
			cache->prev->next = cache->next;

#if !defined RTCW_ET
		} else { ( *aasworld ).portalcache[bestarea] = cache->next;}
#else
		} else { aasworld->portalcache[bestarea] = cache->next;}
#endif // RTCW_XX

		if ( cache->next ) {
			cache->next->prev = cache->prev;
		}
		AAS_FreeRoutingCache( cache );
		freed = qtrue;
	} //end if
	return freed;
} //end of the function AAS_FreeOldestCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
aas_routingcache_t *AAS_AllocRoutingCache( int numtraveltimes ) {
	aas_routingcache_t *cache;
	int size;

#if !defined RTCW_ET
	//
	size = sizeof( aas_routingcache_t )
		   + numtraveltimes * sizeof( unsigned short int )
		   + numtraveltimes * sizeof( unsigned char );
	//
	routingcachesize += size;
	//
	cache = (aas_routingcache_t *) AAS_RoutingGetMemory( size );

    //BBi
	//cache->reachabilities = (unsigned char *) cache + sizeof( aas_routingcache_t )
	//						+ numtraveltimes * sizeof( unsigned short int );
    cache->reachabilities = reinterpret_cast<byte*> (cache) + sizeof (aas_routingcache_t) +
        (numtraveltimes * sizeof (unsigned short)) - AAS_RCS_TT_DIFF;

    //BBi
#else
	size = sizeof( aas_routingcache_t ) + numtraveltimes * sizeof( unsigned short int ) + numtraveltimes * sizeof( unsigned char );

	routingcachesize += size;

	cache = (aas_routingcache_t *) AAS_RoutingGetMemory( size );
	cache->reachabilities = (unsigned char *) cache + sizeof( aas_routingcache_t ) + numtraveltimes * sizeof( unsigned short int );
#endif // RTCW_XX

	cache->size = size;
	return cache;
} //end of the function AAS_AllocRoutingCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_FreeAllClusterAreaCache( void ) {
	int i, j;
	aas_routingcache_t *cache, *nextcache;
	aas_cluster_t *cluster;

	//free all cluster cache if existing

#if !defined RTCW_ET
	if ( !( *aasworld ).clusterareacache ) {
		return;
	}
	//free caches
	for ( i = 0; i < ( *aasworld ).numclusters; i++ )
	{
		cluster = &( *aasworld ).clusters[i];
		for ( j = 0; j < cluster->numareas; j++ )
		{
			for ( cache = ( *aasworld ).clusterareacache[i][j]; cache; cache = nextcache )
			{
				nextcache = cache->next;
				AAS_FreeRoutingCache( cache );
			} //end for
			( *aasworld ).clusterareacache[i][j] = NULL;
		} //end for
	} //end for
	  //free the cluster cache array
	AAS_RoutingFreeMemory( ( *aasworld ).clusterareacache );
	( *aasworld ).clusterareacache = NULL;
#else
	if ( !aasworld->clusterareacache ) {
		return;
	}
	//free caches
	for ( i = 0; i < aasworld->numclusters; i++ )
	{
		cluster = &aasworld->clusters[i];
		for ( j = 0; j < cluster->numareas; j++ )
		{
			for ( cache = aasworld->clusterareacache[i][j]; cache; cache = nextcache )
			{
				nextcache = cache->next;
				AAS_FreeRoutingCache( cache );
			} //end for
			aasworld->clusterareacache[i][j] = NULL;
		} //end for
	} //end for
	  //free the cluster cache array
	AAS_RoutingFreeMemory( aasworld->clusterareacache );
	aasworld->clusterareacache = NULL;
#endif // RTCW_XX

} //end of the function AAS_FreeAllClusterAreaCache
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_InitClusterAreaCache( void ) {
	int i, size;
	char *ptr;

	//

#if !defined RTCW_ET
	for ( size = 0, i = 0; i < ( *aasworld ).numclusters; i++ )
	{
		size += ( *aasworld ).clusters[i].numareas;
	} //end for
	  //two dimensional array with pointers for every cluster to routing cache
	  //for every area in that cluster
	ptr = (char *) AAS_RoutingGetMemory(
		( *aasworld ).numclusters * sizeof( aas_routingcache_t * * ) +
		size * sizeof( aas_routingcache_t * ) );
	( *aasworld ).clusterareacache = (aas_routingcache_t ***) ptr;
	ptr += ( *aasworld ).numclusters * sizeof( aas_routingcache_t * * );
	for ( i = 0; i < ( *aasworld ).numclusters; i++ )
	{
		( *aasworld ).clusterareacache[i] = (aas_routingcache_t **) ptr;
		ptr += ( *aasworld ).clusters[i].numareas * sizeof( aas_routingcache_t * );
#else
	for ( size = 0, i = 0; i < aasworld->numclusters; i++ )
	{
		size += aasworld->clusters[i].numareas;
	} //end for
	  //two dimensional array with pointers for every cluster to routing cache
	  //for every area in that cluster
	ptr = (char *) AAS_RoutingGetMemory(
		aasworld->numclusters * sizeof( aas_routingcache_t * * ) +
		size * sizeof( aas_routingcache_t * ) );
	aasworld->clusterareacache = (aas_routingcache_t ***) ptr;
	ptr += aasworld->numclusters * sizeof( aas_routingcache_t * * );
	for ( i = 0; i < aasworld->numclusters; i++ )
	{
		aasworld->clusterareacache[i] = (aas_routingcache_t **) ptr;
		ptr += aasworld->clusters[i].numareas * sizeof( aas_routingcache_t * );
#endif // RTCW_XX

	} //end for
} //end of the function AAS_InitClusterAreaCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_FreeAllPortalCache( void ) {
	int i;
	aas_routingcache_t *cache, *nextcache;

	//free all portal cache if existing

#if !defined RTCW_ET
	if ( !( *aasworld ).portalcache ) {
		return;
	}
	//free portal caches
	for ( i = 0; i < ( *aasworld ).numareas; i++ )
	{
		for ( cache = ( *aasworld ).portalcache[i]; cache; cache = nextcache )
		{
			nextcache = cache->next;
			AAS_FreeRoutingCache( cache );
		} //end for
		( *aasworld ).portalcache[i] = NULL;
	} //end for
	AAS_RoutingFreeMemory( ( *aasworld ).portalcache );
	( *aasworld ).portalcache = NULL;
#else
	if ( !aasworld->portalcache ) {
		return;
	}
	//free portal caches
	for ( i = 0; i < aasworld->numareas; i++ )
	{
		for ( cache = aasworld->portalcache[i]; cache; cache = nextcache )
		{
			nextcache = cache->next;
			AAS_FreeRoutingCache( cache );
		} //end for
		aasworld->portalcache[i] = NULL;
	} //end for
	AAS_RoutingFreeMemory( aasworld->portalcache );
	aasworld->portalcache = NULL;
#endif // RTCW_XX

} //end of the function AAS_FreeAllPortalCache
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_InitPortalCache( void ) {
	//

#if !defined RTCW_ET
	( *aasworld ).portalcache = (aas_routingcache_t **) AAS_RoutingGetMemory(
		( *aasworld ).numareas * sizeof( aas_routingcache_t * ) );
#else
	aasworld->portalcache = (aas_routingcache_t **) AAS_RoutingGetMemory(
		aasworld->numareas * sizeof( aas_routingcache_t * ) );
#endif // RTCW_XX

} //end of the function AAS_InitPortalCache
//
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_FreeAreaVisibility( void ) {
	int i;

#if !defined RTCW_ET
	if ( ( *aasworld ).areavisibility ) {
		for ( i = 0; i < ( *aasworld ).numareas; i++ )
		{
			if ( ( *aasworld ).areavisibility[i] ) {
				FreeMemory( ( *aasworld ).areavisibility[i] );
			}
		}
	}
	if ( ( *aasworld ).areavisibility ) {
		FreeMemory( ( *aasworld ).areavisibility );
	}
	( *aasworld ).areavisibility = NULL;
	if ( ( *aasworld ).decompressedvis ) {
		FreeMemory( ( *aasworld ).decompressedvis );
	}
	( *aasworld ).decompressedvis = NULL;
#else
	if ( aasworld->areavisibility ) {
		for ( i = 0; i < aasworld->numareas; i++ )
		{
			if ( aasworld->areavisibility[i] ) {
				FreeMemory( aasworld->areavisibility[i] );
			}
		}
	}
	if ( aasworld->areavisibility ) {
		FreeMemory( aasworld->areavisibility );
	}
	aasworld->areavisibility = NULL;
	if ( aasworld->decompressedvis ) {
		FreeMemory( aasworld->decompressedvis );
	}
	aasworld->decompressedvis = NULL;
#endif // RTCW_XX

}
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_InitRoutingUpdate( void ) {
//	int i, maxreachabilityareas;

	//free routing update fields if already existing

#if !defined RTCW_ET
	if ( ( *aasworld ).areaupdate ) {
		AAS_RoutingFreeMemory( ( *aasworld ).areaupdate );
	}
	//
// Ridah, had to change it to numareas for hidepos checking
/*
	maxreachabilityareas = 0;
	for (i = 0; i < (*aasworld).numclusters; i++)
	{
		if ((*aasworld).clusters[i].numreachabilityareas > maxreachabilityareas)
		{
			maxreachabilityareas = (*aasworld).clusters[i].numreachabilityareas;
		} //end if
	} //end for
	//allocate memory for the routing update fields
	(*aasworld).areaupdate = (aas_routingupdate_t *) AAS_RoutingGetMemory(
									maxreachabilityareas * sizeof(aas_routingupdate_t));
*/
	( *aasworld ).areaupdate = (aas_routingupdate_t *) AAS_RoutingGetMemory(
		( *aasworld ).numareas * sizeof( aas_routingupdate_t ) );
	//
	if ( ( *aasworld ).portalupdate ) {
		AAS_RoutingFreeMemory( ( *aasworld ).portalupdate );
	}
	//allocate memory for the portal update fields
	( *aasworld ).portalupdate = (aas_routingupdate_t *) AAS_RoutingGetMemory(
		( ( *aasworld ).numportals + 1 ) * sizeof( aas_routingupdate_t ) );
#else
	if ( aasworld->areaupdate ) {
		AAS_RoutingFreeMemory( aasworld->areaupdate );
	}

	aasworld->areaupdate = (aas_routingupdate_t*)AAS_RoutingGetMemory( aasworld->numareas * sizeof( aas_routingupdate_t ) );

	if ( aasworld->portalupdate ) {
		AAS_RoutingFreeMemory( aasworld->portalupdate );
	}
	//allocate memory for the portal update fields
	aasworld->portalupdate = (aas_routingupdate_t *) AAS_RoutingGetMemory( ( aasworld->numportals + 1 ) * sizeof( aas_routingupdate_t ) );
#endif // RTCW_XX

} //end of the function AAS_InitRoutingUpdate
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================

void AAS_CreateAllRoutingCache( void ) {
	int i, j, k, t, tfl, numroutingareas;
	aas_areasettings_t *areasettings;
	aas_reachability_t *reach;

	numroutingareas = 0;
	tfl = TFL_DEFAULT & ~( TFL_JUMPPAD | TFL_ROCKETJUMP | TFL_BFGJUMP | TFL_GRAPPLEHOOK | TFL_DOUBLEJUMP | TFL_RAMPJUMP | TFL_STRAFEJUMP | TFL_LAVA );  //----(SA)	modified since slime is no longer deadly
//	tfl = TFL_DEFAULT & ~(TFL_JUMPPAD|TFL_ROCKETJUMP|TFL_BFGJUMP|TFL_GRAPPLEHOOK|TFL_DOUBLEJUMP|TFL_RAMPJUMP|TFL_STRAFEJUMP|TFL_SLIME|TFL_LAVA);
	botimport.Print( PRT_MESSAGE, "AAS_CreateAllRoutingCache\n" );
	//

#if !defined RTCW_ET
	for ( i = 1; i < ( *aasworld ).numareas; i++ )
	{
		if ( !AAS_AreaReachability( i ) ) {
			continue;
		}
		areasettings = &( *aasworld ).areasettings[i];
		for ( k = 0; k < areasettings->numreachableareas; k++ )
		{
			reach = &( *aasworld ).reachability[areasettings->firstreachablearea + k];
			if ( ( *aasworld ).travelflagfortype[reach->traveltype] & tfl ) {
				break;
			}
		}
		if ( k >= areasettings->numreachableareas ) {
			continue;
		}
		( *aasworld ).areasettings[i].areaflags |= AREA_USEFORROUTING;
		numroutingareas++;
	}
	for ( i = 1; i < ( *aasworld ).numareas; i++ )
	{
		if ( !( ( *aasworld ).areasettings[i].areaflags & AREA_USEFORROUTING ) ) {
			continue;
		}
		for ( j = 1; j < ( *aasworld ).numareas; j++ )
		{
			if ( i == j ) {
				continue;
			}
			if ( !( ( *aasworld ).areasettings[j].areaflags & AREA_USEFORROUTING ) ) {
				continue;
			}
			t = AAS_AreaTravelTimeToGoalArea( j, ( *aasworld ).areawaypoints[j], i, tfl );

#if defined RTCW_SP
			( *aasworld ).frameroutingupdates = 0;
#endif // RTCW_XX
#else
	for ( i = 1; i < aasworld->numareas; i++ )
	{
		if ( !AAS_AreaReachability( i ) ) {
			continue;
		}
		areasettings = &aasworld->areasettings[i];
		for ( k = 0; k < areasettings->numreachableareas; k++ )
		{
			reach = &aasworld->reachability[areasettings->firstreachablearea + k];
			if ( aasworld->travelflagfortype[reach->traveltype] & tfl ) {
				break;
			}
		}
		if ( k >= areasettings->numreachableareas ) {
			continue;
		}
		aasworld->areasettings[i].areaflags |= AREA_USEFORROUTING;
		numroutingareas++;
	}
	for ( i = 1; i < aasworld->numareas; i++ )
	{
		if ( !( aasworld->areasettings[i].areaflags & AREA_USEFORROUTING ) ) {
			continue;
		}
		for ( j = 1; j < aasworld->numareas; j++ )
		{
			if ( i == j ) {
				continue;
			}
			if ( !( aasworld->areasettings[j].areaflags & AREA_USEFORROUTING ) ) {
				continue;
			}
			t = AAS_AreaTravelTimeToGoalArea( j, aasworld->areawaypoints[j], i, tfl );
			aasworld->frameroutingupdates = 0;
#endif // RTCW_XX

			//if (t) break;
			//Log_Write("traveltime from %d to %d is %d", i, j, t);
		} //end for
	} //end for
} //end of the function AAS_CreateAllRoutingCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
unsigned short CRC_ProcessString( unsigned char *data, int length );

//the route cache header
//this header is followed by numportalcache + numareacache aas_routingcache_t
//structures that store routing cache
typedef struct routecacheheader_s
{
	int ident;
	int version;
	int numareas;
	int numclusters;
	int areacrc;
	int clustercrc;
	int reachcrc;
	int numportalcache;
	int numareacache;
} routecacheheader_t;

#define RCID                        ( ( 'C' << 24 ) + ( 'R' << 16 ) + ( 'E' << 8 ) + 'M' )

#if defined RTCW_SP
#define RCVERSION                   15
#elif defined RTCW_MP
#define RCVERSION                   12
#else
#define RCVERSION                   16
#endif // RTCW_XX

void AAS_DecompressVis( byte *in, int numareas, byte *decompressed );
int AAS_CompressVis( byte *vis, int numareas, byte *dest );


void AAS_WriteRouteCache( void ) {
	int i, j, numportalcache, numareacache, size;
	aas_routingcache_t *cache;
	aas_cluster_t *cluster;
	fileHandle_t fp;
	char filename[MAX_QPATH];
	routecacheheader_t routecacheheader;
	byte *buf;

//BBi Merged
//#if !defined RTCW_ET
//	buf = (byte *) GetClearedMemory( ( *aasworld ).numareas * 2 * sizeof( byte ) );   // in case it ends up bigger than the decompressedvis, which is rare but possible
//
//	numportalcache = 0;
//	for ( i = 0; i < ( *aasworld ).numareas; i++ )
//	{
//		for ( cache = ( *aasworld ).portalcache[i]; cache; cache = cache->next )
//		{
//			numportalcache++;
//		} //end for
//	} //end for
//	numareacache = 0;
//	for ( i = 0; i < ( *aasworld ).numclusters; i++ )
//	{
//		cluster = &( *aasworld ).clusters[i];
//		for ( j = 0; j < cluster->numareas; j++ )
//		{
//			for ( cache = ( *aasworld ).clusterareacache[i][j]; cache; cache = cache->next )
//			{
//				numareacache++;
//			} //end for
//		} //end for
//	} //end for
//	  // open the file for writing
//	Com_sprintf( filename, MAX_QPATH, "maps/%s.rcd", ( *aasworld ).mapname );
//	botimport.FS_FOpenFile( filename, &fp, FS_WRITE );
//	if ( !fp ) {
//		AAS_Error( "Unable to open file: %s\n", filename );
//		return;
//	} //end if
//	  //create the header
//	routecacheheader.ident = RCID;
//	routecacheheader.version = RCVERSION;
//	routecacheheader.numareas = ( *aasworld ).numareas;
//	routecacheheader.numclusters = ( *aasworld ).numclusters;
//	routecacheheader.areacrc = CRC_ProcessString( (unsigned char *)( *aasworld ).areas, sizeof( aas_area_t ) * ( *aasworld ).numareas );
//	routecacheheader.clustercrc = CRC_ProcessString( (unsigned char *)( *aasworld ).clusters, sizeof( aas_cluster_t ) * ( *aasworld ).numclusters );
//	routecacheheader.reachcrc = CRC_ProcessString( (unsigned char *)( *aasworld ).reachability, sizeof( aas_reachability_t ) * ( *aasworld ).reachabilitysize );
//	routecacheheader.numportalcache = numportalcache;
//	routecacheheader.numareacache = numareacache;
//	//write the header
//	botimport.FS_Write( &routecacheheader, sizeof( routecacheheader_t ), fp );
//	//write all the cache
//	for ( i = 0; i < ( *aasworld ).numareas; i++ )
//	{
//		for ( cache = ( *aasworld ).portalcache[i]; cache; cache = cache->next )
//		{
//			botimport.FS_Write( cache, cache->size, fp );
//		} //end for
//	} //end for
//	for ( i = 0; i < ( *aasworld ).numclusters; i++ )
//	{
//		cluster = &( *aasworld ).clusters[i];
//		for ( j = 0; j < cluster->numareas; j++ )
//		{
//			for ( cache = ( *aasworld ).clusterareacache[i][j]; cache; cache = cache->next )
//			{
//				botimport.FS_Write( cache, cache->size, fp );
//			} //end for
//		} //end for
//	} //end for
//	  // write the visareas
//	for ( i = 0; i < ( *aasworld ).numareas; i++ )
//	{
//		if ( !( *aasworld ).areavisibility[i] ) {
//			size = 0;
//			botimport.FS_Write( &size, sizeof( int ), fp );
//			continue;
//		}
//		AAS_DecompressVis( ( *aasworld ).areavisibility[i], ( *aasworld ).numareas, ( *aasworld ).decompressedvis );
//		size = AAS_CompressVis( ( *aasworld ).decompressedvis, ( *aasworld ).numareas, buf );
//		botimport.FS_Write( &size, sizeof( int ), fp );
//		botimport.FS_Write( buf, size, fp );
//	}
//	// write the waypoints
//	botimport.FS_Write( ( *aasworld ).areawaypoints, sizeof( vec3_t ) * ( *aasworld ).numareas, fp );
//#else
//	buf = (byte *) GetClearedMemory( aasworld->numareas * 2 * sizeof( byte ) ); // in case it ends up bigger than the decompressedvis, which is rare but possible
//
//	numportalcache = 0;
//	for ( i = 0; i < aasworld->numareas; i++ )
//	{
//		for ( cache = aasworld->portalcache[i]; cache; cache = cache->next )
//		{
//			numportalcache++;
//		} //end for
//	} //end for
//	numareacache = 0;
//	for ( i = 0; i < aasworld->numclusters; i++ )
//	{
//		cluster = &aasworld->clusters[i];
//		for ( j = 0; j < cluster->numareas; j++ )
//		{
//			for ( cache = aasworld->clusterareacache[i][j]; cache; cache = cache->next )
//			{
//				numareacache++;
//			} //end for
//		} //end for
//	} //end for
//	  // open the file for writing
//	Com_sprintf( filename, MAX_QPATH, "maps/%s.rcd", aasworld->mapname );
//	botimport.FS_FOpenFile( filename, &fp, FS_WRITE );
//	if ( !fp ) {
//		AAS_Error( "Unable to open file: %s\n", filename );
//		return;
//	} //end if
//	  //create the header
//	routecacheheader.ident = RCID;
//	routecacheheader.version = RCVERSION;
//	routecacheheader.numareas = aasworld->numareas;
//	routecacheheader.numclusters = aasworld->numclusters;
//	routecacheheader.areacrc = CRC_ProcessString( (unsigned char *)aasworld->areas, sizeof( aas_area_t ) * aasworld->numareas );
//	routecacheheader.clustercrc = CRC_ProcessString( (unsigned char *)aasworld->clusters, sizeof( aas_cluster_t ) * aasworld->numclusters );
//	routecacheheader.reachcrc = CRC_ProcessString( (unsigned char *)aasworld->reachability, sizeof( aas_reachability_t ) * aasworld->reachabilitysize );
//	routecacheheader.numportalcache = numportalcache;
//	routecacheheader.numareacache = numareacache;
//	//write the header
//	botimport.FS_Write( &routecacheheader, sizeof( routecacheheader_t ), fp );
//	//write all the cache
//	for ( i = 0; i < aasworld->numareas; i++ )
//	{
//		for ( cache = aasworld->portalcache[i]; cache; cache = cache->next )
//		{
//			botimport.FS_Write( cache, cache->size, fp );
//		} //end for
//	} //end for
//	for ( i = 0; i < aasworld->numclusters; i++ )
//	{
//		cluster = &aasworld->clusters[i];
//		for ( j = 0; j < cluster->numareas; j++ )
//		{
//			for ( cache = aasworld->clusterareacache[i][j]; cache; cache = cache->next )
//			{
//				botimport.FS_Write( cache, cache->size, fp );
//			} //end for
//		} //end for
//	} //end for
//	  // write the visareas
//	for ( i = 0; i < aasworld->numareas; i++ )
//	{
//		if ( !aasworld->areavisibility[i] ) {
//			size = 0;
//			botimport.FS_Write( &size, sizeof( int ), fp );
//			continue;
//		}
//		AAS_DecompressVis( aasworld->areavisibility[i], aasworld->numareas, aasworld->decompressedvis );
//		size = AAS_CompressVis( aasworld->decompressedvis, aasworld->numareas, buf );
//		botimport.FS_Write( &size, sizeof( int ), fp );
//		botimport.FS_Write( buf, size, fp );
//	}
//	// write the waypoints
//	botimport.FS_Write( aasworld->areawaypoints, sizeof( vec3_t ) * aasworld->numareas, fp );
//#endif // RTCW_XX

    buf = static_cast<byte*> (GetClearedMemory (aasworld->numareas * 2 * sizeof (byte))); // in case it ends up bigger than the decompressedvis, which is rare but possible

    numportalcache = 0;

    for (i = 0; i < aasworld->numareas; ++i) {
        for (cache = aasworld->portalcache[i]; cache; cache = cache->next)
            ++numportalcache;
    }

    numareacache = 0;

    for (i = 0; i < aasworld->numclusters; ++i) {
        cluster = &aasworld->clusters[i];

        for ( j = 0; j < cluster->numareas; j++ ) {
            for (cache = aasworld->clusterareacache[i][j]; cache; cache = cache->next)
                ++numareacache;
        }
    }

    // open the file for writing
    Com_sprintf (filename, MAX_QPATH, "maps/%s.rcd", aasworld->mapname);

    botimport.FS_FOpenFile (filename, &fp, FS_WRITE);

    if (fp == 0) {
        AAS_Error ("Unable to open file: %s\n", filename);
        return;
    }

    //create the header
    routecacheheader.ident = rtcw::Endian::le(static_cast<uint32_t> (RCID));
    routecacheheader.version = rtcw::Endian::le(static_cast<uint32_t> (RCVERSION));
    routecacheheader.numareas = rtcw::Endian::le( aasworld->numareas);
    routecacheheader.numclusters = rtcw::Endian::le(aasworld->numclusters);
    routecacheheader.areacrc = rtcw::Endian::le(CRC_ProcessString (
        reinterpret_cast<unsigned char*>(aasworld->areas), sizeof (aas_area_t) * aasworld->numareas));
    routecacheheader.clustercrc = rtcw::Endian::le(CRC_ProcessString (
        reinterpret_cast<unsigned char*>(aasworld->clusters), sizeof (aas_cluster_t) * aasworld->numclusters));
    routecacheheader.reachcrc = rtcw::Endian::le(CRC_ProcessString (
        reinterpret_cast<unsigned char*>(aasworld->reachability), sizeof (aas_reachability_t) * aasworld->reachabilitysize));
    routecacheheader.numportalcache = rtcw::Endian::le(numportalcache);
    routecacheheader.numareacache = rtcw::Endian::le(numareacache);

    //write the header
    botimport.FS_Write (&routecacheheader, sizeof (routecacheheader_t), fp);

    //write all the cache
    for (i = 0; i < aasworld->numareas; ++i) {
        for (cache = aasworld->portalcache[i]; cache; cache = cache->next)
            aasWriteRcStruct (cache, fp);
    }

    for (i = 0; i < aasworld->numclusters; ++i) {
        cluster = &aasworld->clusters[i];

        for (j = 0; j < cluster->numareas; ++j) {
            for (cache = aasworld->clusterareacache[i][j]; cache; cache = cache->next)
                aasWriteRcStruct (cache, fp);
        }
    }

    // write the visareas
    for (i = 0; i < aasworld->numareas; ++i) {
        if (!aasworld->areavisibility[i]) {
            size = 0;
            botimport.FS_Write (&size, sizeof (int), fp);
            continue;
        }

        AAS_DecompressVis (aasworld->areavisibility[i], aasworld->numareas, aasworld->decompressedvis);
        size = AAS_CompressVis (aasworld->decompressedvis, aasworld->numareas, buf);
        rtcw::Endian::lei(size);
        botimport.FS_Write (&size, sizeof (int), fp);
        botimport.FS_Write (buf, size, fp);
    }

    // write the waypoints
    botimport.FS_Write (aasworld->areawaypoints, sizeof (vec3_t) * aasworld->numareas, fp);

    vec3_t vecBuffer;

    for (i = 0; i < aasworld->numareas; ++i) {
        rtcw::Endian::le(aasworld->areawaypoints[i], vecBuffer);

        botimport.FS_Write (vecBuffer, sizeof (vec3_t), fp);
    }
//BBi

	//
	botimport.FS_FCloseFile( fp );
	botimport.Print( PRT_MESSAGE, "\nroute cache written to %s\n", filename );
} //end of the function AAS_WriteRouteCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================

//BBi
//aas_routingcache_t *AAS_ReadCache( fileHandle_t fp ) {
//
//#if defined RTCW_SP
//	int i, size;
//#else
//	int size, i;
//#endif // RTCW_XX
//
//	aas_routingcache_t *cache;
//
//	botimport.FS_Read( &size, sizeof( size ), fp );
//
//#if !defined RTCW_MP
//	size = LittleLong( size );
//#endif // RTCW_XX
//
//	cache = (aas_routingcache_t *) AAS_RoutingGetMemory( size );
//	cache->size = size;
//	botimport.FS_Read( (unsigned char *)cache + sizeof( size ), size - sizeof( size ), fp );
//
//#if !defined RTCW_MP
//	if ( 1 != LittleLong( 1 ) ) {
//		cache->time = LittleFloat( cache->time );
//		cache->cluster = LittleLong( cache->cluster );
//		cache->areanum = LittleLong( cache->areanum );
//		cache->origin[0] = LittleFloat( cache->origin[0] );
//		cache->origin[1] = LittleFloat( cache->origin[1] );
//		cache->origin[2] = LittleFloat( cache->origin[2] );
//		cache->starttraveltime = LittleFloat( cache->starttraveltime );
//		cache->travelflags = LittleLong( cache->travelflags );
//	}
//#endif // RTCW_XX
//
////	cache->reachabilities = (unsigned char *) cache + sizeof(aas_routingcache_t) - sizeof(unsigned short) +
////		(size - sizeof(aas_routingcache_t) + sizeof(unsigned short)) / 3 * 2;
//	cache->reachabilities = (unsigned char *) cache + sizeof( aas_routingcache_t ) +
//							( ( size - sizeof( aas_routingcache_t ) ) / 3 ) * 2;
//
//	//DAJ BUGFIX for missing byteswaps for traveltimes
//	size = ( size - sizeof( aas_routingcache_t ) ) / 3 + 1;
//	for ( i = 0; i < size; i++ ) {
//		cache->traveltimes[i] = LittleShort( cache->traveltimes[i] );
//	}
//	return cache;
//} //end of the function AAS_ReadCache

aas_routingcache_t* AAS_ReadCache (
    fileHandle_t fp)
{
    if (aasIs32) {
        aas_routingcache_t* cache;

        int size;
        botimport.FS_Read (&size, sizeof (size), fp);
        rtcw::Endian::lei(size);

        cache = static_cast<aas_routingcache_t*> (AAS_RoutingGetMemory (size));
        cache->size = size;
        botimport.FS_Read (reinterpret_cast<byte*> (cache) + sizeof (size), size - sizeof (size), fp);

        cache->reachabilities = reinterpret_cast<byte*> (cache) + sizeof (aas_routingcache_t) +
            ((size - sizeof (aas_routingcache_t)) / 3) * 2;

        if (!rtcw::Endian::is_little()) {
            rtcw::Endian::lei(cache->time);
            rtcw::Endian::lei(cache->cluster);
            rtcw::Endian::lei(cache->areanum);
            rtcw::Endian::lei(cache->origin);
            rtcw::Endian::lei(cache->starttraveltime);
            rtcw::Endian::lei(cache->travelflags);
        }

        size = ((size - sizeof (aas_routingcache_t)) / 3) + 1;

        rtcw::Endian::lei(cache->traveltimes, size);

        return cache;
    } else {
        int size;
        botimport.FS_Read (&size, sizeof (size), fp);
        rtcw::Endian::lei(size);

        aas_rcs_buffer.resize (size);

        aas_routingcache_t::Struct32* cache = reinterpret_cast<aas_routingcache_t::Struct32*> (&aas_rcs_buffer[0]);

        cache->size = size;
        botimport.FS_Read (&aas_rcs_buffer[0] + sizeof (size), size - sizeof (size), fp);

        return aas_routingcache_t::convertFrom32 (cache);
    }
}
//BBi

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_ReadRouteCache( void ) {
	int i, clusterareanum, size;

#if !defined RTCW_MP
	fileHandle_t fp = 0;
#else
	fileHandle_t fp;
#endif // RTCW_XX

	char filename[MAX_QPATH];
	routecacheheader_t routecacheheader;
	aas_routingcache_t *cache;

#if !defined RTCW_ET
	Com_sprintf( filename, MAX_QPATH, "maps/%s.rcd", ( *aasworld ).mapname );
#else
	Com_sprintf( filename, MAX_QPATH, "maps/%s.rcd", aasworld->mapname );
#endif // RTCW_XX

	botimport.FS_FOpenFile( filename, &fp, FS_READ );
	if ( !fp ) {
		return qfalse;
	} //end if
	botimport.FS_Read( &routecacheheader, sizeof( routecacheheader_t ), fp );

//BBi
//#if defined RTCW_SP
//	// GJD: route cache data MUST be written on a PC because I've not altered the writing code.
//
//	routecacheheader.areacrc = LittleLong( routecacheheader.areacrc );
//	routecacheheader.clustercrc = LittleLong( routecacheheader.clustercrc );
//	routecacheheader.ident = LittleLong( routecacheheader.ident );
//	routecacheheader.numareacache = LittleLong( routecacheheader.numareacache );
//	routecacheheader.numareas = LittleLong( routecacheheader.numareas );
//	routecacheheader.numclusters = LittleLong( routecacheheader.numclusters );
//	routecacheheader.numportalcache = LittleLong( routecacheheader.numportalcache );
//	routecacheheader.reachcrc = LittleLong( routecacheheader.reachcrc );
//	routecacheheader.version = LittleLong( routecacheheader.version );
//#endif // RTCW_XX

    rtcw::Endian::lei(routecacheheader.areacrc);
    rtcw::Endian::lei(routecacheheader.clustercrc);
    rtcw::Endian::lei(routecacheheader.ident);
    rtcw::Endian::lei(routecacheheader.numareacache);
    rtcw::Endian::lei(routecacheheader.numareas);
    rtcw::Endian::lei(routecacheheader.numclusters);
    rtcw::Endian::lei(routecacheheader.numportalcache);
    rtcw::Endian::lei(routecacheheader.reachcrc);
    rtcw::Endian::lei(routecacheheader.version);
//BBi

	if ( routecacheheader.ident != RCID ) {
		botimport.FS_FCloseFile( fp );

#if defined RTCW_SP
		Com_Printf( "%s is not a route cache dump\n", filename );       // not an aas_error because we want to continue
		return qfalse;                                              // and remake them by returning false here
#else
		AAS_Error( "%s is not a route cache dump\n" );
		return qfalse;
#endif // RTCW_XX

	} //end if

	if ( routecacheheader.version != RCVERSION ) {
		botimport.FS_FCloseFile( fp );

#if defined RTCW_SP
		Com_Printf( "route cache dump has wrong version %d, should be %d", routecacheheader.version, RCVERSION );
#else
		AAS_Error( "route cache dump has wrong version %d, should be %d", routecacheheader.version, RCVERSION );
#endif // RTCW_XX

		return qfalse;
	} //end if

#if !defined RTCW_ET
	if ( routecacheheader.numareas != ( *aasworld ).numareas ) {
#else
	if ( routecacheheader.numareas != aasworld->numareas ) {
#endif // RTCW_XX

		botimport.FS_FCloseFile( fp );
		//AAS_Error("route cache dump has wrong number of areas\n");
		return qfalse;
	} //end if

#if !defined RTCW_ET
	if ( routecacheheader.numclusters != ( *aasworld ).numclusters ) {
#else
	if ( routecacheheader.numclusters != aasworld->numclusters ) {
#endif // RTCW_XX

		botimport.FS_FCloseFile( fp );
		//AAS_Error("route cache dump has wrong number of clusters\n");
		return qfalse;
	} //end if

#if defined RTCW_SP
#ifdef _WIN32                           // crc code is only good on intel machines
	if ( routecacheheader.areacrc !=
		 CRC_ProcessString( (unsigned char *)( *aasworld ).areas, sizeof( aas_area_t ) * ( *aasworld ).numareas ) ) {
		botimport.FS_FCloseFile( fp );
		//AAS_Error("route cache dump area CRC incorrect\n");
		return qfalse;
	} //end if
	if ( routecacheheader.clustercrc !=
		 CRC_ProcessString( (unsigned char *)( *aasworld ).clusters, sizeof( aas_cluster_t ) * ( *aasworld ).numclusters ) ) {
		botimport.FS_FCloseFile( fp );
		//AAS_Error("route cache dump cluster CRC incorrect\n");
		return qfalse;
	} //end if
	if ( routecacheheader.reachcrc !=
		 CRC_ProcessString( (unsigned char *)( *aasworld ).reachability, sizeof( aas_reachability_t ) * ( *aasworld ).reachabilitysize ) ) {
		botimport.FS_FCloseFile( fp );
		//AAS_Error("route cache dump reachability CRC incorrect\n");
		return qfalse;
	} //end if
#endif
#else
#if defined( MACOSX )
	// the crc table stuff is endian orientated....
#else
	if ( routecacheheader.areacrc !=

#if !defined RTCW_ET
		 CRC_ProcessString( (unsigned char *)( *aasworld ).areas, sizeof( aas_area_t ) * ( *aasworld ).numareas ) ) {
#else
		 CRC_ProcessString( (unsigned char *)aasworld->areas, sizeof( aas_area_t ) * aasworld->numareas ) ) {
#endif // RTCW_XX

		botimport.FS_FCloseFile( fp );
		//AAS_Error("route cache dump area CRC incorrect\n");
		return qfalse;
	} //end if
	if ( routecacheheader.clustercrc !=

#if !defined RTCW_ET
		 CRC_ProcessString( (unsigned char *)( *aasworld ).clusters, sizeof( aas_cluster_t ) * ( *aasworld ).numclusters ) ) {
#else
		 CRC_ProcessString( (unsigned char *)aasworld->clusters, sizeof( aas_cluster_t ) * aasworld->numclusters ) ) {
#endif // RTCW_XX

		botimport.FS_FCloseFile( fp );
		//AAS_Error("route cache dump cluster CRC incorrect\n");
		return qfalse;
	} //end if
	if ( routecacheheader.reachcrc !=

#if !defined RTCW_ET
		 CRC_ProcessString( (unsigned char *)( *aasworld ).reachability, sizeof( aas_reachability_t ) * ( *aasworld ).reachabilitysize ) ) {
#else
		 CRC_ProcessString( (unsigned char *)aasworld->reachability, sizeof( aas_reachability_t ) * aasworld->reachabilitysize ) ) {
#endif // RTCW_XX

		botimport.FS_FCloseFile( fp );
		//AAS_Error("route cache dump reachability CRC incorrect\n");
		return qfalse;
	} //end if
#endif
#endif // RTCW_XX

	//read all the portal cache
	for ( i = 0; i < routecacheheader.numportalcache; i++ )
	{
		cache = AAS_ReadCache( fp );

#if !defined RTCW_ET
		cache->next = ( *aasworld ).portalcache[cache->areanum];
		cache->prev = NULL;
		if ( ( *aasworld ).portalcache[cache->areanum] ) {
			( *aasworld ).portalcache[cache->areanum]->prev = cache;
		}
		( *aasworld ).portalcache[cache->areanum] = cache;
	} //end for
	  //read all the cluster area cache
	for ( i = 0; i < routecacheheader.numareacache; i++ )
	{
		cache = AAS_ReadCache( fp );
		clusterareanum = AAS_ClusterAreaNum( cache->cluster, cache->areanum );
		cache->next = ( *aasworld ).clusterareacache[cache->cluster][clusterareanum];
		cache->prev = NULL;
		if ( ( *aasworld ).clusterareacache[cache->cluster][clusterareanum] ) {
			( *aasworld ).clusterareacache[cache->cluster][clusterareanum]->prev = cache;
		}
		( *aasworld ).clusterareacache[cache->cluster][clusterareanum] = cache;
	} //end for
	  // read the visareas
	( *aasworld ).areavisibility = (byte **) GetClearedMemory( ( *aasworld ).numareas * sizeof( byte * ) );
	( *aasworld ).decompressedvis = (byte *) GetClearedMemory( ( *aasworld ).numareas * sizeof( byte ) );
	for ( i = 0; i < ( *aasworld ).numareas; i++ )
#else
		cache->next = aasworld->portalcache[cache->areanum];
		cache->prev = NULL;
		if ( aasworld->portalcache[cache->areanum] ) {
			aasworld->portalcache[cache->areanum]->prev = cache;
		}
		aasworld->portalcache[cache->areanum] = cache;
	} //end for
	  //read all the cluster area cache
	for ( i = 0; i < routecacheheader.numareacache; i++ )
	{
		cache = AAS_ReadCache( fp );
		clusterareanum = AAS_ClusterAreaNum( cache->cluster, cache->areanum );
		cache->next = aasworld->clusterareacache[cache->cluster][clusterareanum];
		cache->prev = NULL;
		if ( aasworld->clusterareacache[cache->cluster][clusterareanum] ) {
			aasworld->clusterareacache[cache->cluster][clusterareanum]->prev = cache;
		}
		aasworld->clusterareacache[cache->cluster][clusterareanum] = cache;
	} //end for
	  // read the visareas
	aasworld->areavisibility = (byte **) GetClearedMemory( aasworld->numareas * sizeof( byte * ) );
	aasworld->decompressedvis = (byte *) GetClearedMemory( aasworld->numareas * sizeof( byte ) );
	for ( i = 0; i < aasworld->numareas; i++ )
#endif // RTCW_XX

	{
		botimport.FS_Read( &size, sizeof( size ), fp );

#if defined RTCW_SP
		rtcw::Endian::lei(size);
#endif // RTCW_XX

		if ( size ) {

#if !defined RTCW_ET
			( *aasworld ).areavisibility[i] = (byte *) GetMemory( size );
			botimport.FS_Read( ( *aasworld ).areavisibility[i], size, fp );
		}
	}
	// read the area waypoints
	( *aasworld ).areawaypoints = (vec3_t *) GetClearedMemory( ( *aasworld ).numareas * sizeof( vec3_t ) );
	botimport.FS_Read( ( *aasworld ).areawaypoints, ( *aasworld ).numareas * sizeof( vec3_t ), fp );
#else
			aasworld->areavisibility[i] = (byte *) GetMemory( size );
			botimport.FS_Read( aasworld->areavisibility[i], size, fp );
		}
	}
	// read the area waypoints
	aasworld->areawaypoints = (vec3_t *) GetClearedMemory( aasworld->numareas * sizeof( vec3_t ) );
	botimport.FS_Read( aasworld->areawaypoints, aasworld->numareas * sizeof( vec3_t ), fp );
#endif // RTCW_XX

#if defined RTCW_SP
	if (!rtcw::Endian::is_little ()) {
		for (i = 0; i < aasworld->numareas; ++i)
			rtcw::Endian::lei(aasworld->areawaypoints[i]);
	}
#endif // RTCW_XX

	//
	botimport.FS_FCloseFile( fp );
	return qtrue;
} //end of the function AAS_ReadRouteCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================

#if !defined RTCW_ET
void AAS_CreateVisibility( void );
#else
void AAS_CreateVisibility( qboolean waypointsOnly );
#endif // RTCW_XX

void AAS_InitRouting( void ) {
	AAS_InitTravelFlagFromType();
	//initialize the routing update fields
	AAS_InitRoutingUpdate();
	//create reversed reachability links used by the routing update algorithm
	AAS_CreateReversedReachability();
	//initialize the cluster cache
	AAS_InitClusterAreaCache();
	//initialize portal cache
	AAS_InitPortalCache();
	//initialize the area travel times
	AAS_CalculateAreaTravelTimes();
	//calculate the maximum travel times through portals
	AAS_InitPortalMaxTravelTimes();
	//
#ifdef ROUTING_DEBUG
	numareacacheupdates = 0;
	numportalcacheupdates = 0;
#endif //ROUTING_DEBUG
	   //
	routingcachesize = 0;

#if !defined RTCW_ET
	max_routingcachesize = 1024 * (int) LibVarValue( "max_routingcache", "4096" );
	//
	// Ridah, load or create the routing cache
	if ( !AAS_ReadRouteCache() ) {
		( *aasworld ).initialized = qtrue;    // Hack, so routing can compute traveltimes
		AAS_CreateVisibility();
		AAS_CreateAllRoutingCache();
		( *aasworld ).initialized = qfalse;

		AAS_WriteRouteCache();  // save it so we don't have to create it again
	}
	// done.
#else
	max_routingcachesize = 1024 * (int) LibVarValue( "max_routingcache", DEFAULT_MAX_ROUTINGCACHESIZE );
	max_frameroutingupdates = (int) LibVarGetValue( "bot_frameroutingupdates" );
	//
	// enable this for quick testing of maps without enemies
	if ( LibVarGetValue( "bot_norcd" ) ) {
		// RF, create the waypoints for each area
		AAS_CreateVisibility( qtrue );
	} else {
		// Ridah, load or create the routing cache
		if ( !AAS_ReadRouteCache() ) {
			aasworld->initialized = qtrue;  // Hack, so routing can compute traveltimes
			AAS_CreateVisibility( qfalse );
			// RF, removed, going back to dynamic routes
			//AAS_CreateAllRoutingCache();
			aasworld->initialized = qfalse;

			AAS_WriteRouteCache();  // save it so we don't have to create it again
		}
		// done.
	}
#endif // RTCW_XX

} //end of the function AAS_InitRouting
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_FreeRoutingCaches( void ) {
	// free all the existing cluster area cache
	AAS_FreeAllClusterAreaCache();
	// free all the existing portal cache
	AAS_FreeAllPortalCache();
	// free all the existing area visibility data
	AAS_FreeAreaVisibility();
	// free cached travel times within areas

#if !defined RTCW_ET
	if ( ( *aasworld ).areatraveltimes ) {
		AAS_RoutingFreeMemory( ( *aasworld ).areatraveltimes );
	}
	( *aasworld ).areatraveltimes = NULL;
	// free cached maximum travel time through cluster portals
	if ( ( *aasworld ).portalmaxtraveltimes ) {
		AAS_RoutingFreeMemory( ( *aasworld ).portalmaxtraveltimes );
	}
	( *aasworld ).portalmaxtraveltimes = NULL;
	// free reversed reachability links
	if ( ( *aasworld ).reversedreachability ) {
		AAS_RoutingFreeMemory( ( *aasworld ).reversedreachability );
	}
	( *aasworld ).reversedreachability = NULL;
	// free routing algorithm memory
	if ( ( *aasworld ).areaupdate ) {
		AAS_RoutingFreeMemory( ( *aasworld ).areaupdate );
	}
	( *aasworld ).areaupdate = NULL;
	if ( ( *aasworld ).portalupdate ) {
		AAS_RoutingFreeMemory( ( *aasworld ).portalupdate );
	}
	( *aasworld ).portalupdate = NULL;
	// free area waypoints
	if ( ( *aasworld ).areawaypoints ) {
		FreeMemory( ( *aasworld ).areawaypoints );
	}
	( *aasworld ).areawaypoints = NULL;
#else
	if ( aasworld->areatraveltimes ) {
		AAS_RoutingFreeMemory( aasworld->areatraveltimes );
	}
	aasworld->areatraveltimes = NULL;
	// free cached maximum travel time through cluster portals
	if ( aasworld->portalmaxtraveltimes ) {
		AAS_RoutingFreeMemory( aasworld->portalmaxtraveltimes );
	}
	aasworld->portalmaxtraveltimes = NULL;
	// free reversed reachability links
	if ( aasworld->reversedreachability ) {
		AAS_RoutingFreeMemory( aasworld->reversedreachability );
	}
	aasworld->reversedreachability = NULL;
	// free routing algorithm memory
	if ( aasworld->areaupdate ) {
		AAS_RoutingFreeMemory( aasworld->areaupdate );
	}
	aasworld->areaupdate = NULL;
	if ( aasworld->portalupdate ) {
		AAS_RoutingFreeMemory( aasworld->portalupdate );
	}
	aasworld->portalupdate = NULL;
	// free area waypoints
	if ( aasworld->areawaypoints ) {
		FreeMemory( aasworld->areawaypoints );
	}
	aasworld->areawaypoints = NULL;
#endif // RTCW_XX

} //end of the function AAS_FreeRoutingCaches
//===========================================================================
// this function could be replaced by a bubble sort or for even faster
// routing by a B+ tree
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_AddUpdateToList( aas_routingupdate_t **updateliststart,
								   aas_routingupdate_t **updatelistend,
								   aas_routingupdate_t *update ) {
	if ( !update->inlist ) {
		if ( *updatelistend ) {
			( *updatelistend )->next = update;
		} else { *updateliststart = update;}
		update->prev = *updatelistend;
		update->next = NULL;
		*updatelistend = update;
		update->inlist = qtrue;
	} //end if
} //end of the function AAS_AddUpdateToList
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_AreaContentsTravelFlag( int areanum ) {
	int contents, tfl;

#if !defined RTCW_ET
	contents = ( *aasworld ).areasettings[areanum].contents;
#else
	contents = aasworld->areasettings[areanum].contents;
#endif // RTCW_XX

	tfl = 0;
	if ( contents & AREACONTENTS_WATER ) {
		return tfl |= TFL_WATER;
	} else if ( contents & AREACONTENTS_SLIME ) {
		return tfl |= TFL_SLIME;
	} else if ( contents & AREACONTENTS_LAVA )                                                                      {
		return tfl |= TFL_LAVA;
	} else { tfl |= TFL_AIR;}
	if ( contents & AREACONTENTS_DONOTENTER_LARGE ) {
		tfl |= TFL_DONOTENTER_LARGE;
	}
	if ( contents & AREACONTENTS_DONOTENTER ) {
		return tfl |= TFL_DONOTENTER;
	}
	return tfl;
} //end of the function AAS_AreaContentsTravelFlag
//===========================================================================
// update the given routing cache
//
// Parameter:			areacache		: routing cache to update
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_UpdateAreaRoutingCache( aas_routingcache_t *areacache ) {
	int i, nextareanum, cluster, badtravelflags, clusterareanum, linknum;
	int numreachabilityareas;
	unsigned short int t, startareatraveltimes[128];
	aas_routingupdate_t *updateliststart, *updatelistend, *curupdate, *nextupdate;
	aas_reachability_t *reach;
	aas_reversedreachability_t *revreach;
	aas_reversedlink_t *revlink;

#ifdef ROUTING_DEBUG
	numareacacheupdates++;
#endif //ROUTING_DEBUG
	   //number of reachability areas within this cluster

#if !defined RTCW_ET
	numreachabilityareas = ( *aasworld ).clusters[areacache->cluster].numreachabilityareas;
#else
	numreachabilityareas = aasworld->clusters[areacache->cluster].numreachabilityareas;
#endif // RTCW_XX

	//
	//clear the routing update fields

#if !defined RTCW_ET
//	memset((*aasworld).areaupdate, 0, (*aasworld).numareas * sizeof(aas_routingupdate_t));
#else
//	memset(aasworld->areaupdate, 0, aasworld->numareas * sizeof(aas_routingupdate_t));
#endif // RTCW_XX

	//
	badtravelflags = ~areacache->travelflags;
	//
	clusterareanum = AAS_ClusterAreaNum( areacache->cluster, areacache->areanum );
	if ( clusterareanum >= numreachabilityareas ) {
		return;
	}
	//
	memset( startareatraveltimes, 0, sizeof( startareatraveltimes ) );
	//

#if !defined RTCW_ET
	curupdate = &( *aasworld ).areaupdate[clusterareanum];
#else
	curupdate = &aasworld->areaupdate[clusterareanum];
#endif // RTCW_XX

	curupdate->areanum = areacache->areanum;
	//VectorCopy(areacache->origin, curupdate->start);

#if !defined RTCW_ET
	curupdate->areatraveltimes = ( *aasworld ).areatraveltimes[areacache->areanum][0];
#else
	curupdate->areatraveltimes = aasworld->areatraveltimes[areacache->areanum][0];
#endif // RTCW_XX

	curupdate->tmptraveltime = areacache->starttraveltime;
	//
	areacache->traveltimes[clusterareanum] = areacache->starttraveltime;
	//put the area to start with in the current read list
	curupdate->next = NULL;
	curupdate->prev = NULL;
	updateliststart = curupdate;
	updatelistend = curupdate;
	//while there are updates in the current list, flip the lists
	while ( updateliststart )
	{
		curupdate = updateliststart;
		//
		if ( curupdate->next ) {
			curupdate->next->prev = NULL;
		} else { updatelistend = NULL;}
		updateliststart = curupdate->next;
		//
		curupdate->inlist = qfalse;
		//check all reversed reachability links

#if !defined RTCW_ET
		revreach = &( *aasworld ).reversedreachability[curupdate->areanum];
#else
		revreach = &aasworld->reversedreachability[curupdate->areanum];
#endif // RTCW_XX

		//
		for ( i = 0, revlink = revreach->first; revlink; revlink = revlink->next, i++ )
		{
			linknum = revlink->linknum;

#if !defined RTCW_ET
			reach = &( *aasworld ).reachability[linknum];
#else
			reach = &aasworld->reachability[linknum];
#endif // RTCW_XX

			//if there is used an undesired travel type

#if !defined RTCW_ET
			if ( ( *aasworld ).travelflagfortype[reach->traveltype] & badtravelflags ) {
#else
			if ( aasworld->travelflagfortype[reach->traveltype] & badtravelflags ) {
#endif // RTCW_XX

				continue;
			}
			//if not allowed to enter the next area

#if !defined RTCW_ET
			if ( ( *aasworld ).areasettings[reach->areanum].areaflags & AREA_DISABLED ) {
#else
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_DISABLED ) {
#endif // RTCW_XX

				continue;
			}
			//if the next area has a not allowed travel flag
			if ( AAS_AreaContentsTravelFlag( reach->areanum ) & badtravelflags ) {
				continue;
			}
			//number of the area the reversed reachability leads to
			nextareanum = revlink->areanum;
			//get the cluster number of the area

#if !defined RTCW_ET
			cluster = ( *aasworld ).areasettings[nextareanum].cluster;
#else
			cluster = aasworld->areasettings[nextareanum].cluster;
#endif // RTCW_XX

			//don't leave the cluster
			if ( cluster > 0 && cluster != areacache->cluster ) {
				continue;
			}
			//get the number of the area in the cluster
			clusterareanum = AAS_ClusterAreaNum( areacache->cluster, nextareanum );
			if ( clusterareanum >= numreachabilityareas ) {
				continue;
			}
			//time already travelled plus the traveltime through
			//the current area plus the travel time from the reachability
			t = curupdate->tmptraveltime +
				//AAS_AreaTravelTime(curupdate->areanum, curupdate->start, reach->end) +
				curupdate->areatraveltimes[i] +
				reach->traveltime;

#if !defined RTCW_ET
			//
			( *aasworld ).frameroutingupdates++;
			//
			if ( !areacache->traveltimes[clusterareanum] ||
				 areacache->traveltimes[clusterareanum] > t ) {
				areacache->traveltimes[clusterareanum] = t;
				areacache->reachabilities[clusterareanum] = linknum - ( *aasworld ).areasettings[nextareanum].firstreachablearea;
				nextupdate = &( *aasworld ).areaupdate[clusterareanum];
#else
			//if trying to avoid this area
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_AVOID ) {
				t += 1000;
			} else if ( ( aasworld->areasettings[reach->areanum].areaflags & AREA_AVOID_AXIS ) && ( areacache->travelflags & TFL_TEAM_AXIS ) ) {
				t += 200; // + (curupdate->areatraveltimes[i] + reach->traveltime) * 30;
			} else if ( ( aasworld->areasettings[reach->areanum].areaflags & AREA_AVOID_ALLIES ) && ( areacache->travelflags & TFL_TEAM_ALLIES ) ) {
				t += 200; // + (curupdate->areatraveltimes[i] + reach->traveltime) * 30;
			}
			//
			aasworld->frameroutingupdates++;
			//
			if ( aasworld->areatraveltimes[nextareanum] &&
				 ( !areacache->traveltimes[clusterareanum] ||
				   areacache->traveltimes[clusterareanum] > t ) ) {
				areacache->traveltimes[clusterareanum] = t;
				areacache->reachabilities[clusterareanum] = linknum - aasworld->areasettings[nextareanum].firstreachablearea;
				nextupdate = &aasworld->areaupdate[clusterareanum];
#endif // RTCW_XX

				nextupdate->areanum = nextareanum;
				nextupdate->tmptraveltime = t;
				//VectorCopy(reach->start, nextupdate->start);

#if !defined RTCW_ET
				nextupdate->areatraveltimes = ( *aasworld ).areatraveltimes[nextareanum][linknum -
																						 ( *aasworld ).areasettings[nextareanum].firstreachablearea];
#else
				nextupdate->areatraveltimes = aasworld->areatraveltimes[nextareanum][linknum -
																					 aasworld->areasettings[nextareanum].firstreachablearea];
#endif // RTCW_XX

				if ( !nextupdate->inlist ) {
					nextupdate->next = NULL;
					nextupdate->prev = updatelistend;
					if ( updatelistend ) {
						updatelistend->next = nextupdate;
					} else { updateliststart = nextupdate;}
					updatelistend = nextupdate;
					nextupdate->inlist = qtrue;
				} //end if
			} //end if
		} //end for
	} //end while
} //end of the function AAS_UpdateAreaRoutingCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
aas_routingcache_t *AAS_GetAreaRoutingCache( int clusternum, int areanum, int travelflags, qboolean forceUpdate ) {
	int clusterareanum;
	aas_routingcache_t *cache, *clustercache;

	//number of the area in the cluster
	clusterareanum = AAS_ClusterAreaNum( clusternum, areanum );
	//pointer to the cache for the area in the cluster

#if !defined RTCW_ET
	clustercache = ( *aasworld ).clusterareacache[clusternum][clusterareanum];
#else
	clustercache = aasworld->clusterareacache[clusternum][clusterareanum];

	// RF, remove team-specific flags which don't exist in this cluster
	travelflags &= ~TFL_TEAM_FLAGS | aasworld->clusterTeamTravelFlags[clusternum];
#endif // RTCW_XX

	//find the cache without undesired travel flags
	for ( cache = clustercache; cache; cache = cache->next )
	{
		//if there aren't used any undesired travel types for the cache
		if ( cache->travelflags == travelflags ) {
			break;
		}
	} //end for

#if !defined RTCW_ET
	  //if there was no cache
	if ( !cache ) {
		//NOTE: the number of routing updates is limited per frame
		if ( !forceUpdate && ( ( *aasworld ).frameroutingupdates > MAX_FRAMEROUTINGUPDATES ) ) {
			return NULL;
		} //end if

		cache = AAS_AllocRoutingCache( ( *aasworld ).clusters[clusternum].numreachabilityareas );
		cache->cluster = clusternum;
		cache->areanum = areanum;
		VectorCopy( ( *aasworld ).areas[areanum].center, cache->origin );
#else
	//if there was no cache
	if ( !cache ) {
		//NOTE: the number of routing updates is limited per frame
		if ( !forceUpdate && ( aasworld->frameroutingupdates > max_frameroutingupdates ) ) {
			return NULL;
		} //end if
		cache = AAS_AllocRoutingCache( aasworld->clusters[clusternum].numreachabilityareas );
		cache->cluster = clusternum;
		cache->areanum = areanum;
		VectorCopy( aasworld->areas[areanum].center, cache->origin );
#endif // RTCW_XX

		cache->starttraveltime = 1;
		cache->travelflags = travelflags;
		cache->prev = NULL;
		cache->next = clustercache;
		if ( clustercache ) {
			clustercache->prev = cache;
		}

#if !defined RTCW_ET
		( *aasworld ).clusterareacache[clusternum][clusterareanum] = cache;
#else
		aasworld->clusterareacache[clusternum][clusterareanum] = cache;
#endif // RTCW_XX

		AAS_UpdateAreaRoutingCache( cache );
	} //end if
	  //the cache has been accessed
	cache->time = AAS_RoutingTime();
	return cache;
} //end of the function AAS_GetAreaRoutingCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_UpdatePortalRoutingCache( aas_routingcache_t *portalcache ) {

#if !defined RTCW_ET
	int i, portalnum, clusterareanum, clusternum;
#else
	int i, portalnum, clusterareanum; //, clusternum;
#endif // RTCW_XX

	unsigned short int t;
	aas_portal_t *portal;
	aas_cluster_t *cluster;
	aas_routingcache_t *cache;
	aas_routingupdate_t *updateliststart, *updatelistend, *curupdate, *nextupdate;

#ifdef ROUTING_DEBUG
	numportalcacheupdates++;
#endif //ROUTING_DEBUG
	   //clear the routing update fields

#if !defined RTCW_ET
//	memset((*aasworld).portalupdate, 0, ((*aasworld).numportals+1) * sizeof(aas_routingupdate_t));
	//
	curupdate = &( *aasworld ).portalupdate[( *aasworld ).numportals];
#else
//	memset(aasworld->portalupdate, 0, (aasworld->numportals+1) * sizeof(aas_routingupdate_t));
	//
	curupdate = &aasworld->portalupdate[aasworld->numportals];
#endif // RTCW_XX

	curupdate->cluster = portalcache->cluster;
	curupdate->areanum = portalcache->areanum;
	curupdate->tmptraveltime = portalcache->starttraveltime;

#if !defined RTCW_ET
	//if the start area is a cluster portal, store the travel time for that portal
	clusternum = ( *aasworld ).areasettings[portalcache->areanum].cluster;
	if ( clusternum < 0 ) {
		portalcache->traveltimes[-clusternum] = portalcache->starttraveltime;
	} //end if
	  //put the area to start with in the current read list
#else
	//put the area to start with in the current read list
#endif // RTCW_XX

	curupdate->next = NULL;
	curupdate->prev = NULL;
	updateliststart = curupdate;
	updatelistend = curupdate;
	//while there are updates in the current list, flip the lists
	while ( updateliststart )
	{
		curupdate = updateliststart;
		//remove the current update from the list
		if ( curupdate->next ) {
			curupdate->next->prev = NULL;
		} else { updatelistend = NULL;}
		updateliststart = curupdate->next;
		//current update is removed from the list
		curupdate->inlist = qfalse;
		//

#if !defined RTCW_ET
		cluster = &( *aasworld ).clusters[curupdate->cluster];
#else
		cluster = &aasworld->clusters[curupdate->cluster];
#endif // RTCW_XX

		//
		cache = AAS_GetAreaRoutingCache( curupdate->cluster,
										 curupdate->areanum, portalcache->travelflags, qtrue );
		//take all portals of the cluster
		for ( i = 0; i < cluster->numportals; i++ )
		{

#if !defined RTCW_ET
			portalnum = ( *aasworld ).portalindex[cluster->firstportal + i];
			portal = &( *aasworld ).portals[portalnum];
			//if this is the portal of the current update continue
			if ( portal->areanum == curupdate->areanum ) {
				continue;
			}
#else
			portalnum = aasworld->portalindex[cluster->firstportal + i];
			portal = &aasworld->portals[portalnum];
#endif // RTCW_XX

			//
			clusterareanum = AAS_ClusterAreaNum( curupdate->cluster, portal->areanum );
			if ( clusterareanum >= cluster->numreachabilityareas ) {
				continue;
			}
			//
			t = cache->traveltimes[clusterareanum];
			if ( !t ) {
				continue;
			}
			t += curupdate->tmptraveltime;
			//
			if ( !portalcache->traveltimes[portalnum] ||
				 portalcache->traveltimes[portalnum] > t ) {
				portalcache->traveltimes[portalnum] = t;
				portalcache->reachabilities[portalnum] = cache->reachabilities[clusterareanum];

#if !defined RTCW_ET
				nextupdate = &( *aasworld ).portalupdate[portalnum];
#else
				nextupdate = &aasworld->portalupdate[portalnum];
#endif // RTCW_XX

				if ( portal->frontcluster == curupdate->cluster ) {
					nextupdate->cluster = portal->backcluster;
				} //end if
				else
				{
					nextupdate->cluster = portal->frontcluster;
				} //end else
				nextupdate->areanum = portal->areanum;
				//add travel time through actual portal area for the next update

#if !defined RTCW_ET
				nextupdate->tmptraveltime = t + ( *aasworld ).portalmaxtraveltimes[portalnum];
#else
				nextupdate->tmptraveltime = t + aasworld->portalmaxtraveltimes[portalnum];
#endif // RTCW_XX

				if ( !nextupdate->inlist ) {
					nextupdate->next = NULL;
					nextupdate->prev = updatelistend;
					if ( updatelistend ) {
						updatelistend->next = nextupdate;
					} else { updateliststart = nextupdate;}
					updatelistend = nextupdate;
					nextupdate->inlist = qtrue;
				} //end if
			} //end if
		} //end for
	} //end while
} //end of the function AAS_UpdatePortalRoutingCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
aas_routingcache_t *AAS_GetPortalRoutingCache( int clusternum, int areanum, int travelflags ) {
	aas_routingcache_t *cache;

	//find the cached portal routing if existing

#if !defined RTCW_ET
	for ( cache = ( *aasworld ).portalcache[areanum]; cache; cache = cache->next )
#else
	for ( cache = aasworld->portalcache[areanum]; cache; cache = cache->next )
#endif // RTCW_XX

	{
		if ( cache->travelflags == travelflags ) {
			break;
		}
	} //end for
	  //if the portal routing isn't cached
	if ( !cache ) {

#if !defined RTCW_ET
		cache = AAS_AllocRoutingCache( ( *aasworld ).numportals );
		cache->cluster = clusternum;
		cache->areanum = areanum;
		VectorCopy( ( *aasworld ).areas[areanum].center, cache->origin );
		cache->starttraveltime = 1;
		cache->travelflags = travelflags;
		//add the cache to the cache list
		cache->prev = NULL;
		cache->next = ( *aasworld ).portalcache[areanum];
		if ( ( *aasworld ).portalcache[areanum] ) {
			( *aasworld ).portalcache[areanum]->prev = cache;
		}
		( *aasworld ).portalcache[areanum] = cache;
#else
		cache = AAS_AllocRoutingCache( aasworld->numportals );
		cache->cluster = clusternum;
		cache->areanum = areanum;
		VectorCopy( aasworld->areas[areanum].center, cache->origin );
		cache->starttraveltime = 1;
		cache->travelflags = travelflags;
		//add the cache to the cache list
		cache->prev = NULL;
		cache->next = aasworld->portalcache[areanum];
		if ( aasworld->portalcache[areanum] ) {
			aasworld->portalcache[areanum]->prev = cache;
		}
		aasworld->portalcache[areanum] = cache;
#endif // RTCW_XX

		//update the cache
		AAS_UpdatePortalRoutingCache( cache );
	} //end if
	  //the cache has been accessed
	cache->time = AAS_RoutingTime();
	return cache;
} //end of the function AAS_GetPortalRoutingCache
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_AreaRouteToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags, int *traveltime, int *reachnum ) {
	int clusternum, goalclusternum, portalnum, i, clusterareanum, bestreachnum;
	unsigned short int t, besttime;
	aas_portal_t *portal;
	aas_cluster_t *cluster;
	aas_routingcache_t *areacache, *portalcache;
	aas_reachability_t *reach;
	aas_portalindex_t *pPortalnum;

#if !defined RTCW_ET
	if ( !( *aasworld ).initialized ) {
#else
	if ( !aasworld->initialized ) {
#endif // RTCW_XX

		return qfalse;
	}

	if ( areanum == goalareanum ) {
		*traveltime = 1;
		*reachnum = 0;
		return qtrue;
	} //end if
	  //

#if !defined RTCW_ET
	if ( areanum <= 0 || areanum >= ( *aasworld ).numareas ) {
#else
	if ( areanum <= 0 || areanum >= aasworld->numareas ) {
#endif // RTCW_XX

		if ( bot_developer ) {
			botimport.Print( PRT_ERROR, "AAS_AreaTravelTimeToGoalArea: areanum %d out of range\n", areanum );
		} //end if
		return qfalse;
	} //end if

#if !defined RTCW_ET
	if ( goalareanum <= 0 || goalareanum >= ( *aasworld ).numareas ) {
#else
	if ( goalareanum <= 0 || goalareanum >= aasworld->numareas ) {
#endif // RTCW_XX

		if ( bot_developer ) {
			botimport.Print( PRT_ERROR, "AAS_AreaTravelTimeToGoalArea: goalareanum %d out of range\n", goalareanum );
		} //end if
		return qfalse;
	} //end if

	//make sure the routing cache doesn't grow to large
	while ( routingcachesize > max_routingcachesize ) {
		if ( !AAS_FreeOldestCache() ) {
			break;
		}
	}
	//
	if ( AAS_AreaDoNotEnter( areanum ) || AAS_AreaDoNotEnter( goalareanum ) ) {
		travelflags |= TFL_DONOTENTER;
	} //end if
	if ( AAS_AreaDoNotEnterLarge( areanum ) || AAS_AreaDoNotEnterLarge( goalareanum ) ) {
		travelflags |= TFL_DONOTENTER_LARGE;
	} //end if

#if !defined RTCW_ET
	  //NOTE: the number of routing updates is limited per frame
	  /*
	  if ((*aasworld).frameroutingupdates > MAX_FRAMEROUTINGUPDATES)
	  {
  #ifdef DEBUG
		  //Log_Write("WARNING: AAS_AreaTravelTimeToGoalArea: frame routing updates overflowed");
  #endif
		  return 0;
	  } //end if
	  */
	  //
	clusternum = ( *aasworld ).areasettings[areanum].cluster;
	goalclusternum = ( *aasworld ).areasettings[goalareanum].cluster;
	//check if the area is a portal of the goal area cluster
	if ( clusternum < 0 && goalclusternum > 0 ) {
		portal = &( *aasworld ).portals[-clusternum];
		if ( portal->frontcluster == goalclusternum ||
			 portal->backcluster == goalclusternum ) {
			clusternum = goalclusternum;
		} //end if
	} //end if
	  //check if the goalarea is a portal of the area cluster
	else if ( clusternum > 0 && goalclusternum < 0 ) {
		portal = &( *aasworld ).portals[-goalclusternum];
		if ( portal->frontcluster == clusternum ||
			 portal->backcluster == clusternum ) {
			goalclusternum = clusternum;
		} //end if
	} //end if
	  //if both areas are in the same cluster
	  //NOTE: there might be a shorter route via another cluster!!! but we don't care
	if ( clusternum > 0 && goalclusternum > 0 && clusternum == goalclusternum ) {
		//
#else
	  //
	clusternum = aasworld->areasettings[areanum].cluster;
	goalclusternum = aasworld->areasettings[goalareanum].cluster;

	//check if the area is a portal of the goal area cluster
	if ( clusternum < 0 && goalclusternum > 0 ) {
		portal = &aasworld->portals[-clusternum];
		if ( portal->frontcluster == goalclusternum || portal->backcluster == goalclusternum ) {
			clusternum = goalclusternum;
		}
	} else if ( clusternum > 0 && goalclusternum < 0 ) { //check if the goalarea is a portal of the area cluster
		portal = &aasworld->portals[-goalclusternum];
		if ( portal->frontcluster == clusternum || portal->backcluster == clusternum ) {
			goalclusternum = clusternum;
		}
	}

	//if both areas are in the same cluster
	//NOTE: there might be a shorter route via another cluster!!! but we don't care
	if ( clusternum > 0 && goalclusternum > 0 && clusternum == goalclusternum ) {
#endif // RTCW_XX

		areacache = AAS_GetAreaRoutingCache( clusternum, goalareanum, travelflags, qfalse );
		// RF, note that the routing cache might be NULL now since we are restricting
		// the updates per frame, hopefully rejected cache's will be requested again
		// when things have settled down
		if ( !areacache ) {
			return qfalse;
		}
		//the number of the area in the cluster
		clusterareanum = AAS_ClusterAreaNum( clusternum, areanum );
		//the cluster the area is in

#if !defined RTCW_ET
		cluster = &( *aasworld ).clusters[clusternum];
#else
		cluster = &aasworld->clusters[clusternum];
#endif // RTCW_XX

		//if the area is NOT a reachability area
		if ( clusterareanum >= cluster->numreachabilityareas ) {
			return qfalse;
		}
		//if it is possible to travel to the goal area through this cluster
		if ( areacache->traveltimes[clusterareanum] != 0 ) {

#if !defined RTCW_ET
			*reachnum = ( *aasworld ).areasettings[areanum].firstreachablearea +
#else
			*reachnum = aasworld->areasettings[areanum].firstreachablearea +
#endif // RTCW_XX

						areacache->reachabilities[clusterareanum];
			//
			if ( !origin ) {
				*traveltime = areacache->traveltimes[clusterareanum];
				return qtrue;
			}
			//

#if !defined RTCW_ET
			reach = &( *aasworld ).reachability[*reachnum];
#else
			reach = &aasworld->reachability[*reachnum];
#endif // RTCW_XX

			*traveltime = areacache->traveltimes[clusterareanum] +
						  AAS_AreaTravelTime( areanum, origin, reach->start );
			return qtrue;
		} //end if
	} //end if
	  //

#if !defined RTCW_ET
	clusternum = ( *aasworld ).areasettings[areanum].cluster;
	goalclusternum = ( *aasworld ).areasettings[goalareanum].cluster;
#else
	clusternum = aasworld->areasettings[areanum].cluster;
	goalclusternum = aasworld->areasettings[goalareanum].cluster;
#endif // RTCW_XX

	//if the goal area is a portal
	if ( goalclusternum < 0 ) {
		//just assume the goal area is part of the front cluster

#if !defined RTCW_ET
		portal = &( *aasworld ).portals[-goalclusternum];
#else
		portal = &aasworld->portals[-goalclusternum];
#endif // RTCW_XX

		goalclusternum = portal->frontcluster;
	} //end if
	  //get the portal routing cache
	portalcache = AAS_GetPortalRoutingCache( goalclusternum, goalareanum, travelflags );
	//if the area is a cluster portal, read directly from the portal cache
	if ( clusternum < 0 ) {
		*traveltime = portalcache->traveltimes[-clusternum];

#if !defined RTCW_ET
		*reachnum = ( *aasworld ).areasettings[areanum].firstreachablearea +
#else
		*reachnum = aasworld->areasettings[areanum].firstreachablearea +
#endif // RTCW_XX

					portalcache->reachabilities[-clusternum];
		return qtrue;
	}
	//
	besttime = 0;
	bestreachnum = -1;
	//the cluster the area is in

#if !defined RTCW_ET
	cluster = &( *aasworld ).clusters[clusternum];
#else
	cluster = &aasworld->clusters[clusternum];
#endif // RTCW_XX

	//current area inside the current cluster
	clusterareanum = AAS_ClusterAreaNum( clusternum, areanum );
	//if the area is NOT a reachability area
	if ( clusterareanum >= cluster->numreachabilityareas ) {
		return qfalse;
	}
	//

#if !defined RTCW_ET
	pPortalnum = ( *aasworld ).portalindex + cluster->firstportal;
#else
	pPortalnum = aasworld->portalindex + cluster->firstportal;
#endif // RTCW_XX

	//find the portal of the area cluster leading towards the goal area
	for ( i = 0; i < cluster->numportals; i++, pPortalnum++ )
	{
		portalnum = *pPortalnum;
		//if the goal area isn't reachable from the portal
		if ( !portalcache->traveltimes[portalnum] ) {
			continue;
		}
		//

#if !defined RTCW_ET
		portal = ( *aasworld ).portals + portalnum;
		// if the area in disabled
		if ( ( *aasworld ).areasettings[portal->areanum].areaflags & AREA_DISABLED ) {
#else
		portal = aasworld->portals + portalnum;
		// if the area in disabled
		if ( aasworld->areasettings[portal->areanum].areaflags & AREA_DISABLED ) {
			continue;
		}
		// if there is no reachability out of the area
		if ( !aasworld->areasettings[portal->areanum].numreachableareas ) {
#endif // RTCW_XX

			continue;
		}
		//get the cache of the portal area
		areacache = AAS_GetAreaRoutingCache( clusternum, portal->areanum, travelflags, qfalse );
		// RF, this may be NULL if we were unable to calculate the cache this frame
		if ( !areacache ) {
			return qfalse;
		}
		//if the portal is NOT reachable from this area
		if ( !areacache->traveltimes[clusterareanum] ) {
			continue;
		}
		//total travel time is the travel time the portal area is from
		//the goal area plus the travel time towards the portal area
		t = portalcache->traveltimes[portalnum] + areacache->traveltimes[clusterareanum];
		//FIXME: add the exact travel time through the actual portal area
		//NOTE: for now we just add the largest travel time through the area portal
		//		because we can't directly calculate the exact travel time
		//		to be more specific we don't know which reachability is used to travel
		//		into the portal area when coming from the current area

#if !defined RTCW_ET
		t += ( *aasworld ).portalmaxtraveltimes[portalnum];
#else
		t += aasworld->portalmaxtraveltimes[portalnum];
#endif // RTCW_XX

		//
		// Ridah, needs to be up here

#if !defined RTCW_ET
		*reachnum = ( *aasworld ).areasettings[areanum].firstreachablearea +
#else
		*reachnum = aasworld->areasettings[areanum].firstreachablearea +
#endif // RTCW_XX

					areacache->reachabilities[clusterareanum];

//botimport.Print(PRT_MESSAGE, "portal reachability: %i\n", (int)areacache->reachabilities[clusterareanum] );

		if ( origin ) {

#if !defined RTCW_ET
			reach = ( *aasworld ).reachability + *reachnum;
#else
			reach = aasworld->reachability + *reachnum;
#endif // RTCW_XX

			t += AAS_AreaTravelTime( areanum, origin, reach->start );
		} //end if
		  //if the time is better than the one already found
		if ( !besttime || t < besttime ) {
			bestreachnum = *reachnum;
			besttime = t;
		} //end if
	} //end for
	  // Ridah, check a route was found
	if ( bestreachnum < 0 ) {
		return qfalse;
	}
	*reachnum = bestreachnum;
	*traveltime = besttime;
	return qtrue;
} //end of the function AAS_AreaRouteToGoalArea
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_AreaTravelTimeToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags ) {
	int traveltime, reachnum;

	if ( AAS_AreaRouteToGoalArea( areanum, origin, goalareanum, travelflags, &traveltime, &reachnum ) ) {
		return traveltime;
	}
	return 0;
} //end of the function AAS_AreaTravelTimeToGoalArea

#if !defined RTCW_MP
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_AreaTravelTimeToGoalAreaCheckLoop( int areanum, vec3_t origin, int goalareanum, int travelflags, int loopareanum ) {
	int traveltime, reachnum;
	aas_reachability_t *reach;

	if ( AAS_AreaRouteToGoalArea( areanum, origin, goalareanum, travelflags, &traveltime, &reachnum ) ) {

#if !defined RTCW_ET
		reach = &( *aasworld ).reachability[reachnum];
#else
		reach = &aasworld->reachability[reachnum];
#endif // RTCW_XX

		if ( loopareanum && reach->areanum == loopareanum ) {
			return 0;   // going here will cause a looped route
		}
		return traveltime;
	}
	return 0;
} //end of the function AAS_AreaTravelTimeToGoalArea
#endif // RTCW_XX

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_AreaReachabilityToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags ) {
	int traveltime, reachnum;

	if ( AAS_AreaRouteToGoalArea( areanum, origin, goalareanum, travelflags, &traveltime, &reachnum ) ) {
		return reachnum;
	}
	return 0;
} //end of the function AAS_AreaReachabilityToGoalArea
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_ReachabilityFromNum( int num, struct aas_reachability_s *reach ) {

#if !defined RTCW_ET
	if ( !( *aasworld ).initialized ) {
		memset( reach, 0, sizeof( aas_reachability_t ) );
		return;
	} //end if
	if ( num < 0 || num >= ( *aasworld ).reachabilitysize ) {
		memset( reach, 0, sizeof( aas_reachability_t ) );
		return;
	} //end if
	memcpy( reach, &( *aasworld ).reachability[num], sizeof( aas_reachability_t ) );;
#else
	if ( !aasworld->initialized ) {
		memset( reach, 0, sizeof( aas_reachability_t ) );
		return;
	} //end if
	if ( num < 0 || num >= aasworld->reachabilitysize ) {
		memset( reach, 0, sizeof( aas_reachability_t ) );
		return;
	} //end if
	memcpy( reach, &aasworld->reachability[num], sizeof( aas_reachability_t ) );;
#endif // RTCW_XX

} //end of the function AAS_ReachabilityFromNum
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_NextAreaReachability( int areanum, int reachnum ) {
	aas_areasettings_t *settings;

#if !defined RTCW_ET
	if ( !( *aasworld ).initialized ) {
		return 0;
	}

	if ( areanum <= 0 || areanum >= ( *aasworld ).numareas ) {
		botimport.Print( PRT_ERROR, "AAS_NextAreaReachability: areanum %d out of range\n", areanum );
		return 0;
	} //end if

	settings = &( *aasworld ).areasettings[areanum];
#else
	if ( !aasworld->initialized ) {
		return 0;
	}

	if ( areanum <= 0 || areanum >= aasworld->numareas ) {
		botimport.Print( PRT_ERROR, "AAS_NextAreaReachability: areanum %d out of range\n", areanum );
		return 0;
	} //end if

	settings = &aasworld->areasettings[areanum];
#endif // RTCW_XX

	if ( !reachnum ) {
		return settings->firstreachablearea;
	} //end if
	if ( reachnum < settings->firstreachablearea ) {
		botimport.Print( PRT_FATAL, "AAS_NextAreaReachability: reachnum < settings->firstreachableara" );
		return 0;
	} //end if
	reachnum++;
	if ( reachnum >= settings->firstreachablearea + settings->numreachableareas ) {
		return 0;
	} //end if
	return reachnum;
} //end of the function AAS_NextAreaReachability
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_NextModelReachability( int num, int modelnum ) {
	int i;

	if ( num <= 0 ) {
		num = 1;

#if !defined RTCW_ET
	} else if ( num >= ( *aasworld ).reachabilitysize )  {
		return 0;
	} else { num++;}
	//
	for ( i = num; i < ( *aasworld ).reachabilitysize; i++ )
	{
		if ( ( *aasworld ).reachability[i].traveltype == TRAVEL_ELEVATOR ) {
			if ( ( *aasworld ).reachability[i].facenum == modelnum ) {
				return i;
			}
		} //end if
		else if ( ( *aasworld ).reachability[i].traveltype == TRAVEL_FUNCBOB ) {
			if ( ( ( *aasworld ).reachability[i].facenum & 0x0000FFFF ) == modelnum ) {
#else
	} else if ( num >= aasworld->reachabilitysize ) {
		return 0;
	} else { num++;}
	//
	for ( i = num; i < aasworld->reachabilitysize; i++ )
	{
		if ( aasworld->reachability[i].traveltype == TRAVEL_ELEVATOR ) {
			if ( aasworld->reachability[i].facenum == modelnum ) {
				return i;
			}
		} //end if
		else if ( aasworld->reachability[i].traveltype == TRAVEL_FUNCBOB ) {
			if ( ( aasworld->reachability[i].facenum & 0x0000FFFF ) == modelnum ) {
#endif // RTCW_XX

				return i;
			}
		} //end if
	} //end for
	return 0;
} //end of the function AAS_NextModelReachability
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_RandomGoalArea( int areanum, int travelflags, int *goalareanum, vec3_t goalorigin ) {
	int i, n, t;
	vec3_t start, end;
	aas_trace_t trace;

	//if the area has no reachabilities
	if ( !AAS_AreaReachability( areanum ) ) {
		return qfalse;
	}
	//

#if !defined RTCW_ET
	n = ( *aasworld ).numareas * random();
	for ( i = 0; i < ( *aasworld ).numareas; i++ )
	{
		if ( n <= 0 ) {
			n = 1;
		}
		if ( n >= ( *aasworld ).numareas ) {
			n = 1;
		}
		if ( AAS_AreaReachability( n ) ) {
			t = AAS_AreaTravelTimeToGoalArea( areanum, ( *aasworld ).areas[areanum].center, n, travelflags );
			//if the goal is reachable
			if ( t > 0 ) {
				if ( AAS_AreaSwim( n ) ) {
					*goalareanum = n;
					VectorCopy( ( *aasworld ).areas[n].center, goalorigin );
					//botimport.Print(PRT_MESSAGE, "found random goal area %d\n", *goalareanum);
					return qtrue;
				} //end if
				VectorCopy( ( *aasworld ).areas[n].center, start );
#else
	n = aasworld->numareas * random();
	for ( i = 0; i < aasworld->numareas; i++ )
	{
		if ( n <= 0 ) {
			n = 1;
		}
		if ( n >= aasworld->numareas ) {
			n = 1;
		}
		if ( AAS_AreaReachability( n ) ) {
			t = AAS_AreaTravelTimeToGoalArea( areanum, aasworld->areas[areanum].center, n, travelflags );
			//if the goal is reachable
			if ( t > 0 ) {
				if ( AAS_AreaSwim( n ) ) {
					*goalareanum = n;
					VectorCopy( aasworld->areas[n].center, goalorigin );
					//botimport.Print(PRT_MESSAGE, "found random goal area %d\n", *goalareanum);
					return qtrue;
				} //end if
				VectorCopy( aasworld->areas[n].center, start );
#endif // RTCW_XX

				if ( !AAS_PointAreaNum( start ) ) {
					Log_Write( "area %d center %f %f %f in solid?", n,
							   start[0], start[1], start[2] );
				}
				VectorCopy( start, end );
				end[2] -= 300;
				trace = AAS_TraceClientBBox( start, end, PRESENCE_CROUCH, -1 );
				if ( !trace.startsolid && AAS_PointAreaNum( trace.endpos ) == n ) {
					if ( AAS_AreaGroundFaceArea( n ) > 300 ) {
						*goalareanum = n;
						VectorCopy( trace.endpos, goalorigin );
						//botimport.Print(PRT_MESSAGE, "found random goal area %d\n", *goalareanum);
						return qtrue;
					} //end if
				} //end if
			} //end if
		} //end if
		n++;
	} //end for
	return qfalse;
} //end of the function AAS_RandomGoalArea
//===========================================================================
// run-length compression on zeros
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_CompressVis( byte *vis, int numareas, byte *dest ) {
	int j;
	int rep;
	//int		visrow;
	byte    *dest_p;
	byte check;

	//
	dest_p = dest;
	//visrow = (numareas + 7)>>3;

	for ( j = 0 ; j < numareas /*visrow*/ ; j++ )
	{
		*dest_p++ = vis[j];
		check = vis[j];
		//if (vis[j])
		//	continue;

		rep = 1;
		for ( j++; j < numareas /*visrow*/ ; j++ )
			if ( vis[j] != check || rep == 255 ) {
				break;
			} else {
				rep++;
			}
		*dest_p++ = rep;
		j--;
	}
	return dest_p - dest;
} //end of the function AAS_CompressVis
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_DecompressVis( byte *in, int numareas, byte *decompressed ) {
	byte c;
	byte    *out;
	//int		row;
	byte    *end;

	// initialize the vis data, only set those that are visible
	memset( decompressed, 0, numareas );

	//row = (numareas+7)>>3;
	out = decompressed;

    //BBi
	//end = ( byte * )( (int)decompressed + numareas );
    end = decompressed + numareas;
    //BBi

	do
	{
		/*
		if (*in)
		{
			*out++ = *in++;
			continue;
		}
		*/

		c = in[1];
		if ( !c ) {
			AAS_Error( "DecompressVis: 0 repeat" );
		}
		if ( *in ) { // we need to set these bits
			memset( out, 1, c );
		}
		in += 2;
		/*
		while (c)
		{
			*out++ = 0;
			c--;
		}
		*/
		out += c;
	} while ( out < end );
} //end of the function AAS_DecompressVis
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_AreaVisible( int srcarea, int destarea ) {

#if !defined RTCW_ET
	if ( srcarea != ( *aasworld ).decompressedvisarea ) {
		if ( !( *aasworld ).areavisibility[srcarea] ) {
			return qfalse;
		}
		AAS_DecompressVis( ( *aasworld ).areavisibility[srcarea],
						   ( *aasworld ).numareas, ( *aasworld ).decompressedvis );
		( *aasworld ).decompressedvisarea = srcarea;
	}
	return ( *aasworld ).decompressedvis[destarea];
#else
	if ( !aasworld->areavisibility ) {
//		botimport.Print(PRT_MESSAGE, "AAS_AreaVisible: no vis data available, returning qtrue\n" );
		return qtrue;
	}
	if ( srcarea != aasworld->decompressedvisarea ) {
		if ( !aasworld->areavisibility[srcarea] ) {
			return qfalse;
		}
		AAS_DecompressVis( aasworld->areavisibility[srcarea],
						   aasworld->numareas, aasworld->decompressedvis );
		aasworld->decompressedvisarea = srcarea;
	}
	return aasworld->decompressedvis[destarea];
#endif // RTCW_XX

} //end of the function AAS_AreaVisible
//===========================================================================
// just center to center visibility checking...
// FIXME: implement a correct full vis
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================

#if !defined RTCW_ET
void AAS_CreateVisibility( void ) {
	int i, j, size, totalsize;
	vec3_t endpos, mins, maxs;
	bsp_trace_t trace;
	byte *buf;
	byte *validareas;
	int numvalid = 0;

#if defined RTCW_SP
	byte *areaTable = NULL;
	int numAreas, numAreaBits;

	numAreas = ( *aasworld ).numareas;
	numAreaBits = ( ( numAreas + 8 ) >> 3 );
	areaTable = (byte *) GetClearedMemory( numAreas * numAreaBits * sizeof( byte ) );

	buf = (byte *) GetClearedMemory( numAreas * 2 * sizeof( byte ) );   // in case it ends up bigger than the decompressedvis, which is rare but possible
	validareas = (byte *) GetClearedMemory( numAreas * sizeof( byte ) );

	( *aasworld ).areavisibility = (byte **) GetClearedMemory( numAreas * sizeof( byte * ) );
	( *aasworld ).decompressedvis = (byte *) GetClearedMemory( numAreas * sizeof( byte ) );
	( *aasworld ).areawaypoints = (vec3_t *) GetClearedMemory( numAreas * sizeof( vec3_t ) );
	totalsize = numAreas * sizeof( byte * );
	for ( i = 1; i < numAreas; i++ )
#elif defined RTCW_MP
	buf = (byte *) GetClearedMemory( ( *aasworld ).numareas * 2 * sizeof( byte ) );   // in case it ends up bigger than the decompressedvis, which is rare but possible
	validareas = (byte *) GetClearedMemory( ( *aasworld ).numareas * sizeof( byte ) );

	( *aasworld ).areavisibility = (byte **) GetClearedMemory( ( *aasworld ).numareas * sizeof( byte * ) );
	( *aasworld ).decompressedvis = (byte *) GetClearedMemory( ( *aasworld ).numareas * sizeof( byte ) );
	( *aasworld ).areawaypoints = (vec3_t *) GetClearedMemory( ( *aasworld ).numareas * sizeof( vec3_t ) );
	totalsize = ( *aasworld ).numareas * sizeof( byte * );
	for ( i = 1; i < ( *aasworld ).numareas; i++ )
#endif // RTCW_XX

	{
		if ( !AAS_AreaReachability( i ) ) {
			continue;
		}

		// find the waypoint
		VectorCopy( ( *aasworld ).areas[i].center, endpos );
		endpos[2] -= 256;
		AAS_PresenceTypeBoundingBox( PRESENCE_NORMAL, mins, maxs );
//		maxs[2] = 0;

#if defined RTCW_SP
		trace = AAS_Trace( ( *aasworld ).areas[i].center, mins, maxs, endpos, -1, CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP );
#elif defined RTCW_MP
		trace = AAS_Trace( ( *aasworld ).areas[i].center, mins, maxs, endpos, -1, CONTENTS_SOLID );
#endif // RTCW_XX

		if ( !trace.startsolid && trace.fraction < 1 && AAS_PointAreaNum( trace.endpos ) == i ) {
			VectorCopy( trace.endpos, ( *aasworld ).areawaypoints[i] );
			validareas[i] = 1;
			numvalid++;
		} else {
			continue;
		}
	}

#if defined RTCW_SP
	for ( i = 1; i < numAreas; i++ )
#elif defined RTCW_MP
	for ( i = 1; i < ( *aasworld ).numareas; i++ )
#endif // RTCW_XX

	{
		if ( !validareas[i] ) {
			continue;
		}

#if defined RTCW_MP
		if ( !AAS_AreaReachability( i ) ) {
			continue;
		}
#endif // RTCW_XX

#if defined RTCW_SP
		for ( j = 1; j < numAreas; j++ )
#elif defined RTCW_MP
		for ( j = 1; j < ( *aasworld ).numareas; j++ )
#endif // RTCW_XX

		{

#if defined RTCW_SP
			( *aasworld ).decompressedvis[j] = 0;
#endif // RTCW_XX

			if ( i == j ) {
				( *aasworld ).decompressedvis[j] = 1;

#if defined RTCW_SP
				if ( areaTable ) {
					areaTable[ ( i * numAreaBits ) + ( j >> 3 ) ] |= ( 1 << ( j & 7 ) );
				}
#endif // RTCW_XX

				continue;
			}

#if defined RTCW_SP
			if ( !validareas[j] ) {
#elif defined RTCW_MP
			if ( !validareas[j] || !AAS_AreaReachability( j ) ) {
				( *aasworld ).decompressedvis[j] = 0;
#endif // RTCW_XX

				continue;
			} //end if

#if defined RTCW_SP
			  // if we have already checked this combination, copy the result
			if ( areaTable && ( i > j ) ) {
				// use the reverse result stored in the table
				if ( areaTable[ ( j * numAreaBits ) + ( i >> 3 ) ] & ( 1 << ( i & 7 ) ) ) {
					( *aasworld ).decompressedvis[j] = 1;
				}
				// done, move to the next area
				continue;
			}
#endif // RTCW_XX

#if defined RTCW_SP
			// RF, check PVS first, since it's much faster
			if ( !AAS_inPVS( ( *aasworld ).areawaypoints[i], ( *aasworld ).areawaypoints[j] ) ) {
				continue;
			}
#elif defined RTCW_MP
			// Ridah, this always returns false?!
			//if (AAS_inPVS( (*aasworld).areawaypoints[i], (*aasworld).areawaypoints[j] ))
#endif // RTCW_XX

			trace = AAS_Trace( ( *aasworld ).areawaypoints[i], NULL, NULL, ( *aasworld ).areawaypoints[j], -1, CONTENTS_SOLID );
			if ( trace.fraction >= 1 ) {

#if defined RTCW_MP
				//if (botimport.inPVS( (*aasworld).areawaypoints[i], (*aasworld).areawaypoints[j] ))
#endif // RTCW_XX

				( *aasworld ).decompressedvis[j] = 1;
			} //end if

#if defined RTCW_MP
			else
			{
				( *aasworld ).decompressedvis[j] = 0;
			} //end else
#endif // RTCW_XX

		} //end for

#if defined RTCW_SP
		size = AAS_CompressVis( ( *aasworld ).decompressedvis, numAreas, buf );
#elif defined RTCW_MP
		size = AAS_CompressVis( ( *aasworld ).decompressedvis, ( *aasworld ).numareas, buf );
#endif // RTCW_XX

		( *aasworld ).areavisibility[i] = (byte *) GetMemory( size );
		memcpy( ( *aasworld ).areavisibility[i], buf, size );
		totalsize += size;
	} //end for

#if defined RTCW_SP
	if ( areaTable ) {
		FreeMemory( areaTable );
	}
#endif // RTCW_XX

	botimport.Print( PRT_MESSAGE, "AAS_CreateVisibility: compressed vis size = %i\n", totalsize );
} //end of the function AAS_CreateVisibility
#else
void AAS_CreateVisibility( qboolean waypointsOnly ) {
	int i, j, size, totalsize;
	vec3_t endpos, mins, maxs;
	bsp_trace_t trace;
	byte *buf;
	byte *validareas = NULL;
	int numvalid = 0;
	byte *areaTable = NULL;
	int numAreas, numAreaBits;

	numAreas = aasworld->numareas;
	numAreaBits = ( ( numAreas + 8 ) >> 3 );

	if ( !waypointsOnly ) {
		validareas = (byte *) GetClearedMemory( numAreas * sizeof( byte ) );
	}

	aasworld->areawaypoints = (vec3_t *) GetClearedMemory( numAreas * sizeof( vec3_t ) );
	totalsize = numAreas * sizeof( byte * );

	for ( i = 1; i < numAreas; i++ ) {
		if ( !AAS_AreaReachability( i ) ) {
			continue;
		}

		// find the waypoint
		VectorCopy( aasworld->areas[i].center, endpos );
		endpos[2] -= 256;
		AAS_PresenceTypeBoundingBox( PRESENCE_NORMAL, mins, maxs );
		trace = AAS_Trace( aasworld->areas[i].center, mins, maxs, endpos, -1, CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP );
		if ( trace.startsolid && trace.ent < ENTITYNUM_WORLD ) {
			trace = AAS_Trace( aasworld->areas[i].center, mins, maxs, endpos, trace.ent, CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP );
		}
		if ( !trace.startsolid && trace.fraction < 1 && AAS_PointAreaNum( trace.endpos ) == i ) {
			VectorCopy( trace.endpos, aasworld->areawaypoints[i] );

			if ( !waypointsOnly ) {
				validareas[i] = 1;
			}

			numvalid++;
		} else {
			VectorClear( aasworld->areawaypoints[i] );
		}
	}

	if ( waypointsOnly ) {
		return;
	}

	aasworld->areavisibility = (byte **) GetClearedMemory( numAreas * sizeof( byte * ) );

	aasworld->decompressedvis = (byte *) GetClearedMemory( numAreas * sizeof( byte ) );

	areaTable = (byte *) GetClearedMemory( numAreas * numAreaBits * sizeof( byte ) );

	buf = (byte *) GetClearedMemory( numAreas * 2 * sizeof( byte ) );   // in case it ends up bigger than the decompressedvis, which is rare but possible

	for ( i = 1; i < numAreas; i++ ) {
		if ( !validareas[i] ) {
			continue;
		}

		for ( j = 1; j < numAreas; j++ ) {
			aasworld->decompressedvis[j] = 0;
			if ( i == j ) {
				aasworld->decompressedvis[j] = 1;
				if ( areaTable ) {
					areaTable[ ( i * numAreaBits ) + ( j >> 3 ) ] |= ( 1 << ( j & 7 ) );
				}
				continue;
			}

			if ( !validareas[j] ) {
				continue;
			}

			// if we have already checked this combination, copy the result
			if ( areaTable && ( i > j ) ) {
				// use the reverse result stored in the table
				if ( areaTable[ ( j * numAreaBits ) + ( i >> 3 ) ] & ( 1 << ( i & 7 ) ) ) {
					aasworld->decompressedvis[j] = 1;
				}
				// done, move to the next area
				continue;
			}

			// RF, check PVS first, since it's much faster
			if ( !AAS_inPVS( aasworld->areawaypoints[i], aasworld->areawaypoints[j] ) ) {
				continue;
			}

			trace = AAS_Trace( aasworld->areawaypoints[i], NULL, NULL, aasworld->areawaypoints[j], -1, CONTENTS_SOLID );
			if ( trace.startsolid && trace.ent < ENTITYNUM_WORLD ) {
				trace = AAS_Trace( aasworld->areas[i].center, mins, maxs, endpos, trace.ent, CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP );
			}

			if ( trace.fraction >= 1 ) {
				if ( areaTable ) {
					areaTable[ ( i * numAreaBits ) + ( j >> 3 ) ] |= ( 1 << ( j & 7 ) );
				}
				aasworld->decompressedvis[j] = 1;
			}
		}

		size = AAS_CompressVis( aasworld->decompressedvis, numAreas, buf );
		aasworld->areavisibility[i] = (byte *) GetMemory( size );
		memcpy( aasworld->areavisibility[i], buf, size );
		totalsize += size;
	}

	if ( areaTable ) {
		FreeMemory( areaTable );
	}

	botimport.Print( PRT_MESSAGE, "AAS_CreateVisibility: compressed vis size = %i\n", totalsize );
}
#endif // RTCW_XX

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
float VectorDistance( vec3_t v1, vec3_t v2 );
extern void ProjectPointOntoVector( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj ) ;

#if !defined RTCW_ET
int AAS_NearestHideArea( int srcnum, vec3_t origin, int areanum, int enemynum, vec3_t enemyorigin, int enemyareanum, int travelflags ) {
#else
int AAS_NearestHideArea( int srcnum, vec3_t origin, int areanum, int enemynum, vec3_t enemyorigin, int enemyareanum, int travelflags, float maxdist, vec3_t distpos ) {
#endif // RTCW_XX

	int i, j, nextareanum, badtravelflags, numreach, bestarea;
	unsigned short int t, besttraveltime, enemytraveltime;
	aas_routingupdate_t *updateliststart, *updatelistend, *curupdate, *nextupdate;
	aas_reachability_t *reach;
	float dist1, dist2;
	float enemytraveldist;
	vec3_t enemyVec;
	qboolean startVisible;
	vec3_t v1, v2, p;

#if !defined RTCW_MP
	#define MAX_HIDEAREA_LOOPS  3000
	static float lastTime;
	static int loopCount;
#else
	int count = 0;
	#define MAX_HIDEAREA_LOOPS  4000
#endif // RTCW_XX

	//

#if defined RTCW_ET
	if ( !aasworld->areavisibility ) {
		return 0;
	}
	//
#endif // RTCW_XX

#if !defined RTCW_MP
	if ( srcnum < 0 ) {   // hack to force run this call
		srcnum = -srcnum - 1;
		lastTime = 0;
	}
#endif // RTCW_XX

	// don't run this more than once per frame

#if !defined RTCW_MP
	if ( lastTime == AAS_Time() && loopCount >= MAX_HIDEAREA_LOOPS ) {
#else
	static float lastTime;
	if ( lastTime == AAS_Time() ) {
#endif // RTCW_XX

		return 0;
	}

#if !defined RTCW_ET
	lastTime = AAS_Time();

#if defined RTCW_SP
	loopCount = 0;
#endif // RTCW_XX

	//
	if ( !( *aasworld ).hidetraveltimes ) {
		( *aasworld ).hidetraveltimes = (unsigned short int *) GetClearedMemory( ( *aasworld ).numareas * sizeof( unsigned short int ) );
	} else {
		memset( ( *aasworld ).hidetraveltimes, 0, ( *aasworld ).numareas * sizeof( unsigned short int ) );
	} //end else
	  //
	if ( !( *aasworld ).visCache ) {
		( *aasworld ).visCache = (byte *) GetClearedMemory( ( *aasworld ).numareas * sizeof( byte ) );
	} else {
		memset( ( *aasworld ).visCache, 0, ( *aasworld ).numareas * sizeof( byte ) );
#else
	if ( lastTime != AAS_Time() ) {
		loopCount = 0;
	}
	lastTime = AAS_Time();
	//
	if ( !aasworld->hidetraveltimes ) {
		aasworld->hidetraveltimes = (unsigned short int *) GetClearedMemory( aasworld->numareas * sizeof( unsigned short int ) );
	} else {
		memset( aasworld->hidetraveltimes, 0, aasworld->numareas * sizeof( unsigned short int ) );
	} //end else
	  //
	if ( !aasworld->visCache ) {
		aasworld->visCache = (byte *) GetClearedMemory( aasworld->numareas * sizeof( byte ) );
	} else {
		memset( aasworld->visCache, 0, aasworld->numareas * sizeof( byte ) );
#endif // RTCW_XX

	} //end else
	besttraveltime = 0;
	bestarea = 0;
	if ( enemyareanum ) {
		enemytraveltime = AAS_AreaTravelTimeToGoalArea( areanum, origin, enemyareanum, travelflags );
	}
	VectorSubtract( enemyorigin, origin, enemyVec );
	enemytraveldist = VectorNormalize( enemyVec );

#if !defined RTCW_ET
	startVisible = botimport.AICast_VisibleFromPos( enemyorigin, enemynum, origin, srcnum, qfalse );
	//
	badtravelflags = ~travelflags;
	//
	curupdate = &( *aasworld ).areaupdate[areanum];
	curupdate->areanum = areanum;
	VectorCopy( origin, curupdate->start );
	curupdate->areatraveltimes = ( *aasworld ).areatraveltimes[areanum][0];
#else
	startVisible = botimport.BotVisibleFromPos( enemyorigin, enemynum, origin, srcnum, qfalse );
	//
	badtravelflags = ~travelflags;
	//
	curupdate = &aasworld->areaupdate[areanum];
	curupdate->areanum = areanum;
	VectorCopy( origin, curupdate->start );
	// GORDON: TEMP: FIXME: just avoiding a crash for the moment
	if ( areanum == 0 ) {
		return 0;
	}
	curupdate->areatraveltimes = aasworld->areatraveltimes[areanum][0];
#endif // RTCW_XX

	curupdate->tmptraveltime = 0;
	//put the area to start with in the current read list
	curupdate->next = NULL;
	curupdate->prev = NULL;
	updateliststart = curupdate;
	updatelistend = curupdate;
	//while there are updates in the current list, flip the lists
	while ( updateliststart )
	{
		curupdate = updateliststart;
		//
		if ( curupdate->next ) {
			curupdate->next->prev = NULL;
		} else { updatelistend = NULL;}
		updateliststart = curupdate->next;
		//
		curupdate->inlist = qfalse;
		//check all reversed reachability links

#if !defined RTCW_ET
		numreach = ( *aasworld ).areasettings[curupdate->areanum].numreachableareas;
		reach = &( *aasworld ).reachability[( *aasworld ).areasettings[curupdate->areanum].firstreachablearea];
		//
		for ( i = 0; i < numreach; i++, reach++ )
		{
			//if an undesired travel type is used
			if ( ( *aasworld ).travelflagfortype[reach->traveltype] & badtravelflags ) {
				continue;
			}
			//
			if ( AAS_AreaContentsTravelFlag( reach->areanum ) & badtravelflags ) {
				continue;
			}

#if defined RTCW_SP
			// dont pass through ladder areas
			if ( ( *aasworld ).areasettings[reach->areanum].areaflags & AREA_LADDER ) {
				continue;
			}
#endif // RTCW_XX

			//
			if ( ( *aasworld ).areasettings[reach->areanum].areaflags & AREA_DISABLED ) {
#else
		numreach = aasworld->areasettings[curupdate->areanum].numreachableareas;
		reach = &aasworld->reachability[aasworld->areasettings[curupdate->areanum].firstreachablearea];
		//
		for ( i = 0; i < numreach; i++, reach++ )
		{
			//if an undesired travel type is used
			if ( aasworld->travelflagfortype[reach->traveltype] & badtravelflags ) {
				continue;
			}
			//
			if ( AAS_AreaContentsTravelFlag( reach->areanum ) & badtravelflags ) {
				continue;
			}
			// dont pass through ladder areas
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_LADDER ) {
				continue;
			}
			//
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_DISABLED ) {
#endif // RTCW_XX

				continue;
			}
			//number of the area the reachability leads to
			nextareanum = reach->areanum;
			// if this moves us into the enemies area, skip it
			if ( nextareanum == enemyareanum ) {
				continue;
			}

#if defined RTCW_ET
			// if this moves us outside maxdist
			if ( distpos && ( VectorDistance( reach->end, distpos ) > maxdist ) ) {
				continue;
			}
#endif // RTCW_XX

			//time already travelled plus the traveltime through
			//the current area plus the travel time from the reachability
			t = curupdate->tmptraveltime +
				AAS_AreaTravelTime( curupdate->areanum, curupdate->start, reach->start ) +
				reach->traveltime;

#if !defined RTCW_MP
			// inc the loopCount, we are starting to use a bit of cpu time
			loopCount++;
#endif // RTCW_XX

			// if this isn't the fastest route to this area, ignore

#if !defined RTCW_ET
			if ( ( *aasworld ).hidetraveltimes[nextareanum] && ( *aasworld ).hidetraveltimes[nextareanum] < t ) {
				continue;
			}
			( *aasworld ).hidetraveltimes[nextareanum] = t;
#else
			if ( aasworld->hidetraveltimes[nextareanum] && aasworld->hidetraveltimes[nextareanum] < t ) {
				continue;
			}
			aasworld->hidetraveltimes[nextareanum] = t;
#endif // RTCW_XX

			// if the bestarea is this area, then it must be a longer route, so ignore it
			if ( bestarea == nextareanum ) {
				bestarea = 0;
				besttraveltime = 0;
			}
			// do this test now, so we can reject the route if it starts out too long
			if ( besttraveltime && t >= besttraveltime ) {
				continue;
			}
			//
			//avoid going near the enemy
			ProjectPointOntoVector( enemyorigin, curupdate->start, reach->end, p );
			for ( j = 0; j < 3; j++ ) {
				if ( ( p[j] > curupdate->start[j] + 0.1 && p[j] > reach->end[j] + 0.1 ) ||
					 ( p[j] < curupdate->start[j] - 0.1 && p[j] < reach->end[j] - 0.1 ) ) {
					break;
				}
			}
			if ( j < 3 ) {
				VectorSubtract( enemyorigin, reach->end, v2 );
			} //end if
			else
			{
				VectorSubtract( enemyorigin, p, v2 );
			} //end else
			dist2 = VectorLength( v2 );
			//never go through the enemy
			if ( enemytraveldist > 32 && dist2 < enemytraveldist && dist2 < 256 ) {
				continue;
			}
			//
			VectorSubtract( reach->end, origin, v2 );
			if ( enemytraveldist > 32 && DotProduct( v2, enemyVec ) > enemytraveldist / 2 ) {
				continue;
			}
			//
			VectorSubtract( enemyorigin, curupdate->start, v1 );
			dist1 = VectorLength( v1 );
			//
			if ( enemytraveldist > 32 && dist2 < dist1 ) {
				t += ( dist1 - dist2 ) * 10;
				// test it again after modifying it
				if ( besttraveltime && t >= besttraveltime ) {
					continue;
				}
			}
			// make sure the hide area doesn't have anyone else in it
			if ( AAS_IsEntityInArea( srcnum, -1, nextareanum ) ) {
				t += 1000;  // avoid this path/area
				//continue;
			}
			//
			// if we weren't visible when starting, make sure we don't move into their view
			if ( enemyareanum && !startVisible && AAS_AreaVisible( enemyareanum, nextareanum ) ) {
				continue;
				//t += 1000;
			}
			//
			if ( !besttraveltime || besttraveltime > t ) {
				//
				// if this area doesn't have a vis list, ignore it

#if !defined RTCW_ET
				if ( ( *aasworld ).areavisibility[nextareanum] ) {
					//if the nextarea is not visible from the enemy area
					if ( !AAS_AreaVisible( enemyareanum, nextareanum ) ) { // now last of all, check that this area is a safe hiding spot
						if (    ( ( *aasworld ).visCache[nextareanum] == 2 ) ||
								( !( *aasworld ).visCache[nextareanum] && !botimport.AICast_VisibleFromPos( enemyorigin, enemynum, ( *aasworld ).areawaypoints[nextareanum], srcnum, qfalse ) ) ) {
							( *aasworld ).visCache[nextareanum] = 2;
							besttraveltime = t;
							bestarea = nextareanum;
						} else {
							( *aasworld ).visCache[nextareanum] = 1;
#else
				if ( aasworld->areavisibility[nextareanum] ) {
					//if the nextarea is not visible from the enemy area
					if ( !AAS_AreaVisible( enemyareanum, nextareanum ) ) { // now last of all, check that this area is a safe hiding spot
						if (    ( aasworld->visCache[nextareanum] == 2 ) ||
								( !aasworld->visCache[nextareanum] && !botimport.BotVisibleFromPos( enemyorigin, enemynum, aasworld->areawaypoints[nextareanum], srcnum, qfalse ) ) ) {
							aasworld->visCache[nextareanum] = 2;
							besttraveltime = t;
							bestarea = nextareanum;
						} else {
							aasworld->visCache[nextareanum] = 1;
#endif // RTCW_XX

						}
					} //end if
				}
				//
				// getting down to here is bad for cpu usage

#if !defined RTCW_MP
				if ( loopCount++ > MAX_HIDEAREA_LOOPS ) {
#else
				if ( count++ > MAX_HIDEAREA_LOOPS ) {
#endif // RTCW_XX

					//botimport.Print(PRT_MESSAGE, "AAS_NearestHideArea: exceeded max loops, aborting\n" );
					continue;
				}
				//
				// otherwise, add this to the list so we check is reachables
				// disabled, this should only store the raw traveltime, not the adjusted time

#if !defined RTCW_ET
				//(*aasworld).hidetraveltimes[nextareanum] = t;
				nextupdate = &( *aasworld ).areaupdate[nextareanum];
#else
				//aasworld->hidetraveltimes[nextareanum] = t;
				nextupdate = &aasworld->areaupdate[nextareanum];
#endif // RTCW_XX

				nextupdate->areanum = nextareanum;
				nextupdate->tmptraveltime = t;
				//remember where we entered this area
				VectorCopy( reach->end, nextupdate->start );
				//if this update is not in the list yet
				if ( !nextupdate->inlist ) {
					//add the new update to the end of the list
					nextupdate->next = NULL;
					nextupdate->prev = updatelistend;
					if ( updatelistend ) {
						updatelistend->next = nextupdate;
					} else { updateliststart = nextupdate;}
					updatelistend = nextupdate;
					nextupdate->inlist = qtrue;
				} //end if
			} //end if
		} //end for
	} //end while
	  //botimport.Print(PRT_MESSAGE, "AAS_NearestHideArea: hidearea: %i, %i loops\n", bestarea, count );
	return bestarea;
} //end of the function AAS_NearestHideArea

#if defined RTCW_ET
int BotFuzzyPointReachabilityArea( vec3_t origin );
#endif // RTCW_XX

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_FindAttackSpotWithinRange( int srcnum, int rangenum, int enemynum, float rangedist, int travelflags, float *outpos ) {
	int i, nextareanum, badtravelflags, numreach, bestarea;
	unsigned short int t, besttraveltime, enemytraveltime;
	aas_routingupdate_t *updateliststart, *updatelistend, *curupdate, *nextupdate;
	aas_reachability_t *reach;
	vec3_t srcorg, rangeorg, enemyorg;
	int srcarea, rangearea, enemyarea;
	unsigned short int srctraveltime;
	int count = 0;
	#define MAX_ATTACKAREA_LOOPS    200

#if !defined RTCW_MP
	static float lastTime;
	//
	// RF, currently doesn't work with multiple AAS worlds, so only enable for the default world
	//if (aasworld != aasworlds) return 0;
#endif // RTCW_XX

	//
	// don't run this more than once per frame

#if defined RTCW_MP
	static float lastTime;
#endif // RTCW_XX

	if ( lastTime == AAS_Time() ) {
		return 0;
	}
	lastTime = AAS_Time();
	//

#if !defined RTCW_ET
	if ( !( *aasworld ).hidetraveltimes ) {
		( *aasworld ).hidetraveltimes = (unsigned short int *) GetClearedMemory( ( *aasworld ).numareas * sizeof( unsigned short int ) );
	} else {
		memset( ( *aasworld ).hidetraveltimes, 0, ( *aasworld ).numareas * sizeof( unsigned short int ) );
	} //end else
	  //
	if ( !( *aasworld ).visCache ) {
		( *aasworld ).visCache = (byte *) GetClearedMemory( ( *aasworld ).numareas * sizeof( byte ) );
	} else {
		memset( ( *aasworld ).visCache, 0, ( *aasworld ).numareas * sizeof( byte ) );
#else
	if ( !aasworld->hidetraveltimes ) {
		aasworld->hidetraveltimes = (unsigned short int *) GetClearedMemory( aasworld->numareas * sizeof( unsigned short int ) );
	} else {
		memset( aasworld->hidetraveltimes, 0, aasworld->numareas * sizeof( unsigned short int ) );
	} //end else
	  //
	if ( !aasworld->visCache ) {
		aasworld->visCache = (byte *) GetClearedMemory( aasworld->numareas * sizeof( byte ) );
	} else {
		memset( aasworld->visCache, 0, aasworld->numareas * sizeof( byte ) );
#endif // RTCW_XX

	} //end else
	  //
#if defined RTCW_MP
	srcarea = AAS_BestReachableEntityArea( srcnum );
	rangearea = AAS_BestReachableEntityArea( rangenum );
	enemyarea = AAS_BestReachableEntityArea( enemynum );
	//
#endif // RTCW_XX

	AAS_EntityOrigin( srcnum, srcorg );
	AAS_EntityOrigin( rangenum, rangeorg );
	AAS_EntityOrigin( enemynum, enemyorg );
	//

#if !defined RTCW_MP
	srcarea = BotFuzzyPointReachabilityArea( srcorg );
	rangearea = BotFuzzyPointReachabilityArea( rangeorg );
	enemyarea = BotFuzzyPointReachabilityArea( enemyorg );
#endif // RTCW_XX

	//
	besttraveltime = 0;
	bestarea = 0;
	enemytraveltime = AAS_AreaTravelTimeToGoalArea( srcarea, srcorg, enemyarea, travelflags );
	//
	badtravelflags = ~travelflags;
	//

#if !defined RTCW_ET
	curupdate = &( *aasworld ).areaupdate[rangearea];
#else
	curupdate = &aasworld->areaupdate[rangearea];
#endif // RTCW_XX

	curupdate->areanum = rangearea;
	VectorCopy( rangeorg, curupdate->start );

#if !defined RTCW_ET
	curupdate->areatraveltimes = ( *aasworld ).areatraveltimes[srcarea][0];
#else
	curupdate->areatraveltimes = aasworld->areatraveltimes[srcarea][0];
#endif // RTCW_XX

	curupdate->tmptraveltime = 0;
	//put the area to start with in the current read list
	curupdate->next = NULL;
	curupdate->prev = NULL;
	updateliststart = curupdate;
	updatelistend = curupdate;
	//while there are updates in the current list, flip the lists
	while ( updateliststart )
	{
		curupdate = updateliststart;
		//
		if ( curupdate->next ) {
			curupdate->next->prev = NULL;
		} else { updatelistend = NULL;}
		updateliststart = curupdate->next;
		//
		curupdate->inlist = qfalse;
		//check all reversed reachability links

#if !defined RTCW_ET
		numreach = ( *aasworld ).areasettings[curupdate->areanum].numreachableareas;
		reach = &( *aasworld ).reachability[( *aasworld ).areasettings[curupdate->areanum].firstreachablearea];
#else
		numreach = aasworld->areasettings[curupdate->areanum].numreachableareas;
		reach = &aasworld->reachability[aasworld->areasettings[curupdate->areanum].firstreachablearea];
#endif // RTCW_XX

		//
		for ( i = 0; i < numreach; i++, reach++ )
		{
			//if an undesired travel type is used

#if !defined RTCW_ET
			if ( ( *aasworld ).travelflagfortype[reach->traveltype] & badtravelflags ) {
#else
			if ( aasworld->travelflagfortype[reach->traveltype] & badtravelflags ) {
#endif // RTCW_XX

				continue;
			}
			//
			if ( AAS_AreaContentsTravelFlag( reach->areanum ) & badtravelflags ) {
				continue;
			}

#if !defined RTCW_MP
			// dont pass through ladder areas

#if !defined RTCW_ET
			if ( ( *aasworld ).areasettings[reach->areanum].areaflags & AREA_LADDER ) {
#else
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_LADDER ) {
#endif // RTCW_XX

				continue;
			}
#endif // RTCW_XX

			//

#if !defined RTCW_ET
			if ( ( *aasworld ).areasettings[reach->areanum].areaflags & AREA_DISABLED ) {
#else
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_DISABLED ) {
#endif // RTCW_XX

				continue;
			}
			//number of the area the reachability leads to
			nextareanum = reach->areanum;
			// if this moves us into the enemies area, skip it
			if ( nextareanum == enemyarea ) {
				continue;
			}
			// if we've already been to this area

#if !defined RTCW_ET
			if ( ( *aasworld ).hidetraveltimes[nextareanum] ) {
#else
			if ( aasworld->hidetraveltimes[nextareanum] ) {
#endif // RTCW_XX

				continue;
			}
			//time already travelled plus the traveltime through
			//the current area plus the travel time from the reachability
			if ( count++ > MAX_ATTACKAREA_LOOPS ) {
				//botimport.Print(PRT_MESSAGE, "AAS_FindAttackSpotWithinRange: exceeded max loops, aborting\n" );
				if ( bestarea ) {

#if !defined RTCW_ET
					VectorCopy( ( *aasworld ).areawaypoints[bestarea], outpos );
#else
					VectorCopy( aasworld->areawaypoints[bestarea], outpos );
#endif // RTCW_XX

				}
				return bestarea;
			}
			t = curupdate->tmptraveltime +
				AAS_AreaTravelTime( curupdate->areanum, curupdate->start, reach->start ) +
				reach->traveltime;
			//
			// if it's too far from rangenum, ignore

#if !defined RTCW_ET
			if ( Distance( rangeorg, ( *aasworld ).areawaypoints[nextareanum] ) > rangedist ) {
#else
			if ( Distance( rangeorg, aasworld->areawaypoints[nextareanum] ) > rangedist ) {
#endif // RTCW_XX

				continue;
			}
			//
			// find the traveltime from srcnum
			srctraveltime = AAS_AreaTravelTimeToGoalArea( srcarea, srcorg, nextareanum, travelflags );
			// do this test now, so we can reject the route if it starts out too long
			if ( besttraveltime && srctraveltime >= besttraveltime ) {
				continue;
			}
			//
			// if this area doesn't have a vis list, ignore it

#if !defined RTCW_ET
			if ( ( *aasworld ).areavisibility[nextareanum] ) {
				//if the nextarea can see the enemy area
				if ( AAS_AreaVisible( enemyarea, nextareanum ) ) { // now last of all, check that this area is a good attacking spot
					if (    ( ( *aasworld ).visCache[nextareanum] == 2 ) ||
							(   !( *aasworld ).visCache[nextareanum] &&
								( count += 10 ) &&    // we are about to use lots of CPU time
								botimport.AICast_CheckAttackAtPos( srcnum, enemynum, ( *aasworld ).areawaypoints[nextareanum], qfalse, qfalse ) ) ) {
						( *aasworld ).visCache[nextareanum] = 2;
						besttraveltime = srctraveltime;
						bestarea = nextareanum;
					} else {
						( *aasworld ).visCache[nextareanum] = 1;
					}
				} //end if
			}
			( *aasworld ).hidetraveltimes[nextareanum] = t;
			nextupdate = &( *aasworld ).areaupdate[nextareanum];
#else
			if ( aasworld->areavisibility[nextareanum] ) {
				//if the nextarea can see the enemy area
				if ( AAS_AreaVisible( enemyarea, nextareanum ) ) { // now last of all, check that this area is a good attacking spot
					if (    ( aasworld->visCache[nextareanum] == 2 ) ||
							(   !aasworld->visCache[nextareanum] &&
								( count += 10 ) &&    // we are about to use lots of CPU time
								botimport.BotCheckAttackAtPos( srcnum, enemynum, aasworld->areawaypoints[nextareanum], qfalse, qfalse ) ) ) {
						aasworld->visCache[nextareanum] = 2;
						besttraveltime = srctraveltime;
						bestarea = nextareanum;
					} else {
						aasworld->visCache[nextareanum] = 1;
					}
				} //end if
			}
			aasworld->hidetraveltimes[nextareanum] = t;
			nextupdate = &aasworld->areaupdate[nextareanum];
#endif // RTCW_XX

			nextupdate->areanum = nextareanum;
			nextupdate->tmptraveltime = t;
			//remember where we entered this area
			VectorCopy( reach->end, nextupdate->start );
			//if this update is not in the list yet
			if ( !nextupdate->inlist ) {
				//add the new update to the end of the list
				nextupdate->next = NULL;
				nextupdate->prev = updatelistend;
				if ( updatelistend ) {
					updatelistend->next = nextupdate;
				} else { updateliststart = nextupdate;}
				updatelistend = nextupdate;
				nextupdate->inlist = qtrue;
			} //end if
		} //end for
	} //end while
//botimport.Print(PRT_MESSAGE, "AAS_NearestHideArea: hidearea: %i, %i loops\n", bestarea, count );
	if ( bestarea ) {

#if !defined RTCW_ET
		VectorCopy( ( *aasworld ).areawaypoints[bestarea], outpos );
#else
		VectorCopy( aasworld->areawaypoints[bestarea], outpos );
#endif // RTCW_XX

	}
	return bestarea;
} //end of the function AAS_NearestHideArea

#if !defined RTCW_MP
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
qboolean AAS_GetRouteFirstVisPos( vec3_t srcpos, vec3_t destpos, int travelflags, vec3_t retpos ) {
	int srcarea, destarea, travarea;
	vec3_t travpos;
	int ftraveltime, freachnum, lasttraveltime;
	aas_reachability_t reach;
	int loops = 0;
#define MAX_GETROUTE_VISPOS_LOOPS   200
	//
	// SRCPOS: enemy
	// DESTPOS: self
	// RETPOS: first area that is visible from destpos, in route from srcpos to destpos
	srcarea = BotFuzzyPointReachabilityArea( srcpos );
	if ( !srcarea ) {
		return qfalse;
	}
	destarea = BotFuzzyPointReachabilityArea( destpos );
	if ( !destarea ) {
		return qfalse;
	}
	if ( destarea == srcarea ) {
		VectorCopy( srcpos, retpos );
		return qtrue;
	}
	//
	//if the srcarea can see the destarea
	if ( AAS_AreaVisible( srcarea, destarea ) ) {
		VectorCopy( srcpos, retpos );
		return qtrue;
	}
	// if this area doesn't have a vis list, ignore it

#if !defined RTCW_ET
	if ( !( *aasworld ).areavisibility[destarea] ) {
#else
	if ( !aasworld->areavisibility[destarea] ) {
#endif // RTCW_XX

		return qfalse;
	}
	//
	travarea = srcarea;
	VectorCopy( srcpos, travpos );
	lasttraveltime = -1;
	while ( ( loops++ < MAX_GETROUTE_VISPOS_LOOPS ) && AAS_AreaRouteToGoalArea( travarea, travpos, destarea, travelflags, &ftraveltime, &freachnum ) ) {
		if ( lasttraveltime >= 0 && ftraveltime >= lasttraveltime ) {
			return qfalse;  // we may be in a loop
		}
		lasttraveltime = ftraveltime;
		//
		AAS_ReachabilityFromNum( freachnum, &reach );
		if ( reach.areanum == destarea ) {
			VectorCopy( travpos, retpos );
			return qtrue;
		}
		//if the reach area can see the destarea
		if ( AAS_AreaVisible( reach.areanum, destarea ) ) {
			VectorCopy( reach.end, retpos );
			return qtrue;
		}
		//
		travarea = reach.areanum;
		VectorCopy( reach.end, travpos );
	}
	//
	// unsuccessful
	return qfalse;
}
#endif // RTCW_XX

#if defined RTCW_ET
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_ListAreasInRange( vec3_t srcpos, int srcarea, float range, int travelflags, vec3_t *outareas, int maxareas ) {
	int i, nextareanum, badtravelflags, numreach;
	aas_routingupdate_t *updateliststart, *updatelistend, *curupdate, *nextupdate;
	aas_reachability_t *reach;
	int count = 0;
	//
	if ( !aasworld->hidetraveltimes ) {
		aasworld->hidetraveltimes = (unsigned short int *) GetClearedMemory( aasworld->numareas * sizeof( unsigned short int ) );
	} else {
		memset( aasworld->hidetraveltimes, 0, aasworld->numareas * sizeof( unsigned short int ) );
	} //end else
	  //
	badtravelflags = ~travelflags;
	//
	curupdate = &aasworld->areaupdate[srcarea];
	curupdate->areanum = srcarea;
	VectorCopy( srcpos, curupdate->start );
	// GORDON: TEMP: FIXME: temp to stop crash
	if ( srcarea == 0 ) {
		return 0;
	}
	curupdate->areatraveltimes = aasworld->areatraveltimes[srcarea][0];
	curupdate->tmptraveltime = 0;
	//put the area to start with in the current read list
	curupdate->next = NULL;
	curupdate->prev = NULL;
	updateliststart = curupdate;
	updatelistend = curupdate;
	//while there are updates in the current list, flip the lists
	while ( updateliststart )
	{
		curupdate = updateliststart;
		//
		if ( curupdate->next ) {
			curupdate->next->prev = NULL;
		} else { updatelistend = NULL;}
		updateliststart = curupdate->next;
		//
		curupdate->inlist = qfalse;
		//check all reversed reachability links
		numreach = aasworld->areasettings[curupdate->areanum].numreachableareas;
		reach = &aasworld->reachability[aasworld->areasettings[curupdate->areanum].firstreachablearea];
		//
		for ( i = 0; i < numreach; i++, reach++ )
		{
			//if an undesired travel type is used
			if ( aasworld->travelflagfortype[reach->traveltype] & badtravelflags ) {
				continue;
			}
			//
			if ( AAS_AreaContentsTravelFlag( reach->areanum ) & badtravelflags ) {
				continue;
			}
			// dont pass through ladder areas
			//if (aasworld->areasettings[reach->areanum].areaflags & AREA_LADDER) continue;
			//
			//if (aasworld->areasettings[reach->areanum].areaflags & AREA_DISABLED) continue;
			//number of the area the reachability leads to
			nextareanum = reach->areanum;
			// if we've already been to this area
			if ( aasworld->hidetraveltimes[nextareanum] ) {
				continue;
			}
			aasworld->hidetraveltimes[nextareanum] = 1;
			// if it's too far from srcpos, ignore
			if ( Distance( srcpos, aasworld->areawaypoints[nextareanum] ) > range ) {
				continue;
			}
			//
			// if visible from srcarea
			if ( !( aasworld->areasettings[reach->areanum].areaflags & AREA_LADDER ) &&
				 !( aasworld->areasettings[reach->areanum].areaflags & AREA_DISABLED ) &&
				 ( AAS_AreaVisible( srcarea, nextareanum ) ) ) {
				VectorCopy( aasworld->areawaypoints[nextareanum], outareas[count] );
				count++;
				if ( count >= maxareas ) {
					break;
				}
			}
			//
			nextupdate = &aasworld->areaupdate[nextareanum];
			nextupdate->areanum = nextareanum;
			//remember where we entered this area
			VectorCopy( reach->end, nextupdate->start );
			//if this update is not in the list yet
			if ( !nextupdate->inlist ) {
				//add the new update to the end of the list
				nextupdate->next = NULL;
				nextupdate->prev = updatelistend;
				if ( updatelistend ) {
					updatelistend->next = nextupdate;
				} else { updateliststart = nextupdate;}
				updatelistend = nextupdate;
				nextupdate->inlist = qtrue;
			} //end if
		} //end for
	} //end while
	return count;
}

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_AvoidDangerArea( vec3_t srcpos, int srcarea, vec3_t dangerpos, int dangerarea, float range, int travelflags ) {
	int i, nextareanum, badtravelflags, numreach, bestarea = 0;
	aas_routingupdate_t *updateliststart, *updatelistend, *curupdate, *nextupdate;
	aas_reachability_t *reach;
	int bestTravel = 999999, t;
	const int maxTime = 5000;
	const int goodTime = 1000;
	vec_t dangerDistance = 0;

	//
	if ( !aasworld->areavisibility ) {
		return 0;
	}
	if ( !srcarea ) {
		return 0;
	}
	//
	if ( !aasworld->hidetraveltimes ) {
		aasworld->hidetraveltimes = (unsigned short int *) GetClearedMemory( aasworld->numareas * sizeof( unsigned short int ) );
	} else {
		memset( aasworld->hidetraveltimes, 0, aasworld->numareas * sizeof( unsigned short int ) );
	} //end else
	  //
	badtravelflags = ~travelflags;
	//
	curupdate = &aasworld->areaupdate[srcarea];
	curupdate->areanum = srcarea;
	VectorCopy( srcpos, curupdate->start );
	curupdate->areatraveltimes = aasworld->areatraveltimes[srcarea][0];
	curupdate->tmptraveltime = 0;
	//put the area to start with in the current read list
	curupdate->next = NULL;
	curupdate->prev = NULL;
	updateliststart = curupdate;
	updatelistend = curupdate;

	// Mad Doctor I, 11/3/2002.  The source area never needs to be expanded
	// again, so mark it as cut off
	aasworld->hidetraveltimes[srcarea] = 1;

	//while there are updates in the current list, flip the lists
	while ( updateliststart )
	{
		curupdate = updateliststart;
		//
		if ( curupdate->next ) {
			curupdate->next->prev = NULL;
		} else {
			updatelistend = NULL;
		}
		updateliststart = curupdate->next;
		//
		curupdate->inlist = qfalse;
		//check all reversed reachability links
		numreach = aasworld->areasettings[curupdate->areanum].numreachableareas;
		reach = &aasworld->reachability[aasworld->areasettings[curupdate->areanum].firstreachablearea];
		//
		for ( i = 0; i < numreach; i++, reach++ )
		{
			//if an undesired travel type is used
			if ( aasworld->travelflagfortype[reach->traveltype] & badtravelflags ) {
				continue;
			}
			//
			if ( AAS_AreaContentsTravelFlag( reach->areanum ) & badtravelflags ) {
				continue;
			}
			// dont pass through ladder areas
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_LADDER ) {
				continue;
			}
			//
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_DISABLED ) {
				continue;
			}
			//number of the area the reachability leads to
			nextareanum = reach->areanum;
			// if we've already been to this area
			if ( aasworld->hidetraveltimes[nextareanum] ) {
				continue;
			}
			aasworld->hidetraveltimes[nextareanum] = 1;
			// calc traveltime from srcpos
			t = curupdate->tmptraveltime +
				AAS_AreaTravelTime( curupdate->areanum, curupdate->start, reach->start ) +
				reach->traveltime;
			if ( t > maxTime ) {
				continue;
			}
			if ( t > bestTravel ) {
				continue;
			}

			// How far is it
			dangerDistance = Distance( dangerpos, aasworld->areawaypoints[nextareanum] );

			// if it's safe from dangerpos
			if ( aasworld->areavisibility[nextareanum] && ( dangerDistance > range ) ) {
				if ( t < goodTime ) {
					return nextareanum;
				}
				if ( t < bestTravel ) {
					bestTravel = t;
					bestarea = nextareanum;
				}
			}
			//
			nextupdate = &aasworld->areaupdate[nextareanum];
			nextupdate->areanum = nextareanum;
			//remember where we entered this area
			VectorCopy( reach->end, nextupdate->start );
			//if this update is not in the list yet

// Mad Doctor I, 11/3/2002.  The inlist field seems to not be inited properly for this function.
// It causes certain routes to be excluded unnecessarily, so I'm trying to do without it.
// Note that the hidetraveltimes array seems to cut off duplicates already.
//			if (!nextupdate->inlist)
			{
				//add the new update to the end of the list
				nextupdate->next = NULL;
				nextupdate->prev = updatelistend;
				if ( updatelistend ) {
					updatelistend->next = nextupdate;
				} else {
					updateliststart = nextupdate;
				}
				updatelistend = nextupdate;
				nextupdate->inlist = qtrue;
			} //end if
		} //end for
	} //end while
	return bestarea;
}


//
// AAS_DangerPQInit()
//
// Description: Init the priority queue
// Written: 12/12/2002
//
void AAS_DangerPQInit
(
) {
	// Local Variables ////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

//@TEST. To test that the retreat algorithm is going to work, I've implemented
// a quicky, slow-ass PQ.  This needs to be replaced with a heap implementation.

	// Init the distanceFromDanger array if needed
	if ( !aasworld->PQ_accumulator ) {
		// Get memory for this array the safe way.
		aasworld->PQ_accumulator = (unsigned short int *) GetClearedMemory( aasworld->numareas * sizeof( unsigned short int ) );

	}  // if (!aasworld->distanceFromDanger) ...

	// There are no items in the PQ right now
	aasworld->PQ_size = 0;

}
//
// AAS_DangerPQInit()
//



//
// AAS_DangerPQInsert
//
// Description: Put an area into the PQ.  ASSUMES the dangerdistance for the
// area is set ahead of time.
// Written: 12/11/2002
//
void AAS_DangerPQInsert
(
	// The area to insert
	int areaNum
) {
	// Local Variables ////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

//@TEST. To test that the retreat algorithm is going to work, I've implemented
// a quicky, slow-ass PQ.  This needs to be replaced with a heap implementation.

	// Increment the count in the accum
	aasworld->PQ_size++;

	// Put this one at the end
	aasworld->PQ_accumulator[aasworld->PQ_size] = areaNum;

}
//
// AAS_DangerPQInsert
//



//
// AAS_DangerPQEmpty
//
// Description: Is the Danger Priority Queue empty?
// Written: 12/11/2002
//
qboolean AAS_DangerPQEmpty
(
) {
	// Local Variables ////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

//@TEST. To test that the retreat algorithm is going to work, I've implemented
// a quicky, slow-ass PQ.  This needs to be replaced with a heap implementation.

	// It's not empty if anything is in the accumulator
	if ( aasworld->PQ_size > 0 ) {
		return qfalse;
	}

	return qtrue;
}
//
// AAS_DangerPQEmpty
//


//
// AAS_DangerPQRemove
//
// Description: Pull the smallest distance area off of the danger Priority
// Queue.
// Written: 12/11/2002
//
int AAS_DangerPQRemove
(
) {
	// Local Variables ////////////////////////////////////////////////////////
	int j;
	int nearest = 1;
	int nearestArea = aasworld->PQ_accumulator[nearest];
	int nearestDistance = aasworld->distanceFromDanger[nearestArea];
	int distance;
	int temp;
	int currentArea;
	///////////////////////////////////////////////////////////////////////////

//@TEST. To test that the retreat algorithm is going to work, I've implemented
// a quicky, slow-ass PQ.  This needs to be replaced with a heap implementation.

	// Just loop through the items in the PQ
	for ( j = 2; j <= aasworld->PQ_size; j++ )
	{
		// What's the next area?
		currentArea = aasworld->PQ_accumulator[j];

		// What's the danerg distance of it
		distance = aasworld->distanceFromDanger[currentArea];

		// Is this element the best one? Top of the heap, so to speak
		if ( distance < nearestDistance ) {
			// Save this one
			nearest = j;

			// This has the best distance
			nearestDistance = distance;

			// This one is the nearest region so far
			nearestArea = currentArea;

		} // if (distance < nearestDistance)...

	} // for (j = 2; j <= aasworld->PQ_size; j++)...

	// Save out the old end of list
	temp = aasworld->PQ_accumulator[aasworld->PQ_size];

	// Put this where the old one was
	aasworld->PQ_accumulator[nearest] = temp;

	// Decrement the count
	aasworld->PQ_size--;

	return nearestArea;
}
//
// AAS_DangerPQRemove
//


//
// AAS_DangerPQChange
//
// Description: We've changed the danger distance for this node, so change
// its place in the PQ if needed.
// Written: 12/11/2002
//
void AAS_DangerPQChange
(
	// The area to change in the PQ
	int areaNum
) {
	// Local Variables ////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

//@TEST. To test that the retreat algorithm is going to work, I've implemented
// a quicky, slow-ass PQ.  This needs to be replaced with a heap implementation.
// NOTHING NEEDS TO BE DONE HERE FOR THE SLOW-ASS PQ.

}
//
// AAS_DangerPQChange
//






//===========================================================================
//
// AAS_CalculateDangerZones
//
// Description:
// Written: 12/11/2002
//
//===========================================================================
void AAS_CalculateDangerZones
(
	// Locations of the danger spots
	int *dangerSpots,

	// The number of danger spots
	int dangerSpotCount,

	// What is the furthest danger range we care about? (Everything further is safe)
	float dangerRange,

	// A safe distance to init distanceFromDanger to
	unsigned short int definitelySafe
) {
	// Local Variables ////////////////////////////////////////////////////////
	// Generic index used to loop through danger zones (and later, the reachabilities)
	int i;

	// The area number of a danger spot
	int dangerAreaNum;

	// What's the current AAS area we're measuring distance from danger to
	int currentArea;

	// How many links come out of the current AAS area?
	int numreach;

	// Number of the area the reachability leads to
	int nextareanum;

	// Distance from current node to next node
	float distanceFromCurrentNode;

	// Total distance from danger to next node
	int dangerDistance;

	// The previous distance for this node
	int oldDistance;

	// A link from the current node
	aas_reachability_t *reach = NULL;

	///////////////////////////////////////////////////////////////////////////

	// Initialize all of the starting danger zones.
	for ( i = 0; i < dangerSpotCount; i++ )
	{
		// Get the area number of this danger spot
		dangerAreaNum = dangerSpots[i];

		// Set it's distance to 0, meaning it's 0 units to danger
		aasworld->distanceFromDanger[dangerAreaNum] = 0;

		// Add the zone to the PQ.
		AAS_DangerPQInsert( dangerAreaNum );

	} // for (i = 0; i < dangerSpotCount; i++)...

	// Go through the Priority Queue, pop off the smallest distance, and expand
	// to the neighboring nodes.  Stop when the PQ is empty.
	while ( !AAS_DangerPQEmpty() )
	{
		// Get the smallest distance in the PQ.
		currentArea = AAS_DangerPQRemove();

		// Check all reversed reachability links
		numreach = aasworld->areasettings[currentArea].numreachableareas;
		reach = &aasworld->reachability[aasworld->areasettings[currentArea].firstreachablearea];

		// Loop through the neighbors to this node.
		for ( i = 0; i < numreach; i++, reach++ )
		{
			// Number of the area the reachability leads to
			nextareanum = reach->areanum;

			// How far was it from the last node to this one?
			distanceFromCurrentNode = Distance( aasworld->areawaypoints[currentArea],
												aasworld->areawaypoints[nextareanum] );

			// Calculate the distance from danger to this neighbor along the
			// current route.
			dangerDistance = aasworld->distanceFromDanger[currentArea]
							 + (int) distanceFromCurrentNode;

			// Skip this neighbor if the distance is bigger than we care about.
			if ( dangerDistance > dangerRange ) {
				continue;
			}

			// Store the distance from danger if it's smaller than any previous
			// distance to this node (note that unvisited nodes are inited with
			// a big distance, so this check will be satisfied).
			if ( dangerDistance < aasworld->distanceFromDanger[nextareanum] ) {
				// How far was this node from danger before this visit?
				oldDistance = aasworld->distanceFromDanger[nextareanum];

				// Store the new value
				aasworld->distanceFromDanger[nextareanum] = dangerDistance;

				// If the neighbor has been calculated already, see if we need to
				// update the priority.
				if ( oldDistance != definitelySafe ) {
					// We need to update the priority queue's position for this node
					AAS_DangerPQChange( nextareanum );

				} // if (aasworld->distanceFromDanger[nextareanum] == definitelySafe)...
				  // Otherwise, insert the neighbor into the PQ.
				else
				{
					// Insert this node into the PQ
					AAS_DangerPQInsert( nextareanum );

				} // else ...

			} // if (dangerDistance < aasworld->distanceFromDanger[nextareanum])...

		} // for (i = 0; i < numreach; i++, reach++)...

	} // while (!AAS_DangerPQEmpty())...

	// At this point, all of the nodes within our danger range have their
	// distance from danger calculated.

}
//
// AAS_CalculateDangerZones
//



//===========================================================================
//
// AAS_Retreat
//
// Use this to find a safe spot away from a dangerous situation/enemy.
// This differs from AAS_AvoidDangerArea in the following ways:
// * AAS_Retreat will return the farthest location found even if it does not
//   exceed the desired minimum distance.
// * AAS_Retreat will give preference to nodes on the "safe" side of the danger
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_Retreat
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
	int i, nextareanum, badtravelflags, numreach, bestarea = 0;
	aas_routingupdate_t *updateliststart, *updatelistend, *curupdate, *nextupdate;
	aas_reachability_t *reach;
	int t;
	const int maxTime = 5000;
//	const int goodTime = 1000;
	vec_t dangerDistance = 0;
	vec_t sourceDistance = 0;
	float bestDistanceSoFar = 0;

	// Choose a safe distance to init distanceFromDanger to
//	int definitelySafe = 1 + (int) range;
	unsigned short int definitelySafe = 0xFFFF;

	//
	if ( !aasworld->areavisibility ) {
		return 0;
	}

	// Init the hide travel time if needed
	if ( !aasworld->hidetraveltimes ) {
		aasworld->hidetraveltimes = (unsigned short int *) GetClearedMemory( aasworld->numareas * sizeof( unsigned short int ) );
	}
	// Otherwise, it exists already, so just reset the values
	else
	{
		memset( aasworld->hidetraveltimes, 0, aasworld->numareas * sizeof( unsigned short int ) );
	} //end else

	// Init the distanceFromDanger array if needed
	if ( !aasworld->distanceFromDanger ) {
		// Get memory for this array the safe way.
		aasworld->distanceFromDanger = (unsigned short int *) GetClearedMemory( aasworld->numareas * sizeof( unsigned short int ) );

	}  // if (!aasworld->distanceFromDanger) ...

	// Set all the values in the distanceFromDanger array to a safe value
	memset( aasworld->distanceFromDanger, 0xFF, aasworld->numareas * sizeof( unsigned short int ) );

	// Init the priority queue
	AAS_DangerPQInit();

	// Set up the distanceFromDanger array
	AAS_CalculateDangerZones( dangerSpots, dangerSpotCount, dangerRange, definitelySafe );

	//
	badtravelflags = ~travelflags;
	//
	curupdate = &aasworld->areaupdate[srcarea];
	curupdate->areanum = srcarea;
	VectorCopy( srcpos, curupdate->start );
	curupdate->areatraveltimes = aasworld->areatraveltimes[srcarea][0];
	curupdate->tmptraveltime = 0;
	//put the area to start with in the current read list
	curupdate->next = NULL;
	curupdate->prev = NULL;
	updateliststart = curupdate;
	updatelistend = curupdate;

	// Mad Doctor I, 11/3/2002.  The source area never needs to be expanded
	// again, so mark it as cut off
	aasworld->hidetraveltimes[srcarea] = 1;

	//while there are updates in the current list, flip the lists
	while ( updateliststart )
	{
		curupdate = updateliststart;
		//
		if ( curupdate->next ) {
			curupdate->next->prev = NULL;
		} else {
			updatelistend = NULL;
		}
		updateliststart = curupdate->next;
		//
		curupdate->inlist = qfalse;
		//check all reversed reachability links
		numreach = aasworld->areasettings[curupdate->areanum].numreachableareas;
		reach = &aasworld->reachability[aasworld->areasettings[curupdate->areanum].firstreachablearea];
		//
		for ( i = 0; i < numreach; i++, reach++ )
		{
			//if an undesired travel type is used
			if ( aasworld->travelflagfortype[reach->traveltype] & badtravelflags ) {
				continue;
			}
			//
			if ( AAS_AreaContentsTravelFlag( reach->areanum ) & badtravelflags ) {
				continue;
			}
			// dont pass through ladder areas
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_LADDER ) {
				continue;
			}
			//
			if ( aasworld->areasettings[reach->areanum].areaflags & AREA_DISABLED ) {
				continue;
			}
			//number of the area the reachability leads to
			nextareanum = reach->areanum;
			// if we've already been to this area
			if ( aasworld->hidetraveltimes[nextareanum] ) {
				continue;
			}
			aasworld->hidetraveltimes[nextareanum] = 1;
			// calc traveltime from srcpos
			t = curupdate->tmptraveltime +
				AAS_AreaTravelTime( curupdate->areanum, curupdate->start, reach->start ) +
				reach->traveltime;
			if ( t > maxTime ) {
				continue;
			}

			// How far is it from a danger area?
			dangerDistance = aasworld->distanceFromDanger[nextareanum];

			// How far is it from our starting position?
			sourceDistance = Distance( srcpos, aasworld->areawaypoints[nextareanum] );

			// If it's safe from dangerpos
			if ( aasworld->areavisibility[nextareanum]
				 && ( sourceDistance > range )
				 && ( ( dangerDistance > dangerRange )
					  || ( dangerDistance == definitelySafe )
					  )
				 ) {
				// Just use this area
				return nextareanum;
			}

			// In case we don't find a perfect one, save the best
			if ( dangerDistance > bestDistanceSoFar ) {
				bestarea = nextareanum;
				bestDistanceSoFar = dangerDistance;

			} // if (dangerDistance > bestDistanceSoFar)...

			//
			nextupdate = &aasworld->areaupdate[nextareanum];
			nextupdate->areanum = nextareanum;
			//remember where we entered this area
			VectorCopy( reach->end, nextupdate->start );
			//if this update is not in the list yet

			//add the new update to the end of the list
			nextupdate->next = NULL;
			nextupdate->prev = updatelistend;
			if ( updatelistend ) {
				updatelistend->next = nextupdate;
			} else {
				updateliststart = nextupdate;
			}
			updatelistend = nextupdate;
			nextupdate->inlist = qtrue;

		} //end for
	} //end while

	return bestarea;
}

/*
===================
AAS_InitTeamDeath
===================
*/
void AAS_InitTeamDeath( void ) {
	if ( aasworld->teamDeathTime ) {
		FreeMemory( aasworld->teamDeathTime );
	}
	aasworld->teamDeathTime = static_cast<int*> (GetClearedMemory( sizeof( int ) * aasworld->numareas * 2 ));
	if ( aasworld->teamDeathCount ) {
		FreeMemory( aasworld->teamDeathCount );
	}
	aasworld->teamDeathCount = static_cast<byte*> (GetClearedMemory( sizeof( byte ) * aasworld->numareas * 2 ));
	if ( aasworld->teamDeathAvoid ) {
		FreeMemory( aasworld->teamDeathAvoid );
	}
	aasworld->teamDeathAvoid = static_cast<byte*> (GetClearedMemory( sizeof( byte ) * aasworld->numareas * 2 ));
}

#define TEAM_DEATH_TIMEOUT              120.0
#define TEAM_DEATH_AVOID_COUNT_SCALE    0.5     // so if there are 12 players, then if count reaches (12 * scale) then AVOID
#define TEAM_DEATH_RANGE                512.0

/*
===================
AAS_UpdateTeamDeath
===================
*/
void AAS_UpdateTeamDeath( void ) {
	int i, j, k;

	// check for areas which have timed out, so we can stop avoiding them

	// for each area
	for ( i = 0; i < aasworld->numareas; i++ ) {
		// for each team
		for ( j = 0; j < 2; j++ ) {
			k = ( aasworld->numareas * j ) + i;
			if ( aasworld->teamDeathTime[k] ) {
				if ( aasworld->teamDeathTime[k] < AAS_Time() - TEAM_DEATH_TIMEOUT ) {
					// this area has timed out
					aasworld->teamDeathAvoid[k] = 0;
					aasworld->teamDeathTime[k] = 0;
					if ( aasworld->teamDeathAvoid[k] ) {
						// unmark this area
						if ( j == 0 ) {
							aasworld->areasettings[i].areaflags &= ~AREA_AVOID_AXIS;
						} else {
							aasworld->areasettings[i].areaflags &= ~AREA_AVOID_ALLIES;
						}
						//remove all routing cache involving this area
						AAS_RemoveRoutingCacheUsingArea( i );
						// recalculate the team flags that are used in this cluster
						AAS_ClearClusterTeamFlags( i );
					}
				}
			}
		}
	}
}

/*
===================
AAS_RecordTeamDeathArea
===================
*/
void AAS_RecordTeamDeathArea( vec3_t srcpos, int srcarea, int team, int teamCount, int travelflags ) {
	int i, nextareanum, badtravelflags, numreach, k;
	aas_routingupdate_t *updateliststart, *updatelistend, *curupdate, *nextupdate;
	aas_reachability_t *reach;
//	int count=0;
//
	return;
	//
	badtravelflags = ~travelflags;
	k = ( aasworld->numareas * team ) + srcarea;
	//
	curupdate = &aasworld->areaupdate[srcarea];
	curupdate->areanum = srcarea;
	VectorCopy( srcpos, curupdate->start );
	//
	if ( srcarea == 0 ) {
		return;
	}
	curupdate->areatraveltimes = aasworld->areatraveltimes[srcarea][0];
	curupdate->tmptraveltime = 0;
	//put the area to start with in the current read list
	curupdate->next = NULL;
	curupdate->prev = NULL;
	updateliststart = curupdate;
	updatelistend = curupdate;
	//while there are updates in the current list, flip the lists
	while ( updateliststart )
	{
		curupdate = updateliststart;
		//
		if ( curupdate->next ) {
			curupdate->next->prev = NULL;
		} else { updatelistend = NULL;}
		updateliststart = curupdate->next;
		//
		curupdate->inlist = qfalse;
		//check all reversed reachability links
		numreach = aasworld->areasettings[curupdate->areanum].numreachableareas;
		reach = &aasworld->reachability[aasworld->areasettings[curupdate->areanum].firstreachablearea];
		//
		for ( i = 0; i < numreach; i++, reach++ )
		{
			// if tihs area has already been done
			if ( aasworld->teamDeathTime[reach->areanum] >= AAS_Time() ) {
				continue;
			}
			aasworld->teamDeathTime[reach->areanum] = AAS_Time();
			//
			//if an undesired travel type is used
			if ( aasworld->travelflagfortype[reach->traveltype] & badtravelflags ) {
				continue;
			}
			//
			if ( AAS_AreaContentsTravelFlag( reach->areanum ) & badtravelflags ) {
				continue;
			}
			//number of the area the reachability leads to
			nextareanum = reach->areanum;
			//
			// if it's too far from srcpos, ignore
			if ( Distance( curupdate->start, reach->end ) + (float)curupdate->tmptraveltime > TEAM_DEATH_RANGE ) {
				continue;
			}
			//
			k = reach->areanum;
			// mark this area
			aasworld->teamDeathCount[k]++;
			if ( aasworld->teamDeathCount[k] > 100 ) {
				aasworld->teamDeathCount[k] = 100;                                      // make sure it doesnt loop around
			}
			//
			// see if this area is now to be avoided
			if ( !aasworld->teamDeathAvoid[k] ) {
				if ( aasworld->teamDeathCount[k] > (int)( TEAM_DEATH_AVOID_COUNT_SCALE * teamCount ) ) {
					// avoid this area
					aasworld->teamDeathAvoid[k] = 1;
					// mark this area
					if ( team == 0 ) {
						aasworld->areasettings[k].areaflags |= AREA_AVOID_AXIS;
					} else {
						aasworld->areasettings[k].areaflags |= AREA_AVOID_ALLIES;
					}
					//remove all routing cache involving this area
					AAS_RemoveRoutingCacheUsingArea( k );
					// recalculate the team flags that are used in this cluster
					AAS_ClearClusterTeamFlags( k );
				}
			}
			//
			nextupdate = &aasworld->areaupdate[nextareanum];
			nextupdate->areanum = nextareanum;
			//remember where we entered this area
			VectorCopy( reach->end, nextupdate->start );
			// calc the distance
			nextupdate->tmptraveltime = (float)curupdate->tmptraveltime + Distance( curupdate->start, reach->end );
			//if this update is not in the list yet
			if ( !nextupdate->inlist ) {
				//add the new update to the end of the list
				nextupdate->next = NULL;
				nextupdate->prev = updatelistend;
				if ( updatelistend ) {
					updatelistend->next = nextupdate;
				} else { updateliststart = nextupdate;}
				updatelistend = nextupdate;
				nextupdate->inlist = qtrue;
			} //end if
		} //end for
	} //end while
	  //
	return;
}
#endif // RTCW_XX

