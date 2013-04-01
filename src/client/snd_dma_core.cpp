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


// BBi
// ************************************************
// Low-level sound routines implemented via OpenAL.
// (Former "win_snd.cpp")
// ************************************************
// BBi


#include <cassert>

#include <algorithm>
#include <deque>
#include <vector>
#include <queue>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "snd_local.h"


namespace {


typedef std::queue<ALuint> OalQueue;
typedef std::vector<bbi::UChar> OalBuffer;

const int OAL_MAX_QUEUE_SIZE = 2;

// WARNING: Must be power of two!
const int OAL_MIX_BUFFER_SIZE = 65536;

const int OAL_SLICE_SIZE = PAINTBUFFER_SIZE;

bool oal_is_initialized = false;
OalQueue oal_free_buffers;
OalQueue oal_used_buffers;
ALCdevice* oal_device = nullptr;
ALCcontext* oal_context = nullptr;
ALuint oal_source = AL_NONE;

ALuint oal_looped_source = AL_NONE;
ALuint oal_looped_buffer = AL_NONE;

bool oal_has_ext_buffer_subdata = false;
PFNALBUFFERSUBDATASOFTPROC oal_buffer_subdata = nullptr;

// We need it for overlap case.
OalBuffer oal_slice_buffer;

OalBuffer oal_mix_buffer;

int oal_position = 0;

void oal_uninitialize ()
{
    if (oal_context != nullptr) {
        ::alcMakeContextCurrent (nullptr);

        if (oal_looped_source != AL_NONE) {
            ::alSourceStop (oal_looped_source);
            ::alDeleteSources (1, &oal_looped_source);
            oal_looped_source = AL_NONE;
        }

        if (oal_looped_buffer != AL_NONE) {
            ::alDeleteBuffers (1, &oal_looped_buffer);
            oal_looped_buffer = AL_NONE;
        }


        if (oal_source != AL_NONE) {
            ::alSourceStop (oal_source);
            ::alDeleteSources (1, &oal_source);
            oal_source = AL_NONE;
        }

        while (!oal_free_buffers.empty ()) {
            auto buffer = oal_free_buffers.front ();
            ::alDeleteBuffers (1, &buffer);
            oal_free_buffers.pop ();
        }

        while (!oal_used_buffers.empty ()) {
            auto buffer = oal_used_buffers.front ();
            ::alDeleteBuffers (1, &buffer);
            oal_used_buffers.pop ();
        }

        ::alcDestroyContext (oal_context);
        oal_context = nullptr;
    }

    if (oal_device != nullptr) {
        ::alcCloseDevice (oal_device);
        oal_device = nullptr;
    }

    oal_has_ext_buffer_subdata = false;
    oal_buffer_subdata = nullptr;

    oal_is_initialized = false;
}

void oal_detect_extension_soft_buffer_sub_data ()
{
    oal_has_ext_buffer_subdata = (::alIsExtensionPresent (
        "AL_SOFT_buffer_sub_data") == AL_TRUE);

    if (!oal_has_ext_buffer_subdata)
        return;

    oal_buffer_subdata = reinterpret_cast<PFNALBUFFERSUBDATASOFTPROC> (
        ::alGetProcAddress ("alBufferSubDataSOFT"));

    if (oal_buffer_subdata != nullptr)
        return;

    oal_has_ext_buffer_subdata = false;
}


} // namespace


void SNDDMA_Shutdown ()
{
    ::Com_DPrintf ("Shutting down sound system\n");

    oal_uninitialize ();
}

qboolean SNDDMA_Init ()
{
    if (oal_is_initialized)
        ::SNDDMA_Shutdown ();

    ::Com_Printf ("Initializing OpenAL...\n");

    bool is_succeed = true;

    oal_device = ::alcOpenDevice (nullptr);

    if (oal_device == nullptr) {
        is_succeed = false;
        ::Com_Printf (S_COLOR_YELLOW "Failed to open a device.\n");
    }

    if (is_succeed) {
        oal_context = ::alcCreateContext (oal_device, nullptr);

        if (oal_context == nullptr) {
            is_succeed = false;
            ::Com_Printf (S_COLOR_YELLOW "Failed to open a context.\n");
        }
    }

    if (is_succeed) {
        auto is_made_current = ::alcMakeContextCurrent (oal_context);

        if (is_made_current == ALC_FALSE) {
            is_succeed = false;
            ::Com_Printf (S_COLOR_YELLOW "Failed to make a context current.\n");
        }
    }


    oal_mix_buffer.clear ();
    oal_mix_buffer.resize (OAL_MIX_BUFFER_SIZE);

    int sample_rate = 0;

    switch (s_khz->integer) {
    case 11:
        sample_rate = 11025;
        break;

    case 22:
        sample_rate = 22050;
        break;

    case 44:
        sample_rate = 44100;
        break;

    default:
        sample_rate = 22050;
        break;
    }


    ALenum oal_result = ::alGetError ();

    if (is_succeed)
        oal_detect_extension_soft_buffer_sub_data ();

    if (is_succeed) {
        ::alGenBuffers (1, &oal_looped_buffer);
        oal_result = ::alGetError ();

        if (oal_result != AL_NO_ERROR) {
            is_succeed = false;
            ::Com_Printf (S_COLOR_YELLOW "Failed to create a buffer object.\n");
        }
    }

    if (is_succeed) {
        ::alBufferData (oal_looped_buffer, AL_FORMAT_STEREO16,
            &oal_mix_buffer[0], OAL_MIX_BUFFER_SIZE, sample_rate);
        oal_result = ::alGetError ();

        if (oal_result != AL_NO_ERROR) {
            is_succeed = false;
            ::Com_Printf (S_COLOR_YELLOW "Failed to initialize a buffer with data.\n");
        }
    }

    if (is_succeed) {
        ::alGenSources (1, &oal_looped_source);
        oal_result = ::alGetError ();

        if (oal_result != AL_NO_ERROR) {
            is_succeed = false;
            ::Com_Printf (S_COLOR_YELLOW "Failed to create a source object.\n");
        }
    }

    if (is_succeed) {
        ::alSourcei (oal_looped_source, AL_LOOPING, AL_TRUE);
        ::alSourcei (oal_looped_source, AL_BUFFER,
            static_cast<ALint> (oal_looped_buffer));
        ::alSourcePlay (oal_looped_source);
        oal_result = ::alGetError ();

        if (oal_result != AL_NO_ERROR) {
            is_succeed = false;
            ::Com_Printf (S_COLOR_YELLOW "Failed to play a source.\n");
        }
    }

    if (is_succeed && (!oal_has_ext_buffer_subdata)) {
        ::alGenSources (1, &oal_source);
        oal_result = ::alGetError ();

        if (oal_result != AL_NO_ERROR) {
            is_succeed = false;
            ::Com_Printf (S_COLOR_YELLOW "Failed to create a source object.\n");
        }
    }

    if (is_succeed && (!oal_has_ext_buffer_subdata)) {
        ALuint buffers[OAL_MAX_QUEUE_SIZE];

        ::alGenBuffers (OAL_MAX_QUEUE_SIZE, buffers);
        oal_result = ::alGetError ();

        if (oal_result == AL_NO_ERROR) {
            for (auto buffer : buffers)
                oal_free_buffers.push (buffer);
        } else {
            is_succeed = false;
            ::Com_Printf (S_COLOR_YELLOW "Failed to create buffer objects.\n");
        }
    }

    if (is_succeed) {
        std::uninitialized_fill_n (
            reinterpret_cast<char*> (&dma), sizeof (dma_t), 0);

        dma.channels = 2;
        dma.samplebits = 16;
        dma.submission_chunk = 1;
        dma.samples = OAL_MIX_BUFFER_SIZE / 2;
        dma.speed = sample_rate;

        if  (oal_has_ext_buffer_subdata)
            ::Com_Printf ("... using AL_SOFT_buffer_sub_data extension.\n");
        else
            oal_slice_buffer.resize (OAL_SLICE_SIZE);

        oal_is_initialized = true;

        ::Com_Printf ("Completed successfully.\n");
    } else
        oal_uninitialize ();

    return oal_is_initialized;
}

// Return the current sample position (in mono samples read)
// inside the recirculating dma buffer, so the mixing code will know
// how many sample are required to fill it up.
int SNDDMA_GetDMAPos ()
{
    if (!oal_is_initialized)
        return 0;

    ALint offset = 0;
    ::alGetSourcei (oal_looped_source, AL_BYTE_OFFSET, &offset);

    oal_position = offset;

    if (oal_has_ext_buffer_subdata)
        return oal_position / 2;


    ALint queued = 0;
    ::alGetSourcei (oal_source, AL_BUFFERS_QUEUED, &queued);

    ALint processed = 0;
    ::alGetSourcei (oal_source, AL_BUFFERS_PROCESSED, &processed);

    for (int i = 0; i < processed; ++i) {
        ALuint buffer = AL_NONE;
        ::alSourceUnqueueBuffers (oal_source, 1, &buffer);

        if (buffer == AL_NONE)
            continue;

        assert (oal_used_buffers.front () == buffer);

        oal_free_buffers.push (buffer);
        oal_used_buffers.pop ();
    }

    if (queued < OAL_MAX_QUEUE_SIZE) {
        int read_offset = (oal_position / OAL_SLICE_SIZE) * OAL_SLICE_SIZE;

        ALuint buffer = oal_free_buffers.front ();
        oal_free_buffers.pop ();
        oal_used_buffers.push (buffer);

        bbi::UChar* data = nullptr;

        if ((read_offset + OAL_SLICE_SIZE) <= OAL_MIX_BUFFER_SIZE)
            data = &oal_mix_buffer[read_offset];
        else {
            int size1 = OAL_MIX_BUFFER_SIZE - read_offset;
            int size2 = OAL_SLICE_SIZE - size1;

            std::uninitialized_copy_n (
                &oal_mix_buffer[read_offset],
                size1,
                &oal_slice_buffer[0]);

            std::uninitialized_copy_n (
                &oal_mix_buffer[0],
                size2,
                &oal_slice_buffer[size1]);

            data = &oal_slice_buffer[0];
        }

        ::alBufferData (buffer, AL_FORMAT_STEREO16,
            data, OAL_SLICE_SIZE, dma.speed);

        ::alSourceQueueBuffers (oal_source, 1, &buffer);
    }

    ALint state = AL_STOPPED;
    ::alGetSourcei (oal_source, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING)
        ::alSourcePlay (oal_source);


    return oal_position / 2;
}

void SNDDMA_BeginPainting ()
{
    if (!oal_is_initialized)
        return;

    assert (dma.buffer == nullptr);

    dma.buffer = &oal_mix_buffer[0];
}

void SNDDMA_Submit ()
{
    if (!oal_is_initialized)
        return;

    assert (dma.buffer != nullptr);

    if (oal_has_ext_buffer_subdata) {
        oal_buffer_subdata (oal_looped_buffer, AL_FORMAT_STEREO16,
            dma.buffer, 0, OAL_MIX_BUFFER_SIZE);
    }

    dma.buffer = nullptr;
}

void SNDDMA_Activate (bool value)
{
    if (!oal_is_initialized)
        return;

    float volume = value ? 1.0F : 0.0F;

    ::alListenerf (AL_GAIN, volume);
}
