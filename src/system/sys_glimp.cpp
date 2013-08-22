/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (?RTCW SP Source Code?).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/


#include "SDL.h"

#include "tr_local.h"


SDL_Window* sys_gl_window;

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


bool gl_has_extension(const char* extension_name)
{
    return ::SDL_GL_ExtensionSupported(extension_name) != SDL_FALSE;
}

void gl_print_extension(
    ExtensionStatus status,
    const char* extension_name)
{
    if (extension_name == nullptr)
        return;

    if (extension_name[0] == '\0')
        return;


    const char* status_mark = nullptr;
    const char* color_mark = nullptr;

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

    ::ri.Printf(
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

void gl_initialize_extensions()
{
    if (r_allowExtensions->integer == 0) {
        ::ri.Printf(PRINT_ALL, S_COLOR_YELLOW "Ignoring OpenGL extensions\n");
        return;
    }

    const char* extension_name1 = nullptr;
    const char* extension_name2 = nullptr;
    const char* extension_name3 = nullptr;
    const char* extension_name4 = nullptr;

    ::ri.Printf(PRINT_ALL, "Initializing OpenGL extensions\n");
    ::ri.Printf(PRINT_ALL, "(Legend: [+] found; [-] not found; [*] ignored)\n");


    extension_name1 = "GL_ARB_texture_compression";
    extension_name2 = "GL_EXT_texture_compression_s3tc";
    extension_name3 = "GL_S3_s3tc";

    if (gl_has_extension(extension_name1)) {
        if (r_ext_compressed_textures->integer != 0) {
            glConfig.textureCompression = TC_ARB;
            gl_print_found_extension(extension_name1);
        } else {
            gl_print_ignored_extension(extension_name1);
        }
    } else if (gl_has_extension(extension_name2)) {
        if (r_ext_compressed_textures->integer != 0) {
            glConfig.textureCompression = TC_EXT_COMP_S3TC;
            gl_print_found_extension(extension_name2);
        } else {
            gl_print_ignored_extension(extension_name2);
        }
    } else if (gl_has_extension(extension_name3)) {
        if (r_ext_compressed_textures->integer != 0) {
            glConfig.textureCompression = TC_S3TC;
            gl_print_found_extension(extension_name3);
        } else
            gl_print_ignored_extension(extension_name3);
    } else
        gl_print_missed_extension("any supported texture compression");


    extension_name1 = "GL_EXT_texture_env_add";

    if (gl_has_extension(extension_name1)) {
        if (r_ext_texture_env_add->integer != 0) {
            glConfig.textureEnvAddAvailable = true;
            gl_print_found_extension(extension_name1);
        } else {
            glConfig.textureEnvAddAvailable = false;
            gl_print_ignored_extension(extension_name1);
        }
    } else
        gl_print_missed_extension(extension_name1);


    extension_name1 = "WGL_EXT_swap_control";

    if (gl_has_extension(extension_name1)) {
        r_swapInterval->modified = true;
        gl_print_found_extension(extension_name1);
    } else
        gl_print_missed_extension(extension_name1);


    extension_name1 = "GL_ARB_multitexture";
    extension_name2 = "GL_ARB_multitexture/units less than 2";

    if (gl_has_extension(extension_name1)) {
        if (r_ext_multitexture->integer != 0) {
            ::glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &glConfig.maxActiveTextures);

            if (glConfig.maxActiveTextures > 1) {
                glConfigEx.useArbMultitexture = true;
                gl_print_found_extension(extension_name1);
            } else {
                gl_print_missed_extension(extension_name2);
            }
        } else
            gl_print_ignored_extension(extension_name1);
    } else
        gl_print_missed_extension(extension_name1);


    extension_name1 = "GL_EXT_compiled_vertex_array";

    if (gl_has_extension(extension_name1)) {
        if (r_ext_compiled_vertex_array->integer != 0) {
            glConfigEx.useExtCompiledVertexArray = true;
            gl_print_found_extension(extension_name1);
        } else {
            gl_print_ignored_extension(extension_name1);
        }
    } else
        gl_print_missed_extension(extension_name1);


    extension_name1 = "GL_NV_fog_distance";

    if (gl_has_extension(extension_name1)) {
        if (r_ext_NV_fog_dist->integer != 0) {
            ::glConfig.NVFogAvailable = true;
            gl_print_found_extension(extension_name1);
        } else {
            ::ri.Cvar_Set ("r_ext_NV_fog_dist", "0");
            gl_print_ignored_extension(extension_name1);
        }
    } else {
        ::ri.Cvar_Set ("r_ext_NV_fog_dist", "0");
        gl_print_missed_extension(extension_name1);
    }


    extension_name1 = "GL_EXT_texture_filter_anisotropic";

    if (gl_has_extension(extension_name1)) {
        if (r_ext_texture_filter_anisotropic->integer != 0) {
            glConfig.anisotropicAvailable = true;
            gl_print_found_extension(extension_name1);
        } else {
            ::ri.Cvar_Set ("r_ext_texture_filter_anisotropic", "0");
            gl_print_ignored_extension(extension_name1);
        }
    } else {
        ::ri.Cvar_Set ("r_ext_texture_filter_anisotropic", "0");
        gl_print_missed_extension(extension_name1);
    }


    extension_name1 = "GL_ARB_framebuffer_object";

    if (gl_has_extension(extension_name1)) {
        glConfigEx.useArbFramebufferObject = true;
        gl_print_found_extension(extension_name1);
    } else
        gl_print_missed_extension(extension_name1);


    extension_name1 = "GL_ARB_framebuffer_object";

    if (gl_has_extension(extension_name1)) {
        glConfigEx.useArbTextureNonPowerOfTwo = true;
        gl_print_found_extension(extension_name1);
    } else
        gl_print_missed_extension(extension_name1);


    extension_name1 = "GL_ARB_draw_elements_base_vertex";

    if (gl_has_extension(extension_name1)) {
        glConfigEx.use_arb_draw_elements_base_vertex = true;
        gl_print_found_extension(extension_name1);
    } else
        gl_print_missed_extension(extension_name1);
}


} // namespace


void GLimp_Init()
{
    ::ri.Printf(PRINT_ALL, "Initializing OpenGL subsystem\n");

    ::r_allowSoftwareGL = ::ri.Cvar_Get("r_allowSoftwareGL", "0", 0);
    ::r_maskMinidriver = ::ri.Cvar_Get("r_maskMinidriver", "0", 0);

    auto r_lastValidRenderer = ::ri.Cvar_Get(
        "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE);

    auto is_succeed = true;
    auto sdl_result = 0;

    if (is_succeed) {
        sdl_result = ::SDL_InitSubSystem(SDL_INIT_VIDEO);

        if (sdl_result != 0) {
            is_succeed = false;
            ::ri.Error(ERR_FATAL, "%s\n", ::SDL_GetError());
        }
    }


    auto display_width = 0;
    auto display_height = 0;
    auto display_refresh_rate = 0;

    if (is_succeed) {
        SDL_DisplayMode dm;

        sdl_result = ::SDL_GetCurrentDisplayMode(
            0,
            &dm);

        if (sdl_result == 0) {
            display_width = dm.w;
            display_height = dm.h;
            display_refresh_rate = dm.refresh_rate;
        } else {
            is_succeed = false;
            ::ri.Error(ERR_FATAL, "Failed to get a current dispay mode.\n");
        }
    }

    auto width = 0;
    auto height = 0;
    auto aspect_ratio = 0.0F;
    auto is_native_mode = false;
    auto is_fullscreen = (r_fullscreen->integer != 0);
    auto is_stereo = (r_stereo->integer != 0);

    if (is_succeed) {
        auto api_result = qfalse;

        ::ri.Printf(PRINT_ALL, "  setting mode: %d\n", r_mode->integer);

        api_result = ::R_GetModeInfo(
            &width,
            &height,
            &aspect_ratio,
            r_mode->integer);

        if (api_result) {
            is_native_mode = (
                width == display_width &&
                height == display_height);
        } else {
            is_succeed = false;
            ::ri.Error(ERR_FATAL, "Invalid mode: %d.\n", r_mode->integer);
        }
    }

    if (is_succeed) {
        Uint32 window_flags =
            SDL_WINDOW_OPENGL |
            SDL_WINDOW_HIDDEN;

        if (is_fullscreen) {
            window_flags |= is_native_mode ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
        }

        sdl_result = ::SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        sdl_result = ::SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        sdl_result = ::SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        sdl_result = ::SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        sdl_result = ::SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        sdl_result = ::SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        sdl_result = ::SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        sdl_result = ::SDL_GL_SetAttribute(SDL_GL_STEREO, is_stereo);

        auto x_cvar = ::ri.Cvar_Get("vid_xpos", "0", 0);
        auto x = x_cvar->integer;

        auto y_cvar = ::ri.Cvar_Get("vid_ypos", "0", 0);
        auto y = y_cvar->integer;

        if (x < 0)
            x = 0;

        if (y < 0)
            y = 0;

        if ((x + display_width) < width && (y + display_height) < height) {
            if ((x + width) > display_width)
                x = display_width - width;

            if ((y + height) > display_height)
                y = display_height - height;
        }

        const char* window_title =
            "Return to Castle Wolfenstein: "
#if defined RTCW_SP
                "Single Player"
#elif defined RTCW_MP
                "Multi Player"
#else
                "Enemy Territory"
#endif // RTCW_XX
            ;

        bool is_fallback_mode = (
            width == FALLBACK_WIDTH ||
            height == FALLBACK_HEIGHT);

        while (is_succeed) {
            sys_gl_window = ::SDL_CreateWindow(
                window_title,
                x,
                y,
                width,
                height,
                window_flags);

            if (sys_gl_window != nullptr)
                break;

            x = 0;
            y = 0;

            if (is_fullscreen) {
                ::ri.Printf(PRINT_ALL, S_COLOR_YELLOW
                    "  SDL: %s\n", ::SDL_GetError());

                ::ri.Printf(PRINT_ALL, S_COLOR_YELLOW
                    "  trying windowed mode...\n");

                is_fullscreen = false;

                window_flags &= ~(SDL_WINDOW_FULLSCREEN |
                    SDL_WINDOW_FULLSCREEN_DESKTOP);
            } else {
                if (is_fallback_mode) {
                    is_succeed = false;

                    ::ri.Printf(PRINT_ALL, S_COLOR_RED
                        "  SDL: %s\n", ::SDL_GetError());
                } else {
                    ::ri.Printf(PRINT_ALL, S_COLOR_YELLOW
                        "  SDL: %s\n", ::SDL_GetError());

                    ::ri.Printf(PRINT_ALL, S_COLOR_YELLOW
                        "  trying fallback mode: %dx%d windowed...\n",
                        FALLBACK_WIDTH, FALLBACK_HEIGHT);

                    width = FALLBACK_WIDTH;
                    height = FALLBACK_HEIGHT;
                }
            }
        }
    }

    bool is_support_gamma = false;

    if (is_succeed) {
        Uint16 ramp[3][256];

        sdl_result = ::SDL_GetWindowGammaRamp(
            sys_gl_window,
            ramp[0],
            ramp[1],
            ramp[2]);

        is_support_gamma = (sdl_result == 0);
    }

    if (is_succeed) {
        gl_context = ::SDL_GL_CreateContext(sys_gl_window);

        if (gl_context == nullptr) {
            is_succeed = false;
            ::ri.Error(ERR_FATAL, "Failed to create an OpenGL context.\n");
        }
    }

    std::string gl_renderer;
    std::string gl_vendor;
    std::string gl_version;
    std::string gl_extensions;

    if (is_succeed) {
        auto glew_result = ::glewInit();

        if (glew_result == GLEW_OK) {
            const int MAX_GL_STRING_LENGTH = MAX_STRING_CHARS - 1;
            const int MAX_GL_EXT_STRING_LENGTH = (4 * MAX_STRING_CHARS) - 1;

            gl_renderer = reinterpret_cast<const char*>(
                ::glGetString(GL_RENDERER));

            if (gl_renderer.size() > MAX_GL_STRING_LENGTH)
                gl_renderer.resize(MAX_GL_STRING_LENGTH);

            gl_vendor = reinterpret_cast<const char*>(
                ::glGetString(GL_VENDOR));

            if (gl_vendor.size() > MAX_GL_STRING_LENGTH)
                gl_vendor.resize(MAX_GL_STRING_LENGTH);

            gl_version = reinterpret_cast<const char*>(
                ::glGetString(GL_VERSION));

            if (gl_version.size() > MAX_GL_STRING_LENGTH)
                gl_version.resize(MAX_GL_STRING_LENGTH);

            gl_extensions = reinterpret_cast<const char*>(
                ::glGetString(GL_EXTENSIONS));

            if (gl_extensions.size() > MAX_GL_EXT_STRING_LENGTH)
                gl_extensions.resize(MAX_GL_EXT_STRING_LENGTH);
        } else {
            is_succeed = false;
            ::ri.Error(ERR_FATAL, "Failed to initialize GLEW: %s\n",
                ::glewGetErrorString(glew_result));
        }
    }

    if (is_succeed) {
        const char* strings[2] = {
            "disabled",
            "enabled",
        };

        auto attribute = 0;

        sdl_result = ::SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &attribute);
        ::ri.Printf(PRINT_ALL, "  GL red size: %d\n", attribute);

        sdl_result = ::SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &attribute);
        ::ri.Printf(PRINT_ALL, "  GL green size: %d\n", attribute);

        sdl_result = ::SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &attribute);
        ::ri.Printf(PRINT_ALL, "  GL blue size: %d\n", attribute);

        sdl_result = ::SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &attribute);
        ::ri.Printf(PRINT_ALL, "  GL alpha size: %d\n", attribute);

        sdl_result = ::SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &attribute);
        ::ri.Printf(PRINT_ALL, "  GL double buffering: %s\n",
            strings[attribute]);

        sdl_result = ::SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &attribute);
        ::ri.Printf(PRINT_ALL, "  GL depth size: %d\n", attribute);

        sdl_result = ::SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &attribute);
        ::ri.Printf(PRINT_ALL, "  GL stencil size: %d\n", attribute);

        sdl_result = ::SDL_GL_GetAttribute(SDL_GL_STEREO, &attribute);
        ::ri.Printf(PRINT_ALL, "  GL stereo mode: %s\n",
            strings[attribute]);
    }


    std::uninitialized_fill_n(
        reinterpret_cast<char*>(&glConfig),
        sizeof(glConfig),
        0);

    glConfigEx.reset();

    if (is_succeed) {
        glConfig.colorBits = 32;
        glConfig.depthBits = 24;
        glConfig.deviceSupportsGamma = is_support_gamma;
        glConfig.displayFrequency = display_refresh_rate;
        glConfig.driverType = GLDRV_ICD;
        std::uninitialized_copy_n(
            gl_extensions.c_str(),
            gl_extensions.size(),
            glConfig.extensions_string);
        glConfig.hardwareType = GLHW_GENERIC;
        glConfig.isFullscreen = is_fullscreen;
        std::uninitialized_copy_n(
            gl_renderer.c_str(),
            gl_renderer.size(),
            glConfig.renderer_string);
        glConfig.stencilBits = 8;
        glConfig.stereoEnabled = is_stereo;
        glConfig.textureCompression = TC_NONE;
        std::uninitialized_copy_n(
            gl_vendor.c_str(),
            gl_vendor.size(),
            glConfig.vendor_string);
        std::uninitialized_copy_n(
            gl_version.c_str(),
            gl_version.size(),
            glConfig.version_string);
        glConfig.vidHeight = height;
        glConfig.vidWidth = width;
        glConfig.windowAspect = aspect_ratio;

        gl_initialize_extensions();

        ::ri.Cvar_Set("r_highQualityVideo", "1");
        ::ri.Cvar_Set("r_lastValidRenderer", gl_renderer.c_str());

        ::SDL_ShowWindow(sys_gl_window);
    } else {
        if (gl_context != nullptr) {
            ::SDL_GL_MakeCurrent(sys_gl_window, nullptr);
            ::SDL_GL_DeleteContext(gl_context);
            gl_context = nullptr;
        }

        if (sys_gl_window != nullptr) {
            ::SDL_DestroyWindow(sys_gl_window);
            sys_gl_window = nullptr;
        }
    }

    if (!is_succeed) {
        ri.Error(ERR_FATAL, "Failed to create a window.\n");
        throw 0;
    }
}

void GLimp_Shutdown()
{
    if (gl_context != nullptr) {
        ::SDL_GL_MakeCurrent(sys_gl_window, nullptr);
        ::SDL_GL_DeleteContext(gl_context);
        gl_context = nullptr;
    }

    if (sys_gl_window != nullptr) {
        ::SDL_DestroyWindow(sys_gl_window);
        sys_gl_window = nullptr;
    }

    std::uninitialized_fill_n(
        reinterpret_cast<char*>(&glConfig),
        sizeof(glConfig),
        0);

    glConfigEx.reset();
}

void GLimp_EndFrame()
{
    if (sys_gl_window == nullptr)
        return;

    if (r_swapInterval->modified) {
        r_swapInterval->modified = false;

        ::SDL_GL_SetSwapInterval(r_swapInterval->integer);
    }

    if (::Q_stricmp(r_drawBuffer->string, "GL_FRONT") != 0)
        ::SDL_GL_SwapWindow(sys_gl_window);
}

void GLimp_SetGamma(
    uint8_t red[256],
    uint8_t green[256],
    uint8_t blue[256])
{
    if (!glConfig.deviceSupportsGamma ||
        r_ignorehwgamma->integer != 0 ||
        sys_gl_window == nullptr)
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

    auto sdl_result = 0;

    sdl_result = ::SDL_SetWindowGammaRamp(
        sys_gl_window,
        table[0],
        table[1],
        table[2]);

    if (sdl_result != 0) {
        ri.Printf(PRINT_ALL, S_COLOR_YELLOW "SDL gamma: %s\n",
            ::SDL_GetError());
    }
}

void GLimp_Activate (
    bool is_activated,
    bool is_minimized)
{
}

bool GLimp_SetFullscreen(bool value)
{
    ::ri.Printf(PRINT_ALL, "Trying to set %s mode without video restart...\n",
        value ? "fullscreen" : "windowed");

    if (sys_gl_window == nullptr) {
        ::ri.Printf(PRINT_ALL, S_COLOR_YELLOW "  no main window.\n");
        return false;
    }

    auto sdl_result = 0;
    uint32_t flags = 0;

    sdl_result = ::SDL_SetWindowFullscreen(
        sys_gl_window,
        value);

    if (sdl_result == 0) {
        ::ri.Printf(PRINT_ALL, "  succeed.\n");
        return true;
    }

    ::ri.Printf(PRINT_ALL, "  failed.\n");
    return false;
}