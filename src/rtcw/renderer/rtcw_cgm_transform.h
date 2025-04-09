// Computer Graphics Math - Common transformations.

#ifndef RTCW_CGM_TRANSFORM_INCLUDED
#define RTCW_CGM_TRANSFORM_INCLUDED

#include <math.h>
#include "rtcw_cgm_mat.h"
#include "rtcw_cgm_vec.h"

namespace rtcw {
namespace cgm {

/*
Composes a translation matrix created from a three-component vector.

Params:
  - m: Input matrix.
  - v: A translation vector.

Returns:
	An input matrix multiplied by the translation one.
*/
template<typename T>
inline Mat<4, 4, T> translate(const Mat<4, 4, T>& m, const Vec<3, T>& v)
{
	const T _14 = v[0];
	const T _24 = v[1];
	const T _34 = v[2];

	return Mat<4, 4, T>(
		m[ 0],
		m[ 1],
		m[ 2],
		m[ 3],

		m[ 4],
		m[ 5],
		m[ 6],
		m[ 7],

		m[ 8],
		m[ 9],
		m[10],
		m[11],

		m[ 0] * _14 + m[ 4] * _24 + m[ 8] * _34 + m[12],
		m[ 1] * _14 + m[ 5] * _24 + m[ 9] * _34 + m[13],
		m[ 2] * _14 + m[ 6] * _24 + m[10] * _34 + m[14],
		m[ 3] * _14 + m[ 7] * _24 + m[11] * _34 + m[15]);
}

} // namespace cgm
} // namespace rtcw

#endif // RTCW_CGM_TRANSFORM_INCLUDED
