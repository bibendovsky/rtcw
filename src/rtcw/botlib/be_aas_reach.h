/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_reach.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

#ifdef AASINTERN
//initialize calculating the reachabilities
void AAS_InitReachability( void );
//continue calculating the reachabilities
int AAS_ContinueInitReachability( float time );
//
int AAS_BestReachableLinkArea( aas_link_t *areas );
#endif //AASINTERN

//returns true if the are has reachabilities to other areas
int AAS_AreaReachability( int areanum );
//returns the best reachable area and goal origin for a bounding box at the given origin
int AAS_BestReachableArea( vec3_t origin, vec3_t mins, vec3_t maxs, vec3_t goalorigin );
//returns the next reachability using the given model
int AAS_NextModelReachability( int num, int modelnum );
//returns the total area of the ground faces of the given area
float AAS_AreaGroundFaceArea( int areanum );
//returns true if the area is crouch only

#if !defined RTCW_ET
int AAS_AreaCrouch( int areanum );
#else
//int AAS_AreaCrouch(int areanum);
#define AAS_AreaCrouch( areanum )   ( ( !( aasworld->areasettings[areanum].presencetype & PRESENCE_NORMAL ) ) ? qtrue : qfalse )
#endif // RTCW_XX

//returns true if a player can swim in this area

#if !defined RTCW_ET
int AAS_AreaSwim( int areanum );
#else
//int AAS_AreaSwim(int areanum);
#define AAS_AreaSwim( areanum )      ( ( aasworld->areasettings[areanum].areaflags & AREA_LIQUID ) ? qtrue : qfalse )
#endif // RTCW_XX

//returns true if the area is filled with a liquid
int AAS_AreaLiquid( int areanum );
//returns true if the area contains lava
int AAS_AreaLava( int areanum );
//returns true if the area contains slime
int AAS_AreaSlime( int areanum );
//returns true if the area has one or more ground faces
int AAS_AreaGrounded( int areanum );
//returns true if the area has one or more ladder faces
int AAS_AreaLadder( int areanum );
//returns true if the area is a jump pad
int AAS_AreaJumpPad( int areanum );
//returns true if the area is donotenter
int AAS_AreaDoNotEnter( int areanum );
//returns true if the area is donotenterlarge
int AAS_AreaDoNotEnterLarge( int areanum );
