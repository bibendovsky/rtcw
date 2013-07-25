#include "rtcw_ogl_tess_program.h"

#include "rtcw_ogl_program.h"


namespace rtcw {


OglTessProgram::OglTessProgram (const std::string& glsl_dir,
    const std::string& base_name) :
        OglProgram (glsl_dir, base_name),
        a_pos_vec4 (-1),
        a_col_vec4 (-1),
        a_tc0_vec2 (-1),
        a_tc1_vec2 (-1),
        u_projection_mat4 (-1),
        u_model_view_mat4 (-1),
        u_use_alpha_test (-1),
        u_alpha_test_func (-1),
        u_alpha_test_ref (-1),
        u_use_multitexturing (-1),
        u_primary_color (-1),
        u_use_fog (-1),
        u_fog_mode (-1),
        u_fog_dist_mode (-1),
        u_fog_hint (-1),
        u_fog_color (-1),
        u_fog_density (-1),
        u_fog_start (-1),
        u_fog_end (-1)
{
    u_tex_env_mode[0] = -1;
    u_tex_env_mode[1] = -1;
    u_tex_2d[0] = -1;
    u_tex_2d[2] = -1;
}

OglTessProgram::~OglTessProgram ()
{
}

bool OglTessProgram::reload ()
{
    if (!OglProgram::reload ())
        return false;


    a_pos_vec4 = ::glGetAttribLocation (
        program, "pos_vec4");

    a_col_vec4 = ::glGetAttribLocation (
        program, "col_vec4");

    a_tc0_vec2 = ::glGetAttribLocation (
        program, "tc0_vec2");

    a_tc1_vec2 = ::glGetAttribLocation (
        program, "tc1_vec2");

    u_projection_mat4 = ::glGetUniformLocation (
        program, "projection_mat4");

    u_model_view_mat4 = ::glGetUniformLocation (
        program, "model_view_mat4");

    u_use_alpha_test = ::glGetUniformLocation (
        program, "use_alpha_test");

    u_alpha_test_func = ::glGetUniformLocation (
        program, "alpha_test_func");

    u_alpha_test_ref = ::glGetUniformLocation (
        program, "alpha_test_ref");

    u_tex_env_mode[0] = ::glGetUniformLocation (
        program, "tex_env_mode[0]");

    u_tex_env_mode[1] = ::glGetUniformLocation (
        program, "tex_env_mode[1]");

    u_use_multitexturing = ::glGetUniformLocation (
        program, "use_multitexturing");

    u_tex_2d[0] = ::glGetUniformLocation (
        program, "tex_2d[0]");

    u_tex_2d[1] = ::glGetUniformLocation (
        program, "tex_2d[1]");

    u_primary_color = ::glGetUniformLocation (
        program, "primary_color");

    u_use_fog = ::glGetUniformLocation (
        program, "use_fog");

    u_fog_mode = ::glGetUniformLocation (
        program, "fog_mode");

    u_fog_dist_mode = ::glGetUniformLocation (
        program, "fog_dist_mode");

    u_fog_hint = ::glGetUniformLocation (
        program, "fog_hint");

    u_fog_color = ::glGetUniformLocation (
        program, "fog_color");

    u_fog_density = ::glGetUniformLocation (
        program, "fog_density");

    u_fog_start = ::glGetUniformLocation (
        program, "fog_start");

    u_fog_end = ::glGetUniformLocation (
        program, "fog_end");


    if (a_col_vec4 >= 0)
        ::glVertexAttrib4f (a_col_vec4, 1.0F, 1.0F, 1.0F, 1.0F);

    if (a_tc0_vec2 >= 0)
        ::glVertexAttrib2f (a_tc0_vec2, 0.0F, 0.0F);

    if (a_tc1_vec2 >= 0)
        ::glVertexAttrib2f (a_tc1_vec2, 0.0F, 0.0F);

    return true;
}

void OglTessProgram::unload ()
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

    OglProgram::unload ();
}

// (virtual)
OglProgram* OglTessProgram::create_new (const std::string& glsl_dir,
    const std::string& base_name)
{
    return new OglTessProgram (glsl_dir, base_name);
}


} // namespace rtcw
