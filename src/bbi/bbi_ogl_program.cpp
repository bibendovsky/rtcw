//
// Custom library.
// Copyright (C) 2012-2013 Boris I. Bendovsky
//
// A helper class for an OpenGL program object.
//


#include "bbi_ogl_program.h"

#include <memory>


namespace bbi {


// (static)
GLuint OglProgram::create (GLuint fragment_object, GLuint vertex_object)
{
    if ((::glIsShader (fragment_object) == GL_FALSE) ||
        (::glIsShader (vertex_object) == GL_FALSE))
    {
        return GL_NONE;
    }

    auto program_object = ::glCreateProgram ();

    if (program_object == GL_NONE)
        return GL_NONE;

    ::glAttachShader (program_object, fragment_object);
    ::glAttachShader (program_object, vertex_object);

    return program_object;
}

// (static)
GLuint OglProgram::create (GLuint fragment_object, GLuint geometry_object,
    GLuint vertex_object)
{
    if ((::glIsShader (fragment_object) == GL_FALSE) ||
        (::glIsShader (geometry_object) == GL_FALSE) ||
        (::glIsShader (vertex_object) == GL_FALSE))
    {
        return GL_NONE;
    }

    auto program_object = ::glCreateProgram ();

    if (program_object == GL_NONE)
        return GL_NONE;

    ::glAttachShader (program_object, fragment_object);
    ::glAttachShader (program_object, geometry_object);
    ::glAttachShader (program_object, vertex_object);

    return program_object;
}

// (static)
bool OglProgram::link (GLuint program_object)
{
    if (::glIsProgram (program_object) == GL_FALSE)
        return false;

    GLint link_status = GL_FALSE;

    ::glLinkProgram (program_object);
    ::glGetProgramiv (program_object, GL_LINK_STATUS, &link_status);

    return link_status != GL_FALSE;
}

// (static)
bool OglProgram::is_linked (GLuint program_object)
{
    if (::glIsProgram (program_object) == GL_FALSE)
        return false;

    GLint link_status = GL_FALSE;

    ::glGetProgramiv (program_object, GL_LINK_STATUS, &link_status);

    return link_status != GL_FALSE;
}

// (static)
std::string OglProgram::link_log (GLuint program_object)
{
    if (::glIsProgram (program_object) == GL_FALSE)
        return std::string ();

    GLint info_log_size = 0; // with a null terminator

    ::glGetProgramiv (program_object, GL_INFO_LOG_LENGTH, &info_log_size);

    if (info_log_size == 0)
        return std::string ();


    GLsizei info_log_length; // without a null terminator

    std::unique_ptr<GLchar> infoLog (new GLchar[info_log_size]);

    ::glGetProgramInfoLog (program_object, info_log_size, &info_log_length,
        infoLog.get ());

    if (info_log_length == 0)
        return std::string ();

    return std::string (infoLog.get (), info_log_length);
}

// (static)
OglUniformInfo OglProgram::uniform_info (GLuint program_object,
    const GLchar* const uniform_name)
{
    auto uniform_names = uniform_name;
    GLuint uniform_indices = GL_INVALID_INDEX;

    ::glGetUniformIndices (program_object, 1, &uniform_names, &uniform_indices);

    if (uniform_indices == GL_INVALID_INDEX)
        return OglUniformInfo ();

    OglUniformInfo result;

    result.index = static_cast<int> (uniform_indices);

    ::glGetActiveUniformsiv (program_object, 1, &uniform_indices,
        GL_UNIFORM_TYPE, &result.item_type);

    ::glGetActiveUniformsiv (program_object, 1, &uniform_indices,
        GL_UNIFORM_SIZE, &result.item_count);

    ::glGetActiveUniformsiv (program_object, 1, &uniform_indices,
        GL_UNIFORM_BLOCK_INDEX, &result.block_index);

    ::glGetActiveUniformsiv (program_object, 1, &uniform_indices,
        GL_UNIFORM_OFFSET, &result.buffer_offset);

    ::glGetActiveUniformsiv (program_object, 1, &uniform_indices,
        GL_UNIFORM_ARRAY_STRIDE, &result.array_stride);

    ::glGetActiveUniformsiv (program_object, 1, &uniform_indices,
        GL_UNIFORM_MATRIX_STRIDE, &result.matrix_stride);

    return result;
}


} // namespace bbi
