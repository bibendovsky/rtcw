//
// Custom library.
// Copyright (C) 2012-2013 Boris I. Bendovsky
//
// A helper class for an OpenGL shader object.
//


#ifndef BBI_OGL_SHADER_H
#define BBI_OGL_SHADER_H


#include <string>

#include <GL/glew.h>


namespace bbi {


class OglShader {
public:
    static GLuint create (GLenum shader_type, const GLchar* const source_line,
        GLint source_length = -1);

    static GLuint create (GLenum shader_type, GLsizei line_count,
        const GLchar** const source_lines, const GLint* source_lengths);

    static GLuint create_fragment (const GLchar* const source,
        GLint source_length = -1);

    static GLuint create_geometry (const GLchar* const source,
        GLint source_length = -1);

    static GLuint create_vertex (const GLchar* const source,
        GLint source_length = -1);


    static void set_source_code (GLuint shader_object,
        const GLchar* const source, GLint source_length = -1);

    static bool compile (GLuint shader_object);

    static bool is_compiled (GLuint shaderObject);

    static std::string compile_log (GLuint shader_object);


private:
    OglShader ();

    OglShader (const OglShader& that);

    ~OglShader ();

    OglShader& operator = (const OglShader& that);
}; // class OglShader


} // namespace bbi


#endif // BBI_OGL_SHADER_H
