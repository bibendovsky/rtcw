/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


// this is only used for visualization tools in cm_ debug functions


#include "cm_local.h"


// counters are only bumped when running single threaded,
// because they are an awefull coherence problem
int c_active_windings;
int c_peak_windings;
int c_winding_allocs;
int c_winding_points;

#if defined RTCW_MP
#define BOGUS_RANGE 8192
#endif // RTCW_XX

void pw( winding_t *w ) {
	int i;
	for ( i = 0 ; i < w->numpoints ; i++ )
		printf( "(%5.1f, %5.1f, %5.1f)\n",w->p[i][0], w->p[i][1],w->p[i][2] );
}


/*
=============
AllocWinding
=============
*/
winding_t   *AllocWinding( int points ) {
	winding_t   *w;
	int s;

	c_winding_allocs++;
	c_winding_points += points;
	c_active_windings++;
	if ( c_active_windings > c_peak_windings ) {
		c_peak_windings = c_active_windings;
	}

	s = sizeof( vec_t ) * 3 * points + sizeof( int );
	w = static_cast<winding_t*> (Z_Malloc( s ));

#if defined RTCW_SP
	Com_Memset( w, 0, s );
#else
	memset( w, 0, s );
#endif // RTCW_XX

	return w;
}

void FreeWinding( winding_t *w ) {
	if ( *(unsigned *)w == 0xdeaddead ) {
		Com_Error( ERR_FATAL, "FreeWinding: freed a freed winding" );
	}
	*(unsigned *)w = 0xdeaddead;

	c_active_windings--;
	Z_Free( w );
}

/*
============
RemoveColinearPoints
============
*/
int c_removed;

void    RemoveColinearPoints( winding_t *w ) {
	int i, j, k;
	vec3_t v1, v2;
	int nump;
	vec3_t p[MAX_POINTS_ON_WINDING];

	nump = 0;
	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		j = ( i + 1 ) % w->numpoints;
		k = ( i + w->numpoints - 1 ) % w->numpoints;
		VectorSubtract( w->p[j], w->p[i], v1 );
		VectorSubtract( w->p[i], w->p[k], v2 );
		VectorNormalize2( v1,v1 );
		VectorNormalize2( v2,v2 );
		if ( DotProduct( v1, v2 ) < 0.999 ) {
			VectorCopy( w->p[i], p[nump] );
			nump++;
		}
	}

	if ( nump == w->numpoints ) {
		return;
	}

	c_removed += w->numpoints - nump;
	w->numpoints = nump;

#if defined RTCW_SP
	Com_Memcpy( w->p, p, nump * sizeof( p[0] ) );
#else
	memcpy( w->p, p, nump * sizeof( p[0] ) );
#endif // RTCW_XX

}

/*
============
WindingPlane
============
*/
void WindingPlane( winding_t *w, vec3_t normal, vec_t *dist ) {
	vec3_t v1, v2;

	VectorSubtract( w->p[1], w->p[0], v1 );
	VectorSubtract( w->p[2], w->p[0], v2 );
	CrossProduct( v2, v1, normal );
	VectorNormalize2( normal, normal );
	*dist = DotProduct( w->p[0], normal );

}

/*
=============
WindingArea
=============
*/
vec_t   WindingArea( winding_t *w ) {
	int i;
	vec3_t d1, d2, cross;
	vec_t total;

	total = 0;
	for ( i = 2 ; i < w->numpoints ; i++ )
	{
		VectorSubtract( w->p[i - 1], w->p[0], d1 );
		VectorSubtract( w->p[i], w->p[0], d2 );
		CrossProduct( d1, d2, cross );
		total += 0.5 * VectorLength( cross );
	}
	return total;
}

/*
=============
WindingBounds
=============
*/
void    WindingBounds( winding_t *w, vec3_t mins, vec3_t maxs ) {
	vec_t v;
	int i,j;

#if !defined RTCW_MP
	mins[0] = mins[1] = mins[2] = MAX_MAP_BOUNDS;
	maxs[0] = maxs[1] = maxs[2] = -MAX_MAP_BOUNDS;
#else
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
#endif // RTCW_XX

	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		for ( j = 0 ; j < 3 ; j++ )
		{
			v = w->p[i][j];
			if ( v < mins[j] ) {
				mins[j] = v;
			}
			if ( v > maxs[j] ) {
				maxs[j] = v;
			}
		}
	}
}

/*
=============
WindingCenter
=============
*/
void    WindingCenter( winding_t *w, vec3_t center ) {
	int i;
	float scale;

	VectorCopy( vec3_origin, center );
	for ( i = 0 ; i < w->numpoints ; i++ )
		VectorAdd( w->p[i], center, center );

	scale = 1.0 / w->numpoints;
	VectorScale( center, scale, center );
}

/*
=================
BaseWindingForPlane
=================
*/
winding_t *BaseWindingForPlane( vec3_t normal, vec_t dist ) {
	int i, x;
	vec_t max, v;
	vec3_t org, vright, vup;
	winding_t   *w;

// find the major axis

#if !defined RTCW_MP
	max = -MAX_MAP_BOUNDS;
#else
	max = -BOGUS_RANGE;
#endif // RTCW_XX

	x = -1;
	for ( i = 0 ; i < 3; i++ )
	{

#if !defined RTCW_ET
		v = c::fabs( normal[i] );
#else
		v = Q_fabs( normal[i] );
#endif // RTCW_XX

		if ( v > max ) {
			x = i;
			max = v;
		}
	}
	if ( x == -1 ) {
		Com_Error( ERR_DROP, "BaseWindingForPlane: no axis found" );
	}

	VectorCopy( vec3_origin, vup );
	switch ( x )
	{
	case 0:
	case 1:
		vup[2] = 1;
		break;
	case 2:
		vup[0] = 1;
		break;
	}

	v = DotProduct( vup, normal );
	VectorMA( vup, -v, normal, vup );
	VectorNormalize2( vup, vup );

	VectorScale( normal, dist, org );

	CrossProduct( vup, normal, vright );

#if !defined RTCW_MP
	VectorScale( vup, MAX_MAP_BOUNDS, vup );
	VectorScale( vright, MAX_MAP_BOUNDS, vright );
#else
	VectorScale( vup, 8192, vup );
	VectorScale( vright, 8192, vright );
#endif // RTCW_XX

// project a really big	axis aligned box onto the plane
	w = AllocWinding( 4 );

	VectorSubtract( org, vright, w->p[0] );
	VectorAdd( w->p[0], vup, w->p[0] );

	VectorAdd( org, vright, w->p[1] );
	VectorAdd( w->p[1], vup, w->p[1] );

	VectorAdd( org, vright, w->p[2] );
	VectorSubtract( w->p[2], vup, w->p[2] );

	VectorSubtract( org, vright, w->p[3] );
	VectorSubtract( w->p[3], vup, w->p[3] );

	w->numpoints = 4;

	return w;
}

/*
==================
CopyWinding
==================
*/
winding_t   *CopyWinding( winding_t *w ) {
	int size;
	winding_t   *c;

	c = AllocWinding( w->numpoints );
#if FIXME
	size = (int)( (winding_t *)0 )->p[w->numpoints];
#else
	size = static_cast<int>(reinterpret_cast<intptr_t>((static_cast<winding_t*>(NULL))->p[w->numpoints]));
#endif // FIXME

#if defined RTCW_SP
	Com_Memcpy( c, w, size );
#else
	memcpy( c, w, size );
#endif // RTCW_XX

	return c;
}

/*
==================
ReverseWinding
==================
*/
winding_t   *ReverseWinding( winding_t *w ) {
	int i;
	winding_t   *c;

	c = AllocWinding( w->numpoints );
	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		VectorCopy( w->p[w->numpoints - 1 - i], c->p[i] );
	}
	c->numpoints = w->numpoints;
	return c;
}


/*
=============
ClipWindingEpsilon
=============
*/
void    ClipWindingEpsilon( winding_t *in, vec3_t normal, vec_t dist,
							vec_t epsilon, winding_t **front, winding_t **back ) {
	vec_t dists[MAX_POINTS_ON_WINDING + 4];
	int sides[MAX_POINTS_ON_WINDING + 4];
	int counts[3];
	static vec_t dot;           // VC 4.2 optimizer bug if not static
	int i, j;
	vec_t   *p1, *p2;
	vec3_t mid;
	winding_t   *f, *b;
	int maxpts;

	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		dot = DotProduct( in->p[i], normal );
		dot -= dist;
		dists[i] = dot;
		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		} else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		} else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	*front = *back = NULL;

	if ( !counts[0] ) {
		*back = CopyWinding( in );
		return;
	}
	if ( !counts[1] ) {
		*front = CopyWinding( in );
		return;
	}

	maxpts = in->numpoints + 4;   // cant use counts[0]+2 because
								  // of fp grouping errors

	*front = f = AllocWinding( maxpts );
	*back = b = AllocWinding( maxpts );

	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		p1 = in->p[i];

		if ( sides[i] == SIDE_ON ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
			VectorCopy( p1, b->p[b->numpoints] );
			b->numpoints++;
			continue;
		}

		if ( sides[i] == SIDE_FRONT ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
		}
		if ( sides[i] == SIDE_BACK ) {
			VectorCopy( p1, b->p[b->numpoints] );
			b->numpoints++;
		}

		if ( sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		// generate a split point
		p2 = in->p[( i + 1 ) % in->numpoints];

		dot = dists[i] / ( dists[i] - dists[i + 1] );
		for ( j = 0 ; j < 3 ; j++ )
		{   // avoid round off error when possible
			if ( normal[j] == 1 ) {
				mid[j] = dist;
			} else if ( normal[j] == -1 ) {
				mid[j] = -dist;
			} else {
				mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
			}
		}

		VectorCopy( mid, f->p[f->numpoints] );
		f->numpoints++;
		VectorCopy( mid, b->p[b->numpoints] );
		b->numpoints++;
	}

	if ( f->numpoints > maxpts || b->numpoints > maxpts ) {
		Com_Error( ERR_DROP, "ClipWinding: points exceeded estimate" );
	}
	if ( f->numpoints > MAX_POINTS_ON_WINDING || b->numpoints > MAX_POINTS_ON_WINDING ) {
		Com_Error( ERR_DROP, "ClipWinding: MAX_POINTS_ON_WINDING" );
	}
}


/*
=============
ChopWindingInPlace
=============
*/
void ChopWindingInPlace( winding_t **inout, vec3_t normal, vec_t dist, vec_t epsilon ) {
	winding_t   *in;
	vec_t dists[MAX_POINTS_ON_WINDING + 4];
	int sides[MAX_POINTS_ON_WINDING + 4];
	int counts[3];
	static vec_t dot;           // VC 4.2 optimizer bug if not static
	int i, j;
	vec_t   *p1, *p2;
	vec3_t mid;
	winding_t   *f;
	int maxpts;

	in = *inout;
	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		dot = DotProduct( in->p[i], normal );
		dot -= dist;
		dists[i] = dot;
		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		} else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		} else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if ( !counts[0] ) {
		FreeWinding( in );
		*inout = NULL;
		return;
	}
	if ( !counts[1] ) {
		return;     // inout stays the same

	}
	maxpts = in->numpoints + 4;   // cant use counts[0]+2 because
								  // of fp grouping errors

	f = AllocWinding( maxpts );

	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		p1 = in->p[i];

		if ( sides[i] == SIDE_ON ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
			continue;
		}

		if ( sides[i] == SIDE_FRONT ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
		}

		if ( sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		// generate a split point
		p2 = in->p[( i + 1 ) % in->numpoints];

		dot = dists[i] / ( dists[i] - dists[i + 1] );
		for ( j = 0 ; j < 3 ; j++ )
		{   // avoid round off error when possible
			if ( normal[j] == 1 ) {
				mid[j] = dist;
			} else if ( normal[j] == -1 ) {
				mid[j] = -dist;
			} else {
				mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
			}
		}

		VectorCopy( mid, f->p[f->numpoints] );
		f->numpoints++;
	}

	if ( f->numpoints > maxpts ) {
		Com_Error( ERR_DROP, "ClipWinding: points exceeded estimate" );
	}
	if ( f->numpoints > MAX_POINTS_ON_WINDING ) {
		Com_Error( ERR_DROP, "ClipWinding: MAX_POINTS_ON_WINDING" );
	}

	FreeWinding( in );
	*inout = f;
}


/*
=================
ChopWinding

Returns the fragment of in that is on the front side
of the cliping plane.  The original is freed.
=================
*/
winding_t   *ChopWinding( winding_t *in, vec3_t normal, vec_t dist ) {
	winding_t   *f, *b;

	ClipWindingEpsilon( in, normal, dist, ON_EPSILON, &f, &b );
	FreeWinding( in );
	if ( b ) {
		FreeWinding( b );
	}
	return f;
}


/*
=================
CheckWinding

=================
*/
void CheckWinding( winding_t *w ) {
	int i, j;
	vec_t   *p1, *p2;
	vec_t d, edgedist;
	vec3_t dir, edgenormal, facenormal;
	vec_t area;
	vec_t facedist;

	if ( w->numpoints < 3 ) {
		Com_Error( ERR_DROP, "CheckWinding: %i points",w->numpoints );
	}

	area = WindingArea( w );
	if ( area < 1 ) {
		Com_Error( ERR_DROP, "CheckWinding: %f area", area );
	}

	WindingPlane( w, facenormal, &facedist );

	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		p1 = w->p[i];

		for ( j = 0 ; j < 3 ; j++ )

#if !defined RTCW_MP
			if ( p1[j] > MAX_MAP_BOUNDS || p1[j] < -MAX_MAP_BOUNDS ) {
#else
			if ( p1[j] > BOGUS_RANGE || p1[j] < -BOGUS_RANGE ) {
#endif // RTCW_XX

#if !defined RTCW_ET
				Com_Error( ERR_DROP, "CheckFace: BUGUS_RANGE: %f",p1[j] );
#else
				Com_Error( ERR_DROP, "CheckFace: MAX_MAP_BOUNDS: %f",p1[j] );
#endif // RTCW_XX

			}

		j = i + 1 == w->numpoints ? 0 : i + 1;

		// check the point is on the face plane
		d = DotProduct( p1, facenormal ) - facedist;
		if ( d < -ON_EPSILON || d > ON_EPSILON ) {
			Com_Error( ERR_DROP, "CheckWinding: point off plane" );
		}

		// check the edge isnt degenerate
		p2 = w->p[j];
		VectorSubtract( p2, p1, dir );

		if ( VectorLength( dir ) < ON_EPSILON ) {
			Com_Error( ERR_DROP, "CheckWinding: degenerate edge" );
		}

		CrossProduct( facenormal, dir, edgenormal );
		VectorNormalize2( edgenormal, edgenormal );
		edgedist = DotProduct( p1, edgenormal );
		edgedist += ON_EPSILON;

		// all other points must be on front side
		for ( j = 0 ; j < w->numpoints ; j++ )
		{
			if ( j == i ) {
				continue;
			}
			d = DotProduct( w->p[j], edgenormal );
			if ( d > edgedist ) {
				Com_Error( ERR_DROP, "CheckWinding: non-convex" );
			}
		}
	}
}


/*
============
WindingOnPlaneSide
============
*/
int     WindingOnPlaneSide( winding_t *w, vec3_t normal, vec_t dist ) {
	qboolean front, back;
	int i;
	vec_t d;

	front = qfalse;
	back = qfalse;
	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		d = DotProduct( w->p[i], normal ) - dist;
		if ( d < -ON_EPSILON ) {
			if ( front ) {
				return SIDE_CROSS;
			}
			back = qtrue;
			continue;
		}
		if ( d > ON_EPSILON ) {
			if ( back ) {
				return SIDE_CROSS;
			}
			front = qtrue;
			continue;
		}
	}

	if ( back ) {
		return SIDE_BACK;
	}
	if ( front ) {
		return SIDE_FRONT;
	}
	return SIDE_ON;
}


/*
=================
AddWindingToConvexHull

Both w and *hull are on the same plane
=================
*/
#define MAX_HULL_POINTS     128
void    AddWindingToConvexHull( winding_t *w, winding_t **hull, vec3_t normal ) {
	int i, j, k;
	float       *p, *copy;
	vec3_t dir;
	float d;
	int numHullPoints, numNew;
	vec3_t hullPoints[MAX_HULL_POINTS];
	vec3_t newHullPoints[MAX_HULL_POINTS];
	vec3_t hullDirs[MAX_HULL_POINTS];
	qboolean hullSide[MAX_HULL_POINTS];
	qboolean outside;

	if ( !*hull ) {
		*hull = CopyWinding( w );
		return;
	}

	numHullPoints = ( *hull )->numpoints;

#if defined RTCW_SP
	Com_Memcpy( hullPoints, ( *hull )->p, numHullPoints * sizeof( vec3_t ) );
#else
	memcpy( hullPoints, ( *hull )->p, numHullPoints * sizeof( vec3_t ) );
#endif // RTCW_XX

	for ( i = 0 ; i < w->numpoints ; i++ ) {
		p = w->p[i];

		// calculate hull side vectors
		for ( j = 0 ; j < numHullPoints ; j++ ) {
			k = ( j + 1 ) % numHullPoints;

			VectorSubtract( hullPoints[k], hullPoints[j], dir );
			VectorNormalize2( dir, dir );
			CrossProduct( normal, dir, hullDirs[j] );
		}

		outside = qfalse;
		for ( j = 0 ; j < numHullPoints ; j++ ) {
			VectorSubtract( p, hullPoints[j], dir );
			d = DotProduct( dir, hullDirs[j] );
			if ( d >= ON_EPSILON ) {
				outside = qtrue;
			}
			if ( d >= -ON_EPSILON ) {
				hullSide[j] = qtrue;
			} else {
				hullSide[j] = qfalse;
			}
		}

		// if the point is effectively inside, do nothing
		if ( !outside ) {
			continue;
		}

		// find the back side to front side transition
		for ( j = 0 ; j < numHullPoints ; j++ ) {
			if ( !hullSide[ j % numHullPoints ] && hullSide[ ( j + 1 ) % numHullPoints ] ) {
				break;
			}
		}
		if ( j == numHullPoints ) {
			continue;
		}

		// insert the point here
		VectorCopy( p, newHullPoints[0] );
		numNew = 1;

		// copy over all points that aren't double fronts
		j = ( j + 1 ) % numHullPoints;
		for ( k = 0 ; k < numHullPoints ; k++ ) {
			if ( hullSide[ ( j + k ) % numHullPoints ] && hullSide[ ( j + k + 1 ) % numHullPoints ] ) {
				continue;
			}
			copy = hullPoints[ ( j + k + 1 ) % numHullPoints ];
			VectorCopy( copy, newHullPoints[numNew] );
			numNew++;
		}

		numHullPoints = numNew;

#if defined RTCW_SP
		Com_Memcpy( hullPoints, newHullPoints, numHullPoints * sizeof( vec3_t ) );
#else
		memcpy( hullPoints, newHullPoints, numHullPoints * sizeof( vec3_t ) );
#endif // RTCW_XX

	}

	FreeWinding( *hull );
	w = AllocWinding( numHullPoints );
	w->numpoints = numHullPoints;
	*hull = w;

#if defined RTCW_SP
	Com_Memcpy( w->p, hullPoints, numHullPoints * sizeof( vec3_t ) );
#else
	memcpy( w->p, hullPoints, numHullPoints * sizeof( vec3_t ) );
#endif // RTCW_XX

}


