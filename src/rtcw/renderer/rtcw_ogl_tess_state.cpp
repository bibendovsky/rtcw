/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_ogl_tess_state.h"

namespace rtcw {

OglTessState::OglTessState()
	:
	program_()
{
	reset();
}

void OglTessState::set_program(const rtcw::OglTessProgram* program)
{
	program_ = program;
}

void OglTessState::reset()
{
	model_view = cgm::Mat4::identity;
	projection = cgm::Mat4::identity;

	use_multitexturing = false;
	tex_2d[0] = 0;
	tex_2d[1] = 1;
	tex_env_mode[0] = GL_MODULATE;
	tex_env_mode[1] = GL_MODULATE;

	primary_color = cgm::Vec4(1.0F, 1.0F, 1.0F, 1.0F);

	use_alpha_test = false;
	alpha_test_func = GL_ALWAYS;
	alpha_test_ref = 0.0F;

	use_fog = false;
	fog_mode = GL_EXP;
	fog_dist_mode = GL_NONE;
	fog_hint = GL_DONT_CARE;
	fog_density = 1.0F;
	fog_start = 0.0F;
	fog_end = 1.0F;
	fog_color = cgm::Vec4();

	intensity = 1.0F;
	overbright = 1.0F;
	gamma = 1.0F;
}

void OglTessState::commit()
{
	if (!is_program_valid())
	{
		return;
	}

	glUseProgram(program_->program_);

	glUniformMatrix4fv(program_->u_model_view_mat4, 1, GL_FALSE, model_view.get_data());
	glUniformMatrix4fv(program_->u_projection_mat4, 1, GL_FALSE, projection.get_data());

	glUniform1i(program_->u_use_multitexturing, use_multitexturing);
	glUniform1i(program_->u_tex_2d[0], tex_2d[0]);
	glUniform1i(program_->u_tex_2d[1], tex_2d[1]);
	glUniform1i(program_->u_tex_env_mode[0], tex_env_mode[0]);
	glUniform1i(program_->u_tex_env_mode[1], tex_env_mode[1]);

	glUniform4fv(program_->u_primary_color, 1, primary_color.get_data());

	glUniform1i(program_->u_use_alpha_test, use_alpha_test);

	if (use_alpha_test)
	{
		glUniform1i(program_->u_alpha_test_func, alpha_test_func);
		glUniform1f(program_->u_alpha_test_ref, alpha_test_ref);
	}

	glUniform1i(program_->u_use_fog, use_fog);

	if (use_fog)
	{
		glUniform1i(program_->u_fog_mode, fog_mode);
		glUniform1i(program_->u_fog_dist_mode, fog_dist_mode);
		glUniform1i(program_->u_fog_hint, fog_hint);
		glUniform1f(program_->u_fog_density, fog_density);
		glUniform1f(program_->u_fog_start, fog_start);
		glUniform1f(program_->u_fog_end, fog_end);
		glUniform4fv(program_->u_fog_color, 1, fog_color.get_data());
	}

	glUniform1f(program_->u_intensity, intensity);
	glUniform1f(program_->u_overbright, overbright);
	glUniform1f(program_->u_gamma, gamma);
}

bool OglTessState::is_program_valid() const
{
	return program_ != NULL && program_->program_ != 0;
}

} // namespace rtcw
