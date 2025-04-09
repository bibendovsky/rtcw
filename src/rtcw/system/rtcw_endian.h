//
// Byte order (endianness) manipulation.
//


#ifndef RTCW_ENDIAN_INCLUDED
#define RTCW_ENDIAN_INCLUDED



#define RTCW_ENDIAN_UNKNOWN 0

#ifndef RTCW_ENDIAN_BIG
	#define RTCW_ENDIAN_BIG 1
#endif

#ifndef RTCW_ENDIAN_LITTLE
	#define RTCW_ENDIAN_LITTLE 2
#endif

#if RTCW_ENDIAN_UNKNOWN != 0 || RTCW_ENDIAN_BIG != 1 || RTCW_ENDIAN_LITTLE != 2
	#error Invalid endian constants.
#endif

#ifndef RTCW_ENDIAN

	#if defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
		#define RTCW_ENDIAN RTCW_ENDIAN_LITTLE
	#else

		//
		// __BYTE_ORDER__
		//
		#ifndef RTCW_ENDIAN
			#if defined(__BYTE_ORDER__)
				#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
					#define RTCW_ENDIAN RTCW_ENDIAN_BIG
				#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
					#define RTCW_ENDIAN RTCW_ENDIAN_LITTLE
				#endif
			#endif
		#endif

		//
		// __BIG_ENDIAN__ / __LITTLE_ENDIAN__
		//
		#ifndef RTCW_ENDIAN
			#if defined(__BIG_ENDIAN__)
				#define RTCW_ENDIAN RTCW_ENDIAN_BIG
			#elif defined(__LITTLE_ENDIAN__)
				#define RTCW_ENDIAN RTCW_ENDIAN_LITTLE
			#endif
		#endif

	#endif

#endif // RTCW_ENDIAN

#ifndef RTCW_ENDIAN
	#define RTCW_ENDIAN RTCW_ENDIAN_UNKNOWN
#endif


namespace rtcw
{


class Endian
{
public:
	// Returns "true" if system's endianness is big-endian.
	static bool is_big()
	{
#if RTCW_ENDIAN == RTCW_ENDIAN_LITTLE
		return false;
#elif RTCW_ENDIAN == RTCW_ENDIAN_BIG
		return true;
#endif
	}

	// Returns "true" if system's endianness is little-endian.
	static bool is_little()
	{
#if RTCW_ENDIAN == RTCW_ENDIAN_LITTLE
		return true;
#elif RTCW_ENDIAN == RTCW_ENDIAN_BIG
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
#if RTCW_ENDIAN == RTCW_ENDIAN_LITTLE
		return value;
#elif RTCW_ENDIAN == RTCW_ENDIAN_BIG
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
#if RTCW_ENDIAN == RTCW_ENDIAN_BIG
		return value;
#elif RTCW_ENDIAN == RTCW_ENDIAN_LITTLE
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
#if RTCW_ENDIAN == RTCW_ENDIAN_BIG
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
#if RTCW_ENDIAN == RTCW_ENDIAN_LITTLE
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
