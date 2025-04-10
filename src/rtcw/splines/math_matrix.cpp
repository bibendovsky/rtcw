/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "q_splineshared.h"

mat3_t mat3_default( idVec3( 1, 0, 0 ), idVec3( 0, 1, 0 ), idVec3( 0, 0, 1 ) );

void toMatrix( quat_t const &src, mat3_t &dst ) {
	float wx, wy, wz;
	float xx, yy, yz;
	float xy, xz, zz;
	float x2, y2, z2;

	x2 = src.x + src.x;
	y2 = src.y + src.y;
	z2 = src.z + src.z;

	xx = src.x * x2;
	xy = src.x * y2;
	xz = src.x * z2;

	yy = src.y * y2;
	yz = src.y * z2;
	zz = src.z * z2;

	wx = src.w * x2;
	wy = src.w * y2;
	wz = src.w * z2;

	dst[ 0 ][ 0 ] = 1.0f - ( yy + zz );
	dst[ 0 ][ 1 ] = xy - wz;
	dst[ 0 ][ 2 ] = xz + wy;

	dst[ 1 ][ 0 ] = xy + wz;
	dst[ 1 ][ 1 ] = 1.0f - ( xx + zz );
	dst[ 1 ][ 2 ] = yz - wx;

	dst[ 2 ][ 0 ] = xz - wy;
	dst[ 2 ][ 1 ] = yz + wx;
	dst[ 2 ][ 2 ] = 1.0f - ( xx + yy );
}

void toMatrix( angles_t const &src, mat3_t &dst ) {
	float angle;
	static float sr, sp, sy, cr, cp, cy;    // static to help MS compiler fp bugs

	angle = src.yaw * ( M_PI * 2.0f / 360.0f );
	sy = sin( angle );
	cy = cos( angle );

	angle = src.pitch * ( M_PI * 2.0f / 360.0f );
	sp = sin( angle );
	cp = cos( angle );

	angle = src.roll * ( M_PI * 2.0f / 360.0f );
	sr = sin( angle );
	cr = cos( angle );

	dst[ 0 ].set( cp * cy, cp * sy, -sp );
	dst[ 1 ].set( sr * sp * cy + cr * -sy, sr * sp * sy + cr * cy, sr * cp );
	dst[ 2 ].set( cr * sp * cy + - sr * -sy, cr * sp * sy + - sr * cy, cr * cp );
}

void toMatrix( idVec3 const &src, mat3_t &dst ) {
	angles_t sup = src;
	toMatrix( sup, dst );
}

void mat3_t::ProjectVector( const idVec3 &src, idVec3 &dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
}

void mat3_t::UnprojectVector( const idVec3 &src, idVec3 &dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z;
}

void mat3_t::Transpose( mat3_t &matrix ) {
	int i;
	int j;

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			matrix[ i ][ j ] = mat[ j ][ i ];
		}
	}
}

void mat3_t::Transpose( void ) {
	float temp;
	int i;
	int j;

	for ( i = 0; i < 3; i++ ) {
		for ( j = i + 1; j < 3; j++ ) {
			temp = mat[ i ][ j ];
			mat[ i ][ j ] = mat[ j ][ i ];
			mat[ j ][ i ] = temp;
		}
	}
}

mat3_t mat3_t::Inverse( void ) const {
	mat3_t inv( *this );

	inv.Transpose();

	return inv;
}

void mat3_t::Clear( void ) {
	mat[0].set( 1, 0, 0 );
	mat[1].set( 0, 1, 0 );
	mat[2].set( 0, 0, 1 );
}
