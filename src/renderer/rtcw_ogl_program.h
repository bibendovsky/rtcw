#ifndef RTCW_OGL_PROGRAM_H
#define RTCW_OGL_PROGRAM_H


#include <string>

#include <GL/glew.h>


namespace rtcw {


//
// Base class for GLSL programs.
//
class OglProgram {
public:
    // Program object.
    GLuint program;


    OglProgram (const std::string& glsl_dir, const std::string& base_name);

    virtual ~OglProgram ();

    virtual bool reload ();
    virtual bool try_reload ();
    virtual void unload ();


protected:
    enum ReloadShaderResult {
        RSR_UNKNOWN,
        RSR_NO_SOURCE,
        RSR_EMPTY_SOURCE,
        RSR_COMPILED,
        RSR_NOT_COMPILED,
    }; // enum ReloadShaderResult


    std::string glsl_dir_;
    std::string base_name_;

    GLuint vertex_shader_;
    GLuint fragment_shader_;

    virtual OglProgram* create_new (const std::string& glsl_dir,
        const std::string& base_name) = 0;


private:
    OglProgram (const OglProgram& that);
    OglProgram& operator = (const OglProgram& that);

    ReloadShaderResult reload_shader (GLenum shader_type,
        const std::string& file_name, GLuint& shaderObject);
}; // class OglProgram


} // namespace rtcw


#endif // RTCW_OGL_PROGRAM_H
