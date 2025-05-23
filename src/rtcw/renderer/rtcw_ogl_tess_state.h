/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_OGL_TESS_STATE_INCLUDED
#define RTCW_OGL_TESS_STATE_INCLUDED

#include "rtcw_cgm_mat.h"
#include "rtcw_cgm_vec.h"
#include "rtcw_mod_value.h"
#include "rtcw_ogl_tess_program.h"

namespace rtcw
{


class OglTessState
{
public:
	rtcw::ModValue<cgm::Mat4> projection;
	rtcw::ModValue<cgm::Mat4> model_view;

	rtcw::ModValue<bool> use_multitexturing;

	rtcw::ModValue<GLint> tex_2d[2];
	rtcw::ModValue<GLenum> tex_env_mode[2];

	rtcw::ModValue<cgm::Vec4> primary_color;

	rtcw::ModValue<bool> use_alpha_test;
	rtcw::ModValue<GLenum> alpha_test_func;
	rtcw::ModValue<float> alpha_test_ref;

	rtcw::ModValue<bool> use_fog;
	rtcw::ModValue<GLenum> fog_mode;
	rtcw::ModValue<GLenum> fog_dist_mode;
	rtcw::ModValue<GLenum> fog_hint;
	rtcw::ModValue<float> fog_density;
	rtcw::ModValue<float> fog_start;
	rtcw::ModValue<float> fog_end;
	rtcw::ModValue<cgm::Vec4> fog_color;

	rtcw::ModValue<float> intensity;
	rtcw::ModValue<float> overbright;
	rtcw::ModValue<float> gamma;


	OglTessState();

	~OglTessState();


	void set_program(
		rtcw::OglTessProgram* program);

	void set_default_values();

	void commit_changes();

	void invalidate();

	void invalidate_and_commit();


private:
	rtcw::OglTessProgram* program_;


	bool is_program_valid() const;

	OglTessState(const OglTessState& that);
	OglTessState& operator = (const OglTessState& that);
}; // OglTessState


} // rtcw


#endif // !RTCW_OGL_TESS_STATE_INCLUDED
