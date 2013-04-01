//
// Custom library.
// Copyright (C) 2012-2013 Boris I. Bendovsky
//
// A helper class for an OpenGL program object.
//


#ifndef BBI_OGL_PROGRAM_H
#define BBI_OGL_PROGRAM_H


#include <string>

#include <GL/glew.h>

#include "bbi_ogl_uniform_info.h"


namespace bbi {


class OglProgram {
public:
    // Creates a program object and attaches shader objects.
    static GLuint create (GLuint fragment_object, GLuint vertex_object);

    // Creates a program object and attaches shader objects.
    static unsigned create (GLuint fragment_object, GLuint geometry_object,
        GLuint vertex_object);

    // Links a program object.
    static bool link (GLuint program_object);

    // Returns a status of the last linking.
    static bool is_linked (GLuint program_object);

    // Returns a last log of linking.
    static std::string link_log (GLuint program_object);

    // Returns a complete info about a uniform.
    static OglUniformInfo uniform_info (GLuint program_object,
        const GLchar* const uniform_name);


private:
    OglProgram ();

    OglProgram (const OglProgram& that);

    ~OglProgram ();

    OglProgram& operator = (const OglProgram& that);
}; // class OglProgram


} // namespace bbi


#endif // BBI_OGL_PROGRAM_H
