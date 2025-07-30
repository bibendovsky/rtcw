/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_OGL_TESS_STATE_INCLUDED
#define RTCW_OGL_TESS_STATE_INCLUDED

#include "qgl.h"
#include "rtcw_cgm_mat.h"
#include "rtcw_cgm_vec.h"
#include "rtcw_ogl_tess_program.h"

namespace rtcw {

class OglTessState
{
public:
	cgm::Mat4 projection;
	cgm::Mat4 model_view;
	bool use_multitexturing;
	GLint tex_2d[2];
	GLenum tex_env_mode[2];
	cgm::Vec4 primary_color;
	bool use_alpha_test;
	GLenum alpha_test_func;
	float alpha_test_ref;
	bool use_fog;
	GLenum fog_mode;
	GLenum fog_dist_mode;
	GLenum fog_hint;
	float fog_density;
	float fog_start;
	float fog_end;
	cgm::Vec4 fog_color;
	float intensity;
	float overbright;
	float gamma;

public:
	OglTessState();

	void set_program(const rtcw::OglTessProgram* program);
	void reset();
	void commit();

private:
	const rtcw::OglTessProgram* program_;

private:
	OglTessState(const OglTessState&);
	OglTessState& operator=(const OglTessState&);

	bool is_program_valid() const;
};

} // namespace rtcw

#endif // RTCW_OGL_TESS_STATE_INCLUDED
