#ifndef RTCW_OGL_PROGRAM_INCLUDED
#define RTCW_OGL_PROGRAM_INCLUDED


#include "glad.h"
#include "rtcw_string.h"


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
		const String& glsl_dir,
		const String& base_name);

	OglProgram(
		const char* v_shader_buffer,
		const char* f_shader_buffer);

	virtual ~OglProgram();


	bool reload();

	bool try_reload();

	void unload();


protected:
	enum ReloadShaderResult
	{
		reload_result_none,
		reload_result_no_source,
		reload_result_empty_source,
		reload_result_compiled,
		reload_result_not_compiled,
	}; // ReloadShaderResult

	enum SourceType
	{
		source_type_none,
		source_type_file,
		source_type_c_string,
	}; // SourceType


	SourceType source_type_;
	String glsl_dir_;
	String base_name_;
	const char* v_shader_c_string_;
	const char* f_shader_c_string_;

	GLuint ogl_vertex_shader_;
	GLuint ogl_fragment_shader_;


	virtual OglProgram* create_new(
		const String& glsl_dir,
		const String& base_name) = 0;

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

	OglProgram(const OglProgram&);
	OglProgram& operator=(const OglProgram&);

	ReloadShaderResult reload_shader(
		const GLenum shader_type,
		const char* shader_c_string,
		GLuint& shaderObject);

	ReloadShaderResult reload_shader(
		const GLenum shader_type,
		const String& file_name,
		GLuint& shaderObject);

	static String get_compile_log(
		const GLuint shader_object);

	String get_link_log();
}; // class OglProgram


} // rtcw


#endif // !RTCW_OGL_PROGRAM_INCLUDED
