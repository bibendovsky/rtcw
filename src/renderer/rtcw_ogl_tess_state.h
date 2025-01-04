#ifndef RTCW_OGL_TESS_STATE_INCLUDED
#define RTCW_OGL_TESS_STATE_INCLUDED


#include "glm/glm.hpp"
#include "rtcw_mod_value.h"
#include "rtcw_ogl_tess_program.h"


namespace rtcw
{


class OglTessState
{
public:
	rtcw::ModValue<glm::mat4> projection{};
	rtcw::ModValue<glm::mat4> model_view{};

	rtcw::ModValue<bool> use_multitexturing{};

	rtcw::ModValue<GLint> tex_2d[2]{};
	rtcw::ModValue<GLenum> tex_env_mode[2]{};

	rtcw::ModValue<glm::vec4> primary_color{};

	rtcw::ModValue<bool> use_alpha_test{};
	rtcw::ModValue<GLenum> alpha_test_func{};
	rtcw::ModValue<float> alpha_test_ref{};

	rtcw::ModValue<bool> use_fog{};
	rtcw::ModValue<GLenum> fog_mode{};
	rtcw::ModValue<GLenum> fog_dist_mode{};
	rtcw::ModValue<GLenum> fog_hint{};
	rtcw::ModValue<float> fog_density{};
	rtcw::ModValue<float> fog_start{};
	rtcw::ModValue<float> fog_end{};
	rtcw::ModValue<glm::vec4> fog_color{};


	OglTessState();

	~OglTessState();


	void set_program(
		rtcw::OglTessProgram* program);

	void set_default_values();

	void commit_changes();

	void invalidate();

	void invalidate_and_commit();


private:
	rtcw::OglTessProgram* program_{};


	bool is_program_valid() const;

	OglTessState(const OglTessState& that);
	OglTessState& operator = (const OglTessState& that);
}; // OglTessState


} // rtcw


#endif // !RTCW_OGL_TESS_STATE_INCLUDED
