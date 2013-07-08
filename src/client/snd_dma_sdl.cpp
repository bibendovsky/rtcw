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
// Low-level sound routines implemented via SDL2.
// (Former "win_snd.cpp")
// ************************************************
// BBi


#include <cassert>

#include <algorithm>

#include "SDL.h"

#include "snd_local.h"


namespace {


// WARNING! Must be power of two.
const int SDL_DMA_BUFFER_SIZE = 65536;

bool sdl_is_initialized = false;

bool sdl_mute = false;
int sdl_dma_position;

void sdl_audio_callback(
    void* userdata,
    Uint8* stream,
    int len)
{
    if (sdl_mute) {
        std::uninitialized_fill_n(stream, len, 0);
        return;
    }

    if (sdl_dma_position < 0 || sdl_dma_position > SDL_DMA_BUFFER_SIZE)
        sdl_dma_position = 0;

    auto remain_length = SDL_DMA_BUFFER_SIZE - sdl_dma_position;
    auto length1 = std::min(len, remain_length);
    auto length2 = len - length1;

    auto src_data = &dma.buffer[sdl_dma_position];

    std::uninitialized_copy_n(src_data, length1, stream);
    sdl_dma_position += length1;

    if (length2 > 0) {
        std::uninitialized_copy_n(dma.buffer, length2, &stream[length1]);
        sdl_dma_position = length2;
    }
}


} // namespace


void SNDDMA_Shutdown()
{
    ::Com_DPrintf("Shutting down sound system\n");

    if (!sdl_is_initialized)
        return;


    ::SDL_PauseAudio(true);

    delete [] dma.buffer;
    dma.buffer = nullptr;

    ::SDL_QuitSubSystem(SDL_INIT_AUDIO);

    sdl_is_initialized = false;
}

qboolean SNDDMA_Init()
{
    if (sdl_is_initialized)
        ::SNDDMA_Shutdown();

    ::Com_Printf("Initializing SDL audio subsystem...\n");

    auto sdl_result = 0;
    auto is_succeed = true;

    sdl_dma_position = 0;

    if (is_succeed) {
        sdl_result = ::SDL_InitSubSystem(SDL_INIT_AUDIO);

        if (sdl_result != 0) {
            is_succeed = false;
            ::Com_Printf(S_COLOR_RED "%s\n", ::SDL_GetError());
        }
    }


    auto frequency = 0;

    if (is_succeed) {
        switch (s_khz->integer) {
        case 11:
            frequency = 11025;
            break;

        case 22:
            frequency = 22050;
            break;

        case 44:
            frequency = 44100;
            break;

        default:
            frequency = 22050;
            ::Com_Printf(S_COLOR_YELLOW "Unsupported frequency: %s.\n",
                s_khz->string);
            ::Com_Printf(S_COLOR_YELLOW "Defaulting to 22 (22050 KHz).\n");
            break;
        }

        SDL_AudioSpec spec;
        SDL_zero(spec);
        spec.callback = sdl_audio_callback;
        spec.channels = 2;
        spec.format = AUDIO_S16;
        spec.freq = frequency;
        spec.samples = 512;

        sdl_result = ::SDL_OpenAudio(&spec, nullptr);

        if (sdl_result != 0) {
            is_succeed = false;
            ::Com_Printf(S_COLOR_RED "Failed to open an audio: %s\n",
                ::SDL_GetError());
        }
    }

    if (is_succeed) {
        sdl_is_initialized = true;

        dma.buffer = new byte[SDL_DMA_BUFFER_SIZE];
        dma.channels = 2;
        dma.samplebits = 16;
        dma.samples = SDL_DMA_BUFFER_SIZE / 2;
        dma.speed = frequency;
        dma.submission_chunk = 1;

        std::uninitialized_fill_n(dma.buffer, SDL_DMA_BUFFER_SIZE, 0);

        ::SDL_PauseAudio(false);

        return true;
    }

    ::SDL_QuitSubSystem(SDL_INIT_AUDIO);

    return false;
}

// Return the current sample position (in mono samples read)
// inside the recirculating dma buffer, so the mixing code will know
// how many sample are required to fill it up.
int SNDDMA_GetDMAPos()
{
    return sdl_dma_position / 2;
}

void SNDDMA_BeginPainting()
{
    if (sdl_is_initialized)
        ::SDL_LockAudio();
}

void SNDDMA_Submit()
{
    if (sdl_is_initialized)
        ::SDL_UnlockAudio();
}

void SNDDMA_Activate(bool value)
{
    sdl_mute = !value;
}
