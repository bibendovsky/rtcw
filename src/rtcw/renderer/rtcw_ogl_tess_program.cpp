/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_ogl_tess_program.h"
#include "qgl.h"
#include "tr_local.h"
#include "rtcw_ogl_program.h"

namespace rtcw {

const char* const OglTessProgram::impl_attribute_names_[max_vertex_attributes] =
{
	"pos_vec4",
	"col_vec4",
	"tc0_vec2",
	"tc1_vec2"
};

OglTessProgram::OglTessProgram(const String& glsl_dir, const String& base_name)
	:
	OglProgram(glsl_dir, base_name),
	a_pos_vec4(-1),
	a_col_vec4(-1),
	a_tc0_vec2(-1),
	a_tc1_vec2(-1),
	u_projection_mat4(-1),
	u_model_view_mat4(-1),
	u_use_alpha_test(-1),
	u_alpha_test_func(-1),
	u_alpha_test_ref(-1),
	u_use_multitexturing(-1),
	u_primary_color(-1),
	u_use_fog(-1),
	u_fog_mode(-1),
	u_fog_dist_mode(-1),
	u_fog_hint(-1),
	u_fog_color(-1),
	u_fog_density(-1),
	u_fog_start(-1),
	u_fog_end(-1),
	u_intensity(-1),
	u_overbright(-1),
	u_gamma(-1)
{
	u_tex_env_mode[0] = -1;
	u_tex_env_mode[1] = -1;
	u_tex_2d[0] = -1;
	u_tex_2d[1] = -1;

	attribute_names_ = impl_attribute_names_;
}

OglTessProgram::OglTessProgram(const char* vertex_shader_source, const char* fragment_shader_source)
	:
	OglProgram(vertex_shader_source, fragment_shader_source),
	a_pos_vec4(-1),
	a_col_vec4(-1),
	a_tc0_vec2(-1),
	a_tc1_vec2(-1),
	u_projection_mat4(-1),
	u_model_view_mat4(-1),
	u_use_alpha_test(-1),
	u_alpha_test_func(-1),
	u_alpha_test_ref(-1),
	u_use_multitexturing(-1),
	u_primary_color(-1),
	u_use_fog(-1),
	u_fog_mode(-1),
	u_fog_dist_mode(-1),
	u_fog_hint(-1),
	u_fog_color(-1),
	u_fog_density(-1),
	u_fog_start(-1),
	u_fog_end(-1),
	u_intensity(-1),
	u_overbright(-1),
	u_gamma(-1)
{
	u_tex_env_mode[0] = -1;
	u_tex_env_mode[1] = -1;
	u_tex_2d[0] = -1;
	u_tex_2d[1] = -1;

	attribute_names_ = impl_attribute_names_;
}

OglTessProgram::~OglTessProgram()
{
	OglTessProgram::unload_internal();
}

OglProgram* OglTessProgram::create_new(const String& glsl_dir, const String& base_name)
{
	return new OglTessProgram(glsl_dir, base_name);
}

OglProgram* OglTessProgram::create_new(const char* vertex_shader_source, const char* fragment_shader_source)
{
	return new OglTessProgram(vertex_shader_source, fragment_shader_source);
}

void OglTessProgram::unload_internal()
{
	a_pos_vec4 = -1;
	a_col_vec4 = -1;
	a_tc0_vec2 = -1;
	a_tc1_vec2 = -1;
	u_projection_mat4 = -1;
	u_model_view_mat4 = -1;
	u_use_alpha_test = -1;
	u_alpha_test_func = -1;
	u_alpha_test_ref = -1;
	u_tex_env_mode[0] = -1;
	u_tex_env_mode[1] = -1;
	u_use_multitexturing = -1;
	u_tex_2d[0] = -1;
	u_tex_2d[1] = -1;
	u_primary_color = -1;
	u_use_fog = -1;
	u_fog_mode = -1;
	u_fog_dist_mode = -1;
	u_fog_hint = -1;
	u_fog_color = -1;
	u_fog_density = -1;
	u_fog_start = -1;
	u_fog_end = -1;
	u_intensity = -1;
	u_overbright = -1;
	u_gamma = -1;

	OglProgram::unload_internal();
}

bool OglTessProgram::reload_internal()
{
	OglTessProgram::unload_internal();

	if (!OglProgram::reload_internal())
	{
		return false;
	}

	a_pos_vec4 = glGetAttribLocation(program_, "pos_vec4");
	a_col_vec4 = glGetAttribLocation(program_, "col_vec4");
	a_tc0_vec2 = glGetAttribLocation(program_, "tc0_vec2");
	a_tc1_vec2 = glGetAttribLocation(program_, "tc1_vec2");

	if (a_pos_vec4 >= max_vertex_attributes ||
		a_col_vec4 >= max_vertex_attributes ||
		a_tc0_vec2 >= max_vertex_attributes ||
		a_tc1_vec2 >= max_vertex_attributes)
	{
		ri.Printf(PRINT_ALL, "Attribute location out of range.\n");
		return false;
	}

	u_projection_mat4 = glGetUniformLocation(program_, "projection_mat4");
	u_model_view_mat4 = glGetUniformLocation(program_, "model_view_mat4");

	u_use_alpha_test = glGetUniformLocation(program_, "use_alpha_test");
	u_alpha_test_func = glGetUniformLocation(program_, "alpha_test_func");
	u_alpha_test_ref = glGetUniformLocation(program_, "alpha_test_ref");

	u_tex_env_mode[0] = glGetUniformLocation(program_, "tex_env_mode[0]");
	u_tex_env_mode[1] = glGetUniformLocation(program_, "tex_env_mode[1]");

	u_use_multitexturing = glGetUniformLocation(program_, "use_multitexturing");
	u_tex_2d[0] = glGetUniformLocation(program_, "tex_2d[0]");
	u_tex_2d[1] = glGetUniformLocation(program_, "tex_2d[1]");
	u_primary_color = glGetUniformLocation(program_, "primary_color");

	u_use_fog = glGetUniformLocation(program_, "use_fog");
	u_fog_mode = glGetUniformLocation(program_, "fog_mode");
	u_fog_dist_mode = glGetUniformLocation(program_, "fog_dist_mode");
	u_fog_hint = glGetUniformLocation(program_, "fog_hint");
	u_fog_color = glGetUniformLocation(program_, "fog_color");
	u_fog_density = glGetUniformLocation(program_, "fog_density");
	u_fog_start = glGetUniformLocation(program_, "fog_start");
	u_fog_end = glGetUniformLocation(program_, "fog_end");

	u_intensity = glGetUniformLocation(program_, "intensity");
	u_overbright = glGetUniformLocation(program_, "overbright");
	u_gamma = glGetUniformLocation(program_, "gamma");

	return true;
}

bool OglTessProgram::do_reload()
{
	return reload_internal();
}

void OglTessProgram::do_unload()
{
	unload_internal();
}

} // namespace rtcw
