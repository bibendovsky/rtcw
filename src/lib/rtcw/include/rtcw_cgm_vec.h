/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// Computer Graphics Math - Vector.

#ifndef RTCW_CGM_VEC_INCLUDED
#define RTCW_CGM_VEC_INCLUDED

#include <assert.h>
#include "rtcw_type_utility.h"

// ==========================================================================

namespace rtcw {
namespace cgm {

namespace detail {

template<typename T>
struct VecCtorTag {};

} // namespace detail

// --------------------------------------------------------------------------

template<int N, typename T>
class Vec
{
public:
	static const int element_count = N;

public:
	Vec();

	template<typename U>
	Vec(U v0, U v1, typename EnableIf<element_count == 2, detail::VecCtorTag<U> >::Type = detail::VecCtorTag<U>())
	{
		elements_[0] = v0;
		elements_[1] = v1;
	}

	template<typename U>
	Vec(U v0, U v1, U v2, typename EnableIf<element_count == 3, detail::VecCtorTag<U> >::Type = detail::VecCtorTag<U>())
	{
		elements_[0] = v0;
		elements_[1] = v1;
		elements_[2] = v2;
	}

	template<typename U>
	Vec(U v0, U v1, U v2, U v3, typename EnableIf<element_count == 4, detail::VecCtorTag<U> >::Type = detail::VecCtorTag<U>())
	{
		elements_[0] = v0;
		elements_[1] = v1;
		elements_[2] = v2;
		elements_[3] = v3;
	}

	Vec(const Vec& that);
	Vec& operator=(const Vec& that);
	~Vec() {}

	const T* get_data() const;
	T* get_data();

	const T& operator[](int index) const;
	T& operator[](int index);

private:
	typedef T Elements[element_count];

private:
	Elements elements_;

private:
	void copy_from(const Vec& that);
};

// --------------------------------------------------------------------------

template<int N, typename T>
Vec<N, T>::Vec()
{
	const T filler = T();

	for (int i = 0; i < element_count; ++i)
	{
		elements_[i] = filler;
	}
}

template<int N, typename T>
Vec<N, T>::Vec(const Vec& that)
{
	copy_from(that);
}

template<int N, typename T>
Vec<N, T>& Vec<N, T>::operator=(const Vec& that)
{
	assert(&that != this);
	copy_from(that);
	return *this;
}

template<int N, typename T>
const T* Vec<N, T>::get_data() const
{
	return elements_;
}

template<int N, typename T>
T* Vec<N, T>::get_data()
{
	return elements_;
}

template<int N, typename T>
const T& Vec<N, T>::operator[](int index) const
{
	assert(index >= 0 && index < element_count);
	return elements_[index];
}

template<int N, typename T>
T& Vec<N, T>::operator[](int index)
{
	assert(index >= 0 && index < element_count);
	return elements_[index];
}

template<int N, typename T>
void Vec<N, T>::copy_from(const Vec& that)
{
	for (int i = 0; i < element_count; ++i)
	{
		elements_[i] = that.elements_[i];
	}
}

// ==========================================================================

template<int N, typename T>
inline bool operator==(const Vec<N, T>& a, const Vec<N, T> &b)
{
	for (int i = 0; i < N; ++i)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}

	return true;
}

template<int N, typename T>
inline bool operator!=(const Vec<N, T>& a, const Vec<N, T> &b)
{
	return !(a == b);
}

// ==========================================================================

template<typename T>
inline Vec<4, T> operator*(const Vec<4, T>& a, T b)
{
	return Vec<4, T>(a[0] * b, a[1] * b, a[2] * b, a[3] * b);
}

template<typename T>
inline Vec<4, T> operator*(T a, const Vec<4, T>& b)
{
	return b * a;
}

// --------------------------------------------------------------------------

template<typename T>
inline Vec<4, T> operator+(const Vec<4, T>& a, const Vec<4, T>& b)
{
	return Vec<4, T>(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

// ==========================================================================

typedef Vec<2, float> Vec2;
typedef Vec<3, float> Vec3;
typedef Vec<4, float> Vec4;

} // namespace cgm
} // namespace rtcw

#endif // RTCW_CGM_VEC_INCLUDED
