/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

//===========================================================================
//
// Name:			ai_cast_funcs.c
// Function:		Wolfenstein AI Character Decision Making
// Programmer:		Ridah
// Tab Size:		4 (real tabs)
//===========================================================================

#include "../game/g_local.h"
#include "../game/q_shared.h"
#include "../game/botlib.h"      //bot lib interface
#include "../game/be_aas.h"
#include "../game/be_ea.h"
#include "../game/be_ai_gen.h"
#include "../game/be_ai_goal.h"
#include "../game/be_ai_move.h"
#include "../botai/botai.h"          //bot ai interface

#include "ai_cast.h"

#if defined RTCW_SP
// TTimo: unused
//static vec3_t forward, right, up;
#endif  RTCW_XX

#if defined RTCW_SP
//=================================================================================
//
// Helga, the first boss
//
//=================================================================================
#elif defined RTCW_MP
//=================================================================================
//
// Helga (in zombie form), the first boss
//
//=================================================================================
#endif RTCW_XX

#if defined RTCW_SP
#define HELGA_SPIRIT_BUILDUP_TIME       8000    // last for this long
#define HELGA_SPIRIT_FADEOUT_TIME       1000
#define HELGA_SPIRIT_DLIGHT_RADIUS_MAX  384
#define HELGA_SPIRIT_FIRE_INTERVAL      1000

extern int lastZombieSpiritAttack;

char *AIFunc_Helga_SpiritAttack( cast_state_t *cs ) {
	bot_state_t *bs;
	gentity_t *ent;
	//
	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	ent = &g_entities[cs->entityNum];
	bs = cs->bs;
	// make sure we're still playing the right anim
	if ( ( ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) - BG_AnimationIndexForString( "attack1", cs->entityNum ) ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	if ( cs->enemyNum < 0 ) {
		ent->client->ps.torsoTimer  = 0;
		ent->client->ps.legsTimer   = 0;
		return AIFunc_DefaultStart( cs );
	}
	//
	// if we can't see them anymore, abort immediately
	if ( cs->vislist[cs->enemyNum].real_visible_timestamp != cs->vislist[cs->enemyNum].real_update_timestamp ) {
		ent->client->ps.torsoTimer  = 0;
		ent->client->ps.legsTimer   = 0;
		return AIFunc_DefaultStart( cs );
	}
	// we are firing this weapon, so record it
	cs->weaponFireTimes[WP_MONSTER_ATTACK2] = level.time;
	//
	// once an attack has started, only abort once the player leaves our view, or time runs out
	if ( cs->thinkFuncChangeTime < level.time - HELGA_SPIRIT_BUILDUP_TIME ) {
		// if enough time has elapsed, finish this attack
		if ( level.time > cs->thinkFuncChangeTime + HELGA_SPIRIT_BUILDUP_TIME + HELGA_SPIRIT_FADEOUT_TIME ) {
			ent->client->ps.torsoTimer  = 0;
			ent->client->ps.legsTimer   = 0;
			return AIFunc_DefaultStart( cs );
		}
	} else {

		// set timers
		ent->client->ps.torsoTimer  = 1000;
		ent->client->ps.legsTimer   = 1000;

		// draw the client-side effect
		ent->client->ps.eFlags |= EF_MONSTER_EFFECT;

		// inform the client of our enemies position
		VectorCopy( g_entities[cs->enemyNum].client->ps.origin, ent->s.origin2 );
		ent->s.origin2[2] += g_entities[cs->enemyNum].client->ps.viewheight;
	}
	//
	//
	return NULL;
}

char *AIFunc_Helga_SpiritAttack_Start( cast_state_t *cs ) {
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	ent->s.otherEntityNum2 = cs->enemyNum;
	ent->s.effect1Time = level.time;
	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	//
	// dont turn
	cs->ideal_viewangles[YAW] = cs->viewangles[YAW];
	// play an anim
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_WEAPON, WP_MONSTER_ATTACK2, qtrue );
	BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );
	//
	cs->aifunc = AIFunc_Helga_SpiritAttack;
	return "AIFunc_Helga_SpiritAttack";
}

//=================================================================================
//
// Standing melee attacks
//
//=================================================================================

#define NUM_HELGA_ANIMS     3
#define MAX_HELGA_IMPACTS   3
int helgaHitTimes[NUM_HELGA_ANIMS][MAX_HELGA_IMPACTS] = {   // up to three hits per attack
	{ANIMLENGTH( 16,20 ),-1},
	{ANIMLENGTH( 11,20 ),ANIMLENGTH( 19,20 ),-1},
	{ANIMLENGTH( 10,20 ),ANIMLENGTH( 17,20 ),ANIMLENGTH( 26,20 )},
};
int helgaHitDamage[NUM_HELGA_ANIMS] = {
	20,
	14,
	12
};

/*
================
AIFunc_Helga_Melee
================
*/
char *AIFunc_Helga_Melee( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	gentity_t *enemy;
	cast_state_t *ecs;
	int hitDelay = -1, anim;
	trace_t tr;
	float enemyDist;
	aicast_predictmove_t move;
	vec3_t vec;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( !ent->client->ps.torsoTimer || !ent->client->ps.legsTimer ) {
		cs->aiFlags &= ~AIFL_SPECIAL_FUNC;
		return AIFunc_DefaultStart( cs );
	}

	if ( cs->enemyNum < 0 ) {
		ent->client->ps.legsTimer = 0;      // allow legs us to move
		ent->client->ps.torsoTimer = 0;     // allow legs us to move
		cs->aiFlags &= ~AIFL_SPECIAL_FUNC;
		return AIFunc_DefaultStart( cs );
	}

	ecs = AICast_GetCastState( cs->enemyNum );
	enemy = &g_entities[cs->enemyNum];

	anim = ( ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) - BG_AnimationIndexForString( "attack3", cs->entityNum );
	if ( anim < 0 || anim >= NUM_HELGA_ANIMS ) {
		// animation interupted
		cs->aiFlags &= ~AIFL_SPECIAL_FUNC;
		return AIFunc_DefaultStart( cs );
		//G_Error( "AIFunc_HelgaZombieMelee: helgaBoss using invalid or unknown attack anim" );
	}
	if ( cs->animHitCount < MAX_HELGA_IMPACTS && helgaHitTimes[anim][cs->animHitCount] >= 0 ) {

		// face them
		VectorCopy( cs->bs->origin, vec );
		vec[2] += ent->client->ps.viewheight;
		VectorSubtract( enemy->client->ps.origin, vec, vec );
		VectorNormalize( vec );
		vectoangles( vec, cs->ideal_viewangles );
		cs->ideal_viewangles[PITCH] = AngleNormalize180( cs->ideal_viewangles[PITCH] );

		// get hitDelay
		if ( !cs->animHitCount ) {
			hitDelay = helgaHitTimes[anim][cs->animHitCount];
		} else {
			hitDelay = helgaHitTimes[anim][cs->animHitCount] - helgaHitTimes[anim][cs->animHitCount - 1];
		}

		// check for inflicting damage
		if ( level.time - cs->weaponFireTimes[cs->weaponNum] > hitDelay ) {
			// do melee damage
			enemyDist = VectorDistance( enemy->r.currentOrigin, ent->r.currentOrigin );
			enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
			enemyDist -= ent->r.maxs[0];
			if ( enemyDist < 10 + AICast_WeaponRange( cs, cs->weaponNum ) ) {
				trap_Trace( &tr, ent->r.currentOrigin, NULL, NULL, enemy->r.currentOrigin, ent->s.number, MASK_SHOT );
				if ( tr.entityNum == cs->enemyNum ) {
					G_Damage( &g_entities[tr.entityNum], ent, ent, vec3_origin, tr.endpos,
							  helgaHitDamage[anim], 0, MOD_GAUNTLET );
					G_AddEvent( enemy, EV_GENERAL_SOUND, G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[STAYSOUNDSCRIPT] ) );
				}
			}
			cs->weaponFireTimes[cs->weaponNum] = level.time;
			cs->animHitCount++;
		}
	}

	// if they are outside range, move forward
	AICast_PredictMovement( ecs, 2, 0.3, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
	VectorSubtract( move.endpos, cs->bs->origin, vec );
	vec[2] = 0;
	enemyDist = VectorLength( vec );
	enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
	enemyDist -= ent->r.maxs[0];
	if ( enemyDist > 8 ) {    // we can get closer
		//if (!ent->client->ps.legsTimer) {
		//	cs->castScriptStatus.scriptNoMoveTime = 0;
		trap_EA_MoveForward( cs->entityNum );
		//}
		//ent->client->ps.legsTimer = 0;		// allow legs us to move
	}

	return NULL;
}

/*
================
AIFunc_Helga_MeleeStart
================
*/
char *AIFunc_Helga_MeleeStart( cast_state_t *cs ) {
	gentity_t *ent;

	ent = &g_entities[cs->entityNum];
	ent->s.effect1Time = level.time;
	cs->ideal_viewangles[YAW] = cs->viewangles[YAW];
	cs->weaponFireTimes[cs->weaponNum] = level.time;
	cs->animHitCount = 0;
	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	// face them
	AICast_AimAtEnemy( cs );

	// play an anim
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_WEAPON, cs->weaponNum, qtrue );
	BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );

	// play a sound
	G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[ATTACKSOUNDSCRIPT] ) );

	cs->aifunc = AIFunc_Helga_Melee;
	cs->aifunc( cs );   // think once now, to prevent a delay
	return "AIFunc_Helga_Melee";
}


//===================================================================

/*
==============
AIFunc_FlameZombie_Portal
==============
*/
char *AIFunc_FlameZombie_Portal( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	if ( cs->thinkFuncChangeTime < level.time - PORTAL_ZOMBIE_SPAWNTIME ) {
		// HACK, make them aware of the player
		AICast_UpdateVisibility( &g_entities[cs->entityNum], AICast_FindEntityForName( "player" ), qfalse, qtrue );
		ent->s.time2 = 0;   // turn spawning effect off
		return AIFunc_DefaultStart( cs );
	}
	//
	return NULL;
}

/*
==============
AIFunc_FlameZombie_PortalStart
==============
*/
char *AIFunc_FlameZombie_PortalStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	ent->s.time2 = level.time + 200;    // hijacking this for portal spawning effect
	//
	// play a special animation
	ent->client->ps.torsoAnim =
		( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_EXTRA1;
	ent->client->ps.legsAnim =
		( ( ent->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_EXTRA1;
	ent->client->ps.torsoTimer = PORTAL_ZOMBIE_SPAWNTIME - 200;
	ent->client->ps.legsTimer = PORTAL_ZOMBIE_SPAWNTIME - 200;
	//
	cs->thinkFuncChangeTime = level.time;
	//
	cs->aifunc = AIFunc_FlameZombie_Portal;
	return "AIFunc_FlameZombie_Portal";
}


//=================================================================================
//
// Heinrich, the LAST boss
//
//=================================================================================

//
// Special Sound Precache
//

typedef enum
{
	HEINRICH_SWORDIMPACT,
	HEINRICH_SWORDLUNGE_START,
	HEINRICH_SWORDKNOCKBACK_START,
	HEINRICH_SWORDKNOCKBACK_WEAPON,
	HEINRICH_SWORDSIDESLASH_START,
	HEINRICH_SWORDSIDESLASH_WEAPON,
	HEINRICH_EARTHQUAKE_START,
	HEINRICH_RAISEDEAD_START,
	HEINRICH_TAUNT_GOODHEALTH,
	HEINRICH_TAUNT_LOWHEALTH,

	MAX_HEINRICH_SOUNDS
} heinrichSounds_t;

char *heinrichSounds[MAX_HEINRICH_SOUNDS] = {
	"heinrichSwordImpact",
	"heinrichSwordLungeStart",
	"heinrichSwordKnockbackStart",
	"heinrichSwordKnockbackWeapon",
	"heinrichSwordSideSlashStart",
	"heinrichSwordSideSlashWeapon",
	"heinrichSwordEarthquakeStart",
	"heinrichRaiseWarriorStart",
	"heinrichTauntGoodHealth",
	"heinrichTauntLowHealth",
};

int heinrichSoundIndex[MAX_HEINRICH_SOUNDS];

void AICast_Heinrich_SoundPrecache( void ) {
	int i;
	for ( i = 0; i < MAX_HEINRICH_SOUNDS; i++ ) {
		heinrichSoundIndex[i] = G_SoundIndex( heinrichSounds[i] );
	}
}

void AICast_Heinrich_Taunt( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	static int lastTaunt;
	// sound
	if ( ent->health > cs->attributes[STARTING_HEALTH] * 0.25 ) {
		if ( lastTaunt > level.time || lastTaunt < level.time - 20000 ) {
			G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_TAUNT_GOODHEALTH] );
			lastTaunt = level.time;
		}
	} else {
		if ( lastTaunt > level.time || lastTaunt < level.time - 40000 ) {
			G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_TAUNT_LOWHEALTH] );
			lastTaunt = level.time;
		}
	}
}

#define HEINRICH_LUNGE_DELAY    ANIMLENGTH( 15,20 )
#define HEINRICH_LUNGE_RANGE    170
#define HEINRICH_LUNGE_DAMAGE   ( 50 + rand() % 20 )

char *AIFunc_Heinrich_SwordLunge( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	trace_t *tr;
	vec3_t left;
	float enemyDist;
	aicast_predictmove_t move;
	vec3_t vec;
	cast_state_t *ecs;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( cs->enemyNum < 0 ) {
		if ( ent->client->ps.torsoTimer ) {
			return NULL;
		}
		return AIFunc_DefaultStart( cs );
	}

	ecs = AICast_GetCastState( cs->enemyNum );


	if ( ent->client->ps.torsoTimer < 500 ) {
		if ( !ent->client->ps.legsTimer ) {
			trap_EA_MoveForward( cs->entityNum );
		}
		ent->client->ps.legsTimer = 0;
		ent->client->ps.torsoTimer = 0;
		cs->castScriptStatus.scriptNoMoveTime = 0;
		AICast_Heinrich_Taunt( cs );
		return AIFunc_BattleChaseStart( cs );
	}

	// time for the melee?
	if ( cs->enemyNum >= 0 && !( cs->aiFlags & AIFL_MISCFLAG1 ) ) {
		// face them
		AICast_AimAtEnemy( cs );
		// keep checking for impact status
		tr = CheckMeleeAttack( ent, HEINRICH_LUNGE_RANGE, qfalse );
/*		// do we need to move?
		if (!(tr && (tr->entityNum == cs->enemyNum))) {
			ent->client->ps.legsTimer = 0;
			cs->castScriptStatus.scriptNoMoveTime = 0;
			trap_EA_MoveForward( cs->entityNum );
		}
*/                                                                                                                                                                                                           // ready for damage?
		if ( cs->thinkFuncChangeTime < level.time - HEINRICH_LUNGE_DELAY ) {
			cs->aiFlags |= AIFL_MISCFLAG1;
			// do melee damage
			if ( tr && ( tr->entityNum == cs->enemyNum ) ) {
				G_Damage( &g_entities[tr->entityNum], ent, ent, left, tr->endpos, HEINRICH_LUNGE_DAMAGE, 0, MOD_GAUNTLET );
				// sound
				G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDIMPACT] );
			}
		}
	}

	// if they are outside range, move forward
	AICast_PredictMovement( ecs, 2, 0.3, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
	VectorSubtract( move.endpos, cs->bs->origin, vec );
	vec[2] = 0;
	enemyDist = VectorLength( vec );
	enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
	enemyDist -= ent->r.maxs[0];
	if ( enemyDist > 30 ) {   // we can get closer
		if ( ent->client->ps.legsTimer ) {
			cs->castScriptStatus.scriptNoMoveTime = level.time + 100;
			ent->client->ps.legsTimer = 0;      // allow legs to move us
		}
		if ( cs->castScriptStatus.scriptNoMoveTime < level.time ) {
			trap_EA_MoveForward( cs->entityNum );
		}
	}

	return NULL;
}

char *AIFunc_Heinrich_SwordLungeStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
//	gentity_t	*enemy = &g_entities[cs->enemyNum];

	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	// sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDLUNGE_START] );
	// face them
	AICast_AimAtEnemy( cs );
	// clear flags
	cs->aiFlags &= ~( AIFL_MISCFLAG1 | AIFL_MISCFLAG2 );
	// play the anim
	BG_PlayAnimName( &ent->client->ps, "attack9", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	// start the func
	cs->aifunc = AIFunc_Heinrich_SwordLunge;
	return "AIFunc_Heinrich_SwordLunge";
}

#define HEINRICH_KNOCKBACK_DELAY    ANIMLENGTH( 26,20 )
#define HEINRICH_KNOCKBACK_RANGE    150
#define HEINRICH_KNOCKBACK_DAMAGE   ( 60 + rand() % 20 )

char *AIFunc_Heinrich_SwordKnockback( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	trace_t *tr;
	vec3_t right, left;
//	float	enemyDist;
//	aicast_predictmove_t move;
//	vec3_t	vec;
	cast_state_t *ecs;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( cs->enemyNum < 0 ) {
		if ( ent->client->ps.torsoTimer ) {
			return NULL;
		}
		return AIFunc_DefaultStart( cs );
	}

	ecs = AICast_GetCastState( cs->enemyNum );

	if ( ent->client->ps.torsoTimer < 500 ) {
		if ( !ent->client->ps.legsTimer ) {
			trap_EA_MoveForward( cs->entityNum );
		}
		ent->client->ps.legsTimer = 0;
		ent->client->ps.torsoTimer = 0;
		cs->castScriptStatus.scriptNoMoveTime = 0;
		AICast_Heinrich_Taunt( cs );
		return AIFunc_BattleChaseStart( cs );
	}

	// time for the melee?
	if ( cs->enemyNum >= 0 && !( cs->aiFlags & AIFL_MISCFLAG1 ) ) {
		// face them
		AICast_AimAtEnemy( cs );
		// keep checking for impact status
		tr = CheckMeleeAttack( ent, HEINRICH_KNOCKBACK_RANGE, qfalse );
/*		// do we need to move?
		if (!(tr && (tr->entityNum == cs->enemyNum))) {
			ent->client->ps.legsTimer = 0;
			cs->castScriptStatus.scriptNoMoveTime = 0;
			trap_EA_MoveForward( cs->entityNum );
		}
*/                                                                                                                                                                                                           // ready for damage?
		if ( cs->thinkFuncChangeTime < level.time - HEINRICH_KNOCKBACK_DELAY ) {
			cs->aiFlags |= AIFL_MISCFLAG1;
			// do melee damage
			if ( tr && ( tr->entityNum == cs->enemyNum ) ) {
				AngleVectors( cs->viewangles, NULL, right, NULL );
				VectorNegate( right, left );
				G_Damage( &g_entities[tr->entityNum], ent, ent, left, tr->endpos, HEINRICH_KNOCKBACK_DAMAGE, 0, MOD_GAUNTLET );
				// sound
				G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDIMPACT] );
				// throw them in direction of impact
				if ( ( ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) == BG_AnimationIndexForString( "attack2", cs->entityNum ) ) {
					// right
					right[2] = 0.5;
					VectorMA( g_entities[cs->enemyNum].client->ps.velocity, 400, right, g_entities[cs->enemyNum].client->ps.velocity );
				} else {
					// left
					left[2] = 0.5;
					VectorMA( g_entities[cs->enemyNum].client->ps.velocity, 400, left, g_entities[cs->enemyNum].client->ps.velocity );
				}
			}
		}
	}
/*	DISABLED FOR SWORDKNOCKBACK..looks bad
	// if they are outside range, move forward
	AICast_PredictMovement( ecs, 2, 0.3, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
	VectorSubtract( move.endpos, cs->bs->origin, vec );
	vec[2] = 0;
	enemyDist = VectorLength( vec );
	enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
	enemyDist -= ent->r.maxs[0];
	if (enemyDist > 30) {	// we can get closer
		if (ent->client->ps.legsTimer) {
			cs->castScriptStatus.scriptNoMoveTime = level.time + 100;
			ent->client->ps.legsTimer = 0;		// allow legs to move us
		}
		if (cs->castScriptStatus.scriptNoMoveTime < level.time) {
			trap_EA_MoveForward(cs->entityNum);
		}
	}
*/
	return NULL;
}

char *AIFunc_Heinrich_SwordKnockbackStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
//	gentity_t	*enemy = &g_entities[cs->enemyNum];

	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	// sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDKNOCKBACK_START] );
	// weapon sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDKNOCKBACK_WEAPON] );
	// face them
	AICast_AimAtEnemy( cs );
	// clear flags
	cs->aiFlags &= ~( AIFL_MISCFLAG1 | AIFL_MISCFLAG2 );
	// play the anim
	if ( rand() % 2 ) {
		BG_PlayAnimName( &ent->client->ps, "attack2", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	} else {
		BG_PlayAnimName( &ent->client->ps, "attack3", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	}
	// start the func
	cs->aifunc = AIFunc_Heinrich_SwordKnockback;
	return "AIFunc_Heinrich_SwordKnockback";
}

#define HEINRICH_SLASH_DELAY    ANIMLENGTH( 17,25 )
#define HEINRICH_SLASH_RANGE    140
#define HEINRICH_SLASH_DAMAGE   ( 30 + rand() % 15 )

char *AIFunc_Heinrich_SwordSideSlash( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	trace_t *tr;
	vec3_t right, left;
	float enemyDist;
	aicast_predictmove_t move;
	vec3_t vec;
	cast_state_t *ecs;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( cs->enemyNum < 0 ) {
		if ( ent->client->ps.torsoTimer ) {
			return NULL;
		}
		return AIFunc_DefaultStart( cs );
	}

	ecs = AICast_GetCastState( cs->enemyNum );

	if ( ent->client->ps.torsoTimer < 500 ) {
		if ( !ent->client->ps.legsTimer ) {
			trap_EA_MoveForward( cs->entityNum );
		}
		ent->client->ps.legsTimer = 0;
		ent->client->ps.torsoTimer = 0;
		cs->castScriptStatus.scriptNoMoveTime = 0;
		AICast_Heinrich_Taunt( cs );
		return AIFunc_BattleChaseStart( cs );
	}

	// time for the melee?
	if ( cs->enemyNum >= 0 && !( cs->aiFlags & AIFL_MISCFLAG1 ) ) {
		// face them
		AICast_AimAtEnemy( cs );
		// keep checking for impact status
		tr = CheckMeleeAttack( ent, HEINRICH_SLASH_RANGE, qfalse );
		// ready for damage?
		if ( cs->thinkFuncChangeTime < level.time - HEINRICH_SLASH_DELAY ) {
			cs->aiFlags |= AIFL_MISCFLAG1;
			// do melee damage
			if ( tr && ( tr->entityNum == cs->enemyNum ) ) {
				AngleVectors( cs->viewangles, NULL, right, NULL );
				VectorNegate( right, left );
				G_Damage( &g_entities[tr->entityNum], ent, ent, left, tr->endpos, HEINRICH_SLASH_DAMAGE, 0, MOD_GAUNTLET );
				// sound
				G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDIMPACT] );
				// throw them in direction of impact
				left[2] = 0.5;
				VectorMA( g_entities[cs->enemyNum].client->ps.velocity, 400, left, g_entities[cs->enemyNum].client->ps.velocity );
			}
		}
	}

	// if they are outside range, move forward
	AICast_PredictMovement( ecs, 2, 0.3, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
	VectorSubtract( move.endpos, cs->bs->origin, vec );
	vec[2] = 0;
	enemyDist = VectorLength( vec );
	enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
	enemyDist -= ent->r.maxs[0];
	if ( enemyDist > 30 ) {   // we can get closer
		if ( ent->client->ps.legsTimer ) {
			cs->castScriptStatus.scriptNoMoveTime = level.time + 100;
			ent->client->ps.legsTimer = 0;      // allow legs to move us
		}
		if ( cs->castScriptStatus.scriptNoMoveTime < level.time ) {
			trap_EA_MoveForward( cs->entityNum );
		}
	}

	return NULL;
}

char *AIFunc_Heinrich_SwordSideSlashStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];

	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	// sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDSIDESLASH_START] );
	// weapon sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDSIDESLASH_WEAPON] );
	// face them
	AICast_AimAtEnemy( cs );
	// clear flags
	cs->aiFlags &= ~( AIFL_MISCFLAG1 | AIFL_MISCFLAG2 );
	// play the anim
	BG_PlayAnimName( &ent->client->ps, "attack8", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	// start the func
	cs->aifunc = AIFunc_Heinrich_SwordSideSlash;
	return "AIFunc_Heinrich_SwordSideSlash";
}

#define HEINRICH_STOMP_DELAY        900
#define HEINRICH_STOMP_RANGE        1024.0
#define HEINRICH_STOMP_VELOCITY_Z   420
#define HEINRICH_STOMP_DAMAGE       35

char *AIFunc_Heinrich_Earthquake( cast_state_t *cs ) {
	gentity_t   *ent = &g_entities[cs->entityNum];
	gentity_t   *enemy;
	cast_state_t *ecs;
	vec3_t enemyVec;
	float enemyDist, scale;
	trace_t *tr;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( cs->enemyNum < 0 ) {
		if ( !ent->client->ps.torsoTimer ) {
			return AIFunc_DefaultStart( cs );
		}
		return NULL;
	}

	enemy = &g_entities[cs->enemyNum];
	ecs = AICast_GetCastState( cs->enemyNum );

	VectorMA( enemy->r.currentOrigin, HEINRICH_STOMP_DELAY, enemy->client->ps.velocity, enemyVec );
	enemyDist = VectorDistance( ent->r.currentOrigin, enemyVec );

	if ( ent->client->ps.torsoTimer < 500 ) {
		int rnd;
		aicast_predictmove_t move;
		vec3_t vec;

		AICast_PredictMovement( ecs, 2, 0.5, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
		VectorSubtract( move.endpos, cs->bs->origin, vec );
		vec[2] = 0;
		enemyDist = VectorLength( vec );
		enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
		enemyDist -= ent->r.maxs[0];
		//
		if ( enemyDist < 140 ) {
			// combo attack
			rnd = rand() % 3;
			switch ( rnd ) {
			case 0:
				return AIFunc_Heinrich_SwordSideSlashStart( cs );
			case 1:
				return AIFunc_Heinrich_SwordKnockbackStart( cs );
			case 2:
				return AIFunc_Heinrich_SwordLungeStart( cs );
			}
		} else {    // back to roaming
			ent->client->ps.legsTimer = 0;
			ent->client->ps.torsoTimer = 0;
			cs->castScriptStatus.scriptNoMoveTime = 0;
			AICast_Heinrich_Taunt( cs );
			return AIFunc_DefaultStart( cs );
		}
	}

	// time for the thump?
	if ( !( cs->aiFlags & AIFL_MISCFLAG1 ) ) {
		// face them
		AICast_AimAtEnemy( cs );
		// ready for damage?
		if ( cs->thinkFuncChangeTime < level.time - HEINRICH_STOMP_DELAY ) {
			cs->aiFlags |= AIFL_MISCFLAG1;
			// play the stomp sound
			G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[ORDERSDENYSOUNDSCRIPT] ) );
			// check for striking the player
			tr = CheckMeleeAttack( ent, 70, qfalse );
			// do melee damage
			if ( tr && ( tr->entityNum == cs->enemyNum ) ) {
				G_Damage( &g_entities[tr->entityNum], ent, ent, vec3_origin, tr->endpos, HEINRICH_STOMP_DAMAGE, 0, MOD_GAUNTLET );
			}
			// call the debris trigger
			AICast_ScriptEvent( cs, "trigger", "quake" );
		}
	}

	enemyDist = Distance( enemy->s.pos.trBase, ent->s.pos.trBase );

	// do the earthquake effects
	if ( cs->thinkFuncChangeTime < level.time - HEINRICH_STOMP_DELAY ) {
		// throw the player into the air, if they are on the ground
		if ( ( enemy->s.groundEntityNum != ENTITYNUM_NONE ) && enemyDist < HEINRICH_STOMP_RANGE ) {
			scale = 0.5 + 0.5 * ( (float)ent->client->ps.torsoTimer / 1000.0 );
			if ( scale > 1.0 ) {
				scale = 1.0;
			}
			VectorSubtract( ent->s.pos.trBase, enemy->s.pos.trBase, enemyVec );
			VectorScale( enemyVec, 2.0 * ( 0.6 + 0.5 * random() ) * scale * ( 0.6 + 0.6 * ( 1.0 - ( enemyDist / HEINRICH_STOMP_RANGE ) ) ), enemyVec );
			enemyVec[2] = scale * HEINRICH_STOMP_VELOCITY_Z * ( 1.0 - 0.5 * ( enemyDist / HEINRICH_STOMP_RANGE ) );
			// bounce the player using this velocity
			VectorAdd( enemy->client->ps.velocity, enemyVec, enemy->client->ps.velocity );
		}
	}

	return NULL;
}

char *AIFunc_Heinrich_MeleeStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	gentity_t   *enemy = &g_entities[cs->enemyNum];
	int rnd;
	static int lastStomp;

	if ( cs->enemyNum < 0 ) {
		return NULL;
	}

	// record weapon fire
	cs->weaponFireTimes[cs->weaponNum] = level.time;
	// face them
	AICast_AimAtEnemy( cs );
	// clear flags
	cs->aiFlags &= ~( AIFL_MISCFLAG1 | AIFL_MISCFLAG2 );
	// decide which attack to use
	if ( VectorDistance( ent->r.currentOrigin, enemy->r.currentOrigin ) < 60 ) {
		rnd = 0;    // sword slash up close
	} else if ( VectorDistance( ent->r.currentOrigin, enemy->r.currentOrigin ) >= HEINRICH_SLASH_RANGE ) {
		rnd = 1;    // too far away, stomp
	} else {
		// pick at random
		rnd = rand() % 2;
	}
	//
	switch ( rnd ) {
	case 0:
	{
		int rnd = rand() % 3;
		switch ( rnd ) {
		case 0:
			return AIFunc_Heinrich_SwordSideSlashStart( cs );
		case 1:
			return AIFunc_Heinrich_SwordKnockbackStart( cs );
		case 2:
			return AIFunc_Heinrich_SwordLungeStart( cs );
		}
	}
	case 1:
		// dont do stomp too often
		if ( lastStomp > level.time - 12000 ) {   // plenty of time to let debris disappear
			return NULL;
		}
		lastStomp = level.time;
		cs->aiFlags |= AIFL_SPECIAL_FUNC;
		// sound
		G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_EARTHQUAKE_START] );
		// play the anim
		BG_PlayAnimName( &ent->client->ps, "attack7", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
		// start the func
		cs->aifunc = AIFunc_Heinrich_Earthquake;
		return "AIFunc_Heinrich_Earthquake";
	}
	// shutup compiler
	return NULL;
}

#define HEINRICH_RAISEDEAD_DELAY        1200
#define HEINRICH_RAISEDEAD_COUNT        3
int lastRaise;

char *AIFunc_Heinrich_RaiseDead( cast_state_t *cs ) {
	int i;
	gentity_t   *ent = &g_entities[cs->entityNum];
	gentity_t   *enemy = &g_entities[cs->enemyNum];
	gentity_t *trav, *closest;
	float closestDist, dist;
	//
	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	if ( cs->enemyNum < 0 ) {
		if ( !ent->client->ps.torsoTimer ) {
			return AIFunc_DefaultStart( cs );
		}
		return NULL;
	}
	//
	// record weapon fire
	cs->weaponFireTimes[cs->weaponNum] = level.time;
	//
	if ( !ent->client->ps.torsoTimer ) {
		return AIFunc_DefaultStart( cs );
	}
	if ( ent->count2 && lastRaise < level.time - HEINRICH_RAISEDEAD_DELAY ) {
		lastRaise = level.time;
		// summons the closest warrior
		closest = NULL;
		closestDist = 0;    // shutup the compiler
		for ( i = 0, trav = g_entities; i < level.maxclients; i++, trav++ ) {
			if ( !trav->inuse ) {
				continue;
			}
			if ( !trav->aiInactive ) {
				continue;
			}
			if ( trav->aiCharacter != AICHAR_WARZOMBIE ) {
				continue;
			}
			dist = VectorDistance( trav->s.pos.trBase, enemy->r.currentOrigin );
			if ( !closest || dist < closestDist ) {
				closest = trav;
				closestDist = dist;
			}
		}
		//
		if ( closest ) {
			closest->AIScript_AlertEntity( closest );
			// make them aware of the player
			AICast_UpdateVisibility( closest, enemy, qtrue, qtrue );
			// reduce the count
			ent->count2--;
		}
	}
	//
	return NULL;
}

char *AIFunc_Heinrich_RaiseDeadStart( cast_state_t *cs ) {
	int i, cnt, free;
	gentity_t   *ent = &g_entities[cs->entityNum];
//	gentity_t	*enemy = &g_entities[cs->enemyNum];
	gentity_t *trav, *spirits;
	float circleDist;
	//
	// count the number of active warriors
	cnt = 0;
	free = 0;
	for ( i = 0, trav = g_entities; i < level.maxclients; i++, trav++ ) {
		if ( !trav->inuse ) {
			continue;
		}
		if ( trav->aiCharacter != AICHAR_WARZOMBIE ) {
			continue;
		}
		if ( trav->aiInactive ) {
			free++;
			continue;
		}
		if ( trav->health <= 0 ) {
			continue;
		}
		cnt++;
	}
	//
	if ( cnt < HEINRICH_RAISEDEAD_COUNT && free ) {   // need a new one
		cs->aiFlags &= ~AIFL_MISCFLAG1;
		ent->count2 = HEINRICH_RAISEDEAD_COUNT - cnt;
		lastRaise = level.time;
		cs->aiFlags |= AIFL_SPECIAL_FUNC;
		// start the animation
		BG_PlayAnimName( &ent->client->ps, "attack4", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
		// play the sound
		G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_RAISEDEAD_START] );
		// start the func
		cs->aifunc = AIFunc_Heinrich_RaiseDead;
		return "AIFunc_Heinrich_RaiseDead";
	}
	// enable all the spirit spawners
	trav = NULL;
	// TTimo: gcc: suggest () around assignment used as truth value
	while ( ( trav = G_Find( trav, FOFS( classname ), "func_bats" ) ) ) {
		if ( !trav->active && trav->spawnflags & 4 ) {
			trav->active = 1;   // let them release spirits now
		}
	}
	// is the player outside the circle?
	trav = NULL;
	// TTimo: gcc: suggest () around assignment used as truth value
	while ( ( trav = G_Find( trav, FOFS( classname ), "func_bats" ) ) ) {
		if ( trav->spawnflags & 4 ) {
			spirits = trav;
			circleDist = trav->radius;
			trav = G_Find( NULL, FOFS( targetname ), trav->target );
			if ( trav ) {
				if ( VectorDistance( g_entities[0].s.pos.trBase, trav->s.origin ) > circleDist ) {
					cs->aiFlags &= ~AIFL_MISCFLAG1;
					ent->count2 = 0;
					cs->aiFlags |= AIFL_SPECIAL_FUNC;
					// start the animation
					BG_PlayAnimName( &ent->client->ps, "attack4", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
					// play the sound
					G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_RAISEDEAD_START] );
					// start the func
					cs->aifunc = AIFunc_Heinrich_RaiseDead;
					return "AIFunc_Heinrich_RaiseDead";
				}
			}
			break;
		}
	}
	//
	return NULL;
}

char *AIFunc_Heinrich_SpawnSpiritsStart( cast_state_t *cs ) {
	gentity_t   *ent = &g_entities[cs->entityNum];
	gentity_t *trav, *spirits;
	float circleDist;
	//
	// enable all the spirit spawners
	trav = NULL;
	// TTimo: gcc: suggest () around assignment used as truth value
	while ( ( trav = G_Find( trav, FOFS( classname ), "func_bats" ) ) ) {
		if ( !trav->active && trav->spawnflags & 4 ) {
			trav->active = 1;   // let them release spirits now
		}
	}
	// is the player outside the circle?
	trav = NULL;
	// TTimo: gcc: suggest () around assignment used as truth value
	while ( ( trav = G_Find( trav, FOFS( classname ), "func_bats" ) ) ) {
		if ( trav->spawnflags & 4 ) {
			spirits = trav;
			circleDist = trav->radius;
			trav = G_Find( NULL, FOFS( targetname ), trav->target );
			if ( trav ) {
				if ( VectorDistance( g_entities[0].s.pos.trBase, trav->s.origin ) > circleDist ) {
					cs->aiFlags &= ~AIFL_MISCFLAG1;
					ent->count2 = 0;
					cs->aiFlags |= AIFL_SPECIAL_FUNC;
					// start the animation
					BG_PlayAnimName( &ent->client->ps, "attack4", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
					// play the sound
					G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_RAISEDEAD_START] );
					// start the func
					cs->aifunc = AIFunc_Heinrich_RaiseDead; // just do raise dead, without raising any warriors
					return "AIFunc_Heinrich_RaiseDead";
				}
			}
			break;
		}
	}
	//
	return NULL;
}
#endif RTCW_XX


#if defined RTCW_MP
void AICast_FZombie_StartLightning( gentity_t *ent ) {
	ent->AIScript_AlertEntity = NULL;
	AIFunc_FZombie_LightningAttackStart( AICast_GetCastState( ent->s.number ) );
}

/*
===============
AIFunc_FZombie_Idle
===============
*/
char *AIFunc_FZombie_Idle( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	if ( cs->thinkFuncChangeTime < level.time - PORTAL_FEMZOMBIE_SPAWNTIME ) {
		// HACK, make them aware of the player
		cs->castScriptStatus.scriptNoSightTime = 0;
		AICast_UpdateVisibility( &g_entities[cs->entityNum], AICast_FindEntityForName( "player" ), qfalse, qtrue );
		ent->s.time2 = 0;   // turn spawning effect off
		// allow us to be informed to start the portal lightning
		ent->AIScript_AlertEntity = AICast_FZombie_StartLightning;
		return AIFunc_DefaultStart( cs );
	}
	//
	return NULL;
}

/*
===============
AIFunc_FZombie_IdleStart
===============
*/
char *AIFunc_FZombie_IdleStart( cast_state_t *cs ) {
	cs->aifunc = AIFunc_FZombie_Idle;
	return "AIFunc_FZombie_Idle";
}

/*
===============
AICast_FZombie_EndLightning
===============
*/
void AICast_FZombie_EndLightning( gentity_t *ent ) {
	ent->s.effect2Time = level.time;
	// allow us to be informed to start the portal lightning
	ent->AIScript_AlertEntity = AICast_FZombie_StartLightning;
}

/*
===============
AIFunc_FZombie_LightningAttack

  The big portal lightning effect. While this is going on, the FemZombie will climb
  across the walls like an insect.
===============
*/
char *AIFunc_FZombie_LightningAttack( cast_state_t *cs ) {
	bot_state_t *bs;
	gentity_t *ent, *marker, *trav;
	// TTimo gcc: 'best' might be used uninitialized in this function
	gentity_t *best = NULL;
	qboolean move;
	float bestdist, dist;
	vec3_t axis[3];
	cast_state_t    *ecs;
	//
	ent = &g_entities[cs->entityNum];
	bs = cs->bs;
	trav = AICast_FindEntityForName( "player" );
	if ( !trav ) {
		return NULL;        // huh?
	}
	cs->bs->enemy = trav->s.number;
	ecs = AICast_GetCastState( cs->bs->enemy );
	//
	// we should show a big lightning effect and then die once we've given the player enough time to get to us
	//
	ent->s.effect1Time = cs->thinkFuncChangeTime;
	ent->client->ps.eFlags |= EF_MONSTER_EFFECT;
	//
	// TODO: might be cool to have the head move around a bit faster (like an insect?)
	g_entities[cs->entityNum].client->ps.eFlags |= EF_HEADLOOK;
	//
	// turn on flight only if we need it this frame
	ent->client->ps.powerups[PW_FLIGHT] = 0;
	//
	// has this effect finished?
	if ( ent->s.effect2Time ) {

		// we are in climb-down mode as lightning subsides
		if ( ent->client->ps.groundEntityNum == ENTITYNUM_WORLD ) {

			return AIFunc_DefaultStart( cs );

		} else {    // climb down

			if ( cs->followEntity == -1 ) {   // find the closest marker and go there

				// assume we are on the upper wall, and have just reached a marker
				trav = NULL;
				// TTimo assignment used as truth value
				while ( ( trav = G_Find( trav, FOFS( classname ), "ai_marker" ) ) ) {
					if ( trav->count == ent->count && trav->targetname ) {
						cs->followEntity = trav->s.number;
						return AIFunc_FZombie_LightningAttack( cs );  // think again
					}
				}
				if ( !trav ) {
					G_Error( "AIFunc_FZombie_LightningAttack: unable to find matching wall marker for count = %i", ent->count );
				}

			} else if ( cs->followEntity == -2 ) {    // at the base of wall, rotate around then dismount

				ent->client->ps.eFlags |= EF_FORCED_ANGLES; // face angles exactly
				cs->bs->ideal_viewangles[ROLL] = 0;

				if ( fabs( cs->bs->viewangles[ROLL] ) < 5 ) { // we are done, dismount and get outta here
					ent->client->ps.powerups[PW_FLIGHT] = 0;
					return AIFunc_DefaultStart( cs );
				}

				ent->client->ps.powerups[PW_FLIGHT] = 1;    // stay here
				return NULL;
			}

		}

	}

	if ( cs->followEntity >= 0 ) {    // move towards our current goal

		marker = &g_entities[cs->followEntity];

		ent->count = marker->count;

		if ( !ent->s.effect2Time && marker->targetname ) {        // ground marker

			dist = VectorDistance( cs->bs->origin, marker->s.origin );
			if ( dist < 4 ) {
				// we made it there, find the corresponding wall marker
				trav = NULL;
				// TTimo gcc: suggest parentheses around assignment used as truth value
				while ( ( trav = G_Find( trav, FOFS( classname ), "ai_marker" ) ) ) {
					if ( trav->count == marker->count && !trav->targetname ) {
						cs->followEntity = trav->s.number;
						return AIFunc_FZombie_LightningAttack( cs );  // think again
					}
				}
				if ( !trav ) {
					G_Error( "AIFunc_FZombie_LightningAttack: unable to find matching wall marker for count = %i", marker->count );
				}
			}

			cs->followSlowApproach = qtrue;
			AICast_MoveToPos( cs, marker->s.origin, cs->followEntity );

			// should we slow down?
			cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, 8 );

			// check for a movement we should be making
			if ( cs->obstructingTime > level.time ) {
				AICast_MoveToPos( cs, cs->obstructingPos, -1 );
				cs->movestate = MS_WALK;
				cs->movestateType = MSTYPE_TEMPORARY;
			}

		} else {                        // in-air

			if ( !ent->s.effect2Time && ( ecs->aiFlags & AIFL_ROLL_ANIM ) ) {
				// hurt player if they are visible from portal
				ent->client->ps.eFlags |= EF_MONSTER_EFFECT2;
			}

			dist = VectorDistance( cs->bs->origin, marker->s.origin );
			if ( dist < 20 ) {
				// we made it there, stop
				if ( ent->s.effect2Time && marker->targetname ) {
					cs->followEntity = -2;
				} else {
					cs->followEntity = -1;
				}
				return NULL;
			}

			// climb the walls

			ent->client->ps.powerups[PW_FLIGHT] = 1;    // let them fly
			ent->client->ps.eFlags |= EF_FORCED_ANGLES; // face angles exactly

			if ( ( ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != BOTH_CLIMB ) {
				ent->client->ps.torsoAnim =
					( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_CLIMB;
				ent->client->ps.legsAnim =
					( ( ent->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_CLIMB;
			}
			ent->client->ps.torsoTimer = 500;
			ent->client->ps.legsTimer = 500;

			// should we slow down?
			cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, 32 );

			// use angles of the marker, but ROLL so we point upwards towards the marker
			// fwd is the marker angles, up is the vec, right is the cross product
			AngleVectors( marker->s.angles, axis[0], NULL, NULL );
			VectorSubtract( marker->s.origin, cs->bs->origin, axis[2] );
			VectorNormalize( axis[2] );
			//if (ent->s.effect2Time && marker->targetname) {	// walk downwards
			//VectorInverse( axis[2] );
			//	VectorSet( axis[2], 0, 0, 1 );
			//}
			CrossProduct( axis[0], axis[2], axis[1] );
			VectorInverse( axis[1] );
			AxisToAngles( axis, cs->bs->ideal_viewangles );

			// movement
			//if (ent->s.effect2Time && marker->targetname) {	// walk downwards
			//	trap_EA_MoveDown(cs->entityNum);
			//} else {
			trap_EA_Jump( cs->entityNum );
			//}
			trap_EA_Move( cs->entityNum, axis[0], 20 );  // move towards the wall so we stay attached to it
		}

	} else {    // we are motionless on the wall.. keep checking to see if we should head somewhere else

		if ( !ent->s.effect2Time && ( ecs->aiFlags & AIFL_ROLL_ANIM ) ) {
			// hurt player if they are visible from portal
			ent->client->ps.eFlags |= EF_MONSTER_EFFECT2;
		}

		ent->client->ps.powerups[PW_FLIGHT] = 1;    // let them fly
		ent->client->ps.eFlags |= EF_FORCED_ANGLES; // face angles exactly

		move = qfalse;

		if ( cs->lastPain >= cs->lastThink ) {    // we've been injured, move
			cs->lastPain = 0;
			move = qtrue;
		} else if ( AICast_VisibleFromPos( g_entities[cs->bs->enemy].client->ps.origin, cs->bs->enemy, cs->bs->origin, cs->entityNum, qfalse ) ) {
			move = qtrue;
		}

		if ( move ) { // we need to start moving again
			trav = NULL;
			bestdist = -1;
			while ( ( trav = G_Find( trav, FOFS( classname ), "ai_marker" ) ) ) {
				if ( trav->targetname ) { // floor marker
					continue;
				}
				if ( VectorDistance( cs->bs->origin, trav->s.origin ) < 48 ) {
					continue;
				}
				if ( ( trav->count < 10 ) == ( ent->count < 10 ) ) {    // this marker is on our wall
					// if this marker no visible from the enemy
					if ( !AICast_VisibleFromPos( g_entities[cs->bs->enemy].client->ps.origin, cs->bs->enemy, trav->s.origin, cs->entityNum, qfalse ) ) {
						cs->followEntity = trav->s.number;
						break;
					}
					dist = VectorDistance( trav->s.origin, g_entities[cs->bs->enemy].client->ps.origin );
					if ( bestdist < 0 || bestdist < dist ) {
						best = trav;
						bestdist = dist;
					}
				}
			}
			if ( !trav ) {
				if ( !best ) {
					G_Error( "AIFunc_FZombie_LightningAttack: unable to find matching wall marker for count = %i", ent->count );
				}
				cs->followEntity = best->s.number;
			}
			//
			return AIFunc_FZombie_LightningAttack( cs );  // think again
		}

	}
	//
	return NULL;
}

/*
===============
AIFunc_FZombie_LightningAttackStart
===============
*/
char *AIFunc_FZombie_LightningAttackStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum], *marker, *best;
	float bestdist, dist;
	//
	ent->AIScript_AlertEntity = AICast_FZombie_EndLightning;    // scripting will tell us to stop
	ent->s.effect2Time = 0;
	//
	// find the closest ai_marker on the ground
	marker = NULL;
	best = NULL;
	bestdist = -1;
	// TTimo gcc: suggest parentheses around assignment used as truth value
	while ( ( marker = G_Find( marker, FOFS( classname ), "ai_marker" ) ) ) {
		if ( !marker->targetname || ( Q_stricmp( marker->targetname, "zfloor" ) != 0 ) ) {
			continue;
		}
		dist = VectorDistance( marker->s.origin, cs->bs->origin );
		if ( bestdist >= 0 && ( bestdist < dist ) ) {
			continue;
		}
		// closer
		best = marker;
		bestdist = dist;
	}
	//
	if ( !best ) {
		G_Error( "AIFunc_FZombie_LightningAttackStart: unable to find a close ai_marker with targetname = \"zfloor\"" );
	}
	cs->followEntity = best->s.number;
	//
	cs->aifunc = AIFunc_FZombie_LightningAttack;
	return "AIFunc_FZombie_LightningAttack";
}

/*
===============
AIFunc_FZombie_HandLightningAttack

  Shoots lightning out at the player from her hands
===============
*/
#define FEMZOMBIE_HANDATTACK_DURATION   3400

char *AIFunc_FZombie_HandLightningAttack( cast_state_t *cs ) {
	bot_state_t *bs;
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	bs = cs->bs;
	cs->weaponFireTimes[WP_MONSTER_ATTACK2] = level.time;
	//
	if ( cs->aiFlags & AIFL_LAND_ANIM_PLAYED ) {  // stop the effects
		if ( !ent->client->ps.torsoTimer ) {
			// are we ready to do the big portal lightning effect?
			if ( AICast_GotEnoughAmmoForWeapon( cs, WP_MONSTER_ATTACK1 ) ) {
				return AIFunc_FZombie_LightningAttackStart( cs );
			} else {
				return AIFunc_BattleChaseStart( cs );
			}
		}
		return NULL;
	}
	//
	// face them and do the effect
	AICast_AimAtEnemy( cs );
	if ( ent->client->ps.torsoTimer < FEMZOMBIE_HANDATTACK_DURATION - 1000 ) {
		ent->client->ps.eFlags |= EF_MONSTER_EFFECT3;
		ent->s.otherEntityNum = bs->enemy;
		//
		if ( ent->client->ps.torsoTimer < 400 || cs->bs->cur_ps.ammo[BG_FindAmmoForWeapon( WP_MONSTER_ATTACK1 )] || !AICast_EntityVisible( cs, bs->enemy, qtrue ) || !AICast_CheckAttack( cs, bs->enemy, qfalse ) ) {
			// finish this attack
			ent->client->ps.torsoAnim =
				( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_ATTACK5;
			ent->client->ps.torsoTimer = 300;
			cs->aiFlags |= AIFL_LAND_ANIM_PLAYED;
		}
	}
	//
	return NULL;
}

/*
===============
AIFunc_FZombie_HandLightningAttackStart
===============
*/
char *AIFunc_FZombie_HandLightningAttackStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	ent->client->ps.torsoAnim =
		( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_ATTACK4;
	ent->client->ps.torsoTimer = FEMZOMBIE_HANDATTACK_DURATION;
	//
	cs->aiFlags &= ~AIFL_LAND_ANIM_PLAYED;
	ent->s.effect3Time = level.time;
	cs->aifunc = AIFunc_FZombie_HandLightningAttack;
	return "AIFunc_FZombie_handLightningAttack";
}

//=================================================================================
//
// Helga (in normal form), the first boss
//
//=================================================================================

/*
===============
AICast_Helga_Alert

  Special code hooks for helga scripting
===============
*/
void AICast_Helga_Alert( gentity_t *ent ) {
	cast_state_t *cs = AICast_GetCastState( ent->s.number );

	if ( !ent->s.effect2Time ) {
		ent->s.eFlags |= EF_MONSTER_EFFECT2;
		ent->s.effect2Time = level.time;
	} else if ( !( cs->aiFlags & AIFL_LAND_ANIM_PLAYED ) ) {
		cs->aiFlags |= AIFL_LAND_ANIM_PLAYED;   // stop the effects
	} else {
		// we are no longer in the game
		ent->aiInactive = qtrue;
		trap_UnlinkEntity( ent );
	}
}

/*
===============
AIFunc_Helga_Idle
===============
*/
char *AIFunc_Helga_Idle( cast_state_t *cs ) {
	bot_state_t *bs;
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	bs = cs->bs;
	//
	if ( cs->aiFlags & AIFL_LAND_ANIM_PLAYED ) {
		return NULL;
	}
	// we should show a big lightning effect and then die once we've given the player enough time to get to us
	//
	ent->s.effect1Time = cs->thinkFuncChangeTime;
	ent->client->ps.eFlags |= EF_MONSTER_EFFECT;
	//
	// are we in lightning death mode?
	if ( ent->s.effect2Time && !( cs->aiFlags & AIFL_LAND_ANIM_PLAYED ) ) {
		ent->client->ps.eFlags |= EF_MONSTER_EFFECT2;
	}
	//
	return NULL;
}

/*
===============
AIFunc_Helga_IdleStart
===============
*/
char *AIFunc_Helga_IdleStart( cast_state_t *cs ) {
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	// special alertEntity function so scripting can initialize the death routine
	ent->AIScript_AlertEntity = AICast_Helga_Alert;
	ent->s.effect2Time = 0;
	//
	cs->aiFlags &= ~AIFL_LAND_ANIM_PLAYED;
	//
	cs->aifunc = AIFunc_Helga_Idle;
	return "AIFunc_Helga_Idle";
}

//===================================================================

/*
==============
AIFunc_FlameZombie_Portal
==============
*/
char *AIFunc_FlameZombie_Portal( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	if ( cs->thinkFuncChangeTime < level.time - PORTAL_ZOMBIE_SPAWNTIME ) {
		// HACK, make them aware of the player
		AICast_UpdateVisibility( &g_entities[cs->entityNum], AICast_FindEntityForName( "player" ), qfalse, qtrue );
		ent->s.time2 = 0;   // turn spawning effect off
		return AIFunc_DefaultStart( cs );
	}
	//
	return NULL;
}

/*
==============
AIFunc_FlameZombie_PortalStart
==============
*/
char *AIFunc_FlameZombie_PortalStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	ent->s.time2 = level.time + 200;    // hijacking this for portal spawning effect
	//
	// play a special animation
	ent->client->ps.torsoAnim =
		( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_EXTRA1;
	ent->client->ps.legsAnim =
		( ( ent->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_EXTRA1;
	ent->client->ps.torsoTimer = PORTAL_ZOMBIE_SPAWNTIME - 200;
	ent->client->ps.legsTimer = PORTAL_ZOMBIE_SPAWNTIME - 200;
	//
	cs->thinkFuncChangeTime = level.time;
	//
	cs->aifunc = AIFunc_FlameZombie_Portal;
	return "AIFunc_FlameZombie_Portal";
}
#endif RTCW_XX
