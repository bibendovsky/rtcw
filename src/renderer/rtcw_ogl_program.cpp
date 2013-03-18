#include "rtcw_ogl_program.h"

#include <memory>

#include "tr_local.h"

#include "bbi_ogl_program.h"
#include "bbi_ogl_shader.h"


namespace rtcw {


OglProgram::OglProgram (const std::string& glsl_dir,
    const std::string& base_name) :
        program (0),
        glsl_dir_ (glsl_dir),
        base_name_ (base_name),
        vertex_shader_ (0),
        fragment_shader_ (0)
{
}

OglProgram::~OglProgram ()
{
    unload ();
}

// (virtual)
bool OglProgram::reload ()
{
    unload ();


    bool result = false;

    const std::string p_name = glsl_dir_ + base_name_;

    ri.Printf (PRINT_ALL, "\"%s\"\n", p_name.c_str ());

    auto v_name = p_name + "_vs.txt";
    auto f_name = p_name + "_fs.txt";

    bool compile_program = false;

    auto v_result = reload_shader (
        GL_VERTEX_SHADER, v_name, vertex_shader_);

    if (v_result == RSR_COMPILED) {
        auto f_result = reload_shader (
            GL_FRAGMENT_SHADER, f_name, fragment_shader_);

        compile_program = (f_result == RSR_COMPILED);
    }

    if (compile_program) {
        program = ::glCreateProgram ();

        ::glAttachShader (program, vertex_shader_);
        ::glAttachShader (program, fragment_shader_);

        GLint link_status = GL_FALSE;

        ::glLinkProgram (program);
        ::glGetProgramiv (program, GL_LINK_STATUS, &link_status);

        auto link_log = bbi::OglProgram::link_log (program);

        if (link_status != GL_FALSE) {
            if (!link_log.empty ()) {
                ri.Printf (PRINT_ALL, "Linkage log:\n%s\n",
                    link_log.c_str ());
            }

            result = true;
        } else {
            ri.Printf (PRINT_ALL, "Failed to link:\n%s\n",
                link_log.c_str ());
        }
    }

    if (!result)
        unload ();

    return result;
}

// (virtual)
void OglProgram::unload ()
{
    if (vertex_shader_ != 0) {
        ::glDeleteShader (vertex_shader_);
        vertex_shader_ = 0;
    }

    if (fragment_shader_ != 0) {
        ::glDeleteShader (fragment_shader_);
        fragment_shader_ = 0;
    }

    if (program != 0) {
        ::glDeleteProgram (program);
        program = 0;
    }
}

// (virtual)
bool OglProgram::try_reload ()
{
    std::unique_ptr<OglProgram> instance (create_new (glsl_dir_, base_name_));

    return instance->reload ();
}

OglProgram::ReloadShaderResult OglProgram::reload_shader (GLenum shader_type,
    const std::string& file_name, GLuint& shader_object)
{
    auto result = RSR_UNKNOWN;

    void* source_buffer = nullptr;

    int source_length = ri.FS_ReadFile (file_name.c_str (), &source_buffer);

    shader_object = 0;

    bool is_compiled = false;

    if (source_length > 0) {
        const GLchar* lines[1] = { static_cast<GLchar*> (source_buffer), };
        int lengths[1] = { source_length, };

        shader_object = ::glCreateShader (shader_type);
        ::glShaderSource (shader_object, 1, lines, lengths);
        ::glCompileShader (shader_object);

        GLint compile_status = GL_FALSE;

        ::glGetShaderiv (shader_object, GL_COMPILE_STATUS, &compile_status);

        auto compile_log = bbi::OglShader::compile_log (shader_object);

        if (compile_status != GL_FALSE) {
            if (!compile_log.empty ()) {
                ri.Printf (PRINT_ALL, "Compilation log of \"%s\":\n%s\n",
                    file_name.c_str (), compile_log.c_str ());
            }

            result = RSR_COMPILED;
        } else {
            ri.Printf (PRINT_ALL, "Failed to compile a shader \"%s\":\n%s\n",
                file_name.c_str (), compile_log.c_str ());
            result = RSR_NOT_COMPILED;
        }
    }

    if (source_buffer != nullptr)
        ri.FS_FreeFile (source_buffer);

    if (result != RSR_COMPILED) {
        ::glDeleteShader (shader_object);
        shader_object = 0;
    }

    return result;
}

} // namespace rtcw
