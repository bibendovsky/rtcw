/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		snd_mix.c
 *
 * desc:		portable code to mix sounds for snd_dma.c
 *
 *
 *****************************************************************************/

#include "snd_local.h"

portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
static int snd_vol;

// TTimo not static, required by unix/snd_mixa.s
int     *snd_p;
int snd_linear_count;
short   *snd_out;


/*
===================
S_WriteLinearBlastStereo16
===================
*/
void S_WriteLinearBlastStereo16( void ) {
	int i;
	int val;

	for ( i = 0 ; i < snd_linear_count ; i += 2 )
	{
		val = snd_p[i] >> 8;
		if ( val > 0x7fff ) {
			snd_out[i] = 0x7fff;
		} else if ( val < (short)0x8000 ) {
			snd_out[i] = (short)0x8000;
		} else {
			snd_out[i] = val;
		}

		val = snd_p[i + 1] >> 8;
		if ( val > 0x7fff ) {
			snd_out[i + 1] = 0x7fff;
		} else if ( val < (short)0x8000 ) {
			snd_out[i + 1] = (short)0x8000;
		} else {
			snd_out[i + 1] = val;
		}
	}
}

/*
===================
S_TransferStereo16
===================
*/
void S_TransferStereo16( uint32_t *pbuf, int endtime ) {
	int lpos;
	int ls_paintedtime;

	snd_p = (int *) paintbuffer;
	ls_paintedtime = s_paintedtime;

	while ( ls_paintedtime < endtime )
	{
		// handle recirculating buffer issues
		lpos = ls_paintedtime & ( ( dma.samples >> 1 ) - 1 );

		snd_out = (short *) pbuf + ( lpos << 1 );

		snd_linear_count = ( dma.samples >> 1 ) - lpos;
		if ( ls_paintedtime + snd_linear_count > endtime ) {
			snd_linear_count = endtime - ls_paintedtime;
		}

		snd_linear_count <<= 1;

		// write a linear blast of samples
		S_WriteLinearBlastStereo16();

		snd_p += snd_linear_count;
		ls_paintedtime += ( snd_linear_count >> 1 );
	}
}

/*
===================
S_TransferPaintBuffer
===================
*/
void S_TransferPaintBuffer( int endtime ) {
	int out_idx;
	int count;
	int out_mask;
	int     *p;
	int step;
	int val;
	uint32_t *pbuf;

	pbuf = (uint32_t *)dma.buffer;
	if ( !pbuf ) {
		return;
	}

	if ( s_testsound->integer ) {
		int i;
		int count;

		// write a fixed sine wave
		count = ( endtime - s_paintedtime );

#if defined RTCW_SP
		for ( i = 0 ; i < count ; i++ ) {
			float v;
			v = c::sin( M_PI * 2 * i / 64 );
			paintbuffer[i].left = paintbuffer[i].right = v * 0x400000;
		}
#else
		for ( i = 0 ; i < count ; i++ )
			paintbuffer[i].left = paintbuffer[i].right = c::sin( ( s_paintedtime + i ) * 0.1 ) * 20000 * 256;
#endif // RTCW_XX

	}


	if ( dma.samplebits == 16 && dma.channels == 2 ) {
		// optimized case
		S_TransferStereo16( pbuf, endtime );
	} else
	{   // general case
		p = (int *) paintbuffer;
		count = ( endtime - s_paintedtime ) * dma.channels;
		out_mask = dma.samples - 1;
		out_idx = s_paintedtime * dma.channels & out_mask;
		step = 3 - dma.channels;

		if ( dma.samplebits == 16 ) {
			short *out = (short *) pbuf;
			while ( count-- )
			{
				val = *p >> 8;
				p += step;
				if ( val > 0x7fff ) {
					val = 0x7fff;
				} else if ( val < -32768 ) {
					val = -32768;
				}
				out[out_idx] = val;
				out_idx = ( out_idx + 1 ) & out_mask;
			}
		} else if ( dma.samplebits == 8 )     {
			unsigned char *out = (unsigned char *) pbuf;
			while ( count-- )
			{
				val = *p >> 8;
				p += step;
				if ( val > 0x7fff ) {
					val = 0x7fff;
				} else if ( val < -32768 ) {
					val = -32768;
				}
				out[out_idx] = ( val >> 8 ) + 128;
				out_idx = ( out_idx + 1 ) & out_mask;
			}
		}
	}
}

/*
===============================================================================

LIP SYNCING

===============================================================================
*/

#ifdef TALKANIM

unsigned char s_entityTalkAmplitude[MAX_CLIENTS];

/*
===================
S_SetVoiceAmplitudeFrom16
===================
*/
void S_SetVoiceAmplitudeFrom16( const sfx_t *sc, int sampleOffset, int count, int entnum ) {
	int data, i, sfx_count;
	sndBuffer *chunk;
	short *samples;

	if ( count <= 0 ) {
		return; // must have gone ahead of the end of the sound
	}
	chunk = sc->soundData;
	while ( sampleOffset >= SND_CHUNK_SIZE ) {
		chunk = chunk->next;
		sampleOffset -= SND_CHUNK_SIZE;
		if ( !chunk ) {
			chunk = sc->soundData;
		}
	}

	sfx_count = 0;
	samples = chunk->sndChunk;
	for ( i = 0; i < count; i++ ) {
		if ( sampleOffset >= SND_CHUNK_SIZE ) {
			chunk = chunk->next;
			samples = chunk->sndChunk;
			sampleOffset = 0;
		}
		data  = samples[sampleOffset++];
		if ( c::abs( data ) > 5000 ) {
			sfx_count += ( data * 255 ) >> 8;
		}
	}
	//Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
	// adjust the sfx_count according to the frametime (scale down for longer frametimes)
	sfx_count = c::abs( sfx_count );
	sfx_count = (int)( (float)sfx_count / ( 2.0 * (float)count ) );
	if ( sfx_count > 255 ) {
		sfx_count = 255;
	}
	if ( sfx_count < 25 ) {
		sfx_count = 0;
	}
	//Com_Printf("sfx_count = %d\n", sfx_count );
	// update the amplitude for this entity
	s_entityTalkAmplitude[entnum] = (unsigned char)sfx_count;
}

/*
===================
S_SetVoiceAmplitudeFromADPCM
===================
*/
void S_SetVoiceAmplitudeFromADPCM( const sfx_t *sc, int sampleOffset, int count, int entnum ) {
	int data, i, sfx_count;
	sndBuffer *chunk;
	short *samples;

	if ( count <= 0 ) {
		return; // must have gone ahead of the end of the sound
	}
	i = 0;
	chunk = sc->soundData;
	while ( sampleOffset >= ( SND_CHUNK_SIZE * 4 ) ) {
		chunk = chunk->next;
		sampleOffset -= ( SND_CHUNK_SIZE * 4 );
		i++;
	}

	if ( i != sfxScratchIndex || sfxScratchPointer != sc ) {
		S_AdpcmGetSamples( chunk, sfxScratchBuffer );
		sfxScratchIndex = i;
		sfxScratchPointer = sc;
	}

	sfx_count = 0;
	samples = sfxScratchBuffer;
	for ( i = 0; i < count; i++ ) {
		if ( sampleOffset >= SND_CHUNK_SIZE * 4 ) {
			chunk = chunk->next;
			S_AdpcmGetSamples( chunk, sfxScratchBuffer );
			sampleOffset = 0;
			sfxScratchIndex++;
		}
		data  = samples[sampleOffset++];
		if ( c::abs( data ) > 5000 ) {
			sfx_count += ( data * 255 ) >> 8;
		}
	}
	//Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
	// adjust the sfx_count according to the frametime (scale down for longer frametimes)
	sfx_count = c::abs( sfx_count );
	sfx_count = (int)( (float)sfx_count / ( 2.0 * (float)count ) );
	if ( sfx_count > 255 ) {
		sfx_count = 255;
	}
	if ( sfx_count < 25 ) {
		sfx_count = 0;
	}
	//Com_Printf("sfx_count = %d\n", sfx_count );
	// update the amplitude for this entity
	s_entityTalkAmplitude[entnum] = (unsigned char)sfx_count;
}

/*
===================
S_SetVoiceAmplitudeFromWavelet
===================
*/
void S_SetVoiceAmplitudeFromWavelet( const sfx_t *sc, int sampleOffset, int count, int entnum ) {
	int data, i, sfx_count;
	sndBuffer *chunk;
	short *samples;

	if ( count <= 0 ) {
		return; // must have gone ahead of the end of the sound
	}
	i = 0;
	chunk = sc->soundData;
	while ( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) ) {
		chunk = chunk->next;
		sampleOffset -= ( SND_CHUNK_SIZE_FLOAT * 4 );
		i++;
	}
	if ( i != sfxScratchIndex || sfxScratchPointer != sc ) {
		decodeWavelet( chunk, sfxScratchBuffer );
		sfxScratchIndex = i;
		sfxScratchPointer = sc;
	}
	sfx_count = 0;
	samples = sfxScratchBuffer;
	for ( i = 0; i < count; i++ ) {
		if ( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) ) {
			chunk = chunk->next;
			decodeWavelet( chunk, sfxScratchBuffer );
			sfxScratchIndex++;
			sampleOffset = 0;
		}
		data = samples[sampleOffset++];
		if ( c::abs( data ) > 5000 ) {
			sfx_count += ( data * 255 ) >> 8;
		}
	}

	//Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
	// adjust the sfx_count according to the frametime (scale down for longer frametimes)
	sfx_count = c::abs( sfx_count );
	sfx_count = (int)( (float)sfx_count / ( 2.0 * (float)count ) );
	if ( sfx_count > 255 ) {
		sfx_count = 255;
	}
	if ( sfx_count < 25 ) {
		sfx_count = 0;
	}
	//Com_Printf("sfx_count = %d\n", sfx_count );
	// update the amplitude for this entity
	s_entityTalkAmplitude[entnum] = (unsigned char)sfx_count;
}

/*
===================
S_SetVoiceAmplitudeFromMuLaw
===================
*/
void S_SetVoiceAmplitudeFromMuLaw( const sfx_t *sc, int sampleOffset, int count, int entnum ) {
	int data, i, sfx_count;
	sndBuffer *chunk;
	byte *samples;

	if ( count <= 0 ) {
		return; // must have gone ahead of the end of the sound
	}
	chunk = sc->soundData;
	while ( sampleOffset >= ( SND_CHUNK_SIZE * 2 ) ) {
		chunk = chunk->next;
		sampleOffset -= ( SND_CHUNK_SIZE * 2 );
		if ( !chunk ) {
			chunk = sc->soundData;
		}
	}
	sfx_count = 0;
	samples = (byte *)chunk->sndChunk + sampleOffset;
	for ( i = 0; i < count; i++ ) {
		if ( samples >= (byte *)chunk->sndChunk + ( SND_CHUNK_SIZE * 2 ) ) {
			chunk = chunk->next;
			samples = (byte *)chunk->sndChunk;
		}
		data  = mulawToShort[*samples];
		if ( c::abs( data ) > 5000 ) {
			sfx_count += ( data * 255 ) >> 8;
		}
		samples++;
	}
	//Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
	// adjust the sfx_count according to the frametime (scale down for longer frametimes)
	sfx_count = c::abs( sfx_count );
	sfx_count = (int)( (float)sfx_count / ( 2.0 * (float)count ) );
	if ( sfx_count > 255 ) {
		sfx_count = 255;
	}
	if ( sfx_count < 25 ) {
		sfx_count = 0;
	}
	//Com_Printf("sfx_count = %d\n", sfx_count );
	// update the amplitude for this entity
	s_entityTalkAmplitude[entnum] = (unsigned char)sfx_count;
}

/*
===================
S_GetVoiceAmplitude
===================
*/
int S_GetVoiceAmplitude( int entityNum ) {
	if ( entityNum >= MAX_CLIENTS ) {
		Com_Printf( "Error: S_GetVoiceAmplitude() called for a non-client\n" );
		return 0;
	}

	return (int)s_entityTalkAmplitude[entityNum];
}
#endif

#if (!defined RTCW_SP) && (!defined TALKANIM)
// NERVE - SMF
int S_GetVoiceAmplitude( int entityNum ) {
	return 0;
}
// -NERVE - SMF
#endif // RTCW_XX

/*
===============================================================================

CHANNEL MIXING

===============================================================================
*/

/*
===================
S_PaintChannelFrom16
===================
*/
static void S_PaintChannelFrom16( channel_t *ch, const sfx_t *sc, int count, int sampleOffset, int bufferOffset ) {
	int data, aoff, boff;
	int leftvol, rightvol;
	int i, j;
	portable_samplepair_t   *samp;
	sndBuffer               *chunk;
	short                   *samples;
	float ooff, fdata, fdiv, fleftvol, frightvol;

	samp = &paintbuffer[ bufferOffset ];

	if ( ch->doppler ) {
		sampleOffset = sampleOffset * ch->oldDopplerScale;
	}

	chunk = sc->soundData;
	while ( sampleOffset >= SND_CHUNK_SIZE ) {
		chunk = chunk->next;
		sampleOffset -= SND_CHUNK_SIZE;
		if ( !chunk ) {
			chunk = sc->soundData;
		}
	}

	if ( !ch->doppler ) {
		leftvol = ch->leftvol * snd_vol;
		rightvol = ch->rightvol * snd_vol;

		samples = chunk->sndChunk;
		for ( i = 0; i < count; i++ ) {
			if ( sampleOffset >= SND_CHUNK_SIZE ) {
				chunk = chunk->next;
				if ( chunk == NULL ) {
					chunk = sc->soundData;
				}
				samples = chunk->sndChunk;
				sampleOffset -= SND_CHUNK_SIZE;
			}
			data  = samples[sampleOffset++];
			samp[i].left += ( data * leftvol ) >> 8;
			samp[i].right += ( data * rightvol ) >> 8;
		}
	} else {
		fleftvol = ch->leftvol * snd_vol;
		frightvol = ch->rightvol * snd_vol;

		ooff = sampleOffset;
		samples = chunk->sndChunk;

		for ( i = 0 ; i < count ; i++ ) {
			aoff = ooff;
			ooff = ooff + ch->dopplerScale;
			boff = ooff;
			fdata = 0;
			for ( j = aoff; j < boff; j++ ) {
				if ( j >= SND_CHUNK_SIZE ) {
					chunk = chunk->next;
					if ( !chunk ) {
						chunk = sc->soundData;
					}
					samples = chunk->sndChunk;
					ooff -= SND_CHUNK_SIZE;
				}
				fdata += samples[j & ( SND_CHUNK_SIZE - 1 )];
			}
			fdiv = 256 * ( boff - aoff );
			samp[i].left += ( fdata * fleftvol ) / fdiv;
			samp[i].right += ( fdata * frightvol ) / fdiv;
		}
	}
}

/*
===================
S_PaintChannelFromWavelet
===================
*/
void S_PaintChannelFromWavelet( channel_t *ch, sfx_t *sc, int count, int sampleOffset, int bufferOffset ) {
	int data;
	int leftvol, rightvol;
	int i;
	portable_samplepair_t   *samp;
	sndBuffer               *chunk;
	short                   *samples;

	leftvol = ch->leftvol * snd_vol;
	rightvol = ch->rightvol * snd_vol;

	i = 0;
	samp = &paintbuffer[ bufferOffset ];
	chunk = sc->soundData;
	while ( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) ) {
		chunk = chunk->next;
		sampleOffset -= ( SND_CHUNK_SIZE_FLOAT * 4 );
		i++;
	}

	if ( i != sfxScratchIndex || sfxScratchPointer != sc ) {
		decodeWavelet( chunk, sfxScratchBuffer );
		sfxScratchIndex = i;
		sfxScratchPointer = sc;
	}

	samples = sfxScratchBuffer;

	// FIXME: doppler

	for ( i = 0; i < count; i++ ) {
		if ( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) ) {
			chunk = chunk->next;
			decodeWavelet( chunk, sfxScratchBuffer );
			sfxScratchIndex++;
			sampleOffset = 0;
		}
		data  = samples[sampleOffset++];
		samp[i].left += ( data * leftvol ) >> 8;
		samp[i].right += ( data * rightvol ) >> 8;
	}
}

/*
===================
S_PaintChannelFromADPCM
===================
*/
void S_PaintChannelFromADPCM( channel_t *ch, sfx_t *sc, int count, int sampleOffset, int bufferOffset ) {
	int data;
	int leftvol, rightvol;
	int i;
	portable_samplepair_t   *samp;
	sndBuffer               *chunk;
	short                   *samples;

	leftvol = ch->leftvol * snd_vol;
	rightvol = ch->rightvol * snd_vol;

	i = 0;
	samp = &paintbuffer[ bufferOffset ];
	chunk = sc->soundData;

	if ( ch->doppler ) {
		sampleOffset = sampleOffset * ch->oldDopplerScale;
	}

	while ( sampleOffset >= ( SND_CHUNK_SIZE * 4 ) ) {
		chunk = chunk->next;
		sampleOffset -= ( SND_CHUNK_SIZE * 4 );
		i++;
	}

	if ( i != sfxScratchIndex || sfxScratchPointer != sc ) {
		S_AdpcmGetSamples( chunk, sfxScratchBuffer );
		sfxScratchIndex = i;
		sfxScratchPointer = sc;
	}

	samples = sfxScratchBuffer;
	for ( i = 0; i < count; i++ ) {
		if ( sampleOffset >= SND_CHUNK_SIZE * 4 ) {
			chunk = chunk->next;
			if ( !chunk ) {
				chunk = sc->soundData;
			}
			S_AdpcmGetSamples( chunk, sfxScratchBuffer );
			sampleOffset = 0;
			sfxScratchIndex++;
		}
		data = samples[sampleOffset++];
		samp[i].left += ( data * leftvol ) >> 8;
		samp[i].right += ( data * rightvol ) >> 8;
	}
}

/*
===================
S_PaintChannelFromMuLaw
===================
*/
void S_PaintChannelFromMuLaw( channel_t *ch, sfx_t *sc, int count, int sampleOffset, int bufferOffset ) {
	int data;
	int leftvol, rightvol;
	int i;
	portable_samplepair_t   *samp;
	sndBuffer               *chunk;
	byte                    *samples;
	float ooff;

	leftvol = ch->leftvol * snd_vol;
	rightvol = ch->rightvol * snd_vol;

	samp = &paintbuffer[ bufferOffset ];
	chunk = sc->soundData;
	while ( sampleOffset >= ( SND_CHUNK_SIZE * 2 ) ) {
		chunk = chunk->next;
		sampleOffset -= ( SND_CHUNK_SIZE * 2 );
		if ( !chunk ) {
			chunk = sc->soundData;
		}
	}

	if ( !ch->doppler ) {
		samples = (byte *)chunk->sndChunk + sampleOffset;
		for ( i = 0; i < count; i++ ) {
			if ( samples >= (byte *)chunk->sndChunk + ( SND_CHUNK_SIZE * 2 ) ) {
				chunk = chunk->next;
				samples = (byte *)chunk->sndChunk;
			}
			data  = mulawToShort[*samples];
			samp[i].left += ( data * leftvol ) >> 8;
			samp[i].right += ( data * rightvol ) >> 8;
			samples++;
		}
	} else {
		ooff = sampleOffset;
		samples = (byte *)chunk->sndChunk;
		for ( i = 0; i < count; i++ ) {
			if ( ooff >= SND_CHUNK_SIZE * 2 ) {
				chunk = chunk->next;
				if ( !chunk ) {
					chunk = sc->soundData;
				}
				samples = (byte *)chunk->sndChunk;
				ooff = 0.0;
			}
			data  = mulawToShort[samples[(int)( ooff )]];
			ooff = ooff + ch->dopplerScale;
			samp[i].left += ( data * leftvol ) >> 8;
			samp[i].right += ( data * rightvol ) >> 8;
		}
	}
}

#if !defined RTCW_MP
#define TALK_FUTURE_SEC 0.25        // go this far into the future (seconds)
#else
#define TALK_FUTURE_SEC 0.2     // go this far into the future (seconds)
#endif // RTCW_XX

#if defined RTCW_ET
//bani - cl_main.c
void CL_WriteWaveFilePacket( int endtime );
#endif // RTCW_XX

/*
===================
S_PaintChannels
===================
*/
void S_PaintChannels( int endtime ) {
	int i, si;
	int end;
	channel_t *ch;
	sfx_t   *sc;
	int ltime, count;
	int sampleOffset;
	streamingSound_t *ss;

#if !defined RTCW_MP
	qboolean firstPass = qtrue;
#endif // RTCW_XX

	if ( s_mute->value ) {
		snd_vol = 0;
	} else {
		snd_vol = s_volume->value * 256;
	}

#if !defined RTCW_MP
	if ( snd.volCurrent < 1 ) { // only when fading (at map start/end)
		snd_vol = (int)( (float)snd_vol * snd.volCurrent );
	}
#endif // RTCW_XX

	//Com_Printf ("%i to %i\n", s_paintedtime, endtime);
	while ( s_paintedtime < endtime ) {
		// if paintbuffer is smaller than DMA buffer
		// we may need to fill it multiple times
		end = endtime;
		if ( endtime - s_paintedtime > PAINTBUFFER_SIZE ) {

#if defined RTCW_SP
			Com_DPrintf( "endtime exceeds PAINTBUFFER_SIZE %i\n", endtime - s_paintedtime );
#elif defined RTCW_ET
			//%	Com_DPrintf("endtime exceeds PAINTBUFFER_SIZE %i\n", endtime - s_paintedtime);
#endif // RTCW_XX

			end = s_paintedtime + PAINTBUFFER_SIZE;
		}

		// clear paint buffer for the current time
		Com_Memset( paintbuffer, 0, ( end - s_paintedtime ) * sizeof( portable_samplepair_t ) );
		// mix all streaming sounds into paint buffer
		for ( si = 0, ss = streamingSounds; si < MAX_STREAMING_SOUNDS; si++, ss++ ) {
			// if this streaming sound is still playing
			if ( s_rawend[si] >= s_paintedtime ) {
				// copy from the streaming sound source
				int s;
				int stop;
//				float	fsir, fsil; // TTimo: unused

				stop = ( end < s_rawend[si] ) ? end : s_rawend[si];

				// precalculating this saves zillions of cycles
//DAJ				fsir = ((float)s_rawVolume[si].left/255.0f);
//DAJ				fsil = ((float)s_rawVolume[si].right/255.0f);
				for ( i = s_paintedtime ; i < stop ; i++ ) {
					s = i & ( MAX_RAW_SAMPLES - 1 );
//DAJ					paintbuffer[i-s_paintedtime].left += (int)((float)s_rawsamples[si][s].left * fsir);
//DAJ					paintbuffer[i-s_paintedtime].right += (int)((float)s_rawsamples[si][s].right * fsil);
					//DAJ even faster
					paintbuffer[i - s_paintedtime].left += ( s_rawsamples[si][s].left * s_rawVolume[si].left ) >> 8;
					paintbuffer[i - s_paintedtime].right += ( s_rawsamples[si][s].right * s_rawVolume[si].right ) >> 8;
				}

#ifdef TALKANIM

#if !defined RTCW_MP
				if ( firstPass && ss->channel == CHAN_VOICE && ss->entnum < MAX_CLIENTS ) {
#else
				if ( ss->channel == CHAN_VOICE && ss->entnum < MAX_CLIENTS ) {
#endif // RTCW_XX

					int talkcnt, talktime;
					int sfx_count, vstop;
					int data;

					// we need to go into the future, since the interpolated behaviour of the facial
					// animation creates lag in the time it takes to display the current facial frame
					talktime = s_paintedtime + (int)( TALK_FUTURE_SEC * (float)s_khz->integer * 1000 );
					vstop = ( talktime + 100 < s_rawend[si] ) ? talktime + 100 : s_rawend[si];
					talkcnt = 1;
					sfx_count = 0;

					for ( i = talktime ; i < vstop ; i++ ) {
						s = i & ( MAX_RAW_SAMPLES - 1 );
						data = c::abs( ( s_rawsamples[si][s].left ) / 8000 );
						if ( data > sfx_count ) {
							sfx_count = data;
						}
					}

					if ( sfx_count > 255 ) {
						sfx_count = 255;
					}
					if ( sfx_count < 25 ) {
						sfx_count = 0;
					}

					//Com_Printf("sfx_count = %d\n", sfx_count );

					// update the amplitude for this entity

#if !defined RTCW_ET
					s_entityTalkAmplitude[ss->entnum] = (unsigned char)sfx_count;
				}
#else
					// rain - the announcer is ent -1, so make sure we're >= 0
					if ( ss->entnum >= 0 ) {
						s_entityTalkAmplitude[ss->entnum] = (unsigned char)sfx_count;
					}
				}
#endif // RTCW_XX

#endif
			}
		}

		// paint in the channels.
		ch = s_channels;
		for ( i = 0; i < MAX_CHANNELS; i++, ch++ ) {
			if ( ch->startSample == START_SAMPLE_IMMEDIATE || !ch->thesfx || ( ch->leftvol < 0.25 && ch->rightvol < 0.25 ) ) {
				continue;
			}

			ltime = s_paintedtime;
			sc = ch->thesfx;

#if !defined RTCW_MP
			// (SA) hmm, why was this commented out?
			if ( !sc->inMemory ) {
				S_memoryLoad( sc );
#else
//			if (!sc->inMemory) {
//				S_memoryLoad(sc);
//			}
#endif // RTCW_XX

#if defined RTCW_MP
			// DHM - Nerve :: Somehow ch->startSample can get here with values around 0x40000000
			if ( ch->startSample > ltime ) {
				ch->startSample = ltime;
#endif // RTCW_XX

			}

#if defined RTCW_MP
			// dhm - end
#endif // RTCW_XX

			sampleOffset = ltime - ch->startSample;
			count = end - ltime;
			if ( sampleOffset + count > sc->soundLength ) {
				count = sc->soundLength - sampleOffset;
			}

			if ( count > 0 ) {
#ifdef TALKANIM
				// Ridah, talking animations
				// TODO: check that this entity has talking animations enabled!

#if !defined RTCW_MP
				if ( firstPass && ch->entchannel == CHAN_VOICE && ch->entnum < MAX_CLIENTS ) {
#else
				if ( ch->entchannel == CHAN_VOICE && ch->entnum < MAX_CLIENTS ) {
#endif // RTCW_XX

					int talkofs, talkcnt, talktime;
					// we need to go into the future, since the interpolated behaviour of the facial
					// animation creates lag in the time it takes to display the current facial frame
					talktime = ltime + (int)( TALK_FUTURE_SEC * (float)s_khz->integer * 1000 );
					talkofs = talktime - ch->startSample;
					talkcnt = 100;
					if ( talkofs + talkcnt < sc->soundLength ) {
						if ( sc->soundCompressionMethod == 1 ) {
							S_SetVoiceAmplitudeFromADPCM( sc, talkofs, talkcnt, ch->entnum );
						} else if ( sc->soundCompressionMethod == 2 ) {
							S_SetVoiceAmplitudeFromWavelet( sc, talkofs, talkcnt, ch->entnum );
						} else if ( sc->soundCompressionMethod == 3 ) {
							S_SetVoiceAmplitudeFromMuLaw( sc, talkofs, talkcnt, ch->entnum );
						} else {
							S_SetVoiceAmplitudeFrom16( sc, talkofs, talkcnt, ch->entnum );
						}
					}
				}
#endif
				if ( sc->soundCompressionMethod == 1 ) {
					S_PaintChannelFromADPCM( ch, sc, count, sampleOffset, ltime - s_paintedtime );
				} else if ( sc->soundCompressionMethod == 2 ) {
					S_PaintChannelFromWavelet( ch, sc, count, sampleOffset, ltime - s_paintedtime );
				} else if ( sc->soundCompressionMethod == 3 ) {
					S_PaintChannelFromMuLaw( ch, sc, count, sampleOffset, ltime - s_paintedtime );
				} else {
					S_PaintChannelFrom16( ch, sc, count, sampleOffset, ltime - s_paintedtime );
				}
			}
		}

		// paint in the looped channels.
		ch = loop_channels;
		for ( i = 0; i < numLoopChannels ; i++, ch++ ) {

#if !defined RTCW_ET
			sc = ch->thesfx;
#endif // RTCW_XX

			if ( !ch->thesfx || ( !ch->leftvol && !ch->rightvol ) ) {
				continue;
			}

			ltime = s_paintedtime;

#if defined RTCW_ET
			sc = ch->thesfx;
#endif // RTCW_XX


			if ( sc->soundData == NULL || sc->soundLength == 0 ) {
				continue;
			}
			// we might have to make two passes if it
			// is a looping sound effect and the end of
			// the sample is hit
			do {

#if !defined RTCW_ET
				sampleOffset = ( ltime % sc->soundLength );
#else
				//%	sampleOffset = (ltime % sc->soundLength);
				//%	sampleOffset = (ltime - ch->startSample) % sc->soundLength;	// ydnar
				sampleOffset = ( ltime /*- ch->startSample*/ ) % sc->soundLength; // ydnar
#endif // RTCW_XX


				count = end - ltime;
				if ( sampleOffset + count > sc->soundLength ) {
					count = sc->soundLength - sampleOffset;
				}

				if ( count > 0 ) {
#ifdef TALKANIM
					// Ridah, talking animations
					// TODO: check that this entity has talking animations enabled!

#if !defined RTCW_MP
					if ( firstPass && ch->entchannel == CHAN_VOICE && ch->entnum < MAX_CLIENTS ) {
#else
					if ( ch->entchannel == CHAN_VOICE && ch->entnum < MAX_CLIENTS ) {
#endif // RTCW_XX

						int talkofs, talkcnt, talktime;
						// we need to go into the future, since the interpolated behaviour of the facial
						// animation creates lag in the time it takes to display the current facial frame
						talktime = ltime + (int)( TALK_FUTURE_SEC * (float)s_khz->integer * 1000 );
						talkofs = talktime % sc->soundLength;
						talkcnt = 100;
						if ( talkofs + talkcnt < sc->soundLength ) {
							if ( sc->soundCompressionMethod == 1 ) {
								S_SetVoiceAmplitudeFromADPCM( sc, talkofs, talkcnt, ch->entnum );
							} else if ( sc->soundCompressionMethod == 2 ) {
								S_SetVoiceAmplitudeFromWavelet( sc, talkofs, talkcnt, ch->entnum );
							} else if ( sc->soundCompressionMethod == 3 ) {
								S_SetVoiceAmplitudeFromMuLaw( sc, talkofs, talkcnt, ch->entnum );
							} else {
								S_SetVoiceAmplitudeFrom16( sc, talkofs, talkcnt, ch->entnum );
							}
						}
					}
#endif
					if ( sc->soundCompressionMethod == 1 ) {
						S_PaintChannelFromADPCM( ch, sc, count, sampleOffset, ltime - s_paintedtime );
					} else if ( sc->soundCompressionMethod == 2 ) {
						S_PaintChannelFromWavelet( ch, sc, count, sampleOffset, ltime - s_paintedtime );
					} else if ( sc->soundCompressionMethod == 3 ) {
						S_PaintChannelFromMuLaw( ch, sc, count, sampleOffset, ltime - s_paintedtime );
					} else {
						S_PaintChannelFrom16( ch, sc, count, sampleOffset, ltime - s_paintedtime );
					}
					ltime += count;
				}
			} while ( ltime < end );
		}

		// transfer out according to DMA format
		S_TransferPaintBuffer( end );

#if defined RTCW_ET
		//bani
		CL_WriteWaveFilePacket( end );
#endif // RTCW_XX

		s_paintedtime = end;

#if !defined RTCW_MP
		firstPass = qfalse;
#endif // RTCW_XX

	}
}
