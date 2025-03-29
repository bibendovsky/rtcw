#ifndef RTCW_TYPE_UTILITY_INCLUDED
#define RTCW_TYPE_UTILITY_INCLUDED

namespace rtcw {

template<bool TCondition, typename TType = void>
struct EnableIf {};

template<typename TType>
struct EnableIf<true, TType>
{
	typedef TType Type;
};

// ==========================================================================

template<bool TCondition, typename TTrue, typename TFalse>
struct Conditional
{
	typedef TFalse Type;
};

template<typename TTrue, typename TFalse>
struct Conditional<true, TTrue, TFalse>
{
	typedef TTrue Type;
};

} // namespace rtcw

#endif // RTCW_TYPE_UTILITY_INCLUDED
