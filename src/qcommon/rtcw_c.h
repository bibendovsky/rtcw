#ifndef RTCW_C_INCLUDED
#define RTCW_C_INCLUDED


// BBi
// A wrappers for standard functions
// to maintain C compatibility.
// (See #BUG0002)
// BBi


#include <cmath>
#include <cstdlib>


namespace c
{


inline int abs(
	int x)
{
	return std::abs(x);
}

inline std::div_t div(
	int numerator,
	int denominator)
{
	return std::div(numerator, denominator);
}

inline double acos(
	double x)
{
	return std::acos(x);
}

inline double asin(
	double x)
{
	return std::asin(x);
}

inline double atan(
	double x)
{
	return std::atan(x);
}

inline double atan2(
	double x,
	double y)
{
	return std::atan2(x, y);
}

inline double cos(
	double x)
{
	return std::cos(x);
}

inline double sin(
	double x)
{
	return std::sin(x);
}

inline double tan(
	double x)
{
	return std::tan(x);
}

inline double exp(
	double x)
{
	return std::exp(x);
}

inline double frexp(
	double value,
	int* exp)
{
	return std::frexp(value, exp);
}

inline double ldexp(
	double x,
	int exp)
{
	return std::ldexp(x, exp);
}

inline double log(
	double x)
{
	return std::log(x);
}

inline double log10(
	double x)
{
	return std::log10(x);
}

inline double modf(
	double value,
	double* iptr)
{
	return std::modf(value, iptr);
}

inline double fabs(
	double x)
{
	return std::fabs(x);
}

inline double pow(
	double x,
	double y)
{
	return std::pow(x, y);
}

inline double sqrt(
	double x)
{
	return std::sqrt(x);
}

inline double ceil(
	double x)
{
	return std::ceil(x);
}

inline double floor(
	double x)
{
	return std::floor(x);
}

inline double fmod(
	double x,
	double y)
{
	return std::fmod(x, y);
}


} // c


#endif // !RTCW_C_INCLUDED
