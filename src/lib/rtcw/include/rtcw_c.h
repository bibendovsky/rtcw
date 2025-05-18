/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_C_INCLUDED
#define RTCW_C_INCLUDED


// BBi
// A wrappers for standard functions
// to maintain C compatibility.
// (See #BUG0002)
// BBi


#include <math.h>
#include <stdlib.h>


namespace c
{


inline int abs(
	int x)
{
	return ::abs(x);
}

inline ::div_t div(
	int numerator,
	int denominator)
{
	return ::div(numerator, denominator);
}

inline double acos(
	double x)
{
	return ::acos(x);
}

inline double asin(
	double x)
{
	return ::asin(x);
}

inline double atan(
	double x)
{
	return ::atan(x);
}

inline double atan2(
	double x,
	double y)
{
	return ::atan2(x, y);
}

inline double cos(
	double x)
{
	return ::cos(x);
}

inline double sin(
	double x)
{
	return ::sin(x);
}

inline double tan(
	double x)
{
	return ::tan(x);
}

inline double exp(
	double x)
{
	return ::exp(x);
}

inline double frexp(
	double value,
	int* exp)
{
	return ::frexp(value, exp);
}

inline double ldexp(
	double x,
	int exp)
{
	return ::ldexp(x, exp);
}

inline double log(
	double x)
{
	return ::log(x);
}

inline double log10(
	double x)
{
	return ::log10(x);
}

inline double modf(
	double value,
	double* iptr)
{
	return ::modf(value, iptr);
}

inline double fabs(
	double x)
{
	return ::fabs(x);
}

inline double pow(
	double x,
	double y)
{
	return ::pow(x, y);
}

inline double sqrt(
	double x)
{
	return ::sqrt(x);
}

inline double ceil(
	double x)
{
	return ::ceil(x);
}

inline double floor(
	double x)
{
	return ::floor(x);
}

inline double fmod(
	double x,
	double y)
{
	return ::fmod(x, y);
}


} // c


#endif // !RTCW_C_INCLUDED
