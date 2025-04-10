/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_routealt.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

#ifdef AASINTERN
void AAS_InitAlternativeRouting( void );
void AAS_ShutdownAlternativeRouting( void );
#endif //AASINTERN


int AAS_AlternativeRouteGoals( vec3_t start, vec3_t goal, int travelflags,
							   aas_altroutegoal_t *altroutegoals, int maxaltroutegoals,
							   int color );
