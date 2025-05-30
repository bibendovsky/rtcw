/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "g_local.h"


/*
==============
alarmExplosion
	copied from propExplosion
==============
*/
void alarmExplosion( gentity_t *ent ) {
	gentity_t *bolt;

	extern void G_ExplodeMissile( gentity_t * ent );
	bolt = G_Spawn();
	bolt->classname = const_cast<char*>("props_explosion");
	bolt->nextthink = level.time + FRAMETIME;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;

	bolt->s.weapon = WP_NONE;

	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = ent->s.number;
	bolt->parent = ent;
	bolt->damage = ent->health;
	bolt->splashDamage = ent->health;
	bolt->splashRadius = ent->health * 1.5;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask = MASK_SHOT;

	VectorCopy( ent->r.currentOrigin, bolt->s.pos.trBase );
	VectorCopy( ent->r.currentOrigin, bolt->r.currentOrigin );
}


/*
==============
alarmbox_updateparts
==============
*/
void alarmbox_updateparts( gentity_t *ent, qboolean matestoo ) {
	gentity_t   *t, *mate;
	qboolean alarming = ( ent->s.frame == 1 );

	// update teammates
	if ( matestoo ) {
		for ( mate = ent->teammaster; mate; mate = mate->teamchain )
		{
			if ( mate == ent ) {
				continue;
			}

			if ( !( mate->active ) ) { // don't update dead alarm boxes, they stay dead
				continue;
			}

			if ( !( ent->active ) ) { // destroyed, so just turn teammates off
				mate->s.frame = 0;
			} else {
				mate->s.frame = ent->s.frame;
			}

			alarmbox_updateparts( mate, qfalse );
		}
	}

	// update lights
	if ( !ent->target ) {
		return;
	}

	t = NULL;
	while ( ( t = G_Find( t, FOFS( targetname ), ent->target ) ) != NULL )
	{
		if ( t == ent ) {
			G_Printf( "WARNING: Entity used itself.\n" );
		} else
		{
			// give the dlight the sound
			if ( !Q_stricmp( t->classname, "dlight" ) ) {
				t->soundLoop = ent->soundLoop;

				if ( alarming ) {
					if ( !( t->r.linked ) ) {
						t->use( t, ent, 0 );
					}
				} else
				{
					if ( t->r.linked ) {
						t->use( t, ent, 0 );
					}
				}
			}
			// alarmbox can tell script_trigger about activation
			// (but don't trigger if dying, only activation)
			else if ( !Q_stricmp( t->classname, "target_script_trigger" ) ) {
				if ( ent->active ) { // not dead
					t->use( t, ent, 0 );
				}
			}
		}
	}
}

/*
==============
alarmbox_use
==============
*/
void alarmbox_use( gentity_t *ent, gentity_t *other, gentity_t *foo ) {
	if ( !( ent->active ) ) {
		return;
	}

	if ( ent->s.frame ) {
		ent->s.frame = 0;
	} else {
		ent->s.frame = 1;
	}

	alarmbox_updateparts( ent, qtrue );
	if ( other->client ) {
		G_AddEvent( ent, EV_GENERAL_SOUND, ent->soundPos3 );
	}
//	G_Printf("touched alarmbox\n");

}


/*
==============
alarmbox_die
==============
*/
void alarmbox_die( gentity_t *ent, gentity_t *inflictor, gentity_t *attacker, int damage, int mod ) {
	alarmExplosion( ent );
	ent->s.frame    = 2;
	ent->active     = qfalse;
	ent->takedamage = qfalse;
	alarmbox_updateparts( ent, qtrue );
}




/*
==============
alarmbox_finishspawning
==============
*/
void alarmbox_finishspawning( gentity_t *ent ) {
	gentity_t *mate;

	// make sure they all have the same master (picked arbitrarily.  last spawned)
	for ( mate = ent; mate; mate = mate->teamchain )
		mate->teammaster = ent->teammaster;

	// find lights and set their state
	alarmbox_updateparts( ent, qtrue );
}


/*QUAKED alarm_box (1 0 1) START_ON
You need to have an origin brush as part of this entity
current alarm box model is (8 x 16 x 28)
"health" defaults to 10

"noise" the sound to play over the system (this would be the siren sound)

START_ON means the button is pushed in, any dlights are cycling, and alarms are sounding

"team" key/value is valid for teamed alarm boxes
teamed alarm_boxes work in tandem (switches/lights syncronize)
target a box to dlights to have them activate/deactivate with the system (use a stylestring that matches the cycletime for the alarmbox sound)
alarm sound locations are also placed in the dlights, so wherever you place an attached dlight, you will hear the alarm
model: the model used is "models/mapobjects/electronics/alarmbox.md3"
place the origin at the center of your trigger box
*/
void SP_alarm_box( gentity_t *ent ) {
	char *s;

	if ( !ent->model ) {
		G_Printf( S_COLOR_RED "alarm_box with NULL model\n" );
		return;
	}

	// model
	trap_SetBrushModel( ent, ent->model );
	ent->s.modelindex2 = G_ModelIndex( "models/mapobjects/electronics/alarmbox.md3" );

	// sound
	if ( G_SpawnString( "noise", "0", &s ) ) {
		ent->soundLoop = G_SoundIndex( s );
	}

	ent->soundPos3 = G_SoundIndex( "sound/world/alarmswitch.wav" );


	G_SetOrigin( ent, ent->s.origin );
	G_SetAngle( ent, ent->s.angles );

	if ( !ent->health ) {
		ent->health = 10;
	}

	if ( ent->spawnflags & 1 ) {
		ent->s.frame = 1;
	} else {
		ent->s.frame = 0;
	}

	ent->active     = qtrue;
	ent->s.eType    = ET_ALARMBOX;
	ent->takedamage = qtrue;
	ent->die        = alarmbox_die;
	ent->use        = alarmbox_use;
	ent->think      = alarmbox_finishspawning;
	ent->nextthink  = level.time + FRAMETIME;

	trap_LinkEntity( ent );
}


