/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_optimize.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

void AAS_Optimize( void );

#if defined RTCW_ET
void AAS_RemoveNonReachability( void );
void AAS_RemoveNonGrounded( void );
#endif // RTCW_XX

