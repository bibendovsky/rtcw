/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// Computer Graphics Math - Clip space transformations.

#ifndef RTCW_CGM_CLIP_SPACE_INCLUDED
#define RTCW_CGM_CLIP_SPACE_INCLUDED

#include <assert.h>
#include <math.h>
#include "rtcw_cgm_mat.h"

namespace rtcw {
namespace cgm {

/*
Creates a matrix for an orthographic parallel viewing volume,
using right-handed coordinates.

The near and far clip planes correspond to z normalized device
coordinates of -1 and +1 respectively.

Params:
  - l: Left point of near clipping plane.
  - r: Right point of near clipping plane.
  - b: Bottom point of near clipping plane.
  - t: Top point of near clipping plane.
  - n: The distance from the eye to the near clipping plane.
  - f: The distance from the eye to the far clipping plane.

Returns:
  A matrix that produces parallel projection.
*/
template<typename T>
inline Mat<4, 4, T> make_ortho_rh_n1p1(T l, T r, T b, T t, T n, T f)
{
	assert(l != r);
	assert(b != t);
	assert(n != f);

	const T r_rml = 1 / (r - l);
	const T r_tmb = 1 / (t - b);
	const T r_fmn = 1 / (f - n);

	const T _11 = 2 * r_rml;
	const T _14 = -(r + l) * r_rml;

	const T _22 = 2 * r_tmb;
	const T _24 = -(t + b) * r_tmb;

	const T _33 = -2 * r_fmn;
	const T _34 = -(f + n) * r_fmn;

	return Mat<4, 4, T>(
		 _11, T(0), T(0), T(0),
		T(0),  _22, T(0), T(0),
		T(0), T(0),  _33, T(0),
		 _14,  _24,  _34, T(1));
}

} // namespace cgm
} // namespace rtcw

#endif // RTCW_CGM_CLIP_SPACE_INCLUDED
