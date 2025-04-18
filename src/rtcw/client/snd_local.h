/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// snd_local.h -- private sound definations


#include "q_shared.h"
#include "qcommon.h"
#include "snd_public.h"

#define PAINTBUFFER_SIZE        4096                    // this is in samples

#define SND_CHUNK_SIZE          1024                    // samples
#define SND_CHUNK_SIZE_FLOAT    ( SND_CHUNK_SIZE / 2 )      // floats
#define SND_CHUNK_SIZE_BYTE     ( SND_CHUNK_SIZE * 2 )      // floats

#if !defined RTCW_MP
#define TALKANIM
#else
//#define TALKANIM			// NERVE - SMF - we don't want this for multiplayer
#endif // RTCW_XX

#if !defined RTCW_MP
typedef struct {
	int left;           // the final values will be clamped to +/- 0x00ffff00 and shifted down
	int right;
} portable_samplepair_t;
#endif // RTCW_XX

typedef struct adpcm_state {
	short sample;       /* Previous output value */
	char index;         /* Index into stepsize table */
} adpcm_state_t;

typedef struct sndBuffer_s {
	short sndChunk[SND_CHUNK_SIZE];
	struct sndBuffer_s      *next;
	int size;
	adpcm_state_t adpcm;
} sndBuffer;

typedef struct sfx_s {
	sndBuffer       *soundData;
	qboolean defaultSound;                  // couldn't be loaded, so use buzz
	qboolean inMemory;                      // not in Memory
	qboolean soundCompressed;               // not in Memory
	int soundCompressionMethod;
	int soundLength;
	char soundName[MAX_QPATH];
	int lastTimeUsed;
	struct sfx_s    *next;
} sfx_t;

typedef struct {
	int channels;
	int samples;                        // mono samples in buffer
	int submission_chunk;               // don't mix less than this #
	int samplebits;
	int speed;
	int samplepos;
	byte        *buffer;
} dma_t;

#define START_SAMPLE_IMMEDIATE  0x7fffffff

typedef struct loopSound_s {
	vec3_t origin;
	vec3_t velocity;

#if !defined RTCW_ET
	float range;            //----(SA)	added
#endif // RTCW_XX

	sfx_t       *sfx;
	int mergeFrame;

#if defined RTCW_ET
	qboolean active;
	qboolean kill;
	qboolean doppler;
	float dopplerScale;
	float oldDopplerScale;
	int framenum;
	float range;            //----(SA)	added
#endif // RTCW_XX

	int vol;
	qboolean loudUnderWater;    // (SA) set if this sound should be played at full vol even when under water (under water loop sound for ex.)

#if defined RTCW_MP
	qboolean doppler;
	float dopplerScale;
	float oldDopplerScale;
	int framenum;
#endif // RTCW_XX

#if defined RTCW_ET
	int startTime, startSample;         // ydnar: so looping sounds can be out of phase
#endif // RTCW_XX

} loopSound_t;

typedef struct
{

#if defined RTCW_SP
	int         *ptr;           //DAJ BUGFIX for freelist/endlist pointer
#else
	int         *prt;           //DAJ BUGFIX for freelist/endlist pointer
#endif // RTCW_XX

	int allocTime;
	int startSample;            // START_SAMPLE_IMMEDIATE = set immediately on next mix
	int entnum;                 // to allow overriding a specific sound
	int entchannel;             // to allow overriding a specific sound
	int leftvol;                // 0-255 volume after spatialization
	int rightvol;               // 0-255 volume after spatialization
	int master_vol;             // 0-255 volume before spatialization
	float dopplerScale;
	float oldDopplerScale;
	vec3_t origin;              // only use if fixed_origin is set
	qboolean fixed_origin;      // use origin instead of fetching entnum's origin
	sfx_t       *thesfx;        // sfx structure
	qboolean doppler;
	int flags;                  //----(SA)	added
	qboolean threadReady;
} channel_t;


#define WAV_FORMAT_PCM      1


typedef struct {
	int format;
	int rate;
	int width;
	int channels;
	int samples;
	int dataofs;                // chunk starts this many bytes from file start
} wavinfo_t;


/*
====================================================================

  SYSTEM SPECIFIC FUNCTIONS

====================================================================
*/

// initializes cycling through a DMA buffer and returns information on it
qboolean SNDDMA_Init( void );

// gets the current DMA position
int     SNDDMA_GetDMAPos( void );

// shutdown the DMA xfer.
void    SNDDMA_Shutdown( void );

void    SNDDMA_BeginPainting( void );

void    SNDDMA_Submit( void );

//====================================================================

#define MAX_CHANNELS 96

extern channel_t s_channels[MAX_CHANNELS];
extern channel_t loop_channels[MAX_CHANNELS];
extern int numLoopChannels;

extern int s_paintedtime;
extern vec3_t listener_forward;
extern vec3_t listener_right;
extern vec3_t listener_up;
extern dma_t dma;

#ifdef TALKANIM
extern unsigned char s_entityTalkAmplitude[MAX_CLIENTS];
#endif

#if !defined RTCW_MP
//----(SA)	some flags for queued music tracks
#define QUEUED_PLAY_ONCE    -1
#define QUEUED_PLAY_LOOPED  -2
#define QUEUED_PLAY_ONCE_SILENT -3  // when done it goes quiet
//----(SA)	end
#endif // RTCW_XX

// Ridah, streaming sounds
typedef struct {
	fileHandle_t file;
	wavinfo_t info;
	int samples;

#if !defined RTCW_MP
	char name[MAX_QPATH];           //----(SA)	added
#endif // RTCW_XX

	char loop[MAX_QPATH];

#if !defined RTCW_MP
	int looped;                 //----(SA)	added
#endif // RTCW_XX

	int entnum;
	int channel;
	int attenuation;

#if !defined RTCW_MP
	int kill;           //----(SA)	changed

	int fadeStart;              //----(SA)	added
	int fadeEnd;                //----(SA)	added
	float fadeStartVol;         //----(SA)	added
	float fadeTargetVol;        //----(SA)	added
#elif defined RTCW_MP
	qboolean kill;
#endif // RTCW_XX

} streamingSound_t;



#if !defined RTCW_MP
typedef struct {
	vec3_t origin;
	qboolean fixedOrigin;
	int entityNum;
	int entityChannel;
	sfxHandle_t sfx;
	int flags;

#if defined RTCW_ET
	int volume;
#endif // RTCW_XX

} s_pushStack;

#define MAX_PUSHSTACK   64
#define LOOP_HASH       128

#if !defined RTCW_ET
#define MAX_LOOP_SOUNDS 128
#else
#define MAX_LOOP_SOUNDS 1024
#endif // RTCW_XX


// removed many statics into a common sound struct
typedef struct {
	sfx_t       *sfxHash[LOOP_HASH];
	int numLoopSounds;
	loopSound_t loopSounds[MAX_LOOP_SOUNDS];

	float volTarget;
	float volStart;
	int volTime1;
	int volTime2;
	float volFadeFrac;
	float volCurrent;

#if defined RTCW_ET
	qboolean stopSounds;
#endif // RTCW_XX

	channel_t   *freelist;
	channel_t   *endflist;

	int s_numSfx;

	s_pushStack pushPop[MAX_PUSHSTACK];
	int tart;

	qboolean s_soundPainted;
	int s_clearSoundBuffer;

	int s_soundStarted;

#if !defined RTCW_ET
//	qboolean	s_soundMute;
#endif // RTCW_XX

	int s_soundMute;                // 0 - not muted, 1 - muted, 2 - no new sounds, but play out remaining sounds (so they can die if necessary)

	vec3_t entityPositions[MAX_GENTITIES];

	char nextMusicTrack[MAX_QPATH];         // extracted from CS_MUSIC_QUEUE //----(SA)	added
	int nextMusicTrackType;
} snd_t;

extern snd_t snd;   // globals for sound
#endif // RTCW_XX



#if !defined RTCW_MP
#define MAX_STREAMING_SOUNDS    12  // need to keep it low, or the rawsamples will get too big
#else
#define MAX_STREAMING_SOUNDS    24  // need to keep it low, or the rawsamples will get too big
#endif // RTCW_XX

#define MAX_RAW_SAMPLES         16384

extern streamingSound_t streamingSounds[MAX_STREAMING_SOUNDS];
extern int s_rawend[MAX_STREAMING_SOUNDS];
extern portable_samplepair_t s_rawsamples[MAX_STREAMING_SOUNDS][MAX_RAW_SAMPLES];
extern portable_samplepair_t s_rawVolume[MAX_STREAMING_SOUNDS];


extern cvar_t   *s_volume;
extern cvar_t   *s_nosound;
extern cvar_t   *s_khz;
extern cvar_t   *s_show;
extern cvar_t   *s_mixahead;
extern cvar_t   *s_mute;

extern cvar_t   *s_testsound;
extern cvar_t   *s_separation;

#if !defined RTCW_MP
extern cvar_t   *s_currentMusic;    //----(SA)	added
extern cvar_t   *s_debugMusic;      //----(SA)	added
#endif // RTCW_XX

#if defined RTCW_ET
// fretn
extern cvar_t   *s_bits;
extern cvar_t   *s_numchannels;
#endif // RTCW_XX

qboolean S_LoadSound( sfx_t *sfx );

void        SND_free( sndBuffer *v );
sndBuffer*  SND_malloc();
void        SND_setup();

void S_PaintChannels( int endtime );

void S_memoryLoad( sfx_t *sfx );
portable_samplepair_t *S_GetRawSamplePointer();

// spatializes a channel
void S_Spatialize( channel_t *ch );

// adpcm functions
int  S_AdpcmMemoryNeeded( const wavinfo_t *info );
void S_AdpcmEncodeSound( sfx_t *sfx, short *samples );
void S_AdpcmGetSamples( sndBuffer *chunk, short *to );

// wavelet function

#define SENTINEL_MULAW_ZERO_RUN 127
#define SENTINEL_MULAW_FOUR_BIT_RUN 126

void S_FreeOldestSound();

#define NXStream byte

void encodeWavelet( sfx_t *sfx, short *packets );
void decodeWavelet( sndBuffer *stream, short *packets );

void encodeMuLaw( sfx_t *sfx, short *packets );
extern short mulawToShort[256];

extern short *sfxScratchBuffer;
extern const sfx_t *sfxScratchPointer;
extern int sfxScratchIndex;

#if !defined RTCW_MP
extern unsigned char s_entityTalkAmplitude[MAX_CLIENTS];

extern float S_GetStreamingFade( streamingSound_t *ss );    //----(SA)	added
#endif // RTCW_XX

