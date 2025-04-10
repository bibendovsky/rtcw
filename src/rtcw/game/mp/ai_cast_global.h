/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

//===========================================================================
//
// Name:			ai_cast_global.h
// Function:		Global AI Cast defines
// Programmer:		Ridah
// Tab Size:		4 (real tabs)
//===========================================================================

// TTimo no typedef, "warning: useless keyword or type name in empty declaration"
struct cast_state_s;

#define AICAST_AIM_SPREAD   2048.0  // a really bad shooter will offset a maximum of this per shot, from the end point of the 8192 trace length

#define DANGER_MISSILE      ( 1 << 0 )
#define DANGER_CLIENTAIM    ( 1 << 1 )
#define DANGER_FLAMES       ( 1 << 2 )

extern qboolean saveGamePending;

qboolean AICast_SameTeam( struct cast_state_s *cs, int enemynum );
struct cast_state_s *AICast_GetCastState( int entitynum );
void AICast_ScriptLoad( void );
void AICast_ScriptEvent( struct cast_state_s *cs, const char *eventStr, const char *params );
void AICast_ForceScriptEvent( struct cast_state_s *cs, const char *eventStr, const char *params );
qboolean AICast_AIDamageOK( struct cast_state_s *cs, struct cast_state_s *ocs );
gentity_t *AICast_FindEntityForName( const char *name );
gentity_t *AICast_TravEntityForName( gentity_t *startent, const char *name );
void AICast_ScriptParse( struct cast_state_s *cs );
void AICast_StartFrame( int time );
void AICast_StartServerFrame( int time );
void AICast_RecordWeaponFire( gentity_t *ent );
void AICast_AIDoor_Touch( gentity_t *ent, gentity_t *aidoor_trigger, gentity_t *door );
float AICast_GetAccuracy( int entnum );
void AICast_Activate( int activatorNum, int entNum );
void AICast_CheckDangerousEntity( gentity_t *ent, int dangerFlags, float dangerDist, float tacticalLevel, float aggressionLevel, qboolean hurtFriendly );
qboolean AICast_NoFlameDamage( int entNum );
qboolean AICast_HasFiredWeapon( int entNum, int weapon );
void G_SetAASBlockingEntity( gentity_t *ent, qboolean blocking );
qboolean AICast_InFieldOfVision( vec3_t viewangles, float fov, vec3_t angles );
qboolean AICast_VisibleFromPos( vec3_t srcpos, int srcnum,
								vec3_t destpos, int destnum, qboolean updateVisPos );
qboolean AICast_AllowFlameDamage( int entNum );
void AICast_AdjustIdealYawForMover( int entnum, float yaw );
void AICast_AgePlayTime( int entnum );
int AICast_NoReload( int entnum );
void AICast_RecordScriptSound( int client );
void AICast_UpdateVisibility( gentity_t *srcent, gentity_t *destent, qboolean shareVis, qboolean directview );
void AICast_ProcessBullet( gentity_t *attacker, vec3_t start, vec3_t end );
void AICast_AudibleEvent( int srcnum, vec3_t pos, float range );
