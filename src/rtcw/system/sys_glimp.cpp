/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include <ctype.h>
#include <string.h>
#include <algorithm>
#include <memory>

#include "SDL_version.h"
#include "SDL_video.h"

#include "tr_local.h"
#include "rtcw_window_rounded_corner_mgr.h"

#if SDL_VERSION_ATLEAST(2, 24, 0)
#	define RTCW_SDL_AT_LEAST_2_24_0 1
#else
#	define RTCW_SDL_AT_LEAST_2_24_0 0
#endif

SDL_Window* sys_gl_window;
Uint32 sys_main_window_id = 0;

// don't abort out if the pixelformat claims software
cvar_t* r_allowSoftwareGL;

// allow a different dll name to be treated as if it were opengl32.dll
cvar_t* r_maskMinidriver;

#ifdef RTCW_ET
int gl_NormalFontBase = 0;
#endif // RTCW_XX


namespace {


const int FALLBACK_WIDTH = 640;
const int FALLBACK_HEIGHT = 480;

enum ExtensionStatus {
	EXT_STATUS_USING,
	EXT_STATUS_NOT_FOUND,
	EXT_STATUS_IGNORING,
}; // enum ExtensionStatus

// ======================================

class GlVersion
{
public:
	GlVersion();
	GlVersion(int version_major, int version_minor);
	GlVersion(const GlVersion& that);
	GlVersion& operator=(const GlVersion& that);

	bool operator>=(const GlVersion& that) const;

private:
	unsigned char value_;
};

// --------------------------------------

GlVersion::GlVersion()
	:
	value_()
{}

GlVersion::GlVersion(int version_major, int version_minor)
	:
	value_(static_cast<unsigned char>(version_major * 10 + version_minor))
{}

GlVersion::GlVersion(const GlVersion& that)
{
	value_ = that.value_;
}

GlVersion& GlVersion::operator=(const GlVersion& that)
{
	value_ = that.value_;
	return *this;
}

bool GlVersion::operator>=(const GlVersion& that) const
{
	return value_ >= that.value_;
}

// ======================================

struct GlFunctionInfo
{
	const char* name;
	void** function_ptr;
};

// ======================================

SDL_GLContext gl_context;
GlVersion glimp_gl_version;

// ======================================

template<typename T>
void maybe_unused(const T&)
{}

// ======================================

template<typename TDst, typename TSrc>
TDst glimp_bit_cast(const TSrc& src)
{
	const size_t size = sizeof(TSrc);
	struct Validator
	{
		bool validate_size[2 * (size == sizeof(TDst)) - 1];
	};
	maybe_unused(Validator().validate_size);
	TDst dst;
	memcpy(&dst, &src, size);
	return dst;
}

template<typename TDst, typename TSrc>
void glimp_bit_cast(const TSrc& src, TDst& dst)
{
	const size_t size = sizeof(TSrc);
	struct Validator
	{
		bool validate_size[2 * (size == sizeof(TDst)) - 1];
	};
	maybe_unused(Validator().validate_size);
	memcpy(&dst, &src, size);
}

template<typename TProc>
bool glimp_gl_get_proc_address(const char* proc_name, TProc& proc)
{
	glimp_bit_cast(SDL_GL_GetProcAddress(proc_name), proc);
	return proc != NULL;
}

#define RTCW_GLIMP_GL_GET_PROC_ADDRESS(symbol) glimp_gl_get_proc_address(#symbol, symbol)

// ======================================

GlVersion glimp_get_gl_version()
{
	SDL_GL_ResetAttributes();

	SDL_Window* const sdl_window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		FALLBACK_WIDTH,
		FALLBACK_HEIGHT,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);

	if (sdl_window != NULL)
	{
		SDL_GLContext const sdl_gl_context = SDL_GL_CreateContext(sdl_window);

		if (sdl_gl_context != NULL)
		{
			PFNGLGETSTRINGPROC impl_glGetString = NULL;
			glimp_gl_get_proc_address("glGetString", impl_glGetString);

			if (impl_glGetString != NULL)
			{
				const char* const gl_version_string = reinterpret_cast<const char*>(impl_glGetString(GL_VERSION));

				if (gl_version_string != NULL &&
					isdigit(gl_version_string[0]) &&
					gl_version_string[1] == '.' &&
					isdigit(gl_version_string[2]) &&
					(gl_version_string[3] == ' ' || gl_version_string[3] == '.'))
				{
					const int version_major = gl_version_string[0] - '0';
					const int version_minor = gl_version_string[2] - '0';
					return GlVersion(version_major, version_minor);
				}
			}

			SDL_GL_DeleteContext(sdl_gl_context);
		}

		SDL_DestroyWindow(sdl_window);
	}

	return GlVersion();
}

// ======================================

bool glimp_load_gl_functions(const char* message_color, GlFunctionInfo* gl_function_infos)
{
	const bool is_white_color = strcmp(message_color, S_COLOR_WHITE) == 0;
	bool is_failed = false;

	for (int i_gl_function_info = 0; ; ++i_gl_function_info)
	{
		GlFunctionInfo& gl_function_info = gl_function_infos[i_gl_function_info];

		if (gl_function_info.name == NULL)
		{
			break;
		}

		if (gl_function_info.function_ptr == NULL)
		{
			continue;
		}

		void* const function = SDL_GL_GetProcAddress(gl_function_info.name);

		if (function == NULL)
		{
			is_failed = true;

			if (is_white_color)
			{
				ri.Printf(PRINT_ALL, "  missing %s\n", gl_function_info.name);
			}
			else
			{
				ri.Printf(PRINT_ALL, "  %smissing %s%s\n", message_color, gl_function_info.name, S_COLOR_WHITE);
			}
		}

		*gl_function_info.function_ptr = function;
	}

	if (is_failed)
	{
		for (int i_gl_function_info = 0; ; ++i_gl_function_info)
		{
			GlFunctionInfo& gl_function_info = gl_function_infos[i_gl_function_info];

			if (gl_function_info.name == NULL || gl_function_info.function_ptr == NULL)
			{
				break;
			}

			*gl_function_info.function_ptr = NULL;
		}

		return false;
	}

	return true;
}

// ======================================

void glimp_initialize_gl1_essential_functions()
{
	GlFunctionInfo gl_function_infos[] =
	{
#define RTCW_MACRO(symbol) {#symbol, glimp_bit_cast<void**>(&symbol)}

		// OpenGL v1.0
		//
		RTCW_MACRO(glAlphaFunc),
		RTCW_MACRO(glBegin),
		RTCW_MACRO(glBlendFunc),
		RTCW_MACRO(glCallList),
		RTCW_MACRO(glCallLists),
		RTCW_MACRO(glClear),
		RTCW_MACRO(glClearColor),
		RTCW_MACRO(glClearDepth),
		RTCW_MACRO(glClearStencil),
		RTCW_MACRO(glClipPlane),
		RTCW_MACRO(glColor3f),
		RTCW_MACRO(glColor3fv),
		RTCW_MACRO(glColor4f),
		RTCW_MACRO(glColor4fv),
		RTCW_MACRO(glColor4ubv),
		RTCW_MACRO(glColorMask),
		RTCW_MACRO(glCullFace),
		RTCW_MACRO(glDepthFunc),
		RTCW_MACRO(glDepthMask),
		RTCW_MACRO(glDepthRange),
		RTCW_MACRO(glDisable),
		RTCW_MACRO(glDrawBuffer),
		RTCW_MACRO(glEnable),
		RTCW_MACRO(glEnd),
		RTCW_MACRO(glFinish),
		RTCW_MACRO(glFogf),
		RTCW_MACRO(glFogfv),
		RTCW_MACRO(glFogi),
		RTCW_MACRO(glGetBooleanv),
		RTCW_MACRO(glGetError),
		RTCW_MACRO(glGetFloatv),
		RTCW_MACRO(glGetIntegerv),
		RTCW_MACRO(glGetString),
		RTCW_MACRO(glGetTexLevelParameteriv),
		RTCW_MACRO(glHint),
		RTCW_MACRO(glLineWidth),
		RTCW_MACRO(glListBase),
		RTCW_MACRO(glLoadIdentity),
		RTCW_MACRO(glLoadMatrixf),
		RTCW_MACRO(glMatrixMode),
		RTCW_MACRO(glOrtho),
		RTCW_MACRO(glPointSize),
		RTCW_MACRO(glPolygonMode),
		RTCW_MACRO(glPopAttrib),
		RTCW_MACRO(glPopMatrix),
		RTCW_MACRO(glPushAttrib),
		RTCW_MACRO(glPushMatrix),
		RTCW_MACRO(glRasterPos3fv),
		RTCW_MACRO(glReadPixels),
		RTCW_MACRO(glScissor),
		RTCW_MACRO(glShadeModel),
		RTCW_MACRO(glStencilFunc),
		RTCW_MACRO(glStencilMask),
		RTCW_MACRO(glStencilOp),
		RTCW_MACRO(glTexCoord2f),
		RTCW_MACRO(glTexCoord2fv),
		RTCW_MACRO(glTexEnvf),
		RTCW_MACRO(glTexImage2D),
		RTCW_MACRO(glTexParameterf),
		RTCW_MACRO(glTexParameterfv),
		RTCW_MACRO(glTexParameteri),
		RTCW_MACRO(glTranslatef),
		RTCW_MACRO(glVertex2f),
		RTCW_MACRO(glVertex3f),
		RTCW_MACRO(glVertex3fv),
		RTCW_MACRO(glViewport),

		// OpenGL v1.1
		//
		RTCW_MACRO(glArrayElement),
		RTCW_MACRO(glBindTexture),
		RTCW_MACRO(glColorPointer),
		RTCW_MACRO(glCopyTexImage2D),
		RTCW_MACRO(glDeleteTextures),
		RTCW_MACRO(glDisableClientState),
		RTCW_MACRO(glDrawArrays),
		RTCW_MACRO(glDrawElements),
		RTCW_MACRO(glEnableClientState),
		RTCW_MACRO(glGenTextures),
		RTCW_MACRO(glIsTexture),
		RTCW_MACRO(glNormalPointer),
		RTCW_MACRO(glPolygonOffset),
		RTCW_MACRO(glTexCoordPointer),
		RTCW_MACRO(glTexSubImage2D),
		RTCW_MACRO(glVertexPointer),

#undef RTCW_MACRO

		// End-of-entries.
		//
		{NULL, NULL}
	};

	if (!glimp_load_gl_functions(S_COLOR_RED, gl_function_infos))
	{
		ri.Error(ERR_FATAL, "Failed to initialize OpenGL 1.X essential functions\n");
	}
}

// ======================================

bool glimp_initialize_gl2_functions()
{
	const bool is_gl2 = glimp_gl_version >= GlVersion(2, 0);

	if (!is_gl2)
	{
		return false;
	}

	GlFunctionInfo gl_function_infos[] =
	{
#define RTCW_MACRO(symbol) {#symbol, glimp_bit_cast<void**>(&symbol)}

		RTCW_MACRO(glActiveTexture),
		RTCW_MACRO(glBindAttribLocation),
		RTCW_MACRO(glBindBuffer),
		RTCW_MACRO(glBufferData),
		RTCW_MACRO(glBufferSubData),
		RTCW_MACRO(glDeleteBuffers),
		RTCW_MACRO(glGenBuffers),
		RTCW_MACRO(glAttachShader),
		RTCW_MACRO(glCompileShader),
		RTCW_MACRO(glCreateProgram),
		RTCW_MACRO(glCreateShader),
		RTCW_MACRO(glDeleteProgram),
		RTCW_MACRO(glDeleteShader),
		RTCW_MACRO(glDetachShader),
		RTCW_MACRO(glDisableVertexAttribArray),
		RTCW_MACRO(glEnableVertexAttribArray),
		RTCW_MACRO(glGetAttribLocation),
		RTCW_MACRO(glGetProgramInfoLog),
		RTCW_MACRO(glGetProgramiv),
		RTCW_MACRO(glGetShaderInfoLog),
		RTCW_MACRO(glGetShaderiv),
		RTCW_MACRO(glGetUniformLocation),
		RTCW_MACRO(glIsBuffer),
		RTCW_MACRO(glIsEnabled),
		RTCW_MACRO(glIsProgram),
		RTCW_MACRO(glIsShader),
		RTCW_MACRO(glLinkProgram),
		RTCW_MACRO(glShaderSource),
		RTCW_MACRO(glUniform1f),
		RTCW_MACRO(glUniform1i),
		RTCW_MACRO(glUniform4fv),
		RTCW_MACRO(glUniformMatrix4fv),
		RTCW_MACRO(glUseProgram),
		RTCW_MACRO(glVertexAttrib2f),
		RTCW_MACRO(glVertexAttrib4f),
		RTCW_MACRO(glVertexAttribPointer),

#undef RTCW_MACRO

		// End-of-entries.
		//
		{NULL, NULL}
	};

	return glimp_load_gl_functions(S_COLOR_WHITE, gl_function_infos);
}

// ======================================

void glimp_print_extension(
	ExtensionStatus status,
	const char* extension_name)
{
	if (extension_name == NULL)
		return;

	if (extension_name[0] == '\0')
		return;


	const char* status_mark = NULL;
	const char* color_mark = NULL;

	switch (status) {
	case EXT_STATUS_USING:
		status_mark = "+";
		color_mark = S_COLOR_GREEN;
		break;

	case EXT_STATUS_NOT_FOUND:
		status_mark = "-";
		color_mark = S_COLOR_RED;
		break;

	case EXT_STATUS_IGNORING:
		status_mark = "*";
		color_mark = S_COLOR_YELLOW;
		break;

	default:
		status_mark = "?";
		color_mark = S_COLOR_WHITE;
		break;
	}

	ri.Printf(
		PRINT_ALL,
		"  [%s%s%s] %s\n",
		color_mark,
		status_mark,
		S_COLOR_WHITE,
		extension_name);
}

void glimp_print_using_extension(const char* extension_name)
{
	glimp_print_extension(EXT_STATUS_USING, extension_name);
}

void glimp_print_not_found_extension(const char* extension_name)
{
	glimp_print_extension(EXT_STATUS_NOT_FOUND, extension_name);
}

#if 0 // FIXME Remove or use.
void glimp_print_ignoring_extension(const char* extension_name)
{
	glimp_print_extension(EXT_STATUS_IGNORING, extension_name);
}
#endif

// ======================================

void glimp_initialize_gl_arb_texture_compression_extension()
{
	const char validate_GL_COMPRESSED_RGB[2 * (GL_COMPRESSED_RGB == GL_COMPRESSED_RGB_ARB) - 1] = {0};
	maybe_unused(validate_GL_COMPRESSED_RGB);
	const char validate_GL_COMPRESSED_RGBA[2 * (GL_COMPRESSED_RGBA == GL_COMPRESSED_RGBA_ARB) - 1] = {0};
	maybe_unused(validate_GL_COMPRESSED_RGBA);

	const char* const gl_arb_texture_compression_string = "GL_ARB_texture_compression";
	const bool is_gl13 = glimp_gl_version >= GlVersion(1, 3);
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (is_gl13 || SDL_GL_ExtensionSupported(gl_arb_texture_compression_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_compressed_textures->integer != 0)
		{
			extension_status = EXT_STATUS_USING;
			glConfig.textureCompression = TC_ARB;
		}
	}

	glimp_print_extension(extension_status, gl_arb_texture_compression_string);
}

void glimp_initialize_ext_texture_compression_s3tc_extension()
{
	const char* const gl_ext_texture_compression_s3tc_string = "GL_EXT_texture_compression_s3tc";
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (SDL_GL_ExtensionSupported(gl_ext_texture_compression_s3tc_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_compressed_textures->integer != 0)
		{
			glConfig.textureCompression = TC_EXT_COMP_S3TC;
			extension_status = EXT_STATUS_USING;
		}
	}

	glimp_print_extension(extension_status, gl_ext_texture_compression_s3tc_string);
}

void glimp_initialize_gl_s3_s3tc_extension()
{
	const char* const gl_s3_s3tc_string = "GL_S3_s3tc";
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (SDL_GL_ExtensionSupported(gl_s3_s3tc_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_compressed_textures->integer != 0)
		{
			glConfig.textureCompression = TC_S3TC;
			extension_status = EXT_STATUS_USING;
		}
	}

	glimp_print_extension(extension_status, gl_s3_s3tc_string);
}

void glimp_initialize_gl_xxx_texture_compression_extensions()
{
	typedef void (* Function)();

	const Function functions[] =
	{
		glimp_initialize_gl_arb_texture_compression_extension,
		glimp_initialize_ext_texture_compression_s3tc_extension,
		glimp_initialize_gl_s3_s3tc_extension,
		NULL
	};

	glConfig.textureCompression = TC_NONE;

	for (int i_function = 0; functions[i_function] != NULL; ++i_function)
	{
		functions[i_function]();

		if (glConfig.textureCompression != TC_NONE)
		{
			break;
		}
	}
}

// ======================================

void glimp_initialize_gl_arb_texture_env_add_extension()
{
	const char* const gl_arb_texture_env_add_string = "GL_ARB_texture_env_add";
	const bool is_gl13 = glimp_gl_version >= GlVersion(1, 3);
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (is_gl13 || SDL_GL_ExtensionSupported(gl_arb_texture_env_add_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_texture_env_add->integer != 0)
		{
			glConfig.textureEnvAddAvailable = true;
			extension_status = EXT_STATUS_USING;
		}
	}

	glimp_print_extension(extension_status, gl_arb_texture_env_add_string);
}

void glimp_initialize_gl_ext_texture_env_add_extension()
{
	const char* const gl_ext_texture_env_add_string = "GL_EXT_texture_env_add";
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (SDL_GL_ExtensionSupported(gl_ext_texture_env_add_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_texture_env_add->integer != 0)
		{
			glConfig.textureEnvAddAvailable = true;
			extension_status = EXT_STATUS_USING;
		}
	}

	glimp_print_extension(extension_status, gl_ext_texture_env_add_string);
}

void glimp_initialize_gl_xxx_texture_env_add_extensions()
{
	typedef void (* Function)();

	const Function functions[] =
	{
		glimp_initialize_gl_arb_texture_env_add_extension,
		glimp_initialize_gl_ext_texture_env_add_extension,
		NULL
	};

	glConfig.textureEnvAddAvailable = false;

	for (int i_function = 0; functions[i_function] != NULL; ++i_function)
	{
		functions[i_function]();

		if (glConfig.textureEnvAddAvailable)
		{
			break;
		}
	}
}

// ======================================

void glimp_probe_swap_control()
{
	const int current_mode = SDL_GL_GetSwapInterval();

	const int adaptive_mode = SDL_GL_SetSwapInterval(-1);
	glConfigEx.has_adaptive_swap_control_ = (adaptive_mode == 0);

	const int off_mode = SDL_GL_SetSwapInterval(0);
	const int on_mode = SDL_GL_SetSwapInterval(1);
	glConfigEx.has_swap_control_ = (off_mode == 0 && on_mode == 0);

	const int new_mode = SDL_GL_SetSwapInterval(current_mode);
	maybe_unused(new_mode);
}

void glimp_initialize_xxx_ext_swap_control_extension()
{
	const char* const xxx_ext_swap_control_string = "XXX_EXT_swap_control";
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (glConfigEx.has_swap_control_)
	{
		r_swapInterval->modified = true;
		extension_status = EXT_STATUS_USING;
	}

	glimp_print_extension(extension_status, xxx_ext_swap_control_string);
}

void glimp_initialize_xxx_ext_swap_control_tear_extension()
{
	const char* const xxx_ext_swap_control_tear_string = "XXX_EXT_swap_control_tear";
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (glConfigEx.has_adaptive_swap_control_)
	{
		r_swapInterval->modified = true;
		extension_status = EXT_STATUS_USING;
	}

	glimp_print_extension(extension_status, xxx_ext_swap_control_tear_string);
}

void glimp_initialize_xxx_ext_swap_control_extensions()
{
	glimp_probe_swap_control();
	glimp_initialize_xxx_ext_swap_control_extension();
	glimp_initialize_xxx_ext_swap_control_tear_extension();
}

// ======================================

void glimp_initialize_gl_arb_multitexture_extension()
{
	const char validate_GL_MAX_TEXTURE_UNITS[2 * (GL_MAX_TEXTURE_UNITS == GL_MAX_TEXTURE_UNITS_ARB) - 1] = {0};
	maybe_unused(validate_GL_MAX_TEXTURE_UNITS);
	const bool is_gl13 = glimp_gl_version >= GlVersion(1, 3);
	const char* const gl_arb_multitexture_string = "GL_ARB_multitexture";

	glConfig.maxActiveTextures = 0;
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (is_gl13 || SDL_GL_ExtensionSupported(gl_arb_multitexture_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_multitexture->integer != 0)
		{
			GLint gl_max_texture_units = 0;
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &gl_max_texture_units);
			const bool has_enough_texture_units = gl_max_texture_units >= 2;

			if (!has_enough_texture_units)
			{
				ri.Printf(PRINT_ALL, "  %d < 2 texture units\n", gl_max_texture_units);
			}

			GlFunctionInfo gl_function_infos[] =
			{
#define RTCW_MACRO0(x) #x
#define RTCW_MACRO(symbol) {is_gl13 ? #symbol : RTCW_MACRO0(symbol##ARB), glimp_bit_cast<void**>(&symbol)}

				RTCW_MACRO(glActiveTexture),
				RTCW_MACRO(glClientActiveTexture),
				RTCW_MACRO(glMultiTexCoord2f),

#undef RTCW_MACRO0
#undef RTCW_MACRO

				{NULL, NULL}
			};

			if (has_enough_texture_units &&
				glimp_load_gl_functions(S_COLOR_WHITE, gl_function_infos))
			{
				glConfig.maxActiveTextures = gl_max_texture_units;
				glConfigEx.use_arb_multitexture_ = true;
				extension_status = EXT_STATUS_USING;
			}
		}
	}

	glimp_print_extension(extension_status, gl_arb_multitexture_string);
}

// ======================================

void glimp_initialize_gl_ext_compiled_vertex_array_extension()
{
	const char* const gl_ext_compiled_vertex_array_string = "GL_EXT_compiled_vertex_array";

	glConfigEx.use_ext_compiled_vertex_array_ = false;
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (SDL_GL_ExtensionSupported(gl_ext_compiled_vertex_array_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_compiled_vertex_array->integer != 0)
		{
			GlFunctionInfo gl_function_infos[] =
			{
#define RTCW_MACRO(symbol) {#symbol, glimp_bit_cast<void**>(&symbol)}

				RTCW_MACRO(glLockArraysEXT),
				RTCW_MACRO(glUnlockArraysEXT),

#undef RTCW_MACRO

				{NULL, NULL}
			};

			if (glimp_load_gl_functions(S_COLOR_WHITE, gl_function_infos))
			{
				glConfigEx.use_ext_compiled_vertex_array_ = true;
				extension_status = EXT_STATUS_USING;
			}
		}
	}

	glimp_print_extension(extension_status, gl_ext_compiled_vertex_array_string);
}

// ======================================

#ifdef RTCW_SP
void glimp_initialize_gl_ati_pn_triangles_extension()
{
	const char* const gl_ati_pn_triangles_string = "GL_ATI_pn_triangles";
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (SDL_GL_ExtensionSupported(gl_ati_pn_triangles_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_ATI_pntriangles->integer != 0)
		{
			GlFunctionInfo gl_function_infos[] =
			{
#define RTCW_MACRO(symbol) {#symbol, glimp_bit_cast<void**>(&symbol)}

				RTCW_MACRO(glPNTrianglesiATI),
				RTCW_MACRO(glPNTrianglesfATI),

#undef RTCW_MACRO

				{NULL, NULL}
			};

			if (glimp_load_gl_functions(S_COLOR_WHITE, gl_function_infos))
			{
				extension_status = EXT_STATUS_USING;
			}
		}
	}

	glimp_print_extension(extension_status, gl_ati_pn_triangles_string);
}
#endif // RTCW_SP

// ======================================

void glimp_initialize_gl_arb_texture_filter_anisotropic_extension()
{
	const char* const gl_arb_texture_filter_anisotropic_string = "GL_ARB_texture_filter_anisotropic";
	const bool is_gl46 = glimp_gl_version >= GlVersion(4, 6);
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (is_gl46 || SDL_GL_ExtensionSupported(gl_arb_texture_filter_anisotropic_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_texture_filter_anisotropic->integer != 0)
		{
			glConfig.anisotropicAvailable = true;
			extension_status = EXT_STATUS_USING;
		}
	}

	glimp_print_extension(extension_status, gl_arb_texture_filter_anisotropic_string);
}

void glimp_initialize_gl_ext_texture_filter_anisotropic_extension()
{
	const char* const gl_ext_texture_filter_anisotropic_string = "GL_EXT_texture_filter_anisotropic";
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (SDL_GL_ExtensionSupported(gl_ext_texture_filter_anisotropic_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_texture_filter_anisotropic->integer != 0)
		{
			glConfig.anisotropicAvailable = true;
			extension_status = EXT_STATUS_USING;
		}
	}

	glimp_print_extension(extension_status, gl_ext_texture_filter_anisotropic_string);
}

void glimp_initialize_gl_xxx_texture_filter_anisotropic_extensions()
{
	const char validate_GL_TEXTURE_MAX_ANISOTROPY[2 * (GL_TEXTURE_MAX_ANISOTROPY == GL_TEXTURE_MAX_ANISOTROPY_EXT) - 1] = {0};
	maybe_unused(validate_GL_TEXTURE_MAX_ANISOTROPY);
	const bool validate_GL_MAX_TEXTURE_MAX_ANISOTROPY[2 * (GL_MAX_TEXTURE_MAX_ANISOTROPY == GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT) - 1] = {0};
	maybe_unused(validate_GL_MAX_TEXTURE_MAX_ANISOTROPY);

	typedef void (* Function)();

	const Function functions[] =
	{
		glimp_initialize_gl_arb_texture_filter_anisotropic_extension,
		glimp_initialize_gl_ext_texture_filter_anisotropic_extension,
		NULL
	};

	glConfig.anisotropicAvailable = false;

	for (int i_function = 0; functions[i_function] != NULL; ++i_function)
	{
		functions[i_function]();

		if (glConfig.anisotropicAvailable)
		{
			break;
		}
	}
}

// ======================================

void glimp_initialize_gl_nv_fog_distance_extension()
{
	const char* const gl_nv_fog_distance_string = "GL_NV_fog_distance";

	glConfig.NVFogAvailable = false;
	glConfig.NVFogMode = 0;
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	if (SDL_GL_ExtensionSupported(gl_nv_fog_distance_string))
	{
		extension_status = EXT_STATUS_IGNORING;

		if (r_ext_NV_fog_dist->integer != 0)
		{
			glConfig.NVFogAvailable = true;
			extension_status = EXT_STATUS_USING;
		}
	}

	if (extension_status != EXT_STATUS_USING)
	{
		ri.Cvar_Set("r_ext_NV_fog_dist", "0");
	}

	glimp_print_extension(extension_status, gl_nv_fog_distance_string);
}

// ======================================

void glimp_initialize_gl_arb_draw_elements_base_vertex_extension()
{
	const bool is_gl32 = glimp_gl_version >= GlVersion(3, 2);
	const char* const gl_arb_draw_elements_base_vertex_string = "GL_ARB_draw_elements_base_vertex";

	if (is_gl32 || SDL_GL_ExtensionSupported(gl_arb_draw_elements_base_vertex_string))
	{
		GlFunctionInfo gl_function_infos[] =
		{
#define RTCW_MACRO0(x) #x
#define RTCW_MACRO(symbol) {is_gl32 ? #symbol : RTCW_MACRO0(symbol##ARB), glimp_bit_cast<void**>(&symbol)}

			RTCW_MACRO(glDrawElementsBaseVertex),

#undef RTCW_MACRO0
#undef RTCW_MACRO

			{NULL, NULL}
		};

		if (glimp_load_gl_functions(S_COLOR_WHITE, gl_function_infos))
		{
			glConfigEx.use_arb_draw_elements_base_vertex = true;
			glimp_print_using_extension(gl_arb_draw_elements_base_vertex_string);
		}
		else
		{
			glimp_print_not_found_extension(gl_arb_draw_elements_base_vertex_string);
		}
	}
	else
	{
		glimp_print_not_found_extension(gl_arb_draw_elements_base_vertex_string);
	}
}

// ======================================

void glimp_initialize_gl_arb_framebuffer_object_extension()
{
	const char* const gl_arb_framebuffer_object_string = "GL_ARB_framebuffer_object";
	const bool is_gl30 = glimp_gl_version >= GlVersion(3, 0);

	if (is_gl30 || SDL_GL_ExtensionSupported(gl_arb_framebuffer_object_string))
	{
		GlFunctionInfo gl_function_infos[] =
		{
#define RTCW_MACRO0(x) #x
#define RTCW_MACRO(symbol) {is_gl30 ? #symbol : RTCW_MACRO0(symbol##ARB), glimp_bit_cast<void**>(&symbol)}

			RTCW_MACRO(glBindFramebuffer),
			RTCW_MACRO(glBindRenderbuffer),
			RTCW_MACRO(glBlitFramebuffer),
			RTCW_MACRO(glCheckFramebufferStatus),
			RTCW_MACRO(glDeleteFramebuffers),
			RTCW_MACRO(glDeleteRenderbuffers),
			RTCW_MACRO(glFramebufferRenderbuffer),
			RTCW_MACRO(glFramebufferTexture1D),
			RTCW_MACRO(glFramebufferTexture2D),
			RTCW_MACRO(glFramebufferTexture3D),
			RTCW_MACRO(glFramebufferTextureLayer),
			RTCW_MACRO(glGenFramebuffers),
			RTCW_MACRO(glGenRenderbuffers),
			RTCW_MACRO(glGenerateMipmap),
			RTCW_MACRO(glGetFramebufferAttachmentParameteriv),
			RTCW_MACRO(glGetRenderbufferParameteriv),
			RTCW_MACRO(glIsFramebuffer),
			RTCW_MACRO(glIsRenderbuffer),
			RTCW_MACRO(glRenderbufferStorage),
			RTCW_MACRO(glRenderbufferStorageMultisample),

#undef RTCW_MACRO0
#undef RTCW_MACRO

			{NULL, NULL}
		};

		if (glimp_load_gl_functions(S_COLOR_WHITE, gl_function_infos))
		{
			glConfigEx.use_arb_framebuffer_object_ = true;
			glimp_print_using_extension(gl_arb_framebuffer_object_string);
		}
		else
		{
			glimp_print_not_found_extension(gl_arb_framebuffer_object_string);
		}
	}
	else
	{
		glimp_print_not_found_extension(gl_arb_framebuffer_object_string);
	}
}

// ======================================

void glimp_initialize_gl_arb_texture_non_power_of_two_extension()
{
	const char* const gl_arb_texture_non_power_of_two_string = "GL_ARB_texture_non_power_of_two";
	const bool is_gl20 = glimp_gl_version >= GlVersion(2, 0);

	if (is_gl20 || SDL_GL_ExtensionSupported(gl_arb_texture_non_power_of_two_string))
	{
		glConfigEx.use_arb_texture_non_power_of_two_ = true;
		glimp_print_using_extension(gl_arb_texture_non_power_of_two_string);
	}
	else
	{
		glimp_print_not_found_extension(gl_arb_texture_non_power_of_two_string);
	}
}

// ======================================

void glimp_initialize_gl_arb_vertex_array_object_extension()
{
	const char* const gl_arb_vertex_array_object_string = "GL_ARB_vertex_array_object";
	const bool is_gl30 = glimp_gl_version >= GlVersion(3, 0);
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	glConfigEx.use_gl_arb_vertex_array_object = false;

	if (is_gl30 || SDL_GL_ExtensionSupported(gl_arb_vertex_array_object_string))
	{
		GlFunctionInfo gl_function_infos[] =
		{
#define RTCW_MACRO0(x) #x
#define RTCW_MACRO(symbol) {is_gl30 ? #symbol : RTCW_MACRO0(symbol##ARB), glimp_bit_cast<void**>(&symbol)}

			RTCW_MACRO(glBindVertexArray),
			RTCW_MACRO(glDeleteVertexArrays),
			RTCW_MACRO(glGenVertexArrays),
			RTCW_MACRO(glIsVertexArray),

#undef RTCW_MACRO0
#undef RTCW_MACRO

			{NULL, NULL}
		};

		if (glimp_load_gl_functions(S_COLOR_WHITE, gl_function_infos))
		{
			glConfigEx.use_gl_arb_vertex_array_object = true;
			extension_status = EXT_STATUS_USING;
		}
	}

	glimp_print_extension(extension_status, gl_arb_vertex_array_object_string);
}

// ======================================

void glimp_initialize_gl_arb_color_buffer_float_extension()
{
	const char validate_GL_CLAMP_VERTEX_COLOR[2 * (GL_CLAMP_VERTEX_COLOR == GL_CLAMP_VERTEX_COLOR_ARB) - 1] = {0};
	maybe_unused(validate_GL_CLAMP_VERTEX_COLOR);
	const char validate_GL_CLAMP_FRAGMENT_COLOR[2 * (GL_CLAMP_FRAGMENT_COLOR == GL_CLAMP_FRAGMENT_COLOR_ARB) - 1] = {0};
	maybe_unused(validate_GL_CLAMP_FRAGMENT_COLOR);
	const char validate_GL_CLAMP_READ_COLOR[2 * (GL_CLAMP_READ_COLOR == GL_CLAMP_READ_COLOR_ARB) - 1] = {0};
	maybe_unused(validate_GL_CLAMP_READ_COLOR);

	const char* const gl_arb_color_buffer_float_string = "GL_ARB_color_buffer_float";
	const bool is_gl30 = glimp_gl_version >= GlVersion(3, 0);
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	glConfigEx.use_gl_arb_color_buffer_float = false;

	if (is_gl30 || SDL_GL_ExtensionSupported(gl_arb_color_buffer_float_string))
	{
		GlFunctionInfo gl_function_infos[] =
		{
#define RTCW_MACRO0(x) #x
#define RTCW_MACRO(symbol) {is_gl30 ? #symbol : RTCW_MACRO0(symbol##ARB), glimp_bit_cast<void**>(&symbol)}

			RTCW_MACRO(glClampColor),

#undef RTCW_MACRO0
#undef RTCW_MACRO

			{NULL, NULL}
		};

		if (glimp_load_gl_functions(S_COLOR_WHITE, gl_function_infos))
		{
			glConfigEx.use_gl_arb_color_buffer_float = true;
			extension_status = EXT_STATUS_USING;
		}
	}

	glimp_print_extension(extension_status, gl_arb_color_buffer_float_string);
}

// ======================================

void glimp_initialize_gl_arb_texture_float_extension()
{
	const char validate_GL_RGB16F[2 * (GL_RGB16F == GL_RGB16F_ARB) - 1] = {0};
	maybe_unused(validate_GL_RGB16F);
	const char validate_GL_RGBA16F[2 * (GL_RGBA16F == GL_RGBA16F_ARB) - 1] = {0};
	maybe_unused(validate_GL_RGBA16F);

	const char* const gl_arb_texture_float_string = "GL_ARB_texture_float";
	const bool is_gl30 = glimp_gl_version >= GlVersion(3, 0);
	ExtensionStatus extension_status = EXT_STATUS_NOT_FOUND;

	glConfigEx.use_gl_arb_texture_float = false;

	if (is_gl30 || SDL_GL_ExtensionSupported(gl_arb_texture_float_string))
	{
		glConfigEx.use_gl_arb_texture_float = true;
		extension_status = EXT_STATUS_USING;
	}

	glimp_print_extension(extension_status, gl_arb_texture_float_string);
}

// ======================================

void gl_initialize_extensions()
{
	if (r_allowExtensions->integer == 0)
	{
		ri.Printf(PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n");
		return;
	}

	ri.Printf(PRINT_ALL, "Initializing OpenGL extensions\n");
	ri.Printf(PRINT_ALL, "(Legend: [+] using; [-] not found; [*] ignoring)\n");

	glimp_initialize_gl_xxx_texture_compression_extensions();
	glimp_initialize_gl_xxx_texture_env_add_extensions();
	glimp_initialize_xxx_ext_swap_control_extensions();
	glimp_initialize_gl_arb_multitexture_extension();
	glimp_initialize_gl_ext_compiled_vertex_array_extension();
#ifdef RTCW_SP
	glimp_initialize_gl_ati_pn_triangles_extension();
#endif // RTCW_SP
	glimp_initialize_gl_xxx_texture_filter_anisotropic_extensions();
	glimp_initialize_gl_nv_fog_distance_extension();

	glimp_initialize_gl_arb_draw_elements_base_vertex_extension();
	glimp_initialize_gl_arb_framebuffer_object_extension();
	glimp_initialize_gl_arb_texture_non_power_of_two_extension();
	glimp_initialize_gl_arb_vertex_array_object_extension();
	glimp_initialize_gl_arb_color_buffer_float_extension();
	glimp_initialize_gl_arb_texture_float_extension();

	glConfigEx.is_2_x_capable_ = glimp_initialize_gl2_functions();
}

} // namespace


void GLimp_Init()
{
	sys_main_window_id = 0;

	ri.Printf(PRINT_ALL, "Initializing OpenGL subsystem\n");

	r_allowSoftwareGL = ri.Cvar_Get("r_allowSoftwareGL", "0", 0);
	r_maskMinidriver = ri.Cvar_Get("r_maskMinidriver", "0", 0);

	ri.Cvar_Get("r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE);

	glimp_gl_version = glimp_get_gl_version();

	bool is_succeed = true;
	int sdl_result = 0;

	int display_width = 0;
	int display_height = 0;
	int display_refresh_rate = 0;

	if (is_succeed)
	{
		SDL_DisplayMode dm;

		sdl_result = SDL_GetCurrentDisplayMode(0, &dm);

		if (sdl_result == 0)
		{
			display_width = dm.w;
			display_height = dm.h;
			display_refresh_rate = dm.refresh_rate;
		}
		else
		{
			is_succeed = false;
			ri.Error(ERR_FATAL, "Failed to get a current dispay mode.\n");
		}
	}

	int width = 0;
	int height = 0;
	float aspect_ratio = 0.0F;
	const bool is_fullscreen = (r_fullscreen->integer != 0);
	const bool is_stereo = (r_stereo->integer != 0);

	if (is_succeed)
	{
		qboolean api_result = qfalse;

		ri.Printf(PRINT_ALL, "  setting mode: %d\n", r_mode->integer);

		api_result = R_GetModeInfo(
			&width,
			&height,
			&aspect_ratio,
			r_mode->integer
		);

		if (!api_result)
		{
			is_succeed = false;
			ri.Error(ERR_FATAL, "Invalid mode: %d.\n", r_mode->integer);
		}
	}

	int sdl_width = 0;
	int sdl_height = 0;
	float sdl_aspect_ratio = 0.0F;

	if (is_succeed)
	{
		const bool has_sdl_gl_floatbuffers = RTCW_SDL_AT_LEAST_2_24_0;
		const bool is_hdr_supported = has_sdl_gl_floatbuffers && r_hdr->integer != 0;

		for (int i_try = 0, n_try = 1 + is_hdr_supported; i_try < n_try; ++i_try)
		{
			Uint32 sdl_window_flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;

			if (is_fullscreen)
			{
				sdl_window_flags |= SDL_WINDOW_FULLSCREEN;
			}
			else
			{
				if (width == display_width && height == display_height)
				{
					sdl_window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
				}
			}

			SDL_GL_ResetAttributes();
			sdl_result = SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			sdl_result = SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			sdl_result = SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			sdl_result = SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			sdl_result = SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			sdl_result = SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			sdl_result = SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
			sdl_result = SDL_GL_SetAttribute(SDL_GL_STEREO, is_stereo);

			if (is_hdr_supported && i_try == 0)
			{
				sdl_result = SDL_GL_SetAttribute(SDL_GL_FLOATBUFFERS, SDL_TRUE);
			}

			int sdl_x = 0;
			int sdl_y = 0;

			if (is_fullscreen)
			{
				sdl_x = SDL_WINDOWPOS_CENTERED;
				sdl_y = SDL_WINDOWPOS_CENTERED;

				sdl_width = display_width;
				sdl_height = display_height;
			}
			else
			{
				const cvar_t* x_cvar = ri.Cvar_Get("vid_xpos", "0", 0);
				sdl_x = x_cvar->integer;

				if (sdl_x < 0)
				{
					sdl_x = 0;
				}
				else if (sdl_x > display_width - 1)
				{
					sdl_x = display_width - 1;
				}

				const cvar_t* y_cvar = ri.Cvar_Get("vid_ypos", "0", 0);
				sdl_y = y_cvar->integer;

				if (sdl_y < 0)
				{
					sdl_y = 0;
				}
				else if (sdl_y > display_height - 1)
				{
					sdl_y = display_height - 1;
				}

				if (sdl_x == 0)
				{
					sdl_x = SDL_WINDOWPOS_CENTERED;
				}

				if (sdl_y == 0)
				{
					sdl_y = SDL_WINDOWPOS_CENTERED;
				}

				sdl_width = width;
				sdl_height = height;
			}

			sdl_aspect_ratio = static_cast<float>(sdl_width) / static_cast<float>(sdl_height);

			const char* const window_title =
				"Return to Castle Wolfenstein ("
	#if defined RTCW_SP
		#ifdef RTCW_SP_DEMO
				"single-player demo"
		#else
				"single-player"
		#endif
	#elif defined RTCW_MP
				"multi-player"
	#elif defined RTCW_ET
				"Enemy Territory"
	#else
				"???"
	#endif // RTCW_XX
				")"
			;

			sys_gl_window = SDL_CreateWindow(
				window_title,
				sdl_x,
				sdl_y,
				sdl_width,
				sdl_height,
				sdl_window_flags
			);

			if (sys_gl_window != NULL)
			{
				break;
			}
		}

		if (sys_gl_window == NULL)
		{
			ri.Error(ERR_FATAL, "Failed to create SDL window (%s).\n", SDL_GetError());
		}

		rtcw::WindowRoundedCornerMgr::disable(sys_gl_window);
	}

	bool is_support_gamma = false;

	if (is_succeed)
	{
		Uint16 ramp[3][256];

		sdl_result = SDL_GetWindowGammaRamp(
			sys_gl_window,
			ramp[0],
			ramp[1],
			ramp[2]);

		is_support_gamma = (sdl_result == 0);
	}

	if (is_succeed)
	{
		gl_context = SDL_GL_CreateContext(sys_gl_window);

		if (gl_context == NULL)
		{
			is_succeed = false;
			ri.Error(ERR_FATAL, "Failed to create an OpenGL context.\n");
		}
	}

	if (is_succeed)
	{
		glimp_initialize_gl1_essential_functions();
	}

	if (is_succeed)
	{
		const char* strings[2] = {
			"disabled",
			"enabled",
		};

		int attribute = 0;

		sdl_result = SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &attribute);
		ri.Printf(PRINT_ALL, "  GL red size: %d\n", attribute);

		sdl_result = SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &attribute);
		ri.Printf(PRINT_ALL, "  GL green size: %d\n", attribute);

		sdl_result = SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &attribute);
		ri.Printf(PRINT_ALL, "  GL blue size: %d\n", attribute);

		sdl_result = SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &attribute);
		ri.Printf(PRINT_ALL, "  GL alpha size: %d\n", attribute);

		sdl_result = SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &attribute);
		ri.Printf(PRINT_ALL, "  GL double buffering: %s\n",
			strings[attribute]);

		sdl_result = SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &attribute);
		ri.Printf(PRINT_ALL, "  GL depth size: %d\n", attribute);

		sdl_result = SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &attribute);
		ri.Printf(PRINT_ALL, "  GL stencil size: %d\n", attribute);

		sdl_result = SDL_GL_GetAttribute(SDL_GL_STEREO, &attribute);
		ri.Printf(PRINT_ALL, "  GL stereo mode: %s\n",
			strings[attribute]);
	}


	memset(&glConfig, 0, sizeof(glconfig_t));
	glConfigEx.reset();

	if (is_succeed)
	{
		const char* gl_renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		const char* gl_vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		const char* gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
		const char* gl_extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

		const int max_gl_string_size = MAX_STRING_CHARS;
		const int max_gl_extensions_string_size = 4 * MAX_STRING_CHARS;

		glConfig.colorBits = 32;
		glConfig.depthBits = 24;
		glConfig.deviceSupportsGamma = is_support_gamma;
		glConfig.displayFrequency = display_refresh_rate;
		glConfig.driverType = GLDRV_ICD;
		Q_strncpyz(glConfig.extensions_string, gl_extensions, max_gl_extensions_string_size);
		glConfig.hardwareType = GLHW_GENERIC;
		glConfig.isFullscreen = is_fullscreen;
		Q_strncpyz(glConfig.renderer_string, gl_renderer, max_gl_string_size);
		glConfig.stencilBits = 8;
		glConfig.stereoEnabled = is_stereo;
		glConfig.textureCompression = TC_NONE;
		Q_strncpyz(glConfig.vendor_string, gl_vendor, max_gl_string_size);
		Q_strncpyz(glConfig.version_string, gl_version, max_gl_string_size);
		glConfig.vidHeight = sdl_height;
		glConfig.vidWidth = sdl_width;
		glConfig.windowAspect = sdl_aspect_ratio;

		gl_initialize_extensions();

		ri.Cvar_Set("r_highQualityVideo", "1");
		ri.Cvar_Set("r_lastValidRenderer", gl_renderer);

		SDL_ShowWindow(sys_gl_window);

		// Clear the screen.
		//
		glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		SDL_GL_SwapWindow(sys_gl_window);
	}
	else
	{
		if (gl_context != NULL)
		{
			SDL_GL_MakeCurrent(sys_gl_window, NULL);
			SDL_GL_DeleteContext(gl_context);
			gl_context = NULL;
		}

		if (sys_gl_window != NULL)
		{
			SDL_DestroyWindow(sys_gl_window);
			sys_gl_window = NULL;
		}
	}

	if (!is_succeed)
	{
		ri.Error(ERR_FATAL, "Failed to create a window.\n");
	}

	sys_main_window_id = SDL_GetWindowID(sys_gl_window);
}

void GLimp_Shutdown()
{
	if (gl_context != NULL)
	{
		SDL_GL_MakeCurrent(sys_gl_window, NULL);
		SDL_GL_DeleteContext(gl_context);
		gl_context = NULL;
	}

	if (sys_gl_window != NULL)
	{
		SDL_DestroyWindow(sys_gl_window);
		sys_gl_window = NULL;
	}

	sys_main_window_id = 0;

	memset(&glConfig, 0, sizeof(glconfig_t));
	glConfigEx.reset();
}

void GLimp_EndFrame()
{
	if (!sys_gl_window)
	{
		return;
	}

	if (r_swapInterval->modified)
	{
		r_swapInterval->modified = false;

		if (glConfigEx.has_swap_control_)
		{
			int swap_interval = 0;

			if (r_swapInterval->integer < 0)
			{
				if (glConfigEx.has_adaptive_swap_control_)
				{
					swap_interval = -1;
				}
				else
				{
					swap_interval = 1;
				}
			}
			else if (r_swapInterval->integer == 0)
			{
				swap_interval = 0;
			}
			else
			{
				swap_interval = 1;
			}

			static_cast<void>(SDL_GL_SetSwapInterval(swap_interval));
		}
	}

	if (Q_stricmp(r_drawBuffer->string, "GL_FRONT") != 0)
	{
		SDL_GL_SwapWindow(sys_gl_window);
	}
}

void GLimp_SetGamma(
	uint8_t red[256],
	uint8_t green[256],
	uint8_t blue[256])
{
	if (!glConfig.deviceSupportsGamma ||
		r_ignorehwgamma->integer != 0 ||
		sys_gl_window == NULL)
	{
		return;
	}

	uint16_t table[3][256];

	for (int i = 0; i < 256; ++i) {
		table[0][i] = static_cast<uint16_t>((red[i] << 8) | red[i]);
		table[1][i] = static_cast<uint16_t>((green[i] << 8) | green[i]);
		table[2][i] = static_cast<uint16_t>((blue[i] << 8) | blue[i]);
	}

	// enforce constantly increasing
	for (int j = 0 ; j < 3 ; ++j) {
		for (int i = 1 ; i < 256 ; ++i) {
			if (table[j][i] < table[j][i - 1])
				table[j][i] = table[j][i - 1];
		}
	}

	int sdl_result = 0;

	sdl_result = SDL_SetWindowGammaRamp(
		sys_gl_window,
		table[0],
		table[1],
		table[2]);

	if (sdl_result != 0) {
		ri.Printf(PRINT_ALL, S_COLOR_YELLOW "SDL gamma: %s\n",
			SDL_GetError());
	}
}

void GLimp_Activate (
	bool is_activated,
	bool is_minimized)
{
	maybe_unused(is_activated);
	maybe_unused(is_minimized);
}

bool GLimp_SetFullscreen(bool value)
{
	ri.Printf(PRINT_ALL, "Trying to set %s mode without video restart...\n",
		value ? "fullscreen" : "windowed");

	if (sys_gl_window == NULL)
	{
		ri.Printf(PRINT_ALL, S_COLOR_YELLOW "  no main window.\n");
		return false;
	}

	const Uint32 sdl_current_flags = SDL_GetWindowFlags(sys_gl_window);

	const bool is_current_fullscreen =
		(sdl_current_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN;

	if (is_current_fullscreen == value)
	{
		ri.Printf(PRINT_ALL, "  nothing to change.\n");
		return true;
	}

	Uint32 sdl_flags = 0;

	if (value)
	{
		sdl_flags = SDL_WINDOW_FULLSCREEN;
	}
	else
	{
		SDL_DisplayMode sdl_display_mode;

		if (SDL_GetDesktopDisplayMode(0, &sdl_display_mode) == 0)
		{
			if (sdl_display_mode.w == glConfig.vidWidth && sdl_display_mode.h == glConfig.vidHeight)
			{
				ri.Printf(PRINT_ALL, "  set borderless\n");
				sdl_flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
			}
		}
	}

	const int sdl_result = SDL_SetWindowFullscreen(sys_gl_window, sdl_flags);

	if (sdl_result == 0)
	{
		ri.Printf(PRINT_ALL, "  succeed.\n");
		return true;
	}

	ri.Printf(PRINT_ALL, "  failed. %s\n", SDL_GetError());
	return false;
}
