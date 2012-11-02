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


//BBi
// *************************************************
// Low-level sound routines implemented via FMOD Ex.
// (Originaly was "win_snd.cpp")
// *************************************************
//BBi


#include <fmod.hpp>

#include "snd_local.h"


static const int FMOD_MIN_CHANNELS = 2;

// WARNING: Must be power of two!
static const int FMOD_DMA_BUFFER_SIZE = 65536;

static bool fmodIsInitialized;
static FMOD::System* fmodSystem;
static FMOD::Sound* fmodDmaSound;
static FMOD::Channel* fmodDmaChannel;


void SNDDMA_Shutdown ()
{
    ::Com_DPrintf ("Shutting down sound system\n");

    if (!fmodIsInitialized)
        return;


    if (fmodDmaChannel != 0) {
        fmodDmaChannel->stop ();
        fmodDmaChannel = 0;
    }

    if (fmodDmaSound != 0) {
        fmodDmaSound->release ();
        fmodDmaSound = 0;
    }

    if (fmodSystem != 0) {
        fmodSystem->release ();
        fmodSystem = 0;
    }
}

qboolean SNDDMA_Init ()
{
    if (fmodIsInitialized)
        ::SNDDMA_Shutdown ();


    ::Com_DPrintf ("Initializing FMOD Ex...\n");

    ::memset (&dma, 0, sizeof (dma_t));

    FMOD_RESULT fmodResult = FMOD_OK;
    FMOD::System* system = 0;

    fmodResult = FMOD::System_Create (&system);

    if (fmodResult != FMOD_OK)
        ::Com_Printf (S_COLOR_RED "FMOD: Failed to %s.\n", "create a system object");


    unsigned fmodVersion = 0;

    if (fmodResult == FMOD_OK) {
        fmodResult = system->getVersion (&fmodVersion);

        if (fmodResult == FMOD_OK) {
            if (fmodVersion < FMOD_VERSION) {
                fmodResult = FMOD_ERR_VERSION;
                ::Com_Printf (S_COLOR_RED "FMOD: Old version: %08x (requires %08x).\n",
                    fmodVersion, static_cast<unsigned> (FMOD_VERSION));
            }
        } else
            ::Com_Printf (S_COLOR_RED "FMOD: Failed to %s.\n", "get a version");
    }


#ifdef _WIN32
    int nDriver = 0;

    if (fmodResult == FMOD_OK) {
        fmodResult = system->getNumDrivers (&nDriver);

        if (fmodResult != FMOD_OK)
            ::Com_Printf (S_COLOR_RED "FMOD: Failed to %s.\n", "get a number of drivers");
    }

    if (fmodResult == FMOD_OK) {
        if (nDriver > 0) {
            FMOD_CAPS driverCaps = FMOD_CAPS_NONE;
            FMOD_SPEAKERMODE sysSpeakerMode = FMOD_SPEAKERMODE_RAW;

            fmodResult = system->getDriverCaps (0, &driverCaps, 0, &sysSpeakerMode);

            if (fmodResult != FMOD_OK)
                ::Com_Printf (S_COLOR_RED "FMOD: Failed to %s.\n", "get a driver caps");

            if ((fmodResult == FMOD_OK) &&
                ((driverCaps & FMOD_CAPS_HARDWARE_EMULATED) != 0))
            {
                ::Com_Printf (S_COLOR_YELLOW "FMOD: %s.\n", "Acceleration in control panel set to off");
                ::Com_Printf (S_COLOR_YELLOW "FMOD: %s...\n", "Adjusting internal mixing buffer size");

                fmodResult = system->setDSPBufferSize (1024, 10);

                if (fmodResult == FMOD_OK)
                    ::Com_Printf (S_COLOR_YELLOW "FMOD: ...%s.\n", "successed");
                else {
                    fmodResult = FMOD_ERR_INTERNAL;
                    ::Com_Printf (S_COLOR_RED "FMOD: ...%s.\n", "failed");
                }
            }

            if (fmodResult == FMOD_OK) {
                static const int MAX_DRIVER_NAME_SIZE = 256;
                char driverNameBuffer[MAX_DRIVER_NAME_SIZE];

                fmodResult = system->getDriverInfo (0, driverNameBuffer, MAX_DRIVER_NAME_SIZE, 0);

                if (fmodResult == FMOD_OK) {
                    driverNameBuffer[MAX_DRIVER_NAME_SIZE - 1] = '\0';

                    if (::strstr (driverNameBuffer, "SigmaTel") != 0) {
                        ::Com_Printf (S_COLOR_YELLOW "FMOD: %s...\n", "Working around SigmaTel driver...");

                        fmodResult = system->setSoftwareFormat (
                            48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);

                        if (fmodResult == FMOD_OK)
                            ::Com_Printf (S_COLOR_YELLOW "FMOD: ...%s.\n", "successed");
                        else
                            ::Com_Printf (S_COLOR_RED "FMOD: ...%s.\n", "failed");
                    }
                }
            }
        } else {
            fmodResult = system->setOutput (FMOD_OUTPUTTYPE_NOSOUND);

            if (fmodResult != FMOD_OK)
                ::Com_Printf (S_COLOR_RED "FMOD: Failed to %s.\n", "set an output mode");
        }
    }
#endif // _WIN32


    if (fmodResult == FMOD_OK) {
        fmodResult = system->init (FMOD_MIN_CHANNELS, FMOD_INIT_NORMAL, 0);

        if (fmodResult != FMOD_OK)
            ::Com_Printf (S_COLOR_RED "FMOD: Failed to %s.\n", "initialize a system object");
    }


#ifdef _WIN32
    if (fmodResult == FMOD_ERR_OUTPUT_CREATEBUFFER) {
        ::Com_Printf (S_COLOR_YELLOW "FMOD: %s.\n", "Selected speaker mode is not supported");
        ::Com_Printf (S_COLOR_YELLOW "FMOD: %s.\n", "Switching to stereo...");

        fmodResult = system->setSpeakerMode (FMOD_SPEAKERMODE_STEREO);

        if (fmodResult == FMOD_OK)
            ::Com_Printf (S_COLOR_YELLOW "FMOD: ...%s.\n", "successed");
        else
            ::Com_Printf (S_COLOR_RED "FMOD: ...%s.\n", "failed");

        if (fmodResult == FMOD_OK) {
            fmodResult = system->init (FMOD_MIN_CHANNELS, FMOD_INIT_NORMAL, 0);

            if (fmodResult != FMOD_OK)
                ::Com_Printf (S_COLOR_RED "FMOD: Failed to %s.\n",
                    "re-initialize a system object");
        }
    }
#endif // _WIN32


    int dmaRate = 0;

    if (fmodResult == FMOD_OK) {
        switch (s_khz->integer) {
        case 11:
            dmaRate = 11025;
            break;

        case 22:
            dmaRate = 22050;
            break;

        case 44:
            dmaRate = 44100;
            break;

        default:
            dmaRate = 22050;
            break;
        }
    }


    FMOD::Sound* sound = 0;

    if (fmodResult == FMOD_OK) {
        FMOD_MODE soundMode =
            FMOD_LOOP_NORMAL |
            FMOD_2D |
            FMOD_SOFTWARE |
            FMOD_OPENUSER |
            FMOD_OPENRAW;

        FMOD_CREATESOUNDEXINFO soundInfo;
        ::memset (&soundInfo, 0, sizeof (FMOD_CREATESOUNDEXINFO));
        soundInfo.cbsize = sizeof (FMOD_CREATESOUNDEXINFO);
        soundInfo.length = FMOD_DMA_BUFFER_SIZE;
        soundInfo.numchannels = 2;
        soundInfo.defaultfrequency = dmaRate;
        soundInfo.format = FMOD_SOUND_FORMAT_PCM16;

        fmodResult = system->createSound (
            0, soundMode, &soundInfo, &sound);

        if (fmodResult != FMOD_OK)
            ::Com_Printf (S_COLOR_RED "FMOD: Failed to %s.\n", "create a DMA sound object");
    }


    FMOD::Channel* channel = 0;

    if (fmodResult == FMOD_OK) {
        fmodResult = system->playSound (FMOD_CHANNEL_FREE, sound, false, &channel);

        if (fmodResult != FMOD_OK)
            ::Com_Printf (S_COLOR_RED "FMOD: Failed to %s.\n", "play a DMA sound");
    }


    if (fmodResult == FMOD_OK) {
        fmodIsInitialized = true;

        fmodSystem = system;
        fmodDmaSound = sound;
        fmodDmaChannel = channel;

        dma.channels = 2;
        dma.speed = dmaRate;
        dma.samplebits = 16;
        dma.submission_chunk = 1;
        dma.samples = FMOD_DMA_BUFFER_SIZE / 2;

        // Generate silence.
        ::SNDDMA_BeginPainting ();
        if (dma.buffer != 0)
            ::memset (dma.buffer, 0, FMOD_DMA_BUFFER_SIZE);
        ::SNDDMA_Submit ();

        ::Com_DPrintf ("Completed successfully\n");
    } else {
        if (channel != 0)
            channel->stop ();

        if (sound != 0)
            sound->release ();

        if (system != 0)
            system->release ();
    }

    return fmodIsInitialized;
}

// Return the current sample position (in mono samples read)
// inside the recirculating dma buffer, so the mixing code will know
// how many sample are required to fill it up.
int SNDDMA_GetDMAPos ()
{
    if (!fmodIsInitialized)
        return 0;


    FMOD_RESULT fmodResult = FMOD_OK;

    unsigned position = 0;

    fmodResult = fmodDmaChannel->getPosition (&position, FMOD_TIMEUNIT_PCMBYTES);

    if (fmodResult == FMOD_OK)
        return static_cast<int> (position / 2);

    return 0;
}

void SNDDMA_BeginPainting ()
{
    if (!fmodIsInitialized)
        return;


    assert (dma.buffer == 0);

    unsigned length;
    FMOD_RESULT fmodResult = FMOD_OK;

    fmodResult = fmodDmaSound->lock (0, FMOD_DMA_BUFFER_SIZE,
        reinterpret_cast<void**> (&dma.buffer), 0, &length, 0);
}

void SNDDMA_Submit ()
{
    if (!fmodIsInitialized)
        return;


    assert (dma.buffer != 0);

    FMOD_RESULT fmodResult = FMOD_OK;

    fmodResult = fmodDmaSound->unlock (dma.buffer, 0, FMOD_DMA_BUFFER_SIZE, 0);

    dma.buffer = 0;
}

void SNDDMA_Activate (
    bool isActive)
{
    if (!fmodIsInitialized)
        return;


    FMOD_RESULT fmodResult = FMOD_OK;

    fmodResult = fmodDmaChannel->setMute (!isActive);
}
