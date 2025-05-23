/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		snd_mem.c
 *
 * desc:		sound caching
 *
 * $Archive: /Wolf5/src/client/snd_mem.c $
 *
 *****************************************************************************/

#include "snd_local.h"
#include "rtcw_endian.h"

#define DEF_COMSOUNDMEGS "24"    // (SA) upped for GD

/*
===============================================================================

SOUND MEMORY MANAGENT

===============================================================================
*/

static sndBuffer   *buffer = NULL;
static sndBuffer   *freelist = NULL;
static int inUse = 0;
static int totalInUse = 0;

#if defined RTCW_ET
static int totalAllocated = 0;
#endif // RTCW_XX

short *sfxScratchBuffer = NULL;
const sfx_t *sfxScratchPointer = NULL;
int sfxScratchIndex = 0;

extern cvar_t   *s_nocompressed;

/*
================
SND_free
================
*/
void SND_free( sndBuffer *v ) {
	*(sndBuffer **)v = freelist;
	freelist = (sndBuffer*)v;
	inUse += sizeof( sndBuffer );

#if defined RTCW_ET
	totalInUse -= sizeof( sndBuffer );
#endif // RTCW_XX

}

/*
================
SND_malloc
================
*/
sndBuffer*  SND_malloc() {
	sndBuffer *v;

	while ( freelist == NULL ) {
		S_FreeOldestSound();
	}

	inUse -= sizeof( sndBuffer );
	totalInUse += sizeof( sndBuffer );
#if defined RTCW_ET
	totalAllocated += sizeof( sndBuffer );
#endif // RTCW_XX


	v = freelist;
	freelist = *(sndBuffer **)freelist;
	v->next = NULL;
	return v;
}

/*
================
SND_setup
================
*/
void SND_setup() {
	sndBuffer *p, *q;
	cvar_t  *cv;
	int scs;

	cv = Cvar_Get( "com_soundMegs", DEF_COMSOUNDMEGS, CVAR_LATCH | CVAR_ARCHIVE );

	scs = cv->integer * 512;

	// BBi
	//buffer = static_cast<sndBuffer*> (malloc( scs * sizeof( sndBuffer ) ));
	buffer = new sndBuffer[scs];
	// BBi

	// allocate the stack based hunk allocator

#if !defined RTCW_MP
	// BBi
	//sfxScratchBuffer = static_cast<short*> (malloc( SND_CHUNK_SIZE * sizeof( short ) * 4 ));  //Hunk_Alloc(SND_CHUNK_SIZE * sizeof(short) * 4);
	sfxScratchBuffer = new short[SND_CHUNK_SIZE * 4];
	// BBi
#else
//DAJ HOG	sfxScratchBuffer = malloc(SND_CHUNK_SIZE * sizeof(short) * 4);
	sfxScratchBuffer = static_cast<short*> (Hunk_Alloc( SND_CHUNK_SIZE * sizeof( short ) * 4, h_high ));  //DAJ HOG was CO
#endif // RTCW_XX

	sfxScratchPointer = NULL;

	inUse = scs * sizeof( sndBuffer );

#if defined RTCW_ET
	totalInUse = 0;
	totalAllocated = 0;
#endif // RTCW_XX

	p = buffer;;
	q = p + scs;
	while ( --q > p ) {
		*(sndBuffer **)q = q - 1;
	}
	*(sndBuffer **)q = NULL;
	freelist = p + scs - 1;

	Com_Printf( "Sound memory manager started\n" );
}

/*
===============================================================================

WAV loading

===============================================================================
*/

static byte    *data_p;
static byte    *iff_end;
static byte    *last_chunk;
static byte    *iff_data;
static int iff_chunk_len;

/*
================
GetLittleShort
================
*/
static short GetLittleShort( void ) {
	short val = 0;
	val = *data_p;
	val = val + ( *( data_p + 1 ) << 8 );
	data_p += 2;
	return val;
}

/*
================
GetLittleLong
================
*/
static int GetLittleLong( void ) {
	int val = 0;
	val = *data_p;
	val = val + ( *( data_p + 1 ) << 8 );
	val = val + ( *( data_p + 2 ) << 16 );
	val = val + ( *( data_p + 3 ) << 24 );
	data_p += 4;
	return val;
}

/*
================
FindNextChunk
================
*/
static void FindNextChunk( const char *name ) {
	while ( 1 )
	{
		data_p = last_chunk;

		if ( data_p >= iff_end ) { // didn't find the chunk
			data_p = NULL;
			return;
		}

		data_p += 4;
		iff_chunk_len = GetLittleLong();
		if ( iff_chunk_len < 0 ) {
			data_p = NULL;
			return;
		}
		data_p -= 8;
		last_chunk = data_p + 8 + ( ( iff_chunk_len + 1 ) & ~1 );
		if ( !strncmp( (char *)data_p, name, 4 ) ) {
			return;
		}
	}
}

/*
================
FindChunk
================
*/
static void FindChunk( const char *name ) {
	last_chunk = iff_data;
	FindNextChunk( name );
}

#if defined RTCW_ET
typedef struct waveFormat_s {
	const char  *name;
	int format;
} waveFormat_t;

static waveFormat_t waveFormats[] = {
	{ "Windows PCM", 1 },
	{ "Antex ADPCM", 14 },
	{ "Antex ADPCME", 33 },
	{ "Antex ADPCM", 40 },
	{ "Audio Processing Technology", 25 },
	{ "Audiofile, Inc.", 24 },
	{ "Audiofile, Inc.", 26 },
	{ "Control Resources Limited", 34 },
	{ "Control Resources Limited", 37 },
	{ "Creative ADPCM", 200 },
	{ "Dolby Laboratories", 30 },
	{ "DSP Group, Inc", 22 },
	{ "DSP Solutions, Inc.", 15 },
	{ "DSP Solutions, Inc.", 16 },
	{ "DSP Solutions, Inc.", 35 },
	{ "DSP Solutions ADPCM", 36 },
	{ "Echo Speech Corporation", 23 },
	{ "Fujitsu Corp.", 300 },
	{ "IBM Corporation", 5 },
	{ "Ing C. Olivetti & C., S.p.A.", 1000 },
	{ "Ing C. Olivetti & C., S.p.A.", 1001 },
	{ "Ing C. Olivetti & C., S.p.A.", 1002 },
	{ "Ing C. Olivetti & C., S.p.A.", 1003 },
	{ "Ing C. Olivetti & C., S.p.A.", 1004 },
	{ "Intel ADPCM", 11 },
	{ "Intel ADPCM", 11 },
	{ "Unknown", 0 },
	{ "Microsoft ADPCM", 2 },
	{ "Microsoft Corporation", 6 },
	{ "Microsoft Corporation", 7 },
	{ "Microsoft Corporation", 31 },
	{ "Microsoft Corporation", 50 },
	{ "Natural MicroSystems ADPCM", 38 },
	{ "OKI ADPCM", 10 },
	{ "Sierra ADPCM", 13 },
	{ "Speech Compression", 21 },
	{ "Videologic ADPCM", 12 },
	{ "Yamaha ADPCM", 20 },
	{ NULL, 0 }
};

static const char *GetWaveFormatName( const int format ) {
	int i = 0;

	while ( waveFormats[i].name ) {
		if ( format == waveFormats[i].format ) {
			return( waveFormats[i].name );
		}
		i++;
	}

	return( "Unknown" );

}
#endif // RTCW_XX

/*
============
GetWavinfo
============
*/
static wavinfo_t GetWavinfo( char *name, byte *wav, int wavlength ) {
	wavinfo_t info;

	Com_Memset( &info, 0, sizeof( info ) );

	if ( !wav ) {
		return info;
	}

	iff_data = wav;
	iff_end = wav + wavlength;

// find "RIFF" chunk
	FindChunk( "RIFF" );
	if ( !( data_p && !strncmp( (char *)data_p + 8, "WAVE", 4 ) ) ) {
		Com_Printf( "Missing RIFF/WAVE chunks\n" );
		return info;
	}

// get "fmt " chunk
	iff_data = data_p + 12;
// DumpChunks ();

	FindChunk( "fmt " );
	if ( !data_p ) {
		Com_Printf( "Missing fmt chunk\n" );
		return info;
	}
	data_p += 8;
	info.format = GetLittleShort();
	info.channels = GetLittleShort();
	info.rate = GetLittleLong();
	data_p += 4 + 2;
	info.width = GetLittleShort() / 8;

	if ( info.format != 1 ) {

#if defined RTCW_ET
		Com_Printf( "Unsupported format: %s\n", GetWaveFormatName( info.format ) );
#endif // RTCW_XX

		Com_Printf( "Microsoft PCM format only\n" );
		return info;
	}


// find data chunk
	FindChunk( "data" );
	if ( !data_p ) {
		Com_Printf( "Missing data chunk\n" );
		return info;
	}

	data_p += 4;
	info.samples = GetLittleLong() / info.width;
	info.dataofs = data_p - wav;

	return info;
}

/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
static void ResampleSfx( sfx_t *sfx, int inrate, int inwidth, byte *data, qboolean compressed ) {
	int outcount;
	int srcsample;
	float stepscale;
	int i;
	int sample, samplefrac, fracstep;
	int part;
	sndBuffer   *chunk;

	stepscale = (float)inrate / dma.speed;  // this is usually 0.5, 1, or 2

	outcount = sfx->soundLength / stepscale;
	sfx->soundLength = outcount;

	samplefrac = 0;
	fracstep = stepscale * 256;
	chunk = sfx->soundData;

#if defined RTCW_ET
	// Gordon: use the littleshort version only if we need to
	if (!rtcw::Endian::is_little()) {
		for ( i = 0 ; i < outcount ; i++ )
		{
			srcsample = samplefrac >> 8;
			samplefrac += fracstep;
			if ( inwidth == 2 ) {
				sample = ( (short *)data )[srcsample];
			} else {
				sample = (int)( ( unsigned char )( data[srcsample] ) - 128 ) << 8;
			}
			part  = ( i & ( SND_CHUNK_SIZE - 1 ) );
			if ( part == 0 ) {
				sndBuffer   *newchunk;
				newchunk = SND_malloc();
				if ( chunk == NULL ) {
					sfx->soundData = newchunk;
				} else {
					chunk->next = newchunk;
				}
				chunk = newchunk;
			}

			chunk->sndChunk[part] = sample;
		}
	} else {
#endif // RTCW_XX

	for ( i = 0 ; i < outcount ; i++ )
	{
		srcsample = samplefrac >> 8;
		samplefrac += fracstep;
		if ( inwidth == 2 ) {
			sample = rtcw::Endian::le( ( (short *)data )[srcsample] );
		} else {
			sample = (int)( ( unsigned char )( data[srcsample] ) - 128 ) << 8;
		}
		part  = ( i & ( SND_CHUNK_SIZE - 1 ) );
		if ( part == 0 ) {
			sndBuffer   *newchunk;
			newchunk = SND_malloc();
			if ( chunk == NULL ) {
				sfx->soundData = newchunk;
			} else {
				chunk->next = newchunk;
			}
			chunk = newchunk;
		}

		chunk->sndChunk[part] = sample;
	}

#if defined RTCW_ET
	}
#endif // RTCW_XX

}

/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
static int ResampleSfxRaw( short *sfx, int inrate, int inwidth, int samples, byte *data ) {
	int outcount;
	int srcsample;
	float stepscale;
	int i;
	int sample, samplefrac, fracstep;

	stepscale = (float)inrate / dma.speed;  // this is usually 0.5, 1, or 2

	outcount = samples / stepscale;

	samplefrac = 0;
	fracstep = stepscale * 256;

#if defined RTCW_ET
	// Gordon: use the littleshort version only if we need to
	if (!rtcw::Endian::is_little()) {
		for ( i = 0 ; i < outcount ; i++ )
		{
			srcsample = samplefrac >> 8;
			samplefrac += fracstep;
			if ( inwidth == 2 ) {
				sample = ( (short *)data )[srcsample];
			} else {
				sample = (int)( ( unsigned char )( data[srcsample] ) - 128 ) << 8;
			}
			sfx[i] = sample;
		}
	} else {
#endif // RTCW_XX

	for ( i = 0 ; i < outcount ; i++ )
	{
		srcsample = samplefrac >> 8;
		samplefrac += fracstep;
		if ( inwidth == 2 ) {
			sample = rtcw::Endian::le( ( (short *)data )[srcsample] );
		} else {
			sample = (int)( ( unsigned char )( data[srcsample] ) - 128 ) << 8;
		}
		sfx[i] = sample;
	}

#if defined RTCW_ET
	}
#endif // RTCW_XX

	return outcount;
}


//=============================================================================

/*
==============
S_LoadSound

The filename may be different than sfx->name in the case
of a forced fallback of a player specific sound
==============
*/
qboolean S_LoadSound( sfx_t *sfx ) {
	byte    *data;
	short   *samples;
	wavinfo_t info;
	int size;

	// player specific sounds are never directly loaded
	if ( sfx->soundName[0] == '*' ) {
		return qfalse;
	}

	// load it in
	size = FS_ReadFile( sfx->soundName, (void **)&data );
	if ( !data ) {
		return qfalse;
	}

	info = GetWavinfo( sfx->soundName, data, size );
	if ( info.channels != 1 ) {
		Com_Printf( "%s is a stereo wav file\n", sfx->soundName );
		FS_FreeFile( data );
		return qfalse;
	}

	if ( info.width == 1 ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: %s is a 8 bit wav file\n", sfx->soundName );
	}

	if ( info.rate != 22050 ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: %s is not a 22kHz wav file\n", sfx->soundName );
	}

	samples = static_cast<short*> (Hunk_AllocateTempMemory( info.samples * sizeof( short ) * 2 ));

#if !defined RTCW_SP
	// DHM - Nerve
#endif // RTCW_XX

	sfx->lastTimeUsed = Sys_Milliseconds() + 1;

	// each of these compression schemes works just fine
	// but the 16bit quality is much nicer and with a local
	// install assured we can rely upon the sound memory
	// manager to do the right thing for us and page
	// sound in as needed


	if ( s_nocompressed->value ) {
		sfx->soundCompressionMethod = 0;
		sfx->soundLength = info.samples;
		sfx->soundData = NULL;
		ResampleSfx( sfx, info.rate, info.width, data + info.dataofs, qfalse );
	} else if ( sfx->soundCompressed == qtrue )     {
		sfx->soundCompressionMethod = 1;
		sfx->soundData = NULL;
		sfx->soundLength = ResampleSfxRaw( samples, info.rate, info.width, info.samples, ( data + info.dataofs ) );
		S_AdpcmEncodeSound( sfx, samples );
#ifdef COMPRESSION
	} else if ( info.samples > ( SND_CHUNK_SIZE * 16 ) && info.width > 1 ) {
		sfx->soundCompressionMethod = 3;
		sfx->soundData = NULL;
		sfx->soundLength = ResampleSfxRaw( samples, info.rate, info.width, info.samples, ( data + info.dataofs ) );
		encodeMuLaw( sfx, samples );
	} else if ( info.samples > ( SND_CHUNK_SIZE * 6400 ) && info.width > 1 ) {
		sfx->soundCompressionMethod = 2;
		sfx->soundData = NULL;
		sfx->soundLength = ResampleSfxRaw( samples, info.rate, info.width, info.samples, ( data + info.dataofs ) );
		encodeWavelet( sfx, samples );
#endif
	} else {
		sfx->soundCompressionMethod = 0;
		sfx->soundLength = info.samples;
		sfx->soundData = NULL;
		ResampleSfx( sfx, info.rate, info.width, data + info.dataofs, qfalse );
	}
	Hunk_FreeTempMemory( samples );
	FS_FreeFile( data );

	return qtrue;
}

/*
================
S_DisplayFreeMemory
================
*/
void S_DisplayFreeMemory() {

#if !defined RTCW_ET
	Com_Printf( "%d bytes free sound buffer memory, %d total used\n", inUse, totalInUse );
#else
	Com_Printf( "%d bytes (%.2fMB) free sound buffer memory, %d bytes (%.2fMB) total used\n%d bytes (%.2fMB) sound buffer memory have been allocated since the last SND_setup", inUse, inUse / Square( 1024.f ), totalInUse, totalInUse / Square( 1024.f ), totalAllocated, totalAllocated / Square( 1024.f ) );
#endif // RTCW_XX

}

