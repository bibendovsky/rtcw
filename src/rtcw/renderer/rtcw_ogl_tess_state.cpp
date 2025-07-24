/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_ogl_tess_state.h"
#include <assert.h>
#include "qgl.h"

namespace rtcw {

OglTessState::OglTessState()
	:
	is_dirty_(),
	program_(NULL)
{
	set_default_values();
}

void OglTessState::use_program()
{
	GLuint gl_program = program_ != NULL ? program_->program_ : 0;
	glUseProgram(gl_program);
}

void OglTessState::set_program(
	rtcw::OglTessProgram* program)
{
	program_ = program;

	invalidate();
}

void OglTessState::set_default_values()
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

	disable_all_vertex_attrib_arrays();
}

void OglTessState::commit_changes()
{
	const bool has_program = is_program_valid();

	if (is_dirty_ || model_view != old_model_view)
	{
		if (has_program && program_->u_model_view_mat4 >= 0)
		{
			glUniformMatrix4fv(program_->u_model_view_mat4, 1, GL_FALSE, model_view.get_data());
		}

		old_model_view = model_view;
	}

	if (is_dirty_ || projection != old_projection)
	{
		if (has_program && program_->u_projection_mat4 >= 0)
		{
			glUniformMatrix4fv(program_->u_projection_mat4, 1, GL_FALSE, projection.get_data());
		}

		old_projection = projection;
	}

	if (is_dirty_ || use_multitexturing != old_use_multitexturing)
	{
		if (has_program && program_->u_use_multitexturing >= 0)
		{
			glUniform1i(program_->u_use_multitexturing, use_multitexturing);
		}

		old_use_multitexturing = use_multitexturing;
	}

	if (is_dirty_ || tex_2d[0] != old_tex_2d[0])
	{
		if (has_program && program_->u_tex_2d[0] >= 0)
		{
			glUniform1i(program_->u_tex_2d[0], tex_2d[0]);
		}

		old_tex_2d[0] = tex_2d[0];
	}

	if (is_dirty_ || tex_2d[1] != old_tex_2d[1])
	{
		if (has_program && program_->u_tex_2d[1] >= 0)
		{
			glUniform1i(program_->u_tex_2d[1], tex_2d[1]);
		}

		old_tex_2d[1] = tex_2d[1];
	}

	if (is_dirty_ || tex_env_mode[0] != old_tex_env_mode[0])
	{
		if (has_program && program_->u_tex_env_mode[0] >= 0)
		{
			glUniform1i(program_->u_tex_env_mode[0], tex_env_mode[0]);
		}

		old_tex_env_mode[0] = tex_env_mode[0];
	}

	if (is_dirty_ || tex_env_mode[1] != old_tex_env_mode[1])
	{
		if (has_program && program_->u_tex_env_mode[1] >= 0)
		{
			glUniform1i(program_->u_tex_env_mode[1], tex_env_mode[1]);
		}

		old_tex_env_mode[1] = tex_env_mode[1];
	}

	if (is_dirty_ || primary_color != old_primary_color)
	{
		if (has_program && program_->u_primary_color >= 0)
		{
			glUniform4fv(program_->u_primary_color, 1, primary_color.get_data());
		}

		old_primary_color = primary_color;
	}

	if (is_dirty_ || use_alpha_test != old_use_alpha_test)
	{
		if (has_program && program_->u_use_alpha_test >= 0)
		{
			glUniform1i(program_->u_use_alpha_test, use_alpha_test);
		}

		old_use_alpha_test = use_alpha_test;
	}

	if (is_dirty_ || alpha_test_func != old_alpha_test_func)
	{
		if (has_program && program_->u_alpha_test_func >= 0)
		{
			glUniform1i(program_->u_alpha_test_func, alpha_test_func);
		}

		old_alpha_test_func = alpha_test_func;
	}

	if (is_dirty_ || alpha_test_ref != old_alpha_test_ref)
	{
		if (has_program && program_->u_alpha_test_ref >= 0)
		{
			glUniform1f(program_->u_alpha_test_ref, alpha_test_ref);
		}

		old_alpha_test_ref = alpha_test_ref;
	}

	if (is_dirty_ || use_fog != old_use_fog)
	{
		if (has_program && program_->u_use_fog >= 0)
		{
			glUniform1i(program_->u_use_fog, use_fog);
		}

		old_use_fog = use_fog;
	}

	if (is_dirty_ || fog_mode != old_fog_mode)
	{
		if (has_program && program_->u_fog_mode >= 0)
		{
			glUniform1i(program_->u_fog_mode, fog_mode);
		}

		old_fog_mode = fog_mode;
	}

	if (is_dirty_ || fog_dist_mode != old_fog_dist_mode)
	{
		if (has_program && program_->u_fog_dist_mode >= 0)
		{
			glUniform1i(program_->u_fog_dist_mode, fog_dist_mode);
		}

		old_fog_dist_mode = fog_dist_mode;
	}

	if (is_dirty_ || fog_hint != old_fog_hint)
	{
		if (has_program && program_->u_fog_hint >= 0)
		{
			glUniform1i(program_->u_fog_hint, fog_hint);
		}

		old_fog_hint = fog_hint;
	}

	if (is_dirty_ || fog_density != old_fog_density)
	{
		if (has_program && program_->u_fog_density >= 0)
		{
			glUniform1f(program_->u_fog_density, fog_density);
		}

		old_fog_density = fog_density;
	}

	if (is_dirty_ || fog_start != old_fog_start)
	{
		if (has_program && program_->u_fog_start >= 0)
		{
			glUniform1f(program_->u_fog_start, fog_start);
		}

		old_fog_start = fog_start;
	}

	if (is_dirty_ || fog_end != old_fog_end)
	{
		if (has_program && program_->u_fog_end >= 0)
		{
			glUniform1f(program_->u_fog_end, fog_end);
		}

		old_fog_end = fog_end;
	}

	if (is_dirty_ || fog_color != old_fog_color)
	{
		if (has_program && program_->u_fog_color >= 0)
		{
			glUniform4fv(program_->u_fog_color, 1, fog_color.get_data());
		}

		old_fog_color = fog_color;
	}

	if (is_dirty_ || intensity != old_intensity)
	{
		if (has_program && program_->u_intensity >= 0)
		{
			glUniform1f(program_->u_intensity, intensity);
		}

		old_intensity = intensity;
	}

	if (is_dirty_ || overbright != old_overbright)
	{
		if (has_program && program_->u_overbright >= 0)
		{
			glUniform1f(program_->u_overbright, overbright);
		}

		old_overbright = overbright;
	}

	if (is_dirty_ || gamma != old_gamma)
	{
		if (has_program && program_->u_gamma >= 0)
		{
			glUniform1f(program_->u_gamma, gamma);
		}

		old_gamma = gamma;
	}

	if (is_dirty_ || enabled_vertex_attrib_arrays != old_enabled_vertex_attrib_arrays)
	{
		for (int i_array_index = 0; i_array_index < OglProgram::max_vertex_attributes; ++i_array_index)
		{
			const EnabledVertexAttribArrays mask = 1U << i_array_index;
			const EnabledVertexAttribArrays old_value = old_enabled_vertex_attrib_arrays & mask;
			const EnabledVertexAttribArrays new_value = enabled_vertex_attrib_arrays & mask;

			if (is_dirty_ || new_value != old_value)
			{
				const GLuint gl_array_index = static_cast<GLuint>(i_array_index);

				if (new_value)
				{
					glEnableVertexAttribArray(gl_array_index);
				}
				else
				{
					glDisableVertexAttribArray(gl_array_index);
				}
			}
		}

		old_enabled_vertex_attrib_arrays = enabled_vertex_attrib_arrays;
	}

	is_dirty_ = false;
}

void OglTessState::invalidate()
{
	is_dirty_ = true;
}

void OglTessState::invalidate_and_commit()
{
	invalidate();
	commit_changes();
}

bool OglTessState::is_program_valid() const
{
	return program_ != NULL && program_->program_ != 0;
}

void OglTessState::disable_all_vertex_attrib_arrays()
{
	enabled_vertex_attrib_arrays = 0;
}

void OglTessState::enable_vertex_attrib_array(int array_index)
{
	enabled_vertex_attrib_arrays |= 1U << array_index;
}

} // namespace rtcw
