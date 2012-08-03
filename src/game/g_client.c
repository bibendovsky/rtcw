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

#include "g_local.h"

// g_client.c -- client functions that don't happen every frame

// Ridah, new bounding box
//static vec3_t	playerMins = {-15, -15, -24};
//static vec3_t	playerMaxs = {15, 15, 32};
vec3_t playerMins = {-18, -18, -24};
vec3_t playerMaxs = {18, 18, 48};
// done.

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
If the start position is targeting an entity, the players camera will start out facing that ent (like an info_notnull)
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int i;
	vec3_t dir;

	G_SpawnInt( "nobots", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}

	ent->enemy = G_PickTarget( ent->target );
	if ( ent->enemy ) {
		VectorSubtract( ent->enemy->s.origin, ent->s.origin, dir );
		vectoangles( dir, ent->s.angles );
	}

}

#if defined RTCW_MP
//----(SA) added
/*QUAKED info_player_checkpoint (1 0 0) (-16 -16 -24) (16 16 32) a b c d
these are start points /after/ the level start
the letter (a b c d) designates the checkpoint that needs to be complete in order to use this start position
*/
void SP_info_player_checkpoint( gentity_t *ent ) {
	ent->classname = "info_player_checkpoint";
	SP_info_player_deathmatch( ent );
}

//----(SA) end
#endif RTCW_XX


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start( gentity_t *ent ) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

#if defined RTCW_SP
/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
#elif defined RTCW_MP
/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32) AXIS ALLIED
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
#endif RTCW_XX

void SP_info_player_intermission( gentity_t *ent ) {

}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int i, num;
	int touch[MAX_GENTITIES];
	gentity_t   *hit;
	vec3_t mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for ( i = 0 ; i < num ; i++ ) {
		hit = &g_entities[touch[i]];
		if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
			return qtrue;
		}

	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define MAX_SPAWN_POINTS    128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
	gentity_t   *spot;
	vec3_t delta;
	float dist, nearestDist;
	gentity_t   *nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ( ( spot = G_Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL ) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define MAX_SPAWN_POINTS    128
gentity_t *SelectRandomDeathmatchSpawnPoint( void ) {
	gentity_t   *spot;
	int count;
	int selection;
	gentity_t   *spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while ( ( spot = G_Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL ) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}
		spots[ count ] = spot;
		count++;
	}

	if ( !count ) { // no spots that won't telefrag
		return G_Find( NULL, FOFS( classname ), "info_player_deathmatch" );
	}

	selection = rand() % count;
	return spots[ selection ];
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
	gentity_t   *spot;
	gentity_t   *nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint();
	if ( spot == nearestSpot ) {
		// roll again if it would be real close to point of death
		spot = SelectRandomDeathmatchSpawnPoint();
		if ( spot == nearestSpot ) {
			// last try
			spot = SelectRandomDeathmatchSpawnPoint();
		}
	}

	// find a single player start spot
	if ( !spot ) {
		G_Error( "Couldn't find a spawn point" );
	}

	VectorCopy( spot->s.origin, origin );
	origin[2] += 9;
	VectorCopy( spot->s.angles, angles );

	return spot;
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles ) {
	gentity_t   *spot;

	spot = NULL;
	while ( ( spot = G_Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL ) {
		if ( spot->spawnflags & 1 ) {
			break;
		}
	}

	if ( !spot || SpotWouldTelefrag( spot ) ) {
		return SelectSpawnPoint( vec3_origin, origin, angles );
	}

	VectorCopy( spot->s.origin, origin );
	origin[2] += 9;
	VectorCopy( spot->s.angles, angles );

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue( void ) {
	int i;
	gentity_t   *ent;

	level.bodyQueIndex = 0;
	for ( i = 0; i < BODY_QUEUE_SIZE ; i++ ) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	if ( level.time - ent->timestamp > 6500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		return;
	}
	ent->nextthink = level.time + 100;
	ent->s.pos.trBase[2] -= 1;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {
	gentity_t       *body;
	int contents, i;

	trap_UnlinkEntity( ent );

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->s.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = ( level.bodyQueIndex + 1 ) % BODY_QUEUE_SIZE;

	trap_UnlinkEntity( body );

	body->s = ent->s;
	body->s.eFlags = EF_DEAD;       // clear EF_TALK, etc

	if ( ent->client->ps.eFlags & EF_HEADSHOT ) {
		body->s.eFlags |= EF_HEADSHOT;          // make sure the dead body draws no head (if killed that way)

	}

#if defined RTCW_MP
	body->s.eType = ET_CORPSE;
	body->classname = "corpse";
#endif RTCW_XX

	body->s.powerups = 0;   // clear powerups
	body->s.loopSound = 0;  // clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;        // don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	// DHM - Clear out event system
	for ( i = 0; i < MAX_EVENTS; i++ )
		body->s.events[i] = 0;
	body->s.eventSequence = 0;

	// DHM - Nerve
	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		// change the animation to the last-frame only, so the sequence
		// doesn't repeat anew for the body
		switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
		case BOTH_DEATH1:
		case BOTH_DEAD1:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
			break;
		case BOTH_DEATH2:
		case BOTH_DEAD2:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
			break;
		case BOTH_DEATH3:
		case BOTH_DEAD3:
		default:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
			break;
		}
	}
	// dhm

	body->r.svFlags = ent->r.svFlags;
	VectorCopy( ent->r.mins, body->r.mins );
	VectorCopy( ent->r.maxs, body->r.maxs );
	VectorCopy( ent->r.absmin, body->r.absmin );
	VectorCopy( ent->r.absmax, body->r.absmax );

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;

#if defined RTCW_SP
	body->r.contents = CONTENTS_CORPSE;
#elif defined RTCW_MP
	// DHM - Nerve :: allow bullets to pass through bbox
	body->r.contents = 0;
#endif RTCW_XX

	body->r.ownerNum = ent->r.ownerNum;

	body->nextthink = level.time + 5000;
	body->think = BodySink;

	body->die = body_die;

	// don't take more damage if already gibbed
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}


	VectorCopy( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity( body );
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int i;

	// set the delta angle
	for ( i = 0 ; i < 3 ; i++ ) {
		int cmdAngle;

		cmdAngle = ANGLE2SHORT( angle[i] );
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy( ent->s.angles, ent->client->ps.viewangles );
}

/* JPW NERVE
================
limbo
================
*/

#if defined RTCW_SP
void limbo( gentity_t *ent ) {
#elif defined RTCW_MP
void limbo( gentity_t *ent, qboolean makeCorpse ) {
#endif RTCW_XX

	int i,contents;
	//int startclient = ent->client->sess.spectatorClient;
	int startclient = ent->client->ps.clientNum;

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		G_Printf( "FIXME: limbo called from single player game.  Shouldn't see this\n" );
		return;
	}
	if ( !( ent->client->ps.pm_flags & PMF_LIMBO ) ) {

		// DHM - Nerve :: First save off persistant info we'll need for respawn
		for ( i = 0; i < MAX_PERSISTANT; i++ )
			ent->client->saved_persistant[i] = ent->client->ps.persistant[i];
		// dhm

		ent->client->ps.pm_flags |= PMF_LIMBO;
		ent->client->ps.pm_flags |= PMF_FOLLOW;

#if defined RTCW_SP
		CopyToBodyQue( ent ); // make a nice looking corpse
#elif defined RTCW_MP
		if ( makeCorpse ) {
			CopyToBodyQue( ent ); // make a nice looking corpse
		} else {
			trap_UnlinkEntity( ent );
		}

		// DHM - Nerve :: reset these values
		ent->client->ps.viewlocked = 0;
		ent->client->ps.viewlocked_entNum = 0;
#endif RTCW_XX

		ent->r.maxs[2] = 0;
		ent->r.currentOrigin[2] += 8;
		contents = trap_PointContents( ent->r.currentOrigin, -1 ); // drop stuff
		ent->s.weapon = ent->client->limboDropWeapon; // stored in player_die()

#if defined RTCW_SP
		if ( !( contents & CONTENTS_NODROP ) ) {
#elif defined RTCW_MP
		if ( makeCorpse && !( contents & CONTENTS_NODROP ) ) {
#endif RTCW_XX

			TossClientItems( ent );
		}

		ent->client->sess.spectatorClient = startclient;
		Cmd_FollowCycle_f( ent,1 ); // get fresh spectatorClient

		if ( ent->client->sess.spectatorClient == startclient ) {
			// No one to follow, so just stay put
			ent->client->sess.spectatorState = SPECTATOR_FREE;
		} else {
			ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		}

#if defined RTCW_SP
		ClientUserinfoChanged( ent->client - level.clients );
#elif defined RTCW_MP
//		ClientUserinfoChanged( ent->client - level.clients );		// NERVE - SMF - don't do this
#endif RTCW_XX

		if ( ent->client->sess.sessionTeam == TEAM_RED ) {
			ent->client->deployQueueNumber = level.redNumWaiting;
			level.redNumWaiting++;
		} else if ( ent->client->sess.sessionTeam == TEAM_BLUE )     {
			ent->client->deployQueueNumber = level.blueNumWaiting;
			level.blueNumWaiting++;
		}

#if defined RTCW_SP
//	ClientBegin( ent->client - level.clients );
#elif defined RTCW_MP
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if ( level.clients[i].ps.pm_flags & PMF_LIMBO
				 && level.clients[i].sess.spectatorClient == ent->s.number ) {
				Cmd_FollowCycle_f( &g_entities[i], 1 );
			}
		}
#endif RTCW_XX

	}
}

/* JPW NERVE
================
reinforce
================
// -- called when time expires for a team deployment cycle and there is at least one guy ready to go
*/
void reinforce( gentity_t *ent ) {

#if defined RTCW_SP
	int i, j, p, team, deployable[256], numDeployable = 0, finished = 0;
	char *classname = NULL;
	gentity_t *spot;
#elif defined RTCW_MP
	int p, team; // numDeployable=0, finished=0; // TTimo unused
	char *classname;
#endif RTCW_XX

	gclient_t *rclient;

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		G_Printf( "FIXME: reinforce called from single player game.  Shouldn't see this\n" );
		return;
	}
	if ( !( ent->client->ps.pm_flags & PMF_LIMBO ) ) {
		G_Printf( "player already deployed, skipping\n" );
		return;
	}
	// get team to deploy from passed entity

#if defined RTCW_SP
	for ( i = 0; i < level.maxclients; i++ )
		deployable[i] = -1;
#endif RTCW_XX

	team = ent->client->sess.sessionTeam;

#if defined RTCW_SP
	// build initial list
	for ( i = 0; i < level.maxclients; i++ ) {
		// skip if not connected
		if ( level.clients[i].pers.connected != CON_CONNECTED ) {
			continue;
		}
		// skip if not in limbo
		if ( !( level.clients[i].ps.pm_flags & PMF_LIMBO ) ) {
			continue;
		}
		// skip if not on same team
		if ( level.clients[i].sess.sessionTeam != team ) {
			continue;
		}
		// still here? add to list
		deployable[numDeployable] = i;
		numDeployable++;
	}
	G_Printf( "numDeployable=%d\n",numDeployable );

	// bubble sort list on time-based death order
	while ( !finished ) {
		finished = 1; // if nothing bad happens, bail out after this pass
		for ( i = 1; i < numDeployable; i++ )
			if ( level.clients[deployable[i]].deployQueueNumber < level.clients[deployable[i - 1]].deployQueueNumber ) {
				G_Printf( "swapping %d and %d (priority %d and %d)\n", deployable[i], deployable[i - 1],
						  level.clients[deployable[i]].deployQueueNumber, level.clients[deployable[i - 1]].deployQueueNumber );
				j = deployable[i - 1];
				deployable[i - 1] = deployable[i];
				deployable[i] = j;
				finished = 0; // continue bubble sorting
			}
	}

// sanity check
	G_Printf( "sorted " );
	for ( i = 0; i < numDeployable; i++ )
		G_Printf( "%d ",level.clients[deployable[i]] );
	G_Printf( "\n" );
#endif RTCW_XX

	// find number active team spawnpoints
	if ( team == TEAM_RED ) {
		classname = "team_CTF_redspawn";
	} else if ( team == TEAM_BLUE ) {
		classname = "team_CTF_bluespawn";
	} else {
		assert( 0 );
	}

#if defined RTCW_SP
	finished = 0;


	spot = NULL;

	while ( ( spot = G_Find( spot, FOFS( classname ), classname ) ) != NULL ) {

		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}
		if ( spot->spawnflags & 2 ) { // spawnpoint is active
			finished++;
		}
	}
	G_Printf( "found %d active spawnpoints\n",finished );

	if ( finished < numDeployable ) {
		j = finished;
	} else {
		j = numDeployable;
	}

	// respawn deployable people
	for ( i = 0; i < j; i++ ) {

		// DHM - Nerve :: restore persistant data now that we're out of Limbo
		rclient = g_entities[deployable[i]].client;
		for ( p = 0; p < MAX_PERSISTANT; p++ )
			rclient->ps.persistant[p] = rclient->saved_persistant[p];
		// dhm```

		respawn( &g_entities[deployable[i]] );
	}

	// re-spool non-deployable people (update time-based death order)
	// reset next sort value
	if ( team == TEAM_RED ) {
		level.redNumWaiting = 0;
	} else if ( team == TEAM_BLUE ) {
		level.blueNumWaiting = 0;
	}

	if ( finished < numDeployable ) {
		G_Printf( "more deployable are waiting in queue\n" );
		for ( i = finished,j = 0; i < numDeployable; i++,j++ )
			level.clients[deployable[i]].deployQueueNumber = j;
		// reset next sort value
		if ( team == TEAM_RED ) {
			level.redNumWaiting = j;
		} else if ( team == TEAM_BLUE ) {
			level.blueNumWaiting = j;
		}

		// sanity check
		G_Printf( "second sanity check sorted " );
		for ( i = finished; i < numDeployable; i++ )
			G_Printf( "%d ",level.clients[deployable[i]].deployQueueNumber );
		G_Printf( "\n" );

	}
#elif defined RTCW_MP
	// DHM - Nerve :: restore persistant data now that we're out of Limbo
	rclient = ent->client;
	for ( p = 0; p < MAX_PERSISTANT; p++ )
		rclient->ps.persistant[p] = rclient->saved_persistant[p];
	// dhm

	respawn( ent );
#endif RTCW_XX

}
// jpw


/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {

#if defined RTCW_SP
	gentity_t   *tent;
#elif defined RTCW_MP
	//gentity_t	*tent;
#endif RTCW_XX

	// Ridah, if single player, reload the last saved game for this player
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {

#if defined RTCW_SP
//		if (reloading || saveGamePending) {
		if ( g_reloading.integer || saveGamePending ) {
#elif defined RTCW_MP
		if ( reloading || saveGamePending ) {
#endif RTCW_XX

			return;
		}

		if ( !( ent->r.svFlags & SVF_CASTAI ) ) {
			// Fast method, just do a map_restart, and then load in the savegame
			// once everything is settled.

#if defined RTCW_SP
			trap_SetConfigstring( CS_SCREENFADE, va( "1 %i 4000", level.time + 2000 ) );
//			reloading = qtrue;
			trap_Cvar_Set( "g_reloading", "1" );

//			level.reloadDelayTime = level.time + 1500;
			level.reloadDelayTime = level.time + 6000;

			trap_SendServerCommand( -1, va( "snd_fade 0 %d", 6000 ) );  // fade sound out
#elif defined RTCW_MP
			trap_SetConfigstring( CS_SCREENFADE, va( "1 %i 500", level.time + 250 ) );
			reloading = qtrue;
			level.reloadDelayTime = level.time + 1500;
#endif RTCW_XX

			return;
		}
	}

	// done.

	ent->client->ps.pm_flags &= ~PMF_LIMBO; // JPW NERVE turns off limbo

#if defined RTCW_MP
	// DHM - Nerve :: Decrease the number of respawns left
	if ( g_maxlives.integer > 0 && ent->client->ps.persistant[PERS_RESPAWNS_LEFT] > 0 ) {
		ent->client->ps.persistant[PERS_RESPAWNS_LEFT]--;
	}

	G_DPrintf( "Respawning %s, %i lives left\n", ent->client->pers.netname, ent->client->ps.persistant[PERS_RESPAWNS_LEFT] );
#endif RTCW_XX

	// DHM - Nerve :: Already handled in 'limbo()'

#if defined RTCW_SP
	if ( g_gametype.integer != GT_WOLF ) {
#elif defined RTCW_MP
	if ( g_gametype.integer < GT_WOLF ) {
#endif RTCW_XX

		CopyToBodyQue( ent );
	}

#if defined RTCW_SP
	ClientSpawn( ent );
#elif defined RTCW_MP
	ClientSpawn( ent, qfalse );
#endif RTCW_XX

	// add a teleportation effect

#if defined RTCW_SP
	tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
	tent->s.clientNum = ent->s.clientNum;
#elif defined RTCW_MP
	// DHM - Nerve :: Add back if we decide to have a spawn effect
	// add a teleportation effect
	//tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
	//tent->s.clientNum = ent->s.clientNum;
#endif RTCW_XX

}

#if defined RTCW_MP
// NERVE - SMF - merge from team arena
/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount( int ignoreClientNum, int team ) {
	int i;
	int count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}
// -NERVE - SMF
#endif RTCW_XX

/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int i;
	int counts[TEAM_NUM_TEAMS];

	memset( counts, 0, sizeof( counts ) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == TEAM_BLUE ) {
			counts[TEAM_BLUE]++;
		} else if ( level.clients[i].sess.sessionTeam == TEAM_RED )   {
			counts[TEAM_RED]++;
		}
	}

	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
		return TEAM_RED;
	}
	return TEAM_BLUE;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ( ( p = strchr( model, '/' ) ) != NULL ) {
		*p = 0;
	}

	Q_strcat( model, MAX_QPATH, "/" );
	Q_strcat( model, MAX_QPATH, skin );
}

#if defined RTCW_MP
// NERVE - SMF
/*
===========
SetWolfUserVars
===========
*/
void SetWolfUserVars( gentity_t *ent, char *wolfinfo ) {
	gclient_t *client;
	int mask, team;

	client = ent->client;
	if ( !client ) {
		return;
	}

	// check if we have a valid snapshot
	mask = MP_TEAM_MASK;
	team = ( client->pers.cmd.mpSetup & mask ) >> MP_TEAM_OFFSET;

	if ( !team ) {
		return;
	}

	// set player class
	mask = MP_CLASS_MASK;
	client->sess.latchPlayerType = ( client->pers.cmd.mpSetup & mask ) >> MP_CLASS_OFFSET;

	// set weapon
	mask = MP_WEAPON_MASK;
	client->sess.latchPlayerWeapon = ( client->pers.cmd.mpSetup & mask ) >> MP_WEAPON_OFFSET;
}

// -NERVE - SMF
#endif RTCW_XX

// DHM - Nerve
/*
===========
SetWolfSkin

Forces a client's skin (for Wolfenstein teamplay)
===========
*/

#if defined RTCW_SP
#define MULTIPLAYER_MODEL   "multi"

#elif defined RTCW_MP
#define MULTIPLAYER_ALLIEDMODEL "multi"
#define MULTIPLAYER_AXISMODEL   "multi_axis"
#endif RTCW_XX

void SetWolfSkin( gclient_t *client, char *model ) {

	switch ( client->sess.sessionTeam ) {
	case TEAM_RED:
		Q_strcat( model, MAX_QPATH, "red" );
		break;
	case TEAM_BLUE:
		Q_strcat( model, MAX_QPATH, "blue" );
		break;
	default:
		Q_strcat( model, MAX_QPATH, "red" );
		break;
	}

	switch ( client->sess.playerType ) {
	case PC_SOLDIER:
		Q_strcat( model, MAX_QPATH, "soldier" );
		break;
	case PC_MEDIC:
		Q_strcat( model, MAX_QPATH, "medic" );
		break;
	case PC_ENGINEER:
		Q_strcat( model, MAX_QPATH, "engineer" );
		break;
	case PC_LT:
		Q_strcat( model, MAX_QPATH, "lieutenant" );
		break;
	default:
		Q_strcat( model, MAX_QPATH, "soldier" );
		break;
	}

	// DHM - A skinnum will be in the session data soon...
	switch ( client->sess.playerSkin ) {
	case 1:
		Q_strcat( model, MAX_QPATH, "1" );
		break;
	case 2:
		Q_strcat( model, MAX_QPATH, "2" );
		break;
	case 3:
		Q_strcat( model, MAX_QPATH, "3" );
		break;
	default:
		Q_strcat( model, MAX_QPATH, "1" );
		break;
	}
}

void SetWolfSpawnWeapons( gclient_t *client ) {

	int pc = client->sess.playerType;
	int starthealth = 100,i,numMedics = 0;   // JPW NERVE

	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

#if defined RTCW_SP
	client->ps.powerups[PW_INVULNERABLE] = level.time + 5000; // JPW NERVE some time to find cover
#elif defined RTCW_MP
	// Reset special weapon time
	client->ps.classWeaponTime = -999999;

// Xian -- Commented out and moved to ClientSpawn for clarity
//	client->ps.powerups[PW_INVULNERABLE] = level.time + 3000; // JPW NERVE some time to find cover
#endif RTCW_XX

	// Communicate it to cgame
	client->ps.stats[STAT_PLAYER_CLASS] = pc;

	// Abuse teamNum to store player class as well (can't see stats for all clients in cgame)
	client->ps.teamNum = pc;

// JPW NERVE -- zero out all ammo counts
	memset( client->ps.ammo,MAX_WEAPONS,sizeof( int ) );

	// All players start with a knife (not OR-ing so that it clears previous weapons)
	client->ps.weapons[0] = 0;
	client->ps.weapons[1] = 0;
	COM_BitSet( client->ps.weapons, WP_KNIFE );

	client->ps.ammo[BG_FindAmmoForWeapon( WP_KNIFE )] = 1;
	client->ps.weapon = WP_KNIFE;
	client->ps.weaponstate = WEAPON_READY;

	// Engineer gets dynamite
	if ( pc == PC_ENGINEER ) {
		COM_BitSet( client->ps.weapons, WP_DYNAMITE );
		client->ps.ammo[BG_FindAmmoForWeapon( WP_DYNAMITE )] = 0;
		client->ps.ammoclip[BG_FindClipForWeapon( WP_DYNAMITE )] = 1;

#if defined RTCW_MP
		// NERVE - SMF
		COM_BitSet( client->ps.weapons, WP_PLIERS );
		client->ps.ammoclip[BG_FindClipForWeapon( WP_PLIERS )] = 1;
		client->ps.ammo[WP_PLIERS] = 1;
#endif RTCW_XX

	}

#if defined RTCW_SP
	// Lieutenant gets binoculars
	if ( pc == PC_LT ) {
		client->ps.stats[STAT_KEYS] |= ( 1 << INV_BINOCS );
/* no grenades for balance JPW NERVE
		COM_BitSet( client->ps.weapons, WP_GRENADE_LAUNCHER );
		client->ps.ammo[BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER)] = 2;
		client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_LAUNCHER)] = 1;
*/
	}

	// Everyone except the Soldier has a special weapon
	if ( pc != PC_SOLDIER ) {
		COM_BitSet( client->ps.weapons, WP_CLASS_SPECIAL );
		client->ps.ammo[WP_CLASS_SPECIAL] = 1;
		// ammo for other special weapons so that icons draw ok
		client->ps.ammo[WP_MEDIC_HEAL] = 1;
		client->ps.ammo[WP_GRENADE_SMOKE] = 1;
	}

	// Everyone gets a pistol
	switch ( client->sess.sessionTeam ) { // JPW NERVE was playerPistol

	case TEAM_RED: // JPW NERVE
		COM_BitSet( client->ps.weapons, WP_LUGER );
		client->ps.ammoclip[BG_FindClipForWeapon( WP_LUGER )] += 8;
		client->ps.ammo[BG_FindAmmoForWeapon( WP_LUGER )] += 24;
		client->ps.weapon = WP_LUGER;
		break;
	default: // '0' // TEAM_BLUE
		COM_BitSet( client->ps.weapons, WP_COLT );
		client->ps.ammoclip[BG_FindClipForWeapon( WP_COLT )] += 8;
		client->ps.ammo[BG_FindAmmoForWeapon( WP_COLT )] += 24;
		client->ps.weapon = WP_COLT;
		break;
	}

	// Everyone except Medic and LT get some grenades
	if ( ( pc != PC_LT ) && ( pc != PC_MEDIC ) ) { // JPW NERVE

		switch ( client->sess.sessionTeam ) { // was playerItem

		case TEAM_BLUE:
			COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
			client->ps.ammo[BG_FindAmmoForWeapon( WP_GRENADE_PINEAPPLE )] = 4;
			client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = 1;
			break;
		case TEAM_RED:
			COM_BitSet( client->ps.weapons, WP_GRENADE_LAUNCHER );
			client->ps.ammo[BG_FindAmmoForWeapon( WP_GRENADE_LAUNCHER )] = 4;
			client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_LAUNCHER )] = 1;
			break;
		default:
			COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
			client->ps.ammo[BG_FindAmmoForWeapon( WP_GRENADE_PINEAPPLE )] = 4;
			client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = 1;
			break;
		}
	}

	// Soldiers and Lieutenants get a 2-handed weapon
	if ( pc == PC_SOLDIER || pc == PC_LT ) {

// JPW NERVE -- if LT is selected but illegal weapon, set to team-specific SMG
		if ( ( pc == PC_LT ) && ( client->sess.playerWeapon > 5 ) ) {
			if ( client->sess.sessionTeam == TEAM_RED ) {
				client->sess.playerWeapon = 3;
			} else {
				client->sess.playerWeapon = 4;
			}
		}
// jpw
		switch ( client->sess.playerWeapon ) {

		case 3:     // WP_MP40
			COM_BitSet( client->ps.weapons, WP_MP40 );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MP40 )] += 32;
			if ( pc == PC_SOLDIER ) {
				client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += 64;
			} else {
				client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += 32;
			}
			client->ps.weapon = WP_MP40;
			break;

		case 4:     // WP_THOMPSON
			COM_BitSet( client->ps.weapons, WP_THOMPSON );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_THOMPSON )] += 30;
			if ( pc == PC_SOLDIER ) {
				client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += 60;
			} else {
				client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += 30;
			}
			client->ps.weapon = WP_THOMPSON;
			break;

		case 5:     // WP_STEN
			COM_BitSet( client->ps.weapons, WP_STEN );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_STEN )] += 32;
			if ( pc == PC_SOLDIER ) {
				client->ps.ammo[BG_FindAmmoForWeapon( WP_STEN )] += 64;
			} else {
				client->ps.ammo[BG_FindAmmoForWeapon( WP_STEN )] += 32;
			}
			client->ps.weapon = WP_STEN;
			break;

		case 6:     // WP_MAUSER, WP_SNIPERRIFLE
			if ( pc != PC_SOLDIER ) {
				return;
			}

			COM_BitSet( client->ps.weapons, WP_SNIPERRIFLE );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_SNIPERRIFLE )] = 10;
			client->ps.ammo[BG_FindAmmoForWeapon( WP_SNIPERRIFLE )] = 10;
			client->ps.weapon = WP_SNIPERRIFLE;

			COM_BitSet( client->ps.weapons, WP_MAUSER );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MAUSER )] = 10;
			client->ps.ammo[BG_FindAmmoForWeapon( WP_MAUSER )] = 10;
			client->ps.weapon = WP_MAUSER;
			break;

		case 7:     // WP_GARAND, WP_SNOOPERSCOPE
			if ( pc != PC_SOLDIER ) {
				return;
			}

			COM_BitSet( client->ps.weapons, WP_SNOOPERSCOPE );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_SNOOPERSCOPE )] = 5;
			client->ps.ammo[BG_FindAmmoForWeapon( WP_SNOOPERSCOPE )] = 15;
			client->ps.weapon = WP_SNOOPERSCOPE;

			COM_BitSet( client->ps.weapons, WP_GARAND );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_GARAND )] = 5;
			client->ps.ammo[BG_FindAmmoForWeapon( WP_GARAND )] = 15;
			client->ps.weapon = WP_GARAND;

			break;

		case 8:     // WP_PANZERFAUST
			if ( pc != PC_SOLDIER ) {
				return;
			}

			COM_BitSet( client->ps.weapons, WP_PANZERFAUST );
			client->ps.ammo[BG_FindAmmoForWeapon( WP_PANZERFAUST )] = 4;
			client->ps.weapon = WP_PANZERFAUST;
			break;

		case 9:     // WP_VENOM
			if ( pc != PC_SOLDIER ) {
				return;
			}
// JPW NERVE may not keep this, but put it in to test
			COM_BitSet( client->ps.weapons, WP_VENOM );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_VENOM )] = 200;
			client->ps.ammo[BG_FindAmmoForWeapon( WP_VENOM )] = 500;
			client->ps.weapon = WP_VENOM;
			break;

		case 10:    // WP_FLAMETHROWER
			if ( pc != PC_SOLDIER ) {
				return;
			}

			COM_BitSet( client->ps.weapons, WP_FLAMETHROWER );
			client->ps.ammo[BG_FindAmmoForWeapon( WP_FLAMETHROWER )] = 300;
			client->ps.weapon = WP_FLAMETHROWER;
			break;

		default:    // give MP40 if given invalid weapon number
			if ( client->sess.sessionTeam == TEAM_RED ) { // JPW NERVE
				COM_BitSet( client->ps.weapons, WP_MP40 );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_MP40 )] += 32;
				client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += 64;
				client->ps.weapon = WP_MP40;
			} else { // TEAM_BLUE
				COM_BitSet( client->ps.weapons, WP_THOMPSON );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_THOMPSON )] += 30;
				client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += 60;
				client->ps.weapon = WP_THOMPSON;
			}
			break;
		}
	} else { // medic or engineer gets assigned MP40 or Thompson with one magazine ammo
		if ( client->sess.sessionTeam == TEAM_RED ) { // axis
			COM_BitSet( client->ps.weapons, WP_MP40 );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MP40 )] += 32;
			client->ps.weapon = WP_MP40;
		} else { // allied
			COM_BitSet( client->ps.weapons, WP_THOMPSON );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_THOMPSON )] += 30;
			client->ps.weapon = WP_THOMPSON;
		}
	}
#elif defined RTCW_MP
	if ( g_knifeonly.integer != 1 ) {

		// Lieutenant gets binoculars, ammo pack, artillery, and a grenade
		if ( pc == PC_LT ) {
			client->ps.stats[STAT_KEYS] |= ( 1 << INV_BINOCS );
			COM_BitSet( client->ps.weapons, WP_AMMO );
			client->ps.ammo[BG_FindAmmoForWeapon( WP_AMMO )] = 0;
			client->ps.ammoclip[BG_FindClipForWeapon( WP_AMMO )] = 1;
			COM_BitSet( client->ps.weapons, WP_ARTY );
			client->ps.ammo[BG_FindAmmoForWeapon( WP_ARTY )] = 0;
			client->ps.ammoclip[BG_FindClipForWeapon( WP_ARTY )] = 1;

			// NERVE - SMF
			COM_BitSet( client->ps.weapons, WP_SMOKE_GRENADE );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_SMOKE_GRENADE )] = 1;
			client->ps.ammo[WP_SMOKE_GRENADE] = 1;

			switch ( client->sess.sessionTeam ) {
			case TEAM_BLUE:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = 1;
				break;
			case TEAM_RED:
				COM_BitSet( client->ps.weapons, WP_GRENADE_LAUNCHER );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_LAUNCHER )] = 1;
				break;
			default:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = 1;
				break;
			}
		}

		// Everyone gets a pistol
		switch ( client->sess.sessionTeam ) { // JPW NERVE was playerPistol

		case TEAM_RED: // JPW NERVE
			COM_BitSet( client->ps.weapons, WP_LUGER );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_LUGER )] += 8;
			client->ps.ammo[BG_FindAmmoForWeapon( WP_LUGER )] += 24;
			client->ps.weapon = WP_LUGER;
			break;
		default: // '0' // TEAM_BLUE
			COM_BitSet( client->ps.weapons, WP_COLT );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_COLT )] += 8;
			client->ps.ammo[BG_FindAmmoForWeapon( WP_COLT )] += 24;
			client->ps.weapon = WP_COLT;
			break;
		}

		// Everyone except Medic and LT get some grenades
		if ( ( pc != PC_LT ) && ( pc != PC_MEDIC ) ) { // JPW NERVE

			switch ( client->sess.sessionTeam ) { // was playerItem

			case TEAM_BLUE:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammo[BG_FindAmmoForWeapon( WP_GRENADE_PINEAPPLE )] = 0;
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = 4 + 4 * ( pc == PC_ENGINEER ); // JPW NERVE
				break;
			case TEAM_RED:
				COM_BitSet( client->ps.weapons, WP_GRENADE_LAUNCHER );
				client->ps.ammo[BG_FindAmmoForWeapon( WP_GRENADE_LAUNCHER )] = 0;
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_LAUNCHER )] = 4 + 4 * ( pc == PC_ENGINEER ); // JPW NERVE
				break;
			default:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammo[BG_FindAmmoForWeapon( WP_GRENADE_PINEAPPLE )] = 0;
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = 4 + 4 * ( pc == PC_ENGINEER ); // JPW NERVE
				break;
			}
		}


		// JPW NERVE
		if ( pc == PC_MEDIC ) {
			COM_BitSet( client->ps.weapons, WP_MEDIC_SYRINGE );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MEDIC_SYRINGE )] = 10;

			// NERVE - SMF
			COM_BitSet( client->ps.weapons, WP_MEDKIT );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MEDKIT )] = 1;
			client->ps.ammo[WP_MEDKIT] = 1;

			switch ( client->sess.sessionTeam ) { // was playerItem
			case TEAM_BLUE:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = 1;
				break;
			case TEAM_RED:
				COM_BitSet( client->ps.weapons, WP_GRENADE_LAUNCHER );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_LAUNCHER )] = 1;
				break;
			default:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = 1;
				break;
			}
		}
		// jpw

		// Soldiers and Lieutenants get a 2-handed weapon
		if ( pc == PC_SOLDIER || pc == PC_LT ) {

			// JPW NERVE -- if LT is selected but illegal weapon, set to team-specific SMG
			if ( ( pc == PC_LT ) && ( client->sess.playerWeapon > 5 ) ) {
				if ( client->sess.sessionTeam == TEAM_RED ) {
					client->sess.playerWeapon = 3;
				} else {
					client->sess.playerWeapon = 4;
				}
			}
			// jpw
			switch ( client->sess.playerWeapon ) {

			case 3:     // WP_MP40
				COM_BitSet( client->ps.weapons, WP_MP40 );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_MP40 )] += 32;
				if ( pc == PC_SOLDIER ) {
					client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += 64;
				} else {
					client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += 32;
				}
				client->ps.weapon = WP_MP40;
				break;

			case 4:     // WP_THOMPSON
				COM_BitSet( client->ps.weapons, WP_THOMPSON );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_THOMPSON )] += 30;
				if ( pc == PC_SOLDIER ) {
					client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += 60;
				} else {
					client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += 30;
				}
				client->ps.weapon = WP_THOMPSON;
				break;

			case 5:     // WP_STEN
				COM_BitSet( client->ps.weapons, WP_STEN );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_STEN )] += 32;
				if ( pc == PC_SOLDIER ) {
					client->ps.ammo[BG_FindAmmoForWeapon( WP_STEN )] += 64;
				} else {
					client->ps.ammo[BG_FindAmmoForWeapon( WP_STEN )] += 32;
				}
				client->ps.weapon = WP_STEN;
				break;

			case 6:     // WP_MAUSER, WP_SNIPERRIFLE
				if ( pc != PC_SOLDIER ) {
					return;
				}

				COM_BitSet( client->ps.weapons, WP_SNIPERRIFLE );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_SNIPERRIFLE )] = 10;
				client->ps.ammo[BG_FindAmmoForWeapon( WP_SNIPERRIFLE )] = 10;
				client->ps.weapon = WP_SNIPERRIFLE;

				COM_BitSet( client->ps.weapons, WP_MAUSER );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_MAUSER )] = 10;
				client->ps.ammo[BG_FindAmmoForWeapon( WP_MAUSER )] = 10;
				client->ps.weapon = WP_MAUSER;
				break;

			case 8:     // WP_PANZERFAUST
				if ( pc != PC_SOLDIER ) {
					return;
				}

				COM_BitSet( client->ps.weapons, WP_PANZERFAUST );
				client->ps.ammo[BG_FindAmmoForWeapon( WP_PANZERFAUST )] = 4;
				client->ps.weapon = WP_PANZERFAUST;
				break;

			case 9:     // WP_VENOM
				if ( pc != PC_SOLDIER ) {
					return;
				}
				COM_BitSet( client->ps.weapons, WP_VENOM );
				client->ps.ammoclip[BG_FindAmmoForWeapon( WP_VENOM )] = 500;
				client->ps.weapon = WP_VENOM;
				break;

			case 10:    // WP_FLAMETHROWER
				if ( pc != PC_SOLDIER ) {
					return;
				}

				COM_BitSet( client->ps.weapons, WP_FLAMETHROWER );
				client->ps.ammoclip[BG_FindAmmoForWeapon( WP_FLAMETHROWER )] = 200;
				client->ps.weapon = WP_FLAMETHROWER;
				break;

			default:    // give MP40 if given invalid weapon number
				if ( client->sess.sessionTeam == TEAM_RED ) { // JPW NERVE
					COM_BitSet( client->ps.weapons, WP_MP40 );
					client->ps.ammoclip[BG_FindClipForWeapon( WP_MP40 )] += 32;
					if ( pc == PC_SOLDIER ) {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += 64;
					} else {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += 32;
					}
					client->ps.weapon = WP_MP40;
				} else { // TEAM_BLUE
					COM_BitSet( client->ps.weapons, WP_THOMPSON );
					client->ps.ammoclip[BG_FindClipForWeapon( WP_THOMPSON )] += 30;
					if ( pc == PC_SOLDIER ) {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += 60;
					} else {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += 30;
					}
					client->ps.weapon = WP_THOMPSON;
				}
				break;
			}
		} else { // medic or engineer gets assigned MP40 or Thompson with one magazine ammo
			if ( client->sess.sessionTeam == TEAM_RED ) { // axis
				COM_BitSet( client->ps.weapons, WP_MP40 );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_MP40 )] += 32;
				// JPW NERVE
				if ( pc == PC_ENGINEER ) { // OK so engineers get two mags
					client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += 32;
				}
				// jpw
				client->ps.weapon = WP_MP40;
			} else { // allied
				COM_BitSet( client->ps.weapons, WP_THOMPSON );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_THOMPSON )] += 30;
				// JPW NERVE
				if ( pc == PC_ENGINEER ) {
					client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += 32;
				}
				// jpw
				client->ps.weapon = WP_THOMPSON;
			}
		}

	} else // Knifeonly block
	{
		if ( pc == PC_MEDIC ) {
			COM_BitSet( client->ps.weapons, WP_MEDIC_SYRINGE );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MEDIC_SYRINGE )] = 20;

			// NERVE - SMF
			COM_BitSet( client->ps.weapons, WP_MEDKIT );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MEDKIT )] = 1;
			client->ps.ammo[WP_MEDKIT] = 1;
		}
	} // End Knifeonly stuff -- Ensure that medics get their basic stuff
#endif RTCW_XX

	// JPW NERVE -- medics on each team make cumulative health bonus -- this gets overridden for "revived" players
	// count up # of medics on team
	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam != client->sess.sessionTeam ) {
			continue;
		}
		if ( level.clients[i].ps.stats[STAT_PLAYER_CLASS] != PC_MEDIC ) {
			continue;
		}
		numMedics++;
	}

// compute health mod
	starthealth = 100 + 10 * numMedics;
	if ( starthealth > 125 ) {
		starthealth = 125;
	}

// give everybody health mod in stat_max_health
	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == client->sess.sessionTeam ) {
			client->ps.stats[STAT_MAX_HEALTH] = starthealth;
		}
	}
// jpw
}
// dhm - end


/*
===========
ClientCheckName
============
*/
static void ClientCleanName( const char *in, char *out, int outSize ) {
	int len, colorlessLen;
	char ch;
	char    *p;
	int spaces;

	//save room for trailing null byte
	outSize--;

	len = 0;
	colorlessLen = 0;
	p = out;
	*p = 0;
	spaces = 0;

	while ( 1 ) {
		ch = *in++;
		if ( !ch ) {
			break;
		}

		// don't allow leading spaces
		if ( !*p && ch == ' ' ) {
			continue;
		}

		// check colors
		if ( ch == Q_COLOR_ESCAPE ) {
			// solo trailing carat is not a color prefix
			if ( !*in ) {
				break;
			}

			// don't allow black in a name, period
			if ( ColorIndex( *in ) == 0 ) {
				in++;
				continue;
			}

			// make sure room in dest for both chars

#if defined RTCW_SP
			if ( len == outSize - 2 ) {
#elif defined RTCW_MP
			if ( len > outSize - 2 ) {
#endif RTCW_XX

				break;
			}

			*out++ = ch;
			*out++ = *in++;
			len += 2;
			continue;
		}

		// don't allow too many consecutive spaces
		if ( ch == ' ' ) {
			spaces++;
			if ( spaces > 3 ) {
				continue;
			}
		} else {
			spaces = 0;
		}

#if defined RTCW_MP
		if ( len > outSize - 1 ) {
			break;
		}
#endif RTCW_XX

		*out++ = ch;
		colorlessLen++;
		len++;

#if defined RTCW_SP
		if ( len == outSize - 1 ) {
			break;
		}
#endif RTCW_XX

	}
	*out = 0;

	// don't allow empty names
	if ( *p == 0 || colorlessLen == 0 ) {

#if defined RTCW_SP
		Q_strncpyz( out, "UnnamedPlayer", outSize );
#elif defined RTCW_MP
		Q_strncpyz( p, "UnnamedPlayer", outSize );
#endif RTCW_XX

	}
}

/*
==================
G_CheckForExistingModelInfo

  If this player model has already been parsed, then use the existing information.
  Otherwise, set the modelInfo pointer to the first free slot.

  returns qtrue if existing model found, qfalse otherwise
==================
*/
qboolean G_CheckForExistingModelInfo( gclient_t *cl, char *modelName, animModelInfo_t **modelInfo ) {
	int i;

#if defined RTCW_SP
	animModelInfo_t *trav;

	for ( i = 0; i < MAX_ANIMSCRIPT_MODELS; i++ ) {
		trav = level.animScriptData.modelInfo[i];
		if ( trav && trav->modelname[0] ) {

#elif defined RTCW_MP
	animModelInfo_t *trav, *firstFree = NULL;
	gclient_t *cl_trav;
	char modelsUsed[MAX_ANIMSCRIPT_MODELS];

	for ( i = 0, trav = level.animScriptData.modelInfo; i < MAX_ANIMSCRIPT_MODELS; i++, trav++ ) {
		if ( trav->modelname[0] ) {
#endif RTCW_XX

			if ( !Q_stricmp( trav->modelname, modelName ) ) {
				// found a match, use this modelinfo
				*modelInfo = trav;
				level.animScriptData.clientModels[cl->ps.clientNum] = i + 1;
				return qtrue;
			}

#if defined RTCW_SP
		} else {
			level.animScriptData.modelInfo[i] = G_Alloc( sizeof( animModelInfo_t ) );
			*modelInfo = level.animScriptData.modelInfo[i];
			// clear the structure out ready for use
			memset( *modelInfo, 0, sizeof( **modelInfo ) );
			level.animScriptData.clientModels[cl->ps.clientNum] = i + 1;
			return qfalse;
#elif defined RTCW_MP
		} else if ( !firstFree ) {
			firstFree = trav;
			level.animScriptData.clientModels[cl->ps.clientNum] = i + 1;
#endif RTCW_XX

		}
	}

#if defined RTCW_SP
	G_Error( "unable to find a free modelinfo slot, cannot continue\n" );
	// qfalse signifies that we need to parse the information from the script files
	return qfalse;
#elif defined RTCW_MP
	// set the modelInfo to the first free slot
	if ( !firstFree ) {
		// attempt to free a model that is no longer being used
		memset( modelsUsed, 0, sizeof( modelsUsed ) );
		for ( i = 0, cl_trav = level.clients; i < MAX_CLIENTS; i++, cl_trav++ ) {
			if ( cl_trav != cl && g_entities[cl_trav->ps.clientNum].inuse && cl_trav->modelInfo ) {
				modelsUsed[ (int)( cl_trav->modelInfo - level.animScriptData.modelInfo ) ] = 1;
			}
		}
		// now use the first slot that isn't being utilized
		for ( i = 0, trav = level.animScriptData.modelInfo; i < MAX_ANIMSCRIPT_MODELS; i++, trav++ ) {
			if ( !modelsUsed[i] ) {
				firstFree = trav;
				level.animScriptData.clientModels[cl->ps.clientNum] = i + 1;
				break;
			}
		}
	}

	if ( !firstFree ) {
		G_Error( "unable to find a free modelinfo slot, cannot continue\n" );
	} else {
		*modelInfo = firstFree;
		// clear the structure out ready for use
		memset( *modelInfo, 0, sizeof( *modelInfo ) );
	}
	// qfalse signifies that we need to parse the information from the script files
	return qfalse;
#endif RTCW_XX

}

#if defined RTCW_SP
/*
==============
G_GetModelInfo
==============
*/
qboolean G_ParseAnimationFiles( char *modelname, gclient_t *cl );
qboolean G_GetModelInfo( int clientNum, char *modelName, animModelInfo_t **modelInfo ) {

	if ( !G_CheckForExistingModelInfo( &level.clients[clientNum], modelName, modelInfo ) ) {
		level.clients[clientNum].modelInfo = *modelInfo;
		if ( !G_ParseAnimationFiles( modelName, &level.clients[clientNum] ) ) {
			G_Error( "Failed to load animation scripts for model %s\n", modelName );
		}
	}

	return qtrue;
}
#endif RTCW_XX

/*
=============
G_ParseAnimationFiles
=============
*/
#if defined RTCW_MP
static char text[100000];                   // <- was causing callstacks >64k
#endif RTCW_XX

qboolean G_ParseAnimationFiles( char *modelname, gclient_t *cl ) {

#if defined RTCW_SP
	char text[100000];
#endif RTCW_XX

	char filename[MAX_QPATH];
	fileHandle_t f;
	int len;

	// set the name of the model in the modelinfo structure
	Q_strncpyz( cl->modelInfo->modelname, modelname, sizeof( cl->modelInfo->modelname ) );

	// load the cfg file
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/wolfanim.cfg", modelname );
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		G_Printf( "G_ParseAnimationFiles(): file '%s' not found\n", filename );       //----(SA)	added
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		G_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	BG_AnimParseAnimConfig( cl->modelInfo, filename, text );

	// load the script file
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/wolfanim.script", modelname );
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		if ( cl->modelInfo->version > 1 ) {
			return qfalse;
		}
		// try loading the default script for old legacy models
		Com_sprintf( filename, sizeof( filename ), "models/players/default.script", modelname );
		len = trap_FS_FOpenFile( filename, &f, FS_READ );
		if ( len <= 0 ) {
			return qfalse;
		}
	}
	if ( len >= sizeof( text ) - 1 ) {
		G_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	BG_AnimParseAnimScript( cl->modelInfo, &level.animScriptData, cl->ps.clientNum, filename, text );

	// ask the client to send us the movespeeds if available
	if ( g_gametype.integer == GT_SINGLE_PLAYER && g_entities[0].client && g_entities[0].client->pers.connected == CON_CONNECTED ) {
		trap_SendServerCommand( 0, va( "mvspd %s", modelname ) );
	}

	return qtrue;
}


/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {

	gentity_t *ent;
	char    *s;
	char model[MAX_QPATH], modelname[MAX_QPATH];

//----(SA) added this for head separation
	char head[MAX_QPATH];

	char oldname[MAX_STRING_CHARS];
	gclient_t   *client;
	char    *c1;
	char userinfo[MAX_INFO_STRING];

	ent = g_entities + clientNum;
	client = ent->client;

	client->ps.clientNum = clientNum;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if ( !Info_Validate( userinfo ) ) {
		strcpy( userinfo, "\\name\\badinfo" );
	}

	// check for local client
	s = Info_ValueForKey( userinfo, "ip" );

#if defined RTCW_SP
	if ( !strcmp( s, "localhost" ) ) {
#elif defined RTCW_MP
	if ( s && !strcmp( s, "localhost" ) ) {
#endif RTCW_XX

		client->pers.localClient = qtrue;
	}

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) ) {
		client->pers.predictItemPickup = qfalse;
	} else {
		client->pers.predictItemPickup = qtrue;
	}

	// check the auto activation
	s = Info_ValueForKey( userinfo, "cg_autoactivate" );
	if ( !atoi( s ) ) {
		client->pers.autoActivate = PICKUP_ACTIVATE;
	} else {
		client->pers.autoActivate = PICKUP_TOUCH;
	}

#if defined RTCW_SP
	// check the auto empty weapon switching
	s = Info_ValueForKey( userinfo, "cg_emptyswitch" );
	if ( !atoi( s ) ) {
		client->pers.emptySwitch = 0;
	} else {
		client->pers.emptySwitch = 1;
	}
#elif defined RTCW_MP
	// check the auto reload setting
	s = Info_ValueForKey( userinfo, "cg_autoReload" );
	if ( atoi( s ) ) {
		client->pers.bAutoReloadAux = qtrue;
		client->pmext.bAutoReload = qtrue;
	} else {
		client->pers.bAutoReloadAux = qfalse;
		client->pmext.bAutoReload = qfalse;
	}
#endif RTCW_XX

	// set name
	Q_strncpyz( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey( userinfo, "name" );
	ClientCleanName( s, client->pers.netname, sizeof( client->pers.netname ) );

	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			Q_strncpyz( client->pers.netname, "scoreboard", sizeof( client->pers.netname ) );
		}
	}

	if ( client->pers.connected == CON_CONNECTED ) {
		if ( strcmp( oldname, client->pers.netname ) ) {

#if defined RTCW_SP
			trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname,
#elif defined RTCW_MP
			trap_SendServerCommand( -1, va( "print \"[lof]%s" S_COLOR_WHITE " [lon]renamed to[lof] %s\n\"", oldname,
#endif RTCW_XX

											client->pers.netname ) );
		}
	}

	// set max health
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;

	// set model
	if ( g_forceModel.integer ) {
		Q_strncpyz( model, DEFAULT_MODEL, sizeof( model ) );
		Q_strcat( model, sizeof( model ), "/default" );
	} else {
		Q_strncpyz( model, Info_ValueForKey( userinfo, "model" ), sizeof( model ) );
	}

	// RF, reset anims so client's dont freak out
	client->ps.legsAnim = 0;
	client->ps.torsoAnim = 0;

	// DHM - Nerve :: Forcibly set both model and skin for multiplayer.

#if defined RTCW_SP
	if ( g_gametype.integer == GT_WOLF ) {
#elif defined RTCW_MP
	if ( g_gametype.integer >= GT_WOLF ) {
#endif RTCW_XX

		// To communicate it to cgame
		client->ps.stats[ STAT_PLAYER_CLASS ] = client->sess.playerType;

#if defined RTCW_SP
		Q_strncpyz( model, MULTIPLAYER_MODEL, MAX_QPATH );
#elif defined RTCW_MP
		if ( client->sess.sessionTeam == TEAM_BLUE ) {
			Q_strncpyz( model, MULTIPLAYER_ALLIEDMODEL, MAX_QPATH );
		} else {
			Q_strncpyz( model, MULTIPLAYER_AXISMODEL, MAX_QPATH );
		}
#endif RTCW_XX

		Q_strcat( model, MAX_QPATH, "/" );

		SetWolfSkin( client, model );

		Q_strncpyz( head, "", MAX_QPATH );
		SetWolfSkin( client, head );
	}

	// strip the skin name
	Q_strncpyz( modelname, model, sizeof( modelname ) );
	if ( strstr( modelname, "/" ) ) {
		modelname[ strstr( modelname, "/" ) - modelname ] = 0;
	} else if ( strstr( modelname, "\\" ) ) {
		modelname[ strstr( modelname, "\\" ) - modelname ] = 0;
	}

	if ( !G_CheckForExistingModelInfo( client, modelname, &client->modelInfo ) ) {
		if ( !G_ParseAnimationFiles( modelname, client ) ) {
			G_Error( "Failed to load animation scripts for model %s\n", modelname );
		}
	}

	// team`
	// DHM - Nerve :: Already took care of models and skins above

#if defined RTCW_SP
	if ( g_gametype.integer != GT_WOLF ) {
#elif defined RTCW_MP
	if ( g_gametype.integer < GT_WOLF ) {
#endif RTCW_XX

//----(SA) added this for head separation
		// set head
		if ( g_forceModel.integer ) {
			Q_strncpyz( head, DEFAULT_HEAD, sizeof( head ) );
		} else {
			Q_strncpyz( head, Info_ValueForKey( userinfo, "head" ), sizeof( head ) );
		}

//----(SA) end

		switch ( client->sess.sessionTeam ) {
		case TEAM_RED:
			ForceClientSkin( client, model, "red" );
			break;
		case TEAM_BLUE:
			ForceClientSkin( client, model, "blue" );
			break;

#if defined RTCW_SP
		default:
#elif defined RTCW_MP
		default: // TEAM_FREE, TEAM_SPECTATOR, TEAM_NUM_TEAMS not handled in switch
#endif RTCW_XX

			break;
		}
		if ( g_gametype.integer >= GT_TEAM && client->sess.sessionTeam == TEAM_SPECTATOR ) {
			// don't ever use a default skin in teamplay, it would just waste memory
			ForceClientSkin( client, model, "red" );
		}

	}
	//dhm - end


	// colors
	c1 = Info_ValueForKey( userinfo, "color" );

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds

//----(SA) modified these for head separation

	if ( ent->r.svFlags & SVF_BOT ) {

		s = va( "n\\%s\\t\\%i\\model\\%s\\head\\%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s",
				client->pers.netname, client->sess.sessionTeam, model, head, c1,
				client->pers.maxHealth, client->sess.wins, client->sess.losses,
				Info_ValueForKey( userinfo, "skill" ) );
	} else {
		s = va( "n\\%s\\t\\%i\\model\\%s\\head\\%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i",
				client->pers.netname, client->sess.sessionTeam, model, head, c1,
				client->pers.maxHealth, client->sess.wins, client->sess.losses );
	}

//----(SA) end

	trap_SetConfigstring( CS_PLAYERS + clientNum, s );

#if defined RTCW_MP
	// this is not the userinfo actually, it's the config string
#endif RTCW_XX

	G_LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, s );

#if defined RTCW_MP
	G_DPrintf( "ClientUserinfoChanged: %i :: %s\n", clientNum, s );
#endif RTCW_XX

}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char        *value;
	gclient_t   *client;
	char userinfo[MAX_INFO_STRING];
	gentity_t   *ent;

	ent = &g_entities[ clientNum ];

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

#if defined RTCW_MP
	// IP filtering
	// show_bug.cgi?id=500
	// recommanding PB based IP / GUID banning, the builtin system is pretty limited
#endif RTCW_XX

	// check to see if they are on the banned IP list
	value = Info_ValueForKey( userinfo, "ip" );

#if defined RTCW_MP
	if ( G_FilterPacket( value ) ) {
		return "You are banned from this server.";
	}
#endif RTCW_XX

#if defined RTCW_SP
	// check for a password
	value = Info_ValueForKey( userinfo, "password" );
	if ( g_password.string[0] && strcmp( g_password.string, value ) != 0 ) {
		return "Invalid password";
#elif defined RTCW_MP
	// Xian - check for max lives enforcement ban
	if ( g_enforcemaxlives.integer && ( g_maxlives.integer > 0 || g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0 ) ) {
		value = Info_ValueForKey( userinfo, "cl_guid" );
		if ( G_FilterMaxLivesPacket( value ) ) {
			return "Max Lives Enforcement Temp Ban";
		}
	}
	// End Xian

	// we don't check password for bots and local client
	// NOTE: local client <-> "ip" "localhost"
	//   this means this client is not running in our current process
	if ( !( ent->r.svFlags & SVF_BOT ) && ( strcmp( Info_ValueForKey( userinfo, "ip" ), "localhost" ) != 0 ) ) {
		// check for a password
		value = Info_ValueForKey( userinfo, "password" );
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
			 strcmp( g_password.string, value ) != 0 ) {
			return "Invalid password";
		}
#endif RTCW_XX

	}

	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

	memset( client, 0, sizeof( *client ) );

	client->pers.connected = CON_CONNECTING;

#if defined RTCW_MP
	client->pers.connectTime = level.time;          // DHM - Nerve

	if ( firstTime ) {
		client->pers.initialSpawn = qtrue;              // DHM - Nerve

	}
	client->pers.complaints = 0;                    // DHM - Nerve
#endif RTCW_XX

	// read or initialize the session data

#if defined RTCW_SP
	if ( firstTime || level.newSession ) {
#elif defined RTCW_MP
	if ( firstTime || ( g_gametype.integer < GT_WOLF && level.newSession ) ) {
#endif  RTCW_XX

		G_InitSessionData( client, userinfo );
	}
	G_ReadSessionData( client );

	if ( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if ( !G_BotConnect( clientNum, !firstTime ) ) {
			return "BotConnectfailed";
		}
	}

	// get and distribute relevent paramters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		// Ridah
		if ( !ent->r.svFlags & SVF_CASTAI ) {
			// done.

#if defined RTCW_SP
			trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname ) );
#elif defined RTCW_MP
			trap_SendServerCommand( -1, va( "print \"[lof]%s" S_COLOR_WHITE " [lon]connected\n\"", client->pers.netname ) );
#endif RTCW_XX

		}
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	gentity_t   *ent;
	gclient_t   *client;

#if defined RTCW_SP
	gentity_t   *tent;
#elif defined RTCW_MP
	//gentity_t	*tent;
#endif RTCW_XX

	int flags;
	int spawn_count;                // DHM - Nerve

	ent = g_entities + clientNum;

	if ( ent->botDelayBegin ) {
		G_QueueBotBegin( clientNum );
		ent->botDelayBegin = qfalse;
		return;
	}

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	client->pers.connected = CON_CONNECTED;

#if defined RTCW_SP
	client->pers.enterTime = level.time;
#elif defined RTCW_MP
	// ATVI Wolfenstein Misc #414
	// don't reset the enterTime during a map_restart, we only want this when user explicitely changes team (and upon entering map)
	if ( !trap_Cvar_VariableIntegerValue( "sv_serverRestarting" ) ) {
		client->pers.enterTime = level.time;
	}
#endif RTCW_XX

	client->pers.teamState.state = TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	// DHM - Nerve :: Also save PERS_SPAWN_COUNT, so that CG_Respawn happens
	spawn_count = client->ps.persistant[PERS_SPAWN_COUNT];
	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;
	client->ps.persistant[PERS_SPAWN_COUNT] = spawn_count;

	// MrE: use capsule for collision

#if defined RTCW_SP
	client->ps.eFlags |= EF_CAPSULE;
	ent->r.svFlags |= SVF_CAPSULE;
#elif defined RTCW_MP
	//client->ps.eFlags |= EF_CAPSULE;
	//ent->r.svFlags |= SVF_CAPSULE;

	client->pers.complaintClient = -1;
	client->pers.complaintEndTime = -1;
#endif RTCW_XX

	// locate ent at a spawn point

#if defined RTCW_SP
	ClientSpawn( ent );
#elif defined RTCW_MP
	ClientSpawn( ent, qfalse );

	// Xian -- Changed below for team independant maxlives

	if ( g_maxlives.integer > 0 ) {
		ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = ( g_maxlives.integer - 1 );
	} else {
		ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = -1;
	}

	if ( g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0 ) {
		if ( client->sess.sessionTeam == TEAM_RED ) {
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = ( g_axismaxlives.integer - 1 );
		} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = ( g_alliedmaxlives.integer - 1 );
		} else {
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = -1;
		}
	}

	// DHM - Nerve :: Start players in limbo mode if they change teams during the match
	if ( g_gametype.integer >= GT_WOLF && client->sess.sessionTeam != TEAM_SPECTATOR
		 && ( level.time - client->pers.connectTime ) > 60000 ) {
		ent->client->ps.pm_type = PM_DEAD;
		ent->r.contents = CONTENTS_CORPSE;
		ent->health = 0;
		ent->client->ps.stats[STAT_HEALTH] = 0;

		if ( g_maxlives.integer > 0 ) {
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT]++;
		}

		limbo( ent, qfalse );
	}
#endif RTCW_XX

	// Ridah, trigger a spawn event
	// DHM - Nerve :: Only in single player
	if ( g_gametype.integer == GT_SINGLE_PLAYER && !( ent->r.svFlags & SVF_CASTAI ) ) {
		AICast_ScriptEvent( AICast_GetCastState( clientNum ), "spawn", "" );
	}

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
		// send event

#if defined RTCW_SP
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = ent->s.clientNum;
#elif defined RTCW_MP
		// DHM - Nerve :: Add back if we decide to have a spawn effect
		//tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		//tent->s.clientNum = ent->s.clientNum;
#endif RTCW_XX

		if ( g_gametype.integer != GT_TOURNAMENT ) {
			// Ridah
			if ( !ent->r.svFlags & SVF_CASTAI ) {
				// done.

#if defined RTCW_SP
				trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname ) );
#elif defined RTCW_MP
				trap_SendServerCommand( -1, va( "print \"[lof]%s" S_COLOR_WHITE " [lon]entered the game\n\"", client->pers.netname ) );
#endif RTCW_XX

			}
		}
	}
	G_LogPrintf( "ClientBegin: %i\n", clientNum );

#if defined RTCW_MP
	// Xian - Check for maxlives enforcement
	if ( g_enforcemaxlives.integer == 1 && ( g_maxlives.integer > 0 || g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0 ) ) {
		char *value;
		char userinfo[MAX_INFO_STRING];
		trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
		value = Info_ValueForKey( userinfo, "cl_guid" );
		G_LogPrintf( "EnforceMaxLives-GUID: %s\n", value );
		AddMaxLivesGUID( value );
	}
	// End Xian
#endif RTCW_XX

	// count current clients and rank for scoreboard
	CalculateRanks();

}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/

#if defined RTCW_SP
void ClientSpawn( gentity_t *ent ) {
#elif defined RTCW_MP
void ClientSpawn( gentity_t *ent, qboolean revived ) {
#endif RTCW_XX

	int index;
	vec3_t spawn_origin, spawn_angles;
	gclient_t   *client;
	int i;
	clientPersistant_t saved;
	clientSession_t savedSess;
	int persistant[MAX_PERSISTANT];
	gentity_t   *spawnPoint;
	int flags;
	int savedPing;
	int savedTeam;

#if defined RTCW_MP
	qboolean savedVoted = qfalse;         // NERVE - SMF
#endif RTCW_XX

	index = ent - g_entities;
	client = ent->client;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client

#if defined RTCW_SP
	// Ridah
	if ( ent->r.svFlags & SVF_CASTAI ) {
#elif defined RTCW_MP
	if ( revived ) {
#endif RTCW_XX

		spawnPoint = ent;
		VectorCopy( ent->s.origin, spawn_origin );
		spawn_origin[2] += 9;   // spawns seem to be sunk into ground?
		VectorCopy( ent->s.angles, spawn_angles );
	} else
	{
		ent->aiName = "player";  // needed for script AI

#if defined RTCW_SP
		ent->aiTeam = 1;        // member of allies
		ent->client->ps.teamNum = ent->aiTeam;
		AICast_ScriptParse( AICast_GetCastState( ent->s.number ) );
#elif defined RTCW_MP
		//ent->aiTeam = 1;		// member of allies
		//ent->client->ps.teamNum = ent->aiTeam;
		//AICast_ScriptParse( AICast_GetCastState(ent->s.number) );
#endif RTCW_XX

		// done.

		if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
			spawnPoint = SelectSpectatorSpawnPoint(
				spawn_origin, spawn_angles );
		} else if ( g_gametype.integer >= GT_TEAM ) {
			spawnPoint = SelectCTFSpawnPoint(
				client->sess.sessionTeam,
				client->pers.teamState.state,

#if defined RTCW_SP
				spawn_origin, spawn_angles );
#elif defined RTCW_MP
				spawn_origin, spawn_angles, client->sess.spawnObjectiveIndex );
#endif RTCW_XX

		} else {
			do {
				// the first spawn should be at a good looking spot
				if ( !client->pers.initialSpawn && client->pers.localClient ) {
					client->pers.initialSpawn = qtrue;
					spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles );
				} else {
					// don't spawn near existing origin if possible
					spawnPoint = SelectSpawnPoint(
						client->ps.origin,
						spawn_origin, spawn_angles );
				}

#if defined RTCW_MP
				// Tim needs to prevent bots from spawning at the initial point
				// on q3dm0...
#endif RTCW_XX

				if ( ( spawnPoint->flags & FL_NO_BOTS ) && ( ent->r.svFlags & SVF_BOT ) ) {
					continue;   // try again
				}
				// just to be symetric, we have a nohumans option...
				if ( ( spawnPoint->flags & FL_NO_HUMANS ) && !( ent->r.svFlags & SVF_BOT ) ) {
					continue;   // try again
				}

				break;

			} while ( 1 );
		}

		// Ridah
	}
	// done.

	client->pers.teamState.state = TEAM_ACTIVE;

	// toggle the teleport bit so the client knows to not lerp
	flags = ent->client->ps.eFlags & EF_TELEPORT_BIT;
	flags ^= EF_TELEPORT_BIT;

	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
	savedTeam = client->ps.teamNum;

#if defined RTCW_MP
	// NERVE - SMF
	if ( client->ps.eFlags & EF_VOTED ) {
		savedVoted = qtrue;
	}
#endif RTCW_XX

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}

	memset( client, 0, sizeof( *client ) );

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
	client->ps.teamNum = savedTeam;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}

	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	client->ps.eFlags = flags;
	// MrE: use capsules for AI and player

#if defined RTCW_SP
	client->ps.eFlags |= EF_CAPSULE;
#elif defined RTCW_MP
	//client->ps.eFlags |= EF_CAPSULE;

	// TTimo
	if ( savedVoted ) {
		client->ps.eFlags |= EF_VOTED;
	}
#endif RTCW_XX

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	if ( !( ent->r.svFlags & SVF_CASTAI ) ) {
		ent->classname = "player";
	}
	ent->r.contents = CONTENTS_BODY;

	// RF, AI should be clipped by monsterclip brushes
	if ( ent->r.svFlags & SVF_CASTAI ) {
		ent->clipmask = MASK_PLAYERSOLID | CONTENTS_MONSTERCLIP;
	} else {
		ent->clipmask = MASK_PLAYERSOLID;
	}

#if defined RTCW_MP
	// DHM - Nerve :: Init to -1 on first spawn;
	if ( !revived ) {
		ent->props_frame_state = -1;
	}
#endif RTCW_XX

	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;

	VectorCopy( playerMins, ent->r.mins );
	VectorCopy( playerMaxs, ent->r.maxs );

	// Ridah, setup the bounding boxes and viewheights for prediction
	VectorCopy( ent->r.mins, client->ps.mins );
	VectorCopy( ent->r.maxs, client->ps.maxs );

	client->ps.crouchViewHeight = CROUCH_VIEWHEIGHT;
	client->ps.standViewHeight = DEFAULT_VIEWHEIGHT;
	client->ps.deadViewHeight = DEAD_VIEWHEIGHT;

	client->ps.crouchMaxZ = client->ps.maxs[2] - ( client->ps.standViewHeight - client->ps.crouchViewHeight );

	client->ps.runSpeedScale = 0.8;

#if defined RTCW_SP
//	client->ps.sprintSpeedScale = 1.20;
	client->ps.sprintSpeedScale = 1.1;  // (SA) trying new value
#elif defined RTCW_MP
	client->ps.sprintSpeedScale = 1.1;
#endif RTCW_XX

	client->ps.crouchSpeedScale = 0.25;

	// Rafael
	client->ps.sprintTime = 20000;
	client->ps.sprintExertTime = 0;

	client->ps.friction = 1.0;
	// done.

#if defined RTCW_MP
	// TTimo
	// retrieve from the persistant storage (we use this in pmoveExt_t beause we need it in bg_*)
	client->pmext.bAutoReload = client->pers.bAutoReloadAux;
	// done
#endif RTCW_XX

	client->ps.clientNum = index;

#if defined RTCW_SP
	// DHM - Nerve :: Add appropriate weapons
	if ( g_gametype.integer == GT_WOLF ) {
		SetWolfSpawnWeapons( client ); // JPW NERVE -- increases stats[STAT_MAX_HEALTH] based on # of medics in game
	}
	// dhm - end

	// Note to Ryan:
	// had to add this because key word giveweapon to player is causing a fatal crash
	// This is only a quick fix for the beach map
/*
	if (!(ent->r.svFlags & SVF_CASTAI) && level.scriptAI && strstr (level.scriptAI, "beach assault"))
	{
		COM_BitSet( client->ps.weapons, WP_THOMPSON );
		client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] = 100;

		COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
		client->ps.ammo[BG_FindAmmoForWeapon(WP_GRENADE_PINEAPPLE)] = 5;

		client->ps.weapon = WP_THOMPSON;
		client->ps.weaponstate = WEAPON_READY;
	}
*/
	//----(SA) no longer giving the player any default stuff

//	COM_BitSet( client->ps.weapons, WP_MP40 );
//	client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] = 100;

//	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
//		client->ps.ammo[BG_FindAmmoForWeapon(WP_LUGER)] = 50;
//	} else {
//		client->ps.ammo[BG_FindAmmoForWeapon(WP_LUGER)] = 100;
//	}

//	COM_BitSet( client->ps.weapons, WP_GAUNTLET );
//	client->ps.ammo[BG_FindAmmoForWeapon(WP_GAUNTLET)] = -1;

	// health will count down towards max_health
//	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] * 1.25;

// JPW NERVE ***NOTE*** the following line is order-dependent and must *FOLLOW* SetWolfSpawnWeapons() in multiplayer
// SetWolfSpawnWeapons() now adds medic team bonus and stores in ps.stats[STAT_MAX_HEALTH].
#elif defined RTCW_MP
	trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );  // NERVE - SMF - moved this up here

	SetWolfUserVars( ent, NULL );           // NERVE - SMF

	// DHM - Nerve :: Add appropriate weapons
	if ( g_gametype.integer >= GT_WOLF ) {

		if ( !revived ) {
			qboolean update = qfalse;

			if ( client->sess.playerType != client->sess.latchPlayerType ) {
				update = qtrue;
			}

			client->sess.playerType = client->sess.latchPlayerType;
			client->sess.playerWeapon = client->sess.latchPlayerWeapon;
			client->sess.playerItem = client->sess.latchPlayerItem;
			client->sess.playerSkin = client->sess.latchPlayerSkin;

			if ( update ) {
				ClientUserinfoChanged( index );
			}
		}

		// TTimo keep it isolated from spectator to be safe still
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			// Xian - Moved the invul. stuff out of SetWolfSpawnWeapons and put it here for clarity
			if ( g_fastres.integer == 1 && revived ) {
				client->ps.powerups[PW_INVULNERABLE] = level.time + g_fastResMsec.integer;
			} else {
				client->ps.powerups[PW_INVULNERABLE] = level.time + 3000;
			}
		}

		// End Xian
		SetWolfSpawnWeapons( client ); // JPW NERVE -- increases stats[STAT_MAX_HEALTH] based on # of medics in game
	}
	// dhm - end

	// JPW NERVE ***NOTE*** the following line is order-dependent and must *FOLLOW* SetWolfSpawnWeapons() in multiplayer
	// SetWolfSpawnWeapons() now adds medic team bonus and stores in ps.stats[STAT_MAX_HEALTH].
#endif RTCW_XX

	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

#if defined RTCW_SP
	trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
#endif RTCW_XX

	SetClientViewAngle( ent, spawn_angles );

#if defined RTCW_SP
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {

	} else {
		G_KillBox( ent );
		trap_LinkEntity( ent );

		// force the base weapon up
//		client->ps.weapon = WP_MP40;
//		client->ps.weaponstate = WEAPON_READY;

	}
#elif defined RTCW_MP
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		//G_KillBox( ent );
		trap_LinkEntity( ent );
	}
#endif RTCW_XX

#if defined RTCW_SP
	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;
#endif RTCW_XX

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;
	client->latched_wbuttons = 0;   //----(SA)	added

#if defined RTCW_MP
	// set default animations
	//client->ps.torsoAnim = TORSO_STAND;
	//client->ps.legsAnim = LEGS_IDLE;
#endif RTCW_XX
	if ( level.intermissiontime ) {
		MoveClientToIntermission( ent );
	} else {
		// fire the targets of the spawn point

#if defined RTCW_SP
		G_UseTargets( spawnPoint, ent );

		// select the highest weapon number available, after any
		// spawn given items have fired
//		client->ps.weapon = 1;
//		for ( i = WP_NUM_WEAPONS - 1 ; i > 0 ; i-- ) {
//			if ( COM_BitCheck( client->ps.weapons, i ) ) {
//				client->ps.weapon = i;
//				break;
//			}
//		}
	}
#elif defined RTCW_MP
		if ( !revived ) {
			G_UseTargets( spawnPoint, ent );
		}
	}
#endif RTCW_XX

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent - g_entities );

	// positively link the client, even if the command times are weird
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
	}

	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );

#if defined RTCW_MP
	// show_bug.cgi?id=569
	G_ResetMarkers( ent );
#endif RTCW_XX

}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t   *ent;

#if defined RTCW_SP
	gentity_t   *tent;
#elif defined RTCW_MP
	gentity_t   *flag = NULL;
	gitem_t     *item = NULL;
	vec3_t launchvel;
#endif RTCW_XX

	int i;

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;
	}

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			 && level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			 && level.clients[i].sess.spectatorClient == clientNum ) {
			StopFollowing( &g_entities[i] );
		}

#if defined RTCW_MP
		if ( g_gametype.integer >= GT_WOLF
			 && level.clients[i].ps.pm_flags & PMF_LIMBO
			 && level.clients[i].sess.spectatorClient == clientNum ) {
			Cmd_FollowCycle_f( &g_entities[i], 1 );
		}
#endif RTCW_XX

	}

#if defined RTCW_SP
	// Ridah
	if ( !( ent->r.svFlags & SVF_CASTAI ) ) {
		// done.

		// send effect if they were completely connected
		if ( ent->client->pers.connected == CON_CONNECTED
			 && ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
			tent->s.clientNum = ent->s.clientNum;

			// They don't get to take powerups with them!
			// Especially important for stuff like CTF flags
			TossClientItems( ent );
		}

		G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

		// Ridah
	}
	// done.
#elif defined RTCW_MP
	// NERVE - SMF - remove complaint client
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.complaintClient == clientNum ) {
			level.clients[i].pers.complaintClient = -1;
			level.clients[i].pers.complaintEndTime = 0;

			trap_SendServerCommand( i, "complaint -2" );
			break;
		}
	}

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED
		 && ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems( ent );

		// New code for tossing flags
		if ( g_gametype.integer >= GT_WOLF ) {
			if ( ent->client->ps.powerups[PW_REDFLAG] ) {
				item = BG_FindItem( "Red Flag" );
				if ( !item ) {
					item = BG_FindItem( "Objective" );
				}

				ent->client->ps.powerups[PW_REDFLAG] = 0;
			}
			if ( ent->client->ps.powerups[PW_BLUEFLAG] ) {
				item = BG_FindItem( "Blue Flag" );
				if ( !item ) {
					item = BG_FindItem( "Objective" );
				}

				ent->client->ps.powerups[PW_BLUEFLAG] = 0;
			}

			if ( item ) {
				launchvel[0] = crandom() * 20;
				launchvel[1] = crandom() * 20;
				launchvel[2] = 10 + random() * 10;

				flag = LaunchItem( item,ent->r.currentOrigin,launchvel,ent->s.number );
				flag->s.modelindex2 = ent->s.otherEntityNum2; // JPW NERVE FIXME set player->otherentitynum2 with old modelindex2 from flag and restore here
				flag->message = ent->message;   // DHM - Nerve :: also restore item name
				// Clear out player's temp copies
				ent->s.otherEntityNum2 = 0;
				ent->message = NULL;
			}
		}
	}

	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );
#endif RTCW_XX

	// if we are playing in tourney mode and losing, give a win to the other player
	if ( g_gametype.integer == GT_TOURNAMENT && !level.intermissiontime
		 && !level.warmupTime && level.sortedClients[1] == clientNum ) {
		level.clients[ level.sortedClients[0] ].sess.wins++;
		ClientUserinfoChanged( level.sortedClients[0] );
	}

	trap_UnlinkEntity( ent );
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->client->sess.sessionTeam = TEAM_FREE;

#if defined RTCW_MP
// JPW NERVE -- mg42 additions
	ent->active = 0;
// jpw
#endif RTCW_XX

	trap_SetConfigstring( CS_PLAYERS + clientNum, "" );

	CalculateRanks();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( clientNum );
	}
}


/*
==================
G_RetrieveMoveSpeedsFromClient
==================
*/
void G_RetrieveMoveSpeedsFromClient( int entnum, char *text ) {
	char *text_p, *token;
	animation_t *anim;
	animModelInfo_t *modelInfo;

	text_p = text;

	// get the model name
	token = COM_Parse( &text_p );
	if ( !token || !token[0] ) {
		G_Error( "G_RetrieveMoveSpeedsFromClient: internal error" );
	}

	modelInfo = BG_ModelInfoForModelname( token );

	if ( !modelInfo ) {
		// ignore it
		return;
	}

	while ( 1 ) {
		token = COM_Parse( &text_p );
		if ( !token || !token[0] ) {
			break;
		}

		// this is a name
		anim = BG_AnimationForString( token, modelInfo );
		if ( anim->moveSpeed == 0 ) {
			G_Error( "G_RetrieveMoveSpeedsFromClient: trying to set movespeed for non-moving animation" );
		}

		// get the movespeed
		token = COM_Parse( &text_p );
		if ( !token || !token[0] ) {
			G_Error( "G_RetrieveMoveSpeedsFromClient: missing movespeed" );
		}
		anim->moveSpeed = atoi( token );

#if defined RTCW_SP
		// get the stepgap
		token = COM_Parse( &text_p );
		if ( !token || !token[0] ) {
			G_Error( "G_RetrieveMoveSpeedsFromClient: missing stepGap" );
		}
		anim->stepGap = atoi( token );
#endif RTCW_XX

	}
}
