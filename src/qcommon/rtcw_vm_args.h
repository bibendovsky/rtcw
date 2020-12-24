#ifndef RTCW_VM_ARGS_INCLUDED
#define RTCW_VM_ARGS_INCLUDED


#include <cstdint>

#include <type_traits>


namespace rtcw
{


namespace detail
{


struct VmArgSimpleTag{};
struct VmArgPointerTag{};
struct VmArgFloatTag{};


template<
	typename T
>
inline std::intptr_t to_vm_arg(
	T simple_value,
	VmArgSimpleTag) noexcept
{
	return static_cast<std::intptr_t>(simple_value);
}

template<
	typename T
>
inline std::intptr_t to_vm_arg(
	T* pointer_value,
	VmArgPointerTag) noexcept
{
	return reinterpret_cast<std::intptr_t>(pointer_value);
}

template<
	typename T
>
inline std::intptr_t to_vm_arg(
	T float_value,
	VmArgFloatTag) noexcept
{
	return reinterpret_cast<const std::int32_t&>(float_value);
}


} // detail


template<
	typename T
>
inline std::intptr_t to_vm_arg(
	T value) noexcept
{
	using Tag = std::conditional_t<
		(std::is_integral_v<T> || std::is_enum_v<T>) && sizeof(T) <= sizeof(std::intptr_t),
		detail::VmArgSimpleTag,
		std::conditional_t<
			std::is_pointer_v<T>,
			detail::VmArgPointerTag,
			std::conditional_t<
				std::is_same_v<T, float> && sizeof(T) == 4,
				detail::VmArgFloatTag,
				void
			>
		>
	>;

	static_assert(!std::is_same_v<Tag, void>, "Unsupported type.");

	return detail::to_vm_arg(value, Tag{});
}


namespace detail
{


template<
	typename T
>
inline T from_vm_arg(
	std::intptr_t vm_arg,
	VmArgSimpleTag) noexcept
{
	return static_cast<T>(vm_arg);
}

template<
	typename T
>
inline T from_vm_arg(
	std::intptr_t vm_arg,
	VmArgPointerTag) noexcept
{
	return reinterpret_cast<T>(vm_arg);
}

template<
	typename T
>
inline T from_vm_arg(
	std::intptr_t vm_arg,
	VmArgFloatTag) noexcept
{
	const auto float_image = static_cast<std::int32_t>(vm_arg);
	return reinterpret_cast<const float&>(float_image);
}


} // detail


template<
	typename T
>
inline T from_vm_arg(
	std::intptr_t vm_arg) noexcept
{
	using Tag = std::conditional_t<
		(std::is_integral_v<T> || std::is_enum_v<T>) && sizeof(T) <= 4,
		detail::VmArgSimpleTag,
		std::conditional_t<
			std::is_pointer_v<T>,
			detail::VmArgPointerTag,
			std::conditional_t<
				std::is_same_v<T, float> && sizeof(T) == 4,
				detail::VmArgFloatTag,
				void
			>
		>
	>;

	return detail::from_vm_arg<T>(vm_arg, Tag{});
}


} // rtcw


#endif // !RTCW_VM_ARGS_INCLUDED
