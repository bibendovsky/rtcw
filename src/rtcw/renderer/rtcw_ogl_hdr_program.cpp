/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_ogl_hdr_program.h"
#include "qgl.h"
#include "tr_local.h"
#include "rtcw_ogl_program.h"

namespace rtcw {

const char* const OglHdrProgram::impl_attribute_names_[max_vertex_attributes] =
{
	"pos_vec2",
	"tc_vec2",
};

OglHdrProgram::OglHdrProgram(const String& glsl_dir, const String& base_name)
	:
	OglProgram(glsl_dir, base_name),
	a_pos_vec2(-1),
	a_tc_vec2(-1),
	u_tex_2d(-1)
{
	attribute_names_ = impl_attribute_names_;
}

OglHdrProgram::OglHdrProgram(const char* vertex_shader_source, const char* fragment_shader_source)
	:
	OglProgram(vertex_shader_source, fragment_shader_source),
	a_pos_vec2(-1),
	a_tc_vec2(-1),
	u_tex_2d(-1)
{
	attribute_names_ = impl_attribute_names_;
}

OglHdrProgram::~OglHdrProgram()
{
	OglHdrProgram::unload_internal();
}

OglProgram* OglHdrProgram::create_new(const String& glsl_dir, const String& base_name)
{
	return new OglHdrProgram(glsl_dir, base_name);
}

OglProgram* OglHdrProgram::create_new(const char* vertex_shader_source, const char* fragment_shader_source)
{
	return new OglHdrProgram(vertex_shader_source, fragment_shader_source);
}

void OglHdrProgram::unload_internal()
{
	a_pos_vec2 = -1;
	a_tc_vec2 = -1;
	u_tex_2d = -1;
	u_cctf_id = -1;
	u_cctf_gamma = -1;
	u_sdr_white_level = -1;

	OglProgram::unload_internal();
}

bool OglHdrProgram::reload_internal()
{
	OglHdrProgram::unload_internal();

	if (!OglProgram::reload_internal())
	{
		return false;
	}

	a_pos_vec2 = glGetAttribLocation(program_, "pos_vec2");
	a_tc_vec2 = glGetAttribLocation(program_, "tc_vec2");

	if (a_pos_vec2 >= max_vertex_attributes ||
		a_tc_vec2 >= max_vertex_attributes)
	{
		ri.Printf(PRINT_ALL, "Attribute location out of range.\n");
		return false;
	}

	u_tex_2d = glGetUniformLocation(program_, "tex_2d");
	u_cctf_id = glGetUniformLocation(program_, "cctf_id");
	u_cctf_gamma = glGetUniformLocation(program_, "cctf_gamma");
	u_sdr_white_level = glGetUniformLocation(program_, "sdr_white_level");
	return true;
}

bool OglHdrProgram::do_reload()
{
	return reload_internal();
}

void OglHdrProgram::do_unload()
{
	unload_internal();
}

} // namespace rtcw
