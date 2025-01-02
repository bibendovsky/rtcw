#ifndef RTCW_VM_ARGS_INCLUDED
#define RTCW_VM_ARGS_INCLUDED

#include <stddef.h>

namespace rtcw {

namespace detail {

template<bool TCond, typename TTrue, typename TFalse>
struct VmArgCond
{
	typedef TFalse Type;
};

template<typename TTrue, typename TFalse>
struct VmArgCond<true, TTrue, TFalse>
{
	typedef TTrue Type;
};

// --------------------------------------------------------------------------

template<typename T>
struct VmArgIsPtr
{
	static const bool value = false;
};

template<typename T>
struct VmArgIsPtr<T*>
{
	static const bool value = true;
};

template<typename T>
struct VmArgIsPtr<const T*>
{
	static const bool value = true;
};

// --------------------------------------------------------------------------

template<typename T>
struct VmArgIsFloat
{
	static const bool value = false;
};

template<>
struct VmArgIsFloat<float>
{
	static const bool value = true;
};

template<>
struct VmArgIsFloat<const float>
{
	static const bool value = true;
};

// --------------------------------------------------------------------------

typedef
	VmArgCond<
		sizeof(unsigned char) == sizeof(float),
		unsigned char,
		VmArgCond<
			sizeof(unsigned short) == sizeof(float),
			unsigned short,
			VmArgCond<
				sizeof(unsigned int) == sizeof(float),
				unsigned int,
				VmArgCond<
					sizeof(unsigned long) == sizeof(float),
					unsigned long,
					VmArgCond<
						sizeof(unsigned long long) == sizeof(float),
						unsigned long long,
						void
					>::Type
				>::Type
			>::Type
		>::Type
	>::Type
VmArgFloatImage;

// --------------------------------------------------------------------------

struct VmArgPtrTag {}; // A poiner.
struct VmArgFloatTag {}; // A binary32.
struct VmArgValueTag {}; // Everything else.

// --------------------------------------------------------------------------

inline ptrdiff_t to_vm_arg(const void* value, VmArgPtrTag)
{
	return reinterpret_cast<ptrdiff_t>(value);
}

inline ptrdiff_t to_vm_arg(float value, VmArgFloatTag)
{
	return static_cast<ptrdiff_t>(*reinterpret_cast<const VmArgFloatImage*>(&value));
}

template<typename T>
inline ptrdiff_t to_vm_arg(T value, VmArgValueTag)
{
	return value;
}

// --------------------------------------------------------------------------

template<typename T>
inline T from_vm_arg(ptrdiff_t value, VmArgPtrTag)
{
	return reinterpret_cast<T>(value);
}

template<typename T>
inline T from_vm_arg(ptrdiff_t value, VmArgFloatTag)
{
	const VmArgFloatImage image = static_cast<VmArgFloatImage>(value);
	return *reinterpret_cast<const float*>(&image);
}

template<typename T>
inline T from_vm_arg(ptrdiff_t value, VmArgValueTag)
{
	return static_cast<T>(value);
}

} // namespace detail

template<typename T>
inline ptrdiff_t to_vm_arg(T value)
{
	typedef
		typename detail::VmArgCond<
			sizeof(T) <= sizeof(ptrdiff_t) && detail::VmArgIsPtr<T>::value,
			detail::VmArgPtrTag,
			typename detail::VmArgCond<
				sizeof(T) <= sizeof(ptrdiff_t) && detail::VmArgIsFloat<T>::value,
				detail::VmArgFloatTag,
				typename detail::VmArgCond<
					sizeof(T) <= sizeof(ptrdiff_t),
					detail::VmArgValueTag,
					void
				>::Type
			>::Type
		>::Type
	Tag;

	return detail::to_vm_arg(value, Tag());
}

template<typename T>
inline T from_vm_arg(ptrdiff_t vm_arg)
{
	typedef
		typename detail::VmArgCond<
			sizeof(T) <= sizeof(ptrdiff_t) && detail::VmArgIsPtr<T>::value,
			detail::VmArgPtrTag,
			typename detail::VmArgCond<
				sizeof(T) <= sizeof(ptrdiff_t) && detail::VmArgIsFloat<T>::value,
				detail::VmArgFloatTag,
				typename detail::VmArgCond<
					sizeof(T) <= sizeof(ptrdiff_t),
					detail::VmArgValueTag,
					void
				>::Type
			>::Type
		>::Type
	Tag;

	return detail::from_vm_arg<T>(vm_arg, Tag());
}

} // namespace rtcw

#endif // RTCW_VM_ARGS_INCLUDED
