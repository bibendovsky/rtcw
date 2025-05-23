/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "client.h"

#include "botlib.h"

#include "rtcw_vm_args.h"


extern botlib_export_t *botlib_export;

vm_t *uivm;

#if defined RTCW_SP
extern char cl_cdkey[34];
#endif // RTCW_XX

#if defined RTCW_ET
// ydnar: can we put this in a header, pls?
void Key_GetBindingByString( const char* binding, int* key1, int* key2 );
#endif // RTCW_XX


/*
====================
GetClientState
====================
*/
static void GetClientState( uiClientState_t *state ) {
	state->connectPacketCount = clc.connectPacketCount;
	state->connState = cls.state;
	Q_strncpyz( state->servername, cls.servername, sizeof( state->servername ) );
	Q_strncpyz( state->updateInfoString, cls.updateInfoString, sizeof( state->updateInfoString ) );
	Q_strncpyz( state->messageString, clc.serverMessage, sizeof( state->messageString ) );
	state->clientNum = cl.snap.ps.clientNum;
}

/*
====================
LAN_LoadCachedServers
====================
*/
void LAN_LoadCachedServers() {

#if defined RTCW_SP
	// TTimo: stub, this is only relevant to MP, SP kills the servercache.dat (and favorites)
	// show_bug.cgi?id=445
	/*
	  int size;
	  fileHandle_t fileIn;
	  cls.numglobalservers = cls.nummplayerservers = cls.numfavoriteservers = 0;
	  cls.numGlobalServerAddresses = 0;
	  if (FS_SV_FOpenFileRead("servercache.dat", &fileIn)) {
		  FS_Read(&cls.numglobalservers, sizeof(int), fileIn);
		  FS_Read(&cls.nummplayerservers, sizeof(int), fileIn);
		  FS_Read(&cls.numfavoriteservers, sizeof(int), fileIn);
		  FS_Read(&size, sizeof(int), fileIn);
		  if (size == sizeof(cls.globalServers) + sizeof(cls.favoriteServers) + sizeof(cls.mplayerServers)) {
			  FS_Read(&cls.globalServers, sizeof(cls.globalServers), fileIn);
			  FS_Read(&cls.mplayerServers, sizeof(cls.mplayerServers), fileIn);
			  FS_Read(&cls.favoriteServers, sizeof(cls.favoriteServers), fileIn);
		  } else {
			  cls.numglobalservers = cls.nummplayerservers = cls.numfavoriteservers = 0;
			  cls.numGlobalServerAddresses = 0;
		  }
		  FS_FCloseFile(fileIn);
	  }
	*/
#elif defined RTCW_MP
	int size;
	fileHandle_t fileIn;
	cls.numglobalservers = cls.nummplayerservers = cls.numfavoriteservers = 0;
	cls.numGlobalServerAddresses = 0;
	if ( FS_SV_FOpenFileRead( "servercache.dat", &fileIn ) ) {
		FS_Read( &cls.numglobalservers, sizeof( int ), fileIn );
		FS_Read( &cls.nummplayerservers, sizeof( int ), fileIn );
		FS_Read( &cls.numfavoriteservers, sizeof( int ), fileIn );
		FS_Read( &size, sizeof( int ), fileIn );
		if ( size == sizeof( cls.globalServers ) + sizeof( cls.favoriteServers ) + sizeof( cls.mplayerServers ) ) {
			FS_Read( &cls.globalServers, sizeof( cls.globalServers ), fileIn );
			FS_Read( &cls.mplayerServers, sizeof( cls.mplayerServers ), fileIn );
			FS_Read( &cls.favoriteServers, sizeof( cls.favoriteServers ), fileIn );
		} else {
			cls.numglobalservers = cls.nummplayerservers = cls.numfavoriteservers = 0;
			cls.numGlobalServerAddresses = 0;
		}
		FS_FCloseFile( fileIn );
	}
#else
	int size;
	fileHandle_t fileIn;
	char filename[MAX_QPATH];

	cls.numglobalservers = cls.numfavoriteservers = 0;
	cls.numGlobalServerAddresses = 0;

	if ( com_gameInfo.usesProfiles && cl_profile->string[0] ) {
		Com_sprintf( filename, sizeof( filename ), "profiles/%s/servercache.dat", cl_profile->string );
	} else {
		Q_strncpyz( filename, "servercache.dat", sizeof( filename ) );
	}

	// Arnout: moved to mod/profiles dir
	//if (FS_SV_FOpenFileRead(filename, &fileIn)) {
	if ( FS_FOpenFileRead( filename, &fileIn, qtrue ) ) {
		FS_Read( &cls.numglobalservers, sizeof( int ), fileIn );
		FS_Read( &cls.numfavoriteservers, sizeof( int ), fileIn );
		FS_Read( &size, sizeof( int ), fileIn );
		if ( size == sizeof( cls.globalServers ) + sizeof( cls.favoriteServers ) ) {
			FS_Read( &cls.globalServers, sizeof( cls.globalServers ), fileIn );
			FS_Read( &cls.favoriteServers, sizeof( cls.favoriteServers ), fileIn );
		} else {
			cls.numglobalservers = cls.numfavoriteservers = 0;
			cls.numGlobalServerAddresses = 0;
		}
		FS_FCloseFile( fileIn );
	}
#endif // RTCW_XX

}

/*
====================
LAN_SaveServersToCache
====================
*/
void LAN_SaveServersToCache() {

#if defined RTCW_SP
	// TTimo: stub, this is only relevant to MP, SP kills the servercache.dat (and favorites)
	// show_bug.cgi?id=445
#elif defined RTCW_MP
	int size;
	fileHandle_t fileOut;

	fileOut = FS_SV_FOpenFileWrite( "servercache.dat" );
	FS_Write( &cls.numglobalservers, sizeof( int ), fileOut );
	FS_Write( &cls.nummplayerservers, sizeof( int ), fileOut );
	FS_Write( &cls.numfavoriteservers, sizeof( int ), fileOut );
	size = sizeof( cls.globalServers ) + sizeof( cls.favoriteServers ) + sizeof( cls.mplayerServers );
	FS_Write( &size, sizeof( int ), fileOut );
	FS_Write( &cls.globalServers, sizeof( cls.globalServers ), fileOut );
	FS_Write( &cls.mplayerServers, sizeof( cls.mplayerServers ), fileOut );
	FS_Write( &cls.favoriteServers, sizeof( cls.favoriteServers ), fileOut );
	FS_FCloseFile( fileOut );
#else
	int size;
	fileHandle_t fileOut;
	char filename[MAX_QPATH];

	if ( com_gameInfo.usesProfiles && cl_profile->string[0] ) {
		Com_sprintf( filename, sizeof( filename ), "profiles/%s/servercache.dat", cl_profile->string );
	} else {
		Q_strncpyz( filename, "servercache.dat", sizeof( filename ) );
	}

	// Arnout: moved to mod/profiles dir
	//fileOut = FS_SV_FOpenFileWrite(filename);
	fileOut = FS_FOpenFileWrite( filename );
	FS_Write( &cls.numglobalservers, sizeof( int ), fileOut );
	FS_Write( &cls.numfavoriteservers, sizeof( int ), fileOut );
	size = sizeof( cls.globalServers ) + sizeof( cls.favoriteServers );
	FS_Write( &size, sizeof( int ), fileOut );
	FS_Write( &cls.globalServers, sizeof( cls.globalServers ), fileOut );
	FS_Write( &cls.favoriteServers, sizeof( cls.favoriteServers ), fileOut );
	FS_FCloseFile( fileOut );
#endif // RTCW_XX
}


/*
====================
LAN_ResetPings
====================
*/
static void LAN_ResetPings( int source ) {
	int count,i;
	serverInfo_t *servers = NULL;
	count = 0;

	switch ( source ) {
	case AS_LOCAL:
		servers = &cls.localServers[0];
		count = MAX_OTHER_SERVERS;
		break;

#if !defined RTCW_ET
	case AS_MPLAYER:
		servers = &cls.mplayerServers[0];
		count = MAX_OTHER_SERVERS;
		break;
#endif // RTCW_XX

	case AS_GLOBAL:
		servers = &cls.globalServers[0];
		count = MAX_GLOBAL_SERVERS;
		break;
	case AS_FAVORITES:
		servers = &cls.favoriteServers[0];
		count = MAX_OTHER_SERVERS;
		break;
	}
	if ( servers ) {
		for ( i = 0; i < count; i++ ) {
			servers[i].ping = -1;
		}
	}
}

/*
====================
LAN_AddServer
====================
*/
static int LAN_AddServer( int source, const char *name, const char *address ) {
	int max, *count, i;
	netadr_t adr;
	serverInfo_t *servers = NULL;
	max = MAX_OTHER_SERVERS;
	count = 0;

	switch ( source ) {
	case AS_LOCAL:
		count = &cls.numlocalservers;
		servers = &cls.localServers[0];
		break;

#if !defined RTCW_ET
	case AS_MPLAYER:
		count = &cls.nummplayerservers;
		servers = &cls.mplayerServers[0];
		break;
#endif // RTCW_XX

	case AS_GLOBAL:
		max = MAX_GLOBAL_SERVERS;
		count = &cls.numglobalservers;
		servers = &cls.globalServers[0];
		break;
	case AS_FAVORITES:
		count = &cls.numfavoriteservers;
		servers = &cls.favoriteServers[0];
		break;
	}
	if ( servers && *count < max ) {
		NET_StringToAdr( address, &adr );
		for ( i = 0; i < *count; i++ ) {
			if ( NET_CompareAdr( servers[i].adr, adr ) ) {
				break;
			}
		}
		if ( i >= *count ) {
			servers[*count].adr = adr;
			Q_strncpyz( servers[*count].hostName, name, sizeof( servers[*count].hostName ) );
			servers[*count].visible = qtrue;
			( *count )++;
			return 1;
		}
		return 0;
	}
	return -1;
}

/*
====================
LAN_RemoveServer
====================
*/
static void LAN_RemoveServer( int source, const char *addr ) {
	int *count, i;
	serverInfo_t *servers = NULL;
	count = 0;
	switch ( source ) {
	case AS_LOCAL:
		count = &cls.numlocalservers;
		servers = &cls.localServers[0];
		break;

#if !defined RTCW_ET
	case AS_MPLAYER:
		count = &cls.nummplayerservers;
		servers = &cls.mplayerServers[0];
		break;
#endif // RTCW_XX

	case AS_GLOBAL:
		count = &cls.numglobalservers;
		servers = &cls.globalServers[0];
		break;
	case AS_FAVORITES:
		count = &cls.numfavoriteservers;
		servers = &cls.favoriteServers[0];
		break;
	}
	if ( servers ) {
		netadr_t comp;
		NET_StringToAdr( addr, &comp );
		for ( i = 0; i < *count; i++ ) {
			if ( NET_CompareAdr( comp, servers[i].adr ) ) {
				int j = i;
				while ( j < *count - 1 ) {
					Com_Memcpy( &servers[j], &servers[j + 1], sizeof( servers[j] ) );
					j++;
				}
				( *count )--;
				break;
			}
		}
	}
}


/*
====================
LAN_GetServerCount
====================
*/
static int LAN_GetServerCount( int source ) {
	switch ( source ) {
	case AS_LOCAL:
		return cls.numlocalservers;
		break;

#if !defined RTCW_ET
	case AS_MPLAYER:
		return cls.nummplayerservers;
		break;
#endif // RTCW_XX

	case AS_GLOBAL:
		return cls.numglobalservers;
		break;
	case AS_FAVORITES:
		return cls.numfavoriteservers;
		break;
	}
	return 0;
}

/*
====================
LAN_GetLocalServerAddressString
====================
*/
static void LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			Q_strncpyz( buf, NET_AdrToString( cls.localServers[n].adr ), buflen );
			return;
		}
		break;

#if !defined RTCW_ET
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			Q_strncpyz( buf, NET_AdrToString( cls.mplayerServers[n].adr ), buflen );
			return;
		}
		break;
#endif // RTCW_XX

	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			Q_strncpyz( buf, NET_AdrToString( cls.globalServers[n].adr ), buflen );
			return;
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			Q_strncpyz( buf, NET_AdrToString( cls.favoriteServers[n].adr ), buflen );
			return;
		}
		break;
	}
	buf[0] = '\0';
}

/*
====================
LAN_GetServerInfo
====================
*/
static void LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	char info[MAX_STRING_CHARS];
	serverInfo_t *server = NULL;
	info[0] = '\0';
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.localServers[n];
		}
		break;

#if !defined RTCW_ET
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.mplayerServers[n];
		}
		break;
#endif // RTCW_XX

	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			server = &cls.globalServers[n];
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.favoriteServers[n];
		}
		break;
	}
	if ( server && buf ) {
		buf[0] = '\0';
		Info_SetValueForKey( info, "hostname", server->hostName );

#if defined RTCW_ET
		Info_SetValueForKey( info, "serverload", va( "%i", server->load ) );
#endif // RTCW_XX

		Info_SetValueForKey( info, "mapname", server->mapName );
		Info_SetValueForKey( info, "clients", va( "%i",server->clients ) );
		Info_SetValueForKey( info, "sv_maxclients", va( "%i",server->maxClients ) );
		Info_SetValueForKey( info, "ping", va( "%i",server->ping ) );
		Info_SetValueForKey( info, "minping", va( "%i",server->minPing ) );
		Info_SetValueForKey( info, "maxping", va( "%i",server->maxPing ) );
		Info_SetValueForKey( info, "game", server->game );
		Info_SetValueForKey( info, "gametype", va( "%i",server->gameType ) );
		Info_SetValueForKey( info, "nettype", va( "%i",server->netType ) );
		Info_SetValueForKey( info, "addr", NET_AdrToString( server->adr ) );
		Info_SetValueForKey( info, "sv_allowAnonymous", va( "%i", server->allowAnonymous ) );

#if !defined RTCW_SP
		Info_SetValueForKey( info, "friendlyFire", va( "%i", server->friendlyFire ) );               // NERVE - SMF
		Info_SetValueForKey( info, "maxlives", va( "%i", server->maxlives ) );                       // NERVE - SMF

#if !defined RTCW_ET
		Info_SetValueForKey( info, "tourney", va( "%i", server->tourney ) );                     // NERVE - SMF
#else
		Info_SetValueForKey( info, "needpass", va( "%i", server->needpass ) );                       // NERVE - SMF
#endif // RTCW_XX

		Info_SetValueForKey( info, "punkbuster", va( "%i", server->punkbuster ) );                   // DHM - Nerve
		Info_SetValueForKey( info, "gamename", server->gameName );                                // Arnout
		Info_SetValueForKey( info, "g_antilag", va( "%i", server->antilag ) ); // TTimo
#endif // RTCW_XX

#if defined RTCW_ET
		Info_SetValueForKey( info, "weaprestrict", va( "%i", server->weaprestrict ) );
		Info_SetValueForKey( info, "balancedteams", va( "%i", server->balancedteams ) );
#endif // RTCW_XX

		Q_strncpyz( buf, info, buflen );
	} else {
		if ( buf ) {
			buf[0] = '\0';
		}
	}
}

/*
====================
LAN_GetServerPing
====================
*/
static int LAN_GetServerPing( int source, int n ) {
	serverInfo_t *server = NULL;
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.localServers[n];
		}
		break;

#if !defined RTCW_ET
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.mplayerServers[n];
		}
		break;
#endif // RTCW_XX

	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			server = &cls.globalServers[n];
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.favoriteServers[n];
		}
		break;
	}
	if ( server ) {
		return server->ping;
	}
	return -1;
}

/*
====================
LAN_GetServerPtr
====================
*/
static serverInfo_t *LAN_GetServerPtr( int source, int n ) {
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return &cls.localServers[n];
		}
		break;

#if !defined RTCW_ET
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return &cls.mplayerServers[n];
		}
		break;
#endif // RTCW_XX

	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			return &cls.globalServers[n];
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return &cls.favoriteServers[n];
		}
		break;
	}
	return NULL;
}

/*
====================
LAN_CompareServers
====================
*/
static int LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	int res;
	serverInfo_t *server1, *server2;

#if defined RTCW_ET
	char name1[ MAX_NAME_LENGTH ], name2[ MAX_NAME_LENGTH ];
#endif // RTCW_XX

	server1 = LAN_GetServerPtr( source, s1 );
	server2 = LAN_GetServerPtr( source, s2 );
	if ( !server1 || !server2 ) {
		return 0;
	}

	res = 0;
	switch ( sortKey ) {
	case SORT_HOST:

#if !defined RTCW_ET
		res = Q_stricmp( server1->hostName, server2->hostName );
#else
		//%	res = Q_stricmp( server1->hostName, server2->hostName );
		Q_strncpyz( name1, server1->hostName, sizeof( name1 ) );
		Q_CleanStr( name1 );
		Q_strncpyz( name2, server2->hostName, sizeof( name2 ) );
		Q_CleanStr( name2 );
		res = Q_stricmp( name1, name2 );
#endif // RTCW_XX

		break;

	case SORT_MAP:
		res = Q_stricmp( server1->mapName, server2->mapName );
		break;
	case SORT_CLIENTS:
		if ( server1->clients < server2->clients ) {
			res = -1;
		} else if ( server1->clients > server2->clients )     {
			res = 1;
		} else {
			res = 0;
		}
		break;
	case SORT_GAME:
		if ( server1->gameType < server2->gameType ) {
			res = -1;
		} else if ( server1->gameType > server2->gameType )     {
			res = 1;
		} else {
			res = 0;
		}
		break;
	case SORT_PING:
		if ( server1->ping < server2->ping ) {
			res = -1;
		} else if ( server1->ping > server2->ping )     {
			res = 1;
		} else {
			res = 0;
		}
		break;

#if defined RTCW_MP
	case SORT_PUNKBUSTER:
		if ( server1->punkbuster < server2->punkbuster ) {
			res = -1;
		} else if ( server1->punkbuster > server2->punkbuster )     {
			res = 1;
		} else {
			res = 0;
		}
#endif // RTCW_XX

	}

	if ( sortDir ) {
		if ( res < 0 ) {
			return 1;
		}
		if ( res > 0 ) {
			return -1;
		}
		return 0;
	}
	return res;
}

/*
====================
LAN_GetPingQueueCount
====================
*/
static int LAN_GetPingQueueCount( void ) {
	return ( CL_GetPingQueueCount() );
}

/*
====================
LAN_ClearPing
====================
*/
static void LAN_ClearPing( int n ) {
	CL_ClearPing( n );
}

/*
====================
LAN_GetPing
====================
*/
static void LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	CL_GetPing( n, buf, buflen, pingtime );
}

/*
====================
LAN_GetPingInfo
====================
*/
static void LAN_GetPingInfo( int n, char *buf, int buflen ) {
	CL_GetPingInfo( n, buf, buflen );
}

/*
====================
LAN_MarkServerVisible
====================
*/
static void LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	if ( n == -1 ) {
		int count = MAX_OTHER_SERVERS;
		serverInfo_t *server = NULL;
		switch ( source ) {
		case AS_LOCAL:
			server = &cls.localServers[0];
			break;

#if !defined RTCW_ET
		case AS_MPLAYER:
			server = &cls.mplayerServers[0];
			break;
#endif // RTCW_XX

		case AS_GLOBAL:
			server = &cls.globalServers[0];
			count = MAX_GLOBAL_SERVERS;
			break;
		case AS_FAVORITES:
			server = &cls.favoriteServers[0];
			break;
		}
		if ( server ) {
			for ( n = 0; n < count; n++ ) {
				server[n].visible = visible;
			}
		}

	} else {
		switch ( source ) {
		case AS_LOCAL:
			if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
				cls.localServers[n].visible = visible;
			}
			break;

#if !defined RTCW_ET
		case AS_MPLAYER:
			if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
				cls.mplayerServers[n].visible = visible;
			}
			break;
#endif // RTCW_XX

		case AS_GLOBAL:
			if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
				cls.globalServers[n].visible = visible;
			}
			break;
		case AS_FAVORITES:
			if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
				cls.favoriteServers[n].visible = visible;
			}
			break;
		}
	}
}


/*
=======================
LAN_ServerIsVisible
=======================
*/
static int LAN_ServerIsVisible( int source, int n ) {
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return cls.localServers[n].visible;
		}
		break;

#if !defined RTCW_ET
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return cls.mplayerServers[n].visible;
		}
		break;
#endif // RTCW_XX

	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			return cls.globalServers[n].visible;
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return cls.favoriteServers[n].visible;
		}
		break;
	}
	return qfalse;
}

/*
=======================
LAN_UpdateVisiblePings
=======================
*/
qboolean LAN_UpdateVisiblePings( int source ) {
	return CL_UpdateVisiblePings_f( source );
}

/*
====================
LAN_GetServerStatus
====================
*/
int LAN_GetServerStatus( char *serverAddress, char *serverStatus, int maxLen ) {
	return CL_ServerStatus( serverAddress, serverStatus, maxLen );
}

#if defined RTCW_ET
/*
=======================
LAN_ServerIsInFavoriteList
=======================
*/
qboolean LAN_ServerIsInFavoriteList( int source, int n ) {
	int i;
	serverInfo_t *server = NULL;

	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.localServers[n];
		}
		break;
	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			server = &cls.globalServers[n];
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return qtrue;
		}
		break;
	}

	if ( !server ) {
		return qfalse;
	}

	for ( i = 0; i < cls.numfavoriteservers; i++ ) {
		if ( NET_CompareAdr( cls.favoriteServers[i].adr, server->adr ) ) {
			return qtrue;
		}
	}

	return qfalse;
}
#endif // RTCW_XX

/*
====================
CL_GetGlConfig
====================
*/
static void CL_GetGlconfig( glconfig_t *config ) {
	*config = cls.glconfig;
}

/*
====================
GetClipboardData
====================
*/
static void GetClipboardData( char *buf, int buflen ) {
	char    *cbd;

	cbd = Sys_GetClipboardData();

	if ( !cbd ) {
		*buf = 0;
		return;
	}

	Q_strncpyz( buf, cbd, buflen );

	Sys_FreeClipboardData(cbd);
}

/*
====================
Key_KeynumToStringBuf
====================
*/

#if defined RTCW_SP
static void Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
#else
void Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
#endif // RTCW_XX

	Q_strncpyz( buf, Key_KeynumToString( keynum, qtrue ), buflen );
}

/*
====================
Key_GetBindingBuf
====================
*/

#if defined RTCW_SP
static void Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
#else
void Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
#endif // RTCW_XX

	const char    *value;

	value = Key_GetBinding( keynum );
	if ( value ) {
		Q_strncpyz( buf, value, buflen );
	} else {
		*buf = 0;
	}
}

/*
====================
Key_GetCatcher
====================
*/
int Key_GetCatcher( void ) {
	return cls.keyCatchers;
}

/*
====================
Ket_SetCatcher
====================
*/
void Key_SetCatcher( int catcher ) {

#if defined RTCW_SP
	cls.keyCatchers = catcher;
#else
	// NERVE - SMF - console overrides everything
	if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {
		cls.keyCatchers = catcher | KEYCATCH_CONSOLE;
	} else {
		cls.keyCatchers = catcher;
	}
#endif // RTCW_XX

}


/*
====================
CLUI_GetCDKey
====================
*/
static void CLUI_GetCDKey( char *buf, int buflen ) {
	cvar_t  *fs;
	fs = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
	if ( UI_usesUniqueCDKey() && fs && fs->string[0] != 0 ) {
		memcpy( buf, &cl_cdkey[16], 16 );
		buf[16] = 0;
	} else {
		memcpy( buf, cl_cdkey, 16 );
		buf[16] = 0;
	}
}


/*
====================
CLUI_SetCDKey
====================
*/
static void CLUI_SetCDKey( char *buf ) {
	cvar_t  *fs;
	fs = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
	if ( UI_usesUniqueCDKey() && fs && fs->string[0] != 0 ) {
		memcpy( &cl_cdkey[16], buf, 16 );
		cl_cdkey[32] = 0;
		// set the flag so the fle will be written at the next opportunity
		cvar_modifiedFlags |= CVAR_ARCHIVE;
	} else {
		memcpy( cl_cdkey, buf, 16 );
		// set the flag so the fle will be written at the next opportunity
		cvar_modifiedFlags |= CVAR_ARCHIVE;
	}
}


/*
====================
GetConfigString
====================
*/
static int GetConfigString( int index, char *buf, int size ) {
	int offset;

	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		return qfalse;
	}

	offset = cl.gameState.stringOffsets[index];
	if ( !offset ) {
		if ( size ) {
			buf[0] = 0;
		}
		return qfalse;
	}

	Q_strncpyz( buf, cl.gameState.stringData + offset, size );

	return qtrue;
}

/*
====================
FloatAsInt
====================
*/

// BBi
//static int FloatAsInt( float f ) {
//	int temp;
//
//	*(float *)&temp = f;
//
//	return temp;
//}
static int FloatAsInt(
	float x)
{
	return reinterpret_cast<const int&>(x);
}
// BBi

#if FIXME
void *VM_ArgPtr( int intValue );
#else
intptr_t VM_ArgPtr(
	int intValue);
#endif // FIXME

#define VMA( x ) VM_ArgPtr( args[x] )

#if FIXME
// BBi
//#define VMF( x )  ( (float *)args )[x]
#define VMF(x) (*reinterpret_cast<const float*> (args + x))
// BBi
#else
#define VMF(x) (rtcw::from_vm_arg<float>(args[x]))
#endif // FIXME

/*
====================
CL_UISystemCalls

The ui module is making a system call
====================
*/

// BBi
//int CL_UISystemCalls( int *args ) {
int32_t CL_UISystemCalls (
	intptr_t* args)
{
// BBi
#if FIXME
	switch ( args[0] ) {
#else
	switch (rtcw::from_vm_arg<uiImport_t>(args[0]))
	{
#endif // FIXME
	case UI_ERROR:
		Com_Error(ERR_DROP, "%s", rtcw::from_vm_arg<const char*>(VMA(1)));
		return 0;

	case UI_PRINT:
		Com_Printf("%s", rtcw::from_vm_arg<const char*>(VMA(1)));
		return 0;

	case UI_MILLISECONDS:
		return Sys_Milliseconds();

	case UI_CVAR_REGISTER:
		Cvar_Register(
			rtcw::from_vm_arg<vmCvar_t*>(VMA(1)),
			rtcw::from_vm_arg<const char*>(VMA(2)),
			rtcw::from_vm_arg<const char*>(VMA(3)),
			rtcw::from_vm_arg<int>(args[4])
		);
		return 0;

	case UI_CVAR_UPDATE:
		Cvar_Update(rtcw::from_vm_arg<vmCvar_t*>(VMA(1)));
		return 0;

	case UI_CVAR_SET:
		Cvar_Set(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<const char*>(VMA(2))
		);
		return 0;

	case UI_CVAR_VARIABLEVALUE:
		return FloatAsInt(Cvar_VariableValue(rtcw::from_vm_arg<const char*>(VMA(1))));

	case UI_CVAR_VARIABLESTRINGBUFFER:
		Cvar_VariableStringBuffer(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;

#if defined RTCW_ET
	case UI_CVAR_LATCHEDVARIABLESTRINGBUFFER:
		Cvar_LatchedVariableStringBuffer(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;
#endif // RTCW_XX

	case UI_CVAR_SETVALUE:
		Cvar_SetValue(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			VMF(2)
		);
		return 0;

	case UI_CVAR_RESET:
		Cvar_Reset(rtcw::from_vm_arg<const char*>(VMA(1)));
		return 0;

	case UI_CVAR_CREATE:
		Cvar_Get(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<const char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;

	case UI_CVAR_INFOSTRINGBUFFER:
		Cvar_InfoStringBuffer(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;

	case UI_ARGC:
		return Cmd_Argc();

	case UI_ARGV:
		Cmd_ArgvBuffer(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;

	case UI_CMD_EXECUTETEXT:
		Cbuf_ExecuteText(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<const char*>(VMA(2))
		);
		return 0;

#if defined RTCW_ET
	case UI_ADDCOMMAND:
		Cmd_AddCommand(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			NULL
		);
		return 0;
#endif // RTCW_XX

	case UI_FS_FOPENFILE:
		return FS_FOpenFileByMode(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<fileHandle_t*>(VMA(2)),
			rtcw::from_vm_arg<fsMode_t>(args[3])
		);

	case UI_FS_READ:
		FS_Read(
			rtcw::from_vm_arg<void*>(VMA(1)),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<fileHandle_t>(args[3])
		);
		return 0;

#if defined RTCW_SP
//----(SA)	added
	case UI_FS_SEEK:
		FS_Seek(
			rtcw::from_vm_arg<fileHandle_t>(args[1]),
			rtcw::from_vm_arg<int32_t>(args[2]),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;
//----(SA)	end
#endif // RTCW_XX

	case UI_FS_WRITE:
		FS_Write(
			rtcw::from_vm_arg<const void*>(VMA(1)),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<fileHandle_t>(args[3])
		);
		return 0;

	case UI_FS_FCLOSEFILE:
		FS_FCloseFile(rtcw::from_vm_arg<fileHandle_t>(args[1]));
		return 0;

	case UI_FS_DELETEFILE:
		return FS_Delete(rtcw::from_vm_arg<char*>(VMA(1)));

	case UI_FS_GETFILELIST:
		return FS_GetFileList(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<const char*>(VMA(2)),
			rtcw::from_vm_arg<char*>(VMA(3)),
			rtcw::from_vm_arg<int>(args[4])
		);

	case UI_R_REGISTERMODEL:
		return re.RegisterModel(rtcw::from_vm_arg<const char*>(VMA(1)));

	case UI_R_REGISTERSKIN:
		return re.RegisterSkin(rtcw::from_vm_arg<const char*>(VMA(1)));

	case UI_R_REGISTERSHADERNOMIP:
		return re.RegisterShaderNoMip(rtcw::from_vm_arg<const char*>(VMA(1)));

	case UI_R_CLEARSCENE:
		re.ClearScene();
		return 0;

	case UI_R_ADDREFENTITYTOSCENE:
		re.AddRefEntityToScene(rtcw::from_vm_arg<const refEntity_t*>(VMA(1)));
		return 0;

	case UI_R_ADDPOLYTOSCENE:
		re.AddPolyToScene(
			rtcw::from_vm_arg<qhandle_t>(args[1]),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<const polyVert_t*>(VMA(3))
		);
		return 0;

		// Ridah
	case UI_R_ADDPOLYSTOSCENE:
		re.AddPolysToScene(
			rtcw::from_vm_arg<qhandle_t>(args[1]),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<const polyVert_t*>(VMA(3)),
			rtcw::from_vm_arg<int>(args[4])
		);
		return 0;
		// done.

	case UI_R_ADDLIGHTTOSCENE:
		// ydnar: new dlight code
		//%	re.AddLightToScene( VMA(1), VMF(2), VMF(3), VMF(4), VMF(5), args[6] );
		re.AddLightToScene(
			rtcw::from_vm_arg<const vec_t*>(VMA(1)),
			VMF(2),
			VMF(3),
			VMF(4),
			VMF(5),
#if defined RTCW_ET
			VMF(6)
#else
			rtcw::from_vm_arg<int>(args[8])
#endif // RTCW_XX
#if defined RTCW_ET
			,
			rtcw::from_vm_arg<qhandle_t>(args[7]),
			rtcw::from_vm_arg<int>(args[8])
#endif // RTCW_XX
		);
		return 0;

	case UI_R_ADDCORONATOSCENE:
		re.AddCoronaToScene(
			rtcw::from_vm_arg<const vec_t*>(VMA(1)),
			VMF(2),
			VMF(3),
			VMF(4),
			VMF(5),
			rtcw::from_vm_arg<int>(args[6]),
			rtcw::from_vm_arg<int>(args[7])
		);
		return 0;

	case UI_R_RENDERSCENE:
		re.RenderScene(rtcw::from_vm_arg<const refdef_t*>(VMA(1)));
		return 0;

	case UI_R_SETCOLOR:
		re.SetColor(rtcw::from_vm_arg<const float*>(VMA(1)));
		return 0;

#if defined RTCW_ET
	case UI_R_DRAW2DPOLYS:
		re.Add2dPolys(
			rtcw::from_vm_arg<polyVert_t*>(VMA(1)),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<qhandle_t>(args[3])
		);
		return 0;
#endif // RTCW_XX

	case UI_R_DRAWSTRETCHPIC:
		re.DrawStretchPic(
			VMF(1),
			VMF(2),
			VMF(3),
			VMF(4),
			VMF(5),
			VMF(6),
			VMF(7),
			VMF(8),
			rtcw::from_vm_arg<qhandle_t>(args[9])
		);
		return 0;

#if defined RTCW_ET
	case UI_R_DRAWROTATEDPIC:
		re.DrawRotatedPic(
			VMF(1),
			VMF(2),
			VMF(3),
			VMF(4),
			VMF(5),
			VMF(6),
			VMF(7),
			VMF(8),
			rtcw::from_vm_arg<qhandle_t>(args[9]),
			VMF(10)
		);
		return 0;
#endif // RTCW_XX

	case UI_R_MODELBOUNDS:
		re.ModelBounds(
			rtcw::from_vm_arg<qhandle_t>(args[1]),
			rtcw::from_vm_arg<vec_t*>(VMA(2)),
			rtcw::from_vm_arg<vec_t*>(VMA(3))
		);
		return 0;

	case UI_UPDATESCREEN:
		SCR_UpdateScreen();
		return 0;

	case UI_CM_LERPTAG:
		return re.LerpTag(
			rtcw::from_vm_arg<orientation_t*>(VMA(1)),
			rtcw::from_vm_arg<const refEntity_t*>(VMA(2)),
			rtcw::from_vm_arg<const char*>(VMA(3)),
			rtcw::from_vm_arg<int>(args[4])
		);

	case UI_S_REGISTERSOUND:
#if !defined RTCW_ET
		return S_RegisterSound(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			qfalse
		);
#else
		return S_RegisterSound(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<qboolean>(args[2])
		);
#endif // RTCW_XX

	case UI_S_STARTLOCALSOUND:
		S_StartLocalSound(
			rtcw::from_vm_arg<sfxHandle_t>(args[1]),
			rtcw::from_vm_arg<int>(args[2])
#if defined RTCW_ET
			,
			rtcw::from_vm_arg<int>(args[3])
#endif // RTCW_XX
		);

		return 0;

#if !defined RTCW_MP
//----(SA)	added
	case UI_S_FADESTREAMINGSOUND:
		S_FadeStreamingSound(
			VMF(1),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;

	case UI_S_FADEALLSOUNDS:

		S_FadeAllSounds(
			VMF(1),
			rtcw::from_vm_arg<int>(args[2])
#if defined RTCW_ET
			,
			rtcw::from_vm_arg<qboolean>(args[3])
#endif // RTCW_XX
		);

		return 0;
//----(SA)	end
#endif // RTCW_XX

	case UI_KEY_KEYNUMTOSTRINGBUF:
		Key_KeynumToStringBuf(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;

	case UI_KEY_GETBINDINGBUF:
		Key_GetBindingBuf(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;

	case UI_KEY_SETBINDING:
		Key_SetBinding(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<const char*>(VMA(2))
		);
		return 0;

#if defined RTCW_ET
	case UI_KEY_BINDINGTOKEYS:
		Key_GetBindingByString(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<int*>(VMA(2)),
			rtcw::from_vm_arg<int*>(VMA(3))
		);
		return 0;
#endif // RTCW_XX

	case UI_KEY_ISDOWN:
		return Key_IsDown(rtcw::from_vm_arg<int>(args[1]));

	case UI_KEY_GETOVERSTRIKEMODE:
		return Key_GetOverstrikeMode();

	case UI_KEY_SETOVERSTRIKEMODE:
		Key_SetOverstrikeMode(rtcw::from_vm_arg<qboolean>(args[1]));
		return 0;

	case UI_KEY_CLEARSTATES:
		Key_ClearStates();
		return 0;

	case UI_KEY_GETCATCHER:
		return Key_GetCatcher();

	case UI_KEY_SETCATCHER:
		Key_SetCatcher(rtcw::from_vm_arg<int>(args[1]));
		return 0;

	case UI_GETCLIPBOARDDATA:
		GetClipboardData(
			rtcw::from_vm_arg<char*>(VMA(1)),
			rtcw::from_vm_arg<int>(args[2])
		);
		return 0;

	case UI_GETCLIENTSTATE:
		GetClientState(rtcw::from_vm_arg<uiClientState_t*>(VMA(1)));
		return 0;

	case UI_GETGLCONFIG:
		CL_GetGlconfig(rtcw::from_vm_arg<glconfig_t*>(VMA(1)));
		return 0;

	case UI_GETCONFIGSTRING:
		return GetConfigString(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);

	case UI_LAN_LOADCACHEDSERVERS:
		LAN_LoadCachedServers();
		return 0;

	case UI_LAN_SAVECACHEDSERVERS:
		LAN_SaveServersToCache();
		return 0;

	case UI_LAN_ADDSERVER:
		return LAN_AddServer(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<const char*>(VMA(2)),
			rtcw::from_vm_arg<const char*>(VMA(3))
		);

	case UI_LAN_REMOVESERVER:
		LAN_RemoveServer(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<const char*>(VMA(2))
		);
		return 0;

	case UI_LAN_GETPINGQUEUECOUNT:
		return LAN_GetPingQueueCount();

	case UI_LAN_CLEARPING:
		LAN_ClearPing(rtcw::from_vm_arg<int>(args[1]));
		return 0;

	case UI_LAN_GETPING:
		LAN_GetPing(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3]),
			rtcw::from_vm_arg<int*>(VMA(4))
		);
		return 0;

	case UI_LAN_GETPINGINFO:
		LAN_GetPingInfo(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);
		return 0;

	case UI_LAN_GETSERVERCOUNT:
		return LAN_GetServerCount(rtcw::from_vm_arg<int>(args[1]));

	case UI_LAN_GETSERVERADDRESSSTRING:
		LAN_GetServerAddressString(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<char*>(VMA(3)),
			rtcw::from_vm_arg<int>(args[4])
		);
		return 0;

	case UI_LAN_GETSERVERINFO:
		LAN_GetServerInfo(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<char*>(VMA(3)),
			rtcw::from_vm_arg<int>(args[4])
		);
		return 0;

	case UI_LAN_GETSERVERPING:
		return LAN_GetServerPing(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<int>(args[2])
		);

	case UI_LAN_MARKSERVERVISIBLE:
		LAN_MarkServerVisible(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<qboolean>(args[3])
		);
		return 0;

	case UI_LAN_SERVERISVISIBLE:
		return LAN_ServerIsVisible(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<int>(args[2])
		);

	case UI_LAN_UPDATEVISIBLEPINGS:
		return LAN_UpdateVisiblePings(rtcw::from_vm_arg<int>(args[1]));

	case UI_LAN_RESETPINGS:
		LAN_ResetPings(rtcw::from_vm_arg<int>(args[1]));
		return 0;

	case UI_LAN_SERVERSTATUS:
		return LAN_GetServerStatus(
			rtcw::from_vm_arg<char*>(VMA(1)),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int>(args[3])
		);

#if defined RTCW_ET
	case UI_LAN_SERVERISINFAVORITELIST:
		return LAN_ServerIsInFavoriteList(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<int>(args[2])
		);
#endif // RTCW_XX

#if !defined RTCW_SP
	case UI_SET_PBCLSTATUS:
		return 0;

	case UI_SET_PBSVSTATUS:
		return 0;
#endif // RTCW_XX

	case UI_LAN_COMPARESERVERS:
		return LAN_CompareServers(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<int>(args[3]),
			rtcw::from_vm_arg<int>(args[4]),
			rtcw::from_vm_arg<int>(args[5])
		);

	case UI_MEMORY_REMAINING:
		return Hunk_MemoryRemaining();

	case UI_GET_CDKEY:
		CLUI_GetCDKey(
			rtcw::from_vm_arg<char*>(VMA(1)),
			rtcw::from_vm_arg<int>(args[2])
		);
		return 0;

	case UI_SET_CDKEY:
		CLUI_SetCDKey(rtcw::from_vm_arg<char*>(VMA(1)));
		return 0;

	case UI_R_REGISTERFONT:
		re.RegisterFont(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<fontInfo_t*> (VMA(3))
		);
		return 0;

// FIXME Not used
#if FIXME
	case UI_MEMSET:
		return (int)memset( VMA(1), args[2], args[3] );

	case UI_MEMCPY:
		return (int)memcpy( VMA(1), VMA(2), args[3] );

	case UI_STRNCPY:
		return (int)strncpy( static_cast<char*> (VMA(1)), static_cast<const char*> (VMA(2)), args[3] );

	case UI_SIN:
		return FloatAsInt( c::sin( VMF(1) ) );

	case UI_COS:
		return FloatAsInt( c::cos( VMF(1) ) );

	case UI_ATAN2:
		return FloatAsInt( c::atan2( VMF(1), VMF(2) ) );

	case UI_SQRT:
		return FloatAsInt( c::sqrt( VMF(1) ) );

	case UI_FLOOR:
		return FloatAsInt( c::floor( VMF(1) ) );

	case UI_CEIL:
		return FloatAsInt( c::ceil( VMF(1) ) );
#endif // FIXME

	case UI_PC_ADD_GLOBAL_DEFINE:
		return botlib_export->PC_AddGlobalDefine(rtcw::from_vm_arg<const char*>(VMA(1)));

#if defined RTCW_ET
	case UI_PC_REMOVE_ALL_GLOBAL_DEFINES:
		botlib_export->PC_RemoveAllGlobalDefines();
		return 0;
#endif // RTCW_XX

	case UI_PC_LOAD_SOURCE:
		return botlib_export->PC_LoadSourceHandle(rtcw::from_vm_arg<const char*>(VMA(1)));

	case UI_PC_FREE_SOURCE:
		return botlib_export->PC_FreeSourceHandle(rtcw::from_vm_arg<int>(args[1]));

	case UI_PC_READ_TOKEN:
		return botlib_export->PC_ReadTokenHandle(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<pc_token_t*>(VMA(2))
		);

	case UI_PC_SOURCE_FILE_AND_LINE:
		return botlib_export->PC_SourceFileAndLine(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<char*>(VMA(2)),
			rtcw::from_vm_arg<int*>(VMA(3))
		);

#if defined RTCW_ET
	case UI_PC_UNREAD_TOKEN:
		botlib_export->PC_UnreadLastTokenHandle(rtcw::from_vm_arg<int>(args[1]));
		return 0;
#endif // RTCW_XX

	case UI_S_STOPBACKGROUNDTRACK:
		S_StopBackgroundTrack();
		return 0;

	case UI_S_STARTBACKGROUNDTRACK:
		S_StartBackgroundTrack(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<const char*>(VMA(2))
#if !defined RTCW_MP
			,
			rtcw::from_vm_arg<int>(args[3])
#endif // RTCW_XX
		);   //----(SA)	added fadeup time
		return 0;

	case UI_REAL_TIME:
		return Com_RealTime(rtcw::from_vm_arg<qtime_t*>(VMA(1)));

	case UI_CIN_PLAYCINEMATIC:
		Com_DPrintf("UI_CIN_PlayCinematic\n");

		return CIN_PlayCinematic(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<int>(args[3]),
			rtcw::from_vm_arg<int>(args[4]),
			rtcw::from_vm_arg<int>(args[5]),
			rtcw::from_vm_arg<int>(args[6])
		);

	case UI_CIN_STOPCINEMATIC:
		return CIN_StopCinematic(rtcw::from_vm_arg<int>(args[1]));

	case UI_CIN_RUNCINEMATIC:
		return CIN_RunCinematic(rtcw::from_vm_arg<int>(args[1]));

	case UI_CIN_DRAWCINEMATIC:
		CIN_DrawCinematic(rtcw::from_vm_arg<int>(args[1]));
		return 0;

	case UI_CIN_SETEXTENTS:
		CIN_SetExtents(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<int>(args[2]),
			rtcw::from_vm_arg<int>(args[3]),
			rtcw::from_vm_arg<int>(args[4]),
			rtcw::from_vm_arg<int>(args[5])
		);
		return 0;

	case UI_R_REMAP_SHADER:
		re.RemapShader(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<const char*>(VMA(2)),
			rtcw::from_vm_arg<const char*>(VMA(3))
		);
		return 0;

	case UI_VERIFY_CDKEY:
		return CL_CDKeyValidate(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<const char*>(VMA(2))
		);

		// NERVE - SMF
	case UI_CL_GETLIMBOSTRING:
		return CL_GetLimboString(
			rtcw::from_vm_arg<int>(args[1]),
			rtcw::from_vm_arg<char*>(VMA(2))
		);

#if defined RTCW_SP
		// -NERVE - SMF
#endif // RTCW_XX

#if !defined RTCW_SP
	case UI_CL_TRANSLATE_STRING:
		CL_TranslateString(
			rtcw::from_vm_arg<const char*>(VMA(1)),
			rtcw::from_vm_arg<char*>(VMA(2))
		);
		return 0;
		// -NERVE - SMF

		// DHM - Nerve
	case UI_CHECKAUTOUPDATE:
		CL_CheckAutoUpdate();
		return 0;

	case UI_GET_AUTOUPDATE:
		CL_GetAutoUpdate();
		return 0;
		// DHM - Nerve

	case UI_OPENURL:
		CL_OpenURL(rtcw::from_vm_arg<const char*>(VMA(1)));
		return 0;
#endif // RTCW_XX

#if defined RTCW_ET
	case UI_GETHUNKDATA:
		Com_GetHunkInfo(
			rtcw::from_vm_arg<int*>(VMA(1)),
			rtcw::from_vm_arg<int*>(VMA(2))
		);
		return 0;
#endif // RTCW_XX


	default:
		Com_Error( ERR_DROP, "Bad UI system trap: %" PRIiPTR, args[0] );
	}

	return 0;
}

/*
====================
CL_ShutdownUI
====================
*/
void CL_ShutdownUI( void ) {
	cls.keyCatchers &= ~KEYCATCH_UI;
	cls.uiStarted = qfalse;
	if ( !uivm ) {
		return;
	}
	VM_Call(uivm, UI_SHUTDOWN);
	VM_Free( uivm );
	uivm = NULL;
}

/*
====================
CL_InitUI
====================
*/

void CL_InitUI( void ) {
	int v;

// BBi
//#if defined RTCW_SP
//	vmInterpret_t interpret;
//
//	// load the dll or bytecode
//	if ( cl_connectedToPureServer != 0 ) {
//		// if sv_pure is set we only allow qvms to be loaded
//		interpret = VMI_COMPILED;
//	} else {
//		interpret = vmInterpret_t (int (Cvar_VariableValue( "vm_ui" )));
//	}
//
////----(SA)	always dll
//
//#ifdef WOLF_SP_DEMO
//	uivm = VM_Create( "ui", CL_UISystemCalls, VMI_NATIVE );
//#else
//	uivm = VM_Create( "ui", CL_UISystemCalls, vmInterpret_t (int (Cvar_VariableValue( "vm_ui" ))) );
//#endif
//#else
//	uivm = VM_Create( "ui", CL_UISystemCalls, VMI_NATIVE );
//#endif // RTCW_XX

	uivm = VM_Create ("ui", CL_UISystemCalls);
// BBi

	if ( !uivm ) {
		Com_Error( ERR_FATAL, "VM_Create on UI failed" );
	}

	// sanity check
	v = VM_Call(uivm, UI_GETAPIVERSION);

	if ( v != UI_API_VERSION ) {
		Com_Error( ERR_FATAL, "User Interface is version %d, expected %d", v, UI_API_VERSION );
		cls.uiStarted = qfalse;
	}

	// init for this gamestate
	VM_Call(
		uivm,
		rtcw::to_vm_arg(UI_INIT),
		rtcw::to_vm_arg(cls.state >= CA_AUTHORIZING && cls.state < CA_ACTIVE)
	);
}


qboolean UI_usesUniqueCDKey() {
	if ( uivm ) {
		return ( VM_Call(uivm, UI_HASUNIQUECDKEY) == qtrue );
	} else {
		return qfalse;
	}
}

#if !defined RTCW_SP
qboolean UI_checkKeyExec( int key ) {
	if ( uivm ) {
		return VM_Call(uivm, UI_CHECKEXECKEY, rtcw::to_vm_arg(key));
	} else {
		return qfalse;
	}
}
#endif // RTCW_XX

/*
====================
UI_GameCommand

See if the current console command is claimed by the ui
====================
*/
qboolean UI_GameCommand( void ) {
	if ( !uivm ) {
		return qfalse;
	}

	return VM_Call(uivm, UI_CONSOLE_COMMAND, rtcw::to_vm_arg(cls.realtime));
}
