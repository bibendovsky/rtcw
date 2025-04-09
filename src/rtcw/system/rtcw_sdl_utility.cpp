#include "rtcw_sdl_utility.h"
#include <stddef.h>
#include <string.h>
#include <algorithm>
#include "SDL.h"

// ==========================================================================

namespace rtcw {

SdlSystem::~SdlSystem()
{
	SDL_Quit();
}

// ==========================================================================

SdlSubsystem::SdlSubsystem()
	:
	subsystem_id_()
{}

SdlSubsystem::SdlSubsystem(Uint32 subsystem_id)
	:
	subsystem_id_(subsystem_id)
{}

SdlSubsystem::~SdlSubsystem()
{
	reset();
}

void SdlSubsystem::reset()
{
	if (subsystem_id_ == 0)
	{
		return;
	}

	SDL_QuitSubSystem(subsystem_id_);
	subsystem_id_ = 0;
}

void SdlSubsystem::reset(Uint32 subsystem_id)
{
	if (subsystem_id_ != 0)
	{
		SDL_QuitSubSystem(subsystem_id_);
	}

	subsystem_id_ = subsystem_id;
}

void SdlSubsystem::swap(SdlSubsystem& rhs)
{
	std::swap(subsystem_id_, rhs.subsystem_id_);
}

// ==========================================================================

void SdlUPtrSurfaceDeleter::operator()(SDL_Surface* pointer) const
{
	SDL_FreeSurface(pointer);
}

// ==========================================================================

void SdlUPtrRendererDeleter::operator()(SDL_Renderer* pointer) const
{
	SDL_DestroyRenderer(pointer);
}

// ==========================================================================

void SdlUPtrTextureDeleter::operator()(SDL_Texture* pointer) const
{
	SDL_DestroyTexture(pointer);
}

// ==========================================================================

void SdlUPtrWindowDeleter::operator()(SDL_Window* pointer) const
{
	SDL_DestroyWindow(pointer);
}

} // namespace rtcw
