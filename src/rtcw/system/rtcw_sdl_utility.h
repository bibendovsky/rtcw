#ifndef RTCW_SDL_UTILITY_INCLUDED
#define RTCW_SDL_UTILITY_INCLUDED

#include <assert.h>
#include <stddef.h>
#include <algorithm>
#include "SDL.h"

namespace rtcw {

class SdlSystem
{
public:
	SdlSystem() {};
	~SdlSystem();

private:
	SdlSystem(const SdlSystem&);
	SdlSystem& operator=(const SdlSystem&);
};

// ==========================================================================

class SdlSubsystem
{
public:
	SdlSubsystem();
	explicit SdlSubsystem(Uint32 subsystem_id);
	~SdlSubsystem();

	void reset();
	void reset(Uint32 subsystem_id);
	void swap(SdlSubsystem& rhs);

private:
	Uint32 subsystem_id_;

private:
	SdlSubsystem(const SdlSubsystem&);
	SdlSubsystem& operator=(const SdlSubsystem&);
};

// ==========================================================================

template<typename T, typename TDeleter>
class SdlUPtr
{
public:
	SdlUPtr()
		:
		pointer_()
	{}

	explicit SdlUPtr(T* pointer)
		:
		pointer_(pointer)
	{}

	~SdlUPtr()
	{
		if (pointer_ == NULL)
		{
			return;
		}

		TDeleter()(pointer_);
	}

	T* get() const
	{
		return pointer_;
	}

	void reset()
	{
		if (pointer_ == NULL)
		{
			return;
		}

		TDeleter()(pointer_);
		pointer_ = NULL;
	}

	void reset(T* ptr)
	{
		if (pointer_ != NULL)
		{
			TDeleter()(pointer_);
		}

		pointer_ = ptr;
	}

	void swap(SdlUPtr& rhs)
	{
		std::swap(pointer_, rhs.pointer_);
	}

private:
	T* pointer_;

private:
	SdlUPtr(const SdlUPtr&);
	SdlUPtr& operator=(const SdlUPtr&);
};

// ==========================================================================

template<typename T, int TSize>
class SdlUPtrArray
{
public:
	SdlUPtrArray()
		:
		elements_()
	{}

	~SdlUPtrArray() {}

	int get_size() const
	{
		return TSize;
	}

	const T& operator[](int index) const
	{
		assert(index >= 0 && index < get_size());
		return elements_[index];
	}

	T& operator[](int index)
	{
		return const_cast<T&>(const_cast<const SdlUPtrArray*>(this)->operator[](index));
	}

private:
	T elements_[TSize];

private:
	SdlUPtrArray(const SdlUPtrArray&);
	SdlUPtrArray& operator=(const SdlUPtrArray&);
};

// ==========================================================================

struct SdlUPtrSurfaceDeleter
{
	void operator()(SDL_Surface* pointer) const;
};

typedef SdlUPtr<SDL_Renderer, SdlUPtrSurfaceDeleter> SdlSurfaceUPtr;

// ==========================================================================

struct SdlUPtrRendererDeleter
{
	void operator()(SDL_Renderer* pointer) const;
};

typedef SdlUPtr<SDL_Renderer, SdlUPtrRendererDeleter> SdlRendererUPtr;

// ==========================================================================

struct SdlUPtrTextureDeleter
{
	void operator()(SDL_Texture* pointer) const;
};

typedef SdlUPtr<SDL_Texture, SdlUPtrTextureDeleter> SdlTextureUPtr;

// ==========================================================================

struct SdlUPtrWindowDeleter
{
	void operator()(SDL_Window* pointer) const;
};

typedef SdlUPtr<SDL_Window, SdlUPtrWindowDeleter> SdlWindowUPtr;

} // namespace rtcw

#endif // RTCW_SDL_UTILITY_INCLUDED
