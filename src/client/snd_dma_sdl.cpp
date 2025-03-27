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


#include <algorithm>
#include <memory>
#include <vector>
#include "SDL.h"
#include "snd_local.h"


namespace {


typedef std::vector<Uint8> Buffer;


// SDL's audio buffer size in milliseconds.
const int k_mix_time_ms = 10;

// Engine's "paint" buffer size.
// FIXME Make PAINTBUFFER_SIZE public
// (2 channels * 16 bits * PAINTBUFFER_SIZE)
const int k_paint_size = 8192;

// How many mix chunks should contain DMA buffer.
const int k_mix_count = 8;

// Engine's audio buffer minimum size.
const int k_min_buffer_size = k_mix_count * k_paint_size;


bool sdl_is_initialized;
SDL_AudioDeviceID sdl_device_id;
Buffer sdl_buffer;
int sdl_buffer_size;
volatile int sdl_buffer_offset;
volatile bool sdl_mute_mixer;


int sdl_calculate_sample_size(
	int channel_count,
	int bits_per_channel)
{
	return channel_count * (bits_per_channel / 8);
}

int sdl_calculate_mix_size(
	int channel_count,
	int bits_per_channel,
	int sample_rate)
{
	int sample_size = sdl_calculate_sample_size(
		channel_count, bits_per_channel);
	int bytes_per_sec = sample_size * sample_rate;
	int exact_mix_size = (k_mix_time_ms * bytes_per_sec) / 1000;
	int mix_size = 1;

	while (mix_size < exact_mix_size)
		mix_size *= 2;

	return mix_size;
}

int sdl_calculate_buffer_size(
	int mix_size)
{
	int buffer_size = k_mix_count * mix_size;

	if (buffer_size < k_min_buffer_size)
		buffer_size = k_min_buffer_size;

	return buffer_size;
}

void SDLCALL sdl_callback(
	void* user_data,
	Uint8* stream,
	int length)
{
	int new_position = sdl_buffer_offset;

	if (sdl_mute_mixer) {
		std::fill_n(stream, length, 0);

		new_position += length;
		new_position %= sdl_buffer_size;
	} else {
		int length1 = std::min(length, sdl_buffer_size - new_position);
		int length2 = length - length1;

		std::copy(
			&sdl_buffer[new_position],
			&sdl_buffer[new_position] + length1,
			stream);

		new_position += length1;

		if (length2 > 0) {
			std::copy(
				&sdl_buffer[0],
				&sdl_buffer[0] + length2,
				stream);

			new_position = length2;
		} else
			new_position %= sdl_buffer_size;
	}

	sdl_buffer_offset = new_position;
}


} // namespace


void SNDDMA_Shutdown()
{
	if (!sdl_is_initialized)
		return;

	Com_DPrintf("Shutting down SDL audio system\n");

	sdl_is_initialized = false;

	SDL_PauseAudioDevice(sdl_device_id, true);
	SDL_CloseAudioDevice(sdl_device_id);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	Buffer().swap(sdl_buffer);

	dma.buffer = NULL;
}

qboolean SNDDMA_Init()
{
	SNDDMA_Shutdown();

	Com_Printf("Initialize audio\n");

	bool is_succeed = true;
	int sdl_result = 0;

	sdl_buffer_offset = 0;

	if (is_succeed) {
		Com_Printf("...initialize SDL audio subsystem\n");

		sdl_result = SDL_InitSubSystem(SDL_INIT_AUDIO);

		if (sdl_result != 0) {
			is_succeed = false;
			Com_Printf(S_COLOR_RED "...failed\n");
		}
	}


	int channel_count = 2;
	int bits_per_channel = 16;
	int sample_rate = 0;
	int mix_size = 0;
	int buffer_size = 0;

	if (is_succeed) {
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

		case 48:
			sample_rate = 48000;
			break;

		default:
			sample_rate = 22050;

			Com_Printf(S_COLOR_YELLOW "...unsupported value of sampling rate: s_khz = %d\n",
				s_khz->integer);

			Com_Printf(S_COLOR_YELLOW "...set default sampling rate to %d Hz\n",
				sample_rate);
			break;
		}

		mix_size = sdl_calculate_mix_size(
			channel_count, bits_per_channel, sample_rate);

		buffer_size = sdl_calculate_buffer_size(mix_size);
	}


	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;
	SDL_AudioDeviceID device_id = 0;

	if (is_succeed) {
		desired.freq = sample_rate;
		desired.format = AUDIO_S16LSB;
		desired.channels = channel_count;
		desired.samples = mix_size /
			sdl_calculate_sample_size(channel_count, bits_per_channel);
		desired.callback = sdl_callback;

		Com_Printf("...open audio device\n");

		device_id = SDL_OpenAudioDevice(
			NULL, false, &desired, &obtained, 0);

		if (device_id == 0) {
			is_succeed = false;
			Com_Printf(S_COLOR_RED "...failed\n");
		}
	}

	if (is_succeed) {
		if (obtained.freq != desired.freq ||
			obtained.format != desired.format ||
			obtained.channels != desired.channels ||
			obtained.samples != desired.samples)
		{
			is_succeed = false;

			if (obtained.freq != desired.freq) {
				Com_Printf(S_COLOR_RED "...sampling rate mismatch: %d\n",
					obtained.freq);
			}

			if (obtained.format != desired.format) {
				Com_Printf(S_COLOR_RED "...format mismatch: %d\n",
					obtained.format);
			}

			if (obtained.channels != desired.channels) {
				Com_Printf(S_COLOR_RED "...channel count mismatch: %d\n",
					obtained.channels);
			}

			if (obtained.samples != desired.samples) {
				Com_Printf(S_COLOR_RED "...sample count mismatch: %d\n",
					obtained.samples);
			}
		}
	}

	if (is_succeed) {
		sdl_is_initialized = true;

		dma.channels = channel_count;
		dma.samples = buffer_size / channel_count;
		dma.samplebits = bits_per_channel;
		dma.speed = sample_rate;
		dma.submission_chunk = 1;
		dma.buffer = NULL;

		sdl_device_id = device_id;
		sdl_buffer.resize(buffer_size);
		sdl_buffer_size = buffer_size;
		sdl_mute_mixer = false;

		SDL_PauseAudioDevice(sdl_device_id, false);

		return true;
	}

	if (device_id != 0)
		SDL_CloseAudioDevice(device_id);

	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	return false;
}

// Return the current sample position (in mono samples read)
// inside the recirculating dma buffer, so the mixing code will know
// how many sample are required to fill it up.
int SNDDMA_GetDMAPos()
{
	if (!sdl_is_initialized)
		return 0;

	return sdl_buffer_offset / dma.channels;
}

void SNDDMA_BeginPainting()
{
	if (!sdl_is_initialized)
		return;

	dma.buffer = &sdl_buffer[0];
}

void SNDDMA_Submit()
{
	dma.buffer = NULL;
}

void SNDDMA_Activate(bool value)
{
	sdl_mute_mixer = !value;
}
