/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

//===========================================================================
//
// Name:			ai_cast_script_ents.c
// Function:		Wolfenstein AI Character Scripting
// Programmer:		Ridah
// Tab Size:		4 (real tabs)
//===========================================================================


#include "g_local.h"
#include "q_shared.h"
#include "botlib.h"      //bot lib interface
#include "be_aas.h"
#include "be_ea.h"
#include "be_ai_gen.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"
#include "botai.h"          //bot ai interface

#include "ai_cast.h"

/*QUAKED ai_marker (1 0.5 0) (-18 -18 -24) (18 18 48) NODROP
AI marker

NODROP means dont drop it to the ground

"targetname" : identifier for this marker
*/

/*
============
SP_ai_marker
============
*/
extern vec3_t playerMins, playerMaxs;
void SP_ai_marker( gentity_t *ent ) {
	vec3_t dest;
	trace_t tr;
	vec3_t checkMins, checkMaxs;

	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		G_FreeEntity( ent );
		return;
	}

//----(SA)	move the bounding box for the check in 1 unit on each side so they can butt up against a wall and not startsolid
	VectorCopy( playerMins, checkMins );
	checkMins[0] += 1;
	checkMins[1] += 1;
	VectorCopy( playerMaxs, checkMaxs );
	checkMaxs[0] -= 1;
	checkMaxs[1] -= 1;
//----(SA)	end

	if ( !( ent->spawnflags & 1 ) ) {
		// drop to floor
		ent->r.currentOrigin[2] += 1.0; // fixes QErad -> engine bug?
		VectorSet( dest, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2] - 4096 );
		trap_Trace( &tr, ent->r.currentOrigin, checkMins, checkMaxs, dest, ent->s.number, MASK_PLAYERSOLID | CONTENTS_MONSTERCLIP );

		if ( tr.startsolid ) {
			G_Printf( "WARNING: ai_marker (%s) in solid at %s\n", ent->targetname, vtos( ent->r.currentOrigin ) );
			return;
		}

		G_SetOrigin( ent, tr.endpos );
	}
}

/*QUAKED ai_effect (0.3 0.8 0.2) (-4 -4 -4) (4 4 4)
AI effect entity

"ainame" is the name of the AI character that uses this entity for effects
*/

/*
============
SP_ai_effect
============
*/
void ai_effect_think( gentity_t *ent ) {
	gentity_t *targ;

	// find the client number that uses this entity
	targ = AICast_FindEntityForName( ent->aiName );
	if ( !targ ) {
		// keep waiting until they enter, if they never do, then we have no purpose, therefore no harm can be done
		ent->think = ai_effect_think;
		ent->nextthink = level.time + 200;
		return;
		//G_Error( "ai_effect with invalid aiName at %s\n", vtos(ent->s.origin) );
	}

	// make sure the clients can use this association
	ent->s.otherEntityNum = targ->s.number;

	ent->s.eType = ET_AI_EFFECT;
	G_SetOrigin( ent, ent->s.origin );
	trap_LinkEntity( ent );
	ent->r.svFlags |= SVF_BROADCAST;    // make sure all clients are aware of this entity
}

void SP_ai_effect( gentity_t *ent ) {
	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		G_FreeEntity( ent );
		return;
	}

	ent->think = ai_effect_think;
	ent->nextthink = level.time + 500;
}

//===========================================================

// the wait time has passed, so set back up for another activation
void AICast_trigger_wait( gentity_t *ent ) {
	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void AICast_trigger_trigger( gentity_t *ent, gentity_t *activator ) {
	if ( ent->nextthink ) {
		return;     // can't retrigger until the wait is over
	}

	ent->activator = AICast_FindEntityForName( ent->aiName );
	if ( ent->activator ) { // they might be dead
		// trigger the script event
		AICast_ScriptEvent( AICast_GetCastState( ent->activator->s.number ), "trigger", ent->target );
	}

	if ( ent->wait > 0 ) {
		ent->think = AICast_trigger_wait;
		ent->nextthink = level.time + ( ent->wait + ent->random * crandom() ) * 1000;
	} else {
		// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = 0;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEntity;
	}
}

void AICast_Touch_Trigger( gentity_t *self, gentity_t *other, trace_t *trace ) {
	if ( !other->client || ( other->r.svFlags & SVF_CASTAI ) ) {
		return;
	}
	AICast_trigger_trigger( self, other );
}

/*QUAKED ai_trigger (1 0.5 0) ? Startoff
Triggered only by the player touching it
"wait" : Seconds between triggerings, -1 = one time only (default).
"ainame" : name of AI to target (use "player" for the.. player)
"target" : trigger identifier for that AI script
*/
extern void InitTrigger( gentity_t *self );

void ai_trigger_activate( gentity_t *self ) {
	if ( self->r.linked ) {
		return;
	}

	self->use = NULL;
	self->AIScript_AlertEntity = NULL;

	self->touch = AICast_Touch_Trigger;

	InitTrigger( self );
	trap_LinkEntity( self );
}

void ai_trigger_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	ai_trigger_activate( self );
}

void SP_ai_trigger( gentity_t *ent ) {
	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		G_FreeEntity( ent );
		return;
	}

	G_SpawnFloat( "wait", "-1", &ent->wait );

	if ( !ent->aiName ) {
		G_Error( "ai_trigger without \"ainame\"\n" );
	}
	if ( !ent->target ) {
		G_Error( "ai_trigger without \"target\"\n" );
	}

	if ( ent->spawnflags & 1 ) { // TriggerSpawn
		ent->AIScript_AlertEntity = ai_trigger_activate;
		ent->use = ai_trigger_use;
		trap_UnlinkEntity( ent );
	} else {
		ai_trigger_activate( ent );
	}
}
