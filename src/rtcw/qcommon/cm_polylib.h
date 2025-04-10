/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


// this is only used for visualization tools in cm_ debug functions

typedef struct
{
	int numpoints;
	vec3_t p[4];        // variable sized
} winding_t;

#define MAX_POINTS_ON_WINDING   64

#define SIDE_FRONT  0
#define SIDE_BACK   1
#define SIDE_ON     2
#define SIDE_CROSS  3

#define CLIP_EPSILON    0.1f

#if defined RTCW_SP
//#define MAX_MAP_BOUNDS			65535
#define MAX_MAP_BOUNDS      ( 128*1024 )    // (SA) (9/19/01) new map dimensions (from Q3TA)
#elif defined RTCW_ET
#define MAX_MAP_BOUNDS  65535
#endif // RTCW_XX

// you can define on_epsilon in the makefile as tighter
#ifndef ON_EPSILON
#define ON_EPSILON  0.1f
#endif

winding_t   *AllocWinding( int points );
vec_t   WindingArea( winding_t *w );
void    WindingCenter( winding_t *w, vec3_t center );
void    ClipWindingEpsilon( winding_t *in, vec3_t normal, vec_t dist,
							vec_t epsilon, winding_t **front, winding_t **back );
winding_t   *ChopWinding( winding_t *in, vec3_t normal, vec_t dist );
winding_t   *CopyWinding( winding_t *w );
winding_t   *ReverseWinding( winding_t *w );
winding_t   *BaseWindingForPlane( vec3_t normal, vec_t dist );
void    CheckWinding( winding_t *w );
void    WindingPlane( winding_t *w, vec3_t normal, vec_t *dist );
void    RemoveColinearPoints( winding_t *w );
int     WindingOnPlaneSide( winding_t *w, vec3_t normal, vec_t dist );
void    FreeWinding( winding_t *w );
void    WindingBounds( winding_t *w, vec3_t mins, vec3_t maxs );

void    AddWindingToConvexHull( winding_t *w, winding_t **hull, vec3_t normal );

void    ChopWindingInPlace( winding_t **w, vec3_t normal, vec_t dist, vec_t epsilon );
// frees the original if clipped

void pw( winding_t *w );
