/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_move.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

#ifdef AASINTERN
extern aas_settings_t aassettings;
#endif //AASINTERN

//movement prediction
int AAS_PredictClientMovement( struct aas_clientmove_s *move,
							   int entnum, vec3_t origin,
							   int presencetype, int onground,
							   vec3_t velocity, vec3_t cmdmove,
							   int cmdframes,
							   int maxframes, float frametime,
							   int stopevent, int stopareanum, int visualize );
//returns true if on the ground at the given origin
int AAS_OnGround( vec3_t origin, int presencetype, int passent );
//returns true if swimming at the given origin
int AAS_Swimming( vec3_t origin );
//returns the jump reachability run start point
void AAS_JumpReachRunStart( struct aas_reachability_s *reach, vec3_t runstart );
//returns true if against a ladder at the given origin
int AAS_AgainstLadder( vec3_t origin, int ms_areanum );
//rocket jump Z velocity when rocket-jumping at origin
float AAS_RocketJumpZVelocity( vec3_t origin );
//bfg jump Z velocity when bfg-jumping at origin
float AAS_BFGJumpZVelocity( vec3_t origin );
//calculates the horizontal velocity needed for a jump and returns true this velocity could be calculated
int AAS_HorizontalVelocityForJump( float zvel, vec3_t start, vec3_t end, float *velocity );
//
void AAS_SetMovedir( vec3_t angles, vec3_t movedir );
//
int AAS_DropToFloor( vec3_t origin, vec3_t mins, vec3_t maxs );
//
void AAS_InitSettings( void );
