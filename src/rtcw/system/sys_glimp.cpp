/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include <algorithm>
#include <memory>

#include "SDL_video.h"

#include "tr_local.h"
#include "rtcw_window_rounded_corner_mgr.h"


SDL_Window* sys_gl_window;
Uint32 sys_main_window_id = 0;

// don't abort out if the pixelformat claims software
cvar_t* r_allowSoftwareGL;

// allow a different dll name to be treated as if it were opengl32.dll
cvar_t* r_maskMinidriver;

#ifdef RTCW_ET
int gl_NormalFontBase = 0;
static qboolean fontbase_init = qfalse;
#endif // RTCW_XX


namespace {


const int FALLBACK_WIDTH = 640;
const int FALLBACK_HEIGHT = 480;

enum ExtensionStatus {
	EXT_STATUS_FOUND,
	EXT_STATUS_MISSED,
	EXT_STATUS_IGNORED,
}; // enum ExtensionStatus


SDL_GLContext gl_context;


bool gl_is_2_x_capable()
{
	if (!(SDL_GL_ExtensionSupported("GL_ARB_multitexture") &&
		SDL_GL_ExtensionSupported("GL_ARB_shader_objects") &&
		SDL_GL_ExtensionSupported("GL_ARB_vertex_buffer_object") &&
		SDL_GL_ExtensionSupported("GL_ARB_vertex_program") &&
		SDL_GL_ExtensionSupported("GL_ARB_vertex_shader")))
	{
		return false;
	}

	if (!(glActiveTexture &&
		glAttachShader &&
		glBindBuffer &&
		glBufferData &&
		glBufferSubData &&
		glCompileShader &&
		glCreateProgram &&
		glCreateShader &&
		glDeleteBuffers &&
		glDeleteProgram &&
		glDeleteShader &&
		glDisableVertexAttribArray &&
		glEnableVertexAttribArray &&
		glGenBuffers &&
		glGetAttribLocation &&
		glGetProgramInfoLog &&
		glGetProgramiv &&
		glGetShaderInfoLog &&
		glGetShaderiv &&
		glGetUniformLocation &&
		glLinkProgram &&
		glShaderSource &&
		glUniform1f &&
		glUniform1i &&
		glUniform4fv &&
		glUniformMatrix4fv &&
		glUseProgram &&
		glVertexAttrib2f &&
		glVertexAttrib4f &&
		glVertexAttribPointer))
	{
		return false;
	}

	return true;
}

void gl_print_extension(
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
	case EXT_STATUS_FOUND:
		status_mark = "+";
		color_mark = S_COLOR_GREEN;
		break;

	case EXT_STATUS_MISSED:
		status_mark = "-";
		color_mark = S_COLOR_RED;
		break;

	case EXT_STATUS_IGNORED:
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

void gl_print_found_extension(const char* extension_name)
{
	gl_print_extension(EXT_STATUS_FOUND, extension_name);
}

void gl_print_missed_extension(const char* extension_name)
{
	gl_print_extension(EXT_STATUS_MISSED, extension_name);
}

void gl_print_ignored_extension(const char* extension_name)
{
	gl_print_extension(EXT_STATUS_IGNORED, extension_name);
}

void gl_probe_swap_control()
{
	const int old_swap_interval = SDL_GL_GetSwapInterval();

	const int adaptive_result = SDL_GL_SetSwapInterval(-1);
	glConfigEx.has_adaptive_swap_control_ = (adaptive_result == 0);

	const int off_result = SDL_GL_SetSwapInterval(0);
	const int on_result = SDL_GL_SetSwapInterval(1);
	glConfigEx.has_swap_control_ = (off_result == 0 && on_result == 0);

	const int old_result = SDL_GL_SetSwapInterval(old_swap_interval);
	static_cast<void>(old_result);
}

void gl_initialize_extensions()
{
	if (r_allowExtensions->integer == 0)
	{
		ri.Printf(PRINT_ALL, S_COLOR_YELLOW "Ignoring OpenGL extensions\n");
		return;
	}

	ri.Printf(PRINT_ALL, "Initializing OpenGL extensions\n");
	ri.Printf(PRINT_ALL, "(Legend: [+] found; [-] not found; [*] ignored)\n");

	const char* const gl_arb_texture_compression_string = "GL_ARB_texture_compression";
	const char* const gl_ext_texture_compression_s3tc_string = "GL_EXT_texture_compression_s3tc";
	const char* const gl_s3_s3tc_string = "GL_S3_s3tc";
	const char* const gl_ext_texture_env_add_string = "GL_EXT_texture_env_add";
	const char* const xxx_ext_swap_control_string = "XXX_EXT_swap_control";
	const char* const xxx_ext_swap_control_tear_string = "XXX_EXT_swap_control_tear";
	const char* const gl_arb_multitexture_string = "GL_ARB_multitexture";
	const char* const gl_ext_compiled_vertex_array_string = "GL_EXT_compiled_vertex_array";
	const char* const gl_nv_fog_distance_string = "GL_NV_fog_distance";
	const char* const gl_ext_texture_filter_anisotropic_string = "GL_EXT_texture_filter_anisotropic";
	const char* const gl_arb_framebuffer_object_string = "GL_ARB_framebuffer_object";
	const char* const gl_arb_draw_elements_base_vertex_string = "GL_ARB_draw_elements_base_vertex";

	if (SDL_GL_ExtensionSupported(gl_arb_texture_compression_string))
	{
		if (r_ext_compressed_textures->integer != 0)
		{
			glConfig.textureCompression = TC_ARB;
			gl_print_found_extension(gl_arb_texture_compression_string);
		}
		else
		{
			gl_print_ignored_extension(gl_arb_texture_compression_string);
		}
	}
	else if (SDL_GL_ExtensionSupported(gl_ext_texture_compression_s3tc_string))
	{
		if (r_ext_compressed_textures->integer != 0)
		{
			glConfig.textureCompression = TC_EXT_COMP_S3TC;
			gl_print_found_extension(gl_ext_texture_compression_s3tc_string);
		}
		else
		{
			gl_print_ignored_extension(gl_ext_texture_compression_s3tc_string);
		}
	}
	else if (SDL_GL_ExtensionSupported(gl_s3_s3tc_string))
	{
		if (r_ext_compressed_textures->integer != 0)
		{
			glConfig.textureCompression = TC_S3TC;
			gl_print_found_extension(gl_s3_s3tc_string);
		}
		else
		{
			gl_print_ignored_extension(gl_s3_s3tc_string);
		}
	}
	else
	{
		gl_print_missed_extension("any supported texture compression");
	}

	if (SDL_GL_ExtensionSupported(gl_ext_texture_env_add_string))
	{
		if (r_ext_texture_env_add->integer != 0)
		{
			glConfig.textureEnvAddAvailable = true;
			gl_print_found_extension(gl_ext_texture_env_add_string);
		}
		else
		{
			glConfig.textureEnvAddAvailable = false;
			gl_print_ignored_extension(gl_ext_texture_env_add_string);
		}
	}
	else
	{
		gl_print_missed_extension(gl_ext_texture_env_add_string);
	}

	gl_probe_swap_control();

	if (glConfigEx.has_swap_control_)
	{
		r_swapInterval->modified = true;
		gl_print_found_extension(xxx_ext_swap_control_string);
	}
	else
	{
		gl_print_missed_extension(xxx_ext_swap_control_string);
	}

	if (glConfigEx.has_adaptive_swap_control_)
	{
		r_swapInterval->modified = true;
		gl_print_found_extension(xxx_ext_swap_control_tear_string);
	}
	else
	{
		gl_print_missed_extension(xxx_ext_swap_control_tear_string);
	}

	if (SDL_GL_ExtensionSupported(gl_arb_multitexture_string))
	{
		if (r_ext_multitexture->integer != 0)
		{
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &glConfig.maxActiveTextures);

			if (glConfig.maxActiveTextures > 1)
			{
				glConfigEx.use_arb_multitexture_ = true;
				gl_print_found_extension(gl_arb_multitexture_string);
			}
			else
			{
				gl_print_missed_extension("GL_ARB_multitexture/units less than 2");
			}
		}
		else
		{
			gl_print_ignored_extension(gl_arb_multitexture_string);
		}
	}
	else
	{
		gl_print_missed_extension(gl_arb_multitexture_string);
	}

	if (SDL_GL_ExtensionSupported("GL_EXT_compiled_vertex_array"))
	{
		if (r_ext_compiled_vertex_array->integer != 0)
		{
			glConfigEx.use_ext_compiled_vertex_array_ = true;
			gl_print_found_extension(gl_ext_compiled_vertex_array_string);
		}
		else
		{
			gl_print_ignored_extension(gl_ext_compiled_vertex_array_string);
		}
	}
	else
	{
		gl_print_missed_extension(gl_ext_compiled_vertex_array_string);
	}

	if (SDL_GL_ExtensionSupported(gl_nv_fog_distance_string))
	{
		if (r_ext_NV_fog_dist->integer != 0)
		{
			glConfig.NVFogAvailable = true;
			gl_print_found_extension(gl_nv_fog_distance_string);
		}
		else
		{
			ri.Cvar_Set("r_ext_NV_fog_dist", "0");
			gl_print_ignored_extension(gl_nv_fog_distance_string);
		}
	}
	else
	{
		ri.Cvar_Set("r_ext_NV_fog_dist", "0");
		gl_print_missed_extension(gl_nv_fog_distance_string);
	}

	if (SDL_GL_ExtensionSupported(gl_ext_texture_filter_anisotropic_string))
	{
		if (r_ext_texture_filter_anisotropic->integer != 0)
		{
			glConfig.anisotropicAvailable = true;
			gl_print_found_extension(gl_ext_texture_filter_anisotropic_string);
		}
		else
		{
			gl_print_ignored_extension(gl_ext_texture_filter_anisotropic_string);
		}
	}
	else
	{
		gl_print_missed_extension(gl_ext_texture_filter_anisotropic_string);
	}

	if (SDL_GL_ExtensionSupported(gl_arb_framebuffer_object_string))
	{
		glConfigEx.use_arb_framebuffer_object_ = true;
		glConfigEx.use_arb_texture_non_power_of_two_ = true;
		gl_print_found_extension(gl_arb_framebuffer_object_string);
	}
	else
	{
		gl_print_missed_extension(gl_arb_framebuffer_object_string);
	}

	if (SDL_GL_ExtensionSupported(gl_arb_draw_elements_base_vertex_string))
	{
		glConfigEx.use_arb_draw_elements_base_vertex = true;
		gl_print_found_extension(gl_arb_draw_elements_base_vertex_string);
	}
	else
	{
		gl_print_missed_extension(gl_arb_draw_elements_base_vertex_string);
	}

	glConfigEx.is_2_x_capable_ = gl_is_2_x_capable();
}

} // namespace


void GLimp_Init()
{
	sys_main_window_id = 0;

	ri.Printf(PRINT_ALL, "Initializing OpenGL subsystem\n");

	r_allowSoftwareGL = ri.Cvar_Get("r_allowSoftwareGL", "0", 0);
	r_maskMinidriver = ri.Cvar_Get("r_maskMinidriver", "0", 0);

	ri.Cvar_Get("r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE);

	bool is_succeed = true;
	int sdl_result = 0;

	if (is_succeed)
	{
		sdl_result = SDL_GL_LoadLibrary(NULL);

		if (sdl_result != 0)
		{
			is_succeed = false;
			ri.Error(ERR_FATAL, "Failed to load OpenGL library: %s\n", SDL_GetError());
		}
	}

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
	bool is_native_mode = false;
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
		Uint32 sdl_window_flags =
			SDL_WINDOW_OPENGL |
			SDL_WINDOW_HIDDEN
			;

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

		sdl_result = SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		sdl_result = SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		sdl_result = SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		sdl_result = SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		sdl_result = SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		sdl_result = SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		sdl_result = SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		sdl_result = SDL_GL_SetAttribute(SDL_GL_STEREO, is_stereo);

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

		const char* window_title =
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
		is_succeed = qgl_init();
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

	SDL_GL_UnloadLibrary();
	qgl_shutdown();
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
		table[0][i] = (static_cast<uint16_t>(red[i]) << 8) | red[i];
		table[1][i] = (static_cast<uint16_t>(green[i]) << 8) | green[i];
		table[2][i] = (static_cast<uint16_t>(blue[i]) << 8) | blue[i];
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
	static_cast<void>(is_activated);
	static_cast<void>(is_minimized);
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
