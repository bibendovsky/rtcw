/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "snd_local.h"


void S_AdpcmEncode( short indata[], char outdata[], int len, struct adpcm_state *state ) {
	// LordHavoc: removed 4-clause BSD code for Intel ADPCM codec
}


void S_AdpcmDecode( const char indata[], short *outdata, int len, struct adpcm_state *state ) {
	// LordHavoc: removed 4-clause BSD code for Intel ADPCM codec
}


/*
====================
S_AdpcmMemoryNeeded

Returns the amount of memory (in bytes) needed to store the samples in out internal adpcm format
====================
*/
int S_AdpcmMemoryNeeded( const wavinfo_t *info ) {
	float scale;
	int scaledSampleCount;
	int sampleMemory;
	int blockCount;
	int headerMemory;

	// determine scale to convert from input sampling rate to desired sampling rate
	scale = (float)info->rate / dma.speed;

	// calc number of samples at playback sampling rate
	scaledSampleCount = info->samples / scale;

	// calc memory need to store those samples using ADPCM at 4 bits per sample
	sampleMemory = scaledSampleCount / 2;

	// calc number of sample blocks needed of PAINTBUFFER_SIZE
	blockCount = scaledSampleCount / PAINTBUFFER_SIZE;
	if ( scaledSampleCount % PAINTBUFFER_SIZE ) {
		blockCount++;
	}

	// calc memory needed to store the block headers
	headerMemory = blockCount * sizeof( adpcm_state_t );

	return sampleMemory + headerMemory;
}


/*
====================
S_AdpcmGetSamples
====================
*/
void S_AdpcmGetSamples( sndBuffer *chunk, short *to ) {
	adpcm_state_t state;
	byte            *out;

	// get the starting state from the block header
	state.index = chunk->adpcm.index;
	state.sample = chunk->adpcm.sample;

	out = (byte *)chunk->sndChunk;
	// get samples

#if !defined RTCW_MP
	S_AdpcmDecode( reinterpret_cast<const char*> (out), to, SND_CHUNK_SIZE_BYTE * 2, &state );       //DAJ added (const char*)
#else
	S_AdpcmDecode( reinterpret_cast<const char*> (out), to, SND_CHUNK_SIZE_BYTE * 2, &state );
#endif // RTCW_XX

}


/*
====================
S_AdpcmEncodeSound
====================
*/
void S_AdpcmEncodeSound( sfx_t *sfx, short *samples ) {
	adpcm_state_t state;
	int inOffset;
	int count;
	int n;
	sndBuffer       *newchunk, *chunk;
	byte            *out;

	inOffset = 0;
	count = sfx->soundLength;
	state.index = 0;
	state.sample = samples[0];

	chunk = NULL;
	while ( count ) {
		n = count;
		if ( n > SND_CHUNK_SIZE_BYTE * 2 ) {
			n = SND_CHUNK_SIZE_BYTE * 2;
		}

		newchunk = SND_malloc();
		if ( sfx->soundData == NULL ) {
			sfx->soundData = newchunk;
		} else {
			chunk->next = newchunk;
		}
		chunk = newchunk;

		// output the header
		chunk->adpcm.index  = state.index;
		chunk->adpcm.sample = state.sample;

		out = (byte *)chunk->sndChunk;

		// encode the samples

#if !defined RTCW_MP
		S_AdpcmEncode( samples + inOffset, reinterpret_cast<char*> (out), n, &state );     //DAJ added (char*)
#else
		S_AdpcmEncode( samples + inOffset, reinterpret_cast<char*> (out), n, &state );
#endif // RTCW_XX

		inOffset += n;
		count -= n;
	}
}
