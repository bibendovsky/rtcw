#ifndef RTCW_OGL_PROGRAM_INCLUDED
#define RTCW_OGL_PROGRAM_INCLUDED


#include <string>
#include "SDL_opengl.h"


namespace rtcw
{


//
// Base class for GLSL programs.
//
class OglProgram
{
public:
	// Program object.
	GLuint program;


	OglProgram(
		const std::string& glsl_dir,
		const std::string& base_name);

	OglProgram(
		const OglProgram& that) = delete;

	OglProgram& operator=(
		const OglProgram& that) = delete;

	virtual ~OglProgram();

	bool reload();
	bool try_reload();
	void unload();


protected:
	enum class ReloadShaderResult
	{
		none,
		no_source,
		empty_source,
		compiled,
		not_compiled,
	}; // ReloadShaderResult


	std::string glsl_dir_;
	std::string base_name_;

	GLuint vertex_shader_;
	GLuint fragment_shader_;

	virtual OglProgram* create_new(
		const std::string& glsl_dir,
		const std::string& base_name) = 0;

	bool reload_internal();
	void unload_internal();


private:
	virtual bool do_reload();
	virtual bool do_try_reload();
	virtual void do_unload();


	ReloadShaderResult reload_shader(
		const GLenum shader_type,
		const std::string& file_name,
		GLuint& shaderObject);

	static std::string get_compile_log(
		const GLuint shader_object);

	std::string get_link_log();
}; // class OglProgram


} // rtcw


#endif // RTCW_OGL_PROGRAM_INCLUDED
