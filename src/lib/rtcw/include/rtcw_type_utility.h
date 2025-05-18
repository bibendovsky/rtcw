/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

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
