/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "server.h"

#include "rtcw_endian.h"
#include "rtcw_vm_args.h"


/*
===============================================================================

OPERATOR CONSOLE ONLY COMMANDS

These commands can only be entered from stdin or by a remote operator datagram
===============================================================================
*/


/*
==================
SV_GetPlayerByName

Returns the player with name from Cmd_Argv(1)
==================
*/
static client_t *SV_GetPlayerByName( void ) {
	client_t    *cl;
	int i;
	const char        *s;
	char cleanName[64];

	// make sure server is running
	if ( !com_sv_running->integer ) {
		return NULL;
	}

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "No player specified.\n" );
		return NULL;
	}

	s = Cmd_Argv( 1 );

	// check for a name match
	for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {

#if !defined RTCW_ET
		if ( !cl->state ) {
#else
		if ( cl->state <= CS_ZOMBIE ) {
#endif // RTCW_XX

			continue;
		}
		if ( !Q_stricmp( cl->name, s ) ) {
			return cl;
		}

		Q_strncpyz( cleanName, cl->name, sizeof( cleanName ) );
		Q_CleanStr( cleanName );
		if ( !Q_stricmp( cleanName, s ) ) {
			return cl;
		}
	}

	Com_Printf( "Player %s is not on the server\n", s );

	return NULL;
}

/*
==================
SV_GetPlayerByNum

Returns the player with idnum from Cmd_Argv(1)
==================
*/
#if !defined RTCW_ET
static client_t *SV_GetPlayerByNum( void ) {
	client_t    *cl;
	int i;
	int idnum;
	const char        *s;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		return NULL;
	}

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "No player specified.\n" );
		return NULL;
	}

	s = Cmd_Argv( 1 );

	for ( i = 0; s[i]; i++ ) {
		if ( s[i] < '0' || s[i] > '9' ) {
			Com_Printf( "Bad slot number: %s\n", s );
			return NULL;
		}
	}
	idnum = atoi( s );
	if ( idnum < 0 || idnum >= sv_maxclients->integer ) {
		Com_Printf( "Bad client slot: %i\n", idnum );
		return NULL;
	}

	cl = &svs.clients[idnum];
	if ( !cl->state ) {
		Com_Printf( "Client %i is not active\n", idnum );
		return NULL;
	}
	return cl;

#ifdef RTCW_VANILLA
	return NULL;
#endif // RTCW_VANILLA
}
#else
// fretn unused
#if 0
static client_t *SV_GetPlayerByNum( void ) {
	client_t    *cl;
	int i;
	int idnum;
	char        *s;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		return NULL;
	}

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "No player specified.\n" );
		return NULL;
	}

	s = Cmd_Argv( 1 );

	for ( i = 0; s[i]; i++ ) {
		if ( s[i] < '0' || s[i] > '9' ) {
			Com_Printf( "Bad slot number: %s\n", s );
			return NULL;
		}
	}
	idnum = atoi( s );
	if ( idnum < 0 || idnum >= sv_maxclients->integer ) {
		Com_Printf( "Bad client slot: %i\n", idnum );
		return NULL;
	}

	cl = &svs.clients[idnum];
	if ( cl->state <= CS_ZOMBIE ) {
		Com_Printf( "Client %i is not active\n", idnum );
		return NULL;
	}
	return cl;

	return NULL;
}
#endif
#endif // RTCW_XX

//=========================================================


/*
==================
SV_Map_f

Restart the server on a different map
==================
*/
static void SV_Map_f( void ) {
	const char        *cmd;
	const char        *map;

#if !defined RTCW_MP
	char smapname[MAX_QPATH];
#endif // RTCW_XX

	char mapname[MAX_QPATH];

#if !defined RTCW_MP
	qboolean killBots, cheat, buildScript;
#else
	qboolean killBots, cheat;
#endif // RTCW_XX

	char expanded[MAX_QPATH];

#if !defined RTCW_MP
	int savegameTime = -1;
#else
	// TTimo: unused
//	int			savegameTime = -1;
#endif // RTCW_XX

#if defined RTCW_ET
	const char        *cl_profileStr = Cvar_VariableString( "cl_profile" );
#endif // RTCW_XX

	map = Cmd_Argv( 1 );
	if ( !map ) {
		return;
	}

#if defined RTCW_ET
	if ( !com_gameInfo.spEnabled ) {
		if ( !Q_stricmp( Cmd_Argv( 0 ), "spdevmap" ) || !Q_stricmp( Cmd_Argv( 0 ), "spmap" ) ) {
			Com_Printf( "Single Player is not enabled.\n" );
			return;
		}
	}
#endif // RTCW_XX

#if !defined RTCW_MP
	buildScript = Cvar_VariableIntegerValue( "com_buildScript" );

#if defined RTCW_ET
	if ( SV_GameIsSinglePlayer() ) {
#endif // RTCW_XX

	if ( !buildScript && sv_reloading->integer && sv_reloading->integer != RELOAD_NEXTMAP ) {  // game is in 'reload' mode, don't allow starting new maps yet.
		return;
	}

#if !defined RTCW_ET
	// Ridah: trap a savegame load
	if ( strstr( map, ".svg" ) ) {
#else
		// Trap a savegame load
		if ( strstr( map, ".sav" ) ) {
#endif // RTCW_XX

		// open the savegame, read the mapname, and copy it to the map string
		char savemap[MAX_QPATH];

#if defined RTCW_ET
			char savedir[MAX_QPATH];
#endif // RTCW_XX

		byte *buffer;
		int size, csize;

#if !defined RTCW_ET
		if ( !( strstr( map, "save/" ) == map ) ) {
			Com_sprintf( savemap, sizeof( savemap ), "save/%s", map );
#else
			if ( com_gameInfo.usesProfiles && cl_profileStr[0] ) {
				Com_sprintf( savedir, sizeof( savedir ), "profiles/%s/save/", cl_profileStr );
			} else {
				Q_strncpyz( savedir, "save/", sizeof( savedir ) );
			}

			if ( !( strstr( map, savedir ) == map ) ) {
				Com_sprintf( savemap, sizeof( savemap ), "%s%s", savedir, map );
#endif // RTCW_XX

		} else {
			strcpy( savemap, map );
		}

		size = FS_ReadFile( savemap, NULL );
		if ( size < 0 ) {
			Com_Printf( "Can't find savegame %s\n", savemap );
			return;
		}

		//buffer = Hunk_AllocateTempMemory(size);
		FS_ReadFile( savemap, (void **)&buffer );

#if !defined RTCW_ET
		if ( Q_stricmp( savemap, "save/current.svg" ) != 0 ) {
#else
			if ( Q_stricmp( savemap, va( "%scurrent.sav", savedir ) ) != 0 ) {
#endif // RTCW_XX

			// copy it to the current savegame file

#if !defined RTCW_ET
			FS_WriteFile( "save/current.svg", buffer, size );
#else
				FS_WriteFile( va( "%scurrent.sav", savedir ), buffer, size );
#endif // RTCW_XX

			// make sure it is the correct size

#if !defined RTCW_ET
			csize = FS_ReadFile( "save/current.svg", NULL );
#else
				csize = FS_ReadFile( va( "%scurrent.sav", savedir ), NULL );
#endif // RTCW_XX

			if ( csize != size ) {
				Hunk_FreeTempMemory( buffer );

#if !defined RTCW_ET
				FS_Delete( "save/current.svg" );
#else
					FS_Delete( va( "%scurrent.sav", savedir ) );
#endif // RTCW_XX

				Com_Error( ERR_DROP, "Unable to save game." );
				return;
			}
		}

		// set the cvar, so the game knows it needs to load the savegame once the clients have connected
		Cvar_Set( "savegame_loading", "1" );
		// set the filename
		Cvar_Set( "savegame_filename", savemap );

		// the mapname is at the very start of the savegame file
		Com_sprintf( savemap, sizeof( savemap ), ( char * )( buffer + sizeof( int ) ) );  // skip the version
		Q_strncpyz( smapname, savemap, sizeof( smapname ) );
		map = smapname;

		savegameTime = *( int * )( buffer + sizeof( int ) + MAX_QPATH );

		if ( savegameTime >= 0 ) {
			svs.time = savegameTime;
		}

		Hunk_FreeTempMemory( buffer );
	} else {
		Cvar_Set( "savegame_loading", "0" );  // make sure it's turned off
		// set the filename
		Cvar_Set( "savegame_filename", "" );
	}

#if defined RTCW_ET
	} else {
		Cvar_Set( "savegame_loading", "0" );  // make sure it's turned off
		// set the filename
		Cvar_Set( "savegame_filename", "" );
	}
#endif // RTCW_XX

	// done.
#endif // RTCW_XX

	// make sure the level exists before trying to change, so that
	// a typo at the server console won't end the game
	Com_sprintf( expanded, sizeof( expanded ), "maps/%s.bsp", map );
	if ( FS_ReadFile( expanded, NULL ) == -1 ) {
		Com_Printf( "Can't find map %s\n", expanded );
		return;
	}

#if defined RTCW_SP
	Cvar_Set( "r_mapFogColor", "0" );       //----(SA)	added
	Cvar_Set( "r_waterFogColor", "0" );     //----(SA)	added
	Cvar_Set( "r_savegameFogColor", "0" );      //----(SA)	added

	// force latched values to get set
	Cvar_Get( "g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH );

	// Rafael gameskill
	Cvar_Get( "g_gameskill", "1", CVAR_SERVERINFO | CVAR_LATCH );
	// done

	Cvar_SetValue( "g_episode", 0 ); //----(SA) added
#else
	Cvar_Set( "gamestate", va( "%i", GS_INITIALIZE ) );       // NERVE - SMF - reset gamestate on map/devmap

#if !defined RTCW_ET
	Cvar_Set( "savegame_loading", "0" );  // make sure it's turned off
#endif // RTCW_XX

	Cvar_Set( "g_currentRound", "0" );            // NERVE - SMF - reset the current round
	Cvar_Set( "g_nextTimeLimit", "0" );           // NERVE - SMF - reset the next time limit

#if !defined RTCW_ET
	// force latched values to get set
	// DHM - Nerve :: default to GT_WOLF
	Cvar_Get( "g_gametype", "5", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH );
#endif // RTCW_XX

#if defined RTCW_ET
	// START	Mad Doctor I changes, 8/14/2002.  Need a way to force load a single player map as single player
	if ( !Q_stricmp( Cmd_Argv( 0 ), "spdevmap" ) || !Q_stricmp( Cmd_Argv( 0 ), "spmap" ) ) {
		// This is explicitly asking for a single player load of this map
		Cvar_Set( "g_gametype", va( "%i", com_gameInfo.defaultSPGameType ) );
		// force latched values to get set
		Cvar_Get( "g_gametype", va( "%i", com_gameInfo.defaultSPGameType ), CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH );
		// enable bot support for AI
		Cvar_Set( "bot_enable", "1" );
	}
#endif // RTCW_XX

	// Rafael gameskill

#if !defined RTCW_ET
	Cvar_Get( "g_gameskill", "3", CVAR_SERVERINFO | CVAR_LATCH );
#else
//	Cvar_Get ("g_gameskill", "3", CVAR_SERVERINFO | CVAR_LATCH);
#endif // RTCW_XX

	// done
#endif // RTCW_XX

	cmd = Cmd_Argv( 0 );

#if !defined RTCW_ET
	if ( Q_stricmpn( cmd, "sp", 2 ) == 0 ) {
		Cvar_SetValue( "g_gametype", GT_SINGLE_PLAYER );
		Cvar_SetValue( "g_doWarmup", 0 );
		// may not set sv_maxclients directly, always set latched
#endif // RTCW_XX

#if defined RTCW_SP || defined RTCW_MP
		Cvar_SetLatched( "sv_maxclients", "32" ); // Ridah, modified this
#endif // RTCW_XX

#if !defined RTCW_ET
		cmd += 2;
		killBots = qtrue;
		if ( !Q_stricmp( cmd, "devmap" ) ) {
			cheat = qtrue;
		} else {
			cheat = qfalse;
		}
	} else {
#endif // RTCW_XX

		if ( !Q_stricmp( cmd, "devmap" ) ) {
			cheat = qtrue;
			killBots = qtrue;

#if !defined RTCW_ET
		} else {
#else
	} else
#endif // RTCW_XX

#if defined RTCW_ET
	if ( !Q_stricmp( Cmd_Argv( 0 ), "spdevmap" ) ) {
		cheat = qtrue;
		killBots = qtrue;
	} else
	{
#endif // RTCW_XX

			cheat = qfalse;
			killBots = qfalse;
		}

#if !defined RTCW_ET
		if ( sv_gametype->integer == GT_SINGLE_PLAYER ) {
			Cvar_SetValue( "g_gametype", GT_FFA );
		}
	}
#endif // RTCW_XX


	// save the map name here cause on a map restart we reload the q3config.cfg
	// and thus nuke the arguments of the map command
	Q_strncpyz( mapname, map, sizeof( mapname ) );

	// start up the map
	SV_SpawnServer( mapname, killBots );

	// set the cheat value
	// if the level was started with "map <levelname>", then
	// cheats will not be allowed.  If started with "devmap <levelname>"
	// then cheats will be allowed
	if ( cheat ) {
		Cvar_Set( "sv_cheats", "1" );
	} else {
		Cvar_Set( "sv_cheats", "0" );
	}

}

#if !defined RTCW_SP
/*
================
SV_CheckTransitionGameState

NERVE - SMF
================
*/
static qboolean SV_CheckTransitionGameState( gamestate_t new_gs, gamestate_t old_gs ) {
	if ( old_gs == new_gs && new_gs != GS_PLAYING ) {
		return qfalse;
	}

//	if ( old_gs == GS_WARMUP && new_gs != GS_WARMUP_COUNTDOWN )
//		return qfalse;

//	if ( old_gs == GS_WARMUP_COUNTDOWN && new_gs != GS_PLAYING )
//		return qfalse;

	if ( old_gs == GS_WAITING_FOR_PLAYERS && new_gs != GS_WARMUP ) {
		return qfalse;
	}

	if ( old_gs == GS_INTERMISSION && new_gs != GS_WARMUP ) {
		return qfalse;
	}

	if ( old_gs == GS_RESET && ( new_gs != GS_WAITING_FOR_PLAYERS && new_gs != GS_WARMUP ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
================
SV_TransitionGameState

NERVE - SMF
================
*/
static qboolean SV_TransitionGameState( gamestate_t new_gs, gamestate_t old_gs, int delay ) {

#if defined RTCW_ET
	if ( !SV_GameIsSinglePlayer() && !SV_GameIsCoop() ) {
#endif // RTCW_XX

	// we always do a warmup before starting match
	if ( old_gs == GS_INTERMISSION && new_gs == GS_PLAYING ) {
		new_gs = GS_WARMUP;
	}

#if defined RTCW_ET
	}
#endif // RTCW_XX


	// check if its a valid state transition
	if ( !SV_CheckTransitionGameState( new_gs, old_gs ) ) {
		return qfalse;
	}

	if ( new_gs == GS_RESET ) {

#if !defined RTCW_ET
		if ( atoi( Cvar_VariableString( "g_noTeamSwitching" ) ) ) {
			new_gs = GS_WAITING_FOR_PLAYERS;
		} else {
#endif // RTCW_XX

			new_gs = GS_WARMUP;

#if !defined RTCW_ET
		}
#endif // RTCW_XX

	}

	Cvar_Set( "gamestate", va( "%i", new_gs ) );

	return qtrue;
}
#endif // RTCW_XX

#if defined RTCW_ET
void MSG_PrioritiseEntitystateFields( void );
void MSG_PrioritisePlayerStateFields( void );

static void SV_FieldInfo_f( void ) {
	MSG_PrioritiseEntitystateFields();
	MSG_PrioritisePlayerStateFields();
}
#endif // RTCW_XX

/*
================
SV_MapRestart_f

Completely restarts a level, but doesn't send a new gamestate to the clients.
This allows fair starts with variable load times.
================
*/
static void SV_MapRestart_f( void ) {
	int i;
	client_t    *client;
	char        *denied;
	qboolean isBot;

#if defined RTCW_SP
	int delay;
#else
	int delay = 0;
	gamestate_t new_gs, old_gs;     // NERVE - SMF

#if !defined RTCW_ET
	int worldspawnflags;            // DHM - Nerve
	int nextgt;                     // DHM - Nerve
	sharedEntity_t  *world;
#endif // RTCW_XX

#endif // RTCW_XX

	// make sure we aren't restarting twice in the same frame
	if ( com_frameTime == sv.serverId ) {
		return;
	}

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

#if !defined RTCW_ET
	if ( sv.restartTime ) {
		return;
	}
#else
	// ydnar: allow multiple delayed server restarts [atvi bug 3813]
	//%	if ( sv.restartTime ) {
	//%		return;
	//%	}
#endif // RTCW_XX

#if defined RTCW_SP
	if ( Cmd_Argc() > 1 ) {
		delay = atoi( Cmd_Argv( 1 ) );
	} else {
		if ( sv_gametype->integer == GT_SINGLE_PLAYER ) { // (SA) no pause by default in sp
			delay = 0;
		} else {
			delay = 5;
		}
	}
	if ( delay && !Cvar_VariableValue( "g_doWarmup" ) ) {
		sv.restartTime = svs.time + delay * 1000;
		SV_SetConfigstring( CS_WARMUP, va( "%i", sv.restartTime ) );
		return;
	}
#else

#if !defined RTCW_ET
	// DHM - Nerve :: Check for invalid gametype
	sv_gametype = Cvar_Get( "g_gametype", "5", CVAR_SERVERINFO | CVAR_LATCH );
	nextgt = sv_gametype->integer;

	world = SV_GentityNum( ENTITYNUM_WORLD );
	worldspawnflags = world->r.worldflags;
	if  (
		( nextgt == GT_WOLF && ( worldspawnflags & 1 ) ) ||
		( nextgt == GT_WOLF_STOPWATCH && ( worldspawnflags & 2 ) ) ||
		( ( nextgt == GT_WOLF_CP || nextgt == GT_WOLF_CPH ) && ( worldspawnflags & 4 ) )
		) {

		if ( !( worldspawnflags & 1 ) ) {
			Cvar_Set( "g_gametype", "5" );
		} else {
			Cvar_Set( "g_gametype", "7" );
		}

		sv_gametype = Cvar_Get( "g_gametype", "5", CVAR_SERVERINFO | CVAR_LATCH );
	}
	// dhm
#endif // RTCW_XX

	if ( Cmd_Argc() > 1 ) {
		delay = atoi( Cmd_Argv( 1 ) );
	}

	if ( delay ) {
		sv.restartTime = svs.time + delay * 1000;
		SV_SetConfigstring( CS_WARMUP, va( "%i", sv.restartTime ) );
		return;
	}

	// NERVE - SMF - read in gamestate or just default to GS_PLAYING
	old_gs = gamestate_t (atoi( Cvar_VariableString( "gamestate" ) ));

#if defined RTCW_ET
	if ( SV_GameIsSinglePlayer() || SV_GameIsCoop() ) {
		new_gs = GS_PLAYING;
	} else {
#endif // RTCW_XX

	if ( Cmd_Argc() > 2 ) {
		new_gs = gamestate_t (atoi( Cmd_Argv( 2 ) ));
	} else {
		new_gs = GS_PLAYING;
	}

#if defined RTCW_ET
	}
#endif // RTCW_XX

	if ( !SV_TransitionGameState( new_gs, old_gs, delay ) ) {
		return;
	}
#endif // RTCW_XX

	// check for changes in variables that can't just be restarted
	// check for maxclients change

#if defined RTCW_SP
	if ( sv_maxclients->modified || sv_gametype->modified ) {
#else
	if ( sv_maxclients->modified ) {
#endif // RTCW_XX

		char mapname[MAX_QPATH];

#if defined RTCW_SP
		Com_Printf( "variable change -- restarting.\n" );
#else
		Com_Printf( "sv_maxclients variable change -- restarting.\n" );
#endif // RTCW_XX

		// restart the map the slow way
		Q_strncpyz( mapname, Cvar_VariableString( "mapname" ), sizeof( mapname ) );

		SV_SpawnServer( mapname, qfalse );
		return;
	}

#if !defined RTCW_MP

#if !defined RTCW_ET
	// Ridah, check for loading a saved game
#else
	// Check for loading a saved game
#endif // RTCW_XX

	if ( Cvar_VariableIntegerValue( "savegame_loading" ) ) {
		// open the current savegame, and find out what the time is, everything else we can ignore

#if !defined RTCW_ET
		const char *savemap = "save/current.svg";
#else
		char savemap[MAX_QPATH];
#endif // RTCW_XX

		byte *buffer;
		int size, savegameTime;

#if defined RTCW_ET
		const char *cl_profileStr = Cvar_VariableString( "cl_profile" );

		if ( com_gameInfo.usesProfiles ) {
			Com_sprintf( savemap, sizeof( savemap ), "profiles/%s/save/current.sav", cl_profileStr );
		} else {
			Q_strncpyz( savemap, "save/current.sav", sizeof( savemap ) );
		}
#endif // RTCW_XX


		size = FS_ReadFile( savemap, NULL );
		if ( size < 0 ) {
			Com_Printf( "Can't find savegame %s\n", savemap );
			return;
		}

		//buffer = Hunk_AllocateTempMemory(size);
		FS_ReadFile( savemap, (void **)&buffer );

		// the mapname is at the very start of the savegame file
		savegameTime = *( int * )( buffer + sizeof( int ) + MAX_QPATH );

		if ( savegameTime >= 0 ) {
			svs.time = savegameTime;
		}

		Hunk_FreeTempMemory( buffer );
	}
	// done.
#endif // RTCW_XX

	// toggle the server bit so clients can detect that a
	// map_restart has happened
	svs.snapFlagServerBit ^= SNAPFLAG_SERVERCOUNT;

	// generate a new serverid

#if defined RTCW_SP
	sv.restartedServerId = sv.serverId;
#else
	// TTimo - don't update restartedserverId there, otherwise we won't deal correctly with multiple map_restart
#endif // RTCW_XX

	sv.serverId = com_frameTime;
	Cvar_Set( "sv_serverid", va( "%i", sv.serverId ) );

	// reset all the vm data in place without changing memory allocation
	// note that we do NOT set sv.state = SS_LOADING, so configstrings that
	// had been changed from their default values will generate broadcast updates
	sv.state = SS_LOADING;
	sv.restarting = qtrue;

#if !defined RTCW_SP
	Cvar_Set( "sv_serverRestarting", "1" );
#endif // RTCW_XX

	SV_RestartGameProgs();

	// run a few frames to allow everything to settle

#if !defined RTCW_ET
	for ( i = 0 ; i < 3 ; i++ ) {
		VM_Call(gvm, GAME_RUN_FRAME, rtcw::to_vm_arg(svs.time));
		svs.time += 100;
	}
#else
	for ( i = 0; i < GAME_INIT_FRAMES; i++ ) {
		VM_Call(gvm, GAME_RUN_FRAME, rtcw::to_vm_arg(svs.time));
		svs.time += FRAMETIME;
	}
#endif // RTCW_XX

#if defined RTCW_ET
	// create a baseline for more efficient communications
	// Gordon: meh, this wont work here as the client doesn't know it has happened
//	SV_CreateBaseline ();
#endif // RTCW_XX

	sv.state = SS_GAME;
	sv.restarting = qfalse;

	// connect and begin all the clients
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		client = &svs.clients[i];

		// send the new gamestate to all connected clients
		if ( client->state < CS_CONNECTED ) {
			continue;
		}

		if ( client->netchan.remoteAddress.type == NA_BOT ) {

#if defined RTCW_ET
			if ( SV_GameIsSinglePlayer() || SV_GameIsCoop() ) {
				continue;   // dont carry across bots in single player
			}
#endif // RTCW_XX

			isBot = qtrue;
		} else {
			isBot = qfalse;
		}

		// add the map_restart command
		SV_AddServerCommand( client, "map_restart\n" );

		// connect the client again, without the firstTime flag
		denied = rtcw::from_vm_arg<char*>(VM_ExplicitArgPtr(
			gvm,
			VM_Call(
				gvm,
				GAME_CLIENT_CONNECT,
				rtcw::to_vm_arg(i),
				rtcw::to_vm_arg(qfalse),
				rtcw::to_vm_arg(isBot)
			)
		));

		if ( denied ) {
			// this generally shouldn't happen, because the client
			// was connected before the level change
			SV_DropClient( client, denied );

#if defined RTCW_ET
			if ( ( !SV_GameIsSinglePlayer() ) || ( !isBot ) ) {
#endif // RTCW_XX

			Com_Printf( "SV_MapRestart_f(%d): dropped client %i - denied!\n", delay, i ); // bk010125

#if defined RTCW_ET
			}
#endif // RTCW_XX

			continue;
		}

		client->state = CS_ACTIVE;

		SV_ClientEnterWorld( client, &client->lastUsercmd );
	}

	// run another frame to allow things to look at all the players
	VM_Call(gvm, GAME_RUN_FRAME, rtcw::to_vm_arg(svs.time));

#if !defined RTCW_ET
	svs.time += 100;
#else
	svs.time += FRAMETIME;
#endif // RTCW_XX

#if !defined RTCW_SP
	Cvar_Set( "sv_serverRestarting", "0" );
#endif // RTCW_XX

}

/*
=================
SV_LoadGame_f
=================
*/
void    SV_LoadGame_f( void ) {

#if !defined RTCW_ET
	char filename[MAX_QPATH], mapname[MAX_QPATH];
#else
	char filename[MAX_QPATH], mapname[MAX_QPATH], savedir[MAX_QPATH];
#endif // RTCW_XX

	byte *buffer;
	int size;

#if defined RTCW_ET
	const char *cl_profileStr = Cvar_VariableString( "cl_profile" );
#endif // RTCW_XX

#if !defined RTCW_MP
	// dont allow command if another loadgame is pending
	if ( Cvar_VariableIntegerValue( "savegame_loading" ) ) {
		return;
	}
	if ( sv_reloading->integer ) {
		// (SA) disabling
//	if(sv_reloading->integer && sv_reloading->integer != RELOAD_FAILED )	// game is in 'reload' mode, don't allow starting new maps yet.
		return;
	}
#endif // RTCW_XX

	Q_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	if ( !filename[0] ) {
		Com_Printf( "You must specify a savegame to load\n" );
		return;
	}

#if !defined RTCW_ET
	if ( Q_strncmp( filename, "save/", 5 ) && Q_strncmp( filename, "save\\", 5 ) ) {
		Q_strncpyz( filename, va( "save/%s", filename ), sizeof( filename ) );
	}
#else
	if ( com_gameInfo.usesProfiles && cl_profileStr[0] ) {
		Com_sprintf( savedir, sizeof( savedir ), "profiles/%s/save/", cl_profileStr );
	} else {
		Q_strncpyz( savedir, "save/", sizeof( savedir ) );
	}

	/*if ( Q_strncmp( filename, "save/", 5 ) && Q_strncmp( filename, "save\\", 5 ) ) {
		Q_strncpyz( filename, va("save/%s", filename), sizeof( filename ) );
	}*/

	// go through a va to avoid vsnprintf call with same source and target
	Q_strncpyz( filename, va( "%s%s", savedir, filename ), sizeof( filename ) );

	// enforce .sav extension
	if ( !strstr( filename, "." ) || Q_strncmp( strstr( filename, "." ) + 1, "sav", 3 ) ) {
		Q_strcat( filename, sizeof( filename ), ".sav" );
	}
	// use '/' instead of '\\' for directories
#endif // RTCW_XX

#if defined RTCW_SP
	// enforce .svg extension
	if ( !strstr( filename, "." ) || Q_strncmp( strstr( filename, "." ) + 1, "svg", 3 ) ) {
#elif defined RTCW_MP
	if ( !strstr( filename, ".svg" ) ) {
#endif // RTCW_XX

#if !defined RTCW_ET
		Q_strcat( filename, sizeof( filename ), ".svg" );
	}
#endif // RTCW_XX

#if !defined RTCW_MP
	// FIXME use '/' instead of '\' for directories
	while ( strstr( filename, "\\" ) ) {
		*(char *)strstr( filename, "\\" ) = '/';
	}
#endif // RTCW_XX

	size = FS_ReadFile( filename, NULL );
	if ( size < 0 ) {
		Com_Printf( "Can't find savegame %s\n", filename );
		return;
	}

#if !defined RTCW_MP
	//buffer = Hunk_AllocateTempMemory(size);
#else
	buffer = static_cast<byte*> (Hunk_AllocateTempMemory( size ));
#endif // RTCW_XX

	FS_ReadFile( filename, (void **)&buffer );

	// read the mapname, if it is the same as the current map, then do a fast load

#if !defined RTCW_MP
	Com_sprintf( mapname, sizeof( mapname ), (const char*)( buffer + sizeof( int ) ) );
#else
	Com_sprintf( mapname, sizeof( mapname ), reinterpret_cast<const char*> (buffer + sizeof( int )) );
#endif // RTCW_XX

	if ( com_sv_running->integer && ( com_frameTime != sv.serverId ) ) {
		// check mapname
		if ( !Q_stricmp( mapname, sv_mapname->string ) ) {    // same

#if !defined RTCW_ET
			if ( Q_stricmp( filename, "save/current.svg" ) != 0 ) {
#else
			if ( Q_stricmp( filename, va( "%scurrent.sav",savedir ) ) != 0 ) {
#endif // RTCW_XX

				// copy it to the current savegame file

#if !defined RTCW_ET
				FS_WriteFile( "save/current.svg", buffer, size );
#else
				FS_WriteFile( va( "%scurrent.sav",savedir ), buffer, size );
#endif // RTCW_XX

			}

			Hunk_FreeTempMemory( buffer );

			Cvar_Set( "savegame_loading", "2" );  // 2 means it's a restart, so stop rendering until we are loaded

#if !defined RTCW_MP
			// set the filename
			Cvar_Set( "savegame_filename", filename );
			// quick-restart the server
#endif // RTCW_XX

			SV_MapRestart_f();  // savegame will be loaded after restart

			return;
		}
	}

	Hunk_FreeTempMemory( buffer );

	// otherwise, do a slow load
	if ( Cvar_VariableIntegerValue( "sv_cheats" ) ) {
		Cbuf_ExecuteText( EXEC_APPEND, va( "spdevmap %s", filename ) );
	} else {    // no cheats
		Cbuf_ExecuteText( EXEC_APPEND, va( "spmap %s", filename ) );
	}
}

//===============================================================

#if !defined RTCW_ET
/*
==================
SV_Kick_f

Kick a user off of the server  FIXME: move to game
==================
*/
static void SV_Kick_f( void ) {
	client_t    *cl;
	int i;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
//		Com_Printf ("Usage: kick <player name>\nkick all = kick everyone\nkick allbots = kick all bots\n");
		Com_Printf( "Usage: kick <player name>\nkick all = kick everyone\n" );
		return;
	}

	cl = SV_GetPlayerByName();
	if ( !cl ) {
		if ( !Q_stricmp( Cmd_Argv( 1 ), "all" ) ) {
			for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
				if ( !cl->state ) {
					continue;
				}
				if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
					continue;
				}

#if defined RTCW_SP
				SV_DropClient( cl, "was kicked" );
#elif defined RTCW_MP
				SV_DropClient( cl, "player kicked" ); // JPW NERVE to match front menu message
#endif // RTCW_XX

				cl->lastPacketTime = svs.time;  // in case there is a funny zombie
			}
		} else if ( !Q_stricmp( Cmd_Argv( 1 ), "allbots" ) )        {
			for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
				if ( !cl->state ) {
					continue;
				}
				if ( cl->netchan.remoteAddress.type != NA_BOT ) {
					continue;
				}
				SV_DropClient( cl, "was kicked" );
				cl->lastPacketTime = svs.time;  // in case there is a funny zombie
			}
		}
		return;
	}
	if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand( NULL, "print \"%s\"", "Cannot kick host player\n" );
		return;
	}

#if defined RTCW_SP
	SV_DropClient( cl, "was kicked" );
#elif defined RTCW_MP
	SV_DropClient( cl, "player kicked" ); // JPW NERVE to match front menu message
#endif // RTCW_XX

	cl->lastPacketTime = svs.time;  // in case there is a funny zombie
}
#else
/*
==================
SV_Kick_f

Kick a user off of the server  FIXME: move to game
// fretn: done
==================
*/
/*
static void SV_Kick_f( void ) {
	client_t	*cl;
	int			i;
	int			timeout = -1;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() < 2 || Cmd_Argc() > 3 ) {
		Com_Printf ("Usage: kick <player name> [timeout]\n");
		return;
	}

	if( Cmd_Argc() == 3 ) {
		timeout = atoi( Cmd_Argv( 2 ) );
	} else {
		timeout = 300;
	}

	cl = SV_GetPlayerByName();
	if ( !cl ) {
		if ( !Q_stricmp(Cmd_Argv(1), "all") ) {
			for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ ) {
				if ( !cl->state ) {
					continue;
				}
				if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
					continue;
				}
				SV_DropClient( cl, "player kicked" ); // JPW NERVE to match front menu message
				if( timeout != -1 ) {
					SV_TempBanNetAddress( cl->netchan.remoteAddress, timeout );
				}
				cl->lastPacketTime = svs.time;	// in case there is a funny zombie
			}
		} else if ( !Q_stricmp(Cmd_Argv(1), "allbots") ) {
			for ( i=0, cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
				if ( !cl->state ) {
					continue;
				}
				if( cl->netchan.remoteAddress.type != NA_BOT ) {
					continue;
				}
				SV_DropClient( cl, "was kicked" );
				if( timeout != -1 ) {
					SV_TempBanNetAddress( cl->netchan.remoteAddress, timeout );
				}
				cl->lastPacketTime = svs.time;	// in case there is a funny zombie
			}
		}
		return;
	}
	if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand(NULL, "print \"%s\"", "Cannot kick host player\n");
		return;
	}

	SV_DropClient( cl, "player kicked" ); // JPW NERVE to match front menu message
	if( timeout != -1 ) {
		SV_TempBanNetAddress( cl->netchan.remoteAddress, timeout );
	}
	cl->lastPacketTime = svs.time;	// in case there is a funny zombie
}
*/
#endif // RTCW_XX

#if !defined RTCW_ET || (defined RTCW_ET && defined AUTHORIZE_SUPPORT)
/*
==================
SV_Ban_f

Ban a user from being able to play on this server through the auth
server
==================
*/
static void SV_Ban_f( void ) {
	client_t    *cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: banUser <player name>\n" );
		return;
	}

	cl = SV_GetPlayerByName();

	if ( !cl ) {
		return;
	}

	if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand( NULL, "print \"%s\"", "Cannot kick host player\n" );
		return;
	}

	// look up the authorize server's IP
	if ( !svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress ) ) {
			Com_Printf( "Couldn't resolve address\n" );
			return;
		}
		svs.authorizeAddress.port = rtcw::Endian::be( PORT_AUTHORIZE );
		Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
					svs.authorizeAddress.ip[0], svs.authorizeAddress.ip[1],
					svs.authorizeAddress.ip[2], svs.authorizeAddress.ip[3],
					rtcw::Endian::be( svs.authorizeAddress.port ) );
	}

	// otherwise send their ip to the authorize server
	if ( svs.authorizeAddress.type != NA_BAD ) {
		NET_OutOfBandPrint( NS_SERVER, svs.authorizeAddress,
							"banUser %i.%i.%i.%i", cl->netchan.remoteAddress.ip[0], cl->netchan.remoteAddress.ip[1],
							cl->netchan.remoteAddress.ip[2], cl->netchan.remoteAddress.ip[3] );
		Com_Printf( "%s was banned from coming back\n", cl->name );
	}
}

/*
==================
SV_BanNum_f

Ban a user from being able to play on this server through the auth
server
==================
*/
static void SV_BanNum_f( void ) {
	client_t    *cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: banClient <client number>\n" );
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}
	if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand( NULL, "print \"%s\"", "Cannot kick host player\n" );
		return;
	}

	// look up the authorize server's IP
	if ( !svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress ) ) {
			Com_Printf( "Couldn't resolve address\n" );
			return;
		}
		svs.authorizeAddress.port = rtcw::Endian::be( PORT_AUTHORIZE );
		Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
					svs.authorizeAddress.ip[0], svs.authorizeAddress.ip[1],
					svs.authorizeAddress.ip[2], svs.authorizeAddress.ip[3],
					rtcw::Endian::be( svs.authorizeAddress.port ) );
	}

	// otherwise send their ip to the authorize server
	if ( svs.authorizeAddress.type != NA_BAD ) {
		NET_OutOfBandPrint( NS_SERVER, svs.authorizeAddress,
							"banUser %i.%i.%i.%i", cl->netchan.remoteAddress.ip[0], cl->netchan.remoteAddress.ip[1],
							cl->netchan.remoteAddress.ip[2], cl->netchan.remoteAddress.ip[3] );
		Com_Printf( "%s was banned from coming back\n", cl->name );
	}
}
#endif // RTCW_XX

#if defined RTCW_ET
/*
==================
==================
*/
void SV_TempBanNetAddress( netadr_t address, int length ) {
	int i;
	int oldesttime = 0;
	int oldest = -1;

	for ( i = 0; i < MAX_TEMPBAN_ADDRESSES; i++ ) {
		if ( !svs.tempBanAddresses[ i ].endtime || svs.tempBanAddresses[ i ].endtime < svs.time ) {
			// found a free slot
			svs.tempBanAddresses[ i ].adr       = address;
			svs.tempBanAddresses[ i ].endtime   = svs.time + ( length * 1000 );

			return;
		} else {
			if ( oldest == -1 || oldesttime > svs.tempBanAddresses[ i ].endtime ) {
				oldesttime  = svs.tempBanAddresses[ i ].endtime;
				oldest      = i;
			}
		}
	}

	svs.tempBanAddresses[ oldest ].adr      = address;
	svs.tempBanAddresses[ oldest ].endtime  = svs.time + length;
}

qboolean SV_TempBanIsBanned( netadr_t address ) {
	int i;

	for ( i = 0; i < MAX_TEMPBAN_ADDRESSES; i++ ) {
		if ( svs.tempBanAddresses[ i ].endtime && svs.tempBanAddresses[ i ].endtime > svs.time ) {
			if ( NET_CompareAdr( address, svs.tempBanAddresses[ i ].adr ) ) {
				return qtrue;
			}
		}
	}

	return qfalse;
}
#endif // RTCW_XX

#if !defined RTCW_ET
/*
==================
SV_KickNum_f

Kick a user off of the server  FIXME: move to game
==================
*/
static void SV_KickNum_f( void ) {
	client_t    *cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: kicknum <client number>\n" );
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}
	if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand( NULL, "print \"%s\"", "Cannot kick host player\n" );
		return;
	}

#if defined RTCW_SP
	SV_DropClient( cl, "was kicked" );
#elif defined RTCW_MP
	SV_DropClient( cl, "player kicked" );
#endif // RTCW_XX

	cl->lastPacketTime = svs.time;  // in case there is a funny zombie
}
#else
/*
==================
SV_KickNum_f

Kick a user off of the server  FIXME: move to game
*DONE*
==================
*/
/*
static void SV_KickNum_f( void ) {
	client_t	*cl;
	int timeout = -1;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() < 2 || Cmd_Argc() > 3 ) {
		Com_Printf ("Usage: kicknum <client number> [timeout]\n");
		return;
	}

	if( Cmd_Argc() == 3 ) {
		timeout = atoi( Cmd_Argv( 2 ) );
	} else {
		timeout = 300;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}
	if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand(NULL, "print \"%s\"", "Cannot kick host player\n");
		return;
	}

	SV_DropClient( cl, "player kicked" );
	if( timeout != -1 ) {
		SV_TempBanNetAddress( cl->netchan.remoteAddress, timeout );
	}
	cl->lastPacketTime = svs.time;	// in case there is a funny zombie
}
*/
#endif // RTCW_XX

/*
================
SV_Status_f
================
*/
static void SV_Status_f( void ) {
	int i, j, l;
	client_t    *cl;
	playerState_t   *ps;
	const char      *s;
	int ping;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	Com_Printf( "map: %s\n", sv_mapname->string );

	Com_Printf( "num score ping name            lastmsg address               qport rate\n" );
	Com_Printf( "--- ----- ---- --------------- ------- --------------------- ----- -----\n" );
	for ( i = 0,cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ )
	{
		if ( !cl->state ) {
			continue;
		}
		Com_Printf( "%3i ", i );
		ps = SV_GameClientNum( i );
		Com_Printf( "%5i ", ps->persistant[PERS_SCORE] );

		if ( cl->state == CS_CONNECTED ) {
			Com_Printf( "CNCT " );
		} else if ( cl->state == CS_ZOMBIE ) {
			Com_Printf( "ZMBI " );
		} else
		{
			ping = cl->ping < 9999 ? cl->ping : 9999;
			Com_Printf( "%4i ", ping );
		}

		Com_Printf( "%s", cl->name );
		l = 16 - strlen( cl->name );
		for ( j = 0 ; j < l ; j++ )
			Com_Printf( " " );

		Com_Printf( "%7i ", svs.time - cl->lastPacketTime );

		s = NET_AdrToString( cl->netchan.remoteAddress );
		Com_Printf( "%s", s );
		l = 22 - strlen( s );
		for ( j = 0 ; j < l ; j++ )
			Com_Printf( " " );

		Com_Printf( "%5i", cl->netchan.qport );

		Com_Printf( " %5i", cl->rate );

		Com_Printf( "\n" );
	}
	Com_Printf( "\n" );
}

/*
==================
SV_ConSay_f
==================
*/
static void SV_ConSay_f( void ) {
	char    *p;
	char text[1024];

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() < 2 ) {
		return;
	}

	strcpy( text, "console: " );
	p = Cmd_Args();

	if ( *p == '"' ) {
		p++;
		p[strlen( p ) - 1] = 0;
	}

	strcat( text, p );

#if !defined RTCW_ET
	SV_SendServerCommand( NULL, "chat \"%s\n\"", text );
#else
	SV_SendServerCommand( NULL, "chat \"%s\"", text );
#endif // RTCW_XX

}


/*
==================
SV_Heartbeat_f

Also called by SV_DropClient, SV_DirectConnect, and SV_SpawnServer
==================
*/
void SV_Heartbeat_f( void ) {
	svs.nextHeartbeatTime = -9999999;
}


/*
===========
SV_Serverinfo_f

Examine the serverinfo string
===========
*/
static void SV_Serverinfo_f( void ) {
	Com_Printf( "Server info settings:\n" );

#if !defined RTCW_ET
	Info_Print( Cvar_InfoString( CVAR_SERVERINFO ) );
#else
	Info_Print( Cvar_InfoString( CVAR_SERVERINFO | CVAR_SERVERINFO_NOUPDATE ) );
#endif // RTCW_XX

}


/*
===========
SV_Systeminfo_f

Examine or change the serverinfo string
===========
*/
static void SV_Systeminfo_f( void ) {
	Com_Printf( "System info settings:\n" );

#if !defined RTCW_ET
	Info_Print( Cvar_InfoString( CVAR_SYSTEMINFO ) );
#else
	Info_Print( Cvar_InfoString( CVAR_SERVERINFO | CVAR_SERVERINFO_NOUPDATE ) );
#endif // RTCW_XX

}


/*
===========
SV_DumpUser_f

Examine all a users info strings FIXME: move to game
===========
*/
static void SV_DumpUser_f( void ) {
	client_t    *cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: info <userid>\n" );
		return;
	}

	cl = SV_GetPlayerByName();
	if ( !cl ) {
		return;
	}

	Com_Printf( "userinfo\n" );
	Com_Printf( "--------\n" );
	Info_Print( cl->userinfo );
}


/*
=================
SV_KillServer
=================
*/
static void SV_KillServer_f( void ) {
	SV_Shutdown( "killserver" );
}

#if !defined RTCW_SP
/*
=================
SV_GameCompleteStatus_f

NERVE - SMF
=================
*/
void SV_GameCompleteStatus_f( void ) {
	SV_MasterGameCompleteStatus();
}
#endif // RTCW_XX

//===========================================================

/*
==================
SV_AddOperatorCommands
==================
*/
void SV_AddOperatorCommands( void ) {
	static qboolean initialized;

	if ( initialized ) {
		return;
	}
	initialized = qtrue;

	Cmd_AddCommand( "heartbeat", SV_Heartbeat_f );

#if !defined RTCW_ET
	Cmd_AddCommand( "kick", SV_Kick_f );
#else
// fretn - moved to qagame
	/*Cmd_AddCommand ("kick", SV_Kick_f);
	Cmd_AddCommand ("clientkick", SV_KickNum_f);*/
#endif // RTCW_XX

#if !defined RTCW_ET || (defined RTCW_ET && defined AUTHORIZE_SUPPORT)

#if defined RTCW_ET
	// Arnout: banning requires auth server
#endif // RTCW_XX

	Cmd_AddCommand( "banUser", SV_Ban_f );
	Cmd_AddCommand( "banClient", SV_BanNum_f );
#endif // RTCW_XX

#if !defined RTCW_ET
	Cmd_AddCommand( "clientkick", SV_KickNum_f );
#endif // RTCW_XX

	Cmd_AddCommand( "status", SV_Status_f );
	Cmd_AddCommand( "serverinfo", SV_Serverinfo_f );
	Cmd_AddCommand( "systeminfo", SV_Systeminfo_f );
	Cmd_AddCommand( "dumpuser", SV_DumpUser_f );
	Cmd_AddCommand( "map_restart", SV_MapRestart_f );

#if defined RTCW_ET
	Cmd_AddCommand( "fieldinfo", SV_FieldInfo_f );
#endif // RTCW_XX

	Cmd_AddCommand( "sectorlist", SV_SectorList_f );

#if defined RTCW_SP
	Cmd_AddCommand( "spmap", SV_Map_f );
#ifndef WOLF_SP_DEMO
	Cmd_AddCommand( "map", SV_Map_f );
	Cmd_AddCommand( "devmap", SV_Map_f );
	Cmd_AddCommand( "spdevmap", SV_Map_f );
#endif
#else
	Cmd_AddCommand( "map", SV_Map_f );
	Cmd_AddCommand( "gameCompleteStatus", SV_GameCompleteStatus_f );      // NERVE - SMF

#if (!defined RTCW_ET && !defined PRE_RELEASE_DEMO) || (defined RTCW_ET && !defined PRE_RELEASE_DEMO_NODEVMAP)
	Cmd_AddCommand( "devmap", SV_Map_f );
	Cmd_AddCommand( "spmap", SV_Map_f );
	Cmd_AddCommand( "spdevmap", SV_Map_f );
#endif // RTCW_XX

#endif // RTCW_XX

	Cmd_AddCommand( "loadgame", SV_LoadGame_f );
	Cmd_AddCommand( "killserver", SV_KillServer_f );
	if ( com_dedicated->integer ) {
		Cmd_AddCommand( "say", SV_ConSay_f );
	}
}

/*
==================
SV_RemoveOperatorCommands
==================
*/
void SV_RemoveOperatorCommands( void ) {
#if 0
	// removing these won't let the server start again
	Cmd_RemoveCommand( "heartbeat" );
	Cmd_RemoveCommand( "kick" );
	Cmd_RemoveCommand( "banUser" );
	Cmd_RemoveCommand( "banClient" );
	Cmd_RemoveCommand( "status" );
	Cmd_RemoveCommand( "serverinfo" );
	Cmd_RemoveCommand( "systeminfo" );
	Cmd_RemoveCommand( "dumpuser" );
	Cmd_RemoveCommand( "map_restart" );
	Cmd_RemoveCommand( "sectorlist" );
	Cmd_RemoveCommand( "say" );
#endif
}

