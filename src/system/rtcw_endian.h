//
// Byte order (endianness) manipulation.
//


#ifndef RTCW_ENDIAN_INCLUDED
#define RTCW_ENDIAN_INCLUDED


#include "SDL_endian.h"


namespace rtcw
{


class Endian
{
public:
	// Returns "true" if system's endianness is big-endian.
	static bool is_big()
	{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		return false;
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
		return true;
#endif
	}

	// Returns "true" if system's endianness is little-endian.
	static bool is_little()
	{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		return true;
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
		return false;
#endif
	}

	// Swaps bytes on non little-endian system.
	template<
		typename T
	>
	static T le(
		T value)
	{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		return value;
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
		return le_be(value);
#endif
	}

	// Copies an array of elements with swapped bytes into another array
	// on non little-endian system.
	template<
		typename T
	>
	static void le(
		const T* src_data,
		std::size_t count,
		T* dst_data)
	{
		for (std::size_t i = 0; i < count; ++i)
		{
			dst_data[i] = le(src_data[i]);
		}
	}

	// Swaps bytes on non big-endian system.
	template<
		typename T
	>
	static T be(
		T value)
	{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		return value;
#elif SDL_BYTEORDER == SDL_LIL_ENDIAN
		return le_be(value);
#endif
	}

	// Copies an array of elements with swapped bytes into another array
	// on non big-endian system.
	template<
		typename T
	>
	static void be(
		const T* src_data,
		std::size_t count,
		T* dst_data)
	{
		for (std::size_t i = 0; i < count; ++i)
		{
			dst_data[i] = be(src_data[i]);
		}
	}

	// Swaps bytes in place on non little-endian system.
	template<
		typename T
	>
	static void lei(
		T& value)
	{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		lei_bei(value);
#endif
	}

	// Swaps bytes in place of an array of elements
	// on non little-endian system.
	template<
		typename T
	>
	static void lei(
		T* data,
		std::size_t count)
	{
		for (std::size_t i = 0; i < count; ++i)
		{
			lei(data[i]);
		}
	}

	// Swaps bytes in place on non big-endian system.
	template<
		typename T>
	static void bei(
		T& value)
	{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		lei_bei(value);
#endif
	}

	// Swaps bytes in place of an array of elements
	// on non big-endian system.
	template<
		typename T
	>
	static void bei(
		T* data,
		std::size_t count)
	{
		for (std::size_t i = 0; i < count; ++i)
		{
			bei(data[i]);
		}
	}


private:
	// Swaps bytes.
	template<
		typename T>
	static T le_be(
		const T& value)
	{
		T result;

		for (std::size_t i = 0, j = sizeof(T) - 1; i < sizeof(T); ++i, --j)
		{
			(reinterpret_cast<char*>(&result))[i] =
				(reinterpret_cast<const char*>(&value))[j];
		}

		return result;
	}

	// Swaps bytes in place.
	template<
		typename T
	>
	static void lei_bei(
		T& value)
	{
		for (std::size_t i = 0, j = sizeof(T) - 1, n = sizeof(T) / 2; i < n; ++i, --j)
		{
			std::swap(
				(reinterpret_cast<char*>(&value))[i],
				(reinterpret_cast<char*>(&value))[j]);
		}
	}
}; // Endian


} // rtcw


#endif // !RTCW_ENDIAN_INCLUDED
