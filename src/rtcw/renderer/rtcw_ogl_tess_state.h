/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_OGL_TESS_STATE_INCLUDED
#define RTCW_OGL_TESS_STATE_INCLUDED

#include "rtcw_array_trivial.h"
#include "rtcw_cgm_mat.h"
#include "rtcw_cgm_vec.h"
#include "rtcw_ogl_tess_program.h"

namespace rtcw {

class OglTessState
{
private:
	typedef char Sanity[2 * (OglProgram::max_vertex_attributes <= 16) - 1];

public:
	typedef unsigned int EnabledVertexAttribArrays;

public:
	cgm::Mat4 old_projection;
	cgm::Mat4 projection;

	cgm::Mat4 old_model_view;
	cgm::Mat4 model_view;

	bool old_use_multitexturing;
	bool use_multitexturing;

	GLint old_tex_2d[2];
	GLint tex_2d[2];

	GLenum old_tex_env_mode[2];
	GLenum tex_env_mode[2];

	cgm::Vec4 old_primary_color;
	cgm::Vec4 primary_color;

	bool old_use_alpha_test;
	bool use_alpha_test;

	GLenum old_alpha_test_func;
	GLenum alpha_test_func;

	float old_alpha_test_ref;
	float alpha_test_ref;

	bool old_use_fog;
	bool use_fog;

	GLenum old_fog_mode;
	GLenum fog_mode;

	GLenum old_fog_dist_mode;
	GLenum fog_dist_mode;

	GLenum old_fog_hint;
	GLenum fog_hint;

	float old_fog_density;
	float fog_density;

	float old_fog_start;
	float fog_start;

	float old_fog_end;
	float fog_end;

	cgm::Vec4 old_fog_color;
	cgm::Vec4 fog_color;

	float old_intensity;
	float intensity;

	float old_overbright;
	float overbright;

	float old_gamma;
	float gamma;

	EnabledVertexAttribArrays old_enabled_vertex_attrib_arrays;
	EnabledVertexAttribArrays enabled_vertex_attrib_arrays;

public:
	OglTessState();
	~OglTessState() {}

	void use_program();
	void set_program(rtcw::OglTessProgram* program);
	void set_default_values();
	void commit_changes();
	void invalidate();
	void invalidate_and_commit();
	void disable_all_vertex_attrib_arrays();
	void enable_vertex_attrib_array(int array_index);
	void mask_vertex_attrib_arrays();
	void unmask_vertex_attrib_arrays();

private:
	bool is_dirty_;
	bool vertex_attrib_arrays_masked_;
	rtcw::OglTessProgram* program_;

private:
	OglTessState(const OglTessState&);
	OglTessState& operator=(const OglTessState&);

	bool is_program_valid() const;
};

} // namespace rtcw

#endif // RTCW_OGL_TESS_STATE_INCLUDED
