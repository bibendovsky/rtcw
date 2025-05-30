/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "server.h"

#include "rtcw_endian.h"
#include "rtcw_vm_args.h"


serverStatic_t svs;                 // persistant server info
server_t sv;                        // local server
vm_t            *gvm = NULL;                // game virtual machine // bk001212 init

#if defined RTCW_MP
#ifdef UPDATE_SERVER
versionMapping_t versionMap[MAX_UPDATE_VERSIONS];
int numVersions = 0;
#endif
#endif // RTCW_XX

cvar_t  *sv_fps;                // time rate for running non-clients
cvar_t  *sv_timeout;            // seconds without any message
cvar_t  *sv_zombietime;         // seconds to sink messages after disconnect
cvar_t  *sv_rconPassword;       // password for remote server commands
cvar_t  *sv_privatePassword;    // password for the privateClient slots
cvar_t  *sv_allowDownload;
cvar_t  *sv_maxclients;

cvar_t  *sv_privateClients;     // number of clients reserved for password
cvar_t  *sv_hostname;
cvar_t  *sv_master[MAX_MASTER_SERVERS];     // master server ip address
cvar_t  *sv_reconnectlimit;     // minimum seconds between connect messages

#if defined RTCW_ET
cvar_t  *sv_tempbanmessage;
#endif // RTCW_XX

cvar_t  *sv_showloss;           // report when usercmds are lost
cvar_t  *sv_padPackets;         // add nop bytes to messages
cvar_t  *sv_killserver;         // menu system can set to 1 to shut server down
cvar_t  *sv_mapname;
cvar_t  *sv_mapChecksum;
cvar_t  *sv_serverid;
cvar_t  *sv_maxRate;
cvar_t  *sv_minPing;
cvar_t  *sv_maxPing;

#if !defined RTCW_ET
cvar_t  *sv_gametype;
#else
//cvar_t	*sv_gametype;
#endif // RTCW_XX

cvar_t  *sv_pure;
cvar_t  *sv_floodProtect;
cvar_t  *sv_allowAnonymous;

#if !defined RTCW_SP
cvar_t  *sv_lanForceRate; // TTimo - dedicated 1 (LAN) server forces local client rates to 99999 (bug #491)
cvar_t  *sv_onlyVisibleClients; // DHM - Nerve
cvar_t  *sv_friendlyFire;       // NERVE - SMF
cvar_t  *sv_maxlives;           // NERVE - SMF

#if !defined RTCW_ET
cvar_t  *sv_tourney;            // NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
cvar_t  *sv_needpass;
#endif // RTCW_XX

cvar_t *sv_dl_maxRate;
#endif // RTCW_XX

#if defined RTCW_ET
cvar_t* g_gameType;
#endif // RTCW_XX

// Rafael gameskill

#if !defined RTCW_ET
cvar_t  *sv_gameskill;
#else
//cvar_t	*sv_gameskill;
#endif // RTCW_XX

// done

#if !defined RTCW_MP
cvar_t  *sv_reloading;  //----(SA)	added
#endif // RTCW_XX

#if !defined RTCW_SP
cvar_t  *sv_showAverageBPS;     // NERVE - SMF - net debugging

#if defined RTCW_ET
cvar_t  *sv_wwwDownload; // server does a www dl redirect
cvar_t  *sv_wwwBaseURL; // base URL for redirect
// tell clients to perform their downloads while disconnected from the server
// this gets you a better throughput, but you loose the ability to control the download usage
cvar_t *sv_wwwDlDisconnected;
cvar_t *sv_wwwFallbackURL; // URL to send to if an http/ftp fails or is refused client side

//bani
cvar_t  *sv_cheats;
cvar_t  *sv_packetloss;
cvar_t  *sv_packetdelay;

// fretn
cvar_t  *sv_fullmsg;
#endif // RTCW_XX

void SVC_GameCompleteStatus( netadr_t from );       // NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
#define LL( x ) x = rtcw::Endian::le(x)
#endif // RTCW_XX

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*
===============
SV_ExpandNewlines

Converts newlines to "\n" so a line prints nicer
===============
*/
char    *SV_ExpandNewlines( char *in ) {
	static char string[1024];
	int l;

	l = 0;
	while ( *in && l < sizeof( string ) - 3 ) {
		if ( *in == '\n' ) {
			string[l++] = '\\';
			string[l++] = 'n';
		} else {

#if !defined RTCW_SP
			// NERVE - SMF - HACK - strip out localization tokens before string command is displayed in syscon window
			if ( !Q_strncmp( in, "[lon]", 5 ) || !Q_strncmp( in, "[lof]", 5 ) ) {
				in += 5;
				continue;
			}
#endif // RTCW_XX

			string[l++] = *in;
		}
		in++;
	}
	string[l] = 0;

	return string;
}

#if defined RTCW_SP
/*
======================
SV_ReplacePendingServerCommands

  This is ugly
======================
*/
#if 0   // RF, not used anymore
int SV_ReplacePendingServerCommands( client_t *client, const char *cmd ) {
	int i, index, csnum1, csnum2;

	for ( i = client->reliableSent + 1; i <= client->reliableSequence; i++ ) {
		index = i & ( MAX_RELIABLE_COMMANDS - 1 );
		//
		//if ( !Q_strncmp(cmd, client->reliableCommands[ index ], strlen("cs")) ) {
		if ( !Q_strncmp( cmd, SV_GetReliableCommand( client, index ), strlen( "cs" ) ) ) {
			sscanf( cmd, "cs %i", &csnum1 );
			//sscanf(client->reliableCommands[ index ], "cs %i", &csnum2);
			sscanf( SV_GetReliableCommand( client, index ), "cs %i", &csnum2 );
			if ( csnum1 == csnum2 ) {
				//Q_strncpyz( client->reliableCommands[ index ], cmd, sizeof( client->reliableCommands[ index ] ) );

				/*
				if ( client->netchan.remoteAddress.type != NA_BOT ) {
					Com_Printf( "WARNING: client %i removed double pending config string %i: %s\n", client-svs.clients, csnum1, cmd );
				}
				*/
				return qtrue;
			}
		}
	}
	return qfalse;
}
#endif
#endif // RTCW_XX

/*
======================
SV_AddServerCommand

The given command will be transmitted to the client, and is guaranteed to
not have future snapshot_t executed before it is executed
======================
*/
void SV_AddServerCommand( client_t *client, const char *cmd ) {
	int index, i;

	client->reliableSequence++;
	// if we would be losing an old command that hasn't been acknowledged,
	// we must drop the connection
	// we check == instead of >= so a broadcast print added by SV_DropClient()
	// doesn't cause a recursive drop client
	if ( client->reliableSequence - client->reliableAcknowledge == MAX_RELIABLE_COMMANDS + 1 ) {
		Com_Printf( "===== pending server commands =====\n" );
		for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {

#if defined RTCW_SP
			//Com_Printf( "cmd %5d: %s\n", i, client->reliableCommands[ i & (MAX_RELIABLE_COMMANDS-1) ] );
			Com_Printf( "cmd %5d: %s\n", i, SV_GetReliableCommand( client, i & ( MAX_RELIABLE_COMMANDS - 1 ) ) );
#else
			Com_Printf( "cmd %5d: %s\n", i, client->reliableCommands[ i & ( MAX_RELIABLE_COMMANDS - 1 ) ] );
#endif // RTCW_XX

		}
		Com_Printf( "cmd %5d: %s\n", i, cmd );
		SV_DropClient( client, "Server command overflow" );
		return;
	}
	index = client->reliableSequence & ( MAX_RELIABLE_COMMANDS - 1 );

#if defined RTCW_SP
	//Q_strncpyz( client->reliableCommands[ index ], cmd, sizeof( client->reliableCommands[ index ] ) );
	SV_AddReliableCommand( client, index, cmd );
#else
	Q_strncpyz( client->reliableCommands[ index ], cmd, sizeof( client->reliableCommands[ index ] ) );
#endif // RTCW_XX

}


/*
=================
SV_SendServerCommand

Sends a reliable command string to be interpreted by
the client game module: "cp", "print", "chat", etc
A NULL client will broadcast to all clients
=================
*/
void QDECL SV_SendServerCommand( client_t *cl, const char *fmt, ... ) {
	va_list argptr;
	byte message[MAX_MSGLEN];
	client_t    *client;
	int j;

	va_start( argptr,fmt );

#if defined RTCW_SP
	vsprintf( (char *)message, fmt,argptr );
#else
	Q_vsnprintf( (char *)message, sizeof( message ), fmt, argptr );
#endif // RTCW_XX

	va_end( argptr );

#if !defined RTCW_SP
	// do not forward server command messages that would be too big to clients
	// ( q3infoboom / q3msgboom stuff )
	if ( strlen( (char *)message ) > 1022 ) {
		return;
	}
#endif // RTCW_XX

	if ( cl != NULL ) {
		SV_AddServerCommand( cl, (char *)message );
		return;
	}

	// hack to echo broadcast prints to console
	if ( com_dedicated->integer && !strncmp( (char *)message, "print", 5 ) ) {
		Com_Printf( "broadcast: %s\n", SV_ExpandNewlines( (char *)message ) );
	}

	// send the data to all relevent clients
	for ( j = 0, client = svs.clients; j < sv_maxclients->integer ; j++, client++ ) {
		if ( client->state < CS_PRIMED ) {
			continue;
		}
		// Ridah, don't need to send messages to AI

#if !defined RTCW_ET
		if ( client->gentity && client->gentity->r.svFlags & SVF_CASTAI ) {
#else
		if ( client->gentity && client->gentity->r.svFlags & SVF_BOT ) {
#endif // RTCW_XX

			continue;
		}
		// done.
		SV_AddServerCommand( client, (char *)message );
	}
}


/*
==============================================================================

MASTER SERVER FUNCTIONS

==============================================================================
*/

/*
================
SV_MasterHeartbeat

Send a message to the masters every few minutes to
let it know we are alive, and log information.
We will also have a heartbeat sent when a server
changes from empty to non-empty, and full to non-full,
but not on every player enter or exit.
================
*/
#define HEARTBEAT_MSEC  300 * 1000

#if !defined RTCW_ET
#define HEARTBEAT_GAME  "Wolfenstein-1"
#else
//#define	HEARTBEAT_GAME	"Wolfenstein-1"
//#define	HEARTBEAT_DEAD	"WolfFlatline-1"			// NERVE - SMF
#define HEARTBEAT_GAME  "EnemyTerritory-1"
#endif // RTCW_XX

#if defined RTCW_SP
void SV_MasterHeartbeat( void ) {
#else

#if !defined RTCW_ET
#define HEARTBEAT_DEAD  "WolfFlatline-1"         // NERVE - SMF
#else
#define HEARTBEAT_DEAD  "ETFlatline-1"           // NERVE - SMF
#endif // RTCW_XX

void SV_MasterHeartbeat( const char *hbname ) {
#endif // RTCW_XX

	static netadr_t adr[MAX_MASTER_SERVERS];
	int i;

#if defined RTCW_MP
	// DHM - Nerve :: Update Server doesn't send heartbeat
#ifdef UPDATE_SERVER
	return;
#endif
#endif // RTCW_XX

#if defined RTCW_ET
	if ( SV_GameIsSinglePlayer() ) {
		return;     // no heartbeats for SP
	}
#endif // RTCW_XX

	// "dedicated 1" is for lan play, "dedicated 2" is for inet public play
	if ( !com_dedicated || com_dedicated->integer != 2 ) {
		return;     // only dedicated servers send heartbeats
	}

	// if not time yet, don't send anything
	if ( svs.time < svs.nextHeartbeatTime ) {
		return;
	}
	svs.nextHeartbeatTime = svs.time + HEARTBEAT_MSEC;


	// send to group masters
	for ( i = 0 ; i < MAX_MASTER_SERVERS ; i++ ) {
		if ( !sv_master[i]->string[0] ) {
			continue;
		}

		// see if we haven't already resolved the name
		// resolving usually causes hitches on win95, so only
		// do it when needed
		if ( sv_master[i]->modified ) {
			sv_master[i]->modified = qfalse;

			Com_Printf( "Resolving %s\n", sv_master[i]->string );
			if ( !NET_StringToAdr( sv_master[i]->string, &adr[i] ) ) {
				// if the address failed to resolve, clear it
				// so we don't take repeated dns hits
				Com_Printf( "Couldn't resolve address: %s\n", sv_master[i]->string );
				Cvar_Set( sv_master[i]->name, "" );
				sv_master[i]->modified = qfalse;
				continue;
			}
			if ( !strstr( ":", sv_master[i]->string ) ) {
				adr[i].port = rtcw::Endian::be( PORT_MASTER );
			}
			Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", sv_master[i]->string,
						adr[i].ip[0], adr[i].ip[1], adr[i].ip[2], adr[i].ip[3],
						rtcw::Endian::be( adr[i].port ) );
		}


		Com_Printf( "Sending heartbeat to %s\n", sv_master[i]->string );
		// this command should be changed if the server info / status format
		// ever incompatably changes

#if defined RTCW_SP
		NET_OutOfBandPrint( NS_SERVER, adr[i], "heartbeat %s\n", HEARTBEAT_GAME );
#else
		NET_OutOfBandPrint( NS_SERVER, adr[i], "heartbeat %s\n", hbname );
#endif // RTCW_XX

	}
}

#if !defined RTCW_SP
/*
=================
SV_MasterGameCompleteStatus

NERVE - SMF - Sends gameCompleteStatus messages to all master servers
=================
*/
void SV_MasterGameCompleteStatus() {
	static netadr_t adr[MAX_MASTER_SERVERS];
	int i;

#if defined RTCW_ET
	if ( SV_GameIsSinglePlayer() ) {
		return;     // no master game status for SP
	}
#endif // RTCW_XX

	// "dedicated 1" is for lan play, "dedicated 2" is for inet public play
	if ( !com_dedicated || com_dedicated->integer != 2 ) {
		return;     // only dedicated servers send master game status
	}

	// send to group masters
	for ( i = 0 ; i < MAX_MASTER_SERVERS ; i++ ) {
		if ( !sv_master[i]->string[0] ) {
			continue;
		}

		// see if we haven't already resolved the name
		// resolving usually causes hitches on win95, so only
		// do it when needed
		if ( sv_master[i]->modified ) {
			sv_master[i]->modified = qfalse;

			Com_Printf( "Resolving %s\n", sv_master[i]->string );
			if ( !NET_StringToAdr( sv_master[i]->string, &adr[i] ) ) {
				// if the address failed to resolve, clear it
				// so we don't take repeated dns hits
				Com_Printf( "Couldn't resolve address: %s\n", sv_master[i]->string );
				Cvar_Set( sv_master[i]->name, "" );
				sv_master[i]->modified = qfalse;
				continue;
			}
			if ( !strstr( ":", sv_master[i]->string ) ) {
				adr[i].port = rtcw::Endian::be ( PORT_MASTER );
			}
			Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", sv_master[i]->string,
						adr[i].ip[0], adr[i].ip[1], adr[i].ip[2], adr[i].ip[3],
						rtcw::Endian::be ( adr[i].port ) );
		}

		Com_Printf( "Sending gameCompleteStatus to %s\n", sv_master[i]->string );
		// this command should be changed if the server info / status format
		// ever incompatably changes
		SVC_GameCompleteStatus( adr[i] );
	}
}
#endif // RTCW_XX

/*
=================
SV_MasterShutdown

Informs all masters that this server is going down
=================
*/
void SV_MasterShutdown( void ) {
	// send a hearbeat right now
	svs.nextHeartbeatTime = -9999;

#if defined RTCW_SP
	SV_MasterHeartbeat();
#else
	SV_MasterHeartbeat( HEARTBEAT_DEAD );               // NERVE - SMF - changed to flatline
#endif // RTCW_XX

	// send it again to minimize chance of drops

#if defined RTCW_SP
	svs.nextHeartbeatTime = -9999;
	SV_MasterHeartbeat();
#else
//	svs.nextHeartbeatTime = -9999;
//	SV_MasterHeartbeat( HEARTBEAT_DEAD );
#endif // RTCW_XX

	// when the master tries to poll the server, it won't respond, so
	// it will be removed from the list
}


/*
==============================================================================

CONNECTIONLESS COMMANDS

==============================================================================
*/

#if defined RTCW_ET
//bani - bugtraq 12534
//returns qtrue if valid challenge
//returns qfalse if m4d h4x0rz
qboolean SV_VerifyChallenge( const char *challenge ) {
	int i, j;

	if ( !challenge ) {
		return qfalse;
	}

	j = strlen( challenge );
	if ( j > 64 ) {
		return qfalse;
	}
	for ( i = 0; i < j; i++ ) {
		if ( challenge[i] == '\\' ||
			 challenge[i] == '/' ||
			 challenge[i] == '%' ||
			 challenge[i] == ';' ||
			 challenge[i] == '"' ||
			 challenge[i] < 32 ||   // non-ascii
			 challenge[i] > 126 // non-ascii
			 ) {
			return qfalse;
		}
	}
	return qtrue;
}
#endif // RTCW_XX

/*
================
SVC_Status

Responds with all the info that qplug or qspy can see about the server
and all connected players.  Used for getting detailed information after
the simple info query.
================
*/
void SVC_Status( netadr_t from ) {
	char player[1024];
	char status[MAX_MSGLEN];
	int i;
	client_t    *cl;
	playerState_t   *ps;
	int statusLength;
	int playerLength;
	char infostring[MAX_INFO_STRING];

	// ignore if we are in single player

#if !defined RTCW_ET
	if ( Cvar_VariableValue( "g_gametype" ) == GT_SINGLE_PLAYER ) {
#else
	if ( SV_GameIsSinglePlayer() ) {
#endif // RTCW_XX
		return;
	}

#if defined RTCW_MP
	// DHM - Nerve
#ifdef UPDATE_SERVER
	return;
#endif
#endif // RTCW_XX

#if defined RTCW_ET
	//bani - bugtraq 12534
	if ( !SV_VerifyChallenge( Cmd_Argv( 1 ) ) ) {
		return;
	}
#endif // RTCW_XX


#if !defined RTCW_ET
	strcpy( infostring, Cvar_InfoString( CVAR_SERVERINFO ) );
#else
	strcpy( infostring, Cvar_InfoString( CVAR_SERVERINFO | CVAR_SERVERINFO_NOUPDATE ) );
#endif // RTCW_XX

	// echo back the parameter to status. so master servers can use it as a challenge
	// to prevent timed spoofed reply packets that add ghost servers
	Info_SetValueForKey( infostring, "challenge", Cmd_Argv( 1 ) );

	// add "demo" to the sv_keywords if restricted
	if ( Cvar_VariableValue( "fs_restrict" ) ) {
		char keywords[MAX_INFO_STRING];

#if !defined RTCW_ET
		Com_sprintf( keywords, sizeof( keywords ), "demo %s",
#else
		Com_sprintf( keywords, sizeof( keywords ), "ettest %s",
#endif // RTCW_XX

					 Info_ValueForKey( infostring, "sv_keywords" ) );
		Info_SetValueForKey( infostring, "sv_keywords", keywords );
	}

	status[0] = 0;
	statusLength = 0;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];
		if ( cl->state >= CS_CONNECTED ) {
			ps = SV_GameClientNum( i );
			Com_sprintf( player, sizeof( player ), "%i %i \"%s\"\n",
						 ps->persistant[PERS_SCORE], cl->ping, cl->name );
			playerLength = strlen( player );
			if ( statusLength + playerLength >= sizeof( status ) ) {
				break;      // can't hold any more
			}
			strcpy( status + statusLength, player );
			statusLength += playerLength;
		}
	}

	NET_OutOfBandPrint( NS_SERVER, from, "statusResponse\n%s\n%s", infostring, status );
}

#if !defined RTCW_SP
/*
=================
SVC_GameCompleteStatus

NERVE - SMF - Send serverinfo cvars, etc to master servers when
game complete. Useful for tracking global player stats.
=================
*/
void SVC_GameCompleteStatus( netadr_t from ) {
	char player[1024];
	char status[MAX_MSGLEN];
	int i;
	client_t    *cl;
	playerState_t   *ps;
	int statusLength;
	int playerLength;
	char infostring[MAX_INFO_STRING];

	// ignore if we are in single player

#if !defined RTCW_ET
	if ( Cvar_VariableValue( "g_gametype" ) == GT_SINGLE_PLAYER ) {
#else
	if ( SV_GameIsSinglePlayer() ) {
#endif // RTCW_XX

		return;
	}

#if defined RTCW_ET
	//bani - bugtraq 12534
	if ( !SV_VerifyChallenge( Cmd_Argv( 1 ) ) ) {
		return;
	}
#endif // RTCW_XX


#if !defined RTCW_ET
	strcpy( infostring, Cvar_InfoString( CVAR_SERVERINFO ) );
#else
	strcpy( infostring, Cvar_InfoString( CVAR_SERVERINFO | CVAR_SERVERINFO_NOUPDATE ) );
#endif // RTCW_XX

	// echo back the parameter to status. so master servers can use it as a challenge
	// to prevent timed spoofed reply packets that add ghost servers
	Info_SetValueForKey( infostring, "challenge", Cmd_Argv( 1 ) );

	// add "demo" to the sv_keywords if restricted
	if ( Cvar_VariableValue( "fs_restrict" ) ) {
		char keywords[MAX_INFO_STRING];

#if !defined RTCW_ET
		Com_sprintf( keywords, sizeof( keywords ), "demo %s",
#else
		Com_sprintf( keywords, sizeof( keywords ), "ettest %s",
#endif // RTCW_XX

					 Info_ValueForKey( infostring, "sv_keywords" ) );
		Info_SetValueForKey( infostring, "sv_keywords", keywords );
	}

	status[0] = 0;
	statusLength = 0;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];
		if ( cl->state >= CS_CONNECTED ) {
			ps = SV_GameClientNum( i );
			Com_sprintf( player, sizeof( player ), "%i %i \"%s\"\n",
						 ps->persistant[PERS_SCORE], cl->ping, cl->name );
			playerLength = strlen( player );
			if ( statusLength + playerLength >= sizeof( status ) ) {
				break;      // can't hold any more
			}
			strcpy( status + statusLength, player );
			statusLength += playerLength;
		}
	}

	NET_OutOfBandPrint( NS_SERVER, from, "gameCompleteStatus\n%s\n%s", infostring, status );
}
#endif // RTCW_XX

/*
================
SVC_Info

Responds with a short info message that should be enough to determine
if a user is interested in a server to do a full status
================
*/
void SVC_Info( netadr_t from ) {
	int i, count;
	const char    *gamedir;
	char infostring[MAX_INFO_STRING];

#if defined RTCW_MP
	const char    *antilag;

	// DHM - Nerve
#ifdef UPDATE_SERVER
	return;
#endif
#endif // RTCW_XX

#if defined RTCW_ET
	const char    *antilag;
	const char    *weaprestrict;
	const char    *balancedteams;

	// ignore if we are in single player
	if ( SV_GameIsSinglePlayer() ) {
		return;
	}
#endif // RTCW_XX

#if !defined RTCW_ET
	// ignore if we are in single player
	if ( Cvar_VariableValue( "g_gametype" ) == GT_SINGLE_PLAYER ) {
		return;
	}
#endif // RTCW_XX

#if defined RTCW_ET
	//bani - bugtraq 12534
	if ( !SV_VerifyChallenge( Cmd_Argv( 1 ) ) ) {
		return;
	}
#endif // RTCW_XX

	// don't count privateclients
	count = 0;
	for ( i = sv_privateClients->integer ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			count++;
		}
	}

	infostring[0] = 0;

	// echo back the parameter to status. so servers can use it as a challenge
	// to prevent timed spoofed reply packets that add ghost servers
	Info_SetValueForKey( infostring, "challenge", Cmd_Argv( 1 ) );

	Info_SetValueForKey( infostring, "protocol", va( "%i", PROTOCOL_VERSION ) );
	Info_SetValueForKey( infostring, "hostname", sv_hostname->string );

#if defined RTCW_ET
	Info_SetValueForKey( infostring, "serverload", va( "%i", svs.serverLoad ) );
#endif // RTCW_XX

	Info_SetValueForKey( infostring, "mapname", sv_mapname->string );
	Info_SetValueForKey( infostring, "clients", va( "%i", count ) );
	Info_SetValueForKey( infostring, "sv_maxclients",
						 va( "%i", sv_maxclients->integer - sv_privateClients->integer ) );

#if !defined RTCW_ET
	Info_SetValueForKey( infostring, "gametype", va( "%i", sv_gametype->integer ) );
#else
	Info_SetValueForKey( infostring, "gametype", Cvar_VariableString( "g_gametype" ) );
#endif // RTCW_XX

	Info_SetValueForKey( infostring, "pure", va( "%i", sv_pure->integer ) );

	if ( sv_minPing->integer ) {
		Info_SetValueForKey( infostring, "minPing", va( "%i", sv_minPing->integer ) );
	}
	if ( sv_maxPing->integer ) {
		Info_SetValueForKey( infostring, "maxPing", va( "%i", sv_maxPing->integer ) );
	}
	gamedir = Cvar_VariableString( "fs_game" );
	if ( *gamedir ) {
		Info_SetValueForKey( infostring, "game", gamedir );
	}
	Info_SetValueForKey( infostring, "sv_allowAnonymous", va( "%i", sv_allowAnonymous->integer ) );

	// Rafael gameskill

#if !defined RTCW_ET
	Info_SetValueForKey( infostring, "gameskill", va( "%i", sv_gameskill->integer ) );
#else
//	Info_SetValueForKey (infostring, "gameskill", va ("%i", sv_gameskill->integer));
#endif // RTCW_XX

	// done

#if !defined RTCW_SP
	Info_SetValueForKey( infostring, "friendlyFire", va( "%i", sv_friendlyFire->integer ) );        // NERVE - SMF
	Info_SetValueForKey( infostring, "maxlives", va( "%i", sv_maxlives->integer ? 1 : 0 ) );        // NERVE - SMF

#if !defined RTCW_ET
	Info_SetValueForKey( infostring, "tourney", va( "%i", sv_tourney->integer ) );              // NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
	Info_SetValueForKey( infostring, "needpass", va( "%i", sv_needpass->integer ? 1 : 0 ) );
#endif // RTCW_XX

	Info_SetValueForKey( infostring, "gamename", GAMENAME_STRING );                               // Arnout: to be able to filter out Quake servers

	// TTimo
	antilag = Cvar_VariableString( "g_antilag" );
	if ( antilag ) {
		Info_SetValueForKey( infostring, "g_antilag", antilag );
	}
#endif // RTCW_XX

#if defined RTCW_ET
	weaprestrict = Cvar_VariableString( "g_heavyWeaponRestriction" );
	if ( weaprestrict ) {
		Info_SetValueForKey( infostring, "weaprestrict", weaprestrict );
	}

	balancedteams = Cvar_VariableString( "g_balancedteams" );
	if ( balancedteams ) {
		Info_SetValueForKey( infostring, "balancedteams", balancedteams );
	}
#endif // RTCW_XX

	NET_OutOfBandPrint( NS_SERVER, from, "infoResponse\n%s", infostring );
}

#if defined RTCW_MP
// DHM - Nerve
#ifdef UPDATE_SERVER
/*
================
SVC_GetUpdateInfo

Responds with a short info message that tells the client if they
have an update available for their version
================
*/
void SVC_GetUpdateInfo( netadr_t from ) {
	char *version;
	char *platform;
	int i;
	qboolean found = qfalse;

	version = Cmd_Argv( 1 );
	platform = Cmd_Argv( 2 );

	Com_DPrintf( "SVC_GetUpdateInfo: version == %s / %s,\n", version, platform );

	for ( i = 0; i < numVersions; i++ ) {
		if ( !strcmp( versionMap[i].version, version ) &&
			 !strcmp( versionMap[i].platform, platform ) ) {

			// If the installer is set to "current", we will skip over it
			if ( strcmp( versionMap[i].installer, "current" ) ) {
				found = qtrue;
			}

			break;
		}
	}

	if ( found ) {
		NET_OutOfBandPrint( NS_SERVER, from, "updateResponse 1 %s", versionMap[i].installer );
		Com_DPrintf( "   SENT:  updateResponse 1 %s\n", versionMap[i].installer );
	} else {
		NET_OutOfBandPrint( NS_SERVER, from, "updateResponse 0" );
		Com_DPrintf( "   SENT:  updateResponse 0\n" );
	}
}
#endif
// DHM - Nerve
#endif // RTCW_XX

/*
==============
SV_FlushRedirect

==============
*/
void SV_FlushRedirect( char *outputbuf ) {
	NET_OutOfBandPrint( NS_SERVER, svs.redirectAddress, "print\n%s", outputbuf );
}

/*
===============
SVC_RemoteCommand

An rcon packet arrived from the network.
Shift down the remaining args
Redirect all printfs
===============
*/
void SVC_RemoteCommand( netadr_t from, msg_t *msg ) {

#if defined RTCW_SP
	qboolean valid;
	int i;
	char remaining[1024];
#define SV_OUTPUTBUF_LENGTH ( MAX_MSGLEN - 16 )
	char sv_outputbuf[SV_OUTPUTBUF_LENGTH];

	if ( !strlen( sv_rconPassword->string ) ||
		 strcmp( Cmd_Argv( 1 ), sv_rconPassword->string ) ) {
		valid = qfalse;
		Com_DPrintf( "Bad rcon from %s:\n%s\n", NET_AdrToString( from ), Cmd_Argv( 2 ) );
	} else {
		valid = qtrue;
		Com_DPrintf( "Rcon from %s:\n%s\n", NET_AdrToString( from ), Cmd_Argv( 2 ) );
	}

	// start redirecting all print outputs to the packet
	svs.redirectAddress = from;
	Com_BeginRedirect( sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect );

	if ( !strlen( sv_rconPassword->string ) ) {
		Com_Printf( "No rconpassword set.\n" );
	} else if ( !valid ) {
		Com_Printf( "Bad rconpassword.\n" );
	} else {
		remaining[0] = 0;

		for ( i = 2 ; i < Cmd_Argc() ; i++ ) {
			strcat( remaining, Cmd_Argv( i ) );
			strcat( remaining, " " );
		}

		Cmd_ExecuteString( remaining );
	}

	Com_EndRedirect();
#else
	qboolean valid;
	unsigned int time;
	char remaining[1024];
	// show_bug.cgi?id=376
	// if we send an OOB print message this size, 1.31 clients die in a Com_Printf buffer overflow
	// the buffer overflow will be fixed in > 1.31 clients
	// but we want a server side fix
	// we must NEVER send an OOB message that will be > 1.31 MAXPRINTMSG (4096)
#define SV_OUTPUTBUF_LENGTH ( 256 - 16 )
	char sv_outputbuf[SV_OUTPUTBUF_LENGTH];
	static unsigned int lasttime = 0;
	char *cmd_aux;

	// TTimo - show_bug.cgi?id=534
	time = Com_Milliseconds();
	if ( time < ( lasttime + 500 ) ) {
		return;
	}
	lasttime = time;

	if ( !strlen( sv_rconPassword->string ) ||
		 strcmp( Cmd_Argv( 1 ), sv_rconPassword->string ) ) {
		valid = qfalse;
		Com_Printf( "Bad rcon from %s:\n%s\n", NET_AdrToString( from ), Cmd_Argv( 2 ) );
	} else {
		valid = qtrue;
		Com_Printf( "Rcon from %s:\n%s\n", NET_AdrToString( from ), Cmd_Argv( 2 ) );
	}

	// start redirecting all print outputs to the packet
	svs.redirectAddress = from;
	// FIXME TTimo our rcon redirection could be improved
	//   big rcon commands such as status lead to sending
	//   out of band packets on every single call to Com_Printf
	//   which leads to client overflows
	//   see show_bug.cgi?id=51
	//     (also a Q3 issue)
	Com_BeginRedirect( sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect );

	if ( !strlen( sv_rconPassword->string ) ) {
		Com_Printf( "No rconpassword set on the server.\n" );
	} else if ( !valid ) {
		Com_Printf( "Bad rconpassword.\n" );
	} else {
		remaining[0] = 0;

		// ATVI Wolfenstein Misc #284
		// get the command directly, "rcon <pass> <command>" to avoid quoting issues
		// extract the command by walking
		// since the cmd formatting can fuckup (amount of spaces), using a dumb step by step parsing
		cmd_aux = Cmd_Cmd();
		cmd_aux += 4;
		while ( cmd_aux[0] == ' ' )
			cmd_aux++;
		while ( cmd_aux[0] && cmd_aux[0] != ' ' ) // password
			cmd_aux++;
		while ( cmd_aux[0] == ' ' )
			cmd_aux++;

		Q_strcat( remaining, sizeof( remaining ), cmd_aux );

		Cmd_ExecuteString( remaining );

	}

	Com_EndRedirect();
#endif // RTCW_XX

}

/*
=================
SV_ConnectionlessPacket

A connectionless packet has four leading 0xff
characters to distinguish it from a game channel.
Clients that are in the game can still send
connectionless packets.
=================
*/
void SV_ConnectionlessPacket( netadr_t from, msg_t *msg ) {
	char    *s;
	const char    *c;

	MSG_BeginReadingOOB( msg );
	MSG_ReadLong( msg );        // skip the -1 marker

#if !defined RTCW_SP
	if ( !Q_strncmp( "connect", reinterpret_cast<const char*> (&msg->data[4]), 7 ) ) {
		Huff_Decompress( msg, 12 );
	}
#endif // RTCW_XX

	s = MSG_ReadStringLine( msg );

	Cmd_TokenizeString( s );

	c = Cmd_Argv( 0 );
	Com_DPrintf( "SV packet %s : %s\n", NET_AdrToString( from ), c );

	if ( !Q_stricmp( c,"getstatus" ) ) {
		SVC_Status( from  );
	} else if ( !Q_stricmp( c,"getinfo" ) ) {
		SVC_Info( from );
	} else if ( !Q_stricmp( c,"getchallenge" ) ) {
		SV_GetChallenge( from );
	} else if ( !Q_stricmp( c,"connect" ) ) {
		SV_DirectConnect( from );

#if !defined RTCW_ET || (defined RTCW_ET && AUTHORIZE_SUPPORT)
	} else if ( !Q_stricmp( c,"ipAuthorize" ) ) {
		SV_AuthorizeIpPacket( from );
#endif // RTCW_XX

	} else if ( !Q_stricmp( c, "rcon" ) ) {
		SVC_RemoteCommand( from, msg );

#if defined RTCW_MP
// DHM - Nerve
#ifdef UPDATE_SERVER
	} else if ( !Q_stricmp( c, "getUpdateInfo" ) ) {
		SVC_GetUpdateInfo( from );
#endif
// DHM - Nerve
#endif // RTCW_XX

	} else if ( !Q_stricmp( c,"disconnect" ) ) {
		// if a client starts up a local server, we may see some spurious
		// server disconnect messages when their new server sees our final
		// sequenced messages to the old client
	} else {
		Com_DPrintf( "bad connectionless packet from %s:\n%s\n"
					 , NET_AdrToString( from ), s );
	}
}

//============================================================================

/*
=================
SV_ReadPackets
=================
*/
void SV_PacketEvent( netadr_t from, msg_t *msg ) {
	int i;
	client_t    *cl;
	int qport;

	// check for connectionless packet (0xffffffff) first
	if ( msg->cursize >= 4 && *(int *)msg->data == -1 ) {
		SV_ConnectionlessPacket( from, msg );
		return;
	}

	// read the qport out of the message so we can fix up
	// stupid address translating routers
	MSG_BeginReadingOOB( msg );
	MSG_ReadLong( msg );                // sequence number
	qport = MSG_ReadShort( msg ) & 0xffff;

	// find which client the message is from
	for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
		if ( cl->state == CS_FREE ) {
			continue;
		}
		if ( !NET_CompareBaseAdr( from, cl->netchan.remoteAddress ) ) {
			continue;
		}
		// it is possible to have multiple clients from a single IP
		// address, so they are differentiated by the qport variable
		if ( cl->netchan.qport != qport ) {
			continue;
		}

		// the IP port can't be used to differentiate them, because
		// some address translating routers periodically change UDP
		// port assignments
		if ( cl->netchan.remoteAddress.port != from.port ) {

#if defined RTCW_SP
			Com_Printf( "SV_ReadPackets: fixing up a translated port\n" );
#else
			Com_Printf( "SV_PacketEvent: fixing up a translated port\n" );
#endif // RTCW_XX

			cl->netchan.remoteAddress.port = from.port;
		}

		// make sure it is a valid, in sequence packet
		if ( SV_Netchan_Process( cl, msg ) ) {
			// zombie clients still need to do the Netchan_Process
			// to make sure they don't need to retransmit the final
			// reliable message, but they don't do any other processing
			if ( cl->state != CS_ZOMBIE ) {
				cl->lastPacketTime = svs.time;  // don't timeout
				SV_ExecuteClientMessage( cl, msg );
			}
		}
		return;
	}

	// if we received a sequenced packet from an address we don't recognize,
	// send an out of band disconnect packet to it
	NET_OutOfBandPrint( NS_SERVER, from, "disconnect" );
}


/*
===================
SV_CalcPings

Updates the cl->ping variables
===================
*/
void SV_CalcPings( void ) {
	int i, j;
	client_t    *cl;
	int total, count;
	int delta;
	playerState_t   *ps;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];

#if defined RTCW_MP
		// DHM - Nerve
#ifdef UPDATE_SERVER
		if ( !cl ) {
			continue;
		}
#endif
#endif // RTCW_XX

		if ( cl->state != CS_ACTIVE ) {
			cl->ping = 999;
			continue;
		}
		if ( !cl->gentity ) {
			cl->ping = 999;
			continue;
		}
		if ( cl->gentity->r.svFlags & SVF_BOT ) {
			cl->ping = 0;
			continue;
		}

		total = 0;
		count = 0;
		for ( j = 0 ; j < PACKET_BACKUP ; j++ ) {
			if ( cl->frames[j].messageAcked <= 0 ) {
				continue;
			}
			delta = cl->frames[j].messageAcked - cl->frames[j].messageSent;
			count++;
			total += delta;
		}
		if ( !count ) {
			cl->ping = 999;
		} else {
			cl->ping = total / count;
			if ( cl->ping > 999 ) {
				cl->ping = 999;
			}
		}

		// let the game dll know about the ping
		ps = SV_GameClientNum( i );
		ps->ping = cl->ping;
	}
}

/*
==================
SV_CheckTimeouts

If a packet has not been received from a client for timeout->integer
seconds, drop the conneciton.  Server time is used instead of
realtime to avoid dropping the local client while debugging.

When a client is normally dropped, the client_t goes into a zombie state
for a few seconds to make sure any final reliable message gets resent
if necessary
==================
*/
void SV_CheckTimeouts( void ) {
	int i;
	client_t    *cl;
	int droppoint;
	int zombiepoint;

	droppoint = svs.time - 1000 * sv_timeout->integer;
	zombiepoint = svs.time - 1000 * sv_zombietime->integer;

	for ( i = 0,cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
		// message times may be wrong across a changelevel
		if ( cl->lastPacketTime > svs.time ) {
			cl->lastPacketTime = svs.time;
		}

		if ( cl->state == CS_ZOMBIE
			 && cl->lastPacketTime < zombiepoint ) {

#if defined RTCW_SP
			Com_DPrintf( "Going from CS_ZOMBIE to CS_FREE for %s\n", cl->name );
			cl->state = CS_FREE;    // can now be reused
			continue;
		}
		// Ridah, AI's don't time out
		if ( cl->gentity && !( cl->gentity->r.svFlags & SVF_CASTAI ) ) {
			if ( cl->state >= CS_CONNECTED && cl->lastPacketTime < droppoint ) {
				// wait several frames so a debugger session doesn't
				// cause a timeout
				if ( ++cl->timeoutCount > 5 ) {
					SV_DropClient( cl, "timed out" );
					cl->state = CS_FREE;    // don't bother with zombie state
				}
			} else {
				cl->timeoutCount = 0;
			}
#else
			// using the client id cause the cl->name is empty at this point
			Com_DPrintf( "Going from CS_ZOMBIE to CS_FREE for client %d\n", i );
			cl->state = CS_FREE;    // can now be reused
			continue;
		}
		if ( cl->state >= CS_CONNECTED && cl->lastPacketTime < droppoint ) {
			// wait several frames so a debugger session doesn't
			// cause a timeout
			if ( ++cl->timeoutCount > 5 ) {
				SV_DropClient( cl, "timed out" );
				cl->state = CS_FREE;    // don't bother with zombie state
			}
		} else {
			cl->timeoutCount = 0;
#endif // RTCW_XX

		}
	}
}


/*
==================
SV_CheckPaused
==================
*/
qboolean SV_CheckPaused( void ) {
	int count;
	client_t    *cl;
	int i;

	if ( !cl_paused->integer ) {
		return qfalse;
	}

	// only pause if there is just a single client connected
	count = 0;
	for ( i = 0,cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
		if ( cl->state >= CS_CONNECTED && cl->netchan.remoteAddress.type != NA_BOT ) {
			count++;
		}
	}

#if defined RTCW_SP
	if ( count > 1 ) {
		// don't pause
		sv_paused->integer = 0;
		return qfalse;
	}

	sv_paused->integer = 1;
#else
	if ( count > 1 ) {
		// don't pause
		if ( sv_paused->integer ) {
			Cvar_Set( "sv_paused", "0" );
		}
		return qfalse;
	}

	if ( !sv_paused->integer ) {
		Cvar_Set( "sv_paused", "1" );
	}
#endif // RTCW_XX

	return qtrue;
}

/*
==================
SV_Frame

Player movement occurs as a result of packet events, which
happen before SV_Frame is called
==================
*/
void SV_Frame( int msec ) {
	int frameMsec;
	int startTime;

#if !defined RTCW_SP
	char mapname[MAX_QPATH];
#endif // RTCW_XX

#if defined RTCW_ET
	int frameStartTime = 0, frameEndTime;
#endif // RTCW_XX

	// the menu kills the server with this cvar
	if ( sv_killserver->integer ) {
		SV_Shutdown( "Server was killed.\n" );
		Cvar_Set( "sv_killserver", "0" );
		return;
	}

	if ( !com_sv_running->integer ) {
		return;
	}

	// allow pause if only the local client is connected
	if ( SV_CheckPaused() ) {
		return;
	}

#if defined RTCW_ET
	if ( com_dedicated->integer ) {
		frameStartTime = Sys_Milliseconds();
	}
#endif // RTCW_XX

	// if it isn't time for the next frame, do nothing
	if ( sv_fps->integer < 1 ) {
		Cvar_Set( "sv_fps", "10" );
	}
	frameMsec = 1000 / sv_fps->integer ;

	sv.timeResidual += msec;

	if ( !com_dedicated->integer ) {
		SV_BotFrame( svs.time + sv.timeResidual );
	}

	if ( com_dedicated->integer && sv.timeResidual < frameMsec ) {
		// NET_Sleep will give the OS time slices until either get a packet
		// or time enough for a server frame has gone by
		NET_Sleep( frameMsec - sv.timeResidual );
		return;
	}

	// if time is about to hit the 32nd bit, kick all clients
	// and clear sv.time, rather
	// than checking for negative time wraparound everywhere.
	// 2giga-milliseconds = 23 days, so it won't be too often
	if ( svs.time > 0x70000000 ) {

#if defined RTCW_SP
		SV_Shutdown( "Restarting server due to time wrapping" );
		Cbuf_AddText( "vstr nextmap\n" );
#else
		Q_strncpyz( mapname, sv_mapname->string, MAX_QPATH );
		SV_Shutdown( "Restarting server due to time wrapping" );
		// TTimo
		// show_bug.cgi?id=388
		// there won't be a map_restart if you have shut down the server
		// since it doesn't restart a non-running server
		// instead, re-run the current map
		Cbuf_AddText( va( "map %s\n", mapname ) );
#endif // RTCW_XX

		return;
	}
	// this can happen considerably earlier when lots of clients play and the map doesn't change
	if ( svs.nextSnapshotEntities >= 0x7FFFFFFE - svs.numSnapshotEntities ) {

#if defined RTCW_SP
		SV_Shutdown( "Restarting server due to numSnapshotEntities wrapping" );
		Cbuf_AddText( "vstr nextmap\n" );
#else
		Q_strncpyz( mapname, sv_mapname->string, MAX_QPATH );
		SV_Shutdown( "Restarting server due to numSnapshotEntities wrapping" );
		// TTimo see above
		Cbuf_AddText( va( "map %s\n", mapname ) );
#endif // RTCW_XX

		return;
	}

	if ( sv.restartTime && svs.time >= sv.restartTime ) {
		sv.restartTime = 0;
		Cbuf_AddText( "map_restart 0\n" );
		return;
	}

	// update infostrings if anything has been changed
	if ( cvar_modifiedFlags & CVAR_SERVERINFO ) {

#if !defined RTCW_ET
		SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO ) );
#else
		SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO | CVAR_SERVERINFO_NOUPDATE ) );
#endif // RTCW_XX

		cvar_modifiedFlags &= ~CVAR_SERVERINFO;
	}

#if defined RTCW_ET
	if ( cvar_modifiedFlags & CVAR_SERVERINFO_NOUPDATE ) {
		SV_SetConfigstringNoUpdate( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO | CVAR_SERVERINFO_NOUPDATE ) );
		cvar_modifiedFlags &= ~CVAR_SERVERINFO_NOUPDATE;
	}
#endif // RTCW_XX

	if ( cvar_modifiedFlags & CVAR_SYSTEMINFO ) {
		SV_SetConfigstring( CS_SYSTEMINFO, Cvar_InfoString_Big( CVAR_SYSTEMINFO ) );
		cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;
	}

#if !defined RTCW_SP
	// NERVE - SMF
	if ( cvar_modifiedFlags & CVAR_WOLFINFO ) {
		SV_SetConfigstring( CS_WOLFINFO, Cvar_InfoString( CVAR_WOLFINFO ) );
		cvar_modifiedFlags &= ~CVAR_WOLFINFO;
	}
#endif // RTCW_XX

	if ( com_speeds->integer ) {
		startTime = Sys_Milliseconds();
	} else {
		startTime = 0;  // quite a compiler warning
	}

	// update ping based on the all received frames
	SV_CalcPings();

	if ( com_dedicated->integer ) {
		SV_BotFrame( svs.time );
	}

	// run the game simulation in chunks
	while ( sv.timeResidual >= frameMsec ) {
		sv.timeResidual -= frameMsec;
		svs.time += frameMsec;

		// let everything in the world think and move

#if !defined RTCW_MP || (defined RTCW_MP && !UPDATE_SERVER)
		VM_Call(gvm, GAME_RUN_FRAME, rtcw::to_vm_arg(svs.time));
#endif // RTCW_XX

	}

	if ( com_speeds->integer ) {
		time_game = Sys_Milliseconds() - startTime;
	}

	// check timeouts
	SV_CheckTimeouts();

	// send messages back to the clients
	SV_SendClientMessages();

	// send a heartbeat to the master if needed

#if defined RTCW_SP
	SV_MasterHeartbeat();
#else
	SV_MasterHeartbeat( HEARTBEAT_GAME );
#endif // RTCW_XX

#if defined RTCW_ET
	if ( com_dedicated->integer ) {
		frameEndTime = Sys_Milliseconds();

		svs.totalFrameTime += ( frameEndTime - frameStartTime );
		svs.currentFrameIndex++;

		//if( svs.currentFrameIndex % 50 == 0 )
		//	Com_Printf( "currentFrameIndex: %i\n", svs.currentFrameIndex );

		if ( svs.currentFrameIndex == SERVER_PERFORMANCECOUNTER_FRAMES ) {
			int averageFrameTime;

			averageFrameTime = svs.totalFrameTime / SERVER_PERFORMANCECOUNTER_FRAMES;

			svs.sampleTimes[svs.currentSampleIndex % SERVER_PERFORMANCECOUNTER_SAMPLES] = averageFrameTime;
			svs.currentSampleIndex++;

			if ( svs.currentSampleIndex > SERVER_PERFORMANCECOUNTER_SAMPLES ) {
				int totalTime, i;

				totalTime = 0;
				for ( i = 0; i < SERVER_PERFORMANCECOUNTER_SAMPLES; i++ ) {
					totalTime += svs.sampleTimes[i];
				}

				if ( !totalTime ) {
					totalTime = 1;
				}

				averageFrameTime = totalTime / SERVER_PERFORMANCECOUNTER_SAMPLES;

				svs.serverLoad = ( averageFrameTime / (float)frameMsec ) * 100;
			}

			//Com_Printf( "serverload: %i (%i/%i)\n", svs.serverLoad, averageFrameTime, frameMsec );

			svs.totalFrameTime = 0;
			svs.currentFrameIndex = 0;
		}
	} else
	{
		svs.serverLoad = -1;
	}
#endif // RTCW_XX

}

#if defined RTCW_ET
/*
=================
SV_LoadTag
=================
*/
int SV_LoadTag( const char *mod_name ) {
	unsigned char*      buffer;
	tagHeader_t         *pinmodel;
	int version;
	md3Tag_t            *tag;
	md3Tag_t            *readTag;
	int i, j;

	for ( i = 0; i < sv.num_tagheaders; i++ ) {
		if ( !Q_stricmp( mod_name, sv.tagHeadersExt[i].filename ) ) {
			return i + 1;
		}
	}

	FS_ReadFile( mod_name, (void**)&buffer );

	if ( !buffer ) {
		return qfalse;
	}

	pinmodel = (tagHeader_t *)buffer;

	version = rtcw::Endian::le( pinmodel->version );
	if ( version != TAG_VERSION ) {
		Com_Printf( S_COLOR_YELLOW "WARNING: SV_LoadTag: %s has wrong version (%i should be %i)\n", mod_name, version, TAG_VERSION );
		return 0;
	}

	if ( sv.num_tagheaders >= MAX_TAG_FILES ) {
		Com_Error( ERR_DROP, "MAX_TAG_FILES reached\n" );

		FS_FreeFile( buffer );
		return 0;
	}

	LL( pinmodel->ident );
	LL( pinmodel->numTags );
	LL( pinmodel->ofsEnd );
	LL( pinmodel->version );

	Q_strncpyz( sv.tagHeadersExt[sv.num_tagheaders].filename, mod_name, MAX_QPATH );
	sv.tagHeadersExt[sv.num_tagheaders].start = sv.num_tags;
	sv.tagHeadersExt[sv.num_tagheaders].count = pinmodel->numTags;

	if ( sv.num_tags + pinmodel->numTags >= MAX_SERVER_TAGS ) {
		Com_Error( ERR_DROP, "MAX_SERVER_TAGS reached\n" );

		FS_FreeFile( buffer );
		return qfalse;
	}

	// swap all the tags
	tag = &sv.tags[sv.num_tags];
	sv.num_tags += pinmodel->numTags;
	readTag = ( md3Tag_t* )( buffer + sizeof( tagHeader_t ) );
	for ( i = 0 ; i < pinmodel->numTags; i++, tag++, readTag++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			rtcw::Endian::lei(readTag->origin);
			rtcw::Endian::lei(readTag->axis[0]);
			rtcw::Endian::lei(readTag->axis[1]);
			rtcw::Endian::lei(readTag->axis[2]);
		}
		Q_strncpyz( tag->name, readTag->name, 64 );
	}

	FS_FreeFile( buffer );
	return ++sv.num_tagheaders;
}
#endif // RTCW_XX

//============================================================================
