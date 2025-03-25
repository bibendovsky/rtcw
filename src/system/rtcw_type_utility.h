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

} // namespace rtcw

#endif // RTCW_TYPE_UTILITY_INCLUDED
