#include "rtcw_ogl_program.h"
#include <memory>
#include "tr_local.h"


namespace rtcw
{


OglProgram::OglProgram(
	const std::string& glsl_dir,
	const std::string& base_name)
	:
	source_type_{SourceType::file},
	glsl_dir_{glsl_dir},
	base_name_{base_name}
{
}

OglProgram::OglProgram(
	const char* v_shader_buffer,
	const char* f_shader_buffer)
	:
	source_type_{SourceType::c_string},
	v_shader_c_string_{v_shader_buffer},
	f_shader_c_string_{f_shader_buffer}
{
}

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
	if (ogl_vertex_shader_ != 0)
	{
		glDeleteShader(ogl_vertex_shader_);
		ogl_vertex_shader_ = 0;
	}

	if (ogl_fragment_shader_ != 0)
	{
		glDeleteShader(ogl_fragment_shader_);
		ogl_fragment_shader_ = 0;
	}

	if (program_ != 0)
	{
		glDeleteProgram(program_);
		program_ = 0;
	}
}

bool OglProgram::reload_internal()
{
	unload_internal();


	bool result = false;
	bool is_compile_program = false;

	if (source_type_ == SourceType::file)
	{
		const std::string p_name = glsl_dir_ + base_name_;

		ri.Printf(PRINT_ALL, "\"%s\"\n", p_name.c_str());

		const std::string v_name = p_name + "_vs.txt";
		const std::string f_name = p_name + "_fs.txt";

		const ReloadShaderResult v_result = reload_shader(GL_VERTEX_SHADER, v_name, ogl_vertex_shader_);

		if (v_result == ReloadShaderResult::compiled)
		{
			const ReloadShaderResult f_result = reload_shader(GL_FRAGMENT_SHADER, f_name, ogl_fragment_shader_);

			is_compile_program = (f_result == ReloadShaderResult::compiled);
		}
	}
	else if (source_type_ == SourceType::c_string)
	{
		const ReloadShaderResult v_result = reload_shader(GL_VERTEX_SHADER, v_shader_c_string_, ogl_vertex_shader_);

		if (v_result == ReloadShaderResult::compiled)
		{
			const ReloadShaderResult f_result = reload_shader(GL_FRAGMENT_SHADER, f_shader_c_string_, ogl_fragment_shader_);

			is_compile_program = (f_result == ReloadShaderResult::compiled);
		}
	}
	else
	{
		return false;
	}

	if (is_compile_program)
	{
		program_ = glCreateProgram();

		glAttachShader(program_, ogl_vertex_shader_);
		glAttachShader(program_, ogl_fragment_shader_);

		GLint link_status = GL_FALSE;

		glLinkProgram(program_);
		glGetProgramiv(program_, GL_LINK_STATUS, &link_status);

		std::string link_log = get_link_log();

		if (link_status != GL_FALSE)
		{
			if (!link_log.empty())
			{
				ri.Printf(PRINT_ALL, "Linkage log:\n%s\n", link_log.c_str());
			}

			result = true;
		}
		else
		{
			ri.Printf(PRINT_ALL, "Failed to link:\n%s\n", link_log.c_str());
		}
	}

	if (!result)
	{
		unload_internal();
	}

	return result;
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
	if (source_type_ == SourceType::file)
	{
		std::unique_ptr<OglProgram> instance(create_new(glsl_dir_, base_name_));

		return instance->reload();
	}
	else if (source_type_ == SourceType::c_string)
	{
		std::unique_ptr<OglProgram> instance(create_new(v_shader_c_string_, f_shader_c_string_));

		return instance->reload();
	}
	else
	{
		return false;
	}
}

OglProgram::ReloadShaderResult OglProgram::reload_shader(
	const GLenum shader_type,
	const char* shader_c_string,
	GLuint& shader_object)
{
	ReloadShaderResult result = ReloadShaderResult::none;

	const GLint source_length = static_cast<GLint>(std::string::traits_type::length(shader_c_string));

	shader_object = 0;

	bool is_compiled = false;

	if (source_length > 0)
	{
		const GLchar* lines[1] = {shader_c_string};
		GLint lengths[1] = {source_length};

		shader_object = glCreateShader(shader_type);
		glShaderSource(shader_object, 1, lines, lengths);
		glCompileShader(shader_object);

		GLint compile_status = GL_FALSE;

		glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compile_status);

		const std::string compile_log = get_compile_log(shader_object);

		if (compile_status != GL_FALSE)
		{
			if (!compile_log.empty())
			{
				ri.Printf(PRINT_ALL, "Compilation log of \"%s\" shader:\n%s\n",
					get_shader_type_string(shader_type), compile_log.c_str());
			}

			result = ReloadShaderResult::compiled;
		}
		else
		{
			ri.Printf(PRINT_ALL, "Failed to compile a \"%s\" shader:\n%s\n",
				get_shader_type_string(shader_type), compile_log.c_str());

			result = ReloadShaderResult::not_compiled;
		}
	}

	if (result != ReloadShaderResult::compiled)
	{
		glDeleteShader(shader_object);
		shader_object = GL_NONE;
	}

	return result;
}

OglProgram::ReloadShaderResult OglProgram::reload_shader(
	const GLenum shader_type,
	const std::string& file_name,
	GLuint& shader_object)
{
	ReloadShaderResult result = ReloadShaderResult::none;

	void* source_buffer = NULL;

	const int source_length = ri.FS_ReadFile(file_name.c_str(), &source_buffer);

	shader_object = GL_NONE;

	bool is_compiled = false;

	if (source_length > 0)
	{
		const GLchar* lines[1] = {static_cast<const GLchar*>(source_buffer)};
		GLint lengths[1] = {source_length};

		shader_object = glCreateShader(shader_type);
		glShaderSource(shader_object, 1, lines, lengths);
		glCompileShader(shader_object);

		GLint compile_status = GL_FALSE;

		glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compile_status);

		const std::string compile_log = get_compile_log(shader_object);

		if (compile_status != GL_FALSE)
		{
			if (!compile_log.empty())
			{
				ri.Printf(PRINT_ALL, "Compilation log of \"%s\":\n%s\n",
					file_name.c_str(), compile_log.c_str());
			}

			result = ReloadShaderResult::compiled;
		}
		else
		{
			ri.Printf(PRINT_ALL, "Failed to compile a shader \"%s\":\n%s\n",
				file_name.c_str(), compile_log.c_str());

			result = ReloadShaderResult::not_compiled;
		}
	}

	if (source_buffer)
	{
		ri.FS_FreeFile(source_buffer);
	}

	if (result != ReloadShaderResult::compiled)
	{
		glDeleteShader(shader_object);
		shader_object = GL_NONE;
	}

	return result;
}

std::string OglProgram::get_compile_log(
	const GLuint shader_object)
{
	GLint info_log_size = 0; // with a null terminator

	glGetShaderiv(shader_object, GL_INFO_LOG_LENGTH, &info_log_size);

	if (info_log_size <= 0)
	{
		return std::string{};
	}

	GLsizei info_log_length = 0; // without a null terminator

	std::string info_log;
	info_log.resize(info_log_size);

	glGetShaderInfoLog(shader_object, info_log_size, &info_log_length, &info_log[0]);

	if (info_log_length == 0)
	{
		return std::string();
	}

	info_log.resize(info_log_length);

	return info_log;
}

std::string OglProgram::get_link_log()
{
	GLint info_log_size = 0; // with a null terminator

	glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &info_log_size);

	if (info_log_size == 0)
	{
		return std::string();
	}


	GLsizei info_log_length = 0; // without a null terminator

	std::string info_log;
	info_log.resize(info_log_size);

	glGetProgramInfoLog(program_, info_log_size, &info_log_length, &info_log[0]);

	if (info_log_length == 0)
	{
		return std::string();
	}

	info_log.resize(info_log_length);

	return info_log;
}

const char* OglProgram::get_shader_type_string(
	const GLenum shader_type)
{
	switch (shader_type)
	{
		case GL_VERTEX_SHADER:
			return "vertex";

		case GL_FRAGMENT_SHADER:
			return "fragment";

		default:
			return "???";
	}
}


} // rtcw
