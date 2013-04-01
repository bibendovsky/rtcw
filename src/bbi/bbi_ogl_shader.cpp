//
// Custom library.
// Copyright (C) 2012-2013 Boris I. Bendovsky
//
// A helper class for an OpenGL shader object.
//


#include "bbi_ogl_shader.h"

#include <memory>


namespace bbi {


// (static)
GLuint OglShader::create (GLenum shader_type, const GLchar* const source,
    GLint source_length)
{
    auto shader_object = ::glCreateShader (shader_type);

    if (shader_object == GL_NONE)
        return GL_NONE;


    auto strings = source;
    auto lengths = source_length;

    ::glShaderSource (shader_object, 1, &strings, &lengths);
    ::glCompileShader (shader_object);

    return shader_object;
}

// (static)
GLuint OglShader::create (GLenum shader_type, GLsizei line_count,
    const GLchar** const source_lines, const GLint* source_lengths)
{
    auto shader_object = ::glCreateShader (shader_type);

    if (shader_object == GL_NONE)
        return GL_NONE;

    ::glShaderSource (shader_object, line_count, source_lines, source_lengths);
    ::glCompileShader (shader_object);

    return shader_object;
}

// (static)
GLuint OglShader::create_fragment (const GLchar* const source,
    GLint source_length)
{
    return create (GL_FRAGMENT_SHADER, source, source_length);
}

// (static)
GLuint OglShader::create_geometry (const GLchar* const source,
    GLint source_length)
{
    return create (GL_GEOMETRY_SHADER, source, source_length);
}

// (static)
GLuint OglShader::create_vertex (const GLchar* const source,
    GLint source_length)
{
    return create (GL_VERTEX_SHADER, source, source_length);
}

// (static)
void OglShader::set_source_code (GLuint shader_object,
    const GLchar* const source, GLint source_length)
{
    if (::glIsShader (shader_object) == GL_FALSE)
        return;

    auto strings = source;
    auto lengths = source_length;

    ::glShaderSource (shader_object, 1, &strings, &lengths);
}

// (static)
bool OglShader::compile (GLuint shader_object)
{
    if (::glIsShader (shader_object) == GL_FALSE)
        return false;

    GLint compileStatus = GL_FALSE;

    ::glCompileShader (shader_object);
    ::glGetShaderiv (shader_object, GL_COMPILE_STATUS, &compileStatus);

    return compileStatus != GL_FALSE;
}

// (static)
bool OglShader::is_compiled (GLuint shader_object)
{
    if (::glIsShader (shader_object) == GL_FALSE)
        return false;

    GLint compileStatus = GL_FALSE;

    ::glGetShaderiv (shader_object, GL_COMPILE_STATUS, &compileStatus);

    return compileStatus != GL_FALSE;
}

// (static)
std::string OglShader::compile_log (GLuint shader_object)
{
    if (::glIsShader (shader_object) == GL_FALSE)
        return std::string ();


    GLint info_log_size = 0; // with a null terminator

    ::glGetShaderiv (shader_object, GL_INFO_LOG_LENGTH, &info_log_size);

    if (info_log_size <= 0)
        return std::string ();

    GLsizei info_log_length; // without a null terminator

    std::unique_ptr<GLchar> info_log (new GLchar[info_log_size]);

    ::glGetShaderInfoLog (shader_object, info_log_size, &info_log_length,
        info_log.get ());

    if (info_log_length != 0)
        return std::string (info_log.get (), info_log_length);

    return std::string ();
}


} // namespace bbi
