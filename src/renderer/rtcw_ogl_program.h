#ifndef RTCW_OGL_PROGRAM_INCLUDED
#define RTCW_OGL_PROGRAM_INCLUDED


#include <string>

#include "glad.h"


namespace rtcw
{


//
// Base class for GLSL programs.
//
class OglProgram
{
public:
	// Program object.
	GLuint program_;


	OglProgram(
		const std::string& glsl_dir,
		const std::string& base_name);

	OglProgram(
		const char* v_shader_buffer,
		const char* f_shader_buffer);

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

	enum class SourceType
	{
		none,
		file,
		c_string,
	}; // SourceType


	SourceType source_type_;
	std::string glsl_dir_;
	std::string base_name_;
	const char* v_shader_c_string_;
	const char* f_shader_c_string_;

	GLuint ogl_vertex_shader_;
	GLuint ogl_fragment_shader_;


	virtual OglProgram* create_new(
		const std::string& glsl_dir,
		const std::string& base_name) = 0;

	virtual OglProgram* create_new(
		const char* v_shader_buffer,
		const char* f_shader_buffer) = 0;

	bool reload_internal();

	void unload_internal();

	static const char* get_shader_type_string(
		const GLenum shader_type);


private:
	virtual bool do_reload();

	virtual bool do_try_reload();

	virtual void do_unload();


	ReloadShaderResult reload_shader(
		const GLenum shader_type,
		const char* shader_c_string,
		GLuint& shaderObject);

	ReloadShaderResult reload_shader(
		const GLenum shader_type,
		const std::string& file_name,
		GLuint& shaderObject);

	static std::string get_compile_log(
		const GLuint shader_object);

	std::string get_link_log();
}; // class OglProgram


} // rtcw


#endif // !RTCW_OGL_PROGRAM_INCLUDED
