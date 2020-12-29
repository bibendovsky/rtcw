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


/*****************************************************************************
 * name:		snd_dma.c
 *
 * desc:		main control for any streaming sound output device
 *
 * $Archive: /Wolf5/src/client/snd_dma.c $
 *
 *****************************************************************************/

#include "snd_local.h"
#include "client.h"

void S_Play_f( void );
void S_SoundList_f( void );
void S_Music_f( void );

#if !defined RTCW_MP
void S_QueueMusic_f( void );
#endif // RTCW_XX

void S_StreamingSound_f( void );

#if !defined RTCW_MP
void S_ClearSounds( qboolean clearStreaming, qboolean clearMusic ); //----(SA)	modified
#endif // RTCW_XX

void S_Update_Mix();
void S_StopAllSounds( void );
void S_UpdateStreamingSounds( void );

#if !defined RTCW_MP
snd_t snd;  // globals for sound
#endif // RTCW_XX

// Ridah, streaming sounds
// !! NOTE: the first streaming sound is always the music
streamingSound_t streamingSounds[MAX_STREAMING_SOUNDS];
int numStreamingSounds = 0;

#if defined RTCW_MP
static vec3_t entityPositions[MAX_GENTITIES];
#endif // RTCW_XX

#if defined RTCW_MP
typedef struct {
	vec3_t origin;
	qboolean fixedOrigin;
	int entityNum;
	int entityChannel;
	sfxHandle_t sfx;
	int flags;
} s_pushStack;

#define MAX_PUSHSTACK 64
static s_pushStack pushPop[MAX_PUSHSTACK];
static int tart = 0;

typedef struct {
	char intro[256];
	char loop[256];
	qboolean music;
	int entnum;
	int channel;
	int attenuation;
} s_streamStack;

//static s_streamStack	Sstream[MAX_PUSHSTACK]; // TTimo: unused
//static int onStream; // TTimo: unused
#endif // RTCW_XX

// =======================================================================
// Internal sound data & structures
// =======================================================================

// only begin attenuating sound volumes when outside the FULLVOLUME range
#define     SOUND_FULLVOLUME    (80)

#define     SOUND_ATTENUATE     (0.0008F)
#define     SOUND_RANGE_DEFAULT (1250)

channel_t s_channels[MAX_CHANNELS];
channel_t loop_channels[MAX_CHANNELS];
int numLoopChannels;

#if defined RTCW_MP
static int s_soundStarted;
static qboolean s_soundMuted;
static qboolean s_soundPainted;
static int s_clearSoundBuffer = 0;
#endif // RTCW_XX

dma_t dma;

static int listener_number;
static vec3_t listener_origin;
static vec3_t listener_axis[3];

int s_soundtime;                // sample PAIRS
int s_paintedtime;              // sample PAIRS

// MAX_SFX may be larger than MAX_SOUNDS because
// of custom player sounds
#define     MAX_SFX         4096
sfx_t s_knownSfx[MAX_SFX];

#if defined RTCW_MP
int s_numSfx = 0;

#define     LOOP_HASH       128
static sfx_t       *sfxHash[LOOP_HASH];
#endif // RTCW_XX

cvar_t      *s_volume;
cvar_t      *s_testsound;
cvar_t      *s_khz;
cvar_t      *s_show;
cvar_t      *s_mixahead;
cvar_t      *s_mixPreStep;
cvar_t      *s_musicVolume;

#if !defined RTCW_MP
cvar_t      *s_currentMusic;    //----(SA)	added
#endif // RTCW_XX

cvar_t      *s_separation;
cvar_t      *s_doppler;
cvar_t      *s_mute;        // (SA) for DM so he can 'toggle' sound on/off without disturbing volume levels
cvar_t      *s_defaultsound; // (SA) added to silence the default beep sound if desired
cvar_t      *cl_cacheGathering; // Ridah
cvar_t      *s_wavonly;

#if !defined RTCW_MP
cvar_t      *s_debugMusic;  //----(SA)	added

#else
#define MAX_LOOP_SOUNDS     128
static int numLoopSounds;
static loopSound_t loopSounds[MAX_LOOP_SOUNDS];

static channel_t       *freelist = NULL;
static channel_t       *endflist = NULL;
#endif // RTCW_XX

// Rafael
cvar_t      *s_nocompressed;

#if defined RTCW_ET
// fretn
cvar_t      *s_bits;
cvar_t      *s_numchannels;
#endif // RTCW_XX

// for streaming sounds
int s_rawend[MAX_STREAMING_SOUNDS];
int s_rawpainted[MAX_STREAMING_SOUNDS];
portable_samplepair_t s_rawsamples[MAX_STREAMING_SOUNDS][MAX_RAW_SAMPLES];
// RF, store the volumes, since now they get adjusted at time of painting, so we can extract talking data first
portable_samplepair_t s_rawVolume[MAX_STREAMING_SOUNDS];


/*
================
S_SoundInfo_f
================
*/
void S_SoundInfo_f( void ) {
	Com_Printf( "----- Sound Info -----\n" );

#if !defined RTCW_MP
	if ( !snd.s_soundStarted ) {
#else
	if ( !s_soundStarted ) {
#endif // RTCW_XX

		Com_Printf( "sound system not started\n" );
	} else {

#if !defined RTCW_MP
		if ( snd.s_soundMute ) {
#else
		if ( s_soundMuted ) {
#endif // RTCW_XX

			Com_Printf( "sound system is muted\n" );
		}

		Com_Printf( "%5d stereo\n", dma.channels - 1 );
		Com_Printf( "%5d samples\n", dma.samples );
		Com_Printf( "%5d samplebits\n", dma.samplebits );
		Com_Printf( "%5d submission_chunk\n", dma.submission_chunk );
		Com_Printf( "%5d speed\n", dma.speed );

// BBi Useless
//#if !defined RTCW_ET
//		Com_Printf( "0x%x dma buffer\n", dma.buffer );
//#else
//		Com_Printf( "0x%p dma buffer\n", dma.buffer );
//#endif // RTCW_XX
// BBi

		if ( streamingSounds[0].file ) {
			Com_Printf( "Background file: %s\n", streamingSounds[0].loop );
		} else {
			Com_Printf( "No background file.\n" );
		}

	}
	Com_Printf( "----------------------\n" );
}

void S_ChannelSetup();

/*
================
S_Init
================
*/
void S_Init( void ) {
	cvar_t  *cv;
	qboolean r;

	Com_Printf( "\n------- sound initialization -------\n" );

	s_mute = Cvar_Get( "s_mute", "0", CVAR_TEMP ); //----(SA)	added
	s_volume = Cvar_Get( "s_volume", "0.8", CVAR_ARCHIVE );
	s_musicVolume = Cvar_Get( "s_musicvolume", "0.25", CVAR_ARCHIVE );

#if !defined RTCW_MP
	s_currentMusic = Cvar_Get( "s_currentMusic", "", CVAR_ROM );
#endif // RTCW_XX

	s_separation = Cvar_Get( "s_separation", "0.5", CVAR_ARCHIVE );
	s_doppler = Cvar_Get( "s_doppler", "1", CVAR_ARCHIVE );

// BBi
//#if !defined RTCW_ET
//	s_khz = Cvar_Get( "s_khz", "22", CVAR_ARCHIVE );
//#else
//	s_khz = Cvar_Get( "s_khz", "22", CVAR_ARCHIVE | CVAR_LATCH );
//#endif // RTCW_XX

	s_khz = Cvar_Get ("s_khz", "22", CVAR_ARCHIVE | CVAR_LATCH);
// BBi

// BBi
//#if defined RTCW_ET
//	s_mixahead = Cvar_Get( "s_mixahead", "0.2", CVAR_ARCHIVE );
//#endif // RTCW_XX
//
//#if defined RTCW_SP
//	s_mixahead = Cvar_Get( "s_mixahead", "0.5", CVAR_ARCHIVE );    //DAJ was 0.2
//	s_debugMusic = Cvar_Get( "s_debugMusic", "0", CVAR_TEMP );
//#elif defined RTCW_MP
//	s_mixahead = Cvar_Get( "s_mixahead", "0.2", CVAR_ARCHIVE );
//#else
//	s_debugMusic = Cvar_Get( "s_debugMusic", "0", CVAR_TEMP );
//#endif // RTCW_XX

	static const char* const DEFAULT_MIX_AHEAD_STRING =
#if defined RTCW_SP
		"0.5"
#else
		"0.2"
#endif //RTCW_XX
		;

	s_mixahead = Cvar_Get ("s_mixahead", DEFAULT_MIX_AHEAD_STRING, CVAR_ARCHIVE);

#if !defined RTCW_MP
	s_debugMusic = Cvar_Get ("s_debugMusic", "0", CVAR_TEMP);
#endif // RTCW_XX
// BBi

	s_mixPreStep = Cvar_Get( "s_mixPreStep", "0.05", CVAR_ARCHIVE );
	s_show = Cvar_Get( "s_show", "0", CVAR_CHEAT );
	s_testsound = Cvar_Get( "s_testsound", "0", CVAR_CHEAT );
	s_defaultsound = Cvar_Get( "s_defaultsound", "0", CVAR_ARCHIVE );
	s_wavonly = Cvar_Get( "s_wavonly", "0", CVAR_ARCHIVE | CVAR_LATCH );
	// Ridah
	cl_cacheGathering = Cvar_Get( "cl_cacheGathering", "0", 0 );

	// Rafael
	s_nocompressed = Cvar_Get( "s_nocompressed", "0", CVAR_INIT );

// BBi Useless but keep for compatibility
//#if defined RTCW_ET
//	// fretn
//	s_bits = Cvar_Get( "s_bits", "16", CVAR_LATCH | CVAR_ARCHIVE );
//	s_numchannels = Cvar_Get( "s_channels", "2", CVAR_LATCH | CVAR_ARCHIVE );
//#endif // RTCW_XX

#if defined RTCW_ET
	// fretn
	s_bits = Cvar_Get ("s_bits", "16", CVAR_ROM);
	s_numchannels = Cvar_Get ("s_channels", "2", CVAR_ROM);
#endif // RTCW_XX
// BBi

	cv = Cvar_Get( "s_initsound", "1", 0 );
	if ( !cv->integer ) {
		Com_Printf( "not initializing.\n" );
		Com_Printf( "------------------------------------\n" );
		return;
	}

	Cmd_AddCommand( "play", S_Play_f );
	Cmd_AddCommand( "music", S_Music_f );

#if defined RTCW_SP
	Cmd_AddCommand( "queuemusic", S_QueueMusic_f );
#elif defined RTCW_ET
	Cmd_AddCommand( "music_queue", S_QueueMusic_f );
#endif // RTCW_XX

	Cmd_AddCommand( "streamingsound", S_StreamingSound_f );
	Cmd_AddCommand( "s_list", S_SoundList_f );
	Cmd_AddCommand( "s_info", S_SoundInfo_f );
	Cmd_AddCommand( "s_stop", S_StopAllSounds );

	r = SNDDMA_Init();
	Com_Printf( "------------------------------------\n" );

	if ( r ) {
#if !defined RTCW_MP
		Com_Memset( &snd, 0, sizeof( snd ) );
//		Com_Memset(snd.sfxHash, 0, sizeof(sfx_t *)*LOOP_HASH);

		snd.s_soundStarted = 1;
		snd.s_soundMute = 1;
//		snd.s_numSfx = 0;

#if !defined RTCW_ET
//		snd.volTarget = 1.0f;	// full volume
		snd.volTarget = 0.0f;   // full volume
#else
		snd.volTarget = 0.0f;
		//snd.volTarget = 1.0f;	// full volume
#endif // RTCW_XX

#else
		Com_Memset( sfxHash, 0, sizeof( sfx_t * ) * LOOP_HASH );

		s_soundStarted = 1;
		s_soundMuted = 1;
//		s_numSfx = 0;
#endif // RTCW_XX

		s_soundtime = 0;
		s_paintedtime = 0;

		S_StopAllSounds();

		S_SoundInfo_f();
		S_ChannelSetup();
	}
}

/*
================
S_ChannelFree
================
*/
void S_ChannelFree( channel_t *v ) {
	v->thesfx = NULL;
	v->threadReady = qfalse;

#if !defined RTCW_MP
#ifdef _DEBUG
	if ( v >  &s_channels[MAX_CHANNELS] || v <  &s_channels[0] ) {
		Com_DPrintf( "s_channel OUT OF BOUNDS\n" );
		return;
	}
#endif
	*(channel_t **)snd.endflist = v;
	snd.endflist = v;
#else
	*(channel_t **)endflist = v;
	endflist = v;
#endif // RTCW_XX

	*(channel_t **)v = NULL;
}

/*
================
S_ChannelMalloc
================
*/
channel_t*  S_ChannelMalloc() {
	channel_t *v;

#if !defined RTCW_MP
	if ( snd.freelist == NULL ) {
		return NULL;
	}
	// RF, be careful not to lose our freelist
	if ( *(channel_t **)snd.freelist == NULL ) {
		return NULL;
	}
#ifdef _DEBUG
	if ( *(channel_t **)snd.freelist > &s_channels[MAX_CHANNELS] || *(channel_t **)snd.freelist < &s_channels[0] ) {   //DAJ	extra check
		Com_DPrintf( "s_channel OUT OF BOUNDS\n" );
		return NULL;
	}
#endif
	v = snd.freelist;
	snd.freelist = *(channel_t **)snd.freelist;
#else
	if ( freelist == NULL ) {
		return NULL;
	}
	v = freelist;
	freelist = *(channel_t **)freelist;
#endif // RTCW_XX

	v->allocTime = Sys_Milliseconds();
	return v;
}

/*
================
S_ChannelSetup
================
*/
void S_ChannelSetup() {
	channel_t *p, *q;

	// clear all the sounds so they don't
	Com_Memset( s_channels, 0, sizeof( s_channels ) );

	p = s_channels;;
	q = p + MAX_CHANNELS;
	while ( --q > p ) {
		*(channel_t **)q = q - 1;
	}

#if !defined RTCW_MP
	snd.endflist = q;
#else
	endflist = q;
#endif // RTCW_XX

	*(channel_t **)q = NULL;

#if !defined RTCW_MP
	snd.freelist = p + MAX_CHANNELS - 1;
#else
	freelist = p + MAX_CHANNELS - 1;
#endif // RTCW_XX

	Com_DPrintf( "Channel memory manager started\n" );
}

/*
================
S_Shutdown
================
*/
void S_Shutdown( void ) {

#if !defined RTCW_MP
	if ( !snd.s_soundStarted ) {
#else
	if ( !s_soundStarted ) {
#endif // RTCW_XX

		return;
	}

	SNDDMA_Shutdown();

#if !defined RTCW_MP
	snd.s_soundStarted = 0;
	snd.s_soundMute = 1;
#else
	s_soundStarted = 0;
	s_soundMuted = qtrue;
#endif // RTCW_XX

	Cmd_RemoveCommand( "play" );
	Cmd_RemoveCommand( "music" );

#if !defined RTCW_ET
	Cmd_RemoveCommand( "stopsound" );
	Cmd_RemoveCommand( "soundlist" );
	Cmd_RemoveCommand( "soundinfo" );
#else
	Cmd_RemoveCommand( "s_stop" );
	Cmd_RemoveCommand( "s_list" );
	Cmd_RemoveCommand( "s_info" );
#endif // RTCW_XX

}

/*
================
S_HashSFXName

return a hash value for the sfx name
================
*/
static int32_t S_HashSFXName( const char *name ) {
	int i;
	int32_t hash;
	char letter;

	hash = 0;
	i = 0;
	while ( name[i] != '\0' ) {
		letter = tolower( name[i] );
		if ( letter == '.' ) {
			break;                          // don't include extension
		}
		if ( letter == '\\' ) {
			letter = '/';                   // damn path names
		}
		hash += (int32_t)( letter ) * ( i + 119 );
		i++;
	}
	hash &= ( LOOP_HASH - 1 );
	return hash;
}

#if defined RTCW_MP
/*
======================
S_FreeOldestSound
======================
*/
void S_FreeOldestSound( void ) {
	int i, oldest, used;
	sfx_t   *sfx;
	sndBuffer   *buffer, *nbuffer;

	oldest = Sys_Milliseconds();
	used = 0;

	for ( i = 1 ; i < s_numSfx ; i++ ) {
		sfx = &s_knownSfx[i];
		if ( sfx->inMemory && sfx->lastTimeUsed < oldest ) {
			used = i;
			oldest = sfx->lastTimeUsed;
		}
	}

	sfx = &s_knownSfx[used];

	// DHM - Nerve :: can cause race conditions
	//Com_DPrintf("S_FreeOldestSound: freeing sound %s\n", sfx->soundName);

	buffer = sfx->soundData;
	while ( buffer != NULL ) {
		nbuffer = buffer->next;
		SND_free( buffer );
		buffer = nbuffer;
	}
	sfx->inMemory = qfalse;
	sfx->soundData = NULL;
}
#endif // RTCW_XX

/*
==================
S_FindName

Will allocate a new sfx if it isn't found
==================
*/
static sfx_t *S_FindName( const char *name ) {
	int i;
	int hash;

	sfx_t   *sfx;

	if ( !name ) {
		//Com_Error (ERR_FATAL, "S_FindName: NULL\n");
		name = "*default*";
	}
	if ( !name[0] ) {
		//Com_Error (ERR_FATAL, "S_FindName: empty name\n");
		name = "*default*";
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Error( ERR_FATAL, "Sound name too long: %s", name );
	}

	// Ridah, caching
	if ( cl_cacheGathering->integer ) {
		Cbuf_ExecuteText( EXEC_NOW, va( "cache_usedfile sound %s\n", name ) );
	}

	hash = S_HashSFXName( name );

#if !defined RTCW_MP
	sfx = snd.sfxHash[hash];
#else
	sfx = sfxHash[hash];
#endif // RTCW_XX

	// see if already loaded
	while ( sfx ) {
		if ( !Q_stricmp( sfx->soundName, name ) ) {
			return sfx;
		}
		sfx = sfx->next;
	}

	// find a free sfx

#if !defined RTCW_MP
	for ( i = 0 ; i < snd.s_numSfx ; i++ ) {
#else
	for ( i = 0 ; i < s_numSfx ; i++ ) {
#endif // RTCW_XX

		if ( !s_knownSfx[i].soundName[0] ) {
			break;
		}
	}

#if !defined RTCW_MP
	if ( i == snd.s_numSfx ) {
		if ( snd.s_numSfx == MAX_SFX ) {
#else
	if ( i == s_numSfx ) {
		if ( s_numSfx == MAX_SFX ) {
#endif // RTCW_XX

			Com_Error( ERR_FATAL, "S_FindName: out of sfx_t" );
		}

#if !defined RTCW_MP
		snd.s_numSfx++;
#else
		s_numSfx++;
#endif // RTCW_XX

	}

	sfx = &s_knownSfx[i];
	Com_Memset( sfx, 0, sizeof( *sfx ) );
	strcpy( sfx->soundName, name );

#if !defined RTCW_MP
	sfx->next = snd.sfxHash[hash];
	snd.sfxHash[hash] = sfx;
#else
	sfx->next = sfxHash[hash];
	sfxHash[hash] = sfx;
#endif // RTCW_XX

	return sfx;
}

/*
=================
S_DefaultSound
=================
*/
void S_DefaultSound( sfx_t *sfx ) {
	int i;

#if !defined RTCW_MP
	if ( s_defaultsound->integer ) {
		sfx->soundLength = 512;
	} else {
		sfx->soundLength = 8;
	}
#else
	sfx->soundLength = 512;
#endif // RTCW_XX

	sfx->soundData = SND_malloc();
	sfx->soundData->next = NULL;

	if ( s_defaultsound->integer ) {
		for ( i = 0 ; i < sfx->soundLength ; i++ ) {
			sfx->soundData->sndChunk[i] = i;
		}
	} else {
		for ( i = 0 ; i < sfx->soundLength ; i++ ) {
			sfx->soundData->sndChunk[i] = 0;
		}
	}
}

#if defined RTCW_ET
/*
================
S_Reload
================
*/
void S_Reload( void ) {
	sfx_t *sfx;
	int i;

	if ( !snd.s_soundStarted ) {
		return;
	}

	Com_Printf( "reloading sounds...\n" );

	S_StopAllSounds();

	for ( sfx = s_knownSfx, i = 0; i < snd.s_numSfx; i++, sfx++ ) {
		sfx->inMemory = qfalse;
		S_memoryLoad( sfx );
	}
}
#endif // RTCW_XX

/*
===================
S_DisableSounds

Disables sounds until the next S_BeginRegistration.
This is called when the hunk is cleared and the sounds
are no longer valid.
===================
*/
void S_DisableSounds( void ) {
	S_StopAllSounds();

#if !defined RTCW_MP
	snd.s_soundMute = 1;
#else
	s_soundMuted = qtrue;
#endif // RTCW_XX

}

/*
=====================
S_BeginRegistration
=====================
*/
void S_BeginRegistration( void ) {
	sfx_t   *sfx;

#if !defined RTCW_MP
	snd.s_soundMute = 0;        // we can play again

	if ( snd.s_numSfx == 0 ) {
		SND_setup();

		snd.s_numSfx = 0;
		Com_Memset( s_knownSfx, 0, sizeof( s_knownSfx ) );
		Com_Memset( snd.sfxHash, 0, sizeof( sfx_t * ) * LOOP_HASH );
#else
	s_soundMuted = qfalse;      // we can play again

	if ( s_numSfx == 0 ) {
		SND_setup();

		s_numSfx = 0;
		Com_Memset( s_knownSfx, 0, sizeof( s_knownSfx ) );
		Com_Memset( sfxHash, 0, sizeof( sfx_t * ) * LOOP_HASH );
#endif // RTCW_XX

		sfx = S_FindName( "***DEFAULT***" );
		S_DefaultSound( sfx );
	}
}

/*
==================
S_RegisterSound

Creates a default buzz sound if the file can't be loaded
==================
*/
sfxHandle_t S_RegisterSound( const char *name, qboolean compressed ) {
	sfx_t   *sfx;

#if defined RTCW_SP
	compressed = qfalse;
	if ( !snd.s_soundStarted ) {
#elif defined RTCW_MP
	if ( !s_soundStarted ) {
#else
	//compressed = qfalse; // Arnout: memory corruption with compressed sounds?

	if ( !snd.s_soundStarted ) {
#endif // RTCW_XX

		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {

#if !defined RTCW_MP
		Com_Printf( "Sound name exceeds MAX_QPATH\n" );
#else
		Com_DPrintf( "Sound name exceeds MAX_QPATH\n" );
#endif // RTCW_XX

		return 0;
	}

	sfx = S_FindName( name );
	if ( sfx->soundData ) {
		if ( sfx->defaultSound ) {
			if ( com_developer->integer ) {

#if !defined RTCW_MP
				Com_Printf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->soundName );
#else
				Com_DPrintf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->soundName );
#endif // RTCW_XX

			}
			return 0;
		}
		return sfx - s_knownSfx;
	}

	sfx->inMemory = qfalse;
	sfx->soundCompressed = compressed;

//	if (!compressed) {
	S_memoryLoad( sfx );
//	}

	if ( sfx->defaultSound ) {
		if ( com_developer->integer ) {

#if !defined RTCW_MP
			Com_Printf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->soundName );
#else
			Com_DPrintf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->soundName );
#endif // RTCW_XX

		}
		return 0;
	}

	return sfx - s_knownSfx;
}

/*
=================
S_memoryLoad
=================
*/
void S_memoryLoad( sfx_t *sfx ) {
	// load the sound file
	if ( !S_LoadSound( sfx ) ) {
//		Com_Printf( S_COLOR_YELLOW "WARNING: couldn't load sound: %s\n", sfx->soundName );
		sfx->defaultSound = qtrue;
	}
	sfx->inMemory = qtrue;
}

//=============================================================================

/*
=================
S_SpatializeOrigin

Used for spatializing s_channels
=================
*/

#if !defined RTCW_ET
void S_SpatializeOrigin( vec3_t origin, int master_vol, int *left_vol, int *right_vol, float range ) {
	vec_t dot;
#else
// xkan, 11/04/2002 - added noAttenuation arg.
void S_SpatializeOrigin( vec3_t origin, int master_vol, int *left_vol, int *right_vol, float range, int noAttenuation ) {
//	vec_t		dot;
#endif // RTCW_XX

	vec_t dist;
	vec_t lscale, rscale, scale;
	vec3_t source_vec;
	vec3_t vec;

//	const float dist_mult = SOUND_ATTENUATE;
	float dist_mult, dist_fullvol;

	dist_fullvol = range * 0.064f;        // default range of 1250 gives 80
	dist_mult = dist_fullvol * 0.00001f;  // default range of 1250 gives .0008
//	dist_mult = range*0.00000064f;		// default range of 1250 gives .0008

	// calculate stereo seperation and distance attenuation
	VectorSubtract( origin, listener_origin, source_vec );

	dist = VectorNormalize( source_vec );
//	dist -= SOUND_FULLVOLUME;
	dist -= dist_fullvol;

#if !defined RTCW_ET
	if ( dist < 0 ) {
#else
	if ( dist < 0 || noAttenuation ) {
#endif // RTCW_XX

		dist = 0;           // close enough to be at full volume

	}
	if ( dist ) {
		dist = dist / range;  // FIXME: lose the divide again
	}
//	dist *= dist_mult;		// different attenuation levels

	VectorRotate( source_vec, listener_axis, vec );

#if !defined RTCW_ET
	dot = -vec[1];

	if ( dma.channels == 1 ) { // no attenuation = no spatialization
#else
//	dot = -vec[1];

	if ( dma.channels == 1 || noAttenuation ) { // no attenuation = no spatialization
#endif // RTCW_XX

		rscale = 1.0;
		lscale = 1.0;
	} else
	{

#if !defined RTCW_ET
		rscale = 0.5 * ( 1.0 + dot );
		lscale = 0.5 * ( 1.0 - dot );
#else
		// xkan 11/22/2002 - the total energy of left + right should stay constant
		// and the energy is proportional to the square of sound volume.
		//
		// therefore lscale and rscale should satisfy the following 2 conditions:
		//		lscale^2 + rscale^2 = 2
		//		lscale^2 - rscale^2 = 2*vec[1]
		// (the second condition here is more experimental than physical).
		// These 2 conditions together give us the following solution.
//		rscale = 0.5 * (1.0 + dot);
//		lscale = 0.5 * (1.0 - dot);
		rscale = c::sqrt( 1.0 - vec[1] );
		lscale = c::sqrt( 1.0 + vec[1] );
#endif // RTCW_XX
		//rscale = s_separation->value + ( 1.0 - s_separation->value ) * dot;
		//lscale = s_separation->value - ( 1.0 - s_separation->value ) * dot;

#if !defined RTCW_ET
		if ( rscale < 0 ) {
			rscale = 0;
		}
		if ( lscale < 0 ) {
			lscale = 0;
		}
#else
/*		if ( rscale < 0 ) {
			rscale = 0;
		}
		if ( lscale < 0 ) {
			lscale = 0;
		}*/
#endif // RTCW_XX

	}

	// add in distance effect
	scale = ( 1.0 - dist ) * rscale;
	*right_vol = ( master_vol * scale );
	if ( *right_vol < 0 ) {
		*right_vol = 0;
	}

	scale = ( 1.0 - dist ) * lscale;
	*left_vol = ( master_vol * scale );
	if ( *left_vol < 0 ) {
		*left_vol = 0;
	}
}

/*
====================
S_StartSound

Validates the parms and queues the sound up
if pos is NULL, the sound will be dynamically sourced from the entity
Entchannel 0 will never override a playing sound

  flags:  (currently apply only to non-looping sounds)
	SND_NORMAL			    0	- (default) allow sound to be cut off only by the same sound on this channel
	SND_OKTOCUT			0x001	- allow sound to be cut off by any following sounds on this channel
	SND_REQUESTCUT		0x002	- allow sound to be cut off by following sounds on this channel only for sounds who request cutoff
	SND_CUTOFF			0x004	- cut off sounds on this channel that are marked 'SND_REQUESTCUT'
	SND_CUTOFF_ALL		0x008	- cut off all sounds on this channel
====================
*/

#if defined RTCW_SP
void S_ThreadStartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, int flags );
#elif defined RTCW_ET
void S_ThreadStartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, int flags, int volume );
#endif // RTCW_XX

#if !defined RTCW_ET
void S_StartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, int flags ) {
#else
void S_StartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, int flags, int volume ) {
#endif // RTCW_XX

#if defined RTCW_SP
	if ( !snd.s_soundStarted || snd.s_soundMute || ( cls.state != CA_ACTIVE && cls.state != CA_DISCONNECTED ) ) {
		return;
	}

	// RF, we have lots of NULL sounds using up valuable channels, so just ignore them
	if ( !sfxHandle && entchannel != CHAN_WEAPON ) {  // let null weapon sounds try to play.  they kill any weapon sounds playing when a guy dies
		return;
	}

	// RF, make the call now, or else we could override following streaming sounds in the same frame, due to the delay
	S_ThreadStartSoundEx( origin, entityNum, entchannel, sfxHandle, flags );
/*
	if (snd.tart < MAX_PUSHSTACK) {
		sfx_t		*sfx;
		if (origin) {
			VectorCopy( origin, snd.pushPop[snd.tart].origin );
			snd.pushPop[snd.tart].fixedOrigin = qtrue;
		} else {
			snd.pushPop[snd.tart].fixedOrigin = qfalse;
		}
		snd.pushPop[snd.tart].entityNum = entityNum;
		snd.pushPop[snd.tart].entityChannel = entchannel;
		snd.pushPop[snd.tart].sfx = sfxHandle;
		snd.pushPop[snd.tart].flags = flags;
		sfx = &s_knownSfx[ sfxHandle ];

		if (sfx->inMemory == qfalse) {
			S_memoryLoad(sfx);
		}

		snd.tart++;
	}
*/
#elif defined RTCW_MP
	if ( !s_soundStarted || s_soundMuted || ( cls.state != CA_ACTIVE && cls.state != CA_DISCONNECTED ) ) {
		return;
	}
	if ( tart < MAX_PUSHSTACK ) {
		sfx_t       *sfx;
		if ( origin ) {
			VectorCopy( origin, pushPop[tart].origin );
			pushPop[tart].fixedOrigin = qtrue;
		} else {
			pushPop[tart].fixedOrigin = qfalse;
		}
		pushPop[tart].entityNum = entityNum;
		pushPop[tart].entityChannel = entchannel;
		pushPop[tart].sfx = sfxHandle;
		pushPop[tart].flags = flags;
		sfx = &s_knownSfx[ sfxHandle ];

		if ( sfx->inMemory == qfalse ) {
			S_memoryLoad( sfx );
		}

		tart++;
	}
#else
	if ( !snd.s_soundStarted || snd.s_soundMute || ( cls.state != CA_ACTIVE && cls.state != CA_DISCONNECTED ) ) {
		return;
	}

	// RF, we have lots of NULL sounds using up valuable channels, so just ignore them
/*	if( !sfxHandle && entchannel != CHAN_WEAPON ) {	// let null weapon sounds try to play.  they kill any weapon sounds playing when a guy dies
		Com_Printf( "^1WARNING: NULL sfx handle\n" );
		return;
	}*/

	// RF, make the call now, or else we could override following streaming sounds in the same frame, due to the delay
	S_ThreadStartSoundEx( origin, entityNum, entchannel, sfxHandle, flags, volume );
/*
	if (snd.tart < MAX_PUSHSTACK) {
		sfx_t		*sfx;
		if (origin) {
			VectorCopy( origin, snd.pushPop[snd.tart].origin );
			snd.pushPop[snd.tart].fixedOrigin = qtrue;
		} else {
			snd.pushPop[snd.tart].fixedOrigin = qfalse;
		}
		snd.pushPop[snd.tart].entityNum = entityNum;
		snd.pushPop[snd.tart].entityChannel = entchannel;
		snd.pushPop[snd.tart].sfx = sfxHandle;
		snd.pushPop[snd.tart].flags = flags;
		// Gordon: more volume control, so more cookies from Tim!
		snd.pushPop[snd.tart].volume = volume; // default was 127
		sfx = &s_knownSfx[ sfxHandle ];

		if (sfx->inMemory == qfalse) {
			S_memoryLoad(sfx);
		}

		snd.tart++;
	}
*/
#endif // RTCW_XX

}

#if !defined RTCW_ET
void S_ThreadStartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, int flags ) {
#else
void S_ThreadStartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, int flags, int volume ) {
#endif // RTCW_XX

	channel_t   *ch;
	sfx_t       *sfx;
	int i, oldest, chosen;

	chosen = -1;

#if !defined RTCW_MP
	if ( !snd.s_soundStarted || snd.s_soundMute ) {
#else
	if ( !s_soundStarted || s_soundMuted ) {
#endif // RTCW_XX

		return;
	}

	if ( !origin && ( entityNum < 0 || entityNum > MAX_GENTITIES ) ) {
		Com_Error( ERR_DROP, "S_StartSound: bad entitynum %i", entityNum );
	}

#if defined RTCW_SP
	if ( sfxHandle < 0 || sfxHandle >= snd.s_numSfx ) {
		Com_Printf( S_COLOR_YELLOW, "S_StartSound: handle %i out of range\n", sfxHandle );
#elif defined RTCW_MP
	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_DPrintf( S_COLOR_YELLOW, "S_StartSound: handle %i out of range\n", sfxHandle );
#else
	if ( sfxHandle < 0 || sfxHandle >= snd.s_numSfx ) {
		Com_Printf( S_COLOR_YELLOW "S_StartSound: handle %i out of range\n", sfxHandle );
#endif // RTCW_XX

		return;
	}

	sfx = &s_knownSfx[ sfxHandle ];

	if ( s_show->integer == 1 ) {
		Com_Printf( "%i : %s\n", s_paintedtime, sfx->soundName );
	}

#if !defined RTCW_ET
//	Com_Printf("playing %s\n", sfx->soundName);
#endif // RTCW_XX

	sfx->lastTimeUsed = Sys_Milliseconds();

	// check for a streaming sound that this entity is playing in this channel
	// kill it if it exists
	if ( entityNum >= 0 ) {
		for ( i = 1; i < MAX_STREAMING_SOUNDS; i++ ) {    // track 0 is music/cinematics
			if ( !streamingSounds[i].file ) {
				continue;
			}
			// check to see if this character currently has another sound streaming on the same channel
			if ( ( entchannel != CHAN_AUTO ) && ( streamingSounds[i].entnum >= 0 ) && ( streamingSounds[i].channel == entchannel ) && ( streamingSounds[i].entnum == entityNum ) ) {
				// found a match, override this channel

#if !defined RTCW_MP
				streamingSounds[i].kill = 1;
#else
				streamingSounds[i].kill = qtrue;
#endif // RTCW_XX

				break;
			}
		}
	}

	ch = NULL;

//----(SA)	modified

	// shut off other sounds on this channel if necessary
	for ( i = 0 ; i < MAX_CHANNELS ; i++ ) {
		if ( s_channels[i].entnum == entityNum && s_channels[i].thesfx && s_channels[i].entchannel == entchannel ) {

			// cutoff all on channel
			if ( flags & SND_CUTOFF_ALL ) {
				S_ChannelFree( &s_channels[i] );
				continue;
			}

			if ( s_channels[i].flags & SND_NOCUT ) {
				continue;
			}

#if defined RTCW_SP
			// RF, let client voice sounds be overwritten
			if ( entityNum < MAX_CLIENTS && s_channels[i].entchannel != CHAN_AUTO && s_channels[i].entchannel != CHAN_WEAPON ) {
				S_ChannelFree( &s_channels[i] );
				continue;
			}
#elif defined RTCW_ET
			// RF, let client voice sounds be overwritten

			// TAT 11/2/2002 - don't do this, then if 2 bots try to talk at the same time, the 1st voice goes away.
			//		multiple people are allowed to talk at the same time

			// xkan, 11/21/2002 - restore this for all non-zero clients, for client 0, we sometimes do get (and want)
			// multiple sounds on him.
			//
			/*if(entityNum != 0 &&
			   entityNum < MAX_CLIENTS && s_channels[i].entchannel != CHAN_AUTO && s_channels[i].entchannel != CHAN_WEAPON) {
				S_ChannelFree(&s_channels[i]);
				continue;
			}*/
#endif // RTCW_XX

			// cutoff sounds that expect to be overwritten
			if ( s_channels[i].flags & SND_OKTOCUT ) {
				S_ChannelFree( &s_channels[i] );
				continue;
			}

			// cutoff 'weak' sounds on channel
			if ( flags & SND_CUTOFF ) {
				if ( s_channels[i].flags & SND_REQUESTCUT ) {
					S_ChannelFree( &s_channels[i] );
					continue;
				}
			}

		}
	}

	// re-use channel if applicable
	for ( i = 0 ; i < MAX_CHANNELS ; i++ ) {

#if !defined RTCW_MP
		if ( s_channels[i].entnum == entityNum && s_channels[i].entchannel == entchannel && entchannel != CHAN_AUTO ) {
#else
		if ( s_channels[i].entnum == entityNum && s_channels[i].entchannel == entchannel ) {
#endif // RTCW_XX

			if ( !( s_channels[i].flags & SND_NOCUT ) && s_channels[i].thesfx == sfx ) {
				ch = &s_channels[i];
				break;
			}
		}
	}

	if ( !ch ) {
		ch = S_ChannelMalloc();
	}
//----(SA)	end

	if ( !ch ) {
		ch = s_channels;

		oldest = sfx->lastTimeUsed;
		for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
			if ( ch->entnum == entityNum && ch->thesfx == sfx ) {
				chosen = i;
				break;
			}
			if ( ch->entnum != listener_number && ch->entnum == entityNum && ch->allocTime < oldest && ch->entchannel != CHAN_ANNOUNCER ) {
				oldest = ch->allocTime;
				chosen = i;
			}
		}
		if ( chosen == -1 ) {
			ch = s_channels;
			for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
				if ( ch->entnum != listener_number && ch->allocTime < oldest && ch->entchannel != CHAN_ANNOUNCER ) {
					oldest = ch->allocTime;
					chosen = i;
				}
			}
			if ( chosen == -1 ) {
				if ( ch->entnum == listener_number ) {
					for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
						if ( ch->allocTime < oldest ) {
							oldest = ch->allocTime;
							chosen = i;
						}
					}
				}
				if ( chosen == -1 ) {
					//Com_Printf("dropping sound\n");
					return;
				}
			}
		}
		ch = &s_channels[chosen];
		ch->allocTime = sfx->lastTimeUsed;
	}

#if !defined RTCW_MP
#ifdef _DEBUG
	if ( ch > &s_channels[MAX_CHANNELS] || ch < &s_channels[0] ) { //DAJ	extra check
		Com_DPrintf( "s_channel OUT OF BOUNDS\n" );
		return;
	}
#endif
#endif // RTCW_XX

	if ( origin ) {
		VectorCopy( origin, ch->origin );
		ch->fixed_origin = qtrue;
	} else {
		ch->fixed_origin = qfalse;
	}

	ch->flags = flags;  //----(SA)	added

#if !defined RTCW_ET
	ch->master_vol = 127;
#else
	ch->master_vol = volume;
#endif // RTCW_XX

	ch->entnum = entityNum;
	ch->thesfx = sfx;
	ch->entchannel = entchannel;
	ch->leftvol = ch->master_vol;       // these will get calced at next spatialize
	ch->rightvol = ch->master_vol;      // unless the game isn't running
	ch->doppler = qfalse;

	if ( ch->fixed_origin ) {

#if !defined RTCW_ET
		S_SpatializeOrigin( ch->origin, ch->master_vol, &ch->leftvol, &ch->rightvol, SOUND_RANGE_DEFAULT );
#else
		S_SpatializeOrigin( ch->origin, ch->master_vol, &ch->leftvol, &ch->rightvol, SOUND_RANGE_DEFAULT, flags & SND_NO_ATTENUATION );
#endif // RTCW_XX

	} else {

#if defined RTCW_SP
		S_SpatializeOrigin( snd.entityPositions[ ch->entnum ], ch->master_vol, &ch->leftvol, &ch->rightvol, SOUND_RANGE_DEFAULT );
#elif defined RTCW_MP
		S_SpatializeOrigin( entityPositions[ ch->entnum ], ch->master_vol, &ch->leftvol, &ch->rightvol, SOUND_RANGE_DEFAULT );
#else
		S_SpatializeOrigin( snd.entityPositions[ch->entnum], ch->master_vol, &ch->leftvol, &ch->rightvol, SOUND_RANGE_DEFAULT, flags & SND_NO_ATTENUATION );
#endif // RTCW_XX

	}

	ch->startSample = START_SAMPLE_IMMEDIATE;
	ch->threadReady = qtrue;
}

/*
==============
S_StartSound
==============
*/

#if !defined RTCW_ET
void S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle ) {
	S_StartSoundEx( origin, entityNum, entchannel, sfxHandle, 0 );
}
#else
void S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, int volume ) {
	S_StartSoundEx( origin, entityNum, entchannel, sfxHandle, 0, volume );
}
#endif // RTCW_XX



/*
==================
S_StartLocalSound
==================
*/

#if !defined RTCW_ET
void S_StartLocalSound( sfxHandle_t sfxHandle, int channelNum ) {

#if defined RTCW_SP
	if ( !snd.s_soundStarted || snd.s_soundMute ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= snd.s_numSfx ) {
		Com_Printf( S_COLOR_YELLOW, "S_StartLocalSound: handle %i out of range\n", sfxHandle );
		return;
	}
#elif defined RTCW_MP
	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_DPrintf( S_COLOR_YELLOW, "S_StartLocalSound: handle %i out of range\n", sfxHandle );
		return;
	}
#endif // RTCW_XX

	S_StartSound( NULL, listener_number, channelNum, sfxHandle );
}
#else
void S_StartLocalSound( sfxHandle_t sfxHandle, int channelNum, int volume ) {
	if ( !snd.s_soundStarted || snd.s_soundMute ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= snd.s_numSfx ) {
		Com_Printf( S_COLOR_YELLOW "S_StartLocalSound: handle %i out of range\n", sfxHandle );
		return;
	}

	S_StartSound( NULL, listener_number, channelNum, sfxHandle, volume );
}
#endif // RTCW_XX

/*
==================
S_ClearSoundBuffer

If we are about to perform file access, clear the buffer
so sound doesn't stutter.
==================
*/

#if !defined RTCW_MP
void S_ClearSoundBuffer( qboolean killStreaming ) {
	if ( !snd.s_soundStarted ) {
		return;
	}

	if ( !snd.s_soundPainted ) {  // RF, buffers are clear, no point clearing again
		return;
	}

	snd.s_soundPainted = qfalse;

//	snd.s_clearSoundBuffer = 4;
	snd.s_clearSoundBuffer = 3;

	S_ClearSounds( killStreaming, qtrue );    // do this now since you might not be allowed to in a sec (no multi-threaeded)
}
#else
void S_ClearSoundBuffer( void ) {
	if ( !s_soundStarted ) {
		return;
	}

	if ( !s_soundPainted ) {  // RF, buffers are clear, no point clearing again
		return;
	}

	s_soundPainted = qfalse;

	s_clearSoundBuffer = 4;

	S_Update();         // NERVE - SMF - force an update
}
#endif // RTCW_XX

/*
==================
S_StopAllSounds
==================
*/
void S_StopAllSounds( void ) {
	int i;

#if !defined RTCW_MP
	if ( !snd.s_soundStarted ) {
		return;
	}

//DAJ BUGFIX	for(i=0;i<numStreamingSounds;i++) {

#if !defined RTCW_ET
	for ( i = 0; i < MAX_STREAMING_SOUNDS; i++ ) {   //DAJ numStreamingSounds can get bigger than the MAX array size
		if ( i == 0 ) {
			continue;   // ignore music
		}
#else
	// Arnout: i = 1, as we ignore music
	for ( i = 1; i < MAX_STREAMING_SOUNDS; i++ ) {   //DAJ numStreamingSounds can get bigger than the MAX array size
#endif // RTCW_XX

		streamingSounds[i].kill = 1;
	}

	// stop the background music
	S_StopBackgroundTrack();

	S_ClearSoundBuffer( qtrue );

	S_UpdateThread();   // clear the stuff that needs to clear
#else
	if ( !s_soundStarted ) {
		return;
	}

//DAJ BUGFIX	for(i=0;i<numStreamingSounds;i++) {
	for ( i = 0; i < MAX_STREAMING_SOUNDS; i++ ) {   //DAJ numStreamingSounds can get bigger than the MAX array size
		streamingSounds[i].kill = qtrue;
	}

	// stop the background music
	S_StopBackgroundTrack();

	S_ClearSoundBuffer();
#endif // RTCW_XX

}

/*
==============================================================

continuous looping sounds are added each frame

==============================================================
*/

/*
==================
S_ClearLoopingSounds

==================
*/
void S_ClearLoopingSounds( void ) {

#if defined RTCW_SP
	snd.numLoopSounds = 0;
#elif defined RTCW_MP
	numLoopSounds = 0;
#else
	int i;

	for ( i = 0 ; i < snd.numLoopSounds ; i++ ) {
		snd.loopSounds[i].active = qfalse;
		//%	snd.loopSounds[i].sfx = NULL;
	}
	snd.numLoopSounds = 0;
	numLoopChannels = 0;
#endif // RTCW_XX

}

/*
==================
S_AddLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...

NOTE: 'volume' with underwater bit set stays at set volume underwater
==================
*/

#if !defined RTCW_ET
#define UNDERWATER_BIT  8
#else
#define UNDERWATER_BIT  16
#endif // RTCW_XX

#if !defined RTCW_ET
void S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, const int range, sfxHandle_t sfxHandle, int volume ) {
#else
void S_AddLoopingSound( const vec3_t origin, const vec3_t velocity, const int range, sfxHandle_t sfxHandle, int volume, int soundTime ) {
#endif // RTCW_XX

	sfx_t *sfx;

#if !defined RTCW_MP
	if ( !snd.s_soundStarted || snd.s_soundMute || cls.state != CA_ACTIVE ) {
		return;
	}
	if ( snd.numLoopSounds >= MAX_LOOP_SOUNDS ) {
#else
	if ( !s_soundStarted || s_soundMuted || cls.state != CA_ACTIVE ) {
		return;
	}
	if ( numLoopSounds >= MAX_LOOP_SOUNDS ) {
#endif // RTCW_XX

		return;
	}
	if ( !volume ) {
		return;
	}

#if !defined RTCW_MP
	if ( sfxHandle < 0 || sfxHandle >= snd.s_numSfx ) {
#else
	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
#endif // RTCW_XX

		Com_Error( ERR_DROP, "S_AddLoopingSound: handle %i out of range", sfxHandle );

#if defined RTCW_ET
		return;
#endif // RTCW_XX

	}

	sfx = &s_knownSfx[ sfxHandle ];

	if ( sfx->inMemory == qfalse ) {
		S_memoryLoad( sfx );
	}

	if ( !sfx->soundLength ) {
		Com_Error( ERR_DROP, "%s has length 0", sfx->soundName );
	}

#if defined RTCW_ET
	// ydnar: allow looped sounds to start when initially triggered, rather than in the middle of the sample
	snd.loopSounds[ snd.numLoopSounds ].startSample = sfx->soundLength
													  ? ( ( s_khz->integer * soundTime ) - s_paintedtime ) % sfx->soundLength
													  : 0;

	snd.loopSounds[ snd.numLoopSounds ].startSample = sfx->soundLength
													  ?  s_paintedtime - ( (int) ( dma.speed * ( cl.serverTime - soundTime ) / 1000.0f ) )
													  : 0;

	snd.loopSounds[ snd.numLoopSounds ].startSample = soundTime % sfx->soundLength;
#endif // RTCW_XX

#if !defined RTCW_MP
	VectorCopy( origin, snd.loopSounds[snd.numLoopSounds].origin );
	VectorCopy( velocity, snd.loopSounds[snd.numLoopSounds].velocity );

#if defined RTCW_ET
	snd.loopSounds[snd.numLoopSounds].active = qtrue;
	snd.loopSounds[snd.numLoopSounds].doppler = qfalse;
	snd.loopSounds[snd.numLoopSounds].oldDopplerScale = 1.0;
	snd.loopSounds[snd.numLoopSounds].dopplerScale = 1.0;
#endif // RTCW_XX

	snd.loopSounds[snd.numLoopSounds].sfx = sfx;
	if ( range ) {
		snd.loopSounds[snd.numLoopSounds].range = range;
	} else {
		snd.loopSounds[snd.numLoopSounds].range = SOUND_RANGE_DEFAULT;
	}

	if ( volume & 1 << UNDERWATER_BIT ) {
		snd.loopSounds[snd.numLoopSounds].loudUnderWater = qtrue;
	}
#else
	VectorCopy( origin, loopSounds[numLoopSounds].origin );
	VectorCopy( velocity, loopSounds[numLoopSounds].velocity );
	loopSounds[numLoopSounds].sfx = sfx;
	if ( range ) {
		loopSounds[numLoopSounds].range = range;
	} else {
		loopSounds[numLoopSounds].range = SOUND_RANGE_DEFAULT;
	}

	if ( volume & 1 << UNDERWATER_BIT ) {
		loopSounds[numLoopSounds].loudUnderWater = qtrue;
	}
#endif // RTCW_XX

#if !defined RTCW_ET
	if ( volume > 255 ) {
		volume = 255;
#else
	if ( volume > 65535 ) {
		volume = 65535;
#endif // RTCW_XX

	} else if ( volume < 0 ) {
		volume = 0;
	}

#if defined RTCW_SP
	snd.loopSounds[snd.numLoopSounds].vol = (int)( (float)volume * snd.volCurrent );  //----(SA)	modified

	snd.numLoopSounds++;
#elif defined RTCW_MP
	loopSounds[numLoopSounds].vol = volume;

	numLoopSounds++;
#else
	snd.loopSounds[snd.numLoopSounds].vol = (int)( (float)volume * snd.volCurrent );  //----(SA)	modified
#endif // RTCW_XX

#if defined RTCW_ET
	if ( s_doppler->integer && VectorLengthSquared( velocity ) > 0.0 ) {
		vec3_t out;
		float lena, lenb;

		snd.loopSounds[snd.numLoopSounds].doppler = qtrue;
		lena = DistanceSquared( snd.entityPositions[listener_number], snd.loopSounds[snd.numLoopSounds].origin );
		VectorAdd( snd.loopSounds[snd.numLoopSounds].origin, snd.loopSounds[snd.numLoopSounds].velocity, out );
		lenb = DistanceSquared( snd.entityPositions[listener_number], out );
		if ( ( snd.loopSounds[snd.numLoopSounds].framenum + 1 ) != cls.framecount ) {
			snd.loopSounds[snd.numLoopSounds].oldDopplerScale = 1.0;
		} else {
			snd.loopSounds[snd.numLoopSounds].oldDopplerScale = snd.loopSounds[snd.numLoopSounds].dopplerScale;
		}
		snd.loopSounds[snd.numLoopSounds].dopplerScale = lenb / ( lena * 100 );
		if ( snd.loopSounds[snd.numLoopSounds].dopplerScale <= 1.0 ) {
			snd.loopSounds[snd.numLoopSounds].doppler = qfalse;         // don't bother doing the math
		}
	}

	snd.loopSounds[snd.numLoopSounds].framenum = cls.framecount;
	snd.numLoopSounds++;
#endif // RTCW_XX
}

#if defined RTCW_ET
/*
==================
S_AddLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/
void S_AddRealLoopingSound( const vec3_t origin, const vec3_t velocity, const int range, sfxHandle_t sfxHandle, int volume, int soundTime ) {
	sfx_t *sfx;

	if ( !snd.s_soundStarted || snd.s_soundMute ) {
		return;
	}

	if ( snd.numLoopSounds >= MAX_LOOP_SOUNDS ) {
		return;
	}

	if ( !volume ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= snd.s_numSfx ) {
		Com_Printf( S_COLOR_YELLOW "S_AddRealLoopingSound: handle %i out of range\n", sfxHandle );
		return;
	}

	sfx = &s_knownSfx[ sfxHandle ];

	if ( sfx->inMemory == qfalse ) {
		S_memoryLoad( sfx );
	}

	if ( !sfx->soundLength ) {
		Com_Error( ERR_DROP, "%s has length 0", sfx->soundName );
	}

	// ydnar: allow looped sounds to start when initially triggered, rather than in the middle of the sample
	/*snd.loopSounds[ snd.numLoopSounds ].startSample = sfx->soundLength
		? ((s_khz->integer * soundTime) - s_paintedtime) % sfx->soundLength
		: 0;

	snd.loopSounds[ snd.numLoopSounds ].startSample = sfx->soundLength
		?  s_paintedtime - ((int) (dma.speed * (cl.serverTime - soundTime) / 1000.0f))
		: 0;

	snd.loopSounds[ snd.numLoopSounds ].startSample = soundTime % sfx->soundLength;*/

	VectorCopy( origin, snd.loopSounds[snd.numLoopSounds].origin );
	VectorCopy( velocity, snd.loopSounds[snd.numLoopSounds].velocity );
	snd.loopSounds[snd.numLoopSounds].sfx = sfx;
	snd.loopSounds[snd.numLoopSounds].active = qtrue;
	snd.loopSounds[snd.numLoopSounds].doppler = qfalse;

	if ( range ) {
		snd.loopSounds[snd.numLoopSounds].range = range;
	} else {
		snd.loopSounds[snd.numLoopSounds].range = SOUND_RANGE_DEFAULT;
	}

	if ( volume & 1 << UNDERWATER_BIT ) {
		snd.loopSounds[snd.numLoopSounds].loudUnderWater = qtrue;
	}

	if ( volume > 65535 ) {
		volume = 65535;
	} else if ( volume < 0 ) {
		volume = 0;
	}

	snd.loopSounds[snd.numLoopSounds].vol = (int)( (float)volume * snd.volCurrent );  //----(SA)	modified
	snd.numLoopSounds++;
}
#endif // RTCW_XX

/*
==================
S_AddLoopSounds

Spatialize all of the looping sounds.
All sounds are on the same cycle, so any duplicates can just
sum up the channel multipliers.
==================
*/
void S_AddLoopSounds( void ) {
	int i, j, time;
	int left_total, right_total, left, right;
	channel_t   *ch;
	loopSound_t *loop, *loop2;
	static int loopFrame;

#if defined RTCW_ET
	if ( !snd.s_soundStarted || ( snd.s_soundMute == 1 ) ) {
//		Com_DPrintf ("not started or muted\n");
		return;
	}
#endif // RTCW_XX

	numLoopChannels = 0;

	time = Sys_Milliseconds();

	loopFrame++;

#if !defined RTCW_MP
	for ( i = 0 ; i < snd.numLoopSounds ; i++ ) {
		loop = &snd.loopSounds[i];
#else
	for ( i = 0 ; i < numLoopSounds ; i++ ) {
		loop = &loopSounds[i];
#endif // RTCW_XX

#if !defined RTCW_ET
		if ( loop->mergeFrame == loopFrame ) {
#else
		if ( !loop->active || loop->mergeFrame == loopFrame ) {
#endif // RTCW_XX

			continue;   // already merged into an earlier sound
		}

#if !defined RTCW_ET
		//if (loop->kill) {
		//	S_SpatializeOrigin( loop->origin, 127, &left_total, &right_total, loop->range);	// 3d
		//} else {
		S_SpatializeOrigin( loop->origin, 90,  &left_total, &right_total, loop->range );    // sphere
		//}
#else
		if ( loop->kill ) {
			S_SpatializeOrigin( loop->origin, 127, &left_total, &right_total, loop->range, qfalse );         // 3d
		} else {
			S_SpatializeOrigin( loop->origin, 90,  &left_total, &right_total, loop->range, qfalse );         // sphere
		}
#endif // RTCW_XX

		// adjust according to volume
		left_total = (int)( (float)loop->vol * (float)left_total / 256.0 );
		right_total = (int)( (float)loop->vol * (float)right_total / 256.0 );

		loop->sfx->lastTimeUsed = time;

#if !defined RTCW_ET
		for ( j = ( i + 1 ); j < numLoopChannels ; j++ ) {
#else
		for ( j = ( i + 1 ); j < MAX_GENTITIES ; j++ ) {
#endif // RTCW_XX

#if !defined RTCW_MP
			loop2 = &snd.loopSounds[j];
#else
			loop2 = &loopSounds[j];
#endif // RTCW_XX

#if !defined RTCW_ET
			if ( loop2->sfx != loop->sfx ) {
#else
			if ( !loop2->active || loop2->doppler || loop2->sfx != loop->sfx ||
				 loop2->startSample != loop->startSample ) { // ydnar
#endif // RTCW_XX

				continue;
			}
			loop2->mergeFrame = loopFrame;

#if !defined RTCW_ET
			//if (loop2->kill) {
			//	S_SpatializeOrigin( loop2->origin, 127, &left, &right, loop2->range);	// 3d
			//} else {
			S_SpatializeOrigin( loop2->origin, 90,  &left, &right, loop2->range );      // sphere
			//}
#else
			if ( loop2->kill ) {
				S_SpatializeOrigin( loop2->origin, 127, &left, &right, loop2->range, qfalse );               // 3d
			} else {
				S_SpatializeOrigin( loop2->origin, 90,  &left, &right, loop2->range, qfalse );               // sphere
			}
#endif // RTCW_XX

			// adjust according to volume
			left = (int)( (float)loop2->vol * (float)left / 256.0 );
			right = (int)( (float)loop2->vol * (float)right / 256.0 );

			loop2->sfx->lastTimeUsed = time;
			left_total += left;
			right_total += right;
		}
		if ( left_total == 0 && right_total == 0 ) {
			continue;       // not audible
		}

		// allocate a channel
		ch = &loop_channels[numLoopChannels];

		if ( left_total > 255 ) {
			left_total = 255;
		}
		if ( right_total > 255 ) {
			right_total = 255;
		}

		ch->master_vol = 127;
		ch->leftvol = left_total;
		ch->rightvol = right_total;
		ch->thesfx = loop->sfx;

#if defined RTCW_ET
		ch->doppler = loop->doppler;
		ch->dopplerScale = loop->dopplerScale;
		ch->oldDopplerScale = loop->oldDopplerScale;

		// ydnar: allow offsetting of sound samples
		ch->startSample = loop->startSample;
#endif // RTCW_XX

#if !defined RTCW_ET
		// RF, disabled doppler for looping sounds for now, since we are reverting to the old looping sound code
		ch->doppler = qfalse;
#endif // RTCW_XX

#if defined RTCW_SP
		//ch->doppler = loop->doppler;
		//ch->dopplerScale = loop->dopplerScale;
		//ch->oldDopplerScale = loop->oldDopplerScale;
#endif // RTCW_XX

		numLoopChannels++;
		if ( numLoopChannels == MAX_CHANNELS ) {

#if !defined RTCW_MP
			i = snd.numLoopSounds + 1;
#else
			i = numLoopSounds + 1;
#endif // RTCW_XX

		}
	}
}

//=============================================================================

/*
=================
S_ByteSwapRawSamples

If raw data has been loaded in little endien binary form, this must be done.
If raw data was calculated, as with ADPCM, this should not be called.
=================
*/
//DAJ  void S_ByteSwapRawSamples( int samples, int width, int s_channels, const byte *data ) {
void S_ByteSwapRawSamples( int samples, int width, int s_channels, short *data ) {
	int i;

	if ( width != 2 ) {
		return;
	}
#ifndef __MACOS__   //DAJ save this test
	if (!rtcw::Endian::is_little ()) {
		return;
	}
#endif
	//DAJ use a faster loop technique
	if ( s_channels == 2 ) {
		i = samples << 1;
	} else {
		i = samples;
	}

	do {
		rtcw::Endian::lei(*data++);
//DAJ		((short *)data)[i] = LittleShort( ((short *)data)[i] );
	} while ( --i );
}

/*
============
S_GetRawSamplePointer
============
*/
portable_samplepair_t *S_GetRawSamplePointer() {
	return s_rawsamples[0];
}

/*
============
S_RawSamples

Music streaming
============
*/
void S_RawSamples( int samples, int rate, int width, int s_channels, const byte *data, float lvol, float rvol, int streamingIndex ) {
	int i;
	int src, dst;
	float scale;
	int intVolumeL, intVolumeR;

#if !defined RTCW_MP
	if ( !snd.s_soundStarted || ( snd.s_soundMute == 1 ) ) {
#else
	if ( !s_soundStarted || s_soundMuted ) {
#endif // RTCW_XX

		return;
	}

	// volume taken into account when mixed
	s_rawVolume[streamingIndex].left = 256 * lvol;
	s_rawVolume[streamingIndex].right = 256 * rvol;

	intVolumeL = 256;
	intVolumeR = 256;

	if ( s_rawend[streamingIndex] < s_soundtime ) {

#if defined RTCW_SP
		Com_DPrintf( "S_RawSamples: resetting minumum: %i\n",s_soundtime - s_rawend[streamingIndex] );
#elif defined RTCW_MP
		Com_DPrintf( "S_RawSamples: resetting minimum: %i < %i\n", s_rawend[streamingIndex], s_soundtime );
#else
		Com_DPrintf( "S_RawSamples: resetting minimum: %i < %i (%i)\n", s_rawend[streamingIndex], s_soundtime, s_soundtime - s_rawend[streamingIndex] );
#endif // RTCW_XX

		s_rawend[streamingIndex] = s_soundtime;
	}

	scale = (float)rate / dma.speed;

#if !defined RTCW_SP
	//Com_Printf ("%i < %i < %i\n", s_soundtime, s_paintedtime, s_rawend);
#endif // RTCW_XX

	if ( s_channels == 2 && width == 2 ) {
		if ( scale == 1.0 ) { // optimized case
			for ( i = 0; i < samples; i++ )
			{
				dst = s_rawend[streamingIndex] & ( MAX_RAW_SAMPLES - 1 );
				s_rawend[streamingIndex]++;
				s_rawsamples[streamingIndex][dst].left = ( (short *)data )[i * 2] * intVolumeL;
				s_rawsamples[streamingIndex][dst].right = ( (short *)data )[i * 2 + 1] * intVolumeR;
			}
		} else
		{
			for ( i = 0; ; i++ )
			{
				src = i * scale;
				if ( src >= samples ) {
					break;
				}
				dst = s_rawend[streamingIndex] & ( MAX_RAW_SAMPLES - 1 );
				s_rawend[streamingIndex]++;
				s_rawsamples[streamingIndex][dst].left = ( (short *)data )[src * 2] * intVolumeL;
				s_rawsamples[streamingIndex][dst].right = ( (short *)data )[src * 2 + 1] * intVolumeR;
			}
		}
	} else if ( s_channels == 1 && width == 2 )     {
		for ( i = 0; ; i++ )
		{
			src = i * scale;
			if ( src >= samples ) {
				break;
			}
			dst = s_rawend[streamingIndex] & ( MAX_RAW_SAMPLES - 1 );
			s_rawend[streamingIndex]++;
			s_rawsamples[streamingIndex][dst].left = ( (short *)data )[src] * intVolumeL;
			s_rawsamples[streamingIndex][dst].right = ( (short *)data )[src] * intVolumeR;
		}
	} else if ( s_channels == 2 && width == 1 )     {
		intVolumeL *= 256;
		intVolumeR *= 256;

		for ( i = 0 ; ; i++ )
		{
			src = i * scale;
			if ( src >= samples ) {
				break;
			}
			dst = s_rawend[streamingIndex] & ( MAX_RAW_SAMPLES - 1 );
			s_rawend[streamingIndex]++;
			s_rawsamples[streamingIndex][dst].left = ( (char *)data )[src * 2] * intVolumeL;
			s_rawsamples[streamingIndex][dst].right = ( (char *)data )[src * 2 + 1] * intVolumeR;
		}
	} else if ( s_channels == 1 && width == 1 )     {
		intVolumeL *= 256;
		intVolumeR *= 256;

		for ( i = 0; ; i++ )
		{
			src = i * scale;
			if ( src >= samples ) {
				break;
			}
			dst = s_rawend[streamingIndex] & ( MAX_RAW_SAMPLES - 1 );
			s_rawend[streamingIndex]++;
			s_rawsamples[streamingIndex][dst].left = ( ( (byte *)data )[src] - 128 ) * intVolumeL;
			s_rawsamples[streamingIndex][dst].right = ( ( (byte *)data )[src] - 128 ) * intVolumeR;
		}
	}

#if defined RTCW_SP
	if ( s_rawend[streamingIndex] > ( s_soundtime + MAX_RAW_SAMPLES ) ) {
//		Com_DPrintf( "S_RawSamples: overflowed %i\n", s_rawend[streamingIndex]-(s_soundtime+ MAX_RAW_SAMPLES) );
#elif defined RTCW_MP
	if ( s_rawend[streamingIndex] > s_soundtime + MAX_RAW_SAMPLES ) {
		Com_DPrintf( "S_RawSamples: overflowed %i > %i\n", s_rawend[streamingIndex], s_soundtime );
#else
	if ( s_rawend[streamingIndex] > ( s_soundtime + MAX_RAW_SAMPLES ) ) {
		Com_DPrintf( "S_RawSamples: overflowed %i > %i (%i)\n",  s_rawend[streamingIndex], s_soundtime + MAX_RAW_SAMPLES, s_rawend[streamingIndex] - ( s_soundtime + MAX_RAW_SAMPLES ) );
#endif // RTCW_XX

	}
}

//=============================================================================

/*
=====================
S_UpdateEntityPosition

let the sound system know where an entity currently is
======================
*/
void S_UpdateEntityPosition( int entityNum, const vec3_t origin ) {
	if ( entityNum < 0 || entityNum > MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "S_UpdateEntityPosition: bad entitynum %i", entityNum );
	}

#if !defined RTCW_MP
	VectorCopy( origin, snd.entityPositions[entityNum] );
#else
	VectorCopy( origin, entityPositions[entityNum] );
#endif // RTCW_XX

}


/*
============
S_Respatialize

Change the volumes of all the playing sounds for changes in their positions
============
*/
void S_Respatialize( int entityNum, const vec3_t head, vec3_t axis[3], int inwater ) {

#if !defined RTCW_MP
	if ( !snd.s_soundStarted || ( snd.s_soundMute == 1 ) ) {
#else
	if ( !s_soundStarted || s_soundMuted ) {
#endif // RTCW_XX

		return;
	}

	listener_number = entityNum;
	VectorCopy( head, listener_origin );
	VectorCopy( axis[0], listener_axis[0] );
	VectorCopy( axis[1], listener_axis[1] );
	VectorCopy( axis[2], listener_axis[2] );
}

void S_ThreadRespatialize() {
	int i;
	channel_t   *ch;
	vec3_t origin;
	// update spatialization for dynamic sounds
	ch = s_channels;
	for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
		if ( !ch->thesfx ) {
			continue;
		}
		// anything coming from the view entity will always be full volume
		if ( ch->entnum == listener_number ) {
			ch->leftvol = ch->master_vol;
			ch->rightvol = ch->master_vol;
		} else {
			if ( ch->fixed_origin ) {
				VectorCopy( ch->origin, origin );
			} else {

#if !defined RTCW_MP
				VectorCopy( snd.entityPositions[ ch->entnum ], origin );
#else
				VectorCopy( entityPositions[ ch->entnum ], origin );
#endif // RTCW_XX

			}

#if !defined RTCW_ET
			S_SpatializeOrigin( origin, ch->master_vol, &ch->leftvol, &ch->rightvol, SOUND_RANGE_DEFAULT );
#else
			S_SpatializeOrigin( origin, ch->master_vol, &ch->leftvol, &ch->rightvol, SOUND_RANGE_DEFAULT, ch->flags & SND_NO_ATTENUATION );
#endif // RTCW_XX

		}
	}
}

/*
========================
S_ScanChannelStarts

Returns qtrue if any new sounds were started since the last mix
========================
*/
qboolean S_ScanChannelStarts( void ) {
	channel_t       *ch;
	int i;
	qboolean newSamples;

	newSamples = qfalse;
	ch = s_channels;

	for ( i = 0; i < MAX_CHANNELS; i++, ch++ ) {
		if ( !ch->thesfx ) {
			continue;
		}
		// if this channel was just started this frame,
		// set the sample count to it begins mixing
		// into the very first sample
		if ( ch->startSample == START_SAMPLE_IMMEDIATE && ch->threadReady == qtrue ) {
			ch->startSample = s_paintedtime;
			newSamples = qtrue;
			continue;
		}

		// if it is completely finished by now, clear it
		if ( ch->startSample + ( ch->thesfx->soundLength ) <= s_paintedtime ) {

#if !defined RTCW_MP
//----(SA)	got from TA sound.  correct?
//			Com_Memset(ch, 0, sizeof(*ch));
#endif // RTCW_XX

			S_ChannelFree( ch );
		}
	}

	return newSamples;
}

#if !defined RTCW_MP
/*
==============
S_CheckForQueuedMusic
==============
*/
int S_CheckForQueuedMusic( void ) {
	streamingSound_t *ss;
	char *nextMusicVA;

	if ( !snd.nextMusicTrack[0] ) {                                    // we didn't actually care about the length
		return 0;
	}

	nextMusicVA = va( "%s", snd.nextMusicTrack );

	ss = &streamingSounds[0];

	if ( snd.nextMusicTrackType == QUEUED_PLAY_ONCE_SILENT ) {
		// do nothing.  current music is dead, don't start another
	} else if ( snd.nextMusicTrackType == QUEUED_PLAY_ONCE ) {
		S_StartBackgroundTrack( nextMusicVA, ss->name, 0 );      // play once, then go back to looping what's currently playing
	} else {        // QUEUED_PLAY_LOOPED
		S_StartBackgroundTrack( nextMusicVA, nextMusicVA, 0 );   // take over
	}

	snd.nextMusicTrackType = 0;     // clear out music queue

#if !defined RTCW_ET
//	snd.nextMusicTrack[0] = 0;		// clear out music queue
#else
	snd.nextMusicTrack[0] = 0;      // clear out music queue
#endif // RTCW_XX

	return 1;
}
#endif // RTCW_XX

#if defined RTCW_ET
/*
============
S_Update

Called once each time through the main loop
============
*/

void S_Update_Debug( void ) {
	int i;
	int total;
	channel_t   *ch;

	if ( !snd.s_soundStarted || ( snd.s_soundMute == 1 ) ) {
//		Com_DPrintf ("not started or muted\n");
		return;
	}

	if ( s_show->integer == 2 ) {
		total = 0;
		ch = s_channels;
		for ( i = 0; i < MAX_CHANNELS; i++, ch++ ) {
			if ( ch->thesfx && ( ch->leftvol || ch->rightvol ) ) {
				Com_Printf( "%i %i %s\n", ch->leftvol, ch->rightvol, ch->thesfx->soundName );          // <- this is not thread safe
				total++;
			}
		}

		Com_Printf( "----(%i)---- painted: %i\n", total, s_paintedtime );
	}
}
#endif // RTCW_XX

#if !defined RTCW_ET
/*
============
S_Update

Called once each time through the main loop
============
*/
#endif // RTCW_XX

void S_Update( void ) {

#if !defined RTCW_ET
	int i;
	int total;
	channel_t   *ch;
#endif // RTCW_XX

#if !defined RTCW_MP
	if ( !snd.s_soundStarted || ( snd.s_soundMute == 1 ) ) {
#else
	if ( !s_soundStarted || s_soundMuted ) {
#endif // RTCW_XX

//		Com_DPrintf ("not started or muted\n");
		return;
	}

#if !defined RTCW_ET
	//
	// debugging output
	//
	if ( s_show->integer == 2 ) {
		total = 0;
		ch = s_channels;
		for ( i = 0; i < MAX_CHANNELS; i++, ch++ ) {
			if ( ch->thesfx && ( ch->leftvol || ch->rightvol ) ) {

#if defined RTCW_SP
				Com_Printf( "%f %f %s\n", ch->leftvol, ch->rightvol, ch->thesfx->soundName );          // <- this is not thread safe
#elif defined RTCW_MP
				Com_DPrintf( "%i %i %s\n", ch->leftvol, ch->rightvol, ch->thesfx->soundName );         // <- this is not thread safe
#endif // RTCW_XX

				total++;
			}
		}

		Com_Printf( "----(%i)---- painted: %i\n", total, s_paintedtime );
	}
#endif // RTCW_XX

	// add loopsounds
	S_AddLoopSounds();

#if !defined RTCW_SP
	// do all the rest
#endif // RTCW_XX

	S_UpdateThread();
}

#if !defined RTCW_MP
/*
==============
S_ClearSounds
==============
*/
void S_ClearSounds( qboolean clearStreaming, qboolean clearMusic ) {
	int clear;
	int i;
	channel_t   *ch;
	streamingSound_t *ss;

	// stop looping sounds

#if !defined RTCW_ET
	S_ClearLoopingSounds();
#else
	Com_Memset( snd.loopSounds, 0, MAX_GENTITIES * sizeof( loopSound_t ) );
	Com_Memset( loop_channels, 0, MAX_CHANNELS * sizeof( channel_t ) );
	numLoopChannels = 0;
#endif // RTCW_XX

	// RF, moved this up so streaming sounds dont get updated with the music, below, and leave us with a snippet off streaming sounds after we reload
	if ( clearStreaming ) {    // we don't want to stop guys with long dialogue from getting cut off by a file read
		// RF, clear talking amplitudes
		Com_Memset( s_entityTalkAmplitude, 0, sizeof( s_entityTalkAmplitude ) );

		for ( i = 0, ss = streamingSounds; i < MAX_STREAMING_SOUNDS; i++, ss++ ) {
			if ( i > 0 || clearMusic ) {
				s_rawend[i] = 0;
				ss->kill = 2;   // get rid of it next sound update
			}
		}

		// RF, we should also kill all channels, since we are killing streaming sounds anyway (fixes siren in forest playing after a map_restart/loadgame
		ch = s_channels;
		for ( i = 0; i < MAX_CHANNELS; i++, ch++ ) {
			if ( ch->thesfx ) {
				S_ChannelFree( ch );
			}
		}

	}

	if ( !clearMusic ) {
		S_UpdateStreamingSounds();  //----(SA)	added so music will get updated if not cleared
	} else {
		// music cleanup
		snd.nextMusicTrack[0] = 0;
		snd.nextMusicTrackType = 0;
	}

	if ( clearStreaming && clearMusic ) {
		if ( dma.samplebits == 8 ) {
			clear = 0x80;
		} else {
			clear = 0;
		}

		SNDDMA_BeginPainting();
		if ( dma.buffer ) {

#if !defined RTCW_ET
			Com_Memset( dma.buffer, clear, dma.samples * dma.samplebits / 8 );
#else
			// TTimo: due to a particular bug workaround in linux sound code,
			//   have to optionally use a custom C implementation of Com_Memset
			//   not affecting win32, we have #define Snd_Memset Com_Memset
			// show_bug.cgi?id=371
			Snd_Memset( dma.buffer, clear, dma.samples * dma.samplebits / 8 );
#endif // RTCW_XX
		}
		SNDDMA_Submit();

#if defined RTCW_ET
		// NERVE - SMF - clear out channels so they don't finish playing when audio restarts
		S_ChannelSetup();
#endif // RTCW_XX
	}
}
#endif // RTCW_XX

#if !defined RTCW_MP
/*
==============
S_UpdateThread
==============
*/
#endif // RTCW_XX

void S_UpdateThread( void ) {

#if !defined RTCW_MP
	if ( !snd.s_soundStarted || ( snd.s_soundMute == 1 ) ) {
#else
	if ( !s_soundStarted || s_soundMuted ) {
#endif // RTCW_XX

//		Com_DPrintf ("not started or muted\n");
		return;
	}

#ifdef TALKANIM
	// default to ZERO amplitude, overwrite if sound is playing
	memset( s_entityTalkAmplitude, 0, sizeof( s_entityTalkAmplitude ) );
#endif

#if !defined RTCW_MP
	if ( snd.s_clearSoundBuffer ) {
		S_ClearSounds( qtrue, (qboolean)( snd.s_clearSoundBuffer >= 4 ) );    //----(SA)	modified
		snd.s_clearSoundBuffer = 0;
	} else {
		S_ThreadRespatialize();
		// add raw data from streamed samples
		S_UpdateStreamingSounds();
		// mix some sound
		S_Update_Mix();
	}
#else
	if ( s_clearSoundBuffer ) {
		int clear;
		int i;

		// stop looping sounds
		S_ClearLoopingSounds();

		for ( i = 0; i < MAX_STREAMING_SOUNDS; i++ ) {
			s_rawend[i] = 0;
		}

		if ( dma.samplebits == 8 ) {
			clear = 0x80;
		} else {
			clear = 0;
		}

		SNDDMA_BeginPainting();
		if ( dma.buffer ) {
			// TTimo: due to a particular bug workaround in linux sound code,
			//   have to optionally use a custom C implementation of Com_Memset
			//   not affecting win32, we have #define Snd_Memset Com_Memset
			// show_bug.cgi?id=371
			Snd_Memset( dma.buffer, clear, dma.samples * dma.samplebits / 8 );
		}
		SNDDMA_Submit();
		s_clearSoundBuffer = 0;

		// NERVE - SMF - clear out channels so they don't finish playing when audio restarts
		S_ChannelSetup();
	} else {
		S_ThreadRespatialize();
		// add raw data from streamed samples
		S_UpdateStreamingSounds();
		// mix some sound
		S_Update_Mix();
	}
#endif // RTCW_XX

}
/*
============
S_GetSoundtime
============
*/
void S_GetSoundtime( void ) {
	int samplepos;
	static int buffers;
	static int oldsamplepos;
	int fullsamples;

	fullsamples = dma.samples / dma.channels;

	// it is possible to miscount buffers if it has wrapped twice between
	// calls to S_Update.  Oh well.
	samplepos = SNDDMA_GetDMAPos();
	if ( samplepos < oldsamplepos ) {
		buffers++;                  // buffer wrapped

		if ( s_paintedtime > 0x40000000 ) { // time to chop things off to avoid 32 bit limits
			buffers = 0;
			s_paintedtime = fullsamples;
			S_StopAllSounds();
		}
	}
	oldsamplepos = samplepos;

	s_soundtime = buffers * fullsamples + samplepos / dma.channels;

#if 0
// check to make sure that we haven't overshot
	if ( s_paintedtime < s_soundtime ) {
		Com_DPrintf( "S_GetSoundtime : overflow\n" );
		s_paintedtime = s_soundtime;
	}
#endif

	if ( dma.submission_chunk < 256 ) {
		s_paintedtime = s_soundtime + s_mixPreStep->value * dma.speed;
	} else {
		s_paintedtime = s_soundtime + dma.submission_chunk;
	}
}

/*
============
S_Update_Mix
============
*/
void S_Update_Mix( void ) {
	unsigned endtime;

#if !defined RTCW_MP
	int samps;            //, i;
#else
	int samps, i;
#endif // RTCW_XX

	static float lastTime = 0.0f;
	float ma, op;
	float thisTime, sane;

#if defined RTCW_ET
	static int ot = -1;
#endif // RTCW_XX

#if defined RTCW_SP
	if ( !snd.s_soundStarted || ( snd.s_soundMute == 1 ) ) {
#elif defined RTCW_MP
	if ( !s_soundStarted || s_soundMuted ) {
#else
	if ( !snd.s_soundStarted || snd.s_soundMute ) {
#endif // RTCW_XX

		return;
	}

#if !defined RTCW_MP
	// RF, this isn't used anymore, since it was causing timing problems with streaming sounds, since the
	// starting of the sound is delayed, it could cause streaming sounds to be cutoff, when the steaming sound was issued after
	// this sound
/*
	for(i=0;i<snd.tart;i++) {
		if (snd.pushPop[i].fixedOrigin) {
			S_ThreadStartSoundEx(snd.pushPop[i].origin, snd.pushPop[i].entityNum, snd.pushPop[i].entityChannel, snd.pushPop[i].sfx, snd.pushPop[i].flags );
		} else {
			S_ThreadStartSoundEx(NULL, snd.pushPop[i].entityNum, snd.pushPop[i].entityChannel, snd.pushPop[i].sfx, snd.pushPop[i].flags );
		}
	}

	snd.tart = 0;
*/
	snd.s_soundPainted = qtrue;
#else
	for ( i = 0; i < tart; i++ ) {
		if ( pushPop[i].fixedOrigin ) {
			S_ThreadStartSoundEx( pushPop[i].origin, pushPop[i].entityNum, pushPop[i].entityChannel, pushPop[i].sfx, pushPop[i].flags );
		} else {
			S_ThreadStartSoundEx( NULL, pushPop[i].entityNum, pushPop[i].entityChannel, pushPop[i].sfx, pushPop[i].flags );
		}
	}

	tart = 0;

	s_soundPainted = qtrue;
#endif // RTCW_XX

	thisTime = Sys_Milliseconds();

	// Updates s_soundtime
	S_GetSoundtime();

#if defined RTCW_ET
	if ( s_soundtime == ot ) {
		return;
	}
	ot = s_soundtime;
#endif // RTCW_XX

	// clear any sound effects that end before the current time,
	// and start any new sounds
	S_ScanChannelStarts();

	sane = thisTime - lastTime;
	if ( sane < 11 ) {
		sane = 11;          // 85hz
	}

	ma = s_mixahead->value * dma.speed;
	op = s_mixPreStep->value + sane * dma.speed * 0.01;

	if ( op < ma ) {
		ma = op;
	}

	// mix ahead of current position
	endtime = s_soundtime + ma;

	// mix to an even submission block size
	endtime = ( endtime + dma.submission_chunk - 1 )
			  & ~( dma.submission_chunk - 1 );

	// never mix more than the complete buffer
	samps = dma.samples >> ( dma.channels - 1 );
	if ( endtime - s_soundtime > samps ) {
		endtime = s_soundtime + samps;
	}

#if !defined RTCW_MP
//----(SA)	added
	// global volume fading

	// endtime or s_paintedtime or s_soundtime...
	if ( s_soundtime < snd.volTime2 ) {    // still has fading to do
		if ( s_soundtime > snd.volTime1 ) {    // has started fading
			snd.volFadeFrac = ( (float)( s_soundtime - snd.volTime1 ) / (float)( snd.volTime2 - snd.volTime1 ) );
			snd.volCurrent = ( ( 1.0 - snd.volFadeFrac ) * snd.volStart + snd.volFadeFrac * snd.volTarget );

//DAJ			Com_DPrintf( "master vol: %f\n", snd.volCurrent );

		} else {
			snd.volCurrent = snd.volStart;
		}
	} else {
		snd.volCurrent = snd.volTarget;

#if defined RTCW_ET
		if ( snd.stopSounds ) {
			S_StopAllSounds();  // faded out, stop playing
			snd.stopSounds = qfalse;
		}
#endif // RTCW_XX

	}
//----(SA)	end
#endif // RTCW_XX

	SNDDMA_BeginPainting();
	S_PaintChannels( endtime );
	SNDDMA_Submit();

	lastTime = thisTime;
}

/*
===============================================================================

console functions

===============================================================================
*/

void S_Play_f( void ) {
	int i;
	sfxHandle_t h;
	char name[256];

	i = 1;
	while ( i < Cmd_Argc() ) {
		if ( !Q_strrchr( Cmd_Argv( i ), '.' ) ) {
			Com_sprintf( name, sizeof( name ), "%s.wav", Cmd_Argv( 1 ) );
		} else {
			Q_strncpyz( name, Cmd_Argv( i ), sizeof( name ) );
		}
		h = S_RegisterSound( name, qfalse );
		if ( h ) {

#if !defined RTCW_ET
			S_StartLocalSound( h, CHAN_LOCAL_SOUND );
#else
			S_StartLocalSound( h, CHAN_LOCAL_SOUND, 127 );
#endif // RTCW_XX

		}
		i++;
	}
}

#if !defined RTCW_MP
/*
==============
S_QueueMusic_f
	console interface really just for testing
==============
*/
void S_QueueMusic_f( void ) {
	int type = -2;  // default to setting this as the next continual loop
	int c;

	c = Cmd_Argc();

	if ( c == 3 ) {
		type = atoi( Cmd_Argv( 2 ) );
	}

	if ( type != -1 ) { // clamp to valid values (-1, -2)
		type = -2;
	}

	// NOTE: could actually use this to touch the file now so there's not a hit when the queue'd music is played?
	S_StartBackgroundTrack( Cmd_Argv( 1 ), Cmd_Argv( 1 ), type );
}
#endif // RTCW_XX

void S_Music_f( void ) {
	int c;

	c = Cmd_Argc();

	if ( c == 2 ) {

#if !defined RTCW_MP
		S_StartBackgroundTrack( Cmd_Argv( 1 ), Cmd_Argv( 1 ), 0 );
#else
		S_StartBackgroundTrack( Cmd_Argv( 1 ), Cmd_Argv( 1 ) );
#endif // RTCW_XX

	} else if ( c == 3 ) {

#if !defined RTCW_MP
		S_StartBackgroundTrack( Cmd_Argv( 1 ), Cmd_Argv( 2 ), 0 );
#else
		S_StartBackgroundTrack( Cmd_Argv( 1 ), Cmd_Argv( 2 ) );
#endif // RTCW_XX

		Q_strncpyz( streamingSounds[0].loop, Cmd_Argv( 2 ), sizeof( streamingSounds[0].loop ) );
	} else {
		Com_Printf( "music <musicfile> [loopfile]\n" );
		return;
	}

}

// Ridah, just for testing the streaming sounds
void S_StreamingSound_f( void ) {
	int c;

	c = Cmd_Argc();

	if ( c == 2 ) {
		S_StartStreamingSound( Cmd_Argv( 1 ), 0, -1, 0, 0 );
	} else if ( c == 5 ) {
		S_StartStreamingSound( Cmd_Argv( 1 ), 0, atoi( Cmd_Argv( 2 ) ), atoi( Cmd_Argv( 3 ) ), atoi( Cmd_Argv( 4 ) ) );
	} else {
		Com_Printf( "streamingsound <soundfile> [entnum channel attenuation]\n" );
		return;
	}

}

void S_SoundList_f( void ) {
	int i;
	sfx_t   *sfx;
	int size, total;
	char type[4][16];
	char mem[2][16];

	strcpy( type[0], "16bit" );
	strcpy( type[1], "adpcm" );
	strcpy( type[2], "daub4" );
	strcpy( type[3], "mulaw" );
	strcpy( mem[0], "paged out" );
	strcpy( mem[1], "resident " );
	total = 0;

#if !defined RTCW_MP
	for ( sfx = s_knownSfx, i = 0 ; i < snd.s_numSfx ; i++, sfx++ ) {
#else
	for ( sfx = s_knownSfx, i = 0 ; i < s_numSfx ; i++, sfx++ ) {
#endif // RTCW_XX

		size = sfx->soundLength;
		total += size;
		Com_Printf( "%6i[%s] : %s[%s]\n", size, type[sfx->soundCompressionMethod], sfx->soundName, mem[sfx->inMemory] );
	}
	Com_Printf( "Total resident: %i\n", total );
	S_DisplayFreeMemory();
}


/*
===============================================================================

STREAMING SOUND

===============================================================================
*/

#if !defined RTCW_MP
int FGetLittleLong( const fileHandle_t f ) {
#else
int FGetLittleLong( fileHandle_t f ) {
#endif // RTCW_XX

	int v;

	FS_Read( &v, sizeof( v ), f );

	return rtcw::Endian::le( v );
}

#if !defined RTCW_MP
int FGetLittleShort( const fileHandle_t f ) {
#else
int FGetLittleShort( fileHandle_t f ) {
#endif // RTCW_XX

	short v;

	FS_Read( &v, sizeof( v ), f );

	return rtcw::Endian::le( v );
}

// returns the length of the data in the chunk, or 0 if not found

#if !defined RTCW_MP
int S_FindWavChunk( const fileHandle_t f, const char *chunk ) {
#else
int S_FindWavChunk( fileHandle_t f, const char *chunk ) {
#endif // RTCW_XX

	char name[5];
	int len;
	int r;

	name[4] = 0;
	len = 0;
	r = FS_Read( name, 4, f );
	if ( r != 4 ) {
		return 0;
	}
	len = FGetLittleLong( f );
	if ( len < 0 || len > 0xfffffff ) {
		len = 0;
		return 0;
	}
	len = ( len + 1 ) & ~1;      // pad to word boundary
//	s_nextWavChunk += len + 8;

	if ( strcmp( name, chunk ) ) {
		return 0;
	}

	return len;
}

/*
======================
S_StartBackgroundTrack
======================
*/

#if !defined RTCW_MP
void S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime ) {
	int len;
	char dump[16];
//	char	name[MAX_QPATH];
	char loopMusic[MAX_QPATH];
	streamingSound_t *ss;
	fileHandle_t fh;

	// music is always track 0
	ss = &streamingSounds[0];

//----(SA)	added
	if ( fadeupTime < 0 ) {    // queue, don't play until music fades to 0 or is stopped
		// -1 - queue to play once then return to music
		// -2 - queue to set as new looping music

		if ( intro && strlen( intro ) ) {
			strcpy( snd.nextMusicTrack, intro );
			snd.nextMusicTrackType = fadeupTime;
			if ( fadeupTime == -2 ) {
				Cvar_Set( "s_currentMusic", intro ); //----(SA)	so the savegame will have the right music

			}
			if ( s_debugMusic->integer ) {
				if ( fadeupTime == -1 ) {
					Com_Printf( "MUSIC: StartBgTrack: queueing '%s' for play once\n", intro );
				} else if ( fadeupTime == -2 ) {
					Com_Printf( "MUSIC: StartBgTrack: queueing '%s' as new loop\n", intro );
				}
			}

		} else {
			snd.nextMusicTrack[0] = 0;  // clear out the next track so things go on as they are
			snd.nextMusicTrackType = 0; // be quiet at the next opportunity

			// clear out looping sound in current music so that it'll stop when it's done
			if ( ss && ss->loop ) {
				ss->loop[0] = 0;    // clear loop
			}

			if ( s_debugMusic->integer ) {
				Com_Printf( "S_StartBgTrack(): queue cleared\n" );
			}
		}

		return; // don't actually start any queued sounds
	}

	// clear out nextMusic
//	snd.nextMusicTrack[0] = 0;
//----(SA)	end

	if (!snd.s_soundStarted)
		return;

	if ( !intro ) {
		intro = "";
	}
	if ( !loop || !loop[0] ) {
		Q_strncpyz( loopMusic, intro, sizeof( loopMusic ) );
	} else {
		Q_strncpyz( loopMusic, loop, sizeof( loopMusic ) );
	}

	Cvar_Set( "s_currentMusic", "" ); //----(SA)	so the savegame will have the right music

	if ( !Q_stricmp( loop, "onetimeonly" ) ) { // don't change the loop if you're playing a single hit
		Q_strncpyz( loopMusic, ss->loop, sizeof( loopMusic ) );
	}

	Q_strncpyz( ss->loop, loopMusic, sizeof( ss->loop ) - 4 );

	Q_strncpyz( ss->name, intro, sizeof( ss->name ) - 4 );
	COM_DefaultExtension( ss->name, sizeof( ss->name ), ".wav" );

	// close the current sound if present, but DON'T reset s_rawend
	if ( ss->file ) {
		FS_FCloseFile( ss->file );
		ss->file = 0;
	}

	if ( !intro[0] ) {
		Com_DPrintf( "Fail to start: %s\n", ss->name );   // (SA) TEMP
		return;
	}

	ss->channel         = 0;
	ss->entnum          = -1;
	ss->attenuation     = 0;

	fh = 0;
	//
	// open up a wav file and get all the info
	//
	FS_FOpenFileRead( ss->name, &fh, qtrue );
	if ( !fh ) {
		Com_Printf( "Couldn't open streaming sound file %s\n", ss->name );
		return;
	}

	// skip the riff wav header

	FS_Read( dump, 12, fh );

	if ( !S_FindWavChunk( fh, "fmt " ) ) {
		Com_Printf( "No fmt chunk in %s\n", ss->name );
		FS_FCloseFile( fh );
		return;
	}

	// save name for soundinfo
	ss->info.format = FGetLittleShort( fh );
	ss->info.channels = FGetLittleShort( fh );
	ss->info.rate = FGetLittleLong( fh );
	FGetLittleLong( fh );
	FGetLittleShort( fh );
	ss->info.width = FGetLittleShort( fh ) / 8;

	if ( ss->info.format != WAV_FORMAT_PCM ) {
		FS_FCloseFile( fh );
		Com_Printf( "Not a microsoft PCM format wav: %s\n", ss->name );
		return;
	}

	if ( ss->info.channels != 2 || ss->info.rate != 22050 ) {
		Com_Printf( "WARNING: music file %s is not 22k stereo\n", ss->name );
	}

	if ( ( len = S_FindWavChunk( fh, "data" ) ) == 0 ) {
		FS_FCloseFile( fh );
		Com_Printf( "No data chunk in %s\n", ss->name );
		return;
	}

	if ( s_debugMusic->integer ) {
		Com_Printf( "MUSIC: StartBgTrack:\n   playing %s\n   looping %s %d\n", intro, loopMusic, fadeupTime );
	}

	Cvar_Set( "s_currentMusic", loopMusic ); //----(SA)	so the savegame will have the right music

	ss->info.samples = len / ( ss->info.width * ss->info.channels );

	ss->samples = ss->info.samples;

	ss->fadeStartVol    = 0;
	ss->fadeStart       = 0;
	ss->fadeEnd         = 0;
	ss->fadeTargetVol   = 0;

	if ( fadeupTime ) {
		ss->fadeStart       = s_soundtime;
		ss->fadeEnd         = s_soundtime + ( ( (float)( ss->info.rate ) / 1000.0f ) * fadeupTime );
//		ss->fadeStart		= s_paintedtime;
//		ss->fadeEnd			= s_paintedtime + (((float)(ss->info.rate)/1000.0f ) * fadeupTime);
		ss->fadeTargetVol   = 1.0;
	}

	ss->looped = 0; //----(SA)	added

	ss->file = fh;
	ss->kill = 0;
	numStreamingSounds++;

	Com_DPrintf( "S_StartBackgroundTrack - Success\n" );
}
#else
void S_StartBackgroundTrack( const char *intro, const char *loop ) {
	int len;
	char dump[16];
	char name[MAX_QPATH];
	int i;
	streamingSound_t *ss;
	fileHandle_t fh;

//	if (!s_soundStarted  || !crit) {
//		return;
//	}

	if ( !intro ) {
		intro = "";
	}
	if ( !loop || !loop[0] ) {
		loop = intro;
	}
	Com_DPrintf( "S_StartBackgroundTrack( %s, %s )...\n", intro, loop );

	// music is always track 0
	i = 0;

	ss = &streamingSounds[i];

	Q_strncpyz( ss->loop, loop, sizeof( ss->loop ) - 4 );

	Q_strncpyz( name, intro, sizeof( name ) - 4 );
	COM_DefaultExtension( name, sizeof( name ), ".wav" );

	// close the current sound if present, but DON'T reset s_rawend
	if ( ss->file ) {
		FS_FCloseFile( ss->file );
		ss->file = 0;
	}

	if ( !intro[0] ) {
		Com_DPrintf( "Fail to start: %s\n", name );
		return;
	}

	ss->channel = 0;
	ss->entnum = -1;
	ss->attenuation = 0;

	//
	// open up a wav file and get all the info
	//
	FS_FOpenFileRead( name, &fh, qtrue );
	if ( !fh ) {
		Com_Printf( "Couldn't open streaming sound file %s\n", name );
		return;
	}

	// skip the riff wav header

	FS_Read( dump, 12, fh );

	if ( !S_FindWavChunk( fh, "fmt " ) ) {
		Com_Printf( "No fmt chunk in %s\n", name );
		FS_FCloseFile( fh );
		return;
	}

	// save name for soundinfo
	ss->info.format = FGetLittleShort( fh );
	ss->info.channels = FGetLittleShort( fh );
	ss->info.rate = FGetLittleLong( fh );
	FGetLittleLong( fh );
	FGetLittleShort( fh );
	ss->info.width = FGetLittleShort( fh ) / 8;

	if ( ss->info.format != WAV_FORMAT_PCM ) {
		FS_FCloseFile( fh );
		Com_Printf( "Not a microsoft PCM format wav: %s\n", name );
		return;
	}

	if ( ss->info.channels != 2 || ss->info.rate != 22050 ) {
		Com_Printf( "WARNING: music file %s is not 22k stereo\n", name );
	}

	if ( ( len = S_FindWavChunk( fh, "data" ) ) == 0 ) {
		FS_FCloseFile( fh );
		Com_Printf( "No data chunk in %s\n", name );
		return;
	}

	ss->info.samples = len / ( ss->info.width * ss->info.channels );
	ss->samples = ss->info.samples;
	ss->file = fh;
	ss->kill = qfalse;
	numStreamingSounds++;

	Com_DPrintf( "S_StartBackgroundTrack - Success\n", intro, loop );
}
#endif // RTCW_XX


#if !defined RTCW_MP
/*
==============
S_FadeAllSounds

==============
*/

#if !defined RTCW_ET
void S_FadeAllSounds( float targetVol, int time ) {
#else
void S_FadeAllSounds( float targetVol, int time, qboolean stopsounds ) {
#endif // RTCW_XX

#if defined RTCW_ET
	// TAT 11/15/2002
	//		Because of strange timing issues, sometimes we try to fade up before the fade down completed
	//		If that's the case, just force an immediate stop to all sounds
	if ( s_soundtime < snd.volTime2 && snd.stopSounds ) {
		S_StopAllSounds();
	}
#endif // RTCW_XX

	snd.volStart = snd.volCurrent;
	snd.volTarget = targetVol;

	snd.volTime1 = s_soundtime;
	snd.volTime2 = s_soundtime + ( ( (float)( dma.speed ) / 1000.0f ) * time );

#if defined RTCW_ET
	snd.stopSounds = stopsounds;
#endif // RTCW_XX

	// instant
	if ( !time ) {
		snd.volTarget = snd.volStart = snd.volCurrent = targetVol;  // set it
		snd.volTime1 = snd.volTime2 = 0;    // no fading
	}
}


//----(SA)	added
/*
==============
S_FadeStreamingSound
==============
*/
void S_FadeStreamingSound( float targetVol, int time, int ssNum ) {
	streamingSound_t *ss;

	if ( ssNum >= numStreamingSounds ) { // invalid sound
		return;
	}

	ss = &streamingSounds[ssNum];

	if ( !ss ) {
		return;
	}

	if ( ss->kill ) {
		return;
	}

	ss->fadeStartVol    = 1.0f;

	if ( ssNum == 0 ) {
		if ( s_debugMusic->integer ) {
			Com_Printf( "MUSIC: Fade: %0.2f %d\n", targetVol, time );
		}
	}

	// get current fraction if already fading/faded
	if ( ss->fadeStart ) {
		if ( ss->fadeEnd <= s_soundtime ) {
//		if(ss->fadeEnd <= s_paintedtime)
			ss->fadeStartVol = ss->fadeTargetVol;
		} else {
			ss->fadeStartVol = ( (float)( s_soundtime - ss->fadeStart ) / (float)( ss->fadeEnd - ss->fadeStart ) );
		}
//			ss->fadeStartVol = ( (float)(s_paintedtime - ss->fadeStart)/(float)(ss->fadeEnd - ss->fadeStart) );
	}

	ss->fadeStart       = s_soundtime;
	ss->fadeEnd         = s_soundtime + ( ( (float)( ss->info.rate ) / 1000.0f ) * time );
//	ss->fadeStart		= s_paintedtime;
//	ss->fadeEnd			= s_paintedtime + (((float)(ss->info.rate)/1000.0f ) * time);
	ss->fadeTargetVol   = targetVol;
}


/*
==============
S_GetStreamingFade
==============
*/
float S_GetStreamingFade( streamingSound_t *ss ) {
	float oldfrac, newfrac;

//	if(ss->kill)
//		return 0;

	if ( !ss->fadeStart ) {
		return 1.0f;    // full volume

	}
	if ( ss->fadeEnd <= s_soundtime ) {    // it's hit it's target
//	if(ss->fadeEnd <= s_paintedtime) {	// it's hit it's target
		if ( ss->fadeTargetVol <= 0 ) {    // faded out.  die next update
			ss->kill = 1;
		}
		return ss->fadeTargetVol;
	}

	newfrac = (float)( s_soundtime - ss->fadeStart ) / (float)( ss->fadeEnd - ss->fadeStart );
//	newfrac = (float)(s_paintedtime - ss->fadeStart)/(float)(ss->fadeEnd - ss->fadeStart);
	oldfrac = 1.0f - newfrac;

	return ( oldfrac * ss->fadeStartVol ) + ( newfrac * ss->fadeTargetVol );
}

//----(SA)	end
#endif // RTCW_XX

/*
======================
S_StartStreamingSound

  FIXME: record the starting cg.time of the sound, so we can determine the
  position by looking at the current cg.time, this way pausing or loading a
  savegame won't screw up the timing of important sounds
======================
*/

#if !defined RTCW_ET
void S_StartStreamingSound( const char *intro, const char *loop, int entnum, int channel, int attenuation ) {
#else
float S_StartStreamingSound( const char *intro, const char *loop, int entnum, int channel, int attenuation ) {
#endif // RTCW_XX

	int len;
	char dump[16];

#if !defined RTCW_MP
//	char	name[MAX_QPATH];
#else
	char name[MAX_QPATH];
#endif // RTCW_XX

	int i;
	streamingSound_t *ss;
	fileHandle_t fh;

#if !defined RTCW_MP
	if (!snd.s_soundStarted || snd.s_soundMute || cls.state != CA_ACTIVE) {
#else
	if (!s_soundStarted || s_soundMuted || cls.state != CA_ACTIVE ) {
#endif // RTCW_XX

#if !defined RTCW_ET
		return;
#else
		return 0;
#endif // RTCW_XX

	}

	if ( !intro || !intro[0] ) {
		if ( loop && loop[0] ) {
			intro = loop;
		} else {
			intro = "";
		}
	}
	Com_DPrintf( "S_StartStreamingSound( %s, %s, %i, %i, %i )\n", intro, loop, entnum, channel, attenuation );

	// look for a free track, but first check for overriding a currently playing sound for this entity
	ss = NULL;
	if ( entnum >= 0 ) {
		for ( i = 1; i < MAX_STREAMING_SOUNDS; i++ ) {    // track 0 is music/cinematics
			if ( !streamingSounds[i].file ) {
				continue;
			}
			// check to see if this character currently has another sound streaming on the same channel
			if ( ( channel != CHAN_AUTO ) && ( streamingSounds[i].entnum >= 0 ) && ( streamingSounds[i].channel == channel ) && ( streamingSounds[i].entnum == entnum ) ) {
				// found a match, override this channel

#if !defined RTCW_MP
				streamingSounds[i].kill = 1;
#else
				streamingSounds[i].kill = qtrue;
#endif // RTCW_XX

				ss = &streamingSounds[i];   // use this track to start the new stream
				break;
			}
		}
	}
	if ( !ss ) {
		// no need to override a current stream, so look for a free track
		for ( i = 1; i < MAX_STREAMING_SOUNDS; i++ ) {    // track 0 is music/cinematics
			if ( !streamingSounds[i].file ) {
				ss = &streamingSounds[i];
				break;
			}
		}
	}
	if ( !ss ) {
		if ( !s_mute->integer ) {  // don't do the print if you're muted
			Com_Printf( "S_StartStreamingSound: No free streaming tracks\n" );
		}

#if !defined RTCW_ET
		return;
#else
		return 0;
#endif // RTCW_XX

	}

	if ( ss->loop && loop ) {
		Q_strncpyz( ss->loop, loop, sizeof( ss->loop ) - 4 );
	} else {
		ss->loop[0] = 0;
	}

#if !defined RTCW_MP
	Q_strncpyz( ss->name, intro, sizeof( ss->name ) - 4 );
	COM_DefaultExtension( ss->name, sizeof( ss->name ), ".wav" );
#else
	Q_strncpyz( name, intro, sizeof( name ) - 4 );
	COM_DefaultExtension( name, sizeof( name ), ".wav" );
#endif // RTCW_XX

	// close the current sound if present, but DON'T reset s_rawend
	if ( ss->file ) {
		FS_FCloseFile( ss->file );
		ss->file = 0;
	}

	if ( !intro[0] ) {
#if !defined RTCW_ET
		return;
#else
		return 0;
#endif // RTCW_XX

	}

#if !defined RTCW_MP
	fh = 0;
#endif // RTCW_XX

	//
	// open up a wav file and get all the info
	//

#if !defined RTCW_MP
	FS_FOpenFileRead( ss->name, &fh, qtrue );
	if ( !fh ) {
		Com_Printf( "Couldn't open streaming sound file %s\n", ss->name );
#else
	FS_FOpenFileRead( name, &fh, qtrue );
	if ( !fh ) {
		Com_Printf( "Couldn't open streaming sound file %s\n", name );
#endif // RTCW_XX

#if !defined RTCW_ET
		return;
#else
		return 0;
#endif // RTCW_XX

	}

	// skip the riff wav header

	FS_Read( dump, 12, fh );

	if ( !S_FindWavChunk( fh, "fmt " ) ) {

#if !defined RTCW_MP
		Com_Printf( "No fmt chunk in %s\n", ss->name );
#else
		Com_Printf( "No fmt chunk in %s\n", name );
#endif // RTCW_XX

		FS_FCloseFile( fh );

#if !defined RTCW_ET
		return;
#else
		return 0;
#endif // RTCW_XX

	}

	// save name for soundinfo
	ss->info.format = FGetLittleShort( fh );
	ss->info.channels = FGetLittleShort( fh );
	ss->info.rate = FGetLittleLong( fh );
	FGetLittleLong(  fh );
	FGetLittleShort(  fh );
	ss->info.width = FGetLittleShort( fh ) / 8;

	if ( ss->info.format != WAV_FORMAT_PCM ) {
		FS_FCloseFile( fh );

#if !defined RTCW_MP
		Com_Printf( "Not a microsoft PCM format wav: %s\n", ss->name );
#else
		Com_Printf( "Not a microsoft PCM format wav: %s\n", name );
#endif // RTCW_XX

#if !defined RTCW_ET
		return;
#else
		return 0;
#endif // RTCW_XX

	}

	//if ( ss->info.channels != 2 || ss->info.rate != 22050 ) {

#if !defined RTCW_MP
	//	Com_Printf("WARNING: music file %s is not 22k stereo\n", ss->name );
#else
	//	Com_Printf("WARNING: music file %s is not 22k stereo\n", name );
#endif // RTCW_XX

	//}

	if ( ( len = S_FindWavChunk( fh, "data" ) ) == 0 ) {
		FS_FCloseFile( fh );

#if !defined RTCW_MP
		Com_Printf( "No data chunk in %s\n", ss->name );
#else
		Com_Printf( "No data chunk in %s\n", name );
#endif // RTCW_XX

#if !defined RTCW_ET
		return;
#else
		return 0;
#endif // RTCW_XX

	}

	ss->info.samples = len / ( ss->info.width * ss->info.channels );

	ss->samples = ss->info.samples;
	ss->channel = channel;
	ss->attenuation = attenuation;
	ss->entnum = entnum;

#if !defined RTCW_MP
	ss->kill = 0;

	ss->fadeStartVol    = 0;
	ss->fadeStart       = 0;
	ss->fadeEnd         = 0;
	ss->fadeTargetVol   = 0;
#else
	ss->kill = qfalse;
#endif // RTCW_XX

	ss->file = fh;
	numStreamingSounds++;

#if defined RTCW_ET
	return ( ss->samples / (float)ss->info.rate ) * 1000.f;
#endif // RTCW_XX

}

/*
======================
S_StopStreamingSound
======================
*/
void S_StopStreamingSound( int index ) {
	if ( !streamingSounds[index].file ) {
		return;
	}

#if !defined RTCW_MP
	streamingSounds[index].kill = 1;
#else
	streamingSounds[index].kill = qtrue;
#endif // RTCW_XX
}

#if !defined RTCW_MP
//----(SA)	added
/*
==============
S_StopEntStreamingSound
==============
*/
void S_StopEntStreamingSound( int entNum ) {
	int i;

#if !defined RTCW_ET
	if ( entNum < 0 ) {
		return;
	}
#endif // RTCW_XX

	for ( i = 1; i < MAX_STREAMING_SOUNDS; i++ ) {    // track 0 is music/cinematics
		if ( !streamingSounds[i].file ) {
			continue;
		}

#if !defined RTCW_ET
		if ( streamingSounds[i].entnum != entNum ) {
#else
		// Gordon: -1 = ALL now
		if ( streamingSounds[i].entnum != entNum && entNum != -1 ) {
#endif // RTCW_XX

			continue;
		}

		S_StopStreamingSound( i );
		s_rawend[i] = 0;    // stop it /now/
	}
}
//----(SA)	end
#endif // RTCW_XX

/*
======================
S_StopBackgroundTrack
======================
*/
void S_StopBackgroundTrack( void ) {
	S_StopStreamingSound( 0 );
}

/*
======================
S_UpdateStreamingSounds
======================
*/
void S_UpdateStreamingSounds( void ) {
	int bufferSamples;
	int fileSamples;
	byte raw[30000];        // just enough to fit in a mac stack frame
	int fileBytes;
	int r, i;
	streamingSound_t *ss;
	int     *re, *rp;

#if !defined RTCW_MP
//	qboolean looped;
#else
	qboolean looped;
#endif // RTCW_XX

	float lvol, rvol;
	int soundMixAheadTime;

#if !defined RTCW_MP
	float streamingVol = 1.0f;

	if (!snd.s_soundStarted)
		return;

	// seems like the mute would be better down lower so no timing gets messed up
//	if ( s_mute->value ) {	//----(SA)	sound is muted, skip everything
//		return;
//	}
#else
//	if (!s_soundStarted  || !crit) {
//		return;
//	}

	if ( s_mute->value ) {  //----(SA)	sound is muted, skip everything
		return;
	}
#endif // RTCW_XX

	soundMixAheadTime = s_soundtime; // + (int)(0.35 * dma.speed);	// allow for talking animations

#if !defined RTCW_MP
	snd.s_soundPainted = qtrue;
#else
	//----(SA)	it seems this could potentially be in the wrong place.
	//			The intended purpose is to just quiet all sounds if s_mute is set (like a TV mute button)
	//			however, it seems the location here could potentially cause some streaming sound updates
	//			to not happen properly, so if you mute and un-mute while listening to a conversation
	//			you could screw up the timing.  Is that the case?
	//			Ryan, could you give it a quick once-over to see if this is okay?
	//
	//	(go ahead and delete commentary when you read)


	s_soundPainted = qtrue;
#endif // RTCW_XX

	for ( i = 0, ss = streamingSounds, re = s_rawend, rp = s_rawpainted; i < MAX_STREAMING_SOUNDS; i++, ss++, re++, rp++ ) {

#if !defined RTCW_MP
		if ( ss->kill && ss->file ) {
#else
		if ( ss->kill ) {
#endif // RTCW_XX

			fileHandle_t file;
			file = ss->file;
			ss->file = 0;
			FS_FCloseFile( file );
			numStreamingSounds--;

#if !defined RTCW_MP
			if ( i == 0 || ss->kill == 2 ) { //  kill whole channel /now/
//				memset( &s_rawsamples[i], 0, MAX_RAW_SAMPLES*sizeof(portable_samplepair_t) );
				*re = 0;    // reset rawend

			}
			ss->kill = 0;
#else
			ss->kill = qfalse;
#endif // RTCW_XX

			continue;
		}

		*rp = qfalse;

#if defined RTCW_MP
		if ( !ss->file ) {
			continue;
		}
#endif // RTCW_XX

		// don't bother playing anything if musicvolume is 0
		if ( i == 0 && s_musicVolume->value <= 0 ) {
			continue;
		}
		if ( i > 0 && s_volume->value <= 0 ) {
			continue;
		}

#if !defined RTCW_MP
		if ( !ss->file ) {
			if ( i == 0 ) {  // music
				// quiet now, so start up queued music if it exists
				S_CheckForQueuedMusic();
			}
			continue;                       // skip until next frame
		}
#endif // RTCW_XX

		// see how many samples should be copied into the raw buffer
		if ( *re < soundMixAheadTime ) {    // RF, read a bit ahead of time to allow for talking animations
			*re = soundMixAheadTime;
		}

#if !defined RTCW_MP
//		looped = qfalse;
#else
		looped = qfalse;
#endif // RTCW_XX

		while ( *re < soundMixAheadTime + MAX_RAW_SAMPLES ) {
			bufferSamples = MAX_RAW_SAMPLES - ( *re - soundMixAheadTime );

			// decide how much data needs to be read from the file
			fileSamples = bufferSamples * ss->info.rate / dma.speed;

			// if there are no samples due to be read this frame, abort painting
			// but keep the streaming going, since it might just need to wait until
			// the next frame before it needs to paint some more
			if ( !fileSamples ) {
				break;
			}

			// don't try and read past the end of the file
			if ( fileSamples > ss->samples ) {
				fileSamples = ss->samples;
			}

			// our max buffer size
			fileBytes = fileSamples * ( ss->info.width * ss->info.channels );
			if ( fileBytes > sizeof( raw ) ) {
				fileBytes = sizeof( raw );
				fileSamples = fileBytes / ( ss->info.width * ss->info.channels );
			}

			r = FS_Read(raw, fileBytes, ss->file);

			if ( r != fileBytes ) {
				Com_DPrintf( "StreamedRead failure on stream sound\n" );

#if !defined RTCW_MP
				ss->kill = 1;
#else
				ss->kill = qtrue;
#endif // RTCW_XX

				break;
			}

			// byte swap if needed
			S_ByteSwapRawSamples( fileSamples, ss->info.width, ss->info.channels, (short*)raw );

			// calculate the volume

#if !defined RTCW_MP
			streamingVol = S_GetStreamingFade( ss );

			streamingVol *= snd.volCurrent; // get current global volume level

			if ( s_mute->value ) {  //----(SA)	sound is muted.  process to maintain timing, but play at 0 volume
				streamingVol = 0;
			}
#endif // RTCW_XX

			if ( i == 0 ) {   // music

#if !defined RTCW_MP
				lvol = rvol = s_musicVolume->value * streamingVol;
#else
				lvol = rvol = s_musicVolume->value;
#endif // RTCW_XX

			} else {        // attenuate if required
				if ( ss->entnum >= 0 && ss->attenuation ) {
					int r, l;

#if defined RTCW_SP
					S_SpatializeOrigin( snd.entityPositions[ ss->entnum ], s_volume->value * 255.0f, &l, &r, SOUND_RANGE_DEFAULT );
#elif defined RTCW_MP
					S_SpatializeOrigin( entityPositions[ ss->entnum ], s_volume->value * 255.0f, &l, &r, SOUND_RANGE_DEFAULT );
#else
					S_SpatializeOrigin( snd.entityPositions[ ss->entnum ], s_volume->value * 255.0f, &l, &r, SOUND_RANGE_DEFAULT, qfalse );
#endif // RTCW_XX

					if ( ( lvol = ( (float)l / 255.0 ) ) > 1.0 ) {
						lvol = 1.0;
					}
					if ( ( rvol = ( (float)r / 255.0 ) ) > 1.0 ) {
						rvol = 1.0;
					}

#if !defined RTCW_MP
					lvol *= streamingVol;
					rvol *= streamingVol;
#endif // RTCW_XX

				} else {

#if !defined RTCW_MP
					lvol = rvol = s_volume->value * streamingVol;
#else
					lvol = rvol = s_volume->value;
#endif // RTCW_XX

				}
			}

			// add to raw buffer
			S_RawSamples( fileSamples, ss->info.rate,
						  ss->info.width, ss->info.channels, raw, lvol, rvol, i );

			*rp = qtrue;

			ss->samples -= fileSamples;

			if ( !ss->samples ) {   // at the end of the sound

#if !defined RTCW_MP
				// Queued music will take over as the new loop
				// start up queued music if it exists
				if ( i == 0 && snd.nextMusicTrackType ) {        // queued music is queued
					if ( ss->file ) {
						fileHandle_t file;
						file = ss->file;
						ss->file = 0;
						FS_FCloseFile( file );
						numStreamingSounds--;
//						memset( &s_rawsamples[i], 0, MAX_RAW_SAMPLES*sizeof(portable_samplepair_t) );	// really clear it
						s_rawend[i] = 0;    // reset rawend
					}
/*
					nextMusicVA = va("%s", snd.nextMusicTrack);
					if(snd.nextMusicTrackType == QUEUED_PLAY_ONCE) {
						S_StartBackgroundTrack( nextMusicVA, ss->name, 0);		// play once, then go back to looping what's currently playing
					} else {	// QUEUED_PLAY_LOOPED
						S_StartBackgroundTrack( nextMusicVA, nextMusicVA, 0);	// take over
					}
					snd.nextMusicTrack[0] = 0;		// clear out music queue
*/
					break;  // this is now the music ss->file, no need to re-start next time through
				} else {
					// loop
					if ( ss->loop && ss->loop[0] ) {
						if ( ss->looped ) {
							char dump[16];
							FS_Seek(ss->file, 0, FS_SEEK_SET);
							FS_Read( dump, 12, ss->file );

							if ( !S_FindWavChunk( ss->file, "fmt " ) ) {
								ss->kill = 1;
								break;
							}

							// save name for soundinfo
							ss->info.format = FGetLittleShort( ss->file );
							ss->info.channels = FGetLittleShort( ss->file );
							ss->info.rate = FGetLittleLong( ss->file );
							FGetLittleLong( ss->file );
							FGetLittleShort( ss->file );
							ss->info.width = FGetLittleShort( ss->file ) / 8;
							ss->samples = ss->info.samples;
							if ( ( S_FindWavChunk( ss->file, "data" ) ) == 0 ) {
								ss->kill = 1;
							}
							if ( s_debugMusic->integer ) {
								Com_Printf( "MUSIC: looping current track\n" );
							}
							break;
						} else {                                            // start up the sound
							S_StartBackgroundTrack( ss->loop, ss->loop, 0 );
							ss->looped = qtrue; // this is now the music ss->file, no need to re-start next time through
							break;
						}


						// no loop, just stop
					} else {
						ss->kill = 1;
						if ( i == 0 ) {
							Cvar_Set( "s_currentMusic", "" ); //----(SA)	so the savegame know's it's supposed to be quiet

							if ( s_debugMusic->integer ) {
								Com_Printf( "MUSIC: Ending current track-> no loop\n" );
							}
						}

						break;
					}
				}
#else
				if ( ss->loop && ss->loop[0] ) {
					// loop
					if ( looped ) {
						// already looped once
						//*re = 0;
						break;
					} else {
						char dump[16];
						FS_Seek(ss->file, 0, FS_SEEK_SET);
						FS_Read( dump, 12, ss->file );

						if ( !S_FindWavChunk( ss->file, "fmt " ) ) {
							ss->kill = qtrue;
							break;
						}

						// save name for soundinfo
						ss->info.format = FGetLittleShort( ss->file );
						ss->info.channels = FGetLittleShort( ss->file );
						ss->info.rate = FGetLittleLong( ss->file );
						FGetLittleLong( ss->file );
						FGetLittleShort( ss->file );
						ss->info.width = FGetLittleShort( ss->file ) / 8;
						looped = qtrue;
						ss->samples = ss->info.samples;
						if ( ( S_FindWavChunk( ss->file, "data" ) ) == 0 ) {
							ss->kill = qtrue;
							return;
						}
					}
				} else {
					// no loop, just stop
					ss->kill = qtrue;;
					break;
				}
#endif // RTCW_XX

			}
		}
	}
}


#if !defined RTCW_MP
/*
======================
S_FreeOldestSound
======================
*/
void S_FreeOldestSound( void ) {
	int i, oldest, used;
	sfx_t   *sfx;
	sndBuffer   *buffer, *nbuffer;

	oldest = Sys_Milliseconds();
	used = 0;

	for ( i = 1 ; i < snd.s_numSfx ; i++ ) {
		sfx = &s_knownSfx[i];
		if ( sfx->inMemory && sfx->lastTimeUsed < oldest ) {
			used = i;
			oldest = sfx->lastTimeUsed;
		}
	}

	sfx = &s_knownSfx[used];

	Com_DPrintf( "S_FreeOldestSound: freeing sound %s\n", sfx->soundName );

	buffer = sfx->soundData;
	while ( buffer != NULL ) {
		nbuffer = buffer->next;
		SND_free( buffer );
		buffer = nbuffer;
	}
	sfx->inMemory = qfalse;
	sfx->soundData = NULL;
}
#endif // RTCW_XX

#if defined RTCW_ET
// START	xkan, 9/23/2002
// returns how long the sound lasts in milliseconds
int S_GetSoundLength( sfxHandle_t sfxHandle ) {
	if ( sfxHandle < 0 || sfxHandle >= snd.s_numSfx ) {
		Com_DPrintf( S_COLOR_YELLOW "S_StartSound: handle %i out of range\n", sfxHandle );
		return -1;
	}
	return (int)( (float)s_knownSfx[ sfxHandle ].soundLength / dma.speed * 1000.0 );
}
// END		xkan, 9/23/2002

// ydnar: for looped sound synchronization
int S_GetCurrentSoundTime( void ) {
	return s_soundtime + dma.speed;
//	 return s_paintedtime;
}
#endif // RTCW_XX

