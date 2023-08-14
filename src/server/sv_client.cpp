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

// sv_client.c -- server code for dealing with clients


#include "server.h"

#include "rtcw_vm_args.h"


static void SV_CloseDownload( client_t *cl );

/*
=================
SV_GetChallenge

A "getchallenge" OOB command has been received
Returns a challenge number that can be used
in a subsequent connectResponse command.
We do this to prevent denial of service attacks that
flood the server with invalid connection IPs.  With a
challenge, they must give a valid IP address.

If we are authorizing, a challenge request will cause a packet
to be sent to the authorize server.

When an authorizeip is returned, a challenge response will be
sent to that ip.
=================
*/
void SV_GetChallenge( netadr_t from ) {
	int i;
	int oldest;
	int oldestTime;
	challenge_t *challenge;

	// ignore if we are in single player

#if !defined RTCW_ET
	if ( Cvar_VariableValue( "g_gametype" ) == GT_SINGLE_PLAYER ) {
#else
	if ( SV_GameIsSinglePlayer() ) {
#endif // RTCW_XX

		return;
	}

#if defined RTCW_ET
	if ( SV_TempBanIsBanned( from ) ) {
		NET_OutOfBandPrint( NS_SERVER, from, "print\n%s\n", sv_tempbanmessage->string );
		return;
	}
#endif // RTCW_XX

	oldest = 0;
	oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	challenge = &svs.challenges[0];
	for ( i = 0 ; i < MAX_CHALLENGES ; i++, challenge++ ) {

#if defined RTCW_SP
		if ( NET_CompareAdr( from, challenge->adr ) ) {
#else
		if ( !challenge->connected && NET_CompareAdr( from, challenge->adr ) ) {
#endif // RTCW_XX

			break;
		}
		if ( challenge->time < oldestTime ) {
			oldestTime = challenge->time;
			oldest = i;
		}
	}

	if ( i == MAX_CHALLENGES ) {
		// this is the first time this client has asked for a challenge
		challenge = &svs.challenges[oldest];

		challenge->challenge = ( ( rand() << 16 ) ^ rand() ) ^ svs.time;
		challenge->adr = from;

#if defined RTCW_SP
		challenge->time = svs.time;
#endif // RTCW_XX

		challenge->firstTime = svs.time;

#if !defined RTCW_SP
		challenge->firstPing = 0;
		challenge->time = svs.time;
		challenge->connected = qfalse;
#endif // RTCW_XX

		i = oldest;
	}

#if defined RTCW_ET && !defined AUTHORIZE_SUPPORT
	// FIXME: deal with restricted filesystem
	if ( 1 ) {
#endif // RTCW_XX

#if !defined RTCW_ET || (defined RTCW_ET && defined AUTHORIZE_SUPPORT)
	// if they are on a lan address, send the challengeResponse immediately
	if ( Sys_IsLANAddress( from ) ) {
#endif // RTCW_XX

		challenge->pingTime = svs.time;

#if defined RTCW_SP
		NET_OutOfBandPrint( NS_SERVER, from, "challengeResponse %i", challenge->challenge );
#else
		if ( sv_onlyVisibleClients->integer ) {
			NET_OutOfBandPrint( NS_SERVER, from, "challengeResponse %i %i", challenge->challenge, sv_onlyVisibleClients->integer );
		} else {
			NET_OutOfBandPrint( NS_SERVER, from, "challengeResponse %i", challenge->challenge );
		}
#endif // RTCW_XX

		return;
	}

#if !defined RTCW_ET || (defined RTCW_ET && defined AUTHORIZE_SUPPORT)
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

	// if they have been challenging for a long time and we
	// haven't heard anything from the authoirze server, go ahead and
	// let them in, assuming the id server is down
	if ( svs.time - challenge->firstTime > AUTHORIZE_TIMEOUT ) {
		Com_DPrintf( "authorize server timed out\n" );

		challenge->pingTime = svs.time;

#if defined RTCW_SP
		NET_OutOfBandPrint( NS_SERVER, challenge->adr,
							"challengeResponse %i", challenge->challenge );
#else
		if ( sv_onlyVisibleClients->integer ) {
			NET_OutOfBandPrint( NS_SERVER, challenge->adr,
								"challengeResponse %i %i", challenge->challenge, sv_onlyVisibleClients->integer );
		} else {
			NET_OutOfBandPrint( NS_SERVER, challenge->adr,
								"challengeResponse %i", challenge->challenge );
		}
#endif // RTCW_XX

		return;
	}

	// otherwise send their ip to the authorize server
	if ( svs.authorizeAddress.type != NA_BAD ) {
		cvar_t  *fs;
		char game[1024];

		game[0] = 0;
		fs = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
		if ( fs && fs->string[0] != 0 ) {
			strcpy( game, fs->string );
		}
		Com_DPrintf( "sending getIpAuthorize for %s\n", NET_AdrToString( from ) );
		fs = Cvar_Get( "sv_allowAnonymous", "0", CVAR_SERVERINFO );

#if !defined RTCW_MP
		// NERVE - SMF - fixed parsing on sv_allowAnonymous
#endif // RTCW_XX

		NET_OutOfBandPrint( NS_SERVER, svs.authorizeAddress,

#if defined RTCW_SP
							"getIpAuthorize %i %i.%i.%i.%i %s %s",  svs.challenges[i].challenge,
#else
							"getIpAuthorize %i %i.%i.%i.%i %s %i",  svs.challenges[i].challenge,
#endif // RTCW_XX

							from.ip[0], from.ip[1], from.ip[2], from.ip[3], game, fs->integer );
	}
#endif // RTCW_XX

}

#if !defined RTCW_ET || (defined RTCW_ET && defined AUTHORIZE_SUPPORT)
/*
====================
SV_AuthorizeIpPacket

A packet has been returned from the authorize server.
If we have a challenge adr for that ip, send the
challengeResponse to it
====================
*/
void SV_AuthorizeIpPacket( netadr_t from ) {
	int challenge;
	int i;
	const char    *s;
	const char    *r;
	char ret[1024];

	if ( !NET_CompareBaseAdr( from, svs.authorizeAddress ) ) {
		Com_Printf( "SV_AuthorizeIpPacket: not from authorize server\n" );
		return;
	}

	challenge = atoi( Cmd_Argv( 1 ) );

	for ( i = 0 ; i < MAX_CHALLENGES ; i++ ) {
		if ( svs.challenges[i].challenge == challenge ) {
			break;
		}
	}
	if ( i == MAX_CHALLENGES ) {
		Com_Printf( "SV_AuthorizeIpPacket: challenge not found\n" );
		return;
	}

	// send a packet back to the original client
	svs.challenges[i].pingTime = svs.time;
	s = Cmd_Argv( 2 );
	r = Cmd_Argv( 3 );          // reason

#if !defined RTCW_ET
	if ( !Q_stricmp( s, "demo" ) ) {
#else
	if ( !Q_stricmp( s, "ettest" ) ) {
#endif // RTCW_XX

		if ( Cvar_VariableValue( "fs_restrict" ) ) {
			// a demo client connecting to a demo server
			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr,
								"challengeResponse %i", svs.challenges[i].challenge );
			return;
		}
		// they are a demo client trying to connect to a real server
		NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, "print\nServer is not a demo server\n" );
		// clear the challenge record so it won't timeout and let them through
		memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
		return;
	}
	if ( !Q_stricmp( s, "accept" ) ) {

#if defined RTCW_SP
		NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr,
							"challengeResponse %i", svs.challenges[i].challenge );
#else
		if ( sv_onlyVisibleClients->integer ) {
			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr,
								"challengeResponse %i %i", svs.challenges[i].challenge, sv_onlyVisibleClients->integer );
		} else {
			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr,
								"challengeResponse %i", svs.challenges[i].challenge );
		}
#endif // RTCW_XX

		return;
	}
	if ( !Q_stricmp( s, "unknown" ) ) {
		if ( !r ) {
			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, "print\nAwaiting CD key authorization\n" );
		} else {
			sprintf( ret, "print\n%s\n", r );
			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, ret );
		}
		// clear the challenge record so it won't timeout and let them through
		memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
		return;
	}

	// authorization failed
	if ( !r ) {
		NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, "print\nSomeone is using this CD Key\n" );
	} else {
		sprintf( ret, "print\n%s\n", r );
		NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, ret );
	}

	// clear the challenge record so it won't timeout and let them through
	memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
}
#endif // RTCW_XX

/*
==================
SV_DirectConnect

A "connect" OOB command has been received
==================
*/

#if defined RTCW_MP
#define PB_MESSAGE "PunkBuster Anti-Cheat software must be installed " \
				   "and Enabled in order to join this server. An updated game patch can be downloaded from " \
				   "www.castlewolfenstein.com.\n"
#endif // RTCW_XX

void SV_DirectConnect( netadr_t from ) {
	char userinfo[MAX_INFO_STRING];
	int i;
	client_t    *cl, *newcl;
	MAC_STATIC client_t temp;
	sharedEntity_t *ent;
	int clientNum;

#if !defined RTCW_MP || (defined RTCW_MP && !defined UPDATE_SERVER)
	int version;
#endif // RTCW_XX

	int qport;
	int challenge;
	const char        *password;
	int startIndex;
	char        *denied;
	int count;

	Com_DPrintf( "SVC_DirectConnect ()\n" );

	Q_strncpyz( userinfo, Cmd_Argv( 1 ), sizeof( userinfo ) );

#if !defined RTCW_MP || (defined RTCW_MP && !defined UPDATE_SERVER)
	// DHM - Nerve :: Update Server allows any protocol to connect

#if defined RTCW_ET
	// NOTE TTimo: but we might need to store the protocol around for potential non http/ftp clients
#endif // RTCW_XX

	version = atoi( Info_ValueForKey( userinfo, "protocol" ) );
	if ( version != PROTOCOL_VERSION ) {

#if defined RTCW_SP
		NET_OutOfBandPrint( NS_SERVER, from, "print\nServer uses protocol version %i.\n", PROTOCOL_VERSION );
#elif defined RTCW_MP
		if ( version <= 59 ) {
			// old clients, don't send them the [err_drop] tag
			NET_OutOfBandPrint( NS_SERVER, from, "print\n" PROTOCOL_MISMATCH_ERROR );
		} else {
			NET_OutOfBandPrint( NS_SERVER, from, "print\n[err_prot]" PROTOCOL_MISMATCH_ERROR );
		}
#else
		NET_OutOfBandPrint( NS_SERVER, from, "print\n[err_prot]" PROTOCOL_MISMATCH_ERROR );
#endif // RTCW_XX

		Com_DPrintf( "    rejected connect from version %i\n", version );
		return;
	}
#endif // RTCW_XX

#if defined RTCW_SP
	qport = atoi( Info_ValueForKey( userinfo, "qport" ) );
#endif // RTCW_XX

	challenge = atoi( Info_ValueForKey( userinfo, "challenge" ) );

#if !defined RTCW_SP
	qport = atoi( Info_ValueForKey( userinfo, "qport" ) );
#endif // RTCW_XX

#if defined RTCW_ET
	if ( SV_TempBanIsBanned( from ) ) {
		NET_OutOfBandPrint( NS_SERVER, from, "print\n%s\n", sv_tempbanmessage->string );
		return;
	}
#endif // RTCW_XX

	// quick reject
	for ( i = 0,cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {

#if defined RTCW_SP
		if ( cl->state == CS_FREE ) {
			continue;
		}
#elif defined RTCW_ET
		// DHM - Nerve :: This check was allowing clients to reconnect after zombietime(2 secs)
		//if ( cl->state == CS_FREE ) {
		//continue;
		//}
#endif // RTCW_XX

		if ( NET_CompareBaseAdr( from, cl->netchan.remoteAddress )
			 && ( cl->netchan.qport == qport
				  || from.port == cl->netchan.remoteAddress.port ) ) {
			if ( ( svs.time - cl->lastConnectTime )
				 < ( sv_reconnectlimit->integer * 1000 ) ) {
				Com_DPrintf( "%s:reconnect rejected : too soon\n", NET_AdrToString( from ) );
				return;
			}
			break;
		}
	}

	// see if the challenge is valid (LAN clients don't need to challenge)
	if ( !NET_IsLocalAddress( from ) ) {
		int ping;

		for ( i = 0 ; i < MAX_CHALLENGES ; i++ ) {
			if ( NET_CompareAdr( from, svs.challenges[i].adr ) ) {
				if ( challenge == svs.challenges[i].challenge ) {
					break;      // good
				}

#if defined RTCW_SP
				NET_OutOfBandPrint( NS_SERVER, from, "print\nBad challenge.\n" );
				return;
#endif // RTCW_XX

			}
		}
		if ( i == MAX_CHALLENGES ) {

#if defined RTCW_SP
			NET_OutOfBandPrint( NS_SERVER, from, "print\nNo challenge for address.\n" );
#elif defined RTCW_MP
			NET_OutOfBandPrint( NS_SERVER, from, "print\nNo or bad challenge for address.\n" );
#else
			NET_OutOfBandPrint( NS_SERVER, from, "print\n[err_dialog]No or bad challenge for address.\n" );
#endif // RTCW_XX

			return;
		}
		// force the IP key/value pair so the game can filter based on ip
		Info_SetValueForKey( userinfo, "ip", NET_AdrToString( from ) );

#if defined RTCW_SP
		ping = svs.time - svs.challenges[i].pingTime;
#else
		if ( svs.challenges[i].firstPing == 0 ) {
			ping = svs.time - svs.challenges[i].pingTime;
			svs.challenges[i].firstPing = ping;
		} else {
			ping = svs.challenges[i].firstPing;
		}
#endif // RTCW_XX

		Com_Printf( "Client %i connecting with %i challenge ping\n", i, ping );

#if !defined RTCW_SP
		svs.challenges[i].connected = qtrue;
#endif // RTCW_XX

		// never reject a LAN client based on ping
		if ( !Sys_IsLANAddress( from ) ) {
			if ( sv_minPing->value && ping < sv_minPing->value ) {

#if defined RTCW_SP
				// don't let them keep trying until they get a big delay
#endif // RTCW_XX

#if !defined RTCW_ET
				NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is for high pings only\n" );
#else
				NET_OutOfBandPrint( NS_SERVER, from, "print\n[err_dialog]Server is for high pings only\n" );
#endif // RTCW_XX

				Com_DPrintf( "Client %i rejected on a too low ping\n", i );

#if defined RTCW_SP
				// reset the address otherwise their ping will keep increasing
				// with each connect message and they'd eventually be able to connect
				svs.challenges[i].adr.port = 0;
#endif // RTCW_XX

				return;
			}
			if ( sv_maxPing->value && ping > sv_maxPing->value ) {

#if !defined RTCW_ET
				NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is for low pings only\n" );
#else
				NET_OutOfBandPrint( NS_SERVER, from, "print\n[err_dialog]Server is for low pings only\n" );
#endif // RTCW_XX

#if defined RTCW_SP
				Com_DPrintf( "Client %i rejected on a too high ping\n", i );
#else
				Com_DPrintf( "Client %i rejected on a too high ping: %i\n", i, ping );
#endif // RTCW_XX

				return;
			}
		}
	} else {
		// force the "ip" info key to "localhost"
		Info_SetValueForKey( userinfo, "ip", "localhost" );
	}

	newcl = &temp;
	memset( newcl, 0, sizeof( client_t ) );

	// if there is already a slot for this ip, reuse it
	for ( i = 0,cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
		if ( cl->state == CS_FREE ) {
			continue;
		}
		if ( NET_CompareBaseAdr( from, cl->netchan.remoteAddress )
			 && ( cl->netchan.qport == qport
				  || from.port == cl->netchan.remoteAddress.port ) ) {
			Com_Printf( "%s:reconnect\n", NET_AdrToString( from ) );
			newcl = cl;

#if defined RTCW_SP
			// disconnect the client from the game first so any flags the
			// player might have are dropped
			VM_Call(gvm, GAME_CLIENT_DISCONNECT, rtcw::to_vm_arg(newcl - svs.clients));
			//
#elif defined RTCW_ET
			// this doesn't work because it nukes the players userinfo

//			// disconnect the client from the game first so any flags the
//			// player might have are dropped
//			VM_Call( gvm, GAME_CLIENT_DISCONNECT, newcl - svs.clients );
			//
#endif // RTCW_XX

			goto gotnewcl;
		}
	}

	// find a client slot
	// if "sv_privateClients" is set > 0, then that number
	// of client slots will be reserved for connections that
	// have "password" set to the value of "sv_privatePassword"
	// Info requests will report the maxclients as if the private
	// slots didn't exist, to prevent people from trying to connect
	// to a full server.
	// This is to allow us to reserve a couple slots here on our
	// servers so we can play without having to kick people.

	// check for privateClient password
	password = Info_ValueForKey( userinfo, "password" );
	if ( !strcmp( password, sv_privatePassword->string ) ) {
		startIndex = 0;
	} else {
		// skip past the reserved slots
		startIndex = sv_privateClients->integer;
	}

	newcl = NULL;
	for ( i = startIndex; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];
		if ( cl->state == CS_FREE ) {
			newcl = cl;
			break;
		}
	}

	if ( !newcl ) {
		if ( NET_IsLocalAddress( from ) ) {
			count = 0;
			for ( i = startIndex; i < sv_maxclients->integer ; i++ ) {
				cl = &svs.clients[i];
				if ( cl->netchan.remoteAddress.type == NA_BOT ) {
					count++;
				}
			}
			// if they're all bots
			if ( count >= sv_maxclients->integer - startIndex ) {
				SV_DropClient( &svs.clients[sv_maxclients->integer - 1], "only bots on server" );
				newcl = &svs.clients[sv_maxclients->integer - 1];
			} else {
				Com_Error( ERR_FATAL, "server is full on local connect\n" );
				return;
			}
		} else {

#if !defined RTCW_ET
			NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is full.\n" );
#else
			NET_OutOfBandPrint( NS_SERVER, from, va( "print\n%s\n", sv_fullmsg->string ) );
#endif // RTCW_XX

			Com_DPrintf( "Rejected a connection.\n" );
			return;
		}
	}

	// we got a newcl, so reset the reliableSequence and reliableAcknowledge
	cl->reliableAcknowledge = 0;
	cl->reliableSequence = 0;

gotnewcl:
	// build a new connection
	// accept the new client
	// this is the only place a client_t is ever initialized
	*newcl = temp;
	clientNum = newcl - svs.clients;
	ent = SV_GentityNum( clientNum );
	newcl->gentity = ent;

	// save the challenge
	newcl->challenge = challenge;

	// save the address
	Netchan_Setup( NS_SERVER, &newcl->netchan, from, qport );

#if defined RTCW_MP
	// init the netchan queue
	newcl->netchan_end_queue = &newcl->netchan_start_queue;
#elif defined RTCW_ET
	// init the netchan queue
#endif // RTCW_XX

	// save the userinfo
	Q_strncpyz( newcl->userinfo, userinfo, sizeof( newcl->userinfo ) );

	// get the game a chance to reject this connection or modify the userinfo
	denied = rtcw::from_vm_arg<char*>(VM_Call(
		gvm,
		GAME_CLIENT_CONNECT,
		rtcw::to_vm_arg(clientNum),
		rtcw::to_vm_arg(qtrue),
		rtcw::to_vm_arg(qfalse)
	)); // firstTime = qtrue
	if ( denied ) {
		// we can't just use VM_ArgPtr, because that is only valid inside a VM_Call
		denied = rtcw::from_vm_arg<char*>(VM_ExplicitArgPtr(gvm, rtcw::to_vm_arg(denied)));

#if !defined RTCW_ET
		NET_OutOfBandPrint( NS_SERVER, from, "print\n%s\n", denied );
#else
		NET_OutOfBandPrint( NS_SERVER, from, "print\n[err_dialog]%s\n", denied );
#endif // RTCW_XX

		Com_DPrintf( "Game rejected a connection: %s.\n", denied );
		return;
	}

#if defined RTCW_SP
	// RF, create the reliable commands
	if ( newcl->netchan.remoteAddress.type != NA_BOT ) {
		SV_InitReliableCommandsForClient( newcl, MAX_RELIABLE_COMMANDS );
	} else {
		SV_InitReliableCommandsForClient( newcl, 0 );
	}
#endif // RTCW_XX

	SV_UserinfoChanged( newcl );

#if !defined RTCW_SP
	// DHM - Nerve :: Clear out firstPing now that client is connected
	svs.challenges[i].firstPing = 0;
#endif // RTCW_XX

	// send the connect packet to the client
	NET_OutOfBandPrint( NS_SERVER, from, "connectResponse" );

	Com_DPrintf( "Going from CS_FREE to CS_CONNECTED for %s\n", newcl->name );

	newcl->state = CS_CONNECTED;
	newcl->nextSnapshotTime = svs.time;
	newcl->lastPacketTime = svs.time;
	newcl->lastConnectTime = svs.time;

	// when we receive the first packet from the client, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	newcl->gamestateMessageNum = -1;

	// if this was the first client on the server, or the last client
	// the server can hold, send a heartbeat to the master.
	count = 0;
	for ( i = 0,cl = svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			count++;
		}
	}
	if ( count == 1 || count == sv_maxclients->integer ) {
		SV_Heartbeat_f();
	}
}

#if !defined RTCW_ET
/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing -- SV_FinalMessage() will handle that
=====================
*/
#else
/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing -- SV_FinalCommand() will handle that
=====================
*/
#endif // RTCW_XX

void SV_DropClient( client_t *drop, const char *reason ) {
	int i;

#if !defined RTCW_SP
	challenge_t *challenge;
#endif // RTCW_XX

#if defined RTCW_ET
	qboolean isBot = qfalse;
#endif // RTCW_XX

	if ( drop->state == CS_ZOMBIE ) {
		return;     // already dropped
	}

#if !defined RTCW_SP

#if !defined RTCW_ET
	if ( !drop->gentity || !( drop->gentity->r.svFlags & SVF_BOT ) ) {
#else
	if ( drop->gentity && ( drop->gentity->r.svFlags & SVF_BOT ) ) {
#endif // RTCW_XX

#if defined RTCW_ET
		isBot = qtrue;
	} else {
		if ( drop->netchan.remoteAddress.type == NA_BOT ) {
			isBot = qtrue;
		}
	}

	if ( !isBot ) {
#endif // RTCW_XX

		// see if we already have a challenge for this ip
		challenge = &svs.challenges[0];

		for ( i = 0 ; i < MAX_CHALLENGES ; i++, challenge++ ) {
			if ( NET_CompareAdr( drop->netchan.remoteAddress, challenge->adr ) ) {
				challenge->connected = qfalse;
				break;
			}
		}

#if !defined RTCW_ET
	}
#endif // RTCW_XX

#endif // RTCW_XX

	// Kill any download
	SV_CloseDownload( drop );

#if defined RTCW_ET
	}

	if ( ( !SV_GameIsSinglePlayer() ) || ( !isBot ) ) {
		// tell everyone why they got dropped

		// Gordon: we want this displayed elsewhere now
		SV_SendServerCommand( NULL, "cpm \"%s" S_COLOR_WHITE " %s\n\"", drop->name, reason );
//		SV_SendServerCommand( NULL, "print \"[lof]%s" S_COLOR_WHITE " [lon]%s\n\"", drop->name, reason );
	}
#endif // RTCW_XX

#if defined RTCW_SP
	// Ridah, no need to tell the player if an AI drops
	if ( !( drop->gentity && drop->gentity->r.svFlags & SVF_CASTAI ) ) {
		// tell everyone why they got dropped
		SV_SendServerCommand( NULL, "print \"%s" S_COLOR_WHITE " %s\n\"", drop->name, reason );
	}
#elif defined RTCW_MP
	// tell everyone why they got dropped
	SV_SendServerCommand( NULL, "print \"[lof]%s" S_COLOR_WHITE " [lon]%s\n\"", drop->name, reason );
#endif // RTCW_XX

	Com_DPrintf( "Going to CS_ZOMBIE for %s\n", drop->name );
	drop->state = CS_ZOMBIE;        // become free in a few seconds

	if ( drop->download ) {
		FS_FCloseFile( drop->download );
		drop->download = 0;
	}

	// call the prog function for removing a client
	// this will remove the body, among other things
	VM_Call(
		gvm,
		GAME_CLIENT_DISCONNECT,
		rtcw::to_vm_arg(drop - svs.clients)
	);

#if defined RTCW_SP
	// Ridah, no need to tell the player if an AI drops
	if ( !( drop->gentity && drop->gentity->r.svFlags & SVF_CASTAI ) ) {
		// add the disconnect command
		SV_SendServerCommand( drop, "disconnect" );
	}
	// done.
#elif defined RTCW_MP
	// add the disconnect command
	SV_SendServerCommand( drop, "disconnect \"%s\"", reason );
#else
	// add the disconnect command
	SV_SendServerCommand( drop, "disconnect \"%s\"\n", reason );
#endif // RTCW_XX

	if ( drop->netchan.remoteAddress.type == NA_BOT ) {
		SV_BotFreeClient( drop - svs.clients );
	}

	// nuke user info
	SV_SetUserinfo( drop - svs.clients, "" );

#if defined RTCW_SP
	// RF, nuke reliable commands
	SV_FreeReliableCommandsForClient( drop );
#endif // RTCW_XX

	// if this was the last client on the server, send a heartbeat
	// to the master so it is known the server is empty
	// send a heartbeat now so the master will get up to date info
	// if there is already a slot for this ip, reuse it
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			break;
		}
	}
	if ( i == sv_maxclients->integer ) {
		SV_Heartbeat_f();
	}
}

/*
================
SV_SendClientGameState

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each new map load.

It will be resent if the client acknowledges a later message but has
the wrong gamestate.
================
*/
void SV_SendClientGameState( client_t *client ) {
	int start;
	entityState_t   *base, nullstate;
	msg_t msg;
	byte msgBuffer[MAX_MSGLEN];

	Com_DPrintf( "SV_SendClientGameState() for %s\n", client->name );
	Com_DPrintf( "Going from CS_CONNECTED to CS_PRIMED for %s\n", client->name );
	client->state = CS_PRIMED;
	client->pureAuthentic = 0;

#if !defined RTCW_SP
	client->gotCP = qfalse;
#endif // RTCW_XX

	// when we receive the first packet from the client, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	client->gamestateMessageNum = client->netchan.outgoingSequence;

	MSG_Init( &msg, msgBuffer, sizeof( msgBuffer ) );

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand );

	// send any server commands waiting to be sent first.
	// we have to do this cause we send the client->reliableSequence
	// with a gamestate and it sets the clc.serverCommandSequence at
	// the client side
	SV_UpdateServerCommandsToClient( client, &msg );

	// send the gamestate
	MSG_WriteByte( &msg, svc_gamestate );
	MSG_WriteLong( &msg, client->reliableSequence );

	// write the configstrings
	for ( start = 0 ; start < MAX_CONFIGSTRINGS ; start++ ) {
		if ( sv.configstrings[start][0] ) {
			MSG_WriteByte( &msg, svc_configstring );
			MSG_WriteShort( &msg, start );
			MSG_WriteBigString( &msg, sv.configstrings[start] );
		}
	}

	// write the baselines
	memset( &nullstate, 0, sizeof( nullstate ) );
	for ( start = 0 ; start < MAX_GENTITIES; start++ ) {
		base = &sv.svEntities[start].baseline;
		if ( !base->number ) {
			continue;
		}
		MSG_WriteByte( &msg, svc_baseline );
		MSG_WriteDeltaEntity( &msg, &nullstate, base, qtrue );
	}

	MSG_WriteByte( &msg, svc_EOF );

	MSG_WriteLong( &msg, client - svs.clients );

	// write the checksum feed
	MSG_WriteLong( &msg, sv.checksumFeed );

#if !defined RTCW_SP
	// NERVE - SMF - debug info
	Com_DPrintf( "Sending %i bytes in gamestate to client: %i\n", msg.cursize, client - svs.clients );
#endif // RTCW_XX

	// deliver this to the client
	SV_SendMessageToClient( &msg, client );
}


/*
==================
SV_ClientEnterWorld
==================
*/
void SV_ClientEnterWorld( client_t *client, usercmd_t *cmd ) {
	int clientNum;
	sharedEntity_t *ent;

	Com_DPrintf( "Going from CS_PRIMED to CS_ACTIVE for %s\n", client->name );
	client->state = CS_ACTIVE;

	// set up the entity for the client
	clientNum = client - svs.clients;
	ent = SV_GentityNum( clientNum );
	ent->s.number = clientNum;
	client->gentity = ent;

	client->deltaMessage = -1;
	client->nextSnapshotTime = svs.time;    // generate a snapshot immediately
	client->lastUsercmd = *cmd;

	// call the game begin function
	VM_Call(
		gvm,
		GAME_CLIENT_BEGIN,
		rtcw::to_vm_arg(client - svs.clients)
	);
}

/*
============================================================

CLIENT COMMAND EXECUTION

============================================================
*/

/*
==================
SV_CloseDownload

clear/free any download vars
==================
*/
static void SV_CloseDownload( client_t *cl ) {
	int i;

	// EOF
	if ( cl->download ) {
		FS_FCloseFile( cl->download );
	}
	cl->download = 0;
	*cl->downloadName = 0;

	// Free the temporary buffer space
	for ( i = 0; i < MAX_DOWNLOAD_WINDOW; i++ ) {
		if ( cl->downloadBlocks[i] ) {
			Z_Free( cl->downloadBlocks[i] );
			cl->downloadBlocks[i] = NULL;
		}
	}

}

/*
==================
SV_StopDownload_f

Abort a download if in progress
==================
*/
void SV_StopDownload_f( client_t *cl ) {
	if ( *cl->downloadName ) {
		Com_DPrintf( "clientDownload: %d : file \"%s\" aborted\n", cl - svs.clients, cl->downloadName );
	}

	SV_CloseDownload( cl );
}

/*
==================
SV_DoneDownload_f

Downloads are finished
==================
*/
void SV_DoneDownload_f( client_t *cl ) {
	Com_DPrintf( "clientDownload: %s Done\n", cl->name );
	// resend the game state to update any clients that entered during the download
	SV_SendClientGameState( cl );
}

/*
==================
SV_NextDownload_f

The argument will be the last acknowledged block from the client, it should be
the same as cl->downloadClientBlock
==================
*/
void SV_NextDownload_f( client_t *cl ) {
	int block = atoi( Cmd_Argv( 1 ) );

	if ( block == cl->downloadClientBlock ) {
		Com_DPrintf( "clientDownload: %d : client acknowledge of block %d\n", cl - svs.clients, block );

		// Find out if we are done.  A zero-length block indicates EOF
		if ( cl->downloadBlockSize[cl->downloadClientBlock % MAX_DOWNLOAD_WINDOW] == 0 ) {
			Com_Printf( "clientDownload: %d : file \"%s\" completed\n", cl - svs.clients, cl->downloadName );
			SV_CloseDownload( cl );
			return;
		}

		cl->downloadSendTime = svs.time;
		cl->downloadClientBlock++;
		return;
	}
	// We aren't getting an acknowledge for the correct block, drop the client
	// FIXME: this is bad... the client will never parse the disconnect message
	//			because the cgame isn't loaded yet
	SV_DropClient( cl, "broken download" );
}

/*
==================
SV_BeginDownload_f
==================
*/
void SV_BeginDownload_f( client_t *cl ) {

	// Kill any existing download
	SV_CloseDownload( cl );

#if defined RTCW_ET
	//bani - stop us from printing dupe messages
	if ( strcmp( cl->downloadName, Cmd_Argv( 1 ) ) ) {
		cl->downloadnotify = DLNOTIFY_ALL;
	}
#endif // RTCW_XX

	// cl->downloadName is non-zero now, SV_WriteDownloadToClient will see this and open
	// the file itself
	Q_strncpyz( cl->downloadName, Cmd_Argv( 1 ), sizeof( cl->downloadName ) );
}

#if defined RTCW_ET
/*
==================
SV_WWWDownload_f
==================
*/
void SV_WWWDownload_f( client_t *cl ) {

	const char *subcmd = Cmd_Argv( 1 );

	// only accept wwwdl commands for clients which we first flagged as wwwdl ourselves
	if ( !cl->bWWWDl ) {
		Com_Printf( "SV_WWWDownload: unexpected wwwdl '%s' for client '%s'\n", subcmd, cl->name );
		SV_DropClient( cl, va( "SV_WWWDownload: unexpected wwwdl %s", subcmd ) );
		return;
	}

	if ( !Q_stricmp( subcmd, "ack" ) ) {
		if ( cl->bWWWing ) {
			Com_Printf( "WARNING: dupe wwwdl ack from client '%s'\n", cl->name );
		}
		cl->bWWWing = qtrue;
		return;
	} else if ( !Q_stricmp( subcmd, "bbl8r" ) ) {
		SV_DropClient( cl, "acking disconnected download mode" );
		return;
	}

	// below for messages that only happen during/after download
	if ( !cl->bWWWing ) {
		Com_Printf( "SV_WWWDownload: unexpected wwwdl '%s' for client '%s'\n", subcmd, cl->name );
		SV_DropClient( cl, va( "SV_WWWDownload: unexpected wwwdl %s", subcmd ) );
		return;
	}

	if ( !Q_stricmp( subcmd, "done" ) ) {
		cl->download = 0;
		*cl->downloadName = 0;
		cl->bWWWing = qfalse;
		return;
	} else if ( !Q_stricmp( subcmd, "fail" ) )        {
		cl->download = 0;
		*cl->downloadName = 0;
		cl->bWWWing = qfalse;
		cl->bFallback = qtrue;
		// send a reconnect
		SV_SendClientGameState( cl );
		return;
	} else if ( !Q_stricmp( subcmd, "chkfail" ) )        {
		Com_Printf( "WARNING: client '%s' reports that the redirect download for '%s' had wrong checksum.\n", cl->name, cl->downloadName );
		Com_Printf( "         you should check your download redirect configuration.\n" );
		cl->download = 0;
		*cl->downloadName = 0;
		cl->bWWWing = qfalse;
		cl->bFallback = qtrue;
		// send a reconnect
		SV_SendClientGameState( cl );
		return;
	}

	Com_Printf( "SV_WWWDownload: unknown wwwdl subcommand '%s' for client '%s'\n", subcmd, cl->name );
	SV_DropClient( cl, va( "SV_WWWDownload: unknown wwwdl subcommand '%s'", subcmd ) );
}

// abort an attempted download
void SV_BadDownload( client_t *cl, msg_t *msg ) {
	MSG_WriteByte( msg, svc_download );
	MSG_WriteShort( msg, 0 ); // client is expecting block zero
	MSG_WriteLong( msg, -1 ); // illegal file size

	*cl->downloadName = 0;
}

/*
==================
SV_CheckFallbackURL

sv_wwwFallbackURL can be used to redirect clients to a web URL in case direct ftp/http didn't work (or is disabled on client's end)
return true when a redirect URL message was filled up
when the cvar is set to something, the download server will effectively never use a legacy download strategy
==================
*/
static qboolean SV_CheckFallbackURL( client_t *cl, msg_t *msg ) {
	if ( !sv_wwwFallbackURL->string || strlen( sv_wwwFallbackURL->string ) == 0 ) {
		return qfalse;
	}

	Com_Printf( "clientDownload: sending client '%s' to fallback URL '%s'\n", cl->name, sv_wwwFallbackURL->string );

	MSG_WriteByte( msg, svc_download );
	MSG_WriteShort( msg, -1 ); // block -1 means ftp/http download
	MSG_WriteString( msg, sv_wwwFallbackURL->string );
	MSG_WriteLong( msg, 0 );
	MSG_WriteLong( msg, 2 ); // DL_FLAG_URL

	return qtrue;
}
#endif // RTCW_XX

/*
==================
SV_WriteDownloadToClient

Check to see if the client wants a file, open it if needed and start pumping the client
Fill up msg with data
==================
*/

void SV_WriteDownloadToClient( client_t *cl, msg_t *msg ) {
	int curindex;
	int rate;
	int blockspersnap;

#if defined RTCW_SP
	int idPack, missionPack;
#else
	int idPack;
#endif // RTCW_XX

	char errorMessage[1024];

#if defined RTCW_ET
	int download_flag;
#endif // RTCW_XX

#if !defined RTCW_SP

#if !defined RTCW_ET && defined UPDATE_SERVER
	int i;
	char testname[MAX_QPATH];
#endif

	qboolean bTellRate = qfalse; // verbosity
#endif // RTCW_XX

	if ( !*cl->downloadName ) {
		return; // Nothing being downloaded

#if !defined RTCW_SP
	}

#if defined RTCW_ET
	if ( cl->bWWWing ) {
		return; // The client acked and is downloading with ftp/http

	}
#endif // RTCW_XX

	// CVE-2006-2082
	// validate the download against the list of pak files
	if ( !FS_VerifyPak( cl->downloadName ) ) {
		// will drop the client and leave it hanging on the other side. good for him
		SV_DropClient( cl, "illegal download request" );
		return;
#endif // RTCW_XX

	}

	if ( !cl->download ) {
		// We open the file here

#if !defined RTCW_ET
		Com_Printf( "clientDownload: %d : begining \"%s\"\n", cl - svs.clients, cl->downloadName );
#else
		//bani - prevent duplicate download notifications
		if ( cl->downloadnotify & DLNOTIFY_BEGIN ) {
			cl->downloadnotify &= ~DLNOTIFY_BEGIN;
			Com_Printf( "clientDownload: %d : beginning \"%s\"\n", cl - svs.clients, cl->downloadName );
		}
#endif // RTCW_XX

#if defined RTCW_SP
		missionPack = FS_idPak( cl->downloadName, "missionpack" );
		idPack = missionPack || FS_idPak( cl->downloadName, "baseq3" );
#elif defined RTCW_MP
		idPack = FS_idPak( cl->downloadName, "main" );

		// DHM - Nerve :: Update server only allows files that are in versionmap.cfg to download
#ifdef UPDATE_SERVER
		for ( i = 0; i < numVersions; i++ ) {

			strcpy( testname, "updates/" );
			Q_strcat( testname, MAX_QPATH, versionMap[ i ].installer );

			if ( !Q_stricmp( cl->downloadName, testname ) ) {
				break;
			}
		}

		if ( i == numVersions ) {
			MSG_WriteByte( msg, svc_download );
			MSG_WriteShort( msg, 0 ); // client is expecting block zero
			MSG_WriteLong( msg, -1 ); // illegal file size

			Com_sprintf( errorMessage, sizeof( errorMessage ), "Invalid download from update server" );
			MSG_WriteString( msg, errorMessage );

			*cl->downloadName = 0;

			SV_DropClient( cl, "Invalid download from update server" );
			return;
		}
#endif
		// DHM - Nerve
#endif // RTCW_XX

#if defined RTCW_ET
		idPack = FS_idPak( cl->downloadName, BASEGAME );

		// sv_allowDownload and idPack checks
#endif // RTCW_XX

#if !defined RTCW_ET
		if ( !sv_allowDownload->integer || idPack ||
			 ( cl->downloadSize = FS_SV_FOpenFileRead( cl->downloadName, &cl->download ) ) <= 0 ) {
#else
		if ( !sv_allowDownload->integer || idPack ) {
#endif // RTCW_XX

			// cannot auto-download file
			if ( idPack ) {
				Com_Printf( "clientDownload: %d : \"%s\" cannot download id pk3 files\n", cl - svs.clients, cl->downloadName );

#if defined RTCW_SP
				if ( missionPack ) {
					Com_sprintf( errorMessage, sizeof( errorMessage ), "Cannot autodownload Team Arena file \"%s\"\n"
																	   "The Team Arena mission pack can be found in your local game store.", cl->downloadName );
				} else {
					Com_sprintf( errorMessage, sizeof( errorMessage ), "Cannot autodownload id pk3 file \"%s\"", cl->downloadName );
				}
#elif defined RTCW_MP
				Com_sprintf( errorMessage, sizeof( errorMessage ), "Cannot autodownload id pk3 file \"%s\"", cl->downloadName );
#else
				Com_sprintf( errorMessage, sizeof( errorMessage ), "Cannot autodownload official pk3 file \"%s\"", cl->downloadName );
#endif // RTCW_XX

#if !defined RTCW_ET
			} else if ( !sv_allowDownload->integer ) {
#else
			} else {
#endif // RTCW_XX

				Com_Printf( "clientDownload: %d : \"%s\" download disabled", cl - svs.clients, cl->downloadName );
				if ( sv_pure->integer ) {
					Com_sprintf( errorMessage, sizeof( errorMessage ), "Could not download \"%s\" because autodownloading is disabled on the server.\n\n"
																	   "You will need to get this file elsewhere before you "
																	   "can connect to this pure server.\n", cl->downloadName );
				} else {
					Com_sprintf( errorMessage, sizeof( errorMessage ), "Could not download \"%s\" because autodownloading is disabled on the server.\n\n"
																	   "Set autodownload to No in your settings and you might be "

#if defined RTCW_SP
																	   "able to connect if you do have the file.\n", cl->downloadName );
#elif defined RTCW_MP
																	   "able to connect even if you do have the file.\n", cl->downloadName );
#else
																	   "able to connect even if you don't have the file.\n", cl->downloadName );
#endif // RTCW_XX
#if !defined RTCW_ET
				}
			} else {
				Com_Printf( "clientDownload: %d : \"%s\" file not found on server\n", cl - svs.clients, cl->downloadName );
				Com_sprintf( errorMessage, sizeof( errorMessage ), "File \"%s\" not found on server for autodownloading.\n", cl->downloadName );
			}
			MSG_WriteByte( msg, svc_download );
			MSG_WriteShort( msg, 0 ); // client is expecting block zero
			MSG_WriteLong( msg, -1 ); // illegal file size
			MSG_WriteString( msg, errorMessage );

			*cl->downloadName = 0;
			return;
		}

		// Init
#endif // RTCW_XX

#if defined RTCW_ET
				}
			}

			SV_BadDownload( cl, msg );
			MSG_WriteString( msg, errorMessage ); // (could SV_DropClient isntead?)

			return;
		}

		// www download redirect protocol
		// NOTE: this is called repeatedly while a client connects. Maybe we should sort of cache the message or something
		// FIXME: we need to abstract this to an independant module for maximum configuration/usability by server admins
		// FIXME: I could rework that, it's crappy
		if ( sv_wwwDownload->integer ) {
			if ( cl->bDlOK ) {
				if ( !cl->bFallback ) {
					fileHandle_t handle;
					int downloadSize = FS_SV_FOpenFileRead( cl->downloadName, &handle );
					if ( downloadSize ) {
						FS_FCloseFile( handle ); // don't keep open, we only care about the size

						Q_strncpyz( cl->downloadURL, va( "%s/%s", sv_wwwBaseURL->string, cl->downloadName ), sizeof( cl->downloadURL ) );

						//bani - prevent multiple download notifications
						if ( cl->downloadnotify & DLNOTIFY_REDIRECT ) {
							cl->downloadnotify &= ~DLNOTIFY_REDIRECT;
							Com_Printf( "Redirecting client '%s' to %s\n", cl->name, cl->downloadURL );
						}
						// once cl->downloadName is set (and possibly we have our listening socket), let the client know
						cl->bWWWDl = qtrue;
						MSG_WriteByte( msg, svc_download );
						MSG_WriteShort( msg, -1 ); // block -1 means ftp/http download
						// compatible with legacy svc_download protocol: [size] [size bytes]
						// download URL, size of the download file, download flags
						MSG_WriteString( msg, cl->downloadURL );
						MSG_WriteLong( msg, downloadSize );
						download_flag = 0;
						if ( sv_wwwDlDisconnected->integer ) {
							download_flag |= ( 1 << DL_FLAG_DISCON );
						}
						MSG_WriteLong( msg, download_flag ); // flags
						return;
					} else {
						// that should NOT happen - even regular download would fail then anyway
						Com_Printf( "ERROR: Client '%s': couldn't extract file size for %s\n", cl->name, cl->downloadName );
					}
				} else {
					cl->bFallback = qfalse;
					if ( SV_CheckFallbackURL( cl, msg ) ) {
						return;
					}
					Com_Printf( "Client '%s': falling back to regular downloading for failed file %s\n", cl->name, cl->downloadName );
				}
			} else {
				if ( SV_CheckFallbackURL( cl, msg ) ) {
					return;
				}
				Com_Printf( "Client '%s' is not configured for www download\n", cl->name );
			}
		}

		// find file
		cl->bWWWDl = qfalse;
		cl->downloadSize = FS_SV_FOpenFileRead( cl->downloadName, &cl->download );
		if ( cl->downloadSize <= 0 ) {
			Com_Printf( "clientDownload: %d : \"%s\" file not found on server\n", cl - svs.clients, cl->downloadName );
			Com_sprintf( errorMessage, sizeof( errorMessage ), "File \"%s\" not found on server for autodownloading.\n", cl->downloadName );
			SV_BadDownload( cl, msg );
			MSG_WriteString( msg, errorMessage ); // (could SV_DropClient isntead?)
			return;
		}

		// is valid source, init
#endif // RTCW_XX

		cl->downloadCurrentBlock = cl->downloadClientBlock = cl->downloadXmitBlock = 0;
		cl->downloadCount = 0;
		cl->downloadEOF = qfalse;

#if !defined RTCW_SP
		bTellRate = qtrue;
#endif // RTCW_XX

	}

	// Perform any reads that we need to
	while ( cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW &&
			cl->downloadSize != cl->downloadCount ) {

		curindex = ( cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW );

		if ( !cl->downloadBlocks[curindex] ) {
			cl->downloadBlocks[curindex] = static_cast<byte*> (Z_Malloc( MAX_DOWNLOAD_BLKSIZE ));
		}

		cl->downloadBlockSize[curindex] = FS_Read( cl->downloadBlocks[curindex], MAX_DOWNLOAD_BLKSIZE, cl->download );

		if ( cl->downloadBlockSize[curindex] < 0 ) {
			// EOF right now
			cl->downloadCount = cl->downloadSize;
			break;
		}

		cl->downloadCount += cl->downloadBlockSize[curindex];

		// Load in next block
		cl->downloadCurrentBlock++;
	}

	// Check to see if we have eof condition and add the EOF block
	if ( cl->downloadCount == cl->downloadSize &&
		 !cl->downloadEOF &&
		 cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW ) {

		cl->downloadBlockSize[cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW] = 0;
		cl->downloadCurrentBlock++;

		cl->downloadEOF = qtrue;  // We have added the EOF block
	}

	// Loop up to window size times based on how many blocks we can fit in the
	// client snapMsec and rate

	// based on the rate, how many bytes can we fit in the snapMsec time of the client
	// normal rate / snapshotMsec calculation
	rate = cl->rate;

#if defined RTCW_SP
	if ( sv_maxRate->integer ) {
		if ( sv_maxRate->integer < 1000 ) {
			Cvar_Set( "sv_MaxRate", "1000" );
		}
		if ( sv_maxRate->integer < rate ) {
			rate = sv_maxRate->integer;
		}
	}
#else
	// show_bug.cgi?id=509
	// for autodownload, we use a seperate max rate value
	// we do this everytime because the client might change it's rate during the download
	if ( sv_dl_maxRate->integer < rate ) {
		rate = sv_dl_maxRate->integer;
		if ( bTellRate ) {
			Com_Printf( "'%s' downloading at sv_dl_maxrate (%d)\n", cl->name, sv_dl_maxRate->integer );
		}
	} else
	if ( bTellRate ) {
		Com_Printf( "'%s' downloading at rate %d\n", cl->name, rate );
	}
#endif // RTCW_XX

	if ( !rate ) {
		blockspersnap = 1;
	} else {
		blockspersnap = ( ( rate * cl->snapshotMsec ) / 1000 + MAX_DOWNLOAD_BLKSIZE ) /
						MAX_DOWNLOAD_BLKSIZE;
	}

	if ( blockspersnap < 0 ) {
		blockspersnap = 1;
	}

	while ( blockspersnap-- ) {

		// Write out the next section of the file, if we have already reached our window,
		// automatically start retransmitting

		if ( cl->downloadClientBlock == cl->downloadCurrentBlock ) {
			return; // Nothing to transmit

		}
		if ( cl->downloadXmitBlock == cl->downloadCurrentBlock ) {
			// We have transmitted the complete window, should we start resending?

			//FIXME:  This uses a hardcoded one second timeout for lost blocks
			//the timeout should be based on client rate somehow
			if ( svs.time - cl->downloadSendTime > 1000 ) {
				cl->downloadXmitBlock = cl->downloadClientBlock;
			} else {
				return;
			}
		}

		// Send current block
		curindex = ( cl->downloadXmitBlock % MAX_DOWNLOAD_WINDOW );

		MSG_WriteByte( msg, svc_download );
		MSG_WriteShort( msg, cl->downloadXmitBlock );

		// block zero is special, contains file size
		if ( cl->downloadXmitBlock == 0 ) {
			MSG_WriteLong( msg, cl->downloadSize );
		}

		MSG_WriteShort( msg, cl->downloadBlockSize[curindex] );

		// Write the block
		if ( cl->downloadBlockSize[curindex] ) {
			MSG_WriteData( msg, cl->downloadBlocks[curindex], cl->downloadBlockSize[curindex] );
		}

		Com_DPrintf( "clientDownload: %d : writing block %d\n", cl - svs.clients, cl->downloadXmitBlock );

		// Move on to the next block
		// It will get sent with next snap shot.  The rate will keep us in line.
		cl->downloadXmitBlock++;

		cl->downloadSendTime = svs.time;
	}
}

/*
=================
SV_Disconnect_f

The client is going to disconnect, so remove the connection immediately  FIXME: move to game?
=================
*/
static void SV_Disconnect_f( client_t *cl ) {
	SV_DropClient( cl, "disconnected" );
}

#if defined RTCW_SP
/*
=================
SV_VerifyPaks_f

If we are pure, disconnect the client if they do no meet the following conditions:

1. the first two checksums match our view of cgame and ui
2. there are no any additional checksums that we do not have

This routine would be a bit simpler with a goto but i abstained

=================
*/
#else
/*
=================
SV_VerifyPaks_f

If we are pure, disconnect the client if they do no meet the following conditions:

1. the first two checksums match our view of cgame and ui DLLs
   Wolf specific: the checksum is the checksum of the pk3 we found the DLL in
2. there are no any additional checksums that we do not have

This routine would be a bit simpler with a goto but i abstained

=================
*/
#endif // RTCW_XX

static void SV_VerifyPaks_f( client_t *cl ) {
	int nChkSum1, nChkSum2, nClientPaks, nServerPaks, i, j, nCurArg;
	int nClientChkSum[1024];
	int nServerChkSum[1024];
	const char *pPaks, *pArg;
	qboolean bGood = qtrue;

	// if we are pure, we "expect" the client to load certain things from
	// certain pk3 files, namely we want the client to have loaded the
	// ui and cgame that we think should be loaded based on the pure setting
	//
	if ( sv_pure->integer != 0 ) {

		bGood = qtrue;
		nChkSum1 = nChkSum2 = 0;

#if defined RTCW_SP
		// we run the game, so determine which cgame and ui the client "should" be running
		bGood = ( FS_FileIsInPAK( "vm/cgame.qvm", &nChkSum1 ) == 1 );
		if ( bGood ) {
			bGood = ( FS_FileIsInPAK( "vm/ui.qvm", &nChkSum2 ) == 1 );
		}

		nClientPaks = Cmd_Argc();

		// start at arg 1 ( skip cl_paks )
		nCurArg = 1;
#else
		bGood = ( FS_FileIsInPAK( FS_ShiftStr( SYS_DLLNAME_CGAME, -SYS_DLLNAME_CGAME_SHIFT ), &nChkSum1 ) == 1 );
		if ( bGood ) {
			bGood = ( FS_FileIsInPAK( FS_ShiftStr( SYS_DLLNAME_UI, -SYS_DLLNAME_UI_SHIFT ), &nChkSum2 ) == 1 );
		}

		nClientPaks = Cmd_Argc();

		// start at arg 2 ( skip serverId cl_paks )
		nCurArg = 1;

		pArg = Cmd_Argv( nCurArg++ );

		if ( !pArg ) {
			bGood = qfalse;
		} else
		{
			// show_bug.cgi?id=475
			// we may get incoming cp sequences from a previous checksumFeed, which we need to ignore
			// since serverId is a frame count, it always goes up
			if ( atoi( pArg ) < sv.checksumFeedServerId ) {
				Com_DPrintf( "ignoring outdated cp command from client %s\n", cl->name );
				return;
			}
		}
#endif // RTCW_XX

		// we basically use this while loop to avoid using 'goto' :)
		while ( bGood ) {

			// must be at least 6: "cl_paks cgame ui @ firstref ... numChecksums"
			// numChecksums is encoded
			if ( nClientPaks < 6 ) {
				bGood = qfalse;
				break;
			}
			// verify first to be the cgame checksum
			pArg = Cmd_Argv( nCurArg++ );
			if ( !pArg || *pArg == '@' || atoi( pArg ) != nChkSum1 ) {
				bGood = qfalse;
				break;
			}
			// verify the second to be the ui checksum
			pArg = Cmd_Argv( nCurArg++ );
			if ( !pArg || *pArg == '@' || atoi( pArg ) != nChkSum2 ) {
				bGood = qfalse;
				break;
			}
			// should be sitting at the delimeter now
			pArg = Cmd_Argv( nCurArg++ );
			if ( *pArg != '@' ) {
				bGood = qfalse;
				break;
			}
			// store checksums since tokenization is not re-entrant
			for ( i = 0; nCurArg < nClientPaks; i++ ) {
				nClientChkSum[i] = atoi( Cmd_Argv( nCurArg++ ) );
			}

			// store number to compare against (minus one cause the last is the number of checksums)
			nClientPaks = i - 1;

			// make sure none of the client check sums are the same
			// so the client can't send 5 the same checksums
			for ( i = 0; i < nClientPaks; i++ ) {
				for ( j = 0; j < nClientPaks; j++ ) {
					if ( i == j ) {
						continue;
					}
					if ( nClientChkSum[i] == nClientChkSum[j] ) {
						bGood = qfalse;
						break;
					}
				}
				if ( bGood == qfalse ) {
					break;
				}
			}
			if ( bGood == qfalse ) {
				break;
			}

			// get the pure checksums of the pk3 files loaded by the server
			pPaks = FS_LoadedPakPureChecksums();
			Cmd_TokenizeString( pPaks );
			nServerPaks = Cmd_Argc();
			if ( nServerPaks > 1024 ) {
				nServerPaks = 1024;
			}

			for ( i = 0; i < nServerPaks; i++ ) {
				nServerChkSum[i] = atoi( Cmd_Argv( i ) );
			}

			// check if the client has provided any pure checksums of pk3 files not loaded by the server
			for ( i = 0; i < nClientPaks; i++ ) {
				for ( j = 0; j < nServerPaks; j++ ) {
					if ( nClientChkSum[i] == nServerChkSum[j] ) {
						break;
					}
				}
				if ( j >= nServerPaks ) {
					bGood = qfalse;
					break;
				}
			}
			if ( bGood == qfalse ) {
				break;
			}

			// check if the number of checksums was correct
			nChkSum1 = sv.checksumFeed;
			for ( i = 0; i < nClientPaks; i++ ) {
				nChkSum1 ^= nClientChkSum[i];
			}
			nChkSum1 ^= nClientPaks;
			if ( nChkSum1 != nClientChkSum[nClientPaks] ) {
				bGood = qfalse;
				break;
			}

			// break out
			break;
		}

#if !defined RTCW_SP
		cl->gotCP = qtrue;
#endif // RTCW_XX

		if ( bGood ) {
			cl->pureAuthentic = 1;
		} else {
			cl->pureAuthentic = 0;
			cl->nextSnapshotTime = -1;
			cl->state = CS_ACTIVE;
			SV_SendClientSnapshot( cl );
			SV_DropClient( cl, "Unpure client detected. Invalid .PK3 files referenced!" );
		}
	}
}

/*
=================
SV_ResetPureClient_f
=================
*/
static void SV_ResetPureClient_f( client_t *cl ) {
	cl->pureAuthentic = 0;

#if !defined RTCW_SP
	cl->gotCP = qfalse;
#endif // RTCW_XX

}

/*
=================
SV_UserinfoChanged

Pull specific info from a newly changed userinfo string
into a more C friendly form.
=================
*/
void SV_UserinfoChanged( client_t *cl ) {
	const char    *val;
	int i;

	// name for C code
	Q_strncpyz( cl->name, Info_ValueForKey( cl->userinfo, "name" ), sizeof( cl->name ) );

	// rate command

	// if the client is on the same subnet as the server and we aren't running an
	// internet public server, assume they don't need a rate choke

#if defined RTCW_SP
	if ( Sys_IsLANAddress( cl->netchan.remoteAddress ) && com_dedicated->integer != 2 ) {
#else
	if ( Sys_IsLANAddress( cl->netchan.remoteAddress ) && com_dedicated->integer != 2 && sv_lanForceRate->integer == 1 ) {
#endif // RTCW_XX

		cl->rate = 99999;   // lans should not rate limit
	} else {
		val = Info_ValueForKey( cl->userinfo, "rate" );
		if ( strlen( val ) ) {
			i = atoi( val );
			cl->rate = i;
			if ( cl->rate < 1000 ) {
				cl->rate = 1000;
			} else if ( cl->rate > 90000 ) {
				cl->rate = 90000;
			}
		} else {

#if defined RTCW_SP
			cl->rate = 3000;
#else
			cl->rate = 5000;
#endif // RTCW_XX

		}
	}
	val = Info_ValueForKey( cl->userinfo, "handicap" );
	if ( strlen( val ) ) {
		i = atoi( val );

#if !defined RTCW_ET
		if ( i <= 0 || i > 100 || strlen( val ) > 4 ) {
			Info_SetValueForKey( cl->userinfo, "handicap", "100" );
#else
		if ( i <= -100 || i > 100 || strlen( val ) > 4 ) {
			Info_SetValueForKey( cl->userinfo, "handicap", "0" );
#endif // RTCW_XX

		}
	}

	// snaps command
	val = Info_ValueForKey( cl->userinfo, "snaps" );
	if ( strlen( val ) ) {
		i = atoi( val );
		if ( i < 1 ) {
			i = 1;
		} else if ( i > 30 ) {
			i = 30;
		}
		cl->snapshotMsec = 1000 / i;
	} else {
		cl->snapshotMsec = 50;
	}

#if !defined RTCW_SP
	// TTimo
	// maintain the IP information
	// this is set in SV_DirectConnect (directly on the server, not transmitted), may be lost when client updates it's userinfo
	// the banning code relies on this being consistently present

#if defined RTCW_ET
	// zinx - modified to always keep this consistent, instead of only
	// when "ip" is 0-length, so users can't supply their own IP
#endif // RTCW_XX

#if !defined RTCW_ET
	val = Info_ValueForKey( cl->userinfo, "ip" );
	if ( !val[0] ) {
#endif // RTCW_XX

		//Com_DPrintf("Maintain IP in userinfo for '%s'\n", cl->name);
		if ( !NET_IsLocalAddress( cl->netchan.remoteAddress ) ) {
			Info_SetValueForKey( cl->userinfo, "ip", NET_AdrToString( cl->netchan.remoteAddress ) );
		} else {
			// force the "ip" info key to "localhost" for local clients
			Info_SetValueForKey( cl->userinfo, "ip", "localhost" );
		}

#if defined RTCW_ET
	// TTimo
	// download prefs of the client
	val = Info_ValueForKey( cl->userinfo, "cl_wwwDownload" );
	cl->bDlOK = qfalse;
	if ( strlen( val ) ) {
		i = atoi( val );
		if ( i != 0 ) {
			cl->bDlOK = qtrue;
		}
#endif // RTCW_XX

	}
#endif // RTCW_XX

}


/*
==================
SV_UpdateUserinfo_f
==================
*/
static void SV_UpdateUserinfo_f( client_t *cl ) {
	Q_strncpyz( cl->userinfo, Cmd_Argv( 1 ), sizeof( cl->userinfo ) );

	SV_UserinfoChanged( cl );
	// call prog code to allow overrides
	VM_Call(
		gvm,
		GAME_CLIENT_USERINFO_CHANGED,
		rtcw::to_vm_arg(cl - svs.clients)
	);
}



typedef struct {
	const char    *name;
	void ( *func )( client_t *cl );

#if defined RTCW_ET
	qboolean allowedpostmapchange;
#endif // RTCW_XX

} ucmd_t;

static ucmd_t ucmds[] = {

#if !defined RTCW_ET
	{"userinfo", SV_UpdateUserinfo_f},
	{"disconnect", SV_Disconnect_f},
	{"cp", SV_VerifyPaks_f},
	{"vdr", SV_ResetPureClient_f},
	{"download", SV_BeginDownload_f},
	{"nextdl", SV_NextDownload_f},
	{"stopdl", SV_StopDownload_f},
	{"donedl", SV_DoneDownload_f},
#else
	{"userinfo", SV_UpdateUserinfo_f,    qfalse },
	{"disconnect",   SV_Disconnect_f,        qtrue },
	{"cp",           SV_VerifyPaks_f,        qfalse },
	{"vdr",          SV_ResetPureClient_f,   qfalse },
	{"download", SV_BeginDownload_f,     qfalse },
	{"nextdl",       SV_NextDownload_f,      qfalse },
	{"stopdl",       SV_StopDownload_f,      qfalse },
	{"donedl",       SV_DoneDownload_f,      qfalse },
	{"wwwdl",        SV_WWWDownload_f,       qfalse },
#endif // RTCW_XX

	{NULL, NULL}
};

/*
==================
SV_ExecuteClientCommand

Also called by bot code
==================
*/

#if !defined RTCW_ET
void SV_ExecuteClientCommand( client_t *cl, const char *s, qboolean clientOK ) {
#else
void SV_ExecuteClientCommand( client_t *cl, const char *s, qboolean clientOK, qboolean premaprestart ) {
#endif // RTCW_XX

	ucmd_t  *u;

#if !defined RTCW_SP
	qboolean bProcessed = qfalse;
#endif // RTCW_XX

	Cmd_TokenizeString( s );

	// see if it is a server level command
	for ( u = ucmds ; u->name ; u++ ) {
		if ( !strcmp( Cmd_Argv( 0 ), u->name ) ) {

#if defined RTCW_ET
			if ( premaprestart && !u->allowedpostmapchange ) {
				continue;
			}
#endif // RTCW_XX

			u->func( cl );

#if !defined RTCW_SP
			bProcessed = qtrue;
#endif // RTCW_XX

			break;
		}
	}

	if ( clientOK ) {
		// pass unknown strings to the game
		if ( !u->name && sv.state == SS_GAME ) {
			VM_Call(
				gvm,
				GAME_CLIENT_COMMAND,
				rtcw::to_vm_arg(cl - svs.clients)
			);
		}

#if !defined RTCW_SP
	} else if ( !bProcessed )     {
		Com_DPrintf( "client text ignored for %s: %s\n", cl->name, Cmd_Argv( 0 ) );
#endif // RTCW_XX

	}
}

/*
===============
SV_ClientCommand
===============
*/

#if !defined RTCW_ET
static qboolean SV_ClientCommand( client_t *cl, msg_t *msg ) {
#else
static qboolean SV_ClientCommand( client_t *cl, msg_t *msg, qboolean premaprestart ) {
#endif // RTCW_XX

	int seq;
	const char  *s;
	qboolean clientOk = qtrue;

#if !defined RTCW_SP
	qboolean floodprotect = qtrue;
#endif // RTCW_XX

	seq = MSG_ReadLong( msg );
	s = MSG_ReadString( msg );

	// see if we have already executed it
	if ( cl->lastClientCommand >= seq ) {
		return qtrue;
	}

	Com_DPrintf( "clientCommand: %s : %i : %s\n", cl->name, seq, s );

	// drop the connection if we have somehow lost commands
	if ( seq > cl->lastClientCommand + 1 ) {
		Com_Printf( "Client %s lost %i clientCommands\n", cl->name,
					seq - cl->lastClientCommand + 1 );
		SV_DropClient( cl, "Lost reliable commands" );
		return qfalse;
	}

#if !defined RTCW_SP

#if defined RTCW_ET
	// Gordon: AHA! Need to steal this for some other stuff BOOKMARK
#endif // RTCW_XX

	// NERVE - SMF - some server game-only commands we cannot have flood protect

#if !defined RTCW_ET
	if ( !Q_strncmp( "team", s, 4 ) || !Q_strncmp( "setspawnpt", s, 10 ) || !Q_strncmp( "score", s, 5 ) ) {
#else
	if ( !Q_strncmp( "team", s, 4 ) || !Q_strncmp( "setspawnpt", s, 10 ) || !Q_strncmp( "score", s, 5 ) || !Q_stricmp( "forcetapout", s ) ) {
#endif // RTCW_XX

//		Com_DPrintf( "Skipping flood protection for: %s\n", s );
		floodprotect = qfalse;
	}
#endif // RTCW_XX

	// malicious users may try using too many string commands
	// to lag other players.  If we decide that we want to stall
	// the command, we will stop processing the rest of the packet,
	// including the usercmd.  This causes flooders to lag themselves
	// but not other people
	// We don't do this when the client hasn't been active yet since its
	// normal to spam a lot of commands when downloading
	if ( !com_cl_running->integer &&
		 cl->state >= CS_ACTIVE &&      // (SA) this was commented out in Wolf.  Did we do that?
		 sv_floodProtect->integer &&

#if defined RTCW_SP
		 svs.time < cl->nextReliableTime ) {
#else
		 svs.time < cl->nextReliableTime &&
		 floodprotect ) {
#endif // RTCW_XX

		// ignore any other text messages from this client but let them keep playing

#if !defined RTCW_SP
		// TTimo - moved the ignored verbose to the actual processing in SV_ExecuteClientCommand, only printing if the core doesn't intercept
#endif // RTCW_XX

		clientOk = qfalse;

#if defined RTCW_SP
		Com_DPrintf( "client text ignored for %s\n", cl->name );
		//return qfalse;	// stop processing
#endif // RTCW_XX

	}

#if !defined RTCW_ET
	// don't allow another command for one second
#else
	// don't allow another command for 800 msec
#endif // RTCW_XX

#if defined RTCW_SP
	cl->nextReliableTime = svs.time + 1000;
#elif defined RTCW_MP
	if ( floodprotect ) {
		cl->nextReliableTime = svs.time + 800;
	}
#else
	if ( floodprotect &&
		 svs.time >= cl->nextReliableTime ) {
		cl->nextReliableTime = svs.time + 800;
	}
#endif // RTCW_XX

#if !defined RTCW_ET
	SV_ExecuteClientCommand( cl, s, clientOk );
#else
	SV_ExecuteClientCommand( cl, s, clientOk, premaprestart );
#endif // RTCW_XX

	cl->lastClientCommand = seq;
	Com_sprintf( cl->lastClientCommandString, sizeof( cl->lastClientCommandString ), "%s", s );

	return qtrue;       // continue procesing
}


//==================================================================================


/*
==================
SV_ClientThink

Also called by bot code
==================
*/
void SV_ClientThink( client_t *cl, usercmd_t *cmd ) {
	cl->lastUsercmd = *cmd;

	if ( cl->state != CS_ACTIVE ) {
		return;     // may have been kicked during the last usercmd
	}

	VM_Call(
		gvm,
		GAME_CLIENT_THINK,
		rtcw::to_vm_arg(cl - svs.clients)
	);
}

/*
==================
SV_UserMove

The message usually contains all the movement commands
that were in the last three packets, so that the information
in dropped packets can be recovered.

On very fast clients, there may be multiple usercmd packed into
each of the backup packets.
==================
*/
static void SV_UserMove( client_t *cl, msg_t *msg, qboolean delta ) {
	int i, key;
	int cmdCount;
	usercmd_t nullcmd;
	usercmd_t cmds[MAX_PACKET_USERCMDS];
	usercmd_t   *cmd, *oldcmd;

	if ( delta ) {
		cl->deltaMessage = cl->messageAcknowledge;
	} else {
		cl->deltaMessage = -1;
	}

	cmdCount = MSG_ReadByte( msg );

	if ( cmdCount < 1 ) {
		Com_Printf( "cmdCount < 1\n" );
		return;
	}

	if ( cmdCount > MAX_PACKET_USERCMDS ) {
		Com_Printf( "cmdCount > MAX_PACKET_USERCMDS\n" );
		return;
	}

	// use the checksum feed in the key
	key = sv.checksumFeed;
	// also use the message acknowledge
	key ^= cl->messageAcknowledge;
	// also use the last acknowledged server command in the key

#if defined RTCW_SP
	//key ^= Com_HashKey(cl->reliableCommands[ cl->reliableAcknowledge & (MAX_RELIABLE_COMMANDS-1) ], 32);
	key ^= Com_HashKey( SV_GetReliableCommand( cl, cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ), 32 );
#else
	key ^= Com_HashKey( cl->reliableCommands[ cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ], 32 );
#endif // RTCW_XX

	memset( &nullcmd, 0, sizeof( nullcmd ) );
	oldcmd = &nullcmd;
	for ( i = 0 ; i < cmdCount ; i++ ) {
		cmd = &cmds[i];
		MSG_ReadDeltaUsercmdKey( msg, key, oldcmd, cmd );
//		MSG_ReadDeltaUsercmd( msg, oldcmd, cmd );
		oldcmd = cmd;
	}

	// save time for ping calculation
	cl->frames[ cl->messageAcknowledge & PACKET_MASK ].messageAcked = svs.time;

#if !defined RTCW_SP
	// TTimo
	// catch the no-cp-yet situation before SV_ClientEnterWorld
	// if CS_ACTIVE, then it's time to trigger a new gamestate emission
	// if not, then we are getting remaining parasite usermove commands, which we should ignore
	if ( sv_pure->integer != 0 && cl->pureAuthentic == 0 && !cl->gotCP ) {
		if ( cl->state == CS_ACTIVE ) {
			// we didn't get a cp yet, don't assume anything and just send the gamestate all over again

#if !defined RTCW_ET
			Com_DPrintf( "%s: didn't get cp command, resending gamestate\n", cl->name, cl->state );
#else
			Com_DPrintf( "%s: didn't get cp command, resending gamestate\n", cl->name );
#endif // RTCW_XX

			SV_SendClientGameState( cl );
		}
		return;
	}
#endif // RTCW_XX

	// if this is the first usercmd we have received
	// this gamestate, put the client into the world
	if ( cl->state == CS_PRIMED ) {
		SV_ClientEnterWorld( cl, &cmds[0] );
		// the moves can be processed normaly
	}

#if defined RTCW_SP
	//
#else
	// a bad cp command was sent, drop the client
#endif // RTCW_XX

	if ( sv_pure->integer != 0 && cl->pureAuthentic == 0 ) {
		SV_DropClient( cl, "Cannot validate pure client!" );
		return;
	}

	if ( cl->state != CS_ACTIVE ) {
		cl->deltaMessage = -1;
		return;
	}

	// usually, the first couple commands will be duplicates
	// of ones we have previously received, but the servertimes
	// in the commands will cause them to be immediately discarded
	for ( i =  0 ; i < cmdCount ; i++ ) {
		// if this is a cmd from before a map_restart ignore it
		if ( cmds[i].serverTime > cmds[cmdCount - 1].serverTime ) {
			continue;
		}
		// extremely lagged or cmd from before a map_restart
		//if ( cmds[i].serverTime > svs.time + 3000 ) {
		//	continue;
		//}

#if !defined RTCW_ET
		if ( sv_gametype->integer != GT_SINGLE_PLAYER ) { // RF, we need to allow this in single player, where loadgame's can cause the player to freeze after reloading if we do this check
#else
		if ( !SV_GameIsSinglePlayer() ) { // We need to allow this in single player, where loadgame's can cause the player to freeze after reloading if we do this check
#endif // RTCW_XX

			// don't execute if this is an old cmd which is already executed
			// these old cmds are included when cl_packetdup > 0
			if ( cmds[i].serverTime <= cl->lastUsercmd.serverTime ) {   // Q3_MISSIONPACK
//			if ( cmds[i].serverTime > cmds[cmdCount-1].serverTime ) {
				continue;   // from just before a map_restart
			}
		}
		SV_ClientThink( cl, &cmds[ i ] );
	}
}

#if defined RTCW_ET
/*
=====================
SV_ParseBinaryMessage
=====================
*/
static void SV_ParseBinaryMessage( client_t *cl, msg_t *msg ) {
	int size;

	MSG_BeginReadingUncompressed( msg );

	size = msg->cursize - msg->readcount;
	if ( size <= 0 || size > MAX_BINARY_MESSAGE ) {
		return;
	}

	SV_GameBinaryMessageReceived( cl - svs.clients, reinterpret_cast<const char*> (&msg->data[msg->readcount]), size, cl->lastUsercmd.serverTime );
}
#endif // RTCW_XX

/*
===========================================================================

USER CMD EXECUTION

===========================================================================
*/

/*
===================
SV_ExecuteClientMessage

Parse a client packet
===================
*/
void SV_ExecuteClientMessage( client_t *cl, msg_t *msg ) {
	int c;
	int serverId;

	MSG_Bitstream( msg );

	serverId = MSG_ReadLong( msg );
	cl->messageAcknowledge = MSG_ReadLong( msg );

	if ( cl->messageAcknowledge < 0 ) {
		// usually only hackers create messages like this
		// it is more annoying for them to let them hanging

#if defined RTCW_SP
		//SV_DropClient( cl, "illegible client message" );
#else
#ifndef NDEBUG
		SV_DropClient( cl, "DEBUG: illegible client message" );
#endif
#endif // RTCW_XX

		return;
	}

	cl->reliableAcknowledge = MSG_ReadLong( msg );

	// NOTE: when the client message is fux0red the acknowledgement numbers
	// can be out of range, this could cause the server to send thousands of server
	// commands which the server thinks are not yet acknowledged in SV_UpdateServerCommandsToClient
	if ( cl->reliableAcknowledge < cl->reliableSequence - MAX_RELIABLE_COMMANDS ) {
		// usually only hackers create messages like this
		// it is more annoying for them to let them hanging

#if defined RTCW_SP
		//SV_DropClient( cl, "illegible client message" );
#else
#ifndef NDEBUG
		SV_DropClient( cl, "DEBUG: illegible client message" );
#endif
#endif // RTCW_XX

		cl->reliableAcknowledge = cl->reliableSequence;
		return;
	}
	// if this is a usercmd from a previous gamestate,
	// ignore it or retransmit the current gamestate
	//
	// if the client was downloading, let it stay at whatever serverId and
	// gamestate it was at.  This allows it to keep downloading even when
	// the gamestate changes.  After the download is finished, we'll
	// notice and send it a new game state

#if defined RTCW_SP
	if ( serverId != sv.serverId &&
		 !*cl->downloadName ) {
		if ( serverId == sv.restartedServerId ) {
			// they just haven't caught the map_restart yet
#else
	//
	// show_bug.cgi?id=536
	// don't drop as long as previous command was a nextdl, after a dl is done, downloadName is set back to ""
	// but we still need to read the next message to move to next download or send gamestate
	// I don't like this hack though, it must have been working fine at some point, suspecting the fix is somewhere else
	if ( serverId != sv.serverId && !*cl->downloadName && !strstr( cl->lastClientCommandString, "nextdl" ) ) {
		if ( serverId >= sv.restartedServerId && serverId < sv.serverId ) { // TTimo - use a comparison here to catch multiple map_restart
			// they just haven't caught the map_restart yet
			Com_DPrintf( "%s : ignoring pre map_restart / outdated client message\n", cl->name );
#endif // RTCW_XX

			return;
		}
		// if we can tell that the client has dropped the last
		// gamestate we sent them, resend it
		if ( cl->messageAcknowledge > cl->gamestateMessageNum ) {
			Com_DPrintf( "%s : dropped gamestate, resending\n", cl->name );
			SV_SendClientGameState( cl );
		}

#if defined RTCW_ET
		// read optional clientCommand strings
		do {
			c = MSG_ReadByte( msg );
			if ( c == clc_EOF ) {
				break;
			}
			if ( c != clc_clientCommand ) {
				break;
			}
			if ( !SV_ClientCommand( cl, msg, qtrue ) ) {
				return; // we couldn't execute it because of the flood protection
			}
			if ( cl->state == CS_ZOMBIE ) {
				return; // disconnect command
			}
		} while ( 1 );
#endif // RTCW_XX

		return;
	}

#if defined RTCW_SP
	// RF, kill any reliableCommands that have been acknowledged
	SV_FreeAcknowledgedReliableCommands( cl );
#endif // RTCW_XX

	// read optional clientCommand strings
	do {
		c = MSG_ReadByte( msg );
		if ( c == clc_EOF ) {
			break;
		}
		if ( c != clc_clientCommand ) {
			break;
		}

#if !defined RTCW_ET
		if ( !SV_ClientCommand( cl, msg ) ) {
#else
		if ( !SV_ClientCommand( cl, msg, qfalse ) ) {
#endif // RTCW_XX

			return; // we couldn't execute it because of the flood protection
		}
		if ( cl->state == CS_ZOMBIE ) {
			return; // disconnect command
		}
	} while ( 1 );

	// read the usercmd_t
	if ( c == clc_move ) {
		SV_UserMove( cl, msg, qtrue );

#if defined RTCW_ET
		c = MSG_ReadByte( msg );
#endif // RTCW_XX

	} else if ( c == clc_moveNoDelta ) {
		SV_UserMove( cl, msg, qfalse );

#if !defined RTCW_ET
	} else if ( c != clc_EOF ) {
#else
		c = MSG_ReadByte( msg );
	}

	if ( c != clc_EOF ) {
#endif // RTCW_XX

		Com_Printf( "WARNING: bad command byte for client %i\n", cl - svs.clients );
	}

#if defined RTCW_ET
	SV_ParseBinaryMessage( cl, msg );
#endif // RTCW_XX

//	if ( msg->readcount != msg->cursize ) {
//		Com_Printf( "WARNING: Junk at end of packet for client %i\n", cl - svs.clients );
//	}
}

