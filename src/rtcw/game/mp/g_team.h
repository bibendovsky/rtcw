/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


// JPW NERVE -- more #defs for GT_WOLF gametype
#define WOLF_CAPTURE_BONUS      15      // capturing major game objective
#define WOLF_STEAL_OBJ_BONUS    10      // stealing objective (first part of capture)
#define WOLF_SECURE_OBJ_BONUS   10      // securing objective from slain enemy
#define WOLF_MEDIC_BONUS        2       // medic resurrect teammate
#define WOLF_REPAIR_BONUS       2       // engineer repair (mg42 etc) bonus
#define WOLF_DYNAMITE_BONUS     5       // engineer dynamite barriacade (dynamite only flag)
#define WOLF_FRAG_CARRIER_BONUS 10      // bonus for fragging enemy carrier
#define WOLF_FLAG_DEFENSE_BONUS 5       // bonus for frag when shooter or target near flag position
#define WOLF_CP_CAPTURE         3       // uncapped checkpoint bonus
#define WOLF_CP_RECOVER         5       // capping an enemy-held checkpoint bonus
#define WOLF_SP_CAPTURE         1       // uncapped spawnpoint bonus
#define WOLF_SP_RECOVER         2       // recovering enemy-held spawnpoint
#define WOLF_CP_PROTECT_BONUS   3       // protect a capture point by shooting target near it
#define WOLF_SP_PROTECT_BONUS   1       // protect a spawnpoint
#define WOLF_FRIENDLY_PENALTY   -3      // penalty for fragging teammate
#define WOLF_FRAG_BONUS         1       // score for fragging enemy soldier
#define WOLF_DYNAMITE_PLANT     5       // planted dynamite at objective
#define WOLF_DYNAMITE_DIFFUSE   5       // diffused dynamite at objective
#define WOLF_CP_PROTECT_RADIUS  600     // wolf capture protect radius
#define WOLF_AMMO_UP            1       // pt for giving ammo not to self
#define WOLF_HEALTH_UP          1       // pt for giving health not to self
#define AXIS_OBJECTIVE      1
#define ALLIED_OBJECTIVE    2
#define OBJECTIVE_DESTROYED 4
// jpw

#define CTF_CAPTURE_BONUS       5       // what you get for capture
#define CTF_TEAM_BONUS          0       // what your team gets for capture
#define CTF_RECOVERY_BONUS      1       // what you get for recovery
#define CTF_FLAG_BONUS          0       // what you get for picking up enemy flag
#define CTF_FRAG_CARRIER_BONUS  2       // what you get for fragging enemy flag carrier
#define CTF_FLAG_RETURN_TIME    40000   // seconds until auto return

#define CTF_CARRIER_DANGER_PROTECT_BONUS    2   // bonus for fraggin someone who has recently hurt your flag carrier
#define CTF_CARRIER_PROTECT_BONUS           1   // bonus for fraggin someone while either you or your target are near your flag carrier
#define CTF_FLAG_DEFENSE_BONUS              1   // bonus for fraggin someone while either you or your target are near your flag
#define CTF_RETURN_FLAG_ASSIST_BONUS        1   // awarded for returning a flag that causes a capture to happen almost immediately
#define CTF_FRAG_CARRIER_ASSIST_BONUS       2   // award for fragging a flag carrier if a capture happens almost immediately

#define CTF_TARGET_PROTECT_RADIUS           400 // the radius around an object being defended where a target will be worth extra frags
#define CTF_ATTACKER_PROTECT_RADIUS         400 // the radius around an object being defended where an attacker will get extra frags when making kills

#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT  8
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT     10000
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT      10000

#define CTF_GRAPPLE_SPEED                   750 // speed of grapple in flight
#define CTF_GRAPPLE_PULL_SPEED              750 // speed player is pulled at

// Prototypes

int OtherTeam( int team );
const char *TeamName( int team );
const char *OtherTeamName( int team );
const char *TeamColorString( int team );

void Team_DroppedFlagThink( gentity_t *ent );
void Team_FragBonuses( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker );
void Team_CheckHurtCarrier( gentity_t *targ, gentity_t *attacker );
void Team_InitGame( void );
void Team_ReturnFlag( int team );
void Team_FreeEntity( gentity_t *ent );
gentity_t *SelectCTFSpawnPoint( team_t team, int teamstate, vec3_t origin, vec3_t angles, int spawnObjective );
gentity_t *Team_GetLocation( gentity_t *ent );
qboolean Team_GetLocationMsg( gentity_t *ent, char *loc, int loclen );
void TeamplayInfoMessage( gentity_t *ent );
void CheckTeamStatus( void );

int Pickup_Team( gentity_t *ent, gentity_t *other );
