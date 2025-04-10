/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#ifndef __snd_public_h__
#define __snd_public_h__

void S_Init( void );
void S_Shutdown( void );
void S_UpdateThread( void );

#if defined RTCW_ET
void S_Reload( void );
#endif // RTCW_XX

// if origin is NULL, the sound will be dynamically sourced from the entity

#if !defined RTCW_ET
void S_StartSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx );
void S_StartSoundEx( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx, int flags );
void S_StartLocalSound( sfxHandle_t sfx, int channelNum );
#else
void S_StartSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx, int volume );
void S_StartSoundEx( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx, int flags, int volume );
void S_StartLocalSound( sfxHandle_t sfx, int channelNum, int volume );
#endif // RTCW_XX

#if !defined RTCW_MP
void S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime );
#else
void S_StartBackgroundTrack( const char *intro, const char *loop );
#endif // RTCW_XX

void S_StopBackgroundTrack( void );

#if !defined RTCW_MP
void S_QueueBackgroundTrack( const char *loop );            //----(SA)	added
void S_FadeStreamingSound( float targetvol, int time, int ssNum );  //----(SA)	added

#if !defined RTCW_ET
void S_FadeAllSounds( float targetvol, int time );    //----(SA)	added
#else
void S_FadeAllSounds( float targetvol, int time, qboolean stopsounds );   //----(SA)	added
#endif // RTCW_XX

#endif // RTCW_XX


#if !defined RTCW_ET
void S_StartStreamingSound( const char *intro, const char *loop, int entnum, int channel, int attenuation );
#else
float S_StartStreamingSound( const char *intro, const char *loop, int entnum, int channel, int attenuation );
#endif // RTCW_XX

void S_StopStreamingSound( int index );

#if !defined RTCW_MP
void S_StopEntStreamingSound( int entNum ); //----(SA)	added
#endif // RTCW_XX

#if defined RTCW_ET
void S_AddLoopSounds( void );
#endif // RTCW_XX

// cinematics and voice-over-network will send raw samples
// 1.0 volume will be direct output of source samples
void S_RawSamples( int samples, int rate, int width, int s_channels,
				   const byte *data, float lvol, float rvol, int streamingIndex );

// stop all sounds and the background track
void S_StopAllSounds( void );

// all continuous looping sounds must be added before calling S_Update
void S_ClearLoopingSounds( void );

#if !defined RTCW_MP
void S_ClearSounds( qboolean clearStreaming, qboolean clearMusic ); //----(SA)	modified
#endif // RTCW_XX

#if !defined RTCW_ET
void S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, const int range, sfxHandle_t sfxHandle, int volume );
void S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, const int range, sfxHandle_t sfx );
void S_StopLoopingSound( int entityNum );
#else
void S_AddLoopingSound( const vec3_t origin, const vec3_t velocity, const int range, sfxHandle_t sfxHandle, int volume, int soundTime );
void S_AddRealLoopingSound( const vec3_t origin, const vec3_t velocity, const int range, sfxHandle_t sfxHandle, int volume, int soundTime );
#endif // RTCW_XX

// recompute the reletive volumes for all running sounds
// reletive to the given entityNum / orientation
void S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );

// let the sound system know where an entity currently is
void S_UpdateEntityPosition( int entityNum, const vec3_t origin );

#if defined RTCW_ET
void S_Update_Debug( void );
#endif // RTCW_XX

void S_Update( void );

void S_DisableSounds( void );

void S_BeginRegistration( void );

// RegisterSound will allways return a valid sample, even if it
// has to create a placeholder.  This prevents continuous filesystem
// checks for missing files
sfxHandle_t S_RegisterSound( const char *sample, qboolean compressed );

void S_DisplayFreeMemory( void );

//
int S_GetVoiceAmplitude( int entityNum );

#if defined RTCW_MP
typedef struct {
	int left;           // the final values will be clamped to +/- 0x00ffff00 and shifted down
	int right;
} portable_samplepair_t;
#endif // RTCW_XX

#if defined RTCW_ET
int S_GetSoundLength( sfxHandle_t sfxHandle );
int S_GetCurrentSoundTime( void );
#endif // RTCW_XX

#endif  // __snd_public_h__
