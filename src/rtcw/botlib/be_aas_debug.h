/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_debug.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

//clear the shown debug lines
void AAS_ClearShownDebugLines( void );
//
void AAS_ClearShownPolygons( void );
//show a debug line
void AAS_DebugLine( vec3_t start, vec3_t end, int color );
//show a permenent line
void AAS_PermanentLine( vec3_t start, vec3_t end, int color );
//show a permanent cross
void AAS_DrawPermanentCross( vec3_t origin, float size, int color );
//draw a cross in the plane
void AAS_DrawPlaneCross( vec3_t point, vec3_t normal, float dist, int type, int color );
//show a bounding box
void AAS_ShowBoundingBox( vec3_t origin, vec3_t mins, vec3_t maxs );
//show a face
void AAS_ShowFace( int facenum );
//show an area
void AAS_ShowArea( int areanum, int groundfacesonly );
//
void AAS_ShowAreaPolygons( int areanum, int color, int groundfacesonly );
//draw a cros
void AAS_DrawCross( vec3_t origin, float size, int color );
//print the travel type
void AAS_PrintTravelType( int traveltype );
//draw an arrow
void AAS_DrawArrow( vec3_t start, vec3_t end, int linecolor, int arrowcolor );
//visualize the given reachability
void AAS_ShowReachability( struct aas_reachability_s *reach );
//show the reachable areas from the given area
void AAS_ShowReachableAreas( int areanum );

