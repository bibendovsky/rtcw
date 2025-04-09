// Computer Graphics Math - Column-major matrix.

#ifndef RTCW_CGM_MAT_INCLUDED
#define RTCW_CGM_MAT_INCLUDED

#include <assert.h>
#include "rtcw_type_utility.h"

// ==========================================================================

namespace rtcw {
namespace cgm {

namespace detail {

template<typename T>
struct MatCtorTag {};
struct Mat4IdentityTag {};

template<int M, int N>
struct MatChooseIdentityTag;

template<>
struct MatChooseIdentityTag<4, 4>
{
	typedef Mat4IdentityTag Type;
};

} // namespace detail

// --------------------------------------------------------------------------

template<int M, int N, typename T>
class Mat
{
public:
	static const int element_count = M * N;

public:
	static const Mat identity;

public:
	Mat();

	template<typename U>
	Mat(U m11, U m21, U m31, U m41,
		U m12, U m22, U m32, U m42,
		U m13, U m23, U m33, U m43,
		U m14, U m24, U m34, U m44,
		typename EnableIf<M == 4 && N == 4, detail::MatCtorTag<U> >::Type = detail::MatCtorTag<U>())
	{
		elements_[ 0] = m11;
		elements_[ 1] = m21;
		elements_[ 2] = m31;
		elements_[ 3] = m41;

		elements_[ 4] = m12;
		elements_[ 5] = m22;
		elements_[ 6] = m32;
		elements_[ 7] = m42;

		elements_[ 8] = m13;
		elements_[ 9] = m23;
		elements_[10] = m33;
		elements_[11] = m43;

		elements_[12] = m14;
		elements_[13] = m24;
		elements_[14] = m34;
		elements_[15] = m44;
	}

	Mat(const Mat& that);
	Mat& operator=(const Mat& that);
	~Mat() {}

	const T* get_data() const;
	T* get_data();

	const T& operator[](int index) const;
	T& operator[](int index);

private:
	typedef T Elements[element_count];

private:
	Elements elements_;

	explicit Mat(detail::Mat4IdentityTag);
	void copy_from(const Mat& that);
};

// --------------------------------------------------------------------------

template<int M, int N, typename T>
Mat<M, N, T>::Mat()
{
	const T filler = T();

	for (int i = 0; i < element_count; ++i)
	{
		elements_[i] = filler;
	}
}

template<int M, int N, typename T>
Mat<M, N, T>::Mat(const Mat& that)
{
	copy_from(that);
}

template<int M, int N, typename T>
Mat<M, N, T>& Mat<M, N, T>::operator=(const Mat& that)
{
	assert(&that != this);
	copy_from(that);
	return *this;
}

template<int M, int N, typename T>
const T* Mat<M, N, T>::get_data() const
{
	return elements_;
}

template<int M, int N, typename T>
T* Mat<M, N, T>::get_data()
{
	return elements_;
}

template<int M, int N, typename T>
const T& Mat<M, N, T>::operator[](int index) const
{
	assert(index >= 0 && index < element_count);
	return elements_[index];
}

template<int M, int N, typename T>
T& Mat<M, N, T>::operator[](int index)
{
	assert(index >= 0 && index < element_count);
	return elements_[index];
}

template<int M, int N, typename T>
Mat<M, N, T>::Mat(detail::Mat4IdentityTag)
{
	elements_[ 0] = 1;
	elements_[ 1] = 0;
	elements_[ 2] = 0;
	elements_[ 3] = 0;

	elements_[ 4] = 0;
	elements_[ 5] = 1;
	elements_[ 6] = 0;
	elements_[ 7] = 0;

	elements_[ 8] = 0;
	elements_[ 9] = 0;
	elements_[10] = 1;
	elements_[11] = 0;

	elements_[12] = 0;
	elements_[13] = 0;
	elements_[14] = 0;
	elements_[15] = 1;
}

template<int M, int N, typename T>
void Mat<M, N, T>::copy_from(const Mat& that)
{
	for (int i = 0; i < element_count; ++i)
	{
		elements_[i] = that.elements_[i];
	}
}

// ==========================================================================

template<int M, int N, typename T>
inline bool operator==(const Mat<M, N, T>& a, const Mat<M, N, T>& b)
{
	const int element_count = Mat<M, N, T>::element_count;

	for (int i = 0; i < element_count; ++i)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}

	return true;
}

template<int M, int N, typename T>
inline bool operator!=(const Mat<M, N, T>& a, const Mat<M, N, T>& b)
{
	return !(a == b);
}

// ==========================================================================

template<int M, int N, typename T>
const Mat<M, N, T> Mat<M, N, T>::identity = Mat<M, N, T>(typename detail::MatChooseIdentityTag<M, N>::Type());

// ==========================================================================

typedef Mat<4, 4, float> Mat4;

} // namespace cgm
} // namespace rtcw

#endif // RTCW_CGM_MAT_INCLUDED
