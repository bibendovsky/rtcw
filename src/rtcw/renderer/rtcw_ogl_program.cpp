/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_ogl_program.h"
#include <assert.h>
#include "tr_local.h"
#include "rtcw_unique_ptr.h"

namespace rtcw {

OglProgram::OglProgram(const String& glsl_dir, const String& base_name)
	:
	program_(),
	source_type_(source_type_file),
	glsl_dir_(glsl_dir),
	base_name_(base_name),
	vertex_shader_source_(),
	fragment_shader_source_(),
	attribute_names_()
{}

OglProgram::OglProgram(const char* vertex_shader_source, const char* fragment_shader_source)
	:
	program_(),
	source_type_(source_type_c_string),
	glsl_dir_(),
	base_name_(),
	vertex_shader_source_(vertex_shader_source),
	fragment_shader_source_(fragment_shader_source),
	attribute_names_()
{}

OglProgram::~OglProgram()
{
	unload_internal();
}

bool OglProgram::reload()
{
	return do_reload();
}

void OglProgram::unload()
{
	do_unload();
}

bool OglProgram::try_reload()
{
	return do_try_reload();
}

void OglProgram::unload_internal()
{
	if (glIsProgram(program_))
	{
		glDeleteProgram(program_);
	}

	program_ = 0;
}

bool OglProgram::bind_attrib_locations()
{
	GLuint gl_index = 0;

	for (int i_name = 0; i_name < max_vertex_attributes; ++i_name)
	{
		const char* attribute_name = attribute_names_[i_name];

		if (attribute_name == NULL)
		{
			continue;
		}

		glBindAttribLocation(program_, gl_index, attribute_name);
		++gl_index;
	}

	return true;
}

bool OglProgram::reload_internal()
{
	unload_internal();

	GLuint gl_vertex_shader = 0;
	GLuint gl_fragment_shader = 0;

	if (source_type_ == source_type_file)
	{
		const char* const vs_suffix = "_vs.txt";
		const int vs_suffix_length = String::traits_type::length(vs_suffix);
		const char* const fs_suffix = "_fs.txt";
		const int fs_suffix_length = String::traits_type::length(fs_suffix);

		String name_buffer;
		name_buffer.reserve(glsl_dir_.length() + base_name_.empty() + std::max(vs_suffix_length, fs_suffix_length));

		name_buffer.clear();
		name_buffer.append(glsl_dir_).append(base_name_);
		ri.Printf(PRINT_ALL, "\"%s\"\n", name_buffer.c_str());

		name_buffer.clear();
		name_buffer.append(glsl_dir_).append(base_name_).append(vs_suffix);
		gl_vertex_shader = create_gl_shader_from_file(GL_VERTEX_SHADER, name_buffer);

		name_buffer.clear();
		name_buffer.append(glsl_dir_).append(base_name_).append(fs_suffix);
		gl_fragment_shader = create_gl_shader_from_file(GL_FRAGMENT_SHADER, name_buffer);
	}
	else if (source_type_ == source_type_c_string)
	{
		gl_vertex_shader = create_gl_shader_from_string(GL_VERTEX_SHADER, vertex_shader_source_);
		gl_fragment_shader = create_gl_shader_from_string(GL_FRAGMENT_SHADER, fragment_shader_source_);
	}
	else
	{
		return false;
	}

	const bool has_gl_vertex_shader = glIsShader(gl_vertex_shader);
	const bool has_gl_fragment_shader = glIsShader(gl_fragment_shader);
	GLint gl_link_status = GL_FALSE;

	if (has_gl_vertex_shader && has_gl_fragment_shader)
	{
		program_ = glCreateProgram();

		if (glIsProgram(program_))
		{
			glAttachShader(program_, gl_vertex_shader);
			glAttachShader(program_, gl_fragment_shader);

			if (bind_attrib_locations())
			{
				glLinkProgram(program_);
				glGetProgramiv(program_, GL_LINK_STATUS, &gl_link_status);
				glDetachShader(program_, gl_vertex_shader);
				glDetachShader(program_, gl_fragment_shader);
				const String link_log = get_gl_program_info_log(program_);

				if (!link_log.empty())
				{
					ri.Printf(PRINT_ALL, "Program info log:\n%s\n", link_log.c_str());
				}

				if (!gl_link_status)
				{
					ri.Printf(PRINT_ALL, "Failed to link a program.\n");
				}
			}
		}
		else
		{
			ri.Printf(PRINT_ALL, "Failed to create a program.\n");
		}
	}

	if (has_gl_vertex_shader)
	{
		glDeleteShader(gl_vertex_shader);
	}

	if (has_gl_fragment_shader)
	{
		glDeleteShader(gl_fragment_shader);
	}

	if (!gl_link_status)
	{
		unload_internal();
		return false;
	}

	return true;
}

bool OglProgram::do_reload()
{
	return reload_internal();
}

void OglProgram::do_unload()
{
	OglProgram::unload_internal();
}

bool OglProgram::do_try_reload()
{
	if (source_type_ == source_type_file)
	{
		rtcw::UniquePtr<OglProgram, UniquePtrDefaultDeleter<OglProgram> > instance(
			create_new(glsl_dir_, base_name_));
		return instance->reload();
	}
	else if (source_type_ == source_type_c_string)
	{
		rtcw::UniquePtr<OglProgram, UniquePtrDefaultDeleter<OglProgram> > instance(
			create_new(vertex_shader_source_, fragment_shader_source_));
		return instance->reload();
	}
	else
	{
		return false;
	}
}

GLuint OglProgram::create_gl_shader(GLenum gl_shader_type, const char* shader_string, int shader_string_length)
{
	const GLuint gl_shader_object = glCreateShader(gl_shader_type);

	if (!glIsShader(gl_shader_object))
	{
		ri.Printf(PRINT_ALL, "Failed to create a shader.\n");
		return 0;
	}

	const GLchar* const gl_lines[1] = {reinterpret_cast<const GLchar*>(shader_string)};
	const GLint gl_lengths[1] = {static_cast<GLint>(shader_string_length)};
	glShaderSource(gl_shader_object, 1, gl_lines, gl_lengths);
	glCompileShader(gl_shader_object);
	GLint gl_compile_status = GL_FALSE;
	glGetShaderiv(gl_shader_object, GL_COMPILE_STATUS, &gl_compile_status);
	const String info_log = get_gl_shader_info_log(gl_shader_object);

	if (!info_log.empty())
	{
		ri.Printf(PRINT_ALL, "Shader info log:\n%s\n", info_log.c_str());
	}

	if (!gl_compile_status)
	{
		ri.Printf(PRINT_ALL, "Failed to compile a shader.\n");
		glDeleteShader(gl_shader_object);
		return 0;
	}

	return gl_shader_object;
}

GLuint OglProgram::create_gl_shader_from_string(GLenum gl_shader_type, const char* shader_string)
{
	const int source_string_length = String::traits_type::length(shader_string);

	if (source_string_length == 0)
	{
		ri.Printf(PRINT_ALL, "Empty shader string.\n");
		return 0;
	}

	return create_gl_shader(gl_shader_type, shader_string, source_string_length);
}

GLuint OglProgram::create_gl_shader_from_file(GLenum gl_shader_type, const String& file_name)
{
	void* shader_string_buffer = NULL;
	const int source_string_length = ri.FS_ReadFile(file_name.c_str(), &shader_string_buffer);

	if (shader_string_buffer == NULL || source_string_length < 0)
	{
		ri.Printf(PRINT_ALL, "Failed to read a shader from \"%s\".\n", file_name.c_str());
		return 0;
	}

	if (source_string_length == 0)
	{
		ri.Printf(PRINT_ALL, "Empty shader file \"%s\".\n", file_name.c_str());
		return 0;
	}

	const char* const shader_string = static_cast<const char*>(shader_string_buffer);
	const GLuint gl_shader = create_gl_shader(gl_shader_type, shader_string, source_string_length);

	if (shader_string_buffer != NULL)
	{
		ri.FS_FreeFile(shader_string_buffer);
	}

	return gl_shader;
}

String OglProgram::get_gl_info_log(ObjectType object_type, GLuint gl_object)
{
	typedef void (GL_APIENTRY * glGetXivFunc)(GLuint object, GLenum pname, GLint* params);
	glGetXivFunc gl_get_x_iv;

	typedef void (GL_APIENTRY * glGetXInfoLogFunc)(GLuint object, GLsizei buf_size, GLsizei* length, GLchar* info_log);
	glGetXInfoLogFunc gl_get_x_info_log;

	switch (object_type)
	{
		case object_type_shader:
			gl_get_x_iv = glGetShaderiv;
			gl_get_x_info_log = glGetShaderInfoLog;
			break;

		case object_type_program:
			gl_get_x_iv = glGetProgramiv;
			gl_get_x_info_log = glGetProgramInfoLog;
			break;

		default:
			assert(false && "Unknown object type.");
			return String();
	}

	GLint gl_info_log_length_with_nul = 0;
	gl_get_x_iv(gl_object, GL_INFO_LOG_LENGTH, &gl_info_log_length_with_nul);

	if (gl_info_log_length_with_nul == 0)
	{
		return String();
	}

	String info_log;
	info_log.resize(static_cast<int>(gl_info_log_length_with_nul));
	GLsizei gl_info_log_length_without_nul = 0;
	gl_get_x_info_log(gl_object, gl_info_log_length_with_nul, &gl_info_log_length_without_nul, info_log.data());

	if (gl_info_log_length_without_nul == 0)
	{
		return String();
	}

	info_log.resize(static_cast<int>(gl_info_log_length_without_nul));
	return info_log;
}

String OglProgram::get_gl_shader_info_log(GLuint gl_object)
{
	return get_gl_info_log(object_type_shader, gl_object);
}

String OglProgram::get_gl_program_info_log(GLuint gl_object)
{
	return get_gl_info_log(object_type_program, gl_object);
}

} // namespace rtcw
