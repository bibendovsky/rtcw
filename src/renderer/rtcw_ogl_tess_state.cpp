#include "rtcw_ogl_tess_state.h"
#include <cassert>
#include "qgl.h"


namespace rtcw
{


OglTessState::OglTessState() :
	program_(NULL)
{
	set_default_values();
}

OglTessState::~OglTessState() {}

void OglTessState::set_program(
	rtcw::OglTessProgram* program)
{
	program_ = program;

	invalidate();
}

void OglTessState::set_default_values()
{
	model_view.set(cgm::Mat4::identity);
	projection.set(cgm::Mat4::identity);

	use_multitexturing.set(false);

	tex_2d[0].set(0);
	tex_2d[1].set(1);

	tex_env_mode[0].set(GL_MODULATE);
	tex_env_mode[1].set(GL_MODULATE);

	primary_color.set(cgm::Vec4(1.0F, 1.0F, 1.0F, 1.0F));

	use_alpha_test.set(false);
	alpha_test_func.set(GL_ALWAYS);
	alpha_test_ref.set(0.0F);

	use_fog.set(false);
	fog_mode.set(GL_EXP);
	fog_dist_mode.set(GL_NONE);
	fog_hint.set(GL_DONT_CARE);
	fog_density.set(1.0F);
	fog_start.set(0.0F);
	fog_end.set(1.0F);
	fog_color.set(cgm::Vec4());

	intensity.set(1.0F);
	overbright.set(1.0F);
	gamma.set(1.0F);
}

void OglTessState::commit_changes()
{
	const bool has_changes =
		model_view.is_modified() ||
		projection.is_modified() ||
		use_multitexturing.is_modified() ||
		tex_2d[0].is_modified() ||
		tex_2d[1].is_modified() ||
		tex_env_mode[0].is_modified() ||
		tex_env_mode[1].is_modified() ||
		primary_color.is_modified() ||
		use_alpha_test.is_modified() ||
		alpha_test_func.is_modified() ||
		alpha_test_ref.is_modified() ||
		use_fog.is_modified() ||
		fog_mode.is_modified() ||
		fog_dist_mode.is_modified() ||
		fog_hint.is_modified() ||
		fog_density.is_modified() ||
		fog_start.is_modified() ||
		fog_end.is_modified() ||
		fog_color.is_modified() ||
		intensity.is_modified() ||
		overbright.is_modified() ||
		gamma.is_modified()
	;

	if (!has_changes)
	{
		return;
	}

	const bool use_program = is_program_valid();

	if (use_program)
	{
		glUseProgram(program_->program_);
	}


	if (model_view.is_modified())
	{
		if (use_program && (program_->u_model_view_mat4 >= 0))
		{
			glUniformMatrix4fv(program_->u_model_view_mat4, 1, GL_FALSE, model_view.get().get_data());
		}

		model_view.set_modified(false);
	}

	if (projection.is_modified())
	{
		if (use_program && (program_->u_projection_mat4 >= 0))
		{
			glUniformMatrix4fv(program_->u_projection_mat4, 1, GL_FALSE, projection.get().get_data());
		}

		projection.set_modified(false);
	}

	if (use_multitexturing.is_modified())
	{
		if (use_program && (program_->u_use_multitexturing >= 0))
		{
			glUniform1i(program_->u_use_multitexturing, use_multitexturing.get());
		}

		use_multitexturing.set_modified(false);
	}

	if (tex_2d[0].is_modified())
	{
		if (use_program && (program_->u_tex_2d[0] >= 0))
		{
			glUniform1i(program_->u_tex_2d[0], tex_2d[0].get());
		}

		tex_2d[0].set_modified(false);
	}

	if (tex_2d[1].is_modified())
	{
		if (use_program && (program_->u_tex_2d[1] >= 0))
		{
			glUniform1i(program_->u_tex_2d[1], tex_2d[1].get());
		}

		tex_2d[1].set_modified(false);
	}

	if (tex_env_mode[0].is_modified())
	{
		if (use_program && (program_->u_tex_env_mode[0] >= 0))
		{
			glUniform1i(program_->u_tex_env_mode[0], tex_env_mode[0].get());
		}

		tex_env_mode[0].set_modified(false);
	}

	if (tex_env_mode[1].is_modified())
	{
		if (use_program && (program_->u_tex_env_mode[1] >= 0))
		{
			glUniform1i(program_->u_tex_env_mode[1], tex_env_mode[1].get());
		}

		tex_env_mode[1].set_modified(false);
	}

	if (primary_color.is_modified())
	{
		if (use_program && (program_->u_primary_color >= 0))
		{
			glUniform4fv(program_->u_primary_color, 1, primary_color.get().get_data());
		}

		primary_color.set_modified(false);
	}

	if (use_alpha_test.is_modified())
	{
		if (use_program && (program_->u_use_alpha_test >= 0))
		{
			glUniform1i(program_->u_use_alpha_test, use_alpha_test.get());
		}

		use_alpha_test.set_modified(false);
	}

	if (alpha_test_func.is_modified())
	{
		if (use_program && (program_->u_alpha_test_func >= 0))
		{
			glUniform1i(program_->u_alpha_test_func, alpha_test_func.get());
		}

		alpha_test_func.set_modified(false);
	}

	if (alpha_test_ref.is_modified())
	{
		if (use_program && (program_->u_alpha_test_ref >= 0))
		{
			glUniform1f(program_->u_alpha_test_ref, alpha_test_ref.get());
		}

		alpha_test_ref.set_modified(false);
	}

	if (use_fog.is_modified())
	{
		if (use_program && (program_->u_use_fog >= 0))
		{
			glUniform1i(program_->u_use_fog, use_fog.get());
		}

		use_fog.set_modified(false);
	}

	if (fog_mode.is_modified())
	{
		if (use_program && (program_->u_fog_mode >= 0))
		{
			glUniform1i(program_->u_fog_mode, fog_mode.get());
		}

		fog_mode.set_modified(false);
	}

	if (fog_dist_mode.is_modified())
	{
		if (use_program && (program_->u_fog_dist_mode >= 0))
		{
			glUniform1i(program_->u_fog_dist_mode, fog_dist_mode.get());
		}

		fog_dist_mode.set_modified(false);
	}

	if (fog_hint.is_modified())
	{
		if (use_program && (program_->u_fog_hint >= 0))
		{
			glUniform1i(program_->u_fog_hint, fog_hint.get());
		}

		fog_hint.set_modified(false);
	}

	if (fog_density.is_modified())
	{
		if (use_program && (program_->u_fog_density >= 0))
		{
			glUniform1f(program_->u_fog_density, fog_density.get());
		}

		fog_density.set_modified(false);
	}

	if (fog_start.is_modified())
	{
		if (use_program && (program_->u_fog_start >= 0))
		{
			glUniform1f(program_->u_fog_start, fog_start.get());
		}

		fog_start.set_modified(false);
	}

	if (fog_end.is_modified())
	{
		if (use_program && (program_->u_fog_end >= 0))
		{
			glUniform1f(program_->u_fog_end, fog_end.get());
		}

		fog_end.set_modified(false);
	}

	if (fog_color.is_modified())
	{
		if (use_program && (program_->u_fog_color >= 0))
		{
			glUniform4fv(program_->u_fog_color, 1, fog_color.get().get_data());
		}

		fog_color.set_modified(false);
	}

	if (intensity.is_modified())
	{
		if (use_program && program_->u_intensity >= 0)
		{
			glUniform1f(program_->u_intensity, intensity.get());
		}

		intensity.set_modified(false);
	}

	if (overbright.is_modified())
	{
		if (use_program && program_->u_overbright >= 0)
		{
			glUniform1f(program_->u_overbright, overbright.get());
		}

		overbright.set_modified(false);
	}

	if (gamma.is_modified())
	{
		if (use_program && program_->u_gamma >= 0)
		{
			glUniform1f(program_->u_gamma, gamma.get());
		}

		gamma.set_modified(false);
	}

	if (use_program)
	{
		glUseProgram(0);
	}
}

void OglTessState::invalidate()
{
	model_view.set_modified(true);
	projection.set_modified(true);

	use_multitexturing.set_modified(true);

	tex_2d[0].set_modified(true);
	tex_2d[1].set_modified(true);

	tex_env_mode[0].set_modified(true);
	tex_env_mode[1].set_modified(true);

	primary_color.set_modified(true);

	use_alpha_test.set_modified(true);
	alpha_test_func.set_modified(true);
	alpha_test_ref.set_modified(true);

	use_fog.set_modified(true);
	fog_mode.set_modified(true);
	fog_dist_mode.set_modified(true);
	fog_hint.set_modified(true);
	fog_density.set_modified(true);
	fog_start.set_modified(true);
	fog_end.set_modified(true);
	fog_color.set_modified(true);

	intensity.set_modified(true);
	overbright.set_modified(true);
	gamma.set_modified(true);
}

void OglTessState::invalidate_and_commit()
{
	invalidate();
	commit_changes();
}

bool OglTessState::is_program_valid() const
{
	if (program_ == NULL)
	{
		return false;
	}

	if (program_->program_ == 0)
	{
		return false;
	}

	return true;
}


} // namespace rtcw
