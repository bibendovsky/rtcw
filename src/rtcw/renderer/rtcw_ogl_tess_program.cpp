#include "rtcw_ogl_tess_program.h"
#include "qgl.h"
#include "rtcw_ogl_program.h"


namespace rtcw
{


OglTessProgram::OglTessProgram(
	const String& glsl_dir,
	const String& base_name)
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
}

OglTessProgram::OglTessProgram(
	const char* v_shader_c_string,
	const char* f_shader_c_string)
	:
	OglProgram(v_shader_c_string, f_shader_c_string),
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
}

OglTessProgram::~OglTessProgram()
{
	OglTessProgram::unload_internal();
}

OglProgram* OglTessProgram::create_new(
	const String& glsl_dir,
	const String& base_name)
{
	return new OglTessProgram(glsl_dir, base_name);
}

OglProgram* OglTessProgram::create_new(
	const char* v_shader_c_string,
	const char* f_shader_c_string)
{
	return new OglTessProgram(v_shader_c_string, f_shader_c_string);
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


	if (a_col_vec4 >= 0)
	{
		glVertexAttrib4f(a_col_vec4, 1.0F, 1.0F, 1.0F, 1.0F);
	}

	if (a_tc0_vec2 >= 0)
	{
		glVertexAttrib2f(a_tc0_vec2, 0.0F, 0.0F);
	}

	if (a_tc1_vec2 >= 0)
	{
		glVertexAttrib2f(a_tc1_vec2, 0.0F, 0.0F);
	}

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


} // rtcw
