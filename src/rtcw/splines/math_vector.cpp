/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

//#include "q_shared.h"
#include "math_vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define M_PI        3.14159265358979323846  // matches value in gcc v2 math.h

#define LERP_DELTA 1e-6

idVec3 vec_zero( 0.0f, 0.0f, 0.0f );

Bounds boundsZero;

float idVec3::toYaw( void ) {
	float yaw;

	if ( ( y == 0 ) && ( x == 0 ) ) {
		yaw = 0;
	} else {
		yaw = atan2( y, x ) * 180 / M_PI;
		if ( yaw < 0 ) {
			yaw += 360;
		}
	}

	return yaw;
}

float idVec3::toPitch( void ) {
	float forward;
	float pitch;

	if ( ( x == 0 ) && ( y == 0 ) ) {
		if ( z > 0 ) {
			pitch = 90;
		} else {
			pitch = 270;
		}
	} else {
		forward = ( float )idSqrt( x * x + y * y );
		pitch = atan2( z, forward ) * 180 / M_PI;
		if ( pitch < 0 ) {
			pitch += 360;
		}
	}

	return pitch;
}

/*
angles_t idVec3::toAngles( void ) {
	float forward;
	float yaw;
	float pitch;

	if ( ( x == 0 ) && ( y == 0 ) ) {
		yaw = 0;
		if ( z > 0 ) {
			pitch = 90;
		} else {
			pitch = 270;
		}
	} else {
		yaw = atan2( y, x ) * 180 / M_PI;
		if ( yaw < 0 ) {
			yaw += 360;
		}

		forward = ( float )idSqrt( x * x + y * y );
		pitch = atan2( z, forward ) * 180 / M_PI;
		if ( pitch < 0 ) {
			pitch += 360;
		}
	}

	return angles_t( -pitch, yaw, 0 );
}
*/

idVec3 LerpVector( idVec3 &w1, idVec3 &w2, const float t ) {
	float omega, cosom, sinom, scale0, scale1;

	cosom = w1 * w2;
	if ( ( 1.0 - cosom ) > LERP_DELTA ) {
		omega = acos( cosom );
		sinom = sin( omega );
		scale0 = sin( ( 1.0 - t ) * omega ) / sinom;
		scale1 = sin( t * omega ) / sinom;
	} else {
		scale0 = 1.0 - t;
		scale1 = t;
	}

	return ( w1 * scale0 + w2 * scale1 );
}

/*
=============
idVec3::string

This is just a convenience function
for printing vectors
=============
*/
char *idVec3::string( void ) {
	static int index = 0;
	static char str[ 8 ][ 36 ];
	char    *s;

	// use an array so that multiple toString's won't collide
	s = str[ index ];
	index = ( index + 1 ) & 7;

	sprintf( s, "%.2f %.2f %.2f", x, y, z );

	return s;
}
