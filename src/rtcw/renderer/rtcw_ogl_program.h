/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// Base class for GLSL programs

#ifndef RTCW_OGL_PROGRAM_INCLUDED
#define RTCW_OGL_PROGRAM_INCLUDED

#include "qgl.h"
#include "rtcw_string.h"

namespace rtcw {

class OglProgram
{
public:
	static const int max_vertex_attributes = 4;

	// GL program object.
	GLuint program_;

	OglProgram(const String& glsl_dir, const String& base_name);
	OglProgram(const char* vertex_shader_source, const char* fragment_shader_source);
	~OglProgram();

	virtual void destroy();
	virtual bool reload();
	virtual bool try_reload();
	virtual void unload();

protected:
	enum SourceType
	{
		source_type_none,
		source_type_file,
		source_type_c_string,
	};

	struct AttributeInfo
	{
		const char* name;
		GLenum gl_type;
	};

	struct Deleter
	{
		void operator()(OglProgram* ogl_program) const;
	};

	SourceType source_type_;
	String glsl_dir_;
	String base_name_;
	const char* vertex_shader_source_;
	const char* fragment_shader_source_;
	const char* const* attribute_names_;

	virtual OglProgram* create_new(const String& glsl_dir, const String& base_name);
	virtual OglProgram* create_new(const char* vertex_shader_source, const char* fragment_shader_source);

	bool bind_attrib_locations();
	bool reload_internal();
	void unload_internal();

private:
	OglProgram(const OglProgram&);
	OglProgram& operator=(const OglProgram&);

	enum ObjectType
	{
		object_type_none = 0,
		object_type_shader,
		object_type_program
	};

	static GLuint create_gl_shader(GLenum gl_shader_type, const char* shader_string, int shader_string_length);
	static GLuint create_gl_shader_from_string(GLenum gl_shader_type, const char* shader_string);
	static GLuint create_gl_shader_from_file(GLenum gl_shader_type, const String& file_name);

	static String get_gl_info_log(ObjectType object_type, GLuint gl_object);
	static String get_gl_shader_info_log(GLuint gl_object);
	static String get_gl_program_info_log(GLuint gl_object);
};

} // namespace rtcw

#endif // RTCW_OGL_PROGRAM_INCLUDED
