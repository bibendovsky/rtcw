/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_OGL_TESS_PROGRAM_INCLUDED
#define RTCW_OGL_TESS_PROGRAM_INCLUDED

#include "rtcw_string.h"
#include "rtcw_ogl_program.h"

namespace rtcw {

class OglTessProgram : public OglProgram
{
public:
	int a_pos_vec4;
	int a_col_vec4;
	int a_tc0_vec2;
	int a_tc1_vec2;
	int u_projection_mat4;
	int u_model_view_mat4;
	int u_use_alpha_test;
	int u_alpha_test_func;
	int u_alpha_test_ref;
	int u_tex_env_mode[2];
	int u_use_multitexturing;
	int u_tex_2d[2];
	int u_primary_color;
	int u_use_fog;
	int u_fog_mode;
	int u_fog_dist_mode;
	int u_fog_hint;
	int u_fog_color;
	int u_fog_density;
	int u_fog_start;
	int u_fog_end;
	int u_intensity;
	int u_overbright;
	int u_gamma;

	OglTessProgram(const String& glsl_dir, const String& base_name);
	OglTessProgram(const char* vertex_shader_source, const char* fragment_shader_source);

	virtual ~OglTessProgram();

	virtual OglProgram* create_new(const String& glsl_dir, const String& base_name);
	virtual OglProgram* create_new(const char* vertex_shader_source, const char* fragment_shader_source);

private:
	virtual bool do_reload();
	virtual void do_unload();

private:
	static const char* const impl_attribute_names_[max_vertex_attributes];

private:
	OglTessProgram(const OglTessProgram&);
	OglTessProgram& operator=(const OglTessProgram&);

	void unload_internal();
	bool reload_internal();
};

} // namespace rtcw

#endif // RTCW_OGL_TESS_PROGRAM_INCLUDED
