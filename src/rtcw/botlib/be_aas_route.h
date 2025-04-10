/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_route.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

#ifdef AASINTERN
//initialize the AAS routing
void AAS_InitRouting( void );
//free the AAS routing caches
void AAS_FreeRoutingCaches( void );
//returns the travel time from start to end in the given area
unsigned short int AAS_AreaTravelTime( int areanum, vec3_t start, vec3_t end );
//
void AAS_CreateAllRoutingCache( void );
//
void AAS_RoutingInfo( void );
#endif //AASINTERN

//returns the travel flag for the given travel type
int AAS_TravelFlagForType( int traveltype );
//
int AAS_AreaContentsTravelFlag( int areanum );
//returns the index of the next reachability for the given area
int AAS_NextAreaReachability( int areanum, int reachnum );
//returns the reachability with the given index
void AAS_ReachabilityFromNum( int num, struct aas_reachability_s *reach );
//returns a random goal area and goal origin
int AAS_RandomGoalArea( int areanum, int travelflags, int *goalareanum, vec3_t goalorigin );
//returns the travel time within the given area from start to end
unsigned short int AAS_AreaTravelTime( int areanum, vec3_t start, vec3_t end );
//returns the travel time from the area to the goal area using the given travel flags
int AAS_AreaTravelTimeToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags );

#if defined RTCW_SP
//returns the travel time from the area to the goal area using the given travel flags
int AAS_AreaTravelTimeToGoalAreaCheckLoop( int areanum, vec3_t origin, int goalareanum, int travelflags, int loopareanum );

extern int BotFuzzyPointReachabilityArea( vec3_t origin );
#endif // RTCW_XX

#if defined RTCW_ET
void AAS_InitTeamDeath( void );
void AAS_RecordTeamDeathArea( vec3_t srcpos, int srcarea, int team, int teamCount, int travelflags );
void AAS_UpdateTeamDeath( void );
#endif // RTCW_XX

