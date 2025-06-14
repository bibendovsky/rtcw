/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// cl_main.c  -- client main loop

#include "client.h"
#include <limits.h>
#include "SDL_platform.h"

#if defined RTCW_MP

cvar_t  *cl_wavefilerecord;
#endif // RTCW_XX

#if defined RTCW_ET
#include "snd_local.h" // fretn

cvar_t  *cl_wavefilerecord;
#endif // RTCW_XX

#include "rtcw_endian.h"
#include "rtcw_vm_args.h"


cvar_t  *cl_nodelta;
cvar_t  *cl_debugMove;

cvar_t  *cl_noprint;
cvar_t  *cl_motd;

#if !defined RTCW_SP
cvar_t  *cl_autoupdate;         // DHM - Nerve
#endif // RTCW_XX

cvar_t  *rcon_client_password;
cvar_t  *rconAddress;

cvar_t  *cl_timeout;
cvar_t  *cl_maxpackets;
cvar_t  *cl_packetdup;
cvar_t  *cl_timeNudge;
cvar_t  *cl_showTimeDelta;
cvar_t  *cl_freezeDemo;

cvar_t  *cl_shownet = NULL;     // NERVE - SMF - This is referenced in msg.c and we need to make sure it is NULL

#if !defined RTCW_SP
cvar_t  *cl_shownuments;        // DHM - Nerve
cvar_t  *cl_visibleClients;     // DHM - Nerve
#endif // RTCW_XX

cvar_t  *cl_showSend;

#if !defined RTCW_SP
cvar_t  *cl_showServerCommands; // NERVE - SMF
#endif // RTCW_XX

cvar_t  *cl_timedemo;
cvar_t  *cl_avidemo;
cvar_t  *cl_forceavidemo;

cvar_t  *cl_freelook;
cvar_t  *cl_sensitivity;

cvar_t  *cl_mouseAccel;
cvar_t  *cl_showMouseRate;

cvar_t  *m_pitch;
cvar_t  *m_yaw;
cvar_t  *m_forward;
cvar_t  *m_side;
cvar_t  *m_filter;

cvar_t  *cl_activeAction;

#if defined RTCW_ET
cvar_t  *cl_autorecord;
#endif // RTCW_XX

cvar_t  *cl_motdString;

cvar_t  *cl_allowDownload;

#if defined RTCW_ET
cvar_t  *cl_wwwDownload;
#endif // RTCW_XX

cvar_t  *cl_conXOffset;
cvar_t  *cl_inGameVideo;

cvar_t  *cl_serverStatusResendTime;
cvar_t  *cl_trn;
cvar_t  *cl_missionStats;
cvar_t  *cl_waitForFire;

// NERVE - SMF - localization
cvar_t  *cl_language;
cvar_t  *cl_debugTranslation;
// -NERVE - SMF

#if defined RTCW_SP
char cl_cdkey[34] = "                                ";
#else
// DHM - Nerve :: Auto-Update
cvar_t  *cl_updateavailable;
cvar_t  *cl_updatefiles;
// DHM - Nerve
#endif // RTCW_XX

#if defined RTCW_ET
cvar_t  *cl_profile;
cvar_t  *cl_defaultProfile;

cvar_t  *cl_demorecording; // fretn
cvar_t  *cl_demofilename; // bani
cvar_t  *cl_demooffset; // bani

cvar_t  *cl_waverecording; //bani
cvar_t  *cl_wavefilename; //bani
cvar_t  *cl_waveoffset; //bani

cvar_t  *cl_packetloss; //bani
cvar_t  *cl_packetdelay;    //bani
//extern qboolean sv_cheats;  //bani
#endif // RTCW_XX


clientActive_t cl;
clientConnection_t clc;
clientStatic_t cls;
vm_t                *cgvm;

// Structure containing functions exported from refresh DLL
refexport_t re;

ping_t cl_pinglist[MAX_PINGREQUESTS];

typedef struct serverStatus_s
{
	char string[BIG_INFO_STRING];
	netadr_t address;
	int time, startTime;
	qboolean pending;
	qboolean print;
	qboolean retrieved;
} serverStatus_t;

serverStatus_t cl_serverStatusList[MAX_SERVERSTATUSREQUESTS];
int serverStatusCount;

#if defined RTCW_SP
#if 0 // MrE defined __USEA3D && defined __A3D_GEOM
void hA3Dg_ExportRenderGeom( refexport_t *incoming_re );
#endif
#else
// DHM - Nerve :: Have we heard from the auto-update server this session?
qboolean autoupdateChecked;
qboolean autoupdateStarted;
// TTimo : moved from char* to array (was getting the char* from va(), broke on big downloads)
char autoupdateFilename[MAX_QPATH];
// "updates" shifted from -7
#define AUTOUPDATE_DIR "ni]Zm^l"
#define AUTOUPDATE_DIR_SHIFT 7
#endif // RTCW_XX

extern void SV_BotFrame( int time );
void CL_CheckForResend( void );
void CL_ShowIP_f( void );
void CL_ServerStatus_f( void );
void CL_ServerStatusResponse( netadr_t from, msg_t *msg );

#if !defined RTCW_SP
void CL_SaveTranslations_f( void );
void CL_LoadTranslations_f( void );
#endif // RTCW_XX

#if defined RTCW_SP
/*
==============
CL_EndgameMenu

Called by Com_Error when a game has ended and is dropping out to main menu in the "endgame" menu ('credits' right now)
==============
*/
void CL_EndgameMenu( void ) {
	cls.endgamemenu = qtrue;    // start it next frame
}
#endif // RTCW_XX

#if defined RTCW_ET
// fretn
void CL_WriteWaveClose( void );
void CL_WavStopRecord_f( void );
#endif // RTCW_XX

/*
===============
CL_CDDialog

Called by Com_Error when a cd is needed
===============
*/
void CL_CDDialog( void ) {
	cls.cddialog = qtrue;   // start it next frame
}

#if defined RTCW_ET
void CL_PurgeCache( void ) {
	cls.doCachePurge = qtrue;
}

void CL_DoPurgeCache( void ) {
	if ( !cls.doCachePurge ) {
		return;
	}

	cls.doCachePurge = qfalse;

	if ( !com_cl_running ) {
		return;
	}

	if ( !com_cl_running->integer ) {
		return;
	}

	if ( !cls.rendererStarted ) {
		return;
	}

	re.purgeCache();
}
#endif // RTCW_XX

/*
=======================================================================

CLIENT RELIABLE COMMAND COMMUNICATION

=======================================================================
*/

/*
======================
CL_AddReliableCommand

The given command will be transmitted to the server, and is gauranteed to
not have future usercmd_t executed before it is executed
======================
*/
void CL_AddReliableCommand( const char *cmd ) {
	int index;

	// if we would be losing an old command that hasn't been acknowledged,
	// we must drop the connection

#if defined RTCW_SP
//	if(cl.cameraMode)
//		Com_Printf ("cmd: %s\n", cmd);
#endif // RTCW_XX

	if ( clc.reliableSequence - clc.reliableAcknowledge > MAX_RELIABLE_COMMANDS ) {
		Com_Error( ERR_DROP, "Client command overflow" );
	}
	clc.reliableSequence++;
	index = clc.reliableSequence & ( MAX_RELIABLE_COMMANDS - 1 );
	Q_strncpyz( clc.reliableCommands[ index ], cmd, sizeof( clc.reliableCommands[ index ] ) );
}

/*
======================
CL_ChangeReliableCommand
======================
*/
void CL_ChangeReliableCommand( void ) {
	int r, index, l;

#if !defined RTCW_SP
	// NOTE TTimo: what is the randomize for?
#endif // RTCW_XX

	r = clc.reliableSequence - ( random() * 5 );
	index = clc.reliableSequence & ( MAX_RELIABLE_COMMANDS - 1 );
	l = strlen( clc.reliableCommands[ index ] );
	if ( l >= MAX_STRING_CHARS - 1 ) {
		l = MAX_STRING_CHARS - 2;
	}
	clc.reliableCommands[ index ][ l ] = '\n';
	clc.reliableCommands[ index ][ l + 1 ] = '\0';
}

/*
=======================================================================

CLIENT SIDE DEMO RECORDING

=======================================================================
*/

/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length
====================
*/
void CL_WriteDemoMessage( msg_t *msg, int headerBytes ) {
	int len, swlen;

	// write the packet sequence
	len = clc.serverMessageSequence;
	swlen = rtcw::Endian::le( len );
	FS_Write( &swlen, 4, clc.demofile );

	// skip the packet sequencing information
	len = msg->cursize - headerBytes;
	swlen = rtcw::Endian::le( len );
	FS_Write( &swlen, 4, clc.demofile );
	FS_Write( msg->data + headerBytes, len, clc.demofile );
}


/*
====================
CL_StopRecording_f

stop recording a demo
====================
*/
void CL_StopRecord_f( void ) {
	int len;

	if ( !clc.demorecording ) {
		Com_Printf( "Not recording a demo.\n" );
		return;
	}

	// finish up
	len = -1;
	FS_Write( &len, 4, clc.demofile );
	FS_Write( &len, 4, clc.demofile );
	FS_FCloseFile( clc.demofile );
	clc.demofile = 0;
	clc.demorecording = qfalse;

#if defined RTCW_ET
	Cvar_Set( "cl_demorecording", "0" ); // fretn
	Cvar_Set( "cl_demofilename", "" ); // bani
	Cvar_Set( "cl_demooffset", "0" ); // bani
#endif // RTCW_XX

	Com_Printf( "Stopped demo.\n" );
}

/*
==================
CL_DemoFilename
==================
*/
void CL_DemoFilename( int number, char *fileName ) {

#if !defined RTCW_ET
	int a,b,c,d;

	if ( number < 0 || number > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "demo9999.tga" );
		return;
	}

	a = number / 1000;
	number -= a * 1000;
	b = number / 100;
	number -= b * 100;
	c = number / 10;
	number -= c * 10;
	d = number;

	Com_sprintf( fileName, MAX_OSPATH, "demo%i%i%i%i"
				 , a, b, c, d );
#else
	if ( number < 0 || number > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "demo9999" ); // fretn - removed .tga
		return;
	}

	Com_sprintf( fileName, MAX_OSPATH, "demo%04i", number );
#endif // RTCW_XX

}

/*
====================
CL_Record_f

record <demoname>

Begins recording a demo from the current position
====================
*/
static char demoName[MAX_QPATH];        // compiler bug workaround
void CL_Record_f( void ) {
	char name[MAX_OSPATH];

#if !defined RTCW_ET
	byte bufData[MAX_MSGLEN];
	msg_t buf;
	int i;
#endif // RTCW_XX

	int len;

#if !defined RTCW_ET
	entityState_t   *ent;
	entityState_t nullstate;
#endif // RTCW_XX

	const char        *s;

	if ( Cmd_Argc() > 2 ) {
		Com_Printf( "record <demoname>\n" );
		return;
	}

	if ( clc.demorecording ) {
		Com_Printf( "Already recording.\n" );
		return;
	}

	if ( cls.state != CA_ACTIVE ) {
		Com_Printf( "You must be in a level to record.\n" );
		return;
	}

#if !defined RTCW_SP
	// ATVI Wolfenstein Misc #479 - changing this to a warning
	// sync 0 doesn't prevent recording, so not forcing it off .. everyone does g_sync 1 ; record ; g_sync 0 ..

#if defined RTCW_ET
	//if ( !Cvar_VariableValue( "g_synchronousClients" ) ) {
	//	Com_Printf (S_COLOR_YELLOW "WARNING: You should set 'g_synchronousClients 1' for smoother demo recording\n");
	//}
#endif // RTCW_XX

#endif // RTCW_XX

#if !defined RTCW_ET
	if ( !Cvar_VariableValue( "g_synchronousClients" ) ) {

#if defined RTCW_SP
		Com_Printf( "The server must have 'g_synchronousClients 1' set for demos\n" );
		return;
#elif defined RTCW_MP
		Com_Printf( S_COLOR_YELLOW "WARNING: You should set 'g_synchronousClients 1' for smoother demo recording\n" );
#endif // RTCW_XX

	}
#endif // RTCW_XX

	if ( Cmd_Argc() == 2 ) {
		s = Cmd_Argv( 1 );
		Q_strncpyz( demoName, s, sizeof( demoName ) );
		Com_sprintf( name, sizeof( name ), "demos/%s.dm_%d", demoName, PROTOCOL_VERSION );
	} else {
		int number;

		// scan for a free demo name
		for ( number = 0 ; number <= 9999 ; number++ ) {
			CL_DemoFilename( number, demoName );
			Com_sprintf( name, sizeof( name ), "demos/%s.dm_%d", demoName, PROTOCOL_VERSION );

			len = FS_ReadFile( name, NULL );
			if ( len <= 0 ) {
				break;  // file doesn't exist
			}
		}
	}

#if !defined RTCW_ET
	// open the demo file

	Com_Printf( "recording to %s.\n", name );
	clc.demofile = FS_FOpenFileWrite( name );
	if ( !clc.demofile ) {
		Com_Printf( "ERROR: couldn't open.\n" );
		return;
	}
	clc.demorecording = qtrue;
	Q_strncpyz( clc.demoName, demoName, sizeof( clc.demoName ) );

	// don't start saving messages until a non-delta compressed message is received
	clc.demowaiting = qtrue;

	// write out the gamestate message
	MSG_Init( &buf, bufData, sizeof( bufData ) );
	MSG_Bitstream( &buf );

	// NOTE, MRE: all server->client messages now acknowledge
	MSG_WriteLong( &buf, clc.reliableSequence );

	MSG_WriteByte( &buf, svc_gamestate );
	MSG_WriteLong( &buf, clc.serverCommandSequence );

	// configstrings
	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		if ( !cl.gameState.stringOffsets[i] ) {
			continue;
		}
		s = cl.gameState.stringData + cl.gameState.stringOffsets[i];
		MSG_WriteByte( &buf, svc_configstring );
		MSG_WriteShort( &buf, i );
		MSG_WriteBigString( &buf, s );
	}

	// baselines
	memset( &nullstate, 0, sizeof( nullstate ) );
	for ( i = 0; i < MAX_GENTITIES ; i++ ) {
		ent = &cl.entityBaselines[i];
		if ( !ent->number ) {
			continue;
		}
		MSG_WriteByte( &buf, svc_baseline );
		MSG_WriteDeltaEntity( &buf, &nullstate, ent, qtrue );
	}

	MSG_WriteByte( &buf, svc_EOF );

	// finished writing the gamestate stuff

	// write the client num
	MSG_WriteLong( &buf, clc.clientNum );
	// write the checksum feed
	MSG_WriteLong( &buf, clc.checksumFeed );

	// finished writing the client packet
	MSG_WriteByte( &buf, svc_EOF );

	// write it to the demo file
	len = rtcw::Endian::le( clc.serverMessageSequence - 1 );
	FS_Write( &len, 4, clc.demofile );

	len = rtcw::Endian::le( buf.cursize );
	FS_Write( &len, 4, clc.demofile );
	FS_Write( buf.data, buf.cursize, clc.demofile );

	// the rest of the demo file will be copied from net messages
#else
	CL_Record( name );
#endif // RTCW_XX

}

#if defined RTCW_ET
void CL_Record( const char* name ) {
	int i;
	msg_t buf;
	byte bufData[MAX_MSGLEN];
	entityState_t   *ent;
	entityState_t nullstate;
	char        *s;
	int len;

	// open the demo file

	Com_Printf( "recording to %s.\n", name );
	clc.demofile = FS_FOpenFileWrite( name );
	if ( !clc.demofile ) {
		Com_Printf( "ERROR: couldn't open.\n" );
		return;
	}

	clc.demorecording = qtrue;
	Cvar_Set( "cl_demorecording", "1" ); // fretn
	Q_strncpyz( clc.demoName, demoName, sizeof( clc.demoName ) );
	Cvar_Set( "cl_demofilename", clc.demoName ); // bani
	Cvar_Set( "cl_demooffset", "0" ); // bani

	// don't start saving messages until a non-delta compressed message is received
	clc.demowaiting = qtrue;

	// write out the gamestate message
	MSG_Init( &buf, bufData, sizeof( bufData ) );
	MSG_Bitstream( &buf );

	// NOTE, MRE: all server->client messages now acknowledge
	MSG_WriteLong( &buf, clc.reliableSequence );

	MSG_WriteByte( &buf, svc_gamestate );
	MSG_WriteLong( &buf, clc.serverCommandSequence );

	// configstrings
	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		if ( !cl.gameState.stringOffsets[i] ) {
			continue;
		}
		s = cl.gameState.stringData + cl.gameState.stringOffsets[i];
		MSG_WriteByte( &buf, svc_configstring );
		MSG_WriteShort( &buf, i );
		MSG_WriteBigString( &buf, s );
	}

	// baselines
	memset( &nullstate, 0, sizeof( nullstate ) );
	for ( i = 0; i < MAX_GENTITIES ; i++ ) {
		ent = &cl.entityBaselines[i];
		if ( !ent->number ) {
			continue;
		}
		MSG_WriteByte( &buf, svc_baseline );
		MSG_WriteDeltaEntity( &buf, &nullstate, ent, qtrue );
	}

	MSG_WriteByte( &buf, svc_EOF );

	// finished writing the gamestate stuff

	// write the client num
	MSG_WriteLong( &buf, clc.clientNum );
	// write the checksum feed
	MSG_WriteLong( &buf, clc.checksumFeed );

	// finished writing the client packet
	MSG_WriteByte( &buf, svc_EOF );

	// write it to the demo file
	len = rtcw::Endian::le( clc.serverMessageSequence - 1 );
	FS_Write( &len, 4, clc.demofile );

	len = rtcw::Endian::le( buf.cursize );
	FS_Write( &len, 4, clc.demofile );
	FS_Write( buf.data, buf.cursize, clc.demofile );

	// the rest of the demo file will be copied from net messages
}
#endif // RTCW_XX

/*
=======================================================================

CLIENT SIDE DEMO PLAYBACK

=======================================================================
*/

/*
=================
CL_DemoCompleted
=================
*/
void CL_DemoCompleted( void ) {
	if ( cl_timedemo && cl_timedemo->integer ) {
		int time;

		time = Sys_Milliseconds() - clc.timeDemoStart;
		if ( time > 0 ) {
			Com_Printf( "%i frames, %3.1f seconds: %3.1f fps\n", clc.timeDemoFrames,
						time / 1000.0, clc.timeDemoFrames * 1000.0 / time );
		}
	}

#if defined RTCW_ET
	// fretn
	if ( clc.waverecording ) {
		CL_WriteWaveClose();
		clc.waverecording = qfalse;
	}
#endif // RTCW_XX

	CL_Disconnect( qtrue );
	CL_NextDemo();
}

/*
=================
CL_ReadDemoMessage
=================
*/
void CL_ReadDemoMessage( void ) {
	int r;
	msg_t buf;
	byte bufData[ MAX_MSGLEN ];
	int s;

	if ( !clc.demofile ) {
		CL_DemoCompleted();
		return;
	}

	// get the sequence number
	r = FS_Read( &s, 4, clc.demofile );
	if ( r != 4 ) {
		CL_DemoCompleted();
		return;
	}
	clc.serverMessageSequence = rtcw::Endian::le( s );

	// init the message
	MSG_Init( &buf, bufData, sizeof( bufData ) );

	// get the length
	r = FS_Read( &buf.cursize, 4, clc.demofile );
	if ( r != 4 ) {
		CL_DemoCompleted();
		return;
	}
	rtcw::Endian::lei(buf.cursize);
	if ( buf.cursize == -1 ) {
		CL_DemoCompleted();
		return;
	}
	if ( buf.cursize > buf.maxsize ) {
		Com_Error( ERR_DROP, "CL_ReadDemoMessage: demoMsglen > MAX_MSGLEN" );
	}
	r = FS_Read( buf.data, buf.cursize, clc.demofile );
	if ( r != buf.cursize ) {
		Com_Printf( "Demo file was truncated.\n" );
		CL_DemoCompleted();
		return;
	}

	clc.lastPacketTime = cls.realtime;
	buf.readcount = 0;
	CL_ParseServerMessage( &buf );
}

#if !defined RTCW_SP

#if !defined RTCW_MP
/*
====================

  Wave file saving functions

====================
*/
#else
/*
====================

  Wave file saving functions

  FIXME: make this actually work

====================
*/
#endif // RTCW_XX

#if defined RTCW_ET
/*
==================
CL_DemoFilename
==================
*/
void CL_WavFilename( int number, char *fileName ) {
	if ( number < 0 || number > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "wav9999" ); // fretn - removed .tga
		return;
	}

	Com_sprintf( fileName, MAX_OSPATH, "wav%04i", number );
}

typedef struct wav_hdr_s {
	unsigned int ChunkID;       // big endian
	unsigned int ChunkSize;     // little endian
	unsigned int Format;        // big endian

	unsigned int Subchunk1ID;   // big endian
	unsigned int Subchunk1Size; // little endian
	unsigned short AudioFormat; // little endian
	unsigned short NumChannels; // little endian
	unsigned int SampleRate;    // little endian
	unsigned int ByteRate;      // little endian
	unsigned short BlockAlign;  // little endian
	unsigned short BitsPerSample;   // little endian

	unsigned int Subchunk2ID;   // big endian
	unsigned int Subchunk2Size;     // little indian ;)

	unsigned int NumSamples;
} wav_hdr_t;

wav_hdr_t hdr;

static void CL_WriteWaveHeader( void ) {
	memset( &hdr, 0, sizeof( hdr ) );

	hdr.ChunkID = 0x46464952;       // "RIFF"
	hdr.ChunkSize = 0;          // total filesize - 8 bytes
	hdr.Format = 0x45564157;        // "WAVE"

	hdr.Subchunk1ID = 0x20746d66;       // "fmt "
	hdr.Subchunk1Size = 16;         // 16 = pcm
	hdr.AudioFormat = 1;            // 1 = linear quantization
	hdr.NumChannels = 2;            // 2 = stereo

	hdr.SampleRate = dma.speed;

	hdr.BitsPerSample = 16;         // 16bits

	// SampleRate * NumChannels * BitsPerSample/8
	hdr.ByteRate = hdr.SampleRate * hdr.NumChannels * ( hdr.BitsPerSample / 8 );

	// NumChannels * BitsPerSample/8
	hdr.BlockAlign = hdr.NumChannels * ( hdr.BitsPerSample / 8 );

	hdr.Subchunk2ID = 0x61746164;       // "data"

	hdr.Subchunk2Size = 0;          // NumSamples * NumChannels * BitsPerSample/8

	// ...
	FS_Write( &hdr.ChunkID, 44, clc.wavefile );
}

static char wavName[MAX_QPATH];     // compiler bug workaround
#endif // RTCW_XX

void CL_WriteWaveOpen() {
	// we will just save it as a 16bit stereo 22050kz pcm file

#if !defined RTCW_ET
	clc.wavefile = FS_FOpenFileWrite( "demodata.pcm" );
	clc.wavetime = -1;
#else
	char name[MAX_OSPATH];
	int len;
	const char        *s;

	if ( Cmd_Argc() > 2 ) {
		Com_Printf( "wav_record <wavname>\n" );
		return;
	}

	if ( clc.waverecording ) {
		Com_Printf( "Already recording a wav file\n" );
		return;
	}

	// yes ... no ? leave it up to them imo
	//if (cl_avidemo.integer)
	//	return;

	if ( Cmd_Argc() == 2 ) {
		s = Cmd_Argv( 1 );
		Q_strncpyz( wavName, s, sizeof( wavName ) );
		Com_sprintf( name, sizeof( name ), "wav/%s.wav", wavName );
	} else {
		int number;

		// I STOLE THIS
		for ( number = 0 ; number <= 9999 ; number++ ) {
			CL_WavFilename( number, wavName );
			Com_sprintf( name, sizeof( name ), "wav/%s.wav", wavName );

			len = FS_FileExists( name );
			if ( len <= 0 ) {
				break;  // file doesn't exist
			}
		}
	}

	Com_Printf( "recording to %s.\n", name );
	clc.wavefile = FS_FOpenFileWrite( name );

	if ( !clc.wavefile ) {
		Com_Printf( "ERROR: couldn't open %s for writing.\n", name );
		return;
	}

	CL_WriteWaveHeader();
	clc.wavetime = -1;

	clc.waverecording = qtrue;

	Cvar_Set( "cl_waverecording", "1" );
	Cvar_Set( "cl_wavefilename", wavName );
	Cvar_Set( "cl_waveoffset", "0" );
#endif // RTCW_XX

}

void CL_WriteWaveClose() {

#if defined RTCW_ET
	Com_Printf( "Stopped recording\n" );

	hdr.Subchunk2Size = hdr.NumSamples * hdr.NumChannels * ( hdr.BitsPerSample / 8 );
	hdr.ChunkSize = 36 + hdr.Subchunk2Size;

	FS_Seek( clc.wavefile, 4, FS_SEEK_SET );
	FS_Write( &hdr.ChunkSize, 4, clc.wavefile );
	FS_Seek( clc.wavefile, 40, FS_SEEK_SET );
	FS_Write( &hdr.Subchunk2Size, 4, clc.wavefile );
#endif // RTCW_XX

	// and we're outta here
	FS_FCloseFile( clc.wavefile );

#if defined RTCW_ET
	clc.wavefile = 0;
#endif // RTCW_XX

}

extern int s_soundtime;

#if !defined RTCW_ET
// BBi
//extern portable_samplepair_t *paintbuffer;
extern portable_samplepair_t paintbuffer[];
// BBi
#else
extern int s_paintedtime;
extern portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
portable_samplepair_t wavbuffer[PAINTBUFFER_SIZE];
#endif // RTCW_XX

#if !defined RTCW_ET
void CL_WriteWaveFilePacket() {
	int total, i;
	if ( clc.wavetime == -1 ) {
		clc.wavetime = s_soundtime;
		return;
	}

	total = s_soundtime - clc.wavetime;
	clc.wavetime = s_soundtime;

	for ( i = 0; i < total; i++ ) {
		int parm;
		short out;
		parm =  ( paintbuffer[i].left ) >> 8;
		if ( parm > 32767 ) {
			parm = 32767;
		}
		if ( parm < -32768 ) {
			parm = -32768;
		}
		out = parm;
		FS_Write( &out, 2, clc.wavefile );
		parm =  ( paintbuffer[i].right ) >> 8;
		if ( parm > 32767 ) {
			parm = 32767;
		}
		if ( parm < -32768 ) {
			parm = -32768;
		}
		out = parm;
		FS_Write( &out, 2, clc.wavefile );
	}
}
#else
void CL_WriteWaveFilePacket( int endtime ) {
	int total, i;

	if ( !clc.waverecording || !clc.wavefile ) {
		return;
	}

	if ( clc.wavetime == -1 ) {
		clc.wavetime = s_soundtime;

		memcpy( wavbuffer, paintbuffer, sizeof( wavbuffer ) );
		return;
	}

	if ( s_soundtime <= clc.wavetime ) {
		return;
	}

	total = s_soundtime - clc.wavetime;

	if ( total < 1 ) {
		return;
	}

	clc.wavetime = s_soundtime;

	for ( i = 0; i < total; i++ ) {
		int parm;
		short out;

		parm = ( wavbuffer[i].left ) >> 8;
		if ( parm > 32767 ) {
			parm = 32767;
		}
		if ( parm < -32768 ) {
			parm = -32768;
		}
		out = parm;
		FS_Write( &out, 2, clc.wavefile );

		parm = ( wavbuffer[i].right ) >> 8;
		if ( parm > 32767 ) {
			parm = 32767;
		}
		if ( parm < -32768 ) {
			parm = -32768;
		}
		out = parm;
		FS_Write( &out, 2, clc.wavefile );
		hdr.NumSamples++;
	}
	memcpy( wavbuffer, paintbuffer, sizeof( wavbuffer ) );

	Cvar_Set( "cl_waveoffset", va( "%d", FS_FTell( clc.wavefile ) ) );
}
#endif // RTCW_XX

#endif // RTCW_XX

/*
====================
CL_PlayDemo_f

demo <demoname>

====================
*/
void CL_PlayDemo_f( void ) {
	char name[MAX_OSPATH], extension[32];
	const char        *arg;

#if defined RTCW_ET
	int prot_ver;
#endif // RTCW_XX


	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "playdemo <demoname>\n" );
		return;
	}

	// make sure a local server is killed
	Cvar_Set( "sv_killserver", "1" );

	CL_Disconnect( qtrue );


//	CL_FlushMemory();	//----(SA)	MEM NOTE: in missionpack, this is moved to CL_DownloadsComplete


	// open the demo file
	arg = Cmd_Argv( 1 );

#if !defined RTCW_ET
	Com_sprintf( extension, sizeof( extension ), ".dm_%d", PROTOCOL_VERSION );
	if ( !Q_stricmp( arg + strlen( arg ) - strlen( extension ), extension ) ) {
		Com_sprintf( name, sizeof( name ), "demos/%s", arg );
	} else {
		Com_sprintf( name, sizeof( name ), "demos/%s.dm_%d", arg, PROTOCOL_VERSION );
	}

	FS_FOpenFileRead( name, &clc.demofile, qtrue );
#else
	prot_ver = PROTOCOL_VERSION - 1;
	while ( prot_ver <= PROTOCOL_VERSION && !clc.demofile ) {
		Com_sprintf( extension, sizeof( extension ), ".dm_%d", prot_ver );
		if ( !Q_stricmp( arg + strlen( arg ) - strlen( extension ), extension ) ) {
			Com_sprintf( name, sizeof( name ), "demos/%s", arg );
		} else {
			Com_sprintf( name, sizeof( name ), "demos/%s.dm_%d", arg, prot_ver );
		}
		FS_FOpenFileRead( name, &clc.demofile, qtrue );
		prot_ver++;
	}
#endif // RTCW_XX

	if ( !clc.demofile ) {
		Com_Error( ERR_DROP, "couldn't open %s", name );
		return;
	}
	Q_strncpyz( clc.demoName, Cmd_Argv( 1 ), sizeof( clc.demoName ) );

	Con_Close();

	cls.state = CA_CONNECTED;
	clc.demoplaying = qtrue;

#if !defined RTCW_SP
	if ( Cvar_VariableValue( "cl_wavefilerecord" ) ) {
		CL_WriteWaveOpen();

#if !defined RTCW_ET
		clc.waverecording = qtrue;
#endif // RTCW_XX

	}
#endif // RTCW_XX

	Q_strncpyz( cls.servername, Cmd_Argv( 1 ), sizeof( cls.servername ) );

	// read demo messages until connected
	while ( cls.state >= CA_CONNECTED && cls.state < CA_PRIMED ) {
		CL_ReadDemoMessage();

#if defined RTCW_MP
		if ( clc.waverecording ) {
			CL_WriteWaveFilePacket();
		}
#endif // RTCW_XX

	}
	// don't get the first snapshot this frame, to prevent the long
	// time from the gamestate load from messing causing a time skip
	clc.firstDemoFrameSkipped = qfalse;

#if defined RTCW_MP
	if ( clc.waverecording ) {
		CL_WriteWaveClose();
		clc.waverecording = qfalse;
	}
#elif defined RTCW_ET
//	if (clc.waverecording) {
//		CL_WriteWaveClose();
//		clc.waverecording = qfalse;
//	}
#endif // RTCW_XX

}


#if !defined RTCW_ET
/*
====================
CL_StartDemoLoop

Closing the main menu will restart the demo loop
====================
*/
void CL_StartDemoLoop( void ) {
	// start the demo loop again
	Cbuf_AddText( "d1\n" );
	cls.keyCatchers = 0;
}
#endif // RTCW_XX

/*
==================
CL_NextDemo

Called when a demo or cinematic finishes
If the "nextdemo" cvar is set, that command will be issued
==================
*/
void CL_NextDemo( void ) {
	char v[MAX_STRING_CHARS];

	Q_strncpyz( v, Cvar_VariableString( "nextdemo" ), sizeof( v ) );
	v[MAX_STRING_CHARS - 1] = 0;
	Com_DPrintf( "CL_NextDemo: %s\n", v );
	if ( !v[0] ) {
		return;
	}

	Cvar_Set( "nextdemo","" );
	Cbuf_AddText( v );
	Cbuf_AddText( "\n" );
	Cbuf_Execute();
}


//======================================================================

/*
=====================
CL_ShutdownAll
=====================
*/
void CL_ShutdownAll( void ) {

	// clear sounds
	S_DisableSounds();

#if defined RTCW_ET
	// download subsystem
	DL_Shutdown();
#endif // RTCW_XX

	// shutdown CGame
	CL_ShutdownCGame();
	// shutdown UI
	CL_ShutdownUI();

	// shutdown the renderer
	if ( re.Shutdown ) {
		re.Shutdown( qfalse );      // don't destroy window or context
	}

#if defined RTCW_ET
	if ( re.purgeCache ) {
		CL_DoPurgeCache();
	}
#endif // RTCW_XX

	cls.uiStarted = qfalse;
	cls.cgameStarted = qfalse;
	cls.rendererStarted = qfalse;
	cls.soundRegistered = qfalse;

#if defined RTCW_ET
	// Gordon: stop recording on map change etc, demos aren't valid over map changes anyway
	if ( clc.demorecording ) {
		CL_StopRecord_f();
	}

	if ( clc.waverecording ) {
		CL_WavStopRecord_f();
	}
#endif // RTCW_XX

}

/*
=================
CL_FlushMemory

Called by CL_MapLoading, CL_Connect_f, CL_PlayDemo_f, and CL_ParseGamestate the only
ways a client gets into a game
Also called by Com_Error
=================
*/
void CL_FlushMemory( void ) {

	// shutdown all the client stuff
	CL_ShutdownAll();

	// if not running a server clear the whole hunk
	if ( !com_sv_running->integer ) {
		// clear the whole hunk
		Hunk_Clear();

#if defined RTCW_SP
//		// clear collision map data
//		CM_ClearMap();
#else
		// clear collision map data
		CM_ClearMap();
#endif // RTCW_XX

	} else {
		// clear all the client data on the hunk
		Hunk_ClearToMark();
	}

	CL_StartHunkUsers();
}

/*
=====================
CL_MapLoading

A local server is starting to load a map, so update the
screen to let the user know about it, then dump all client
memory on the hunk from cgame, ui, and renderer
=====================
*/
void CL_MapLoading( void ) {
	if ( !com_cl_running->integer ) {
		return;
	}

	Con_Close();
	cls.keyCatchers = 0;

#if defined RTCW_SP
// this was for multi-threaded music
//	S_StartBackgroundTrack( "sound/music/l_briefing_1.wav", "", -2);	// '-2' for 'queue looping track' (QUEUED_PLAY_LOOPED)
#endif // RTCW_XX

	// if we are already connected to the local host, stay connected
	if ( cls.state >= CA_CONNECTED && !Q_stricmp( cls.servername, "localhost" ) ) {
		cls.state = CA_CONNECTED;       // so the connect screen is drawn
		memset( cls.updateInfoString, 0, sizeof( cls.updateInfoString ) );
		memset( clc.serverMessage, 0, sizeof( clc.serverMessage ) );
		memset( &cl.gameState, 0, sizeof( cl.gameState ) );
		clc.lastPacketSentTime = -9999;
		SCR_UpdateScreen();
	} else {
		// clear nextmap so the cinematic shutdown doesn't execute it
		Cvar_Set( "nextmap", "" );
		CL_Disconnect( qtrue );
		Q_strncpyz( cls.servername, "localhost", sizeof( cls.servername ) );
		cls.state = CA_CHALLENGING;     // so the connect screen is drawn
		cls.keyCatchers = 0;
		SCR_UpdateScreen();
		clc.connectTime = -RETRANSMIT_TIMEOUT;
		NET_StringToAdr( cls.servername, &clc.serverAddress );
		// we don't need a challenge on the localhost

		CL_CheckForResend();
	}

#if defined RTCW_SP
	// make sure sound is quiet
	S_FadeAllSounds( 0, 0 );
#endif // RTCW_XX

}

/*
=====================
CL_ClearState

Called before parsing a gamestate
=====================
*/
void CL_ClearState( void ) {

#if defined RTCW_SP
	S_StopAllSounds();
#elif defined RTCW_MP
//	S_StopAllSounds();
#endif // RTCW_XX

#if !defined RTCW_ET
	memset( &cl, 0, sizeof( cl ) );
#else
	Com_Memset( &cl, 0, sizeof( cl ) );
#endif // RTCW_XX

}

#if defined RTCW_ET
/*
=====================
CL_ClearStaticDownload
Clear download information that we keep in cls (disconnected download support)
=====================
*/
void CL_ClearStaticDownload( void ) {
	assert( !cls.bWWWDlDisconnected ); // reset before calling
	cls.downloadRestart = qfalse;
	cls.downloadTempName[0] = '\0';
	cls.downloadName[0] = '\0';
	cls.originalDownloadName[0] = '\0';
}
#endif // RTCW_XX


/*
=====================
CL_Disconnect

Called when a connection, demo, or cinematic is being terminated.
Goes from a connected state to either a menu state or a console state
Sends a disconnect message to the server
This is also called on Com_Error and Com_Quit, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect( qboolean showMainMenu ) {
	if ( !com_cl_running || !com_cl_running->integer ) {
		return;
	}

	// shutting down the client so enter full screen ui mode
	Cvar_Set( "r_uiFullScreen", "1" );

	if ( clc.demorecording ) {
		CL_StopRecord_f();
	}

#if !defined RTCW_ET
	if ( clc.download ) {
		FS_FCloseFile( clc.download );
		clc.download = 0;
	}
	*clc.downloadTempName = *clc.downloadName = 0;
	Cvar_Set( "cl_downloadName", "" );

#if defined RTCW_MP
	autoupdateStarted = qfalse;
	autoupdateFilename[0] = '\0';
#endif // RTCW_XX

#else
	if ( !cls.bWWWDlDisconnected ) {
		if ( clc.download ) {
			FS_FCloseFile( clc.download );
			clc.download = 0;
		}
		*cls.downloadTempName = *cls.downloadName = 0;
		Cvar_Set( "cl_downloadName", "" );

		autoupdateStarted = qfalse;
		autoupdateFilename[0] = '\0';
	}
#endif // RTCW_XX

	if ( clc.demofile ) {
		FS_FCloseFile( clc.demofile );
		clc.demofile = 0;
	}

	if ( uivm && showMainMenu ) {
		VM_Call(uivm, UI_SET_ACTIVE_MENU, rtcw::to_vm_arg(UIMENU_NONE));
	}

	SCR_StopCinematic();

#if !defined RTCW_MP
	S_ClearSoundBuffer( qtrue );  //----(SA)	modified
#else
	S_ClearSoundBuffer();
#endif // RTCW_XX

	// send a disconnect message to the server
	// send it a few times in case one is dropped
	if ( cls.state >= CA_CONNECTED ) {
		CL_AddReliableCommand( "disconnect" );
		CL_WritePacket();
		CL_WritePacket();
		CL_WritePacket();
	}

	CL_ClearState();

	// wipe the client connection

#if !defined RTCW_ET
	memset( &clc, 0, sizeof( clc ) );

	cls.state = CA_DISCONNECTED;
#else
	Com_Memset( &clc, 0, sizeof( clc ) );

	if ( !cls.bWWWDlDisconnected ) {
		CL_ClearStaticDownload();
	}
#endif // RTCW_XX


	// allow cheats locally

#if defined RTCW_SP /* Re-merge */
#ifndef WOLF_SP_DEMO
	// except for demo
	Cvar_Set( "sv_cheats", "1" );
#endif
#else
	Cvar_Set( "sv_cheats", "1" );
#endif // RTCW_XX

	// not connected to a pure server anymore
	cl_connectedToPureServer = qfalse;

#if defined RTCW_ET
	// show_bug.cgi?id=589
	// don't try a restart if uivm is NULL, as we might be in the middle of a restart already
	if ( uivm && cls.state > CA_DISCONNECTED ) {
		// restart the UI
		cls.state = CA_DISCONNECTED;

		// shutdown the UI
		CL_ShutdownUI();

		// init the UI
		CL_InitUI();
	} else {
		cls.state = CA_DISCONNECTED;
	}
#endif // RTCW_XX

}


/*
===================
CL_ForwardCommandToServer

adds the current command line as a clientCommand
things like godmode, noclip, etc, are commands directed to the server,
so when they are typed in at the console, they will need to be forwarded.
===================
*/
void CL_ForwardCommandToServer( const char *string ) {
	const char    *cmd;

	cmd = Cmd_Argv( 0 );

	// ignore key up commands
	if ( cmd[0] == '-' ) {
		return;
	}

	if ( clc.demoplaying || cls.state < CA_CONNECTED || cmd[0] == '+' ) {
		Com_Printf( "Unknown command \"%s\"\n", cmd );
		return;
	}

	if ( Cmd_Argc() > 1 ) {
		CL_AddReliableCommand( string );
	} else {
		CL_AddReliableCommand( cmd );
	}
}

/*
===================
CL_RequestMotd

===================
*/
void CL_RequestMotd( void ) {
	char info[MAX_INFO_STRING];

	if ( !cl_motd->integer ) {
		return;
	}

#if !defined RTCW_ET
	Com_Printf( "Resolving %s\n", UPDATE_SERVER_NAME );
	if ( !NET_StringToAdr( UPDATE_SERVER_NAME, &cls.updateServer  ) ) {
#else
	Com_Printf( "Resolving %s\n", MOTD_SERVER_NAME );
	if ( !NET_StringToAdr( MOTD_SERVER_NAME, &cls.updateServer  ) ) {
#endif // RTCW_XX

		Com_Printf( "Couldn't resolve address\n" );
		return;
	}

#if !defined RTCW_ET
	cls.updateServer.port = rtcw::Endian::be( PORT_UPDATE );
	Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", UPDATE_SERVER_NAME,
#else
	cls.updateServer.port = rtcw::Endian::be ( static_cast<unsigned short>(PORT_MOTD) );
	Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", MOTD_SERVER_NAME,
#endif // RTCW_XX

				cls.updateServer.ip[0], cls.updateServer.ip[1],
				cls.updateServer.ip[2], cls.updateServer.ip[3],
				rtcw::Endian::be( cls.updateServer.port ) );

	info[0] = 0;
	Com_sprintf( cls.updateChallenge, sizeof( cls.updateChallenge ), "%i", rand() );

	Info_SetValueForKey( info, "challenge", cls.updateChallenge );
	Info_SetValueForKey( info, "renderer", cls.glconfig.renderer_string );
	Info_SetValueForKey( info, "version", com_version->string );

	NET_OutOfBandPrint( NS_CLIENT, cls.updateServer, "getmotd \"%s\"\n", info );
}

#if (!defined RTCW_ET) || (defined RTCW_ET && defined AUTHORIZE_SUPPORT)

/*
===================
CL_RequestAuthorization

Authorization server protocol
-----------------------------

All commands are text in Q3 out of band packets (leading 0xff 0xff 0xff 0xff).

Whenever the client tries to get a challenge from the server it wants to
connect to, it also blindly fires off a packet to the authorize server:

getKeyAuthorize <challenge> <cdkey>

cdkey may be "demo"


#OLD The authorize server returns a:
#OLD
#OLD keyAthorize <challenge> <accept | deny>
#OLD
#OLD A client will be accepted if the cdkey is valid and it has not been used by any other IP
#OLD address in the last 15 minutes.


The server sends a:

getIpAuthorize <challenge> <ip>

The authorize server returns a:

ipAuthorize <challenge> <accept | deny | demo | unknown >

A client will be accepted if a valid cdkey was sent by that ip (only) in the last 15 minutes.
If no response is received from the authorize server after two tries, the client will be let
in anyway.
===================
*/
void CL_RequestAuthorization( void ) {
	char nums[64];
	int i, j, l;
	cvar_t  *fs;

	if ( !cls.authorizeServer.port ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !NET_StringToAdr( AUTHORIZE_SERVER_NAME, &cls.authorizeServer  ) ) {
			Com_Printf( "Couldn't resolve address\n" );
			return;
		}

		cls.authorizeServer.port = rtcw::Endian::be( PORT_AUTHORIZE );
		Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
					cls.authorizeServer.ip[0], cls.authorizeServer.ip[1],
					cls.authorizeServer.ip[2], cls.authorizeServer.ip[3],
					rtcw::Endian::be( cls.authorizeServer.port ) );
	}
	if ( cls.authorizeServer.type == NA_BAD ) {
		return;
	}

	if ( Cvar_VariableValue( "fs_restrict" ) ) {

#if !defined RTCW_ET
		Q_strncpyz( nums, "demo", sizeof( nums ) );
#else
		Q_strncpyz( nums, "ettest", sizeof( nums ) );
#endif // RTCW_XX

	} else {
		// only grab the alphanumeric values from the cdkey, to avoid any dashes or spaces
		j = 0;
		l = strlen( cl_cdkey );
		if ( l > 32 ) {
			l = 32;
		}
		for ( i = 0 ; i < l ; i++ ) {
			if ( ( cl_cdkey[i] >= '0' && cl_cdkey[i] <= '9' )
				 || ( cl_cdkey[i] >= 'a' && cl_cdkey[i] <= 'z' )
				 || ( cl_cdkey[i] >= 'A' && cl_cdkey[i] <= 'Z' )
				 ) {
				nums[j] = cl_cdkey[i];
				j++;
			}
		}
		nums[j] = 0;
	}

	fs = Cvar_Get( "cl_anonymous", "0", CVAR_INIT | CVAR_SYSTEMINFO );
	NET_OutOfBandPrint( NS_CLIENT, cls.authorizeServer, va( "getKeyAuthorize %i %s", fs->integer, nums ) );
}
#endif // RTCW_XX

/*
======================================================================

CONSOLE COMMANDS

======================================================================
*/

/*
==================
CL_ForwardToServer_f
==================
*/
void CL_ForwardToServer_f( void ) {
	if ( cls.state != CA_ACTIVE || clc.demoplaying ) {
		Com_Printf( "Not connected to a server.\n" );
		return;
	}

	// don't forward the first argument
	if ( Cmd_Argc() > 1 ) {
		CL_AddReliableCommand( Cmd_Args() );
	}
}

/*
==================
CL_Setenv_f

Mostly for controlling voodoo environment variables
==================
*/
void CL_Setenv_f( void ) {
	int argc = Cmd_Argc();

	if ( argc > 2 ) {
		char buffer[1024];
		int i;

		strcpy( buffer, Cmd_Argv( 1 ) );
		strcat( buffer, "=" );

		for ( i = 2; i < argc; i++ ) {
			strcat( buffer, Cmd_Argv( i ) );
			strcat( buffer, " " );
		}

		Q_putenv( buffer );
	} else if ( argc == 2 ) {
		char *env = getenv( Cmd_Argv( 1 ) );

		if ( env ) {
			Com_Printf( "%s=%s\n", Cmd_Argv( 1 ), env );
		} else {

#if !defined RTCW_ET
			Com_Printf( "%s undefined\n", Cmd_Argv( 1 ), env );
#else
			Com_Printf( "%s undefined\n", Cmd_Argv( 1 ) );
#endif // RTCW_XX

		}
	}
}


/*
==================
CL_Disconnect_f
==================
*/
void CL_Disconnect_f( void ) {
	SCR_StopCinematic();

#if !defined RTCW_MP

#if !defined RTCW_ET
	// RF, make sure loading variables are turned off
#endif // RTCW_XX

	Cvar_Set( "savegame_loading", "0" );
	Cvar_Set( "g_reloading", "0" );
#endif // RTCW_XX

	if ( cls.state != CA_DISCONNECTED && cls.state != CA_CINEMATIC ) {
		Com_Error( ERR_DISCONNECT, "Disconnected from server" );
	}
}


/*
================
CL_Reconnect_f

================
*/
void CL_Reconnect_f( void ) {
	if ( !strlen( cls.servername ) || !strcmp( cls.servername, "localhost" ) ) {
		Com_Printf( "Can't reconnect to localhost.\n" );
		return;
	}
	Cbuf_AddText( va( "connect %s\n", cls.servername ) );
}

/*
================
CL_Connect_f

================
*/
void CL_Connect_f( void ) {
	const char    *server;

#if defined RTCW_ET
	char ip_port[MAX_STRING_CHARS];
#endif // RTCW_XX


	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "usage: connect [server]\n" );
		return;
	}

#if !defined RTCW_SP
	S_StopAllSounds();      // NERVE - SMF
#endif // RTCW_XX

	// starting to load a map so we get out of full screen ui mode
	Cvar_Set( "r_uiFullScreen", "0" );

#if defined RTCW_ET
	Cvar_Set( "ui_connecting", "1" );
#endif // RTCW_XX

	// fire a message off to the motd server
	CL_RequestMotd();

	// clear any previous "server full" type messages
	clc.serverMessage[0] = 0;

	server = Cmd_Argv( 1 );

	if ( com_sv_running->integer && !strcmp( server, "localhost" ) ) {
		// if running a local server, kill it
		SV_Shutdown( "Server quit\n" );
	}

	// make sure a local server is killed
	Cvar_Set( "sv_killserver", "1" );
	SV_Frame( 0 );

	CL_Disconnect( qtrue );
	Con_Close();

#if defined RTCW_SP
//	CL_FlushMemory();	//----(SA)	MEM NOTE: in missionpack, this is moved to CL_DownloadsComplete
#endif // RTCW_XX

	Q_strncpyz( cls.servername, server, sizeof( cls.servername ) );

	if ( !NET_StringToAdr( cls.servername, &clc.serverAddress ) ) {
		Com_Printf( "Bad server address\n" );
		cls.state = CA_DISCONNECTED;

#if defined RTCW_ET
		Cvar_Set( "ui_connecting", "0" );
#endif // RTCW_XX

		return;
	}
	if ( clc.serverAddress.port == 0 ) {
		clc.serverAddress.port = rtcw::Endian::be( PORT_SERVER );
	}

#if !defined RTCW_ET
	Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", cls.servername,
				clc.serverAddress.ip[0], clc.serverAddress.ip[1],
				clc.serverAddress.ip[2], clc.serverAddress.ip[3],
				rtcw::Endian::be( clc.serverAddress.port ) );
#else
	Q_strncpyz( ip_port, NET_AdrToString( clc.serverAddress ), sizeof( ip_port ) );
	Com_Printf( "%s resolved to %s\n", cls.servername, ip_port );
#endif // RTCW_XX

	// if we aren't playing on a lan, we need to authenticate
	// with the cd key
	if ( NET_IsLocalAddress( clc.serverAddress ) ) {
		cls.state = CA_CHALLENGING;
	} else {
		cls.state = CA_CONNECTING;
	}

#if !defined RTCW_SP

#if defined RTCW_ET
	Cvar_Set( "cl_avidemo", "0" );
#endif // RTCW_XX

	// show_bug.cgi?id=507
	// prepare to catch a connection process that would turn bad
	Cvar_Set( "com_errorDiagnoseIP", NET_AdrToString( clc.serverAddress ) );
	// ATVI Wolfenstein Misc #439
	// we need to setup a correct default for this, otherwise the first val we set might reappear
	Cvar_Set( "com_errorMessage", "" );
#endif // RTCW_XX

	cls.keyCatchers = 0;
	clc.connectTime = -99999;   // CL_CheckForResend() will fire immediately
	clc.connectPacketCount = 0;

	// server connection string
	Cvar_Set( "cl_currentServerAddress", server );

#if defined RTCW_ET
	Cvar_Set( "cl_currentServerIP", ip_port );

	// Gordon: um, couldnt this be handled
#endif // RTCW_XX


#if !defined RTCW_SP
	// NERVE - SMF - reset some cvars
	Cvar_Set( "mp_playerType", "0" );
	Cvar_Set( "mp_currentPlayerType", "0" );
	Cvar_Set( "mp_weapon", "0" );
	Cvar_Set( "mp_team", "0" );
	Cvar_Set( "mp_currentTeam", "0" );

	Cvar_Set( "ui_limboOptions", "0" );
	Cvar_Set( "ui_limboPrevOptions", "0" );
	Cvar_Set( "ui_limboObjective", "0" );
	// -NERVE - SMF
#endif // RTCW_XX

}


/*
=====================
CL_Rcon_f

  Send the rest of the command line over as
  an unconnected command.
=====================
*/
void CL_Rcon_f( void ) {
	char message[1024];

#if defined RTCW_SP
	int i;
#endif // RTCW_XX

	netadr_t to;

	if ( !rcon_client_password->string ) {

#if defined RTCW_SP
		Com_Printf( "You must set 'rcon_password' before\n"
#elif defined RTCW_MP
		Com_Printf( "You must set 'rconpassword' before\n"
#else
		Com_Printf( "You must set 'rconPassword' before\n"
#endif // RTCW_XX

					"issuing an rcon command.\n" );
		return;
	}

	message[0] = -1;
	message[1] = -1;
	message[2] = -1;
	message[3] = -1;
	message[4] = 0;

	strcat( message, "rcon " );

	strcat( message, rcon_client_password->string );
	strcat( message, " " );

#if defined RTCW_SP
	for ( i = 1 ; i < Cmd_Argc() ; i++ ) {
		strcat( message, Cmd_Argv( i ) );
		strcat( message, " " );
	}
#else
	// ATVI Wolfenstein Misc #284
	strcat( message, Cmd_Cmd() + 5 );
#endif // RTCW_XX

	if ( cls.state >= CA_CONNECTED ) {
		to = clc.netchan.remoteAddress;
	} else {
		if ( !strlen( rconAddress->string ) ) {
			Com_Printf( "You must either be connected,\n"
						"or set the 'rconAddress' cvar\n"
						"to issue rcon commands\n" );

			return;
		}
		NET_StringToAdr( rconAddress->string, &to );
		if ( to.port == 0 ) {
			to.port = rtcw::Endian::be( PORT_SERVER );
		}
	}

	NET_SendPacket( NS_CLIENT, strlen( message ) + 1, message, to );
}

/*
=================
CL_SendPureChecksums
=================
*/
void CL_SendPureChecksums( void ) {
	const char *pChecksums;
	char cMsg[MAX_INFO_VALUE];
	int i;

	// if we are pure we need to send back a command with our referenced pk3 checksums
	pChecksums = FS_ReferencedPakPureChecksums();

	// "cp"

#if defined RTCW_SP
	// "Yf"
	Com_sprintf( cMsg, sizeof( cMsg ), "Yf " );
#else
	Com_sprintf( cMsg, sizeof( cMsg ), "Va " );
	Q_strcat( cMsg, sizeof( cMsg ), va( "%d ", cl.serverId ) );
#endif // RTCW_XX

	Q_strcat( cMsg, sizeof( cMsg ), pChecksums );
	for ( i = 0; i < 2; i++ ) {

#if defined RTCW_SP
		cMsg[i] += 10;
#else
		cMsg[i] += 13 + ( i * 2 );
#endif // RTCW_XX

	}
	CL_AddReliableCommand( cMsg );
}

/*
=================
CL_ResetPureClientAtServer
=================
*/
void CL_ResetPureClientAtServer( void ) {
	CL_AddReliableCommand( va( "vdr" ) );
}

/*
=================
CL_Vid_Restart_f

Restart the video subsystem

we also have to reload the UI and CGame because the renderer
doesn't know what graphics to reload
=================
*/

extern void Sys_In_Restart_f( void ); // fretn

void CL_Vid_Restart_f( void ) {

#if defined RTCW_SP
	vmCvar_t musicCvar;
#endif // RTCW_XX

	// RF, don't show percent bar, since the memory usage will just sit at the same level anyway

#if !defined RTCW_ET
	Cvar_Set( "com_expectedhunkusage", "-1" );
#else
	com_expectedhunkusage = -1;
#endif // RTCW_XX

	// don't let them loop during the restart
	S_StopAllSounds();
	// shutdown the UI
	CL_ShutdownUI();
	// shutdown the CGame
	CL_ShutdownCGame();
	// shutdown the renderer and clear the renderer interface
	CL_ShutdownRef();
	// client is no longer pure untill new checksums are sent
	CL_ResetPureClientAtServer();
	// clear pak references
	FS_ClearPakReferences( FS_UI_REF | FS_CGAME_REF );
	// reinitialize the filesystem if the game directory or checksum has changed
	FS_ConditionalRestart( clc.checksumFeed );

	S_BeginRegistration();  // all sound handles are now invalid

	cls.rendererStarted = qfalse;
	cls.uiStarted = qfalse;
	cls.cgameStarted = qfalse;
	cls.soundRegistered = qfalse;

#if !defined RTCW_SP
	autoupdateChecked = qfalse;
#endif // RTCW_XX

	// unpause so the cgame definately gets a snapshot and renders a frame
	Cvar_Set( "cl_paused", "0" );

	// if not running a server clear the whole hunk
	if ( !com_sv_running->integer ) {
		// clear the whole hunk
		Hunk_Clear();
	} else {
		// clear all the client data on the hunk
		Hunk_ClearToMark();
	}

	// initialize the renderer interface
	CL_InitRef();

	// startup all the client stuff
	CL_StartHunkUsers();

	Sys_In_Restart_f(); // fretn

	// start the cgame if connected
	if ( cls.state > CA_CONNECTED && cls.state != CA_CINEMATIC ) {
		cls.cgameStarted = qtrue;
		CL_InitCGame();
		// send pure checksums
		CL_SendPureChecksums();
	}

#if defined RTCW_SP
	// start music if there was any

	Cvar_Register( &musicCvar, "s_currentMusic", "", CVAR_ROM );
	if ( strlen( musicCvar.string ) ) {
		S_StartBackgroundTrack( musicCvar.string, musicCvar.string, 1000 );
	}

	// fade up volume
	S_FadeAllSounds( 1, 0 );
#endif // RTCW_XX

}

#if !defined RTCW_SP
/*
=================
CL_UI_Restart_f

Restart the ui subsystem
=================
*/
void CL_UI_Restart_f( void ) {          // NERVE - SMF
	// shutdown the UI
	CL_ShutdownUI();

	autoupdateChecked = qfalse;

	// init the UI
	CL_InitUI();
}
#endif // RTCW_XX

#if defined RTCW_ET
/*
=================
CL_Snd_Reload_f

Reloads sounddata from disk, retains soundhandles.
=================
*/
void CL_Snd_Reload_f( void ) {
	S_Reload();
}
#endif // RTCW_XX


/*
=================
CL_Snd_Restart_f

Restart the sound subsystem
The cgame and game must also be forced to restart because
handles will be invalid
=================
*/
void CL_Snd_Restart_f( void ) {
	S_Shutdown();
	S_Init();

	CL_Vid_Restart_f();
}


/*
==================
CL_PK3List_f
==================
*/
void CL_OpenedPK3List_f( void ) {
	Com_Printf( "Opened PK3 Names: %s\n", FS_LoadedPakNames() );
}

/*
==================
CL_PureList_f
==================
*/
void CL_ReferencedPK3List_f( void ) {
	Com_Printf( "Referenced PK3 Names: %s\n", FS_ReferencedPakNames() );
}

/*
==================
CL_Configstrings_f
==================
*/
void CL_Configstrings_f( void ) {
	int i;
	int ofs;

	if ( cls.state != CA_ACTIVE ) {
		Com_Printf( "Not connected to a server.\n" );
		return;
	}

	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		ofs = cl.gameState.stringOffsets[ i ];
		if ( !ofs ) {
			continue;
		}
		Com_Printf( "%4i: %s\n", i, cl.gameState.stringData + ofs );
	}
}

/*
==============
CL_Clientinfo_f
==============
*/
void CL_Clientinfo_f( void ) {
	Com_Printf( "--------- Client Information ---------\n" );
	Com_Printf( "state: %i\n", cls.state );
	Com_Printf( "Server: %s\n", cls.servername );
	Com_Printf( "User info settings:\n" );
	Info_Print( Cvar_InfoString( CVAR_USERINFO ) );
	Com_Printf( "--------------------------------------\n" );
}

#if defined RTCW_ET
/*
==============
CL_EatMe_f

Eat misc console commands to prevent exploits
==============
*/
void CL_EatMe_f( void ) {
	//do nothing kthxbye
}

/*
==============
CL_WavRecord_f
==============
*/

void CL_WavRecord_f( void ) {
	if ( clc.wavefile ) {
		Com_Printf( "Already recording a wav file\n" );
		return;
	}

	CL_WriteWaveOpen();
}

/*
==============
CL_WavStopRecord_f
==============
*/

void CL_WavStopRecord_f( void ) {
	if ( !clc.wavefile ) {
		Com_Printf( "Not recording a wav file\n" );
		return;
	}

	CL_WriteWaveClose();
	Cvar_Set( "cl_waverecording", "0" );
	Cvar_Set( "cl_wavefilename", "" );
	Cvar_Set( "cl_waveoffset", "0" );
	clc.waverecording = qfalse;
}
#endif // RTCW_XX

//====================================================================

/*
=================
CL_DownloadsComplete

Called when all downloading has been completed
=================
*/
void CL_DownloadsComplete( void ) {

#if !defined RTCW_ET

#if defined RTCW_MP
#ifndef _WIN32
	const char* fs_write_path;
#endif
	char    *fn;

	// DHM - Nerve :: Auto-update (not finished yet)
	if ( autoupdateStarted ) {
		if ( strlen( autoupdateFilename ) > 4 ) {
#ifdef _WIN32
			// win32's Sys_StartProcess prepends the current dir
			fn = va( "%s/%s", FS_ShiftStr( AUTOUPDATE_DIR, AUTOUPDATE_DIR_SHIFT ), autoupdateFilename );
#else
			fs_write_path = Cvar_VariableString( "fs_homepath" );
			fn = FS_BuildOSPath( fs_write_path, FS_ShiftStr( AUTOUPDATE_DIR, AUTOUPDATE_DIR_SHIFT ), autoupdateFilename );
#endif
			Sys_StartProcess( fn, qtrue );
		}

		autoupdateStarted = qfalse;
		CL_Disconnect( qtrue );
		return;
	}
#endif // RTCW_XX

	// if we downloaded files we need to restart the file system
	if ( clc.downloadRestart ) {
		clc.downloadRestart = qfalse;

		FS_Restart( clc.checksumFeed ); // We possibly downloaded a pak, restart the file system to load it

		// inform the server so we get new gamestate info
		CL_AddReliableCommand( "donedl" );

#if defined RTCW_SP
		// by sending the donenl command we request a new gamestate
#elif defined RTCW_MP
		// by sending the donedl command we request a new gamestate
#endif // RTCW_XX

		// so we don't want to load stuff yet
		return;
	}

	// let the client game init and load data
	cls.state = CA_LOADING;

#if defined RTCW_SP
//----(SA)	removed some loading stuff
#endif // RTCW_XX

#else
#ifndef _WIN32
	const char* fs_write_path;
#endif
	char    *fn;

	// DHM - Nerve :: Auto-update (not finished yet)
	if ( autoupdateStarted ) {
		if ( strlen( autoupdateFilename ) > 4 ) {
#ifdef _WIN32
			// win32's Sys_StartProcess prepends the current dir
			fn = va( "%s/%s", FS_ShiftStr( AUTOUPDATE_DIR, AUTOUPDATE_DIR_SHIFT ), autoupdateFilename );
#else
			fs_write_path = Cvar_VariableString( "fs_homepath" );
			fn = FS_BuildOSPath( fs_write_path, FS_ShiftStr( AUTOUPDATE_DIR, AUTOUPDATE_DIR_SHIFT ), autoupdateFilename );
#endif
			// will either exit with a successful process spawn, or will Com_Error ERR_DROP
			// so we need to clear the disconnected download data if needed
			if ( cls.bWWWDlDisconnected ) {
				cls.bWWWDlDisconnected = qfalse;
				CL_ClearStaticDownload();
			}
			Sys_StartProcess( fn, qtrue );
		}

		// NOTE - TTimo: that code is never supposed to be reached?

		autoupdateStarted = qfalse;

		if ( !cls.bWWWDlDisconnected ) {
			CL_Disconnect( qtrue );
		}
		// we can reset that now
		cls.bWWWDlDisconnected = qfalse;
		CL_ClearStaticDownload();

		return;
	}

	// if we downloaded files we need to restart the file system
	if ( cls.downloadRestart ) {
		cls.downloadRestart = qfalse;

		FS_Restart( clc.checksumFeed ); // We possibly downloaded a pak, restart the file system to load it

		if ( !cls.bWWWDlDisconnected ) {
			// inform the server so we get new gamestate info
			CL_AddReliableCommand( "donedl" );
		}
		// we can reset that now
		cls.bWWWDlDisconnected = qfalse;
		CL_ClearStaticDownload();

		// by sending the donedl command we request a new gamestate
		// so we don't want to load stuff yet
		return;
	}

	// TTimo: I wonder if that happens - it should not but I suspect it could happen if a download fails in the middle or is aborted
	assert( !cls.bWWWDlDisconnected );

	// let the client game init and load data
	cls.state = CA_LOADING;

	// Pump the loop, this may change gamestate!
#endif // RTCW_XX

	Com_EventLoop();

	// if the gamestate was changed by calling Com_EventLoop
	// then we loaded everything already and we don't want to do it again.
	if ( cls.state != CA_LOADING ) {
		return;
	}

	// starting to load a map so we get out of full screen ui mode
	Cvar_Set( "r_uiFullScreen", "0" );

	// flush client memory and start loading stuff
	// this will also (re)load the UI
	// if this is a local client then only the client part of the hunk
	// will be cleared, note that this is done after the hunk mark has been set
	CL_FlushMemory();

	// initialize the CGame
	cls.cgameStarted = qtrue;
	CL_InitCGame();

	// set pure checksums
	CL_SendPureChecksums();

	CL_WritePacket();
	CL_WritePacket();
	CL_WritePacket();
}

/*
=================
CL_BeginDownload

Requests a file to download from the server.  Stores it in the current
game directory.
=================
*/
void CL_BeginDownload( const char *localName, const char *remoteName ) {

	Com_DPrintf( "***** CL_BeginDownload *****\n"
				 "Localname: %s\n"
				 "Remotename: %s\n"
				 "****************************\n", localName, remoteName );

#if !defined RTCW_ET
	Q_strncpyz( clc.downloadName, localName, sizeof( clc.downloadName ) );
	Com_sprintf( clc.downloadTempName, sizeof( clc.downloadTempName ), "%s.tmp", localName );
#else
	Q_strncpyz( cls.downloadName, localName, sizeof( cls.downloadName ) );
	Com_sprintf( cls.downloadTempName, sizeof( cls.downloadTempName ), "%s.tmp", localName );
#endif // RTCW_XX

	// Set so UI gets access to it
	Cvar_Set( "cl_downloadName", remoteName );
	Cvar_Set( "cl_downloadSize", "0" );
	Cvar_Set( "cl_downloadCount", "0" );
	Cvar_SetValue( "cl_downloadTime", cls.realtime );

	clc.downloadBlock = 0; // Starting new file
	clc.downloadCount = 0;

	CL_AddReliableCommand( va( "download %s", remoteName ) );
}

/*
=================
CL_NextDownload

A download completed or failed
=================
*/
void CL_NextDownload( void ) {
	char *s;
	char *remoteName, *localName;

	// We are looking to start a download here
	if ( *clc.downloadList ) {
		s = clc.downloadList;

		// format is:
		//  @remotename@localname@remotename@localname, etc.

		if ( *s == '@' ) {
			s++;
		}
		remoteName = s;

		if ( ( s = strchr( s, '@' ) ) == NULL ) {
			CL_DownloadsComplete();
			return;
		}

		*s++ = 0;
		localName = s;
		if ( ( s = strchr( s, '@' ) ) != NULL ) {
			*s++ = 0;
		} else {
			s = localName + strlen( localName ); // point at the nul byte

		}
		CL_BeginDownload( localName, remoteName );

#if !defined RTCW_ET
		clc.downloadRestart = qtrue;
#else
		cls.downloadRestart = qtrue;
#endif // RTCW_XX

		// move over the rest
		memmove( clc.downloadList, s, strlen( s ) + 1 );

		return;
	}

	CL_DownloadsComplete();
}

/*
=================
CL_InitDownloads

After receiving a valid game state, we valid the cgame and local zip files here
and determine if we need to download them
=================
*/
void CL_InitDownloads( void ) {

#if defined RTCW_SP
	if ( cl_allowDownload->integer &&
		 FS_ComparePaks( clc.downloadList, sizeof( clc.downloadList ), qfalse ) ) {

		if ( *clc.downloadList ) {
			// if autodownloading is not enabled on the server
			cls.state = CA_CONNECTED;
			CL_NextDownload();
			return;
		}

	}

#else
#ifndef PRE_RELEASE_DEMO
	char missingfiles[1024];
	char *dir = FS_ShiftStr( AUTOUPDATE_DIR, AUTOUPDATE_DIR_SHIFT );

#if defined RTCW_ET
	// TTimo
	// init some of the www dl data
	clc.bWWWDl = qfalse;
	clc.bWWWDlAborting = qfalse;
	cls.bWWWDlDisconnected = qfalse;
	CL_ClearStaticDownload();
#endif // RTCW_XX

	if ( autoupdateStarted && NET_CompareAdr( cls.autoupdateServer, clc.serverAddress ) ) {
		if ( strlen( cl_updatefiles->string ) > 4 ) {
			Q_strncpyz( autoupdateFilename, cl_updatefiles->string, sizeof( autoupdateFilename ) );
			Q_strncpyz( clc.downloadList, va( "@%s/%s@%s/%s", dir, cl_updatefiles->string, dir, cl_updatefiles->string ), MAX_INFO_STRING );
			cls.state = CA_CONNECTED;
			CL_NextDownload();
			return;
		}
	} else
	{
		// whatever autodownlad configuration, store missing files in a cvar, use later in the ui maybe
		if ( FS_ComparePaks( missingfiles, sizeof( missingfiles ), qfalse ) ) {
			Cvar_Set( "com_missingFiles", missingfiles );
		} else {
			Cvar_Set( "com_missingFiles", "" );
		}

#if defined RTCW_ET
		// reset the redirect checksum tracking
		clc.redirectedList[0] = '\0';
#endif // RTCW_XX

		if ( cl_allowDownload->integer && FS_ComparePaks( clc.downloadList, sizeof( clc.downloadList ), qtrue ) ) {
			// this gets printed to UI, i18n
			Com_Printf( CL_TranslateStringBuf( "Need paks: %s\n" ), clc.downloadList );

			if ( *clc.downloadList ) {
				// if autodownloading is not enabled on the server
				cls.state = CA_CONNECTED;
				CL_NextDownload();
				return;
			}
		}
	}

#endif
#endif // RTCW_XX

	CL_DownloadsComplete();
}

/*
=================
CL_CheckForResend

Resend a connect message if the last one has timed out
=================
*/
void CL_CheckForResend( void ) {

#if defined RTCW_SP
	int port;
#else
	int port, i;
#endif // RTCW_XX

	char info[MAX_INFO_STRING];

#if !defined RTCW_SP
	char data[MAX_INFO_STRING];
#endif // RTCW_XX

#if defined RTCW_ET
	char pkt[1024 + 1] ;    // EVEN BALANCE - T.RAY
	int pktlen ;            // EVEN BALANCE - T.RAY
#endif // RTCW_XX


	// don't send anything if playing back a demo
	if ( clc.demoplaying ) {
		return;
	}

	// resend if we haven't gotten a reply yet
	if ( cls.state != CA_CONNECTING && cls.state != CA_CHALLENGING ) {
		return;
	}

	if ( cls.realtime - clc.connectTime < RETRANSMIT_TIMEOUT ) {
		return;
	}

	clc.connectTime = cls.realtime; // for retransmit requests
	clc.connectPacketCount++;


	switch ( cls.state ) {
	case CA_CONNECTING:
		// requesting a challenge

#if (!defined RTCW_ET) || (defined RTCW_ET && defined AUTHORIZE_SUPPORT)
		if ( !Sys_IsLANAddress( clc.serverAddress ) ) {
			CL_RequestAuthorization();
		}
#endif // RTCW_XX

#if !defined RTCW_ET
		NET_OutOfBandPrint( NS_CLIENT, clc.serverAddress, "getchallenge" );
#else
		// EVEN BALANCE - T.RAY
		strcpy( pkt, "getchallenge" ) ;
		pktlen = strlen( pkt ) ;
		NET_OutOfBandPrint( NS_CLIENT, clc.serverAddress, pkt );
#endif // RTCW_XX

		break;

	case CA_CHALLENGING:
		// sending back the challenge
		port = Cvar_VariableValue( "net_qport" );

		Q_strncpyz( info, Cvar_InfoString( CVAR_USERINFO ), sizeof( info ) );
		Info_SetValueForKey( info, "protocol", va( "%i", PROTOCOL_VERSION ) );
		Info_SetValueForKey( info, "qport", va( "%i", port ) );
		Info_SetValueForKey( info, "challenge", va( "%i", clc.challenge ) );

#if defined RTCW_SP
		NET_OutOfBandPrint( NS_CLIENT, clc.serverAddress, "connect \"%s\"", info );
#else
		strcpy( data, "connect " );

		data[8] = '\"';           // NERVE - SMF - spaces in name bugfix

		for ( i = 0; i < strlen( info ); i++ ) {
			data[9 + i] = info[i];    // + (clc.challenge)&0x3;
		}
		data[9 + i] = '\"';     // NERVE - SMF - spaces in name bugfix
		data[10 + i] = 0;

#if !defined RTCW_ET
		NET_OutOfBandData( NS_CLIENT, clc.serverAddress, reinterpret_cast<byte*> (&data[0]), i + 10 );
#else
		// EVEN BALANCE - T.RAY
		pktlen = i + 10 ;
		memcpy( pkt, &data[0], pktlen ) ;

		NET_OutOfBandData( NS_CLIENT, clc.serverAddress, reinterpret_cast<byte*> (pkt), pktlen );
#endif // RTCW_XX

#endif // RTCW_XX

		// the most current userinfo has been sent, so watch for any
		// newer changes to userinfo variables
		cvar_modifiedFlags &= ~CVAR_USERINFO;
		break;

	default:

#if defined RTCW_SP
		Com_Error( ERR_FATAL, "CL_CHeckForResend: bad cls.state" );
#else
		Com_Error( ERR_FATAL, "CL_CheckForResend: bad cls.state" );
#endif // RTCW_XX

	}
}

/*
===================
CL_DisconnectPacket

Sometimes the server can drop the client and the netchan based
disconnect can be lost.  If the client continues to send packets
to the server, the server will send out of band disconnect packets
to the client so it doesn't have to wait for the full timeout period.
===================
*/
void CL_DisconnectPacket( netadr_t from ) {

#if !defined RTCW_SP
	const char* message;
#endif // RTCW_XX

	if ( cls.state < CA_AUTHORIZING ) {
		return;
	}

	// if not from our server, ignore it
	if ( !NET_CompareAdr( from, clc.netchan.remoteAddress ) ) {
		return;
	}

#if defined RTCW_SP
	// if we have received packets within three seconds, ignore it
	// (it might be a malicious spoof)
#else
	// if we have received packets within three seconds, ignore (it might be a malicious spoof)
	// NOTE TTimo:
	// there used to be a  clc.lastPacketTime = cls.realtime; line in CL_PacketEvent before calling CL_ConnectionLessPacket
	// therefore .. packets never got through this check, clients never disconnected
	// switched the clc.lastPacketTime = cls.realtime to happen after the connectionless packets have been processed
	// you still can't spoof disconnects, cause legal netchan packets will maintain realtime - lastPacketTime below the threshold
#endif // RTCW_XX

	if ( cls.realtime - clc.lastPacketTime < 3000 ) {
		return;
	}

#if defined RTCW_SP
	// drop the connection (FIXME: connection dropped dialog)
	Com_Printf( "Server disconnected for unknown reason\n" );
#elif defined RTCW_MP
	// drop the connection
	message = CL_TranslateStringBuf( "Server disconnected for unknown reason\n" );
	Com_Printf( "%s", message );
	Cvar_Set( "com_errorMessage", message );
#else
	// if we are doing a disconnected download, leave the 'connecting' screen on with the progress information
	if ( !cls.bWWWDlDisconnected ) {
		// drop the connection
		message = "Server disconnected for unknown reason";
		Com_Printf( "%s", message );
		Cvar_Set( "com_errorMessage", message );
#endif // RTCW_XX

	CL_Disconnect( qtrue );

#if defined RTCW_ET
	} else {
		CL_Disconnect( qfalse );
		Cvar_Set( "ui_connecting", "1" );
		Cvar_Set( "ui_dl_running", "1" );
	}
#endif // RTCW_XX

}


/*
===================
CL_MotdPacket

===================
*/
void CL_MotdPacket( netadr_t from ) {
	const char    *challenge;
	const char    *info;

	// if not from our server, ignore it
	if ( !NET_CompareAdr( from, cls.updateServer ) ) {
		return;
	}

	info = Cmd_Argv( 1 );

	// check challenge
	challenge = Info_ValueForKey( info, "challenge" );
	if ( strcmp( challenge, cls.updateChallenge ) ) {
		return;
	}

	challenge = Info_ValueForKey( info, "motd" );

	Q_strncpyz( cls.updateInfoString, info, sizeof( cls.updateInfoString ) );
	Cvar_Set( "cl_motdString", challenge );
}

#if !defined RTCW_SP
/*
===================
CL_PrintPackets
an OOB message from server, with potential markups
print OOB are the only messages we handle markups in
[err_dialog]: used to indicate that the connection should be aborted
  no further information, just do an error diagnostic screen afterwards
[err_prot]: HACK. This is a protocol error. The client uses a custom
  protocol error message (client sided) in the diagnostic window.
  The space for the error message on the connection screen is limited
  to 256 chars.
===================
*/
void CL_PrintPacket( netadr_t from, msg_t *msg ) {
	char *s;
	s = MSG_ReadBigString( msg );
	if ( !Q_stricmpn( s, "[err_dialog]", 12 ) ) {
		Q_strncpyz( clc.serverMessage, s + 12, sizeof( clc.serverMessage ) );

#if !defined RTCW_ET
		Cvar_Set( "com_errorMessage", clc.serverMessage );
	} else if ( !Q_stricmpn( s, "[err_prot]", 10 ) )    {
		Q_strncpyz( clc.serverMessage, s + 10, sizeof( clc.serverMessage ) );
		Cvar_Set( "com_errorMessage", CL_TranslateStringBuf( PROTOCOL_MISMATCH_ERROR_LONG ) );
#else
		// Cvar_Set("com_errorMessage", clc.serverMessage );
		Com_Error( ERR_DROP, "%s", clc.serverMessage );
	} else if ( !Q_stricmpn( s, "[err_prot]", 10 ) )       {
		Q_strncpyz( clc.serverMessage, s + 10, sizeof( clc.serverMessage ) );
		// Cvar_Set("com_errorMessage", CL_TranslateStringBuf( PROTOCOL_MISMATCH_ERROR_LONG ) );
		Com_Error( ERR_DROP, "%s", CL_TranslateStringBuf( PROTOCOL_MISMATCH_ERROR_LONG ) );
	} else if ( !Q_stricmpn( s, "[err_update]", 12 ) )       {
		Q_strncpyz( clc.serverMessage, s + 12, sizeof( clc.serverMessage ) );
		Com_Error( ERR_AUTOUPDATE, "%s", clc.serverMessage );
	} else if ( !Q_stricmpn( s, "ET://", 5 ) )       { // fretn
		Q_strncpyz( clc.serverMessage, s, sizeof( clc.serverMessage ) );
		Cvar_Set( "com_errorMessage", clc.serverMessage );
		Com_Error( ERR_DROP, "%s", clc.serverMessage );
#endif // RTCW_XX

	} else {
		Q_strncpyz( clc.serverMessage, s, sizeof( clc.serverMessage ) );
	}
	Com_Printf( "%s", clc.serverMessage );
}
#endif // RTCW_XX

/*
===================
CL_InitServerInfo
===================
*/
void CL_InitServerInfo( serverInfo_t *server, serverAddress_t *address ) {
	server->adr.type  = NA_IP;
	server->adr.ip[0] = address->ip[0];
	server->adr.ip[1] = address->ip[1];
	server->adr.ip[2] = address->ip[2];
	server->adr.ip[3] = address->ip[3];
	server->adr.port  = address->port;
	server->clients = 0;
	server->hostName[0] = '\0';
	server->mapName[0] = '\0';
	server->maxClients = 0;
	server->maxPing = 0;
	server->minPing = 0;
	server->ping = -1;
	server->game[0] = '\0';
	server->gameType = 0;
	server->netType = 0;
	server->allowAnonymous = 0;
}

#define MAX_SERVERSPERPACKET    256

/*
===================
CL_ServersResponsePacket
===================
*/
void CL_ServersResponsePacket( netadr_t from, msg_t *msg ) {
	int i, count, max, total;
	serverAddress_t addresses[MAX_SERVERSPERPACKET];
	int numservers;
	byte*           buffptr;
	byte*           buffend;

	Com_Printf( "CL_ServersResponsePacket\n" );

	if ( cls.numglobalservers == -1 ) {
		// state to detect lack of servers or lack of response
		cls.numglobalservers = 0;
		cls.numGlobalServerAddresses = 0;
	}

#if !defined RTCW_ET
	if ( cls.nummplayerservers == -1 ) {
		cls.nummplayerservers = 0;
	}
#endif // RTCW_XX

	// parse through server response string
	numservers = 0;
	buffptr    = msg->data;
	buffend    = buffptr + msg->cursize;
	while ( buffptr + 1 < buffend ) {
		// advance to initial token
		do {
			if ( *buffptr++ == '\\' ) {
				break;
			}
		}
		while ( buffptr < buffend );

#if !defined RTCW_SP
		if ( buffptr >= buffend - 6 ) {
			break;
		}
#endif // RTCW_XX

		// parse out ip
		addresses[numservers].ip[0] = *buffptr++;
		addresses[numservers].ip[1] = *buffptr++;
		addresses[numservers].ip[2] = *buffptr++;
		addresses[numservers].ip[3] = *buffptr++;

		// parse out port
		addresses[numservers].port = ( *buffptr++ ) << 8;
		addresses[numservers].port += *buffptr++;
		rtcw::Endian::bei(addresses[numservers].port);

		// syntax check
		if ( *buffptr != '\\' ) {
			break;
		}

		Com_DPrintf( "server: %d ip: %d.%d.%d.%d:%d\n",numservers,
					 addresses[numservers].ip[0],
					 addresses[numservers].ip[1],
					 addresses[numservers].ip[2],
					 addresses[numservers].ip[3],
					 addresses[numservers].port );

		numservers++;
		if ( numservers >= MAX_SERVERSPERPACKET ) {
			break;
		}

		// parse out EOT
		if ( buffptr[1] == 'E' && buffptr[2] == 'O' && buffptr[3] == 'T' ) {
			break;
		}
	}

	if ( cls.masterNum == 0 ) {
		count = cls.numglobalservers;
		max = MAX_GLOBAL_SERVERS;
	} else {

#if !defined RTCW_ET
		count = cls.nummplayerservers;
		max = MAX_OTHER_SERVERS;
#else
		// shut up compiler
		count = 0;
		max = 1;
#endif // RTCW_XX

	}

	for ( i = 0; i < numservers && count < max; i++ ) {
		// build net address

#if !defined RTCW_ET
		serverInfo_t *server = ( cls.masterNum == 0 ) ? &cls.globalServers[count] : &cls.mplayerServers[count];
#else
		//serverInfo_t *server = (cls.masterNum == 0) ? &cls.globalServers[count] : &cls.mplayerServers[count];
		serverInfo_t *server = &cls.globalServers[count];
#endif // RTCW_XX

		CL_InitServerInfo( server, &addresses[i] );
		// advance to next slot
		count++;
	}

#if !defined RTCW_ET
	// if getting the global list
	if ( cls.masterNum == 0 ) {
		if ( cls.numGlobalServerAddresses < MAX_GLOBAL_SERVERS ) {
			// if we couldn't store the servers in the main list anymore
			for (; i < numservers && count >= max; i++ ) {
				serverAddress_t *addr;
				// just store the addresses in an additional list
				addr = &cls.globalServerAddresses[cls.numGlobalServerAddresses++];
				addr->ip[0] = addresses[i].ip[0];
				addr->ip[1] = addresses[i].ip[1];
				addr->ip[2] = addresses[i].ip[2];
				addr->ip[3] = addresses[i].ip[3];
				addr->port  = addresses[i].port;
			}
		}
	}
#else
	// if getting the global list and there are too many servers
	if ( cls.masterNum == 0 && count >= max ) {
		for (; i < numservers && cls.numGlobalServerAddresses < MAX_GLOBAL_SERVERS; i++ ) {
			serverAddress_t *addr;
			// just store the addresses in an additional list
			addr = &cls.globalServerAddresses[cls.numGlobalServerAddresses++];
			addr->ip[0] = addresses[i].ip[0];
			addr->ip[1] = addresses[i].ip[1];
			addr->ip[2] = addresses[i].ip[2];
			addr->ip[3] = addresses[i].ip[3];
			addr->port  = addresses[i].port;
		}
	}
#endif // RTCW_XX

	if ( cls.masterNum == 0 ) {
		cls.numglobalservers = count;
		total = count + cls.numGlobalServerAddresses;
	} else {

#if !defined RTCW_ET
		cls.nummplayerservers = count;
		total = count;
#else
		total = cls.numglobalservers = 0;
#endif // RTCW_XX

	}

	Com_Printf( "%d servers parsed (total %d)\n", numservers, total );
}

/*
=================
CL_ConnectionlessPacket

Responses to broadcasts, etc
=================
*/
void CL_ConnectionlessPacket( netadr_t from, msg_t *msg ) {
	char    *s;
	const char    *c;

	MSG_BeginReadingOOB( msg );
	MSG_ReadLong( msg );    // skip the -1

	s = MSG_ReadStringLine( msg );

	Cmd_TokenizeString( s );

	c = Cmd_Argv( 0 );

	Com_DPrintf( "CL packet %s: %s\n", NET_AdrToString( from ), c );

	// challenge from the server we are connecting to
	if ( !Q_stricmp( c, "challengeResponse" ) ) {
		if ( cls.state != CA_CONNECTING ) {
			Com_Printf( "Unwanted challenge response received.  Ignored.\n" );
		} else {
			// start sending challenge repsonse instead of challenge request packets
			clc.challenge = atoi( Cmd_Argv( 1 ) );

#if !defined RTCW_SP
			if ( Cmd_Argc() > 2 ) {
				clc.onlyVisibleClients = atoi( Cmd_Argv( 2 ) );         // DHM - Nerve
			} else {
				clc.onlyVisibleClients = 0;
			}
#endif // RTCW_XX

			cls.state = CA_CHALLENGING;
			clc.connectPacketCount = 0;
			clc.connectTime = -99999;

			// take this address as the new server address.  This allows
			// a server proxy to hand off connections to multiple servers
			clc.serverAddress = from;

#if !defined RTCW_SP
			Com_DPrintf( "challenge: %d\n", clc.challenge );
#endif // RTCW_XX

		}
		return;
	}

	// server connection
	if ( !Q_stricmp( c, "connectResponse" ) ) {
		if ( cls.state >= CA_CONNECTED ) {
			Com_Printf( "Dup connect received.  Ignored.\n" );
			return;
		}
		if ( cls.state != CA_CHALLENGING ) {
			Com_Printf( "connectResponse packet while not connecting.  Ignored.\n" );
			return;
		}
		if ( !NET_CompareBaseAdr( from, clc.serverAddress ) ) {
			Com_Printf( "connectResponse from a different address.  Ignored.\n" );
			Com_Printf( "%s should have been %s\n", NET_AdrToString( from ),
						NET_AdrToString( clc.serverAddress ) );
			return;
		}

#if !defined RTCW_SP
		// DHM - Nerve :: If we have completed a connection to the Auto-Update server...
		if ( autoupdateChecked && NET_CompareAdr( cls.autoupdateServer, clc.serverAddress ) ) {
			// Mark the client as being in the process of getting an update
			if ( cl_updateavailable->integer ) {
				autoupdateStarted = qtrue;
			}
		}
#endif // RTCW_XX

		Netchan_Setup( NS_CLIENT, &clc.netchan, from, Cvar_VariableValue( "net_qport" ) );
		cls.state = CA_CONNECTED;
		clc.lastPacketSentTime = -9999;     // send first packet immediately
		return;
	}

	// server responding to an info broadcast
	if ( !Q_stricmp( c, "infoResponse" ) ) {
		CL_ServerInfoPacket( from, msg );
		return;
	}

	// server responding to a get playerlist
	if ( !Q_stricmp( c, "statusResponse" ) ) {
		CL_ServerStatusResponse( from, msg );
		return;
	}

	// a disconnect message from the server, which will happen if the server
	// dropped the connection but it is still getting packets from us
	if ( !Q_stricmp( c, "disconnect" ) ) {
		CL_DisconnectPacket( from );
		return;
	}

	// echo request from server
	if ( !Q_stricmp( c, "echo" ) ) {
		NET_OutOfBandPrint( NS_CLIENT, from, "%s", Cmd_Argv( 1 ) );
		return;
	}

	// cd check
	if ( !Q_stricmp( c, "keyAuthorize" ) ) {
		// we don't use these now, so dump them on the floor
		return;
	}

	// global MOTD from id
	if ( !Q_stricmp( c, "motd" ) ) {
		CL_MotdPacket( from );
		return;
	}

	// echo request from server
	if ( !Q_stricmp( c, "print" ) ) {

#if defined RTCW_SP
		s = MSG_ReadString( msg );
		Q_strncpyz( clc.serverMessage, s, sizeof( clc.serverMessage ) );
		Com_Printf( "%s", s );
		return;
	}

	// echo request from server
	if ( !Q_stricmp( c, "getserversResponse\\" ) ) {
#else
		CL_PrintPacket( from, msg );
		return;
	}

	// DHM - Nerve :: Auto-update server response message
	if ( !Q_stricmp( c, "updateResponse" ) ) {
		CL_UpdateInfoPacket( from );
		return;
	}
	// DHM - Nerve

	// NERVE - SMF - bugfix, make this compare first n chars so it doesnt bail if token is parsed incorrectly
	// echo request from server
	if ( !Q_strncmp( c, "getserversResponse", 18 ) ) {
#endif // RTCW_XX

		CL_ServersResponsePacket( from, msg );
		return;
	}

	Com_DPrintf( "Unknown connectionless packet command.\n" );
}


/*
=================
CL_PacketEvent

A packet has arrived from the main event loop
=================
*/
void CL_PacketEvent( netadr_t from, msg_t *msg ) {
	int headerBytes;

#if defined RTCW_SP
	clc.lastPacketTime = cls.realtime;
#endif // RTCW_XX

	if ( msg->cursize >= 4 && *(int *)msg->data == -1 ) {
		CL_ConnectionlessPacket( from, msg );
		return;
	}

#if !defined RTCW_SP
	clc.lastPacketTime = cls.realtime;
#endif // RTCW_XX

	if ( cls.state < CA_CONNECTED ) {
		return;     // can't be a valid sequenced packet
	}

	if ( msg->cursize < 4 ) {
		Com_Printf( "%s: Runt packet\n",NET_AdrToString( from ) );
		return;
	}

	//
	// packet from server
	//
	if ( !NET_CompareAdr( from, clc.netchan.remoteAddress ) ) {
		Com_DPrintf( "%s:sequenced packet without connection\n"
					 ,NET_AdrToString( from ) );
		// FIXME: send a client disconnect?
		return;
	}

	if ( !CL_Netchan_Process( &clc.netchan, msg ) ) {
		return;     // out of order, duplicated, etc
	}

	// the header is different lengths for reliable and unreliable messages
	headerBytes = msg->readcount;

	// track the last message received so it can be returned in
	// client messages, allowing the server to detect a dropped
	// gamestate
	clc.serverMessageSequence = rtcw::Endian::le( *(int *)msg->data );

	clc.lastPacketTime = cls.realtime;
	CL_ParseServerMessage( msg );

	//
	// we don't know if it is ok to save a demo message until
	// after we have parsed the frame
	//
	if ( clc.demorecording && !clc.demowaiting ) {
		CL_WriteDemoMessage( msg, headerBytes );
	}
}

/*
==================
CL_CheckTimeout

==================
*/
void CL_CheckTimeout( void ) {
	//
	// check timeout
	//
	if ( ( !cl_paused->integer || !sv_paused->integer )
		 && cls.state >= CA_CONNECTED && cls.state != CA_CINEMATIC
		 && cls.realtime - clc.lastPacketTime > cl_timeout->value * 1000 ) {
		if ( ++cl.timeoutcount > 5 ) {    // timeoutcount saves debugger

#if defined RTCW_SP
			Com_Printf( "\nServer connection timed out.\n" );
#elif defined RTCW_MP
			Cvar_Set( "com_errorMessage", CL_TranslateStringBuf( "Server connection timed out." ) );
#else
			Cvar_Set( "com_errorMessage", "Server connection timed out." );
#endif // RTCW_XX

			CL_Disconnect( qtrue );
			return;
		}
	} else {
		cl.timeoutcount = 0;
	}
}


//============================================================================

/*
==================
CL_CheckUserinfo

==================
*/
void CL_CheckUserinfo( void ) {
	// don't add reliable commands when not yet connected
	if ( cls.state < CA_CHALLENGING ) {
		return;
	}
	// don't overflow the reliable command buffer when paused
	if ( cl_paused->integer ) {
		return;
	}
	// send a reliable userinfo update if needed
	if ( cvar_modifiedFlags & CVAR_USERINFO ) {
		cvar_modifiedFlags &= ~CVAR_USERINFO;
		CL_AddReliableCommand( va( "userinfo \"%s\"", Cvar_InfoString( CVAR_USERINFO ) ) );
	}
}

#if defined RTCW_ET
/*
==================
CL_WWWDownload
==================
*/
void CL_WWWDownload( void ) {
	char *to_ospath;
	dlStatus_t ret;
	static qboolean bAbort = qfalse;

	if ( clc.bWWWDlAborting ) {
		if ( !bAbort ) {
			Com_DPrintf( "CL_WWWDownload: WWWDlAborting\n" );
			bAbort = qtrue;
		}
		return;
	}
	if ( bAbort ) {
		Com_DPrintf( "CL_WWWDownload: WWWDlAborting done\n" );
		bAbort = qfalse;
	}

	ret = DL_DownloadLoop();

	if ( ret == DL_CONTINUE ) {
		return;
	}

	if ( ret == DL_DONE ) {
		// taken from CL_ParseDownload
		// we work with OS paths
		clc.download = 0;
		to_ospath = FS_BuildOSPath( Cvar_VariableString( "fs_homepath" ), cls.originalDownloadName, "" );
		to_ospath[strlen( to_ospath ) - 1] = '\0';
		if ( rename( cls.downloadTempName, to_ospath ) ) {
			FS_CopyFile( cls.downloadTempName, to_ospath );
			remove( cls.downloadTempName );
		}
		*cls.downloadTempName = *cls.downloadName = 0;
		Cvar_Set( "cl_downloadName", "" );
		if ( cls.bWWWDlDisconnected ) {
			// for an auto-update in disconnected mode, we'll be spawning the setup in CL_DownloadsComplete
			if ( !autoupdateStarted ) {
				// reconnect to the server, which might send us to a new disconnected download
				Cbuf_ExecuteText( EXEC_APPEND, "reconnect\n" );
			}
		} else {
			CL_AddReliableCommand( "wwwdl done" );
			// tracking potential web redirects leading us to wrong checksum - only works in connected mode
			if ( strlen( clc.redirectedList ) + strlen( cls.originalDownloadName ) + 1 >= sizeof( clc.redirectedList ) ) {
				// just to be safe
				Com_Printf( "ERROR: redirectedList overflow (%s)\n", clc.redirectedList );
			} else {
				strcat( clc.redirectedList, "@" );
				strcat( clc.redirectedList, cls.originalDownloadName );
			}
		}
	} else
	{
		if ( cls.bWWWDlDisconnected ) {
			// in a connected download, we'd tell the server about failure and wait for a reply
			// but in this case we can't get anything from server
			// if we just reconnect it's likely we'll get the same disconnected download message, and error out again
			// this may happen for a regular dl or an auto update
			const char *error = va( "Download failure while getting '%s'\n", cls.downloadName ); // get the msg before clearing structs
			cls.bWWWDlDisconnected = qfalse; // need clearing structs before ERR_DROP, or it goes into endless reload
			CL_ClearStaticDownload();
			Com_Error( ERR_DROP, "%s", error );
		} else {
			// see CL_ParseDownload, same abort strategy
			Com_Printf( "Download failure while getting '%s'\n", cls.downloadName );
			CL_AddReliableCommand( "wwwdl fail" );
			clc.bWWWDlAborting = qtrue;
		}
		return;
	}

	clc.bWWWDl = qfalse;
	CL_NextDownload();
}

/*
==================
CL_WWWBadChecksum

FS code calls this when doing FS_ComparePaks
we can detect files that we got from a www dl redirect with a wrong checksum
this indicates that the redirect setup is broken, and next dl attempt should NOT redirect
==================
*/
qboolean CL_WWWBadChecksum( const char *pakname ) {
	if ( strstr( clc.redirectedList, va( "@%s", pakname ) ) ) {
		Com_Printf( "WARNING: file %s obtained through download redirect has wrong checksum\n", pakname );
		Com_Printf( "         this likely means the server configuration is broken\n" );
		if ( strlen( clc.badChecksumList ) + strlen( pakname ) + 1 >= sizeof( clc.badChecksumList ) ) {
			Com_Printf( "ERROR: badChecksumList overflowed (%s)\n", clc.badChecksumList );
			return qfalse;
		}
		strcat( clc.badChecksumList, "@" );
		strcat( clc.badChecksumList, pakname );
		Com_DPrintf( "bad checksums: %s\n", clc.badChecksumList );
		return qtrue;
	}
	return qfalse;
}
#endif // RTCW_XX

/*
==================
CL_Frame

==================
*/
void CL_Frame( int msec ) {

	if ( !com_cl_running->integer ) {
		return;
	}

	if ( cls.cddialog ) {
		// bring up the cd error dialog if needed
		cls.cddialog = qfalse;

#if defined RTCW_SP
		VM_Call(uivm, UI_SET_ACTIVE_MENU, rtcw::to_vm_arg(UIMENU_NEED_CD));
	} else if ( cls.endgamemenu ) {
		cls.endgamemenu = qfalse;
		VM_Call(uivm, UI_SET_ACTIVE_MENU, rtcw::to_vm_arg(UIMENU_ENDGAME));
#else
		VM_Call(uivm, UI_SET_ACTIVE_MENU, rtcw::to_vm_arg(UIMENU_NEED_CD));
#endif // RTCW_XX

	} else if ( cls.state == CA_DISCONNECTED && !( cls.keyCatchers & KEYCATCH_UI )
				&& !com_sv_running->integer ) {
		// if disconnected, bring up the menu
		S_StopAllSounds();
		VM_Call(uivm, UI_SET_ACTIVE_MENU, rtcw::to_vm_arg(UIMENU_MAIN));
	}

	// if recording an avi, lock to a fixed fps
	if ( cl_avidemo->integer && msec ) {
		// save the current screen
		if ( cls.state == CA_ACTIVE || cl_forceavidemo->integer ) {
			Cbuf_ExecuteText( EXEC_NOW, "screenshot silent\n" );
		}
		// fixed time for next frame
		msec = ( 1000 / cl_avidemo->integer ) * com_timescale->value;
		if ( msec == 0 ) {
			msec = 1;
		}
	}

	// save the msec before checking pause
	cls.realFrametime = msec;

	// decide the simulation time
	cls.frametime = msec;

	cls.realtime += cls.frametime;

	if ( cl_timegraph->integer ) {
		SCR_DebugGraph( cls.realFrametime * 0.25, 0 );
	}

	// see if we need to update any userinfo
	CL_CheckUserinfo();

	// if we haven't gotten a packet in a long time,
	// drop the connection
	CL_CheckTimeout();

#if defined RTCW_ET
	// wwwdl download may survive a server disconnect
	if ( ( cls.state == CA_CONNECTED && clc.bWWWDl ) || cls.bWWWDlDisconnected ) {
		CL_WWWDownload();
	}
#endif // RTCW_XX

	// send intentions now
	CL_SendCmd();

	// resend a connection request if necessary
	CL_CheckForResend();

	// decide on the serverTime to render
	CL_SetCGameTime();

	// update the screen
	SCR_UpdateScreen();

#if defined RTCW_SP
	// Ridah, don't update if we're doing a quick reload
//	if (Cvar_VariableIntegerValue("savegame_loading") != 2) {
//		// if waiting at intermission, don't update sound
//		char buf[MAX_QPATH];
//		Cvar_VariableStringBuffer( "g_missionStats", buf, sizeof(buf) );
//		if (strlen(buf) <= 1 ) {
//			// update audio
	S_Update();
//		}
//	}
#else
	// update the sound
	S_Update();
#endif // RTCW_XX

	// advance local effects for next frame
	SCR_RunCinematic();

	Con_RunConsole();

	cls.framecount++;
}


//============================================================================
// Ridah, startup-caching system
typedef struct {
	char name[MAX_QPATH];
	int hits;
	int lastSetIndex;
} cacheItem_t;
typedef enum {
	CACHE_SOUNDS,
	CACHE_MODELS,
	CACHE_IMAGES,

	CACHE_NUMGROUPS
} cacheGroup_t;
static cacheItem_t cacheGroups[CACHE_NUMGROUPS] = {
	{{'s','o','u','n','d',0}, CACHE_SOUNDS},
	{{'m','o','d','e','l',0}, CACHE_MODELS},
	{{'i','m','a','g','e',0}, CACHE_IMAGES},
};
#define MAX_CACHE_ITEMS     4096
#define CACHE_HIT_RATIO     0.75        // if hit on this percentage of maps, it'll get cached

static int cacheIndex;
static cacheItem_t cacheItems[CACHE_NUMGROUPS][MAX_CACHE_ITEMS];

static void CL_Cache_StartGather_f( void ) {
	cacheIndex = 0;
	memset( cacheItems, 0, sizeof( cacheItems ) );

	Cvar_Set( "cl_cacheGathering", "1" );
}

static void CL_Cache_UsedFile_f( void ) {
	char groupStr[MAX_QPATH];
	char itemStr[MAX_QPATH];
	int i,group;
	cacheItem_t *item;

	if ( Cmd_Argc() < 2 ) {
		Com_Error( ERR_DROP, "usedfile without enough parameters\n" );
		return;
	}

	strcpy( groupStr, Cmd_Argv( 1 ) );

	strcpy( itemStr, Cmd_Argv( 2 ) );
	for ( i = 3; i < Cmd_Argc(); i++ ) {
		strcat( itemStr, " " );
		strcat( itemStr, Cmd_Argv( i ) );
	}
	Q_strlwr( itemStr );

	// find the cache group
	for ( i = 0; i < CACHE_NUMGROUPS; i++ ) {
		if ( !Q_strncmp( groupStr, cacheGroups[i].name, MAX_QPATH ) ) {
			break;
		}
	}
	if ( i == CACHE_NUMGROUPS ) {
		Com_Error( ERR_DROP, "usedfile without a valid cache group\n" );
		return;
	}

	// see if it's already there
	group = i;
	for ( i = 0, item = cacheItems[group]; i < MAX_CACHE_ITEMS; i++, item++ ) {
		if ( !item->name[0] ) {
			// didn't find it, so add it here
			Q_strncpyz( item->name, itemStr, MAX_QPATH );
			if ( cacheIndex > 9999 ) { // hack, but yeh
				item->hits = cacheIndex;
			} else {
				item->hits++;
			}
			item->lastSetIndex = cacheIndex;
			break;
		}
		if ( item->name[0] == itemStr[0] && !Q_strncmp( item->name, itemStr, MAX_QPATH ) ) {
			if ( item->lastSetIndex != cacheIndex ) {
				item->hits++;
				item->lastSetIndex = cacheIndex;
			}
			break;
		}
	}
}

static void CL_Cache_SetIndex_f( void ) {
	if ( Cmd_Argc() < 2 ) {
		Com_Error( ERR_DROP, "setindex needs an index\n" );
		return;
	}

	cacheIndex = atoi( Cmd_Argv( 1 ) );
}

static void CL_Cache_MapChange_f( void ) {
	cacheIndex++;
}

static void CL_Cache_EndGather_f( void ) {
	// save the frequently used files to the cache list file
	int i, j, handle, cachePass;
	char filename[MAX_QPATH];

	cachePass = (int)c::floor( (float)cacheIndex * CACHE_HIT_RATIO );

	for ( i = 0; i < CACHE_NUMGROUPS; i++ ) {
		Q_strncpyz( filename, cacheGroups[i].name, MAX_QPATH );
		Q_strcat( filename, MAX_QPATH, ".cache" );

		handle = FS_FOpenFileWrite( filename );

		for ( j = 0; j < MAX_CACHE_ITEMS; j++ ) {
			// if it's a valid filename, and it's been hit enough times, cache it
			if ( cacheItems[i][j].hits >= cachePass && strstr( cacheItems[i][j].name, "/" ) ) {
				FS_Write( cacheItems[i][j].name, strlen( cacheItems[i][j].name ), handle );
				FS_Write( "\n", 1, handle );
			}
		}

		FS_FCloseFile( handle );
	}

	Cvar_Set( "cl_cacheGathering", "0" );
}

// done.
//============================================================================

#if defined RTCW_SP
/*
================
CL_MapRestart_f
================
*/
void CL_MapRestart_f( void ) {
	if ( !com_cl_running ) {
		return;
	}
	if ( !com_cl_running->integer ) {
		return;
	}
	Com_Printf( "This command is no longer functional.\nUse \"loadgame current\" to load the current map." );
}
#endif // RTCW_XX

void CL_SetRecommended_f()
{
	if (Cmd_Argc() > 1)
		Com_SetRecommended(true);
	else
		Com_SetRecommended(false);
}



/*
================
CL_RefPrintf

DLL glue
================
*/

#if defined RTCW_SP
#define MAXPRINTMSG 4096
#endif // RTCW_XX

void QDECL CL_RefPrintf( int print_level, const char *fmt, ... ) {
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start( argptr,fmt );

// BBi See #LBUG0001
//#if defined RTCW_SP
//	vsprintf( msg,fmt,argptr );
//#else
//	Q_vsnprintf( msg, sizeof( msg ), fmt, argptr );
//#endif // RTCW_XX

	msg[MAXPRINTMSG - 1] = '\0';

#if defined RTCW_SP
	vsnprintf (msg, MAXPRINTMSG - 1, fmt, argptr);
#else
	Q_vsnprintf (msg, MAXPRINTMSG - 1, fmt, argptr);
#endif // RTCW_XX
// BBi

	va_end( argptr );

	if ( print_level == PRINT_ALL ) {
		Com_Printf( "%s", msg );
	} else if ( print_level == PRINT_WARNING ) {
		Com_Printf( S_COLOR_YELLOW "%s", msg );       // yellow
	} else if ( print_level == PRINT_DEVELOPER ) {
		Com_DPrintf( S_COLOR_RED "%s", msg );     // red
	}
}



/*
============
CL_ShutdownRef
============
*/
void CL_ShutdownRef( void ) {
	if ( !re.Shutdown ) {
		return;
	}
	re.Shutdown( qtrue );
	memset( &re, 0, sizeof( re ) );
}

/*
============
CL_InitRenderer
============
*/
void CL_InitRenderer( void ) {
	// this sets up the renderer and calls R_Init
	re.BeginRegistration( &cls.glconfig );

	// load character sets

#if defined RTCW_SP
	cls.charSetShader = re.RegisterShader( "gfx/2d/bigchars" );
	cls.whiteShader = re.RegisterShader( "white" );
	cls.consoleShader = re.RegisterShader( "console" );
	cls.consoleShader2 = re.RegisterShader( "console2" );
#else

#if !defined RTCW_ET
	cls.charSetShader = re.RegisterShader( "gfx/2d/hudchars" );
#else
	cls.charSetShader = re.RegisterShader( "gfx/2d/consolechars" );
#endif // RTCW_XX

	cls.whiteShader = re.RegisterShader( "white" );

// JPW NERVE

	cls.consoleShader = re.RegisterShader( "console-16bit" ); // JPW NERVE shader works with 16bit
	cls.consoleShader2 = re.RegisterShader( "console2-16bit" ); // JPW NERVE same
#endif // RTCW_XX

	g_console_field_width = cls.glconfig.vidWidth / SMALLCHAR_WIDTH - 2;
	g_consoleField.widthInChars = g_console_field_width;
}

/*
============================
CL_StartHunkUsers

After the server has cleared the hunk, these will need to be restarted
This is the only place that any of these functions are called from
============================
*/
void CL_StartHunkUsers( void ) {
	if ( !com_cl_running ) {
		return;
	}

	if ( !com_cl_running->integer ) {
		return;
	}

	if ( !cls.rendererStarted ) {
		cls.rendererStarted = qtrue;
		CL_InitRenderer();
	}

	if ( !cls.soundStarted ) {
		cls.soundStarted = qtrue;
		S_Init();
	}

	if ( !cls.soundRegistered ) {
		cls.soundRegistered = qtrue;
		S_BeginRegistration();
	}

	if ( !cls.uiStarted ) {
		cls.uiStarted = qtrue;
		CL_InitUI();
	}
}

#if !defined RTCW_SP
// DHM - Nerve
void CL_CheckAutoUpdate( void ) {

#if !defined RTCW_ET
	int validServerNum = 0;
	int i = 0, rnd = 0;
	netadr_t temp;
	char        *servername;

	if ( !cl_autoupdate->integer ) {
		return;
	}

	// Only check once per session
	if ( autoupdateChecked ) {
		return;
	}

	srand( Com_Milliseconds() );

	// Find out how many update servers have valid DNS listings
	for ( i = 0; i < MAX_AUTOUPDATE_SERVERS; i++ ) {
		if ( NET_StringToAdr( cls.autoupdateServerNames[i], &temp ) ) {
			validServerNum++;
		}
	}

	// Pick a random server
	if ( validServerNum > 1 ) {
		rnd = rand() % validServerNum;
	} else {
		rnd = 0;
	}

	servername = cls.autoupdateServerNames[rnd];

	Com_DPrintf( "Resolving AutoUpdate Server... " );
	if ( !NET_StringToAdr( servername, &cls.autoupdateServer  ) ) {
		Com_DPrintf( "Couldn't resolve first address, trying default..." );

		// Fall back to the first one
		if ( !NET_StringToAdr( cls.autoupdateServerNames[0], &cls.autoupdateServer  ) ) {
			Com_DPrintf( "Failed to resolve any Auto-update servers.\n" );
			autoupdateChecked = qtrue;
			return;
		}
	}
	cls.autoupdateServer.port = rtcw::Endian::be( PORT_SERVER );
	Com_DPrintf( "%i.%i.%i.%i:%i\n", cls.autoupdateServer.ip[0], cls.autoupdateServer.ip[1],
				 cls.autoupdateServer.ip[2], cls.autoupdateServer.ip[3],
				 rtcw::Endian::be( cls.autoupdateServer.port ) );

	// BBi
	//NET_OutOfBandPrint( NS_CLIENT, cls.autoupdateServer, "getUpdateInfo \"%s\" \"%s\"\n", Q3_VERSION, CPUSTRING );
	NET_OutOfBandPrint(
		NS_CLIENT,
		cls.autoupdateServer,
		"getUpdateInfo \"%s\" \"%s\"\n",
		RTCW_VERSION,
		SDL_GetPlatform());
	// BBi

	CL_RequestMotd();

	autoupdateChecked = qtrue;
#else
#ifndef PRE_RELEASE_DEMO

	if ( !cl_autoupdate->integer ) {
		return;
	}

	// Only check once per session
	if ( autoupdateChecked ) {
		return;
	}

	srand( Com_Milliseconds() );

	// Resolve update server
	if ( !NET_StringToAdr( cls.autoupdateServerNames[0], &cls.autoupdateServer  ) ) {
		Com_DPrintf( "Failed to resolve any Auto-update servers.\n" );

		cls.autoUpdateServerChecked[0] = qtrue;

		autoupdateChecked = qtrue;
		return;
	}

	cls.autoupdatServerIndex = 0;

	cls.autoupdatServerFirstIndex = cls.autoupdatServerIndex;

	cls.autoUpdateServerChecked[cls.autoupdatServerIndex] = qtrue;

	cls.autoupdateServer.port = rtcw::Endian::be ( PORT_SERVER );
	Com_DPrintf( "autoupdate server at: %i.%i.%i.%i:%i\n", cls.autoupdateServer.ip[0], cls.autoupdateServer.ip[1],
				 cls.autoupdateServer.ip[2], cls.autoupdateServer.ip[3],
				 rtcw::Endian::be ( cls.autoupdateServer.port ) );

	// BBi
	//NET_OutOfBandPrint( NS_CLIENT, cls.autoupdateServer, "getUpdateInfo \"%s\" \"%s\"\n", Q3_VERSION, CPUSTRING );
	NET_OutOfBandPrint(
		NS_CLIENT,
		cls.autoupdateServer,
		"getUpdateInfo \"%s\" \"%s\"\n",
		RTCW_VERSION,
		SDL_GetPlatform());
	// BBi

#endif // !PRE_RELEASE_DEMO

	CL_RequestMotd();

	autoupdateChecked = qtrue;
#endif // RTCW_XX

}

#if defined RTCW_ET
qboolean CL_NextUpdateServer( void ) {
	char        *servername;

#ifdef PRE_RELEASE_DEMO
	return qfalse;
#endif // PRE_RELEASE_DEMO

	if ( !cl_autoupdate->integer ) {
		return qfalse;
	}

#ifdef _DEBUG
	Com_Printf( S_COLOR_MAGENTA "Autoupdate hardcoded OFF in debug build\n" );
	return qfalse;
#else // _DEBUG
	while ( cls.autoUpdateServerChecked[cls.autoupdatServerFirstIndex] ) {
		cls.autoupdatServerIndex++;

		if ( cls.autoupdatServerIndex > MAX_AUTOUPDATE_SERVERS ) {
			cls.autoupdatServerIndex = 0;
		}

		if ( cls.autoupdatServerIndex == cls.autoupdatServerFirstIndex ) {
			// went through all of them already
			return qfalse;
		}
	}

	servername = cls.autoupdateServerNames[cls.autoupdatServerIndex];

	Com_DPrintf( "Resolving AutoUpdate Server... " );
	if ( !NET_StringToAdr( servername, &cls.autoupdateServer  ) ) {
		Com_DPrintf( "Couldn't resolve address, trying next one..." );

		cls.autoUpdateServerChecked[cls.autoupdatServerIndex] = qtrue;

		return CL_NextUpdateServer();
	}

	cls.autoUpdateServerChecked[cls.autoupdatServerIndex] = qtrue;

	cls.autoupdateServer.port = rtcw::Endian::be ( PORT_SERVER );
	Com_DPrintf( "%i.%i.%i.%i:%i\n", cls.autoupdateServer.ip[0], cls.autoupdateServer.ip[1],
				 cls.autoupdateServer.ip[2], cls.autoupdateServer.ip[3],
				 rtcw::Endian::be ( cls.autoupdateServer.port ) );

	return qtrue;
#endif // _DEBUG
}
#endif // RTCW_XX

void CL_GetAutoUpdate( void ) {

	// Don't try and get an update if we haven't checked for one
	if ( !autoupdateChecked ) {
		return;
	}

	// Make sure there's a valid update file to request
	if ( strlen( cl_updatefiles->string ) < 5 ) {
		return;
	}

	Com_DPrintf( "Connecting to auto-update server...\n" );

	S_StopAllSounds();      // NERVE - SMF

	// starting to load a map so we get out of full screen ui mode
	Cvar_Set( "r_uiFullScreen", "0" );

#if defined RTCW_ET
	// toggle on all the download related cvars
	Cvar_Set( "cl_allowDownload", "1" ); // general flag
	Cvar_Set( "cl_wwwDownload", "1" ); // ftp/http support
#endif // RTCW_XX

	// clear any previous "server full" type messages
	clc.serverMessage[0] = 0;

	if ( com_sv_running->integer ) {
		// if running a local server, kill it
		SV_Shutdown( "Server quit\n" );
	}

	// make sure a local server is killed
	Cvar_Set( "sv_killserver", "1" );
	SV_Frame( 0 );

	CL_Disconnect( qtrue );
	Con_Close();

	Q_strncpyz( cls.servername, "Auto-Updater", sizeof( cls.servername ) );

	if ( cls.autoupdateServer.type == NA_BAD ) {
		Com_Printf( "Bad server address\n" );
		cls.state = CA_DISCONNECTED;

#if defined RTCW_ET
		Cvar_Set( "ui_connecting", "0" );
#endif // RTCW_XX

		return;
	}

	// Copy auto-update server address to Server connect address
	memcpy( &clc.serverAddress, &cls.autoupdateServer, sizeof( netadr_t ) );

	Com_DPrintf( "%s resolved to %i.%i.%i.%i:%i\n", cls.servername,
				 clc.serverAddress.ip[0], clc.serverAddress.ip[1],
				 clc.serverAddress.ip[2], clc.serverAddress.ip[3],
				 rtcw::Endian::be( clc.serverAddress.port ) );

	cls.state = CA_CONNECTING;

	cls.keyCatchers = 0;
	clc.connectTime = -99999;   // CL_CheckForResend() will fire immediately
	clc.connectPacketCount = 0;

	// server connection string
	Cvar_Set( "cl_currentServerAddress", "Auto-Updater" );
}
// DHM - Nerve

/*
============
CL_RefMalloc
============
*/
#ifdef ZONE_DEBUG
void *CL_RefMallocDebug( int size, char *label, char *file, int line ) {
	return Z_TagMallocDebug( size, TAG_RENDERER, label, file, line );
}
#else
void *CL_RefMalloc( int size ) {
	return Z_TagMalloc( size, TAG_RENDERER );
}
#endif

/*
============
CL_RefTagFree
============
*/
void CL_RefTagFree( void ) {
	Z_FreeTags( TAG_RENDERER );
	return;
}
#endif // RTCW_XX


int CL_ScaledMilliseconds( void ) {
	return Sys_Milliseconds() * com_timescale->value;
}

/*
============
CL_InitRef
============
*/
void CL_InitRef( void ) {
	refimport_t ri;
	refexport_t *ret;

	Com_Printf( "----- Initializing Renderer ----\n" );

	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.Cmd_Argc = Cmd_Argc;
	ri.Cmd_Argv = Cmd_Argv;
	ri.Cmd_ExecuteText = Cbuf_ExecuteText;
	ri.Printf = CL_RefPrintf;
	ri.Error = Com_Error;
	ri.Milliseconds = CL_ScaledMilliseconds;

#if !defined RTCW_SP
#ifdef ZONE_DEBUG
	ri.Z_MallocDebug = CL_RefMallocDebug;
#else
	ri.Z_Malloc = CL_RefMalloc;
#endif
	ri.Free = Z_Free;
	ri.Tag_Free = CL_RefTagFree;
#endif // RTCW_XX

	ri.Hunk_Clear = Hunk_ClearToMark;
#ifdef HUNK_DEBUG
	ri.Hunk_AllocDebug = Hunk_AllocDebug;
#else
	ri.Hunk_Alloc = Hunk_Alloc;
#endif
	ri.Hunk_AllocateTempMemory = Hunk_AllocateTempMemory;
	ri.Hunk_FreeTempMemory = Hunk_FreeTempMemory;
	ri.CM_DrawDebugSurface = CM_DrawDebugSurface;
	ri.FS_ReadFile = FS_ReadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_WriteFile = FS_WriteFile;
	ri.FS_FreeFileList = FS_FreeFileList;
	ri.FS_ListFiles = FS_ListFiles;
	ri.FS_FileIsInPAK = FS_FileIsInPAK;
	ri.FS_FileExists = FS_FileExists;
	ri.Cvar_Get = Cvar_Get;
	ri.Cvar_Set = Cvar_Set;

	// cinematic stuff

	ri.CIN_UploadCinematic = CIN_UploadCinematic;
	ri.CIN_PlayCinematic = CIN_PlayCinematic;
	ri.CIN_RunCinematic = CIN_RunCinematic;

	ret = GetRefAPI( REF_API_VERSION, &ri );

#if !defined RTCW_ET
#if 0 // MrE defined __USEA3D && defined __A3D_GEOM
	hA3Dg_ExportRenderGeom( ret );
#endif
#endif // RTCW_XX

	Com_Printf( "-------------------------------\n" );

	if ( !ret ) {
		Com_Error( ERR_FATAL, "Couldn't initialize refresh" );
	}

	re = *ret;

	// unpause so the cgame definately gets a snapshot and renders a frame
	Cvar_Set( "cl_paused", "0" );
}

// RF, trap manual client damage commands so users can't issue them manually
void CL_ClientDamageCommand( void ) {
	// do nothing
}

// NERVE - SMF

#if defined RTCW_SP
void CL_startMultiplayer_f( void ) {
	Sys_StartProcess ("rtcw_mp_x86", qtrue);
}
// -NERVE - SMF

//----(SA) added
/*
==============
CL_ShellExecute_URL_f
Format:
  shellExecute "open" <url> <doExit>

TTimo
  show_bug.cgi?id=447
  only supporting "open" syntax for URL openings, others are not portable or need to be added on a case-by-case basis
  the shellExecute syntax as been kept to remain compatible with win32 SP demo pk3, but this thing only does open URL

==============
*/

void CL_ShellExecute_URL_f( void ) {
	qboolean doexit;

	Com_DPrintf( "CL_ShellExecute_URL_f\n" );

	if ( Q_stricmp( Cmd_Argv( 1 ),"open" ) ) {
		Com_DPrintf( "invalid CL_ShellExecute_URL_f syntax (shellExecute \"open\" <url> <doExit>)\n" );
		return;
	}

	if ( Cmd_Argc() < 4 ) {
		doexit = qtrue;
	} else {
		doexit = (qboolean)( atoi( Cmd_Argv( 3 ) ) );
	}

	Sys_OpenURL( Cmd_Argv( 2 ),doexit );
}
//----(SA) end
#else

void CL_startSingleplayer_f ()
{
	Sys_StartProcess ("rtcw_sp_x86", qtrue);
}
// BBi

// NERVE - SMF
#if !defined RTCW_ET
void CL_buyNow_f( void ) {
	Sys_OpenURL( "http://www.activision.com/games/wolfenstein/purchase.html", qtrue );
}

// NERVE - SMF
void CL_singlePlayLink_f( void ) {
	Sys_OpenURL( "http://www.activision.com/games/wolfenstein/home.html", qtrue );
}
#else
// fretn unused
#if 0
void CL_buyNow_f( void ) {
	Sys_OpenURL( "http://www.activision.com/games/wolfenstein/purchase.html", qtrue );
}

// NERVE - SMF
void CL_singlePlayLink_f( void ) {
	Sys_OpenURL( "http://www.activision.com/games/wolfenstein/home.html", qtrue );
}
#endif
#endif // RTCW_XX

void CL_SaveTranslations_f( void ) {
	CL_SaveTransTable( "scripts/translation.cfg", qfalse );
}

void CL_SaveNewTranslations_f( void ) {
	char fileName[512];

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "usage: SaveNewTranslations <filename>\n" );
		return;
	}

	strcpy( fileName, va( "translations/%s.cfg", Cmd_Argv( 1 ) ) );

	CL_SaveTransTable( fileName, qtrue );
}

void CL_LoadTranslations_f( void ) {
	CL_ReloadTranslation();
}
// -NERVE - SMF
#endif // RTCW_XX

//===========================================================================================

/*
====================
CL_Init
====================
*/
void CL_Init( void ) {
	Com_Printf( "----- Client Initialization -----\n" );

	Con_Init();

	CL_ClearState();

	cls.state = CA_DISCONNECTED;    // no longer CA_UNINITIALIZED

	cls.realtime = 0;

	CL_InitInput();

	//
	// register our variables
	//
	cl_noprint = Cvar_Get( "cl_noprint", "0", 0 );
	cl_motd = Cvar_Get( "cl_motd", "1", 0 );

#if !defined RTCW_SP
	cl_autoupdate = Cvar_Get( "cl_autoupdate", "1", CVAR_ARCHIVE );
#endif // RTCW_XX

	cl_timeout = Cvar_Get( "cl_timeout", "200", 0 );

#if !defined RTCW_SP
	cl_wavefilerecord = Cvar_Get( "cl_wavefilerecord", "0", CVAR_TEMP );
#endif // RTCW_XX

	cl_timeNudge = Cvar_Get( "cl_timeNudge", "0", CVAR_TEMP );
	cl_shownet = Cvar_Get( "cl_shownet", "0", CVAR_TEMP );

#if !defined RTCW_SP
	cl_shownuments = Cvar_Get( "cl_shownuments", "0", CVAR_TEMP );
	cl_visibleClients = Cvar_Get( "cl_visibleClients", "0", CVAR_TEMP );
	cl_showServerCommands = Cvar_Get( "cl_showServerCommands", "0", 0 );
#endif // RTCW_XX

	cl_showSend = Cvar_Get( "cl_showSend", "0", CVAR_TEMP );
	cl_showTimeDelta = Cvar_Get( "cl_showTimeDelta", "0", CVAR_TEMP );
	cl_freezeDemo = Cvar_Get( "cl_freezeDemo", "0", CVAR_TEMP );
	rcon_client_password = Cvar_Get( "rconPassword", "", CVAR_TEMP );
	cl_activeAction = Cvar_Get( "activeAction", "", CVAR_TEMP );

#if defined RTCW_ET
	cl_autorecord = Cvar_Get( "cl_autorecord", "0", CVAR_TEMP );
#endif // RTCW_XX

	cl_timedemo = Cvar_Get( "timedemo", "0", 0 );
	cl_avidemo = Cvar_Get( "cl_avidemo", "0", 0 );
	cl_forceavidemo = Cvar_Get( "cl_forceavidemo", "0", 0 );

	rconAddress = Cvar_Get( "rconAddress", "", 0 );

	cl_yawspeed = Cvar_Get( "cl_yawspeed", "140", CVAR_ARCHIVE );
	cl_pitchspeed = Cvar_Get( "cl_pitchspeed", "140", CVAR_ARCHIVE );
	cl_anglespeedkey = Cvar_Get( "cl_anglespeedkey", "1.5", 0 );

	cl_maxpackets = Cvar_Get( "cl_maxpackets", "30", CVAR_ARCHIVE );
	cl_packetdup = Cvar_Get( "cl_packetdup", "1", CVAR_ARCHIVE );

	cl_run = Cvar_Get( "cl_run", "1", CVAR_ARCHIVE );
	cl_sensitivity = Cvar_Get( "sensitivity", "5", CVAR_ARCHIVE );
	cl_mouseAccel = Cvar_Get( "cl_mouseAccel", "0", CVAR_ARCHIVE );
	cl_freelook = Cvar_Get( "cl_freelook", "1", CVAR_ARCHIVE );

	cl_showMouseRate = Cvar_Get( "cl_showmouserate", "0", 0 );

#if !defined RTCW_ET
	cl_allowDownload = Cvar_Get( "cl_allowDownload", "0", CVAR_ARCHIVE );
#else
	cl_allowDownload = Cvar_Get( "cl_allowDownload", "1", CVAR_ARCHIVE );
#endif // RTCW_XX

#if defined RTCW_ET
	cl_wwwDownload = Cvar_Get( "cl_wwwDownload", "1", CVAR_USERINFO | CVAR_ARCHIVE );

	cl_profile = Cvar_Get( "cl_profile", "", CVAR_ROM );
	cl_defaultProfile = Cvar_Get( "cl_defaultProfile", "", CVAR_ROM );
#endif // RTCW_XX

	// init autoswitch so the ui will have it correctly even
	// if the cgame hasn't been started

#if defined RTCW_SP
	Cvar_Get( "cg_autoswitch", "2", CVAR_ARCHIVE );
#else
	// -NERVE - SMF - disabled autoswitch by default
	Cvar_Get( "cg_autoswitch", "0", CVAR_ARCHIVE );
#endif // RTCW_XX

	// Rafael - particle switch
	Cvar_Get( "cg_wolfparticles", "1", CVAR_ARCHIVE );
	// done

	cl_conXOffset = Cvar_Get( "cl_conXOffset", "0", 0 );
	cl_inGameVideo = Cvar_Get( "r_inGameVideo", "1", CVAR_ARCHIVE );

	cl_serverStatusResendTime = Cvar_Get( "cl_serverStatusResendTime", "750", 0 );

	// RF
	cl_recoilPitch = Cvar_Get( "cg_recoilPitch", "0", CVAR_ROM );

#if !defined RTCW_SP
	cl_bypassMouseInput = Cvar_Get( "cl_bypassMouseInput", "0", 0 ); //CVAR_ROM );			// NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
	cl_doubletapdelay = Cvar_Get( "cl_doubletapdelay", "350", CVAR_ARCHIVE ); // Arnout: double tap
#endif // RTCW_XX

	m_pitch = Cvar_Get( "m_pitch", "0.022", CVAR_ARCHIVE );
	m_yaw = Cvar_Get( "m_yaw", "0.022", CVAR_ARCHIVE );
	m_forward = Cvar_Get( "m_forward", "0.25", CVAR_ARCHIVE );
	m_side = Cvar_Get( "m_side", "0.25", CVAR_ARCHIVE );
	m_filter = Cvar_Get( "m_filter", "0", CVAR_ARCHIVE );

	cl_motdString = Cvar_Get( "cl_motdString", "", CVAR_ROM );

#if defined RTCW_ET
	//bani - make these cvars visible to cgame
	cl_demorecording = Cvar_Get( "cl_demorecording", "0", CVAR_ROM );
	cl_demofilename = Cvar_Get( "cl_demofilename", "", CVAR_ROM );
	cl_demooffset = Cvar_Get( "cl_demooffset", "0", CVAR_ROM );
	cl_waverecording = Cvar_Get( "cl_waverecording", "0", CVAR_ROM );
	cl_wavefilename = Cvar_Get( "cl_wavefilename", "", CVAR_ROM );
	cl_waveoffset = Cvar_Get( "cl_waveoffset", "0", CVAR_ROM );

	//bani
	cl_packetloss = Cvar_Get( "cl_packetloss", "0", CVAR_CHEAT );
	cl_packetdelay = Cvar_Get( "cl_packetdelay", "0", CVAR_CHEAT );
#endif // RTCW_XX

	Cvar_Get( "cl_maxPing", "800", CVAR_ARCHIVE );

#if !defined RTCW_SP
	// NERVE - SMF
	Cvar_Get( "cg_drawCompass", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_drawNotifyText", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_quickMessageAlt", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_popupLimboMenu", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_descriptiveText", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_drawTeamOverlay", "2", CVAR_ARCHIVE );

#if !defined RTCW_ET
	Cvar_Get( "cg_uselessNostalgia", "0", CVAR_ARCHIVE ); // JPW NERVE
#else
//	Cvar_Get( "cg_uselessNostalgia", "0", CVAR_ARCHIVE ); // JPW NERVE
#endif // RTCW_XX

	Cvar_Get( "cg_drawGun", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_cursorHints", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_voiceSpriteTime", "6000", CVAR_ARCHIVE );

#if !defined RTCW_ET
	Cvar_Get( "cg_teamChatsOnly", "0", CVAR_ARCHIVE );
	Cvar_Get( "cg_noVoiceChats", "0", CVAR_ARCHIVE );
	Cvar_Get( "cg_noVoiceText", "0", CVAR_ARCHIVE );
#else
//	Cvar_Get( "cg_teamChatsOnly", "0", CVAR_ARCHIVE );
//	Cvar_Get( "cg_noVoiceChats", "0", CVAR_ARCHIVE );
//	Cvar_Get( "cg_noVoiceText", "0", CVAR_ARCHIVE );
#endif // RTCW_XX

	Cvar_Get( "cg_crosshairSize", "48", CVAR_ARCHIVE );
	Cvar_Get( "cg_drawCrosshair", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_zoomDefaultSniper", "20", CVAR_ARCHIVE );
	Cvar_Get( "cg_zoomstepsniper", "2", CVAR_ARCHIVE );

#if !defined RTCW_ET
	Cvar_Get( "mp_playerType", "0", 0 );
	Cvar_Get( "mp_currentPlayerType", "0", 0 );
	Cvar_Get( "mp_weapon", "0", 0 );
	Cvar_Get( "mp_team", "0", 0 );
	Cvar_Get( "mp_currentTeam", "0", 0 );
#else
//	Cvar_Get( "mp_playerType", "0", 0 );
//	Cvar_Get( "mp_currentPlayerType", "0", 0 );
//	Cvar_Get( "mp_weapon", "0", 0 );
//	Cvar_Get( "mp_team", "0", 0 );
//	Cvar_Get( "mp_currentTeam", "0", 0 );
#endif // RTCW_XX

	// -NERVE - SMF
#endif // RTCW_XX

	// userinfo

#if !defined RTCW_ET
#if defined RTCW_SP
	Cvar_Get( "name", "Player", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "rate", "3000", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "snaps", "20", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "model", "bj2", CVAR_USERINFO | CVAR_ARCHIVE ); // temp until we have an skeletal american model
#elif defined RTCW_MP
	Cvar_Get( "name", "WolfPlayer", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "rate", "5000", CVAR_USERINFO | CVAR_ARCHIVE );     // NERVE - SMF - changed from 3000
	Cvar_Get( "snaps", "20", CVAR_USERINFO | CVAR_ARCHIVE );
//	Cvar_Get ("model", "american", CVAR_USERINFO | CVAR_ARCHIVE );	// temp until we have an skeletal american model
	Cvar_Get( "model", "multi", CVAR_USERINFO | CVAR_ARCHIVE );
#endif // RTCW_XX

	Cvar_Get( "head", "default", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "color", "4", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "handicap", "100", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "sex", "male", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "cl_anonymous", "0", CVAR_USERINFO | CVAR_ARCHIVE );

	Cvar_Get( "password", "", CVAR_USERINFO );
	Cvar_Get( "cg_predictItems", "1", CVAR_USERINFO | CVAR_ARCHIVE );

//----(SA) added
	Cvar_Get( "cg_autoactivate", "1", CVAR_USERINFO | CVAR_ARCHIVE );

#if defined RTCW_SP
	Cvar_Get( "cg_emptyswitch", "0", CVAR_USERINFO | CVAR_ARCHIVE );
#endif // RTCW_XX
#else
	Cvar_Get( "name", "ETPlayer", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "rate", "5000", CVAR_USERINFO | CVAR_ARCHIVE );     // NERVE - SMF - changed from 3000
	Cvar_Get( "snaps", "20", CVAR_USERINFO | CVAR_ARCHIVE );
//	Cvar_Get ("model", "american", CVAR_USERINFO | CVAR_ARCHIVE );	// temp until we have an skeletal american model
//	Arnout - no need // Cvar_Get ("model", "multi", CVAR_USERINFO | CVAR_ARCHIVE );
//	Arnout - no need // Cvar_Get ("head", "default", CVAR_USERINFO | CVAR_ARCHIVE );
//	Arnout - no need // Cvar_Get ("color", "4", CVAR_USERINFO | CVAR_ARCHIVE );
//	Arnout - no need // Cvar_Get ("handicap", "0", CVAR_USERINFO | CVAR_ARCHIVE );
//	Cvar_Get ("sex", "male", CVAR_USERINFO | CVAR_ARCHIVE );
	Cvar_Get( "cl_anonymous", "0", CVAR_USERINFO | CVAR_ARCHIVE );

	Cvar_Get( "password", "", CVAR_USERINFO );
	Cvar_Get( "cg_predictItems", "1", CVAR_ARCHIVE );

//----(SA) added
	Cvar_Get( "cg_autoactivate", "1", CVAR_ARCHIVE );
#endif // RTCW_XX

//----(SA) end

	// cgame might not be initialized before menu is used
	Cvar_Get( "cg_viewsize", "100", CVAR_ARCHIVE );

#if defined RTCW_MP
	Cvar_Get( "cg_autoReload", "1", CVAR_ARCHIVE | CVAR_USERINFO );
#elif defined RTCW_ET
	Cvar_Get( "cg_autoReload", "1", CVAR_ARCHIVE );
#endif // RTCW_XX

	cl_missionStats = Cvar_Get( "g_missionStats", "0", CVAR_ROM );
	cl_waitForFire = Cvar_Get( "cl_waitForFire", "0", CVAR_ROM );

	// NERVE - SMF - localization
	cl_language = Cvar_Get( "cl_language", "0", CVAR_ARCHIVE );
	cl_debugTranslation = Cvar_Get( "cl_debugTranslation", "0", 0 );
	// -NERVE - SMF

#if !defined RTCW_SP
	// DHM - Nerve :: Auto-update
	cl_updateavailable = Cvar_Get( "cl_updateavailable", "0", CVAR_ROM );
	cl_updatefiles = Cvar_Get( "cl_updatefiles", "", CVAR_ROM );

	Q_strncpyz( cls.autoupdateServerNames[0], AUTOUPDATE_SERVER1_NAME, MAX_QPATH );
	Q_strncpyz( cls.autoupdateServerNames[1], AUTOUPDATE_SERVER2_NAME, MAX_QPATH );
	Q_strncpyz( cls.autoupdateServerNames[2], AUTOUPDATE_SERVER3_NAME, MAX_QPATH );
	Q_strncpyz( cls.autoupdateServerNames[3], AUTOUPDATE_SERVER4_NAME, MAX_QPATH );
	Q_strncpyz( cls.autoupdateServerNames[4], AUTOUPDATE_SERVER5_NAME, MAX_QPATH );
	// DHM - Nerve
#endif // RTCW_XX
	//
	// register our commands
	//
	Cmd_AddCommand( "cmd", CL_ForwardToServer_f );
	Cmd_AddCommand( "configstrings", CL_Configstrings_f );
	Cmd_AddCommand( "clientinfo", CL_Clientinfo_f );

#if defined RTCW_ET
	Cmd_AddCommand( "snd_reload", CL_Snd_Reload_f );
#endif // RTCW_XX

	Cmd_AddCommand( "snd_restart", CL_Snd_Restart_f );
	Cmd_AddCommand( "vid_restart", CL_Vid_Restart_f );

#if !defined RTCW_SP
	Cmd_AddCommand( "ui_restart", CL_UI_Restart_f );          // NERVE - SMF
#endif // RTCW_XX

	Cmd_AddCommand( "disconnect", CL_Disconnect_f );
	Cmd_AddCommand( "record", CL_Record_f );
	Cmd_AddCommand( "demo", CL_PlayDemo_f );
	Cmd_AddCommand( "cinematic", CL_PlayCinematic_f );
	Cmd_AddCommand( "stoprecord", CL_StopRecord_f );
	Cmd_AddCommand( "connect", CL_Connect_f );
	Cmd_AddCommand( "reconnect", CL_Reconnect_f );
	Cmd_AddCommand( "localservers", CL_LocalServers_f );
	Cmd_AddCommand( "globalservers", CL_GlobalServers_f );
	Cmd_AddCommand( "rcon", CL_Rcon_f );
	Cmd_AddCommand( "setenv", CL_Setenv_f );
	Cmd_AddCommand( "ping", CL_Ping_f );
	Cmd_AddCommand( "serverstatus", CL_ServerStatus_f );
	Cmd_AddCommand( "showip", CL_ShowIP_f );
	Cmd_AddCommand( "fs_openedList", CL_OpenedPK3List_f );
	Cmd_AddCommand( "fs_referencedList", CL_ReferencedPK3List_f );

	// Ridah, startup-caching system
	Cmd_AddCommand( "cache_startgather", CL_Cache_StartGather_f );
	Cmd_AddCommand( "cache_usedfile", CL_Cache_UsedFile_f );
	Cmd_AddCommand( "cache_setindex", CL_Cache_SetIndex_f );
	Cmd_AddCommand( "cache_mapchange", CL_Cache_MapChange_f );
	Cmd_AddCommand( "cache_endgather", CL_Cache_EndGather_f );

	Cmd_AddCommand( "updatehunkusage", CL_UpdateLevelHunkUsage );
	Cmd_AddCommand( "updatescreen", SCR_UpdateScreen );
	// done.

#if defined RTCW_SP
	// RF, add this command so clients can't bind a key to send client damage commands to the server
	Cmd_AddCommand( "cld", CL_ClientDamageCommand );

	Cmd_AddCommand( "startMultiplayer", CL_startMultiplayer_f );        // NERVE - SMF

	// TTimo
	// show_bug.cgi?id=447
	Cmd_AddCommand( "shellExecute", CL_ShellExecute_URL_f );
	//Cmd_AddCommand ( "shellExecute", CL_ShellExecute_f );	//----(SA) added (mainly for opening web pages from the menu)

	// RF, prevent users from issuing a map_restart manually
	Cmd_AddCommand( "map_restart", CL_MapRestart_f );
#else
	Cmd_AddCommand( "SaveTranslations", CL_SaveTranslations_f );     // NERVE - SMF - localization
	Cmd_AddCommand( "SaveNewTranslations", CL_SaveNewTranslations_f );   // NERVE - SMF - localization
	Cmd_AddCommand( "LoadTranslations", CL_LoadTranslations_f );     // NERVE - SMF - localization

	// NERVE - SMF - don't do this in multiplayer
	// RF, add this command so clients can't bind a key to send client damage commands to the server
//	Cmd_AddCommand ("cld", CL_ClientDamageCommand );

#if !defined RTCW_ET
	Cmd_AddCommand( "startSingleplayer", CL_startSingleplayer_f );      // NERVE - SMF
	Cmd_AddCommand( "buyNow", CL_buyNow_f );                            // NERVE - SMF
	Cmd_AddCommand( "singlePlayLink", CL_singlePlayLink_f );            // NERVE - SMF
#else
//	Cmd_AddCommand ( "startSingleplayer", CL_startSingleplayer_f );		// NERVE - SMF
//	fretn - unused
//	Cmd_AddCommand ( "buyNow", CL_buyNow_f );							// NERVE - SMF
//	Cmd_AddCommand ( "singlePlayLink", CL_singlePlayLink_f );			// NERVE - SMF
#endif // RTCW_XX

#endif // RTCW_XX

	Cmd_AddCommand( "setRecommended", CL_SetRecommended_f );

#if defined RTCW_ET
	//bani - we eat these commands to prevent exploits
	Cmd_AddCommand( "userinfo", CL_EatMe_f );

	Cmd_AddCommand( "wav_record", CL_WavRecord_f );
	Cmd_AddCommand( "wav_stoprecord", CL_WavStopRecord_f );
#endif // RTCW_XX

	CL_InitRef();

	SCR_Init();

	Cbuf_Execute();

	Cvar_Set( "cl_running", "1" );

#if !defined RTCW_SP
	// DHM - Nerve
	autoupdateChecked = qfalse;
	autoupdateStarted = qfalse;

	CL_InitTranslation();   // NERVE - SMF - localization
#endif // RTCW_XX

	Com_Printf( "----- Client Initialization Complete -----\n" );
}


/*
===============
CL_Shutdown

===============
*/
void CL_Shutdown( void ) {
	static qboolean recursive = qfalse;

	Com_Printf( "----- CL_Shutdown -----\n" );

	if ( recursive ) {
		printf( "recursive shutdown\n" );
		return;
	}
	recursive = qtrue;

#if defined RTCW_ET
	if ( clc.waverecording ) { // fretn - write wav header when we quit
		CL_WavStopRecord_f();
	}
#endif // RTCW_XX

	CL_Disconnect( qtrue );

	S_Shutdown();

#if defined RTCW_ET
	DL_Shutdown();
#endif // RTCW_XX

	CL_ShutdownRef();

	CL_ShutdownUI();

	Cmd_RemoveCommand( "cmd" );
	Cmd_RemoveCommand( "configstrings" );
	Cmd_RemoveCommand( "userinfo" );

#if defined RTCW_ET
	Cmd_RemoveCommand( "snd_reload" );
#endif // RTCW_XX

	Cmd_RemoveCommand( "snd_restart" );
	Cmd_RemoveCommand( "vid_restart" );
	Cmd_RemoveCommand( "disconnect" );
	Cmd_RemoveCommand( "record" );
	Cmd_RemoveCommand( "demo" );
	Cmd_RemoveCommand( "cinematic" );
	Cmd_RemoveCommand( "stoprecord" );
	Cmd_RemoveCommand( "connect" );
	Cmd_RemoveCommand( "localservers" );
	Cmd_RemoveCommand( "globalservers" );
	Cmd_RemoveCommand( "rcon" );
	Cmd_RemoveCommand( "setenv" );
	Cmd_RemoveCommand( "ping" );
	Cmd_RemoveCommand( "serverstatus" );
	Cmd_RemoveCommand( "showip" );
	Cmd_RemoveCommand( "model" );

	// Ridah, startup-caching system
	Cmd_RemoveCommand( "cache_startgather" );
	Cmd_RemoveCommand( "cache_usedfile" );
	Cmd_RemoveCommand( "cache_setindex" );
	Cmd_RemoveCommand( "cache_mapchange" );
	Cmd_RemoveCommand( "cache_endgather" );

	Cmd_RemoveCommand( "updatehunkusage" );

#if defined RTCW_ET
	Cmd_RemoveCommand( "wav_record" );
	Cmd_RemoveCommand( "wav_stoprecord" );
#endif // RTCW_XX

	// done.

	Cvar_Set( "cl_running", "0" );

	recursive = qfalse;

	memset( &cls, 0, sizeof( cls ) );

	Com_Printf( "-----------------------\n" );
}


static void CL_SetServerInfo( serverInfo_t *server, const char *info, int ping ) {
	if ( server ) {
		if ( info ) {
			server->clients = atoi( Info_ValueForKey( info, "clients" ) );
			Q_strncpyz( server->hostName,Info_ValueForKey( info, "hostname" ), MAX_NAME_LENGTH );

#if defined RTCW_ET
			server->load = atoi( Info_ValueForKey( info, "serverload" ) );
#endif // RTCW_XX

			Q_strncpyz( server->mapName, Info_ValueForKey( info, "mapname" ), MAX_NAME_LENGTH );
			server->maxClients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
			Q_strncpyz( server->game,Info_ValueForKey( info, "game" ), MAX_NAME_LENGTH );
			server->gameType = atoi( Info_ValueForKey( info, "gametype" ) );
			server->netType = atoi( Info_ValueForKey( info, "nettype" ) );
			server->minPing = atoi( Info_ValueForKey( info, "minping" ) );
			server->maxPing = atoi( Info_ValueForKey( info, "maxping" ) );
			server->allowAnonymous = atoi( Info_ValueForKey( info, "sv_allowAnonymous" ) );

#if !defined RTCW_SP
			server->friendlyFire = atoi( Info_ValueForKey( info, "friendlyFire" ) );         // NERVE - SMF
			server->maxlives = atoi( Info_ValueForKey( info, "maxlives" ) );                 // NERVE - SMF

#if !defined RTCW_ET
			server->tourney = atoi( Info_ValueForKey( info, "tourney" ) );                       // NERVE - SMF
#else
			server->needpass = atoi( Info_ValueForKey( info, "needpass" ) );                 // NERVE - SMF
#endif // RTCW_XX

			server->punkbuster = atoi( Info_ValueForKey( info, "punkbuster" ) );             // DHM - Nerve
			Q_strncpyz( server->gameName, Info_ValueForKey( info, "gamename" ), MAX_NAME_LENGTH );   // Arnout
			server->antilag = atoi( Info_ValueForKey( info, "g_antilag" ) );
#endif // RTCW_XX

#if defined RTCW_ET
			server->weaprestrict = atoi( Info_ValueForKey( info, "weaprestrict" ) );
			server->balancedteams = atoi( Info_ValueForKey( info, "balancedteams" ) );
#endif // RTCW_XX

		}
		server->ping = ping;
	}
}

static void CL_SetServerInfoByAddress( netadr_t from, const char *info, int ping ) {
	int i;

	for ( i = 0; i < MAX_OTHER_SERVERS; i++ ) {
		if ( NET_CompareAdr( from, cls.localServers[i].adr ) ) {
			CL_SetServerInfo( &cls.localServers[i], info, ping );
		}
	}

#if !defined RTCW_ET
	for ( i = 0; i < MAX_OTHER_SERVERS; i++ ) {
		if ( NET_CompareAdr( from, cls.mplayerServers[i].adr ) ) {
			CL_SetServerInfo( &cls.mplayerServers[i], info, ping );
		}
	}
#endif // RTCW_XX

	for ( i = 0; i < MAX_GLOBAL_SERVERS; i++ ) {
		if ( NET_CompareAdr( from, cls.globalServers[i].adr ) ) {
			CL_SetServerInfo( &cls.globalServers[i], info, ping );
		}
	}

	for ( i = 0; i < MAX_OTHER_SERVERS; i++ ) {
		if ( NET_CompareAdr( from, cls.favoriteServers[i].adr ) ) {
			CL_SetServerInfo( &cls.favoriteServers[i], info, ping );
		}
	}

}

/*
===================
CL_ServerInfoPacket
===================
*/
void CL_ServerInfoPacket( netadr_t from, msg_t *msg ) {
	int i, type;
	char info[MAX_INFO_STRING];
	const char*   str;
	char    *infoString;
	int prot;

#if !defined RTCW_SP
	const char    *gameName;
#endif // RTCW_XX

#if defined RTCW_ET
	int debug_protocol;
	int protocol = PROTOCOL_VERSION;

	debug_protocol = Cvar_VariableIntegerValue( "debug_protocol" );
	if ( debug_protocol ) {
		protocol = debug_protocol;
	}
#endif // RTCW_XX

	infoString = MSG_ReadString( msg );

#if defined RTCW_ET
	// if this isn't the correct protocol version, ignore it
	prot = atoi( Info_ValueForKey( infoString, "protocol" ) );
	if ( prot != protocol ) {
		Com_DPrintf( "Different protocol info packet: %s\n", infoString );
		return;
	}
#endif // RTCW_XX

#if !defined RTCW_SP
	// Arnout: if this isn't the correct game, ignore it
	gameName = Info_ValueForKey( infoString, "gamename" );
	if ( !gameName[0] || Q_stricmp( gameName, GAMENAME_STRING ) ) {
		Com_DPrintf( "Different game info packet: %s\n", infoString );
		return;
	}
#endif // RTCW_XX

#if !defined RTCW_ET
	// if this isn't the correct protocol version, ignore it
	prot = atoi( Info_ValueForKey( infoString, "protocol" ) );
	if ( prot != PROTOCOL_VERSION ) {
		Com_DPrintf( "Different protocol info packet: %s\n", infoString );
#endif // RTCW_XX

#if defined RTCW_SP
//		return;
#elif defined RTCW_MP
		return;
#endif // RTCW_XX

#if !defined RTCW_ET
	}
#endif // RTCW_XX

	// iterate servers waiting for ping response
	for ( i = 0; i < MAX_PINGREQUESTS; i++ )
	{
		if ( cl_pinglist[i].adr.port && !cl_pinglist[i].time && NET_CompareAdr( from, cl_pinglist[i].adr ) ) {
			// calc ping time
			cl_pinglist[i].time = cls.realtime - cl_pinglist[i].start + 1;
			Com_DPrintf( "ping time %dms from %s\n", cl_pinglist[i].time, NET_AdrToString( from ) );

			// save of info
			Q_strncpyz( cl_pinglist[i].info, infoString, sizeof( cl_pinglist[i].info ) );

			// tack on the net type
			// NOTE: make sure these types are in sync with the netnames strings in the UI
			switch ( from.type )
			{
			case NA_BROADCAST:
			case NA_IP:
				str = "udp";
				type = 1;
				break;

			case NA_IPX:
			case NA_BROADCAST_IPX:
				str = "ipx";
				type = 2;
				break;

			default:
				str = "???";
				type = 0;
				break;
			}
			Info_SetValueForKey( cl_pinglist[i].info, "nettype", va( "%d", type ) );
			CL_SetServerInfoByAddress( from, infoString, cl_pinglist[i].time );

			return;
		}
	}

	// if not just sent a local broadcast or pinging local servers
	if ( cls.pingUpdateSource != AS_LOCAL ) {
		return;
	}

	for ( i = 0 ; i < MAX_OTHER_SERVERS ; i++ ) {
		// empty slot
		if ( cls.localServers[i].adr.port == 0 ) {
			break;
		}

		// avoid duplicate
		if ( NET_CompareAdr( from, cls.localServers[i].adr ) ) {
			return;
		}
	}

	if ( i == MAX_OTHER_SERVERS ) {
		Com_DPrintf( "MAX_OTHER_SERVERS hit, dropping infoResponse\n" );
		return;
	}

	// add this to the list
	cls.numlocalservers = i + 1;
	cls.localServers[i].adr = from;
	cls.localServers[i].clients = 0;
	cls.localServers[i].hostName[0] = '\0';

#if defined RTCW_ET
	cls.localServers[i].load = -1;
#endif // RTCW_XX

	cls.localServers[i].mapName[0] = '\0';
	cls.localServers[i].maxClients = 0;
	cls.localServers[i].maxPing = 0;
	cls.localServers[i].minPing = 0;
	cls.localServers[i].ping = -1;
	cls.localServers[i].game[0] = '\0';
	cls.localServers[i].gameType = 0;
	cls.localServers[i].netType = from.type;
	cls.localServers[i].allowAnonymous = 0;

#if defined RTCW_MP
	cls.localServers[i].friendlyFire = 0;           // NERVE - SMF
	cls.localServers[i].maxlives = 0;               // NERVE - SMF
	cls.localServers[i].tourney = 0;                // NERVE - SMF
	cls.localServers[i].punkbuster = 0;             // DHM - Nerve
	cls.localServers[i].gameName[0] = '\0';           // Arnout
#endif // RTCW_XX

#if defined RTCW_ET
	cls.localServers[i].friendlyFire = 0;           // NERVE - SMF
	cls.localServers[i].maxlives = 0;               // NERVE - SMF
	cls.localServers[i].needpass = 0;
	cls.localServers[i].punkbuster = 0;             // DHM - Nerve
	cls.localServers[i].antilag = 0;
	cls.localServers[i].weaprestrict = 0;
	cls.localServers[i].balancedteams = 0;
	cls.localServers[i].gameName[0] = '\0';           // Arnout
#endif // RTCW_XX

	Q_strncpyz( info, MSG_ReadString( msg ), MAX_INFO_STRING );
	if ( strlen( info ) ) {
		if ( info[strlen( info ) - 1] != '\n' ) {
			strncat( info, "\n", sizeof( info ) - 1 );
		}
		Com_Printf( "%s: %s", NET_AdrToString( from ), info );
	}
}

#if !defined RTCW_SP
/*
===================
CL_UpdateInfoPacket
===================
*/
void CL_UpdateInfoPacket( netadr_t from ) {

	if ( cls.autoupdateServer.type == NA_BAD ) {
		Com_DPrintf( "CL_UpdateInfoPacket:  Auto-Updater has bad address\n" );
		return;
	}

	Com_DPrintf( "Auto-Updater resolved to %i.%i.%i.%i:%i\n",
				 cls.autoupdateServer.ip[0], cls.autoupdateServer.ip[1],
				 cls.autoupdateServer.ip[2], cls.autoupdateServer.ip[3],
				 rtcw::Endian::be ( cls.autoupdateServer.port ) );

	if ( !NET_CompareAdr( from, cls.autoupdateServer ) ) {
		Com_DPrintf( "CL_UpdateInfoPacket:  Received packet from %i.%i.%i.%i:%i\n",
					 from.ip[0], from.ip[1], from.ip[2], from.ip[3],
					 rtcw::Endian::be ( from.port ) );
		return;
	}

	Cvar_Set( "cl_updateavailable", Cmd_Argv( 1 ) );

	if ( !Q_stricmp( cl_updateavailable->string, "1" ) ) {
		Cvar_Set( "cl_updatefiles", Cmd_Argv( 2 ) );
		VM_Call(uivm, UI_SET_ACTIVE_MENU, rtcw::to_vm_arg(UIMENU_WM_AUTOUPDATE));
	}
}
// DHM - Nerve
#endif // RTCW_XX

/*
===================
CL_GetServerStatus
===================
*/
serverStatus_t *CL_GetServerStatus( netadr_t from ) {
	serverStatus_t *serverStatus;
	int i, oldest, oldestTime;

	serverStatus = NULL;
	for ( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ ) {
		if ( NET_CompareAdr( from, cl_serverStatusList[i].address ) ) {
			return &cl_serverStatusList[i];
		}
	}
	for ( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ ) {
		if ( cl_serverStatusList[i].retrieved ) {
			return &cl_serverStatusList[i];
		}
	}
	oldest = -1;
	oldestTime = 0;
	for ( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ ) {
		if ( oldest == -1 || cl_serverStatusList[i].startTime < oldestTime ) {
			oldest = i;
			oldestTime = cl_serverStatusList[i].startTime;
		}
	}
	if ( oldest != -1 ) {
		return &cl_serverStatusList[oldest];
	}
	serverStatusCount++;
	return &cl_serverStatusList[serverStatusCount & ( MAX_SERVERSTATUSREQUESTS - 1 )];
}

/*
===================
CL_ServerStatus
===================
*/
int CL_ServerStatus( char *serverAddress, char *serverStatusString, int maxLen ) {
	int i;
	netadr_t to;
	serverStatus_t *serverStatus;

	// if no server address then reset all server status requests
	if ( !serverAddress ) {
		for ( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ ) {
			cl_serverStatusList[i].address.port = 0;
			cl_serverStatusList[i].retrieved = qtrue;
		}
		return qfalse;
	}
	// get the address
	if ( !NET_StringToAdr( serverAddress, &to ) ) {
		return qfalse;
	}
	serverStatus = CL_GetServerStatus( to );
	// if no server status string then reset the server status request for this address
	if ( !serverStatusString ) {
		serverStatus->retrieved = qtrue;
		return qfalse;
	}

	// if this server status request has the same address
	if ( NET_CompareAdr( to, serverStatus->address ) ) {
		// if we recieved an response for this server status request
		if ( !serverStatus->pending ) {
			Q_strncpyz( serverStatusString, serverStatus->string, maxLen );
			serverStatus->retrieved = qtrue;
			serverStatus->startTime = 0;
			return qtrue;
		}
		// resend the request regularly
		else if ( serverStatus->startTime < Sys_Milliseconds() - cl_serverStatusResendTime->integer ) {
			serverStatus->print = qfalse;
			serverStatus->pending = qtrue;
			serverStatus->retrieved = qfalse;
			serverStatus->time = 0;
			serverStatus->startTime = Sys_Milliseconds();
			NET_OutOfBandPrint( NS_CLIENT, to, "getstatus" );
			return qfalse;
		}
	}
	// if retrieved
	else if ( serverStatus->retrieved ) {
		serverStatus->address = to;
		serverStatus->print = qfalse;
		serverStatus->pending = qtrue;
		serverStatus->retrieved = qfalse;
		serverStatus->startTime = Sys_Milliseconds();
		serverStatus->time = 0;
		NET_OutOfBandPrint( NS_CLIENT, to, "getstatus" );
		return qfalse;
	}
	return qfalse;
}

/*
===================
CL_ServerStatusResponse
===================
*/
void CL_ServerStatusResponse( netadr_t from, msg_t *msg ) {
	const char    *s;
	char info[MAX_INFO_STRING];
	int i, l, score, ping;
	int len;
	serverStatus_t *serverStatus;

	serverStatus = NULL;
	for ( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ ) {
		if ( NET_CompareAdr( from, cl_serverStatusList[i].address ) ) {
			serverStatus = &cl_serverStatusList[i];
			break;
		}
	}
	// if we didn't request this server status
	if ( !serverStatus ) {
		return;
	}

	s = MSG_ReadStringLine( msg );

	len = 0;
	Com_sprintf( &serverStatus->string[len], sizeof( serverStatus->string ) - len, "%s", s );

	if ( serverStatus->print ) {
		Com_Printf( "Server settings:\n" );
		// print cvars
		while ( *s ) {
			for ( i = 0; i < 2 && *s; i++ ) {
				if ( *s == '\\' ) {
					s++;
				}
				l = 0;
				while ( *s ) {
					info[l++] = *s;
					if ( l >= MAX_INFO_STRING - 1 ) {
						break;
					}
					s++;
					if ( *s == '\\' ) {
						break;
					}
				}
				info[l] = '\0';
				if ( i ) {
					Com_Printf( "%s\n", info );
				} else {
					Com_Printf( "%-24s", info );
				}
			}
		}
	}

	len = strlen( serverStatus->string );
	Com_sprintf( &serverStatus->string[len], sizeof( serverStatus->string ) - len, "\\" );

	if ( serverStatus->print ) {
		Com_Printf( "\nPlayers:\n" );
		Com_Printf( "num: score: ping: name:\n" );
	}
	for ( i = 0, s = MSG_ReadStringLine( msg ); *s; s = MSG_ReadStringLine( msg ), i++ ) {

		len = strlen( serverStatus->string );
		Com_sprintf( &serverStatus->string[len], sizeof( serverStatus->string ) - len, "\\%s", s );

		if ( serverStatus->print ) {
			score = ping = 0;
			sscanf( s, "%d %d", &score, &ping );
			s = strchr( s, ' ' );
			if ( s ) {
				s = strchr( s + 1, ' ' );
			}
			if ( s ) {
				s++;
			} else {
				s = "unknown";
			}
			Com_Printf( "%-2d   %-3d    %-3d   %s\n", i, score, ping, s );
		}
	}
	len = strlen( serverStatus->string );
	Com_sprintf( &serverStatus->string[len], sizeof( serverStatus->string ) - len, "\\" );

	serverStatus->time = Sys_Milliseconds();
	serverStatus->address = from;
	serverStatus->pending = qfalse;
	if ( serverStatus->print ) {
		serverStatus->retrieved = qtrue;
	}
}

/*
==================
CL_LocalServers_f
==================
*/
void CL_LocalServers_f( void ) {
	const char        *message;
	int i, j;
	netadr_t to;

	Com_Printf( "Scanning for servers on the local network...\n" );

	// reset the list, waiting for response
	cls.numlocalservers = 0;
	cls.pingUpdateSource = AS_LOCAL;

	for ( i = 0; i < MAX_OTHER_SERVERS; i++ ) {
		qboolean b = cls.localServers[i].visible;
		Com_Memset( &cls.localServers[i], 0, sizeof( cls.localServers[i] ) );
		cls.localServers[i].visible = b;
	}
	Com_Memset( &to, 0, sizeof( to ) );

	// The 'xxx' in the message is a challenge that will be echoed back
	// by the server.  We don't care about that here, but master servers
	// can use that to prevent spoofed server responses from invalid ip
	message = "\377\377\377\377getinfo xxx";

	// send each message twice in case one is dropped
	for ( i = 0 ; i < 2 ; i++ ) {
		// send a broadcast packet on each server port
		// we support multiple server ports so a single machine
		// can nicely run multiple servers
		for ( j = 0 ; j < NUM_SERVER_PORTS ; j++ ) {
			to.port = rtcw::Endian::be( (short)( PORT_SERVER + j ) );

			to.type = NA_BROADCAST;
			NET_SendPacket( NS_CLIENT, strlen( message ), message, to );

			to.type = NA_BROADCAST_IPX;
			NET_SendPacket( NS_CLIENT, strlen( message ), message, to );
		}
	}
}

/*
==================
CL_GlobalServers_f
==================
*/
void CL_GlobalServers_f( void ) {
	netadr_t to;
	int i;
	int count;
	char        *buffptr;
	char command[1024];

	if ( Cmd_Argc() < 3 ) {
		Com_Printf( "usage: globalservers <master# 0-1> <protocol> [keywords]\n" );
		return;
	}

	cls.masterNum = atoi( Cmd_Argv( 1 ) );

	Com_Printf( "Requesting servers from the master...\n" );

	// reset the list, waiting for response
	// -1 is used to distinguish a "no response"

#if !defined RTCW_ET
	if ( cls.masterNum == 1 ) {
		NET_StringToAdr( "master.quake3world.com", &to );
		cls.nummplayerservers = -1;
		cls.pingUpdateSource = AS_MPLAYER;
	} else {
#else
	if ( cls.masterNum == 0 ) {
#endif // RTCW_XX

		NET_StringToAdr( MASTER_SERVER_NAME, &to );
		cls.numglobalservers = -1;
		cls.pingUpdateSource = AS_GLOBAL;
	}
	to.type = NA_IP;
	to.port = rtcw::Endian::be( PORT_MASTER );

	sprintf( command, "getservers %s", Cmd_Argv( 2 ) );

	// tack on keywords
	buffptr = command + strlen( command );
	count   = Cmd_Argc();
	for ( i = 3; i < count; i++ )
		buffptr += sprintf( buffptr, " %s", Cmd_Argv( i ) );

	// if we are a demo, automatically add a "demo" keyword
	if ( Cvar_VariableValue( "fs_restrict" ) ) {
		buffptr += sprintf( buffptr, " demo" );
	}

	NET_OutOfBandPrint( NS_SERVER, to, command );
}


/*
==================
CL_GetPing
==================
*/
void CL_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	const char  *str;
	int time;
	int maxPing;

#if !defined RTCW_ET
	if ( !cl_pinglist[n].adr.port ) {
#else
	if ( n < 0 || n >= MAX_PINGREQUESTS || !cl_pinglist[n].adr.port ) {
#endif // RTCW_XX

		// empty slot
		buf[0]    = '\0';
		*pingtime = 0;
		return;
	}

	str = NET_AdrToString( cl_pinglist[n].adr );
	Q_strncpyz( buf, str, buflen );

	time = cl_pinglist[n].time;
	if ( !time ) {
		// check for timeout
		time = cls.realtime - cl_pinglist[n].start;
		maxPing = Cvar_VariableIntegerValue( "cl_maxPing" );
		if ( maxPing < 100 ) {
			maxPing = 100;
		}
		if ( time < maxPing ) {
			// not timed out yet
			time = 0;
		}
	}

	CL_SetServerInfoByAddress( cl_pinglist[n].adr, cl_pinglist[n].info, cl_pinglist[n].time );

	*pingtime = time;
}

/*
==================
CL_UpdateServerInfo
==================
*/
void CL_UpdateServerInfo( int n ) {

#if !defined RTCW_ET
	if ( !cl_pinglist[n].adr.port ) {
#else
	if ( n < 0 || n >= MAX_PINGREQUESTS || !cl_pinglist[n].adr.port ) {
#endif // RTCW_XX

		return;
	}

	CL_SetServerInfoByAddress( cl_pinglist[n].adr, cl_pinglist[n].info, cl_pinglist[n].time );
}

/*
==================
CL_GetPingInfo
==================
*/
void CL_GetPingInfo( int n, char *buf, int buflen ) {

#if !defined RTCW_ET
	if ( !cl_pinglist[n].adr.port ) {
#else
	if ( n < 0 || n >= MAX_PINGREQUESTS || !cl_pinglist[n].adr.port ) {
#endif // RTCW_XX

		// empty slot
		if ( buflen ) {
			buf[0] = '\0';
		}
		return;
	}

	Q_strncpyz( buf, cl_pinglist[n].info, buflen );
}

/*
==================
CL_ClearPing
==================
*/
void CL_ClearPing( int n ) {
	if ( n < 0 || n >= MAX_PINGREQUESTS ) {
		return;
	}

	cl_pinglist[n].adr.port = 0;
}

/*
==================
CL_GetPingQueueCount
==================
*/
int CL_GetPingQueueCount( void ) {
	int i;
	int count;
	ping_t* pingptr;

	count   = 0;
	pingptr = cl_pinglist;

	for ( i = 0; i < MAX_PINGREQUESTS; i++, pingptr++ ) {
		if ( pingptr->adr.port ) {
			count++;
		}
	}

	return ( count );
}

/*
==================
CL_GetFreePing
==================
*/
ping_t* CL_GetFreePing( void ) {
	ping_t* pingptr;
	ping_t* best;
	int oldest;
	int i;
	int time;

	pingptr = cl_pinglist;
	for ( i = 0; i < MAX_PINGREQUESTS; i++, pingptr++ )
	{
		// find free ping slot
		if ( pingptr->adr.port ) {
			if ( !pingptr->time ) {
				if ( cls.realtime - pingptr->start < 500 ) {
					// still waiting for response
					continue;
				}
			} else if ( pingptr->time < 500 )     {
				// results have not been queried
				continue;
			}
		}

		// clear it
		pingptr->adr.port = 0;
		return ( pingptr );
	}

	// use oldest entry
	pingptr = cl_pinglist;
	best    = cl_pinglist;
	oldest  = INT_MIN;
	for ( i = 0; i < MAX_PINGREQUESTS; i++, pingptr++ )
	{
		// scan for oldest
		time = cls.realtime - pingptr->start;
		if ( time > oldest ) {
			oldest = time;
			best   = pingptr;
		}
	}

	return ( best );
}

/*
==================
CL_Ping_f
==================
*/
void CL_Ping_f( void ) {
	netadr_t to;
	ping_t*     pingptr;
	const char*       server;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "usage: ping [server]\n" );
		return;
	}

	memset( &to, 0, sizeof( netadr_t ) );

	server = Cmd_Argv( 1 );

	if ( !NET_StringToAdr( server, &to ) ) {
		return;
	}

	pingptr = CL_GetFreePing();

	memcpy( &pingptr->adr, &to, sizeof( netadr_t ) );
	pingptr->start = cls.realtime;
	pingptr->time  = 0;

	CL_SetServerInfoByAddress( pingptr->adr, NULL, 0 );

	NET_OutOfBandPrint( NS_CLIENT, to, "getinfo xxx" );
}

/*
==================
CL_UpdateVisiblePings_f
==================
*/
qboolean CL_UpdateVisiblePings_f( int source ) {
	int slots, i;
	char buff[MAX_STRING_CHARS];
	int pingTime;
	int max;
	qboolean status = qfalse;

	if ( source < 0 || source > AS_FAVORITES ) {
		return qfalse;
	}

	cls.pingUpdateSource = source;

	slots = CL_GetPingQueueCount();
	if ( slots < MAX_PINGREQUESTS ) {
		serverInfo_t *server = NULL;

		max = ( source == AS_GLOBAL ) ? MAX_GLOBAL_SERVERS : MAX_OTHER_SERVERS;
		switch ( source ) {
		case AS_LOCAL:
			server = &cls.localServers[0];
			max = cls.numlocalservers;
			break;

#if !defined RTCW_ET
		case AS_MPLAYER:
			server = &cls.mplayerServers[0];
			max = cls.nummplayerservers;
			break;
#endif // RTCW_XX

		case AS_GLOBAL:
			server = &cls.globalServers[0];
			max = cls.numglobalservers;
			break;
		case AS_FAVORITES:
			server = &cls.favoriteServers[0];
			max = cls.numfavoriteservers;
			break;
		}
		for ( i = 0; i < max; i++ ) {
			if ( server[i].visible ) {
				if ( server[i].ping == -1 ) {
					int j;

					if ( slots >= MAX_PINGREQUESTS ) {
						break;
					}
					for ( j = 0; j < MAX_PINGREQUESTS; j++ ) {
						if ( !cl_pinglist[j].adr.port ) {
							continue;
						}
						if ( NET_CompareAdr( cl_pinglist[j].adr, server[i].adr ) ) {
							// already on the list
							break;
						}
					}
					if ( j >= MAX_PINGREQUESTS ) {
						status = qtrue;
						for ( j = 0; j < MAX_PINGREQUESTS; j++ ) {
							if ( !cl_pinglist[j].adr.port ) {
								break;
							}
						}
						memcpy( &cl_pinglist[j].adr, &server[i].adr, sizeof( netadr_t ) );
						cl_pinglist[j].start = cls.realtime;
						cl_pinglist[j].time = 0;
						NET_OutOfBandPrint( NS_CLIENT, cl_pinglist[j].adr, "getinfo xxx" );
						slots++;
					}
				}
				// if the server has a ping higher than cl_maxPing or
				// the ping packet got lost
				else if ( server[i].ping == 0 ) {
					// if we are updating global servers
					if ( source == AS_GLOBAL ) {
						//
						if ( cls.numGlobalServerAddresses > 0 ) {
							// overwrite this server with one from the additional global servers
							cls.numGlobalServerAddresses--;
							CL_InitServerInfo( &server[i], &cls.globalServerAddresses[cls.numGlobalServerAddresses] );
							// NOTE: the server[i].visible flag stays untouched
						}
					}
				}
			}
		}
	}

	if ( slots ) {
		status = qtrue;
	}
	for ( i = 0; i < MAX_PINGREQUESTS; i++ ) {
		if ( !cl_pinglist[i].adr.port ) {
			continue;
		}
		CL_GetPing( i, buff, MAX_STRING_CHARS, &pingTime );
		if ( pingTime != 0 ) {
			CL_ClearPing( i );
			status = qtrue;
		}
	}

	return status;
}

/*
==================
CL_ServerStatus_f
==================
*/
void CL_ServerStatus_f( void ) {
	netadr_t to;
	const char        *server;
	serverStatus_t *serverStatus;

	Com_Memset( &to, 0, sizeof( netadr_t ) );

	if ( Cmd_Argc() != 2 ) {
		if ( cls.state != CA_ACTIVE || clc.demoplaying ) {
			Com_Printf( "Not connected to a server.\n" );
			Com_Printf( "Usage: serverstatus [server]\n" );
			return;
		}
		server = cls.servername;
	} else {
		server = Cmd_Argv( 1 );
	}

	if ( !NET_StringToAdr( server, &to ) ) {
		return;
	}

	NET_OutOfBandPrint( NS_CLIENT, to, "getstatus" );

	serverStatus = CL_GetServerStatus( to );
	serverStatus->address = to;
	serverStatus->print = qtrue;
	serverStatus->pending = qtrue;
}

/*
==================
CL_ShowIP_f
==================
*/
void CL_ShowIP_f( void ) {
	Sys_ShowIP();
}

/*
=================
bool CL_CDKeyValidate
=================
*/
qboolean CL_CDKeyValidate( const char *key, const char *checksum ) {
	char ch;
	byte sum;
	char chs[3];
	int i, len;

	len = strlen( key );
	if ( len != CDKEY_LEN ) {
		return qfalse;
	}

	if ( checksum && strlen( checksum ) != CDCHKSUM_LEN ) {
		return qfalse;
	}

	sum = 0;
	// for loop gets rid of conditional assignment warning
	for ( i = 0; i < len; i++ ) {
		ch = *key++;
		if ( ch >= 'a' && ch <= 'z' ) {
			ch -= 32;
		}
		switch ( ch ) {
		case '2':
		case '3':
		case '7':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'G':
		case 'H':
		case 'J':
		case 'L':
		case 'P':
		case 'R':
		case 'S':
		case 'T':
		case 'W':

#if defined RTCW_SP
			sum += ch;
#else
			sum = ( sum << 1 ) ^ ch;
#endif // RTCW_XX

			continue;
		default:
			return qfalse;
		}
	}


	sprintf( chs, "%02x", sum );

	if ( checksum && !Q_stricmp( chs, checksum ) ) {
		return qtrue;
	}

	if ( !checksum ) {
		return qtrue;
	}

	return qfalse;
}

// NERVE - SMF
/*
=======================
CL_AddToLimboChat

=======================
*/
void CL_AddToLimboChat( const char *str ) {
	int len;
	char *p, *ls;
	int lastcolor;
	int chatHeight;
	int i;

	chatHeight = LIMBOCHAT_HEIGHT;
	cl.limboChatPos = LIMBOCHAT_HEIGHT - 1;
	len = 0;

	// copy old strings
	for ( i = cl.limboChatPos; i > 0; i-- ) {
		strcpy( cl.limboChatMsgs[i], cl.limboChatMsgs[i - 1] );
	}

	// copy new string
	p = cl.limboChatMsgs[0];
	*p = 0;

	lastcolor = '7';

	ls = NULL;
	while ( *str ) {
		if ( len > LIMBOCHAT_WIDTH - 1 ) {

#if defined RTCW_SP
#if 0
			if ( ls ) {
				str -= ( p - ls );
				str++;
				p -= ( p - ls );
			}
			*p = 0;

			if ( cl.limboChatPos < LIMBOCHAT_HEIGHT - 1 ) {
				cl.limboChatPos++;
			}
			p = cl.limboChatMsgs[cl.limboChatPos];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
#endif
#endif // RTCW_XX

			break;
		}

		if ( Q_IsColorString( str ) ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		if ( *str == ' ' ) {
			ls = p;
		}
		*p++ = *str++;
		len++;
	}
	*p = 0;
}

/*
=======================
CL_GetLimboString

=======================
*/
qboolean CL_GetLimboString( int index, char *buf ) {
	if ( index >= LIMBOCHAT_HEIGHT ) {
		return qfalse;
	}

	strncpy( buf, cl.limboChatMsgs[index], 140 );
	return qtrue;
}
// -NERVE - SMF

#if !defined RTCW_SP
// NERVE - SMF - Localization code
#define FILE_HASH_SIZE      1024
#define MAX_VA_STRING       32000
#define MAX_TRANS_STRING    4096

typedef struct trans_s {
	char original[MAX_TRANS_STRING];
	char translated[MAX_LANGUAGES][MAX_TRANS_STRING];
	struct      trans_s *next;
	float x_offset;
	float y_offset;
	qboolean fromFile;
} trans_t;

static trans_t* transTable[FILE_HASH_SIZE];

/*
=======================
AllocTrans
=======================
*/
static trans_t* AllocTrans( char *original, char *translated[MAX_LANGUAGES] ) {
	trans_t *t;
	int i;

	t = static_cast<trans_t*> (malloc( sizeof( trans_t ) ));
	memset( t, 0, sizeof( trans_t ) );

	if ( original ) {
		strncpy( t->original, original, MAX_TRANS_STRING );
	}

	if ( translated ) {
		for ( i = 0; i < MAX_LANGUAGES; i++ )
			strncpy( t->translated[i], translated[i], MAX_TRANS_STRING );
	}

	return t;
}

/*
=======================
generateHashValue
=======================
*/
static int32_t generateHashValue( const char *fname ) {
	int i;
	int32_t hash;
	char letter;

	hash = 0;
	i = 0;
	while ( fname[i] != '\0' ) {
		letter = tolower( fname[i] );
		hash += (int32_t)( letter ) * ( i + 119 );
		i++;
	}
	hash &= ( FILE_HASH_SIZE - 1 );
	return hash;
}

/*
=======================
LookupTrans
=======================
*/
static trans_t* LookupTrans( char *original, char *translated[MAX_LANGUAGES], qboolean isLoading ) {
	trans_t *t, *newt, *prev = NULL;
	int32_t hash;

	hash = generateHashValue( original );

	for ( t = transTable[hash]; t; prev = t, t = t->next ) {
		if ( !Q_stricmp( original, t->original ) ) {
			if ( isLoading ) {
				Com_DPrintf( S_COLOR_YELLOW "WARNING: Duplicate string found: \"%s\"\n", original );
			}
			return t;
		}
	}

	newt = AllocTrans( original, translated );

	if ( prev ) {
		prev->next = newt;
	} else {
		transTable[hash] = newt;
	}

	if ( cl_debugTranslation->integer >= 1 && !isLoading ) {
		Com_Printf( "Missing translation: \'%s\'\n", original );
	}

	// see if we want to save out the translation table everytime a string is added
	if ( cl_debugTranslation->integer == 2 && !isLoading ) {

		//FIXME
		//CL_SaveTransTable();
		CL_SaveTransTable ("scripts/translation.cfg", qtrue);
	}

	return newt;
}

/*
=======================
CL_SaveTransTable
=======================
*/
void CL_SaveTransTable( const char *fileName, qboolean newOnly ) {
	int bucketlen, bucketnum, maxbucketlen, avebucketlen;
	int untransnum, transnum;
	const char *buf;
	fileHandle_t f;
	trans_t *t;
	int i, j, len;

	if ( cl.corruptedTranslationFile ) {
		Com_Printf( S_COLOR_YELLOW "WARNING: Cannot save corrupted translation file. Please reload first." );
		return;
	}

	FS_FOpenFileByMode( fileName, &f, FS_WRITE );

	bucketnum = 0;
	maxbucketlen = 0;
	avebucketlen = 0;
	transnum = 0;
	untransnum = 0;

	// write out version, if one
	if ( strlen( cl.translationVersion ) ) {
		buf = va( "#version\t\t\"%s\"\n", cl.translationVersion );
	} else {
		buf = va( "#version\t\t\"1.0 01/01/01\"\n" );
	}

	len = strlen( buf );
	FS_Write( buf, len, f );

	// write out translated strings
	for ( j = 0; j < 2; j++ ) {

		for ( i = 0; i < FILE_HASH_SIZE; i++ ) {
			t = transTable[i];

			if ( !t || ( newOnly && t->fromFile ) ) {
				continue;
			}

			bucketlen = 0;

			for ( ; t; t = t->next ) {
				bucketlen++;

				if ( strlen( t->translated[0] ) ) {
					if ( j ) {
						continue;
					}
					transnum++;
				} else {
					if ( !j ) {
						continue;
					}
					untransnum++;
				}

				buf = va( "{\n\tenglish\t\t\"%s\"\n", t->original );
				len = strlen( buf );
				FS_Write( buf, len, f );

				buf = va( "\tfrench\t\t\"%s\"\n", t->translated[LANGUAGE_FRENCH] );
				len = strlen( buf );
				FS_Write( buf, len, f );

				buf = va( "\tgerman\t\t\"%s\"\n", t->translated[LANGUAGE_GERMAN] );
				len = strlen( buf );
				FS_Write( buf, len, f );

				buf = va( "\titalian\t\t\"%s\"\n", t->translated[LANGUAGE_ITALIAN] );
				len = strlen( buf );
				FS_Write( buf, len, f );

				buf = va( "\tspanish\t\t\"%s\"\n", t->translated[LANGUAGE_SPANISH] );
				len = strlen( buf );
				FS_Write( buf, len, f );

#if !defined RTCW_ET
				buf = va( "}\n", t->original );
#else
				buf = "}\n";
#endif // RTCW_XX

				len = strlen( buf );
				FS_Write( buf, len, f );
			}

			if ( bucketlen > maxbucketlen ) {
				maxbucketlen = bucketlen;
			}

			if ( bucketlen ) {
				bucketnum++;
				avebucketlen += bucketlen;
			}
		}
	}

	Com_Printf( "Saved translation table.\nTotal = %i, Translated = %i, Untranslated = %i, aveblen = %2.2f, maxblen = %i\n",
				transnum + untransnum, transnum, untransnum, (float)avebucketlen / bucketnum, maxbucketlen );

	FS_FCloseFile( f );
}

/*
=======================
CL_CheckTranslationString

NERVE - SMF - compare formatting characters
=======================
*/
qboolean CL_CheckTranslationString( char *original, char *translated ) {
	char format_org[128], format_trans[128];
	int len, i;

	memset( format_org, 0, 128 );
	memset( format_trans, 0, 128 );

	// generate formatting string for original
	len = strlen( original );

	for ( i = 0; i < len; i++ ) {
		if ( original[i] != '%' ) {
			continue;
		}

		strcat( format_org, va( "%c%c ", '%', original[i + 1] ) );
	}

	// generate formatting string for translated
	len = strlen( translated );
	if ( !len ) {
		return qtrue;
	}

	for ( i = 0; i < len; i++ ) {
		if ( translated[i] != '%' ) {
			continue;
		}

		strcat( format_trans, va( "%c%c ", '%', translated[i + 1] ) );
	}

	// compare
	len = strlen( format_org );

	if ( len != strlen( format_trans ) ) {
		return qfalse;
	}

	for ( i = 0; i < len; i++ ) {
		if ( format_org[i] != format_trans[i] ) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
=======================
CL_LoadTransTable
=======================
*/
void CL_LoadTransTable( const char *fileName ) {
	char translated[MAX_LANGUAGES][MAX_VA_STRING];
	char original[MAX_VA_STRING];
	qboolean aborted;
	char *text;
	fileHandle_t f;
	const char *text_p;
	char *token;
	int len, i;
	trans_t *t;
	int count;

	count = 0;
	aborted = qfalse;
	cl.corruptedTranslationFile = qfalse;

	len = FS_FOpenFileByMode( fileName, &f, FS_READ );
	if ( len <= 0 ) {
		return;
	}

#if defined RTCW_ET
	// Gordon: shouldn't this be a z_malloc or something?
#endif // RTCW_XX

	text = static_cast<char*> (malloc( len + 1 ));

	if ( !text ) {
		return;
	}

	FS_Read( text, len, f );
	text[len] = 0;
	FS_FCloseFile( f );

	// parse the text
	text_p = text;

	do {
		token = COM_Parse( &text_p );
		if ( Q_stricmp( "{", token ) ) {
			// parse version number
			if ( !Q_stricmp( "#version", token ) ) {
				token = COM_Parse( &text_p );
				strcpy( cl.translationVersion, token );
				continue;
			}

			break;
		}

		// english
		token = COM_Parse( &text_p );
		if ( Q_stricmp( "english", token ) ) {
			aborted = qtrue;
			break;
		}

		token = COM_Parse( &text_p );
		strcpy( original, token );

		if ( cl_debugTranslation->integer == 3 ) {
			Com_Printf( "%i Loading: \"%s\"\n", count, original );
		}

		// french
		token = COM_Parse( &text_p );
		if ( Q_stricmp( "french", token ) ) {
			aborted = qtrue;
			break;
		}

		token = COM_Parse( &text_p );
		strcpy( translated[LANGUAGE_FRENCH], token );
		if ( !CL_CheckTranslationString( original, translated[LANGUAGE_FRENCH] ) ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: Translation formatting doesn't match up with English version!\n" );
			aborted = qtrue;
			break;
		}

		// german
		token = COM_Parse( &text_p );
		if ( Q_stricmp( "german", token ) ) {
			aborted = qtrue;
			break;
		}

		token = COM_Parse( &text_p );
		strcpy( translated[LANGUAGE_GERMAN], token );
		if ( !CL_CheckTranslationString( original, translated[LANGUAGE_GERMAN] ) ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: Translation formatting doesn't match up with English version!\n" );
			aborted = qtrue;
			break;
		}

		// italian
		token = COM_Parse( &text_p );
		if ( Q_stricmp( "italian", token ) ) {
			aborted = qtrue;
			break;
		}

		token = COM_Parse( &text_p );
		strcpy( translated[LANGUAGE_ITALIAN], token );
		if ( !CL_CheckTranslationString( original, translated[LANGUAGE_ITALIAN] ) ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: Translation formatting doesn't match up with English version!\n" );
			aborted = qtrue;
			break;
		}

		// spanish
		token = COM_Parse( &text_p );
		if ( Q_stricmp( "spanish", token ) ) {
			aborted = qtrue;
			break;
		}

		token = COM_Parse( &text_p );
		strcpy( translated[LANGUAGE_SPANISH], token );
		if ( !CL_CheckTranslationString( original, translated[LANGUAGE_SPANISH] ) ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: Translation formatting doesn't match up with English version!\n" );
			aborted = qtrue;
			break;
		}

		// do lookup
		t = LookupTrans( original, NULL, qtrue );

		if ( t ) {
			t->fromFile = qtrue;

			for ( i = 0; i < MAX_LANGUAGES; i++ )
				strncpy( t->translated[i], translated[i], MAX_TRANS_STRING );
		}

		token = COM_Parse( &text_p );

		// set offset if we have one
		if ( !Q_stricmp( "offset", token ) ) {
			token = COM_Parse( &text_p );
			t->x_offset = atof( token );

			token = COM_Parse( &text_p );
			t->y_offset = atof( token );

			token = COM_Parse( &text_p );
		}

		if ( Q_stricmp( "}", token ) ) {
			aborted = qtrue;
			break;
		}

		count++;
	} while ( token );

	if ( aborted ) {
		int i, line = 1;

		for ( i = 0; i < len && ( text + i ) < text_p; i++ ) {
			if ( text[i] == '\n' ) {
				line++;
			}
		}

		Com_Printf( S_COLOR_YELLOW "WARNING: Problem loading %s on line %i\n", fileName, line );
		cl.corruptedTranslationFile = qtrue;
	} else {
		Com_Printf( "Loaded %i translation strings from %s\n", count, fileName );
	}

	// cleanup
	free( text );
}

/*
=======================
CL_ReloadTranslation
=======================
*/
void CL_ReloadTranslation() {
	char    **fileList;
	int numFiles, i;

	for ( i = 0; i < FILE_HASH_SIZE; i++ ) {
		if ( transTable[i] ) {
			free( transTable[i] );
		}
	}

	memset( transTable, 0, sizeof( trans_t* ) * FILE_HASH_SIZE );
	CL_LoadTransTable( "scripts/translation.cfg" );

	fileList = FS_ListFiles( "translations", "cfg", &numFiles );

	for ( i = 0; i < numFiles; i++ ) {
		CL_LoadTransTable( va( "translations/%s", fileList[i] ) );
	}
}

/*
=======================
CL_InitTranslation
=======================
*/
void CL_InitTranslation() {
	char    **fileList;
	int numFiles, i;

	memset( transTable, 0, sizeof( trans_t* ) * FILE_HASH_SIZE );
	CL_LoadTransTable( "scripts/translation.cfg" );

	fileList = FS_ListFiles( "translations", ".cfg", &numFiles );

	for ( i = 0; i < numFiles; i++ ) {
		CL_LoadTransTable( va( "translations/%s", fileList[i] ) );
	}
}

/*
=======================
CL_TranslateString
=======================
*/
void CL_TranslateString( const char *string, char *dest_buffer ) {
	int i, count, currentLanguage;
	trans_t *t;
	qboolean newline = qfalse;
	char *buf;

	buf = dest_buffer;
	currentLanguage = cl_language->integer - 1;

	// early bail if we only want english or bad language type
	if ( !string ) {
		strcpy( buf, "(null)" );
		return;

#if !defined RTCW_ET
	} else if ( currentLanguage == -1 || currentLanguage >= MAX_LANGUAGES || !strlen( string ) )   {
#else
	} else if ( currentLanguage < 0 || currentLanguage >= MAX_LANGUAGES || !strlen( string ) )   {
#endif // RTCW_XX

		strcpy( buf, string );
		return;
	}

	// ignore newlines
	if ( string[strlen( string ) - 1] == '\n' ) {
		newline = qtrue;
	}

	for ( i = 0, count = 0; string[i] != '\0'; i++ ) {
		if ( string[i] != '\n' ) {
			buf[count++] = string[i];
		}
	}
	buf[count] = '\0';

	t = LookupTrans( buf, NULL, qfalse );

	if ( t && strlen( t->translated[currentLanguage] ) ) {
		int offset = 0;

		if ( cl_debugTranslation->integer >= 1 ) {
			buf[0] = '^';
			buf[1] = '1';
			buf[2] = '[';
			offset = 3;
		}

		strcpy( buf + offset, t->translated[currentLanguage] );

		if ( cl_debugTranslation->integer >= 1 ) {
			int len2 = strlen( buf );

			buf[len2] = ']';
			buf[len2 + 1] = '^';
			buf[len2 + 2] = '7';
			buf[len2 + 3] = '\0';
		}

		if ( newline ) {
			int len2 = strlen( buf );

			buf[len2] = '\n';
			buf[len2 + 1] = '\0';
		}
	} else {
		int offset = 0;

		if ( cl_debugTranslation->integer >= 1 ) {
			buf[0] = '^';
			buf[1] = '1';
			buf[2] = '[';
			offset = 3;
		}

		strcpy( buf + offset, string );

		if ( cl_debugTranslation->integer >= 1 ) {
			int len2 = strlen( buf );
			qboolean addnewline = qfalse;

			if ( buf[len2 - 1] == '\n' ) {
				len2--;
				addnewline = qtrue;
			}

			buf[len2] = ']';
			buf[len2 + 1] = '^';
			buf[len2 + 2] = '7';
			buf[len2 + 3] = '\0';

			if ( addnewline ) {
				buf[len2 + 3] = '\n';
				buf[len2 + 4] = '\0';
			}
		}
	}
}

/*
=======================
CL_TranslateStringBuf
TTimo - handy, stores in a static buf, converts \n to chr(13)
=======================
*/
const char* CL_TranslateStringBuf( const char *string ) {
	char *p;
	int i,l;
	static char buf[MAX_VA_STRING];
	CL_TranslateString( string, buf );

#if !defined RTCW_ET
	while ( ( p = strstr( buf, "\\n" ) ) )
#else
	while ( ( p = strstr( buf, "\\n" ) ) != NULL )
#endif // RTCW_XX

	{
		*p = '\n';
		p++;
		// Com_Memcpy(p, p+1, strlen(p) ); b0rks on win32
		l = strlen( p );
		for ( i = 0; i < l; i++ )
		{
			*p = *( p + 1 );
			p++;
		}
	}
	return buf;
}

/*
=======================
CL_OpenURLForCvar
=======================
*/
void CL_OpenURL( const char *url ) {
	if ( !url || !strlen( url ) ) {
		Com_Printf( "%s", CL_TranslateStringBuf( "invalid/empty URL\n" ) );
		return;
	}
	Sys_OpenURL( url, qtrue );
}
#endif // RTCW_XX

#if defined RTCW_ET
// Gordon: TEST TEST TEST
/*
==================
BotImport_DrawPolygon
==================
*/
void BotImport_DrawPolygon( int color, int numpoints, float* points ) {
	re.DrawDebugPolygon( color, numpoints, points );
}
#endif // RTCW_XX

