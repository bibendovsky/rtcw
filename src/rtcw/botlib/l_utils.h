/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		l_util.h
 *
 * desc:		utils
 *
 *
 *****************************************************************************/

#define Vector2Angles( v,a )      vectoangles( v,a )

#if !defined RTCW_ET
#define MAX_PATH                MAX_QPATH
#else
#ifndef MAX_PATH // LBO 1/25/05
#define MAX_PATH                MAX_QPATH
#endif
#endif // RTCW_XX

#define Maximum( x,y )            ( x > y ? x : y )
#define Minimum( x,y )            ( x < y ? x : y )
