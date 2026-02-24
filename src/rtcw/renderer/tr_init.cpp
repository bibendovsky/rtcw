/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// tr_init.c -- functions that are not called every frame

#include "tr_local.h"
#include "rtcw_hdr_mgr.h"
#include "rtcw_unique_ptr.h"

#if !defined RTCW_ET
//#ifdef __USEA3D
//// Defined in snd_a3dg_refcommon.c
//void RE_A3D_RenderGeometry (void *pVoidA3D, void *pVoidGeom, void *pVoidMat, void *pVoidGeomStatus);
//#endif
#endif // RTCW_XX

glconfig_t glConfig;

// BBi
const GLintptr OglTessLayout::POS_OFS =
	offsetof (OglTessLayout, position);

const GLintptr OglTessLayout::TC0_OFS =
	offsetof (OglTessLayout, texture_coords[0]);

const GLintptr OglTessLayout::TC1_OFS =
	offsetof (OglTessLayout, texture_coords[1]);

const GLintptr OglTessLayout::COL_OFS =
	offsetof (OglTessLayout, color);


const GLvoid* OglTessLayout::POS_PTR =
	reinterpret_cast<const GLvoid*> (OglTessLayout::POS_OFS);

const GLvoid* OglTessLayout::TC0_PTR =
	reinterpret_cast<const GLvoid*> (OglTessLayout::TC0_OFS);

const GLvoid* OglTessLayout::TC1_PTR =
	reinterpret_cast<const GLvoid*> (OglTessLayout::TC1_OFS);

const GLvoid* OglTessLayout::COL_PTR =
	reinterpret_cast<const GLvoid*> (OglTessLayout::COL_OFS);


const size_t OglTessLayout::POS_SIZE =
	offsetof (OglTessLayout, position[1]) -
	offsetof (OglTessLayout, position[0]);

const size_t OglTessLayout::TC0_SIZE =
	offsetof (OglTessLayout, texture_coords[0][1]) -
	offsetof (OglTessLayout, texture_coords[0][0]);

const size_t OglTessLayout::TC1_SIZE =
	offsetof (OglTessLayout, texture_coords[1][1]) -
	offsetof (OglTessLayout, texture_coords[1][0]);

const size_t OglTessLayout::COL_SIZE =
	offsetof (OglTessLayout, color[1]) -
	offsetof (OglTessLayout, color[0]);

rtcw::UniquePtr<rtcw::HdrMgr, rtcw::HdrMgrDeleter> r_hdr_mgr_uptr;

GlConfigEx glConfigEx;

rtcw::OglTessState ogl_tess_state;

GLuint ogl_tess_vbo = 0;
int ogl_tess_base_vertex = 0;

rtcw::OglTessProgram* ogl_tess_program = NULL;

OglTessLayout ogl_tess2;
GLuint ogl_tess2_vbo = 0;
int ogl_tess2_base_vertex = 0;

bool ogl_tess_use_vao = false;
OglTessVaos ogl_tess_vaos;

rtcw::OglMatrixStack ogl_model_view_stack(rtcw::OglMatrixStack::model_view_max_depth);
rtcw::OglMatrixStack ogl_projection_stack(rtcw::OglMatrixStack::projection_max_depth);

rtcw::OglHdrProgram* ogl_hdr_program = NULL;
// BBi

glstate_t glState;

static void GfxInfo_f( void );

cvar_t  *r_flareSize;
cvar_t  *r_flareFade;

cvar_t  *r_railWidth;
cvar_t  *r_railCoreWidth;
cvar_t  *r_railSegmentLength;

cvar_t  *r_ignoreFastPath;

cvar_t  *r_verbose;
cvar_t  *r_ignore;

cvar_t  *r_displayRefresh;

cvar_t  *r_detailTextures;

cvar_t  *r_znear;
cvar_t  *r_zfar;

cvar_t  *r_smp;
cvar_t  *r_showSmp;
cvar_t  *r_skipBackEnd;

cvar_t  *r_ignorehwgamma;
cvar_t  *r_measureOverdraw;

cvar_t  *r_inGameVideo;
cvar_t  *r_fastsky;
cvar_t  *r_drawSun;
cvar_t  *r_dynamiclight;
cvar_t  *r_dlightBacks;

// BBi
//#if defined RTCW_SP
// BBi
cvar_t  *r_dlightScale; //----(SA)	added

cvar_t  *r_waterFogColor;   //----(SA)	added
cvar_t  *r_mapFogColor;
cvar_t  *r_savegameFogColor;    //----(SA)	added
// BBi
//#endif // RTCW_XX
// BBi

cvar_t  *r_lodbias;
cvar_t  *r_lodscale;

cvar_t  *r_norefresh;
cvar_t  *r_drawentities;
cvar_t  *r_drawworld;

// BBi
//#if defined RTCW_ET
// BBi
cvar_t  *r_drawfoliage;     // ydnar
// BBi
//#endif // RTCW_XX
// BBi

cvar_t  *r_speeds;
cvar_t  *r_fullbright; // JPW NERVE removed per atvi request
cvar_t  *r_novis;
cvar_t  *r_nocull;
cvar_t  *r_facePlaneCull;
cvar_t  *r_showcluster;
cvar_t  *r_nocurves;

cvar_t  *r_allowExtensions;

cvar_t  *r_ext_compressed_textures;
cvar_t  *r_ext_gamma_control;
cvar_t  *r_ext_multitexture;
cvar_t  *r_ext_compiled_vertex_array;
cvar_t  *r_ext_texture_env_add;

// BBi
//#if defined RTCW_ET
// BBi
cvar_t  *r_clampToEdge; // ydnar: opengl 1.2 GL_CLAMP_TO_EDGE SUPPORT
// BBi
//#endif // RTCW_XX
// BBi

//----(SA)	added
cvar_t  *r_ext_texture_filter_anisotropic;

cvar_t  *r_ext_NV_fog_dist;
cvar_t  *r_nv_fogdist_mode;

cvar_t  *r_ext_ATI_pntriangles;
cvar_t  *r_ati_truform_tess;        //
cvar_t  *r_ati_truform_normalmode;  // linear/quadratic
cvar_t  *r_ati_truform_pointmode;   // linear/cubic
//----(SA)	end

cvar_t  *r_ati_fsaa_samples;        //DAJ valids are 1, 2, 4

cvar_t  *r_ignoreGLErrors;
cvar_t  *r_logFile;

cvar_t  *r_stencilbits;
cvar_t  *r_depthbits;
cvar_t  *r_colorbits;
cvar_t  *r_stereo;
cvar_t  *r_primitives;
cvar_t  *r_texturebits;

cvar_t  *r_drawBuffer;
cvar_t  *r_glDriver;
cvar_t  *r_glIgnoreWicked3D;
cvar_t  *r_lightmap;

// BBi
//#if !defined RTCW_ET
// BBi
cvar_t  *r_vertexLight;
// BBi
//#endif // RTCW_XX
// BBi

cvar_t  *r_uiFullScreen;
cvar_t  *r_shadows;
cvar_t  *r_portalsky;   //----(SA)	added
cvar_t  *r_flares;
cvar_t  *r_mode;

// BBi
//#if defined RTCW_ET
// BBi
cvar_t  *r_oldMode;     // ydnar
// BBi
//#endif // RTCW_XX
// BBi

cvar_t  *r_nobind;
cvar_t  *r_singleShader;
cvar_t  *r_roundImagesDown;

// BBi
//#if defined RTCW_SP
// BBi
cvar_t  *r_lowMemTextureSize;
cvar_t  *r_lowMemTextureThreshold;
// BBi
//#endif // RTCW_XX
// BBi

cvar_t  *r_colorMipLevels;
cvar_t  *r_picmip;

// BBi
//#if defined RTCW_SP
// BBi
cvar_t  *r_picmip2;
// BBi
//#endif // RTCW_XX
// BBi

cvar_t  *r_showtris;

// BBi
//#if defined RTCW_ET
// BBi
cvar_t  *r_trisColor;
// BBi
//#endif // RTCW_XX
// BBi

cvar_t  *r_showsky;
cvar_t  *r_shownormals;

//#if defined RTCW_ET
cvar_t  *r_normallength;
cvar_t  *r_showmodelbounds;
//#endif // RTCW_XX

cvar_t  *r_finish;
cvar_t  *r_clear;
cvar_t  *r_swapInterval;
cvar_t  *r_textureMode;

// BBi
//#if defined RTCW_ET
// BBi

cvar_t  *r_textureAnisotropy;

// BBi
//#endif // RTCW_XX
// BBi

cvar_t  *r_offsetFactor;
cvar_t  *r_offsetUnits;
cvar_t  *r_gamma;
cvar_t  *r_intensity;
cvar_t  *r_lockpvs;
cvar_t  *r_noportals;
cvar_t  *r_portalOnly;

cvar_t  *r_subdivisions;
cvar_t  *r_lodCurveError;

cvar_t  *r_fullscreen;

cvar_t  *r_customwidth;
cvar_t  *r_customheight;
cvar_t  *r_customaspect;

cvar_t  *r_overBrightBits;
cvar_t  *r_mapOverBrightBits;

cvar_t  *r_debugSurface;
cvar_t  *r_simpleMipMaps;

cvar_t  *r_showImages;

cvar_t  *r_ambientScale;
cvar_t  *r_directedScale;
cvar_t  *r_debugLight;
cvar_t  *r_debugSort;
cvar_t  *r_printShaders;
cvar_t  *r_saveFontData;

// Ridah
cvar_t  *r_cache;
cvar_t  *r_cacheShaders;
cvar_t  *r_cacheModels;

// BBi
//#if !defined RTCW_ET
// BBi
cvar_t  *r_compressModels;
cvar_t  *r_exportCompressedModels;
// BBi
//#endif // RTCW_XX
// BBi

cvar_t  *r_cacheGathering;

cvar_t  *r_buildScript;

cvar_t  *r_bonesDebug;
// done.

// Rafael - wolf fog
cvar_t  *r_wolffog;
// done

cvar_t  *r_highQualityVideo;
cvar_t  *r_rmse;

cvar_t  *r_maxpolys;
int max_polys;
cvar_t  *r_maxpolyverts;
int max_polyverts;

// Controls the source of GLSL shaders.
// Non-zero - external; zero - embeded.
// Works in debug builds only.
cvar_t* r_dbg_use_glsl_shader_files;

/*
HDR support.

Values:
  - "0": Disables HDR support.
  - Non-zero integer: Enables HDR support.

Default value: "1"
*/
cvar_t* r_hdr;

/*
HDR detection mode.

Values:
  - "auto": Tries to detect if HDR enabled automatically.
  - "always": Assumes HDR is always enabled. Useful when auto-detection not implemented.

Default value: "auto"
*/
cvar_t* r_hdr_detection;

/*
HDR color component transfer function.

Values:
  - "srgb": sRGB (https://en.wikipedia.org/wiki/SRGB)
  - "gamma" A power function where the exponent specified by the appropriate cvar.

Default value: "srgb"
*/
cvar_t* r_hdr_cctf;

/*
An exponent for gamma HDR color component transfer function.
Formula: y = pow(x, gamma), where gamma is the exponent.

Range: [1.0 .. 3.0]
Default value: "2.2"
*/
cvar_t* r_hdr_cctf_gamma;

/*
User specified SDR white level relative to the baseline 80 nits.
Formula: sdr_white_level = sdr_white_level_in_nits / 80 nits

Range: [0.25 .. 6.25]
Default value: "1.0"
*/
cvar_t* r_hdr_sdr_white_level;

/*
Overrides auto-detection of SDR white level.
Useful when auto-detection not implemented.

Valid values:
- "0" - uses auto-detected value.
- "1" - uses a value from the appropriate cvar.
Default value: 0
*/
cvar_t* r_hdr_override_sdr_white_level;

vec4hack_t tess_xyz[SHADER_MAX_VERTEXES];
vec4hack_t tess_normal[SHADER_MAX_VERTEXES];
vec2hack_t tess_texCoords0[SHADER_MAX_VERTEXES];
vec2hack_t tess_texCoords1[SHADER_MAX_VERTEXES];
glIndex_t tess_indexes[SHADER_MAX_INDEXES];
color4ubhack_t tess_vertexColors[SHADER_MAX_VERTEXES];

#ifndef RTCW_ET
int tess_vertexDlightBits[SHADER_MAX_VERTEXES];
#endif // RTCW_X

// BBi
//void ( APIENTRY * qglMultiTexCoord2fARB )( GLenum texture, GLfloat s, GLfloat t );
//void ( APIENTRY * qglActiveTextureARB )( GLenum texture );
//void ( APIENTRY * qglClientActiveTextureARB )( GLenum texture );
//
//void ( APIENTRY * qglLockArraysEXT )( GLint, GLint );
//void ( APIENTRY * qglUnlockArraysEXT )( void );
//
////----(SA)	added
//void ( APIENTRY * qglPNTrianglesiATI )( GLenum pname, GLint param );
//void ( APIENTRY * qglPNTrianglesfATI )( GLenum pname, GLfloat param );
// BBi

/*
The tessellation level and normal generation mode are specified with:

	void qglPNTriangles{if}ATI(enum pname, T param)

	If <pname> is:
		GL_PN_TRIANGLES_NORMAL_MODE_ATI -
			<param> must be one of the symbolic constants:
				- GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI or
				- GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI
			which will select linear or quadratic normal interpolation respectively.
		GL_PN_TRIANGLES_POINT_MODE_ATI -
			<param> must be one of the symbolic  constants:
				- GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI or
				- GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI
			which will select linear or cubic interpolation respectively.
		GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI -
			<param> should be a value specifying the number of evaluation points on each edge.  This value must be
			greater than 0 and less than or equal to the value given by GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI.

	An INVALID_VALUE error will be generated if the value for <param> is less than zero or greater than the max value.

Associated 'gets':
Get Value                               Get Command Type     Minimum Value								Attribute
---------                               ----------- ----     ------------								---------
PN_TRIANGLES_ATI						IsEnabled   B		False                                       PN Triangles/enable
PN_TRIANGLES_NORMAL_MODE_ATI			GetIntegerv Z2		PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI		PN Triangles
PN_TRIANGLES_POINT_MODE_ATI				GetIntegerv Z2		PN_TRIANGLES_POINT_MODE_CUBIC_ATI			PN Triangles
PN_TRIANGLES_TESSELATION_LEVEL_ATI		GetIntegerv Z+		1											PN Triangles
MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI	GetIntegerv Z+		1											-




*/
//----(SA)	end


static void AssertCvarRange( cvar_t *cv, float minVal, float maxVal, qboolean shouldBeIntegral ) {
	if ( shouldBeIntegral ) {
		if ( ( int ) cv->value != cv->integer ) {
			ri.Printf( PRINT_WARNING, "WARNING: cvar '%s' must be integral (%f)\n", cv->name, cv->value );
			ri.Cvar_Set( cv->name, va( "%d", cv->integer ) );
		}
	}

	if ( cv->value < minVal ) {
		ri.Printf( PRINT_WARNING, "WARNING: cvar '%s' out of range (%f < %f)\n", cv->name, cv->value, minVal );
		ri.Cvar_Set( cv->name, va( "%f", minVal ) );
	} else if ( cv->value > maxVal )   {
		ri.Printf( PRINT_WARNING, "WARNING: cvar '%s' out of range (%f > %f)\n", cv->name, cv->value, maxVal );
		ri.Cvar_Set( cv->name, va( "%f", maxVal ) );
	}
}

// BBi
static int R_GetMaxTextureSize ()
{
	int result = 0;

	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &result);

	if (result > 0) {
		while (true) {
			glTexImage2D (
				GL_PROXY_TEXTURE_2D,
				0,
				GL_RGBA,
				result,
				result,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				NULL);


			int width = 0;

			glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);

			if (width > 0)
				break;
			else
				result /= 2;
		}
	}

	return result;
}

void r_invalidate_hdr_cvars()
{
	r_hdr->modified = true;
	r_hdr_detection->modified = true;
	r_hdr_cctf->modified = true;
	r_hdr_cctf_gamma->modified = true;
	r_hdr_sdr_white_level->modified = true;
	r_hdr_override_sdr_white_level->modified = true;
}

rtcw::String r_dbg_get_glsl_path()
{
	if (glConfigEx.is_path_ogl_2_x())
	{
		return "glsl/110/";
	}
	else
	{
		return rtcw::String();
	}
}

bool r_dbg_probe_programs()
{
	bool is_try_successfull = true;

	const rtcw::String glsl_dir = r_dbg_get_glsl_path();

	if (glsl_dir.empty())
	{
		ri.Printf(PRINT_WARNING, "Invalid OpenGL path value.\n");
		return false;
	}

	rtcw::OglTessProgram tess_program(glsl_dir, "tess");

	ri.Printf(PRINT_ALL, "\n======== GLSL probe (debug) ========\n");
	ri.Printf(PRINT_ALL, "%s...\n", "Trying to reload programs");

	is_try_successfull &= tess_program.try_reload();

	ri.Printf(PRINT_ALL, "======== GLSL probe (debug) ========\n");

	return is_try_successfull;
}

void r_dbg_reload_programs_f()
{
	if (glConfigEx.is_path_ogl_1_x())
	{
		ri.Printf(PRINT_WARNING, "Expected OpenGL v2.0+.\n");
		return;
	}

	ri.Printf(PRINT_ALL, "\n======== GLSL (debug) ========\n");
	ri.Printf(PRINT_ALL, "Reload the programs...\n");

	const rtcw::String glsl_dir = r_dbg_get_glsl_path();

	if (glsl_dir.empty())
	{
		ri.Printf(PRINT_WARNING, "Unspecified GLSL directory.\n");
		return;
	}

	bool is_try_successfull = true;
	ogl_tess_state.set_program(NULL);

	if (ogl_tess_program == NULL)
	{
		ogl_tess_program = new rtcw::OglTessProgram(glsl_dir, "tess");
	}

	if (ogl_hdr_program == NULL)
	{
		ogl_hdr_program = new rtcw::OglHdrProgram(glsl_dir, "hdr");
	}

	if (ogl_tess_program != NULL && ogl_hdr_program != NULL)
	{
		is_try_successfull &= ogl_tess_program->try_reload();
		is_try_successfull &= ogl_hdr_program->try_reload();
	}
	else
	{
		is_try_successfull = false;
		ri.Printf(PRINT_ALL, "Out of memory.\n");
	}

	if (is_try_successfull)
	{
		ogl_tess_program->reload();
		ogl_hdr_program->reload();
	}

	ogl_tess_state.set_program(ogl_tess_program);
	r_invalidate_hdr_cvars();

	ri.Printf(PRINT_ALL, "======== GLSL (debug) ========\n");
}


static const char* r_get_embeded_tess_vertex_shader()
{
	static const char* const result =
		"//\n"
		"// Project: RTCW\n"
		"// Author: Boris I. Bendovsky\n"
		"//\n"
		"// Shader type: vertex.\n"
		"// Purpose: Generic drawing.\n"
		"//\n"
		"\n"
		"#version 110\n"
		"\n"
		"// Known GL constants.\n"
		"const int GL_DONT_CARE = 0x1100;\n"
		"const int GL_EXP = 0x0800;\n"
		"const int GL_FASTEST = 0x1101;\n"
		"const int GL_NICEST = 0x1102;\n"
		"const int GL_NONE = 0x0000;\n"
		"const int GL_EYE_PLANE = 0x2502;\n"
		"const int GL_EYE_RADIAL_NV = 0x855B;\n"
		"\n"
		"attribute vec4 pos_vec4; // position\n"
		"attribute vec4 col_vec4; // color\n"
		"attribute vec2 tc0_vec2; // texture coords (0)\n"
		"attribute vec2 tc1_vec2; // texture coords (1)\n"
		"\n"
		"uniform bool use_fog;\n"
		"uniform int fog_mode;\n"
		"uniform int fog_dist_mode; // GL_NV_fog_distance emulation\n"
		"uniform int fog_hint;\n"
		"\n"
		"uniform mat4 projection_mat4; // projection matrix\n"
		"uniform mat4 model_view_mat4; // model-view matrix\n"
		"\n"
		"varying vec4 col; // interpolated color\n"
		"varying vec2 tc[2]; // interpolated texture coords\n"
		"varying float fog_vc; // interpolated calculated fog coords\n"
		"varying vec4 fog_fc; // interpolated fog coords\n"
		"\n"
		"void main()\n"
		"{\n"
		"    col = col_vec4;\n"
		"    tc[0] = tc0_vec2;\n"
		"    tc[1] = tc1_vec2;\n"
		"\n"
		"    vec4 eye_pos = model_view_mat4 * pos_vec4;\n"
		"\n"
		"    if (use_fog)\n"
		"    {\n"
		"        if (fog_hint != GL_FASTEST)\n"
		"        {\n"
		"            fog_fc = eye_pos;\n"
		"        }\n"
		"        else\n"
		"        {\n"
		"            if (fog_dist_mode == GL_EYE_RADIAL_NV)\n"
		"            {\n"
		"                fog_vc = length(eye_pos.xyz);\n"
		"            }\n"
		"            else if (fog_dist_mode == GL_EYE_PLANE)\n"
		"            {\n"
		"                fog_vc = eye_pos.z;\n"
		"            }\n"
		"            else\n"
		"            {\n"
		"                fog_vc = abs(eye_pos.z);\n"
		"            }\n"
		"        }\n"
		"    }\n"
		"\n"
		"    gl_Position = projection_mat4 * eye_pos;\n"
		"}\n"
	;

	return result;
}

static const char* r_get_embeded_tess_fragment_shader()
{
	static const char* const result =
		"//\n"
		"// Project: RTCW\n"
		"// Author: Boris I. Bendovsky\n"
		"//\n"
		"// Shader type: fragment.\n"
		"// Purpose: Generic drawing.\n"
		"//\n"
		"\n"
		"#version 110\n"
		"\n"
		"// Known constants.\n"
		"const int GL_ADD = 0x0104;\n"
		"const int GL_DECAL = 0x2101;\n"
		"const int GL_DONT_CARE = 0x1100;\n"
		"const int GL_EYE_PLANE = 0x2502;\n"
		"const int GL_EYE_RADIAL_NV = 0x855B;\n"
		"const int GL_EXP = 0x0800;\n"
		"const int GL_FASTEST = 0x1101;\n"
		"const int GL_GEQUAL = 0x0206;\n"
		"const int GL_GREATER = 0x0204;\n"
		"const int GL_LESS = 0x0201;\n"
		"const int GL_LINEAR = 0x2601;\n"
		"const int GL_MODULATE = 0x2100;\n"
		"const int GL_NICEST = 0x1102;\n"
		"const int GL_REPLACE = 0x1E01;\n"
		"\n"
		"uniform vec4 primary_color; // primary color\n"
		"uniform bool use_alpha_test; // alpha test switch\n"
		"uniform int alpha_test_func; // alpha test function\n"
		"uniform float alpha_test_ref; // alpha test reference value\n"
		"uniform int tex_env_mode[2]; // texture environment mode\n"
		"uniform bool use_multitexturing; // mutitexturing switch\n"
		"uniform sampler2D tex_2d[2]; // textures\n"
		"\n"
		"uniform bool use_fog;\n"
		"uniform int fog_mode;\n"
		"uniform int fog_hint;\n"
		"uniform int fog_dist_mode; // GL_NV_fog_distance emulation\n"
		"uniform vec4 fog_color;\n"
		"uniform float fog_density;\n"
		"uniform float fog_start;\n"
		"uniform float fog_end;\n"
		"\n"
		"uniform float intensity;\n"
		"uniform float overbright;\n"
		"uniform float gamma;\n"
		"\n"
		"varying vec4 col; // interpolated color\n"
		"varying vec2 tc[2]; // interpolated texture coords\n"
		"varying float fog_vc; // interpolated calculated fog coords\n"
		"varying vec4 fog_fc; // interpolated fog coords\n"
		"\n"
		"vec4 apply_intensity(vec4 value)\n"
		"{\n"
		"    return vec4(clamp(value.rgb * intensity, vec3(0.0), vec3(1.0)), value.a);\n"
		"}\n"
		"\n"
		"vec4 apply_gamma(vec4 value)\n"
		"{\n"
		"    return vec4(pow(value.rgb, vec3(1.0 / (overbright * gamma))), value.a);\n"
		"}\n"
		"\n"
		"vec4 apply_tex_env(\n"
		"    vec4 previous_color,\n"
		"    int env_index)\n"
		"{\n"
		"    vec2 texel_tc = tc[env_index];\n"
		"    vec4 texel;\n"
		"\n"
		"    if (env_index == 0)\n"
		"    {\n"
		"        texel = texture2D(tex_2d[0], texel_tc);\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        texel = texture2D(tex_2d[1], texel_tc);\n"
		"    }\n"
		"\n"
		"    texel = apply_intensity(texel);\n"
		"    vec4 result = previous_color;\n"
		"\n"
		"    if (tex_env_mode[env_index] == GL_REPLACE)\n"
		"    {\n"
		"        result = texel;\n"
		"    }\n"
		"    else if (tex_env_mode[env_index] == GL_MODULATE)\n"
		"    {\n"
		"        result *= texel;\n"
		"    }\n"
		"    else if (tex_env_mode[env_index] == GL_DECAL)\n"
		"    {\n"
		"        result.rgb = mix(result.rgb, texel.rgb, texel.a);\n"
		"    }\n"
		"    else if (tex_env_mode[env_index] == GL_ADD)\n"
		"    {\n"
		"        result.rgb += texel.rgb;\n"
		"        result.a *= texel.a;\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        // invalid mode\n"
		"        result *= vec4(0.5, 0.0, 0.0, 1.0);\n"
		"    }\n"
		"\n"
		"    return result;\n"
		"}\n"
		"\n"
		"vec4 apply_alpha_test(\n"
		"    vec4 color)\n"
		"{\n"
		"    float test_ref = clamp(alpha_test_ref, 0.0, 1.0);\n"
		"\n"
		"    if (alpha_test_func == GL_GEQUAL)\n"
		"    {\n"
		"        if (color.a < test_ref)\n"
		"        {\n"
		"            discard;\n"
		"        }\n"
		"    }\n"
		"    else if (alpha_test_func == GL_GREATER)\n"
		"    {\n"
		"        if (color.a <= test_ref)\n"
		"        {\n"
		"            discard;\n"
		"        }\n"
		"    }\n"
		"    else if (alpha_test_func == GL_LESS)\n"
		"    {\n"
		"        if (color.a >= test_ref)\n"
		"        {\n"
		"            discard;\n"
		"        }\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        // invalid function\n"
		"        color *= vec4(0.0, 0.5, 0.0, 1.0);\n"
		"    }\n"
		"\n"
		"    return color;\n"
		"}\n"
		"\n"
		"vec4 apply_fog(\n"
		"    vec4 color)\n"
		"{\n"
		"    float c;\n"
		"\n"
		"    if (fog_hint != GL_FASTEST)\n"
		"    {\n"
		"        vec4 r_fog_fc = fog_fc / fog_fc.w;\n"
		"\n"
		"        if (fog_dist_mode == GL_EYE_RADIAL_NV)\n"
		"        {\n"
		"            c = length(r_fog_fc.xyz);\n"
		"        }\n"
		"        else if (fog_dist_mode == GL_EYE_PLANE)\n"
		"        {\n"
		"            c = fog_fc.z;\n"
		"        }\n"
		"        else\n"
		"        {\n"
		"            c = abs(fog_fc.z);\n"
		"        }\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        c = fog_vc;\n"
		"    }\n"
		"\n"
		"\n"
		"    float f = 1.0;\n"
		"\n"
		"    if (fog_mode == GL_LINEAR)\n"
		"    {\n"
		"        float es = fog_end - fog_start;\n"
		"\n"
		"        if (es != 0.0)\n"
		"        {\n"
		"            f = (fog_end - c) / es;\n"
		"        }\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        f = exp(-fog_density * c);\n"
		"    }\n"
		"\n"
		"    f = clamp(f, 0.0, 1.0);\n"
		"    vec4 mixed_color = mix(fog_color, color, f);\n"
		"\n"
		"    return vec4(mixed_color.rgb, color.a);\n"
		"}\n"
		"\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 frag_color = primary_color * col;\n"
		"\n"
		"    frag_color = apply_tex_env(frag_color, 0);\n"
		"\n"
		"    if (use_multitexturing)\n"
		"    {\n"
		"        frag_color = apply_tex_env(frag_color, 1);\n"
		"    }\n"
		"\n"
		"    if (use_fog)\n"
		"    {\n"
		"        frag_color = apply_fog(frag_color);\n"
		"    }\n"
		"\n"
		"    if (use_alpha_test)\n"
		"    {\n"
		"        frag_color = apply_alpha_test(frag_color);\n"
		"    }\n"
		"\n"
		"    frag_color = apply_gamma(frag_color);\n"
		"\n"
		"    gl_FragColor = frag_color;\n"
		"}\n"
		"\n"
	;

	return result;
}

namespace {

static const char* r_get_embeded_hdr_vertex_shader()
{
	static const char* const result =
		"//\n"
		"// Project: RTCW\n"
		"// Author: Boris I. Bendovsky\n"
		"//\n"
		"// Shader type: vertex\n"
		"// Purpose: HDR\n"
		"//\n"
		"\n"
		"#version 110\n"
		"\n"
		"attribute vec2 pos_vec2; // position\n"
		"attribute vec2 tc_vec2; // texture coord\n"
		"\n"
		"varying vec2 tc; // interpolated texture coord\n"
		"\n"
		"void main()\n"
		"{\n"
		"    tc = tc_vec2;\n"
		"    gl_Position = vec4(pos_vec2, 0.0, 1.0);\n"
		"}\n"
	;

	return result;
}

static const char* r_get_embeded_hdr_fragment_shader()
{
	static const char* const result =
		"//\n"
		"// Project: RTCW\n"
		"// Author: Boris I. Bendovsky\n"
		"//\n"
		"// Shader type: fragment\n"
		"// Purpose: HDR\n"
		"//\n"
		"\n"
		"#version 110\n"
		"\n"
		"const int cctf_id_srgb = 1; // sRGB\n"
		"const int cctf_id_gamma = 2; // pow(x, gamma)\n"
		"\n"
		"uniform sampler2D tex_2d; // texture\n"
		"uniform int cctf_id; // identifier of color component transfer function\n"
		"uniform float cctf_gamma; // an exponent of gamma CCTF\n"
		"uniform float sdr_white_level; // Relative SDR white level to 80 nits\n"
		"\n"
		"varying vec2 tc; // interpolated texture coord\n"
		"\n"
		"vec3 linear_to_srgb(vec3 x)\n"
		"{\n"
		"    // https://en.wikipedia.org/wiki/SRGB\n"
		"    return mix(pow((x + 0.055) / 1.055, vec3(2.4)), x / 12.92, vec3(lessThanEqual(x, vec3(0.04045))));\n"
		"}\n"
		"\n"
		"vec3 linear_to_gamma(vec3 x, float power)\n"
		"{\n"
		"    return pow(x, vec3(power));\n"
		"}\n"
		"\n"
		"vec3 apply_cctf(vec3 x)\n"
		"{\n"
		"    if (cctf_id == cctf_id_srgb)\n"
		"    {\n"
		"        return linear_to_srgb(x);\n"
		"    }\n"
		"    else if (cctf_id == cctf_id_gamma)\n"
		"    {\n"
		"        return linear_to_gamma(x, cctf_gamma);\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        return x;\n"
		"    }\n"
		"}\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 texel = texture2D(tex_2d, tc);\n"
		"    gl_FragColor = vec4(apply_cctf(texel.rgb) * sdr_white_level, texel.a);\n"
		"}\n"
	;

	return result;
}

} // namespace

bool r_probe_programs()
{
#ifdef _DEBUG
	if (r_dbg_use_glsl_shader_files->integer != 0)
	{
		return r_dbg_probe_programs();
	}
#endif // _DEBUG

	if (!glConfigEx.is_path_ogl_2_x())
	{
		ri.Printf(PRINT_WARNING, "No OpenGL 2.0+.\n");
		return false;
	}

	bool is_try_successfull = true;

	rtcw::OglTessProgram tess_program(r_get_embeded_tess_vertex_shader(), r_get_embeded_tess_fragment_shader());
	rtcw::OglHdrProgram hdr_program(r_get_embeded_hdr_vertex_shader(), r_get_embeded_hdr_fragment_shader());

	ri.Printf(PRINT_ALL, "\n======== GLSL probe ========\n");
	ri.Printf(PRINT_ALL, "%s...\n", "Trying to reload programs");

	is_try_successfull &= tess_program.try_reload();
	is_try_successfull &= hdr_program.try_reload();

	ri.Printf(PRINT_ALL, "======== GLSL probe ========\n");

	return is_try_successfull;
}

void r_reload_programs_f()
{
#ifdef _DEBUG
	if (r_dbg_use_glsl_shader_files->integer != 0)
	{
		r_dbg_reload_programs_f();
		return;
	}
#endif // _DEBUG

	if (glConfigEx.is_path_ogl_1_x())
	{
		ri.Printf(PRINT_WARNING, "Expected OpenGL v2.0+.\n");
		return;
	}

	ri.Printf(PRINT_ALL, "\n======== GLSL ========\n");
	ri.Printf(PRINT_ALL, "Reload the programs...\n");

	bool is_try_successfull = true;

	ogl_tess_state.set_program(NULL);

	if (ogl_tess_program == NULL)
	{
		ogl_tess_program = new rtcw::OglTessProgram(r_get_embeded_tess_vertex_shader(), r_get_embeded_tess_fragment_shader());
	}

	if (ogl_hdr_program == NULL)
	{
		ogl_hdr_program = new rtcw::OglHdrProgram(
			r_get_embeded_hdr_vertex_shader(),
			r_get_embeded_hdr_fragment_shader());
	}

	if (ogl_tess_program != NULL && ogl_hdr_program != NULL)
	{
		is_try_successfull &= ogl_tess_program->try_reload();
		is_try_successfull &= ogl_hdr_program->try_reload();
	}
	else
	{
		is_try_successfull = false;
		ri.Printf(PRINT_ALL, "Out of memory.\n");
	}

	if (is_try_successfull)
	{
		ogl_tess_program->reload();
		ogl_hdr_program->reload();
	}

	ogl_tess_state.set_program(ogl_tess_program);
	r_invalidate_hdr_cvars();

	ri.Printf(PRINT_ALL, "======== GLSL ========\n");
}

namespace {

void r_destroy_tess_vertex_array_objects()
{
	glBindVertexArray(0);
	glDeleteVertexArrays(ogl_tess_vao_total_count, ogl_tess_vaos);
	std::fill_n(ogl_tess_vaos, ogl_tess_vao_total_count, GLuint());
}

bool r_create_tess_vertex_array_objects()
{
	std::fill_n(ogl_tess_vaos, ogl_tess_vao_total_count, GLuint());
	glGenVertexArrays(ogl_tess_vao_total_count, ogl_tess_vaos);
	bool failed = false;

	for (int i_vao = 0; i_vao < ogl_tess_vao_total_count; ++i_vao)
	{
		const GLuint gl_vao = ogl_tess_vaos[i_vao];
		glBindVertexArray(gl_vao);

		if (!glIsVertexArray(gl_vao))
		{
			failed = true;
			break;
		}
	}

	if (failed)
	{
		r_destroy_tess_vertex_array_objects();
		ri.Printf(
			PRINT_ALL,
			"%sFailed to create GL vertex array objects.%s\n",
			S_COLOR_YELLOW,
			S_COLOR_WHITE);
		return false;
	}

	glBindVertexArray(0);
	return true;
}

void r_initialize_tess_vertex_array_objects()
{
	for (int permutation = 1; permutation <= ogl_tess_vao_count; ++permutation)
	{
		const bool use_tc0_array = (permutation & (1 << 0)) != 0;
		const bool use_tc1_array = (permutation & (1 << 1)) != 0;
		const bool use_col_array = (permutation & (1 << 2)) != 0;
		assert(use_tc0_array || use_tc1_array || use_col_array);
		const int vao_index = ogl_tess_vao_base_index + permutation - 1;
		GLuint& gl_vao = ogl_tess_vaos[vao_index];

		glBindVertexArray(gl_vao);
		glBindBuffer(GL_ARRAY_BUFFER, ogl_tess_vbo);

		// position
		glVertexAttribPointer(
			/* index */      ogl_tess_program->a_pos_vec4,
			/* size */       3,
			/* type */       GL_FLOAT,
			/* normalized */ GL_FALSE,
			/* stride */     static_cast<GLsizei>(OglTessLayout::POS_SIZE),
			/* pointer */    OglTessLayout::POS_PTR);

		glEnableVertexAttribArray(ogl_tess_program->a_pos_vec4);

		// texture coordinates (0)
		if (use_tc0_array)
		{
			glVertexAttribPointer(
				/* index */      ogl_tess_program->a_tc0_vec2,
				/* size */       2,
				/* type */       GL_FLOAT,
				/* normalized */ GL_FALSE,
				/* stride */     0,
				/* pointer */    OglTessLayout::TC0_PTR);

			glEnableVertexAttribArray(ogl_tess_program->a_tc0_vec2);
		}

		// texture coordinates (1)
		if (use_tc1_array)
		{
			glVertexAttribPointer(
				/* index */      ogl_tess_program->a_tc1_vec2,
				/* size */       2,
				/* type */       GL_FLOAT,
				/* normalized */ GL_FALSE,
				/* stride */     0,
				/* pointer */    OglTessLayout::TC1_PTR);

			glEnableVertexAttribArray(ogl_tess_program->a_tc1_vec2);
		}

		// color
		if (use_col_array)
		{
			glVertexAttribPointer(
				/* index */      ogl_tess_program->a_col_vec4,
				/* size */       4,
				/* type */       GL_UNSIGNED_BYTE,
				/* normalized */ GL_TRUE,
				/* stride */     0,
				/* pointer */    OglTessLayout::COL_PTR);

			glEnableVertexAttribArray(ogl_tess_program->a_col_vec4);
		}
	}

	glBindVertexArray(0);
}

void r_initialize_tess2_vertex_array_objects()
{
	for (int permutation = 1; permutation <= ogl_tess2_vao_count; ++permutation)
	{
		const bool use_tc0_array = (permutation & (1 << 0)) != 0;
		const bool use_col_array = (permutation & (1 << 1)) != 0;
		assert(use_tc0_array || use_col_array);
		const int vao_index = ogl_tess2_vao_base_index + permutation - 1;
		const GLuint gl_vao = ogl_tess_vaos[vao_index];

		glBindVertexArray(gl_vao);
		glBindBuffer(GL_ARRAY_BUFFER, ogl_tess2_vbo);

		// position
		glVertexAttribPointer(
			/* index */      ogl_tess_program->a_pos_vec4,
			/* size */       3,
			/* type */       GL_FLOAT,
			/* normalized */ GL_FALSE,
			/* stride */     static_cast<GLsizei>(OglTessLayout::POS_SIZE),
			/* pointer */    OglTessLayout::POS_PTR);

		glEnableVertexAttribArray(ogl_tess_program->a_pos_vec4);

		// texture coordinates (0)
		if (use_tc0_array)
		{
			glVertexAttribPointer(
				/* index */      ogl_tess_program->a_tc0_vec2,
				/* size */       2,
				/* type */       GL_FLOAT,
				/* normalized */ GL_FALSE,
				/* stride */     0,
				/* pointer */    OglTessLayout::TC0_PTR);

			glEnableVertexAttribArray(ogl_tess_program->a_tc0_vec2);
		}

		// color
		if (use_col_array)
		{
			glVertexAttribPointer(
				/* index */      ogl_tess_program->a_col_vec4,
				/* size */       4,
				/* type */       GL_UNSIGNED_BYTE,
				/* normalized */ GL_TRUE,
				/* stride */     0,
				/* pointer */    OglTessLayout::COL_PTR);

			glEnableVertexAttribArray(ogl_tess_program->a_col_vec4);
		}
	}

	glBindVertexArray(0);
}

void r_initialize_tess_default_vertex_array_object()
{
	const GLuint gl_vao = ogl_tess_vaos[ogl_tess_default_vao_index];
	glBindVertexArray(gl_vao);
}

} // namespace

static void r_tess_initialize ()
{
	GLsizeiptr vbo_size = sizeof (OglTessLayout);

	glGenBuffers (1, &ogl_tess_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, ogl_tess_vbo);
	glBufferData (GL_ARRAY_BUFFER, vbo_size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, 0);

	glGenBuffers (1, &ogl_tess2_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, ogl_tess2_vbo);
	glBufferData (GL_ARRAY_BUFFER, vbo_size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, 0);

	ogl_tess_base_vertex = 0;
	ogl_tess2_base_vertex = 0;

	ogl_tess_use_vao = false;

	if (glConfigEx.use_gl_arb_vertex_array_object)
	{
		if (r_create_tess_vertex_array_objects())
		{
			r_initialize_tess_vertex_array_objects();
			r_initialize_tess2_vertex_array_objects();
			r_initialize_tess_default_vertex_array_object();

			ogl_tess_use_vao = true;
		}
	}
}

static void r_tess_uninitialize ()
{
	if (ogl_tess_use_vao)
	{
		r_destroy_tess_vertex_array_objects();
	}

	if (glDeleteBuffers != NULL)
	{
		glDeleteBuffers (1, &ogl_tess_vbo);
	}

	ogl_tess_vbo = 0;

	if (glDeleteBuffers != NULL)
	{
		glDeleteBuffers (1, &ogl_tess2_vbo);
	}

	ogl_tess2_vbo = 0;
}

namespace {

GLuint r_offscreen_color_target = 0;
GLuint r_offscreen_depth_stencil_target = 0;
GLuint r_offscreen_framebuffer = 0;
GLuint r_offscreen_vbo = 0;
GLuint r_offscreen_vao = 0;
float r_offscreen_sdr_white_level = 1.0F;

struct ROffscreenVertex
{
	float pos[2];
	float tc[2];
};

void r_initialize_rgba_mode()
{
	glConfigEx.is_default_framebuffer_float = false;

	if (glConfigEx.use_gl_arb_color_buffer_float)
	{
		GLboolean gl_rgba_float_mode_arb = GL_FALSE;
		glGetBooleanv(GL_RGBA_FLOAT_MODE_ARB, &gl_rgba_float_mode_arb);
		glConfigEx.is_default_framebuffer_float = gl_rgba_float_mode_arb == GL_TRUE;
	}

	ri.Printf(PRINT_ALL, "Floating-point color format of swapchain: %s\n",
		glConfigEx.is_default_framebuffer_float ? "yes" : "no");
}

GLuint r_create_offscreen_color_gl_texture()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	GLuint gl_texture = 0;
	glGenTextures(1, &gl_texture);
	glBindTexture(GL_TEXTURE_2D, gl_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glTexImage2D(
		/* target */         GL_TEXTURE_2D,
		/* level */          0,
		/* internalformat */ GL_RGBA8,
		/* width */          glConfig.vidWidth,
		/* height */         glConfig.vidHeight,
		/* border */         0,
		/* format */         GL_RGBA,
		/* type */           GL_UNSIGNED_BYTE,
		/* pixels */         NULL
	);

	if (!glIsTexture(gl_texture))
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &gl_texture);
		return 0;
	}

	return gl_texture;
}

bool r_create_offscreen_color_texture_target()
{
	r_offscreen_color_target = r_create_offscreen_color_gl_texture();

	if (r_offscreen_color_target == 0)
	{
		return false;
	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, r_offscreen_color_target);
	glActiveTexture(GL_TEXTURE0);
	return true;
}

bool r_create_offscreen_depth_stencil_renderbuffer_target()
{
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	r_offscreen_depth_stencil_target = 0;
	glGenRenderbuffers(1, &r_offscreen_depth_stencil_target);
	glBindRenderbuffer(GL_RENDERBUFFER, r_offscreen_depth_stencil_target);

	if (!glIsRenderbuffer(r_offscreen_depth_stencil_target))
	{
		return false;
	}

	glRenderbufferStorage(
		/* target */         GL_RENDERBUFFER,
		/* internalformat */ GL_DEPTH24_STENCIL8,
		/* width */          glConfig.vidWidth,
		/* height */         glConfig.vidHeight
	);

	return true;
}

bool r_create_offscreen_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	r_offscreen_framebuffer = 0;
	glGenFramebuffers(1, &r_offscreen_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, r_offscreen_framebuffer);

	if (!glIsFramebuffer(r_offscreen_framebuffer))
	{
		return false;
	}

	glFramebufferTexture2D(
		/* target */     GL_FRAMEBUFFER,
		/* attachment */ GL_COLOR_ATTACHMENT0,
		/* textarget */  GL_TEXTURE_2D,
		/* texture */    r_offscreen_color_target,
		/* level */      0
	);

	glFramebufferRenderbuffer(
		/* target */             GL_FRAMEBUFFER,
		/* attachment */         GL_DEPTH_STENCIL_ATTACHMENT,
		/* renderbuffertarget */ GL_RENDERBUFFER,
		/* renderbuffer */       r_offscreen_depth_stencil_target
	);

	const GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
	{
		r_offscreen_framebuffer = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &r_offscreen_framebuffer);
		return false;
	}

	return true;
}

bool r_create_offscreen_vbo()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	r_offscreen_vbo = 0;
	glGenBuffers(1, &r_offscreen_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, r_offscreen_vbo);

	if (!glIsBuffer(r_offscreen_vbo))
	{
		return false;
	}

	const ROffscreenVertex vertices[4] =
	{
		// left-top
		{{-1.0F,  1.0F}, {0.0F, 1.0F}},
		// left-bottom
		{{-1.0F, -1.0F}, {0.0F, 0.0F}},
		// right-top
		{{ 1.0F,  1.0F}, {1.0F, 1.0F}},
		// right-bottom
		{{ 1.0F, -1.0F}, {1.0F, 0.0F}},
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	return true;
}

bool r_create_offscreen_vao()
{
	glBindVertexArray(0);
	r_offscreen_vao = 0;
	glGenVertexArrays(1, &r_offscreen_vao);
	glBindVertexArray(r_offscreen_vao);

	if (!glIsVertexArray(r_offscreen_vao))
	{
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, r_offscreen_vbo);

	// position
	//
	glVertexAttribPointer(
		/* index */      ogl_hdr_program->a_pos_vec2,
		/* size */       2,
		/* type */       GL_FLOAT,
		/* normalized */ GL_FALSE,
		/* stride */     sizeof(ROffscreenVertex),
		/* pointer */    reinterpret_cast<const void*>(offsetof(ROffscreenVertex, pos)));

	glEnableVertexAttribArray(ogl_hdr_program->a_pos_vec2);

	// texture coordinates
	//
	glVertexAttribPointer(
		/* index */      ogl_hdr_program->a_tc_vec2,
		/* size */       2,
		/* type */       GL_FLOAT,
		/* normalized */ GL_FALSE,
		/* stride */     sizeof(ROffscreenVertex),
		/* pointer */    reinterpret_cast<const void*>(offsetof(ROffscreenVertex, tc)));

	glEnableVertexAttribArray(ogl_hdr_program->a_tc_vec2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

void r_terminate_offscreen()
{
	glConfigEx.has_offscreen = false;

	if (r_offscreen_framebuffer != 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &r_offscreen_framebuffer);
		r_offscreen_framebuffer = 0;
	}

	if (r_offscreen_depth_stencil_target != 0)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glDeleteRenderbuffers(1, &r_offscreen_depth_stencil_target);
		r_offscreen_depth_stencil_target = 0;
	}

	if (r_offscreen_color_target != 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &r_offscreen_color_target);
		r_offscreen_color_target = 0;
	}

	if (r_offscreen_vbo != 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &r_offscreen_vbo);
		r_offscreen_vbo = 0;
	}

	if (r_offscreen_vao != 0)
	{
		if (glConfigEx.use_gl_arb_vertex_array_object)
		{
			glBindVertexArray(0);
			glDeleteVertexArrays(1, &r_offscreen_vao);
		}

		r_offscreen_vao = 0;
	}
}

bool r_update_offscreen_sdr_white_level()
{
	r_offscreen_sdr_white_level = r_hdr_mgr_uptr->get_sdr_white_level_float();
	return true;
}

void r_initialize_offscreen()
{
	glConfigEx.has_offscreen = false;

	if (
		glConfig.maxActiveTextures <= 2 ||
		!glConfigEx.is_2_x_capable_ ||
		!glConfigEx.use_arb_framebuffer_object_ ||
		!glConfigEx.use_arb_texture_non_power_of_two_)
	{
		return;
	}

	const bool is_hdr_allowed = r_hdr->integer != 0;
	const bool is_hdr_always_detected = strcmp(r_hdr_detection->string, "always") == 0;
	const bool is_hdr_auto_detected = strcmp(r_hdr_detection->string, "auto") == 0;
	const bool is_hdr_unknown_detected = !is_hdr_always_detected && !is_hdr_auto_detected;
	const bool is_hdr_cctf_srgb = strcmp(r_hdr_cctf->string, "srgb") == 0;
	const bool is_hdr_cctf_gamma = strcmp(r_hdr_cctf->string, "gamma") == 0;
	const bool is_hdr_cctf_unknown = !is_hdr_cctf_srgb && !is_hdr_cctf_gamma;
	const bool is_hdr_enabled = r_hdr_mgr_uptr->is_hdr_enabled();
	const float sdr_white_level = r_hdr_mgr_uptr->get_sdr_white_level_float();
	const float sdr_white_level_nits = sdr_white_level * 80.0F;
	const bool is_override_sdr_white_level = r_hdr_override_sdr_white_level->integer != 0;

	ri.Printf(PRINT_ALL, "HDR support: %s\n", is_hdr_allowed ? "on" : "off");

	ri.Printf(PRINT_ALL, "HDR detection: %s%s%s%s\n",
		is_hdr_always_detected ? "always" : "auto",
		is_hdr_unknown_detected ? " (" : "",
		is_hdr_unknown_detected ? r_hdr_detection->string : "",
		is_hdr_unknown_detected ? ")" : "");

	ri.Printf(PRINT_ALL, "HDR enabled: %s\n", is_hdr_enabled ? "yes" : "no");

	ri.Printf(PRINT_ALL, "HDR color component transfer function: %s%s%s%s\n",
		is_hdr_cctf_gamma ? "gamma" : "srgb",
		is_hdr_cctf_unknown ? " (" : "",
		is_hdr_cctf_unknown ? r_hdr_cctf->string : "",
		is_hdr_cctf_unknown ? ")" : "");

	ri.Printf(PRINT_ALL, "HDR value for gamma CCTF: %f (%s)\n", r_hdr_cctf_gamma->value, r_hdr_cctf_gamma->string);
	ri.Printf(PRINT_ALL, "SDR white level: %f (%f nits)\n", sdr_white_level, sdr_white_level_nits);
	ri.Printf(PRINT_ALL, "Override SDR white level: %s\n", is_override_sdr_white_level ? "yes" : "no");

	if (!glConfigEx.is_default_framebuffer_float ||
		!is_hdr_allowed ||
		!(is_hdr_always_detected || is_hdr_enabled) ||
		!r_create_offscreen_color_texture_target() ||
		!r_create_offscreen_depth_stencil_renderbuffer_target() ||
		!r_create_offscreen_framebuffer() ||
		!r_create_offscreen_vbo() ||
		(glConfigEx.use_gl_arb_vertex_array_object && !r_create_offscreen_vao()) ||
		!r_update_offscreen_sdr_white_level())
	{
		ri.Printf(PRINT_ALL, "Skipping HDR processing.\n");
		r_terminate_offscreen();
		return;
	}

	glConfigEx.has_offscreen = true;
	r_invalidate_hdr_cvars();
}

void r_clear_gl_errors()
{
	for (int i = 0; i < 32; ++i)
	{
		const GLenum gl_error_code = glGetError();

		if (gl_error_code == GL_NO_ERROR)
		{
			break;
		}
	}
}

void r_assert_no_gl_errors()
{
	bool was_error = false;

	for (int i = 0; i < 32; ++i)
	{
		const GLenum gl_error_code = glGetError();

		if (gl_error_code == GL_NO_ERROR)
		{
			break;
		}

		was_error = true;
	}

	assert(!was_error);
}

} // namespace

void r_present_offscreen()
{
	glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);

	const bool blend_enabled = glIsEnabled(GL_BLEND) != 0;
	glDisable(GL_BLEND);

	const bool cull_face = glIsEnabled(GL_CULL_FACE) != 0;
	glDisable(GL_CULL_FACE);

	const bool depth_test = glIsEnabled(GL_DEPTH_TEST) != 0;
	glDisable(GL_DEPTH_TEST);

	const bool scissor_test = glIsEnabled(GL_SCISSOR_TEST) != 0;
	glDisable(GL_SCISSOR_TEST);

	const bool stencil_test = glIsEnabled(GL_STENCIL_TEST) != 0;
	glDisable(GL_STENCIL_TEST);

	GLboolean gl_depth_writemask = GL_FALSE;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &gl_depth_writemask);
	glDepthMask(GL_FALSE);

	GLint gl_polygon_mode[2] = {0, 0};
	glGetIntegerv(GL_POLYGON_MODE, gl_polygon_mode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GLint gl_stencil_writemask = 0;
	glGetIntegerv(GL_STENCIL_WRITEMASK, &gl_stencil_writemask);
	glStencilMask(0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glUseProgram(ogl_hdr_program->program_);

	glUniform1i(ogl_hdr_program->u_tex_2d, 2);

	if (r_hdr_cctf->modified || r_hdr_cctf_gamma->modified)
	{
		r_hdr_cctf->modified = false;
		r_hdr_cctf_gamma->modified = false;

		enum CctfId
		{
			cctf_id_none = 0,
			cctf_id_srgb = 1,
			cctf_id_gamma = 2
		};

		int cctf_id = cctf_id_none;

		if (strcmp(r_hdr_cctf->string, "srgb") == 0)
		{
			cctf_id = cctf_id_srgb;
		}
		else if (strcmp(r_hdr_cctf->string, "gamma") == 0)
		{
			cctf_id = cctf_id_gamma;
		}
		else
		{
			cctf_id = cctf_id_srgb;

			ri.Printf(
				PRINT_ALL,
				"%sUnknown HDR color component transfer function (%s).%s\n",
				S_COLOR_YELLOW,
				r_hdr_cctf->string,
				S_COLOR_WHITE);
		}

		const float cctf_gamma_min = 1.0F;
		const float cctf_gamma_max = 3.0F;

		float cctf_gamma = r_hdr_cctf_gamma->value;

		if (cctf_gamma < cctf_gamma_min)
		{
			cctf_gamma = cctf_gamma_min;
		}
		else if (cctf_gamma > cctf_gamma_max)
		{
			cctf_gamma = cctf_gamma_max;
		}

		glUniform1i(ogl_hdr_program->u_cctf_id, cctf_id);
		glUniform1f(ogl_hdr_program->u_cctf_gamma, cctf_gamma);
	}

	if (r_hdr_override_sdr_white_level->modified || r_hdr_sdr_white_level->modified)
	{
		r_hdr_override_sdr_white_level->modified = false;
		r_hdr_sdr_white_level->modified = false;

		const float sdr_white_level_min = 0.25F;
		const float sdr_white_level_max = 6.25F;

		float sdr_white_level;

		if (r_hdr_override_sdr_white_level->integer != 0)
		{
			sdr_white_level = r_hdr_sdr_white_level->value;
		}
		else
		{
			sdr_white_level = r_offscreen_sdr_white_level;
		}

		if (sdr_white_level < sdr_white_level_min)
		{
			sdr_white_level = sdr_white_level_min;
		}
		else if (sdr_white_level > sdr_white_level_max)
		{
			sdr_white_level = sdr_white_level_max;
		}

		glUniform1f(ogl_hdr_program->u_sdr_white_level, sdr_white_level);
	}

	if (ogl_tess_use_vao)
	{
		glBindVertexArray(r_offscreen_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(ogl_tess_vaos[ogl_tess_default_vao_index]);
		//glBindVertexArray(0);
	}
	else
	{
		for (GLuint i_array = 0; i_array < rtcw::OglProgram::max_vertex_attributes; ++i_array)
		{
			glDisableVertexAttribArray(i_array);
		}

		glBindBuffer(GL_ARRAY_BUFFER, r_offscreen_vbo);

		// position
		//
		glVertexAttribPointer(
			/* index */      ogl_hdr_program->a_pos_vec2,
			/* size */       2,
			/* type */       GL_FLOAT,
			/* normalized */ GL_FALSE,
			/* stride */     sizeof(ROffscreenVertex),
			/* pointer */    reinterpret_cast<const void*>(offsetof(ROffscreenVertex, pos))
		);

		glEnableVertexAttribArray(ogl_hdr_program->a_pos_vec2);

		// texture coordinates
		//
		glVertexAttribPointer(
			/* index */      ogl_hdr_program->a_tc_vec2,
			/* size */       2,
			/* type */       GL_FLOAT,
			/* normalized */ GL_FALSE,
			/* stride */     sizeof(ROffscreenVertex),
			/* pointer */    reinterpret_cast<const void*>(offsetof(ROffscreenVertex, tc))
		);

		glEnableVertexAttribArray(ogl_hdr_program->a_tc_vec2);

		// commit
		//
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glUseProgram(0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_offscreen_framebuffer);

	if (blend_enabled)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	if (cull_face)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	if (depth_test)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (scissor_test)
	{
		glEnable(GL_SCISSOR_TEST);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}

	if (stencil_test)
	{
		glEnable(GL_STENCIL_TEST);
	}
	else
	{
		glDisable(GL_STENCIL_TEST);
	}

	glDepthMask(gl_depth_writemask);

	if (gl_polygon_mode[0] == gl_polygon_mode[1])
	{
		glPolygonMode(GL_FRONT_AND_BACK, gl_polygon_mode[0]);
	}
	else
	{
		glPolygonMode(GL_FRONT, gl_polygon_mode[0]);
		glPolygonMode(GL_BACK, gl_polygon_mode[1]);
	}

	glStencilMask(static_cast<GLuint>(gl_stencil_writemask));

	glClampColor(GL_CLAMP_VERTEX_COLOR, GL_TRUE);
	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_TRUE);
	glClampColor(GL_CLAMP_READ_COLOR, GL_TRUE);
}

// BBi

/*
** InitOpenGL
**
** This function is responsible for initializing a valid OpenGL subsystem.  This
** is done by calling GLimp_Init (which gives us a working OGL subsystem) then
** setting variables, checking GL constants, and reporting the gfx system config
** to the user.
*/
static void InitOpenGL( void ) {
	char renderer_buffer[1024];

	//
	// initialize OS specific portions of the renderer
	//
	// GLimp_Init directly or indirectly references the following cvars:
	//		- r_fullscreen
	//		- r_glDriver
	//		- r_mode
	//		- r_(color|depth|stencil)bits
	//		- r_ignorehwgamma
	//		- r_gamma
	//

	if ( glConfig.vidWidth == 0 ) {
		// BBi
		//GLint temp;
		// BBi

		GLimp_Init();

		strcpy( renderer_buffer, glConfig.renderer_string );
		Q_strlwr( renderer_buffer );

		// BBi
		//// OpenGL driver constants
		//::glGetIntegerv( GL_MAX_TEXTURE_SIZE, &temp );
		//glConfig.maxTextureSize = temp;

		glConfig.maxTextureSize = R_GetMaxTextureSize ();
		// BBi

		// stubbed or broken drivers may have reported 0...
		if ( glConfig.maxTextureSize <= 0 ) {
			glConfig.maxTextureSize = 0;
		}
	}

	// BBi
	glConfigEx.renderer_path_ = RENDERER_PATH_OGL_1_X;

	if (glConfigEx.is_2_x_capable_) {
		glConfigEx.renderer_path_ = RENDERER_PATH_OGL_2_X;

		if (r_probe_programs()) {
			ri.Printf(PRINT_ALL, "\nUsing OpenGL 2.0+ path...\n");
		} else {
			glConfigEx.renderer_path_ = RENDERER_PATH_OGL_1_X;
			ri.Printf(PRINT_WARNING, "\nFalling back to OpenGL 1.1+ path...");
		}
	}

	if (!glConfigEx.is_path_ogl_1_x()) {
		r_reload_programs_f();
		r_tess_initialize();
		r_initialize_rgba_mode();
		r_initialize_offscreen();

		ri.Cvar_Set("r_ext_NV_fog_dist", "1");
		ri.Printf(PRINT_ALL, "Emulating %s...\n", "GL_NV_fog_distance");
	}
	// BBi

	// init command buffers and SMP
	R_InitCommandBuffers();

	// print info
	GfxInfo_f();

	// set default state
	GL_SetDefaultState();
}

/*
==================
GL_CheckErrors
==================
*/
void GL_CheckErrors( void ) {
	int err;
	char s[64];

	err = glGetError();
	if ( err == GL_NO_ERROR ) {
		return;
	}
	if ( r_ignoreGLErrors->integer ) {
		return;
	}
	switch ( err ) {
	case GL_INVALID_ENUM:
		strcpy( s, "GL_INVALID_ENUM" );
		break;
	case GL_INVALID_VALUE:
		strcpy( s, "GL_INVALID_VALUE" );
		break;
	case GL_INVALID_OPERATION:
		strcpy( s, "GL_INVALID_OPERATION" );
		break;
	case GL_STACK_OVERFLOW:
		strcpy( s, "GL_STACK_OVERFLOW" );
		break;
	case GL_STACK_UNDERFLOW:
		strcpy( s, "GL_STACK_UNDERFLOW" );
		break;
	case GL_OUT_OF_MEMORY:
		strcpy( s, "GL_OUT_OF_MEMORY" );
		break;
	default:
		Com_sprintf( s, sizeof( s ), "%i", err );
		break;
	}

#if !defined RTCW_ET
	ri.Error( ERR_FATAL, "GL_CheckErrors: %s", s );
#else
	ri.Error( ERR_VID_FATAL, "GL_CheckErrors: %s", s );
#endif // RTCW_XX

}


/*
** R_GetModeInfo
*/
typedef struct vidmode_s
{
	const char *description;
	int width, height;
	float pixelAspect;              // pixel width / height
} vidmode_t;

vidmode_t r_vidModes[] =
{
	{ "Mode  0: 320x240",        320,    240,    1 },
	{ "Mode  1: 400x300",        400,    300,    1 },
	{ "Mode  2: 512x384",        512,    384,    1 },
	{ "Mode  3: 640x480",        640,    480,    1 },
	{ "Mode  4: 800x600",        800,    600,    1 },
	{ "Mode  5: 960x720",        960,    720,    1 },
	{ "Mode  6: 1024x768",       1024,   768,    1 },
	{ "Mode  7: 1152x864",       1152,   864,    1 },
	{ "Mode  8: 1280x1024",      1280,   1024,   1 },
	{ "Mode  9: 1600x1200",      1600,   1200,   1 },
	{ "Mode 10: 2048x1536",      2048,   1536,   1 },

#if defined RTCW_SP
	{ "Mode 11: 856x480 (wide)",856, 480,    1 },
	{ "Mode 12: 1920x1200 (wide)",1920,  1200,   1 }     //----(SA)	added
#else
	{ "Mode 11: 856x480 (wide)",856, 480,    1 }
#endif // RTCW_XX

};
static int s_numVidModes = ( sizeof( r_vidModes ) / sizeof( r_vidModes[0] ) );

qboolean R_GetModeInfo( int *width, int *height, float *windowAspect, int mode ) {
	vidmode_t   *vm;

	if ( mode < -1 ) {
		return qfalse;
	}
	if ( mode >= s_numVidModes ) {
		return qfalse;
	}

	if ( mode == -1 ) {
		*width = r_customwidth->integer;
		*height = r_customheight->integer;
		*windowAspect = r_customaspect->value;
		return qtrue;
	}

	vm = &r_vidModes[mode];

	*width  = vm->width;
	*height = vm->height;
	*windowAspect = (float)vm->width / ( vm->height * vm->pixelAspect );

	return qtrue;
}

/*
** R_ModeList_f
*/
static void R_ModeList_f( void ) {
	int i;

	ri.Printf( PRINT_ALL, "\n" );
	for ( i = 0; i < s_numVidModes; i++ )
	{
		ri.Printf( PRINT_ALL, "%s\n", r_vidModes[i].description );
	}
	ri.Printf( PRINT_ALL, "\n" );
}


/*
==============================================================================

						SCREEN SHOTS

==============================================================================
*/

/*
==================
R_TakeScreenshot
==================
*/
void R_TakeScreenshot( int x, int y, int width, int height, char *fileName ) {
	byte        *buffer;
	int i, c, temp;

	buffer = static_cast<byte*> (ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight * 3 + 18 ));

	memset( buffer, 0, 18 );
	buffer[2] = 2;      // uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 24;    // pixel size

	glReadPixels( x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer + 18 );

	// swap rgb to bgr
	c = 18 + width * height * 3;
	for ( i = 18 ; i < c ; i += 3 ) {
		temp = buffer[i];
		buffer[i] = buffer[i + 2];
		buffer[i + 2] = temp;
	}

#ifndef RTCW_VANILLA
	if (glConfigEx.is_path_ogl_1_x())
	{
#endif // RTCW_VANILLA
	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer + 18, glConfig.vidWidth * glConfig.vidHeight * 3 );
	}
#ifndef RTCW_VANILLA
	}
#endif // RTCW_VANILLA

	ri.FS_WriteFile( fileName, buffer, c );

	ri.Hunk_FreeTempMemory( buffer );
}

/*
==============
R_TakeScreenshotJPEG
==============
*/
void R_TakeScreenshotJPEG( int x, int y, int width, int height, char *fileName ) {
	byte        *buffer;

	buffer = static_cast<byte*> (ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight * 4 ));

	glReadPixels( x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer );

#ifndef RTCW_VANILLA
	if (glConfigEx.is_path_ogl_1_x())
	{
#endif // RTCW_VANILLA
	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer, glConfig.vidWidth * glConfig.vidHeight * 4 );
	}
#ifndef RTCW_VANILLA
	}
#endif // RTCW_VANILLA

	ri.FS_WriteFile( fileName, buffer, 1 );     // create path
	SaveJPG( fileName, 95, glConfig.vidWidth, glConfig.vidHeight, buffer );

	ri.Hunk_FreeTempMemory( buffer );
}

/*
==================
R_ScreenshotFilename
==================
*/
void R_ScreenshotFilename( int lastNumber, char *fileName ) {

#if !defined RTCW_ET
	int a,b,c,d;
#endif // RTCW_XX

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot9999.tga" );
		return;
	}

#if !defined RTCW_ET
	a = lastNumber / 1000;
	lastNumber -= a * 1000;
	b = lastNumber / 100;
	lastNumber -= b * 100;
	c = lastNumber / 10;
	lastNumber -= c * 10;
	d = lastNumber;

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.tga"
				 , a, b, c, d );
#else
	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%04i.tga", lastNumber );
#endif // RTCW_XX

}

/*
==============
R_ScreenshotFilenameJPEG
==============
*/
void R_ScreenshotFilenameJPEG( int lastNumber, char *fileName ) {

#if !defined RTCW_ET
	int a,b,c,d;
#endif // RTCW_XX

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot9999.jpg" );
		return;
	}

#if !defined RTCW_ET
	a = lastNumber / 1000;
	lastNumber -= a * 1000;
	b = lastNumber / 100;
	lastNumber -= b * 100;
	c = lastNumber / 10;
	lastNumber -= c * 10;
	d = lastNumber;

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.jpg"
				 , a, b, c, d );
#else
	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%04i.jpg", lastNumber );
#endif // RTCW_XX

}

/*
====================
R_LevelShot

levelshots are specialized 128*128 thumbnails for
the menu system, sampled down from full screen distorted images
====================
*/
void R_LevelShot( void ) {
	char checkname[MAX_OSPATH];
	byte        *buffer;
	byte        *source;
	byte        *src, *dst;
	int x, y;
	int r, g, b;
	float xScale, yScale;
	int xx, yy;

	sprintf( checkname, "levelshots/%s.tga", tr.world->baseName );

	source = static_cast<byte*> (ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight * 3 ));

	buffer = static_cast<byte*> (ri.Hunk_AllocateTempMemory( 128 * 128 * 3 + 18 ));
	memset( buffer, 0, 18 );
	buffer[2] = 2;      // uncompressed type
	buffer[12] = 128;
	buffer[14] = 128;
	buffer[16] = 24;    // pixel size

	glReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_RGB, GL_UNSIGNED_BYTE, source );

	// resample from source
	xScale = glConfig.vidWidth / 512.0f;
	yScale = glConfig.vidHeight / 384.0f;
	for ( y = 0 ; y < 128 ; y++ ) {
		for ( x = 0 ; x < 128 ; x++ ) {
			r = g = b = 0;
			for ( yy = 0 ; yy < 3 ; yy++ ) {
				for ( xx = 0 ; xx < 4 ; xx++ ) {
					src = source + 3 * ( glConfig.vidWidth * (int)( ( y * 3 + yy ) * yScale ) + (int)( ( x * 4 + xx ) * xScale ) );
					r += src[0];
					g += src[1];
					b += src[2];
				}
			}
			dst = buffer + 18 + 3 * ( y * 128 + x );
			dst[0] = b / 12;
			dst[1] = g / 12;
			dst[2] = r / 12;
		}
	}

#ifndef RTCW_VANILLA
	if (glConfigEx.is_path_ogl_1_x())
	{
#endif // RTCW_VANILLA
	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer + 18, 128 * 128 * 3 );
	}
#ifndef RTCW_VANILLA
	}
#endif // RTCW_VANILLA

	ri.FS_WriteFile( checkname, buffer, 128 * 128 * 3 + 18 );

	ri.Hunk_FreeTempMemory( buffer );
	ri.Hunk_FreeTempMemory( source );

	ri.Printf( PRINT_ALL, "Wrote %s\n", checkname );
}

/*
==================
R_ScreenShot_f

screenshot
screenshot [silent]
screenshot [levelshot]
screenshot [filename]

Doesn't print the pacifier message if there is a second arg
==================
*/
void R_ScreenShot_f( void ) {
	char checkname[MAX_OSPATH];
	int len;
	static int lastNumber = -1;
	qboolean silent;

	if ( !strcmp( ri.Cmd_Argv( 1 ), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv( 1 ), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, MAX_OSPATH, "screenshots/%s.tga", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilename( lastNumber, checkname );

			len = ri.FS_ReadFile( checkname, NULL );
			if ( len <= 0 ) {
				break;  // file doesn't exist
			}
		}

		if ( lastNumber >= 9999 ) {
			ri.Printf( PRINT_ALL, "ScreenShot: Couldn't create a file\n" );
			return;
		}

		lastNumber++;
	}


	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname );

	if ( !silent ) {
		ri.Printf( PRINT_ALL, "Wrote %s\n", checkname );
	}
}

void R_ScreenShotJPEG_f( void ) {
	char checkname[MAX_OSPATH];
	int len;
	static int lastNumber = -1;
	qboolean silent;

	if ( !strcmp( ri.Cmd_Argv( 1 ), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv( 1 ), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, MAX_OSPATH, "screenshots/%s.jpg", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilenameJPEG( lastNumber, checkname );

			len = ri.FS_ReadFile( checkname, NULL );
			if ( len <= 0 ) {
				break;  // file doesn't exist
			}
		}

		if ( lastNumber == 10000 ) {
			ri.Printf( PRINT_ALL, "ScreenShot: Couldn't create a file\n" );
			return;
		}

		lastNumber++;
	}


	R_TakeScreenshotJPEG( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname );

	if ( !silent ) {
		ri.Printf( PRINT_ALL, "Wrote %s\n", checkname );
	}
}

//============================================================================

/*
** GL_SetDefaultState
*/
void GL_SetDefaultState( void ) {
	if (glConfig.anisotropicAvailable)
	{
		float maxAnisotropy;

		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
		glConfig.maxAnisotropy = maxAnisotropy;
	}

	glConfig.maxAnisotropy = std::max(glConfig.maxAnisotropy, 1.0F);


	// BBi
	ogl_tess_state.reset ();
	// BBi

	glClearDepth( 1.0f );

	glCullFace( GL_FRONT );

	// BBi
	if (!glConfigEx.is_path_ogl_1_x ()) {
		ogl_tess_state.primary_color = rtcw::cgm::Vec4(1.0F, 1.0F, 1.0F, 1.0F);
	} else {
	// BBi

	glColor4f( 1,1,1,1 );

	// BBi
	}
	// BBi

	// initialize downstream texture unit if we're running
	// in a multitexture environment
	if (glConfigEx.use_arb_multitexture_) {
		GL_SelectTexture( 1 );
		GL_TextureMode( r_textureMode->string );

// BBi
//#if defined RTCW_ET
// BBi

		GL_TextureAnisotropy( r_textureAnisotropy->value );

// BBi
//#endif // RTCW_XX
// BBi

		GL_TexEnv( GL_MODULATE );

		// BBi
		if (glConfigEx.is_path_ogl_1_x ()) {
		// BBi

		glDisable( GL_TEXTURE_2D );

		// BBi
		}
		// BBi

		GL_SelectTexture( 0 );
	}

	// BBi
	if (glConfigEx.is_path_ogl_1_x ()) {
	// BBi

	glEnable( GL_TEXTURE_2D );

	// BBi
	}
	// BBi

	GL_TextureMode( r_textureMode->string );

	// BBi
	if (!glConfigEx.is_path_ogl_1_x ()) {
		ogl_tess_state.tex_2d[0] = 0;
		ogl_tess_state.tex_2d[1] = 1;
		ogl_tess_state.use_multitexturing = false;
	}
	// BBi

	GL_TexEnv( GL_MODULATE );

	if (glConfigEx.is_path_ogl_1_x ())
		glShadeModel( GL_SMOOTH );

	glDepthFunc( GL_LEQUAL );

	// BBi
	if (glConfigEx.is_path_ogl_1_x ()) {
	// BBi

	// the vertex array is always enabled, but the color and texture
	// arrays are enabled and disabled around the compiled vertex array call
	glEnableClientState( GL_VERTEX_ARRAY );

	// BBi
	}
	// BBi

	//
	// make sure our GL state vector is set correctly
	//
	glState.glStateBits = GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_TRUE;

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glDepthMask( GL_TRUE );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_SCISSOR_TEST );
	glDisable( GL_CULL_FACE );
	glDisable( GL_BLEND );

//----(SA)	added.

	// ATI pn_triangles
	if ( glPNTrianglesiATI ) {
		GLint maxtess = 0;
		// get max supported tesselation
		glGetIntegerv( GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI, &maxtess );
		glConfig.ATIMaxTruformTess = maxtess;
		// cap if necessary
		if ( r_ati_truform_tess->value > maxtess ) {
			ri.Cvar_Set( "r_ati_truform_tess", va( "%d", maxtess ) );
		}

		// set Wolf defaults
		glPNTrianglesiATI( GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI, static_cast<GLint>(r_ati_truform_tess->value) );
	}

//----(SA)	end
}


// BBi
static void gfxPrintExtensions ()
{
	const GLubyte* extString = glGetString (GL_EXTENSIONS);

	if (extString != 0) {
		GLubyte extBuffer[128];

		const GLubyte* beginExt = extString;
		const GLubyte* endExt = extString;

		while (true) {
			while ((*endExt != ' ') && (*endExt != '\0')) {
				++endExt;
			}

			size_t extLen = endExt - beginExt;

			memcpy (extBuffer, beginExt, extLen);
			extBuffer[extLen] = '\0';

			ri.Printf (PRINT_ALL, "  %s\n", extBuffer);

			if (*endExt == '\0')
				break;

			++endExt;

			beginExt = endExt;
		}
	} else
		ri.Printf (PRINT_ALL, "  none\n");
}
// BBi

/*
================
GfxInfo_f
================
*/
void GfxInfo_f( void ) {
	cvar_t *sys_cpustring = ri.Cvar_Get( "sys_cpustring", "", 0 );
	const char *enablestrings[] =
	{
		"disabled",
		"enabled"
	};
	const char *fsstrings[] =
	{
		"windowed",
		"fullscreen"
	};

	ri.Printf( PRINT_ALL, "\nGL_VENDOR: %s\n", glConfig.vendor_string );
	ri.Printf( PRINT_ALL, "GL_RENDERER: %s\n", glConfig.renderer_string );
	ri.Printf( PRINT_ALL, "GL_VERSION: %s\n", glConfig.version_string );

	// BBi See #LBUG0001
	//ri.Printf( PRINT_ALL, "GL_EXTENSIONS: %s\n", glConfig.extensions_string );
	ri.Printf (PRINT_ALL, "GL_EXTENSIONS:\n");
	gfxPrintExtensions ();
	// BBi

	ri.Printf( PRINT_ALL, "GL_MAX_TEXTURE_SIZE: %d\n", glConfig.maxTextureSize );
	ri.Printf( PRINT_ALL, "GL_MAX_TEXTURE_UNITS_ARB: %d\n", glConfig.maxActiveTextures );
	ri.Printf( PRINT_ALL, "\nPIXELFORMAT: color(%d-bits) Z(%d-bit) stencil(%d-bits)\n", glConfig.colorBits, glConfig.depthBits, glConfig.stencilBits );
	ri.Printf( PRINT_ALL, "MODE: %d, %d x %d %s hz:", r_mode->integer, glConfig.vidWidth, glConfig.vidHeight, fsstrings[r_fullscreen->integer == 1] );
	if ( glConfig.displayFrequency ) {
		ri.Printf( PRINT_ALL, "%d\n", glConfig.displayFrequency );
	} else
	{
		ri.Printf( PRINT_ALL, "N/A\n" );
	}
	if ( glConfig.deviceSupportsGamma ) {
		ri.Printf( PRINT_ALL, "GAMMA: hardware w/ %d overbright bits\n", tr.overbrightBits );
	} else
	{
		ri.Printf( PRINT_ALL, "GAMMA: software w/ %d overbright bits\n", tr.overbrightBits );
	}
	ri.Printf( PRINT_ALL, "CPU: %s\n", sys_cpustring->string );

	// rendering primitives
	{
		int primitives;

		// default is to use triangles if compiled vertex arrays are present
		ri.Printf( PRINT_ALL, "rendering primitives: " );
		primitives = r_primitives->integer;
		if ( primitives == 0 ) {
			if (glConfigEx.use_ext_compiled_vertex_array_) {
				primitives = 2;
			} else {
				primitives = 1;
			}
		}
		if ( primitives == -1 ) {
			ri.Printf( PRINT_ALL, "none\n" );
		} else if ( primitives == 2 ) {
			ri.Printf( PRINT_ALL, "single glDrawElements\n" );
		} else if ( primitives == 1 ) {
			ri.Printf( PRINT_ALL, "multiple glArrayElement\n" );
		} else if ( primitives == 3 ) {
			ri.Printf( PRINT_ALL, "multiple glColor4ubv + glTexCoord2fv + glVertex3fv\n" );
		}
	}

	ri.Printf( PRINT_ALL, "texturemode: %s\n", r_textureMode->string );
	ri.Printf( PRINT_ALL, "picmip: %d\n", r_picmip->integer );

#if defined RTCW_SP
	ri.Printf( PRINT_ALL, "picmip2: %d\n", r_picmip2->integer );
#endif // RTCW_XX

	ri.Printf( PRINT_ALL, "texture bits: %d\n", r_texturebits->integer );
	ri.Printf( PRINT_ALL, "multitexture: %s\n", enablestrings[glConfigEx.use_arb_multitexture_] );
	ri.Printf( PRINT_ALL, "compiled vertex arrays: %s\n", enablestrings[glConfigEx.use_ext_compiled_vertex_array_] );
	ri.Printf( PRINT_ALL, "texenv add: %s\n", enablestrings[glConfig.textureEnvAddAvailable != 0] );
	ri.Printf( PRINT_ALL, "compressed textures: %s\n", enablestrings[glConfig.textureCompression != TC_NONE] );

#ifdef RTCW_SP
	ri.Printf( PRINT_ALL, "ATI truform: %s\n", enablestrings[glPNTrianglesiATI != 0] );
	if ( glPNTrianglesiATI ) {
//DAJ bogus at this point		ri.Printf( PRINT_ALL, "MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI: %d\n", glConfig.ATIMaxTruformTess );
		ri.Printf( PRINT_ALL, "Truform Tess: %d\n", r_ati_truform_tess->integer );
		ri.Printf( PRINT_ALL, "Truform Point Mode: %s\n", r_ati_truform_pointmode->string );
		ri.Printf( PRINT_ALL, "Truform Normal Mode: %s\n", r_ati_truform_normalmode->string );
	}
#endif // RTCW_SP

// BBi
//#if defined RTCW_ET
// BBi

	ri.Printf( PRINT_ALL, "anisotropy: %s\n", r_textureAnisotropy->string );

// BBi
//#endif // RTCW_XX
// BBi

// BBi
	//ri.Printf( PRINT_ALL, "NV distance fog: %s\n", enablestrings[glConfig.NVFogAvailable != 0] );
	//if ( glConfig.NVFogAvailable ) {
	//	ri.Printf( PRINT_ALL, "Fog Mode: %s\n", r_nv_fogdist_mode->string );
	//}
// BBi

// BBi
//#if !defined RTCW_ET
//	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
//#else
//	if ( glConfig.hardwareType == GLHW_PERMEDIA2 ) {
//#endif // RTCW_XX
//
//		ri.Printf( PRINT_ALL, "HACK: using vertex lightmap approximation\n" );
//	}
//	if ( glConfig.hardwareType == GLHW_RAGEPRO ) {
//		ri.Printf( PRINT_ALL, "HACK: ragePro approximations\n" );
//	}
//	if ( glConfig.hardwareType == GLHW_RIVA128 ) {
//		ri.Printf( PRINT_ALL, "HACK: riva128 approximations\n" );
//	}
// BBi

// BBi
//	if ( glConfig.smpActive ) {
//		ri.Printf( PRINT_ALL, "Using dual processor acceleration\n" );
//	}
// BBi

	if ( r_finish->integer ) {
		ri.Printf( PRINT_ALL, "Forcing glFinish\n" );
	}
}

//#if defined RTCW_SP
// RF
extern void R_CropImages_f( void );
//#endif // RTCW_XX


/*
===============
R_Register
===============
*/
void R_Register( void ) {
	//
	// latched and archived variables
	//

	r_glDriver = ri.Cvar_Get("r_glDriver", OPENGL_DRIVER_NAME, CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_allowExtensions = ri.Cvar_Get("r_allowExtensions", "1", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_ext_compressed_textures = ri.Cvar_Get("r_ext_compressed_textures", "1", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE); // (SA) ew, a spelling change I missed from the missionpack
	r_ext_gamma_control = ri.Cvar_Get("r_ext_gamma_control", "1", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_ext_multitexture = ri.Cvar_Get("r_ext_multitexture", "1", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_ext_compiled_vertex_array = ri.Cvar_Get("r_ext_compiled_vertex_array", "1", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_glIgnoreWicked3D = ri.Cvar_Get("r_glIgnoreWicked3D", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	//----(SA)	added
	r_ext_ATI_pntriangles = ri.Cvar_Get("r_ext_ATI_pntriangles", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE); //----(SA)	default to '0'
	r_ati_truform_tess = ri.Cvar_Get("r_ati_truform_tess", "1", CVAR_ARCHIVE | CVAR_UNSAFE);
	r_ati_truform_normalmode = ri.Cvar_Get("r_ati_truform_normalmode", "GL_PN_TRIANGLES_NORMAL_MODE_LINEAR", CVAR_ARCHIVE | CVAR_UNSAFE);
	r_ati_truform_pointmode = ri.Cvar_Get("r_ati_truform_pointmode", "GL_PN_TRIANGLES_POINT_MODE_LINEAR", CVAR_ARCHIVE | CVAR_UNSAFE);
	// GR - Change default mode -- linear doesn't do much...
	r_ati_truform_normalmode = ri.Cvar_Get( "r_ati_truform_normalmode", "QUADRATIC", CVAR_ARCHIVE );
	r_ati_truform_pointmode = ri.Cvar_Get( "r_ati_truform_pointmode", "CUBIC", CVAR_ARCHIVE );
	r_ati_fsaa_samples = ri.Cvar_Get("r_ati_fsaa_samples", "0", CVAR_ARCHIVE | CVAR_UNSAFE);
	r_ext_texture_filter_anisotropic = ri.Cvar_Get("r_ext_texture_filter_anisotropic", "1", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_ext_NV_fog_dist = ri.Cvar_Get("r_ext_NV_fog_dist", "1", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	// default to 'looking good'
	r_nv_fogdist_mode = ri.Cvar_Get("r_nv_fogdist_mode", "GL_EYE_RADIAL_NV", CVAR_ARCHIVE | CVAR_UNSAFE);

//----(SA)	end

	r_ext_texture_env_add = ri.Cvar_Get("r_ext_texture_env_add", "1", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	// ydnar: opengl 1.2 GL_CLAMP_TO_EDGE support
	r_clampToEdge = ri.Cvar_Get("r_clampToEdge", "1", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_picmip = ri.Cvar_Get("r_picmip", "0", CVAR_ARCHIVE | CVAR_LATCH);
	// used for character skins picmipping at a different level from the rest of the game
	r_picmip2 = ri.Cvar_Get("r_picmip2", "0", CVAR_ARCHIVE | CVAR_LATCH); 
	r_roundImagesDown = ri.Cvar_Get( "r_roundImagesDown", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_lowMemTextureSize = ri.Cvar_Get( "r_lowMemTextureSize", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_lowMemTextureThreshold = ri.Cvar_Get( "r_lowMemTextureThreshold", "15.0", CVAR_ARCHIVE | CVAR_LATCH );
	r_rmse = ri.Cvar_Get( "r_rmse", "0.0", CVAR_ARCHIVE | CVAR_LATCH );
	r_colorMipLevels = ri.Cvar_Get( "r_colorMipLevels", "0", CVAR_LATCH );
	AssertCvarRange( r_picmip, 0, 3, qtrue );
	AssertCvarRange( r_picmip2, 0, 3, qtrue );
	r_detailTextures = ri.Cvar_Get( "r_detailtextures", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_texturebits = ri.Cvar_Get("r_texturebits", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_colorbits = ri.Cvar_Get("r_colorbits", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_stereo = ri.Cvar_Get("r_stereo", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_stencilbits = ri.Cvar_Get("r_stencilbits", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_depthbits = ri.Cvar_Get("r_depthbits", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_overBrightBits = ri.Cvar_Get("r_overBrightBits", "0", CVAR_ARCHIVE | CVAR_LATCH); // Arnout: disable overbrightbits by default
	AssertCvarRange( r_overBrightBits, 0, 1, qtrue ); // ydnar: limit to overbrightbits 1 (sorry 1337 players)
	r_ignorehwgamma = ri.Cvar_Get("r_ignorehwgamma", "1", CVAR_ARCHIVE | CVAR_LATCH); //----(SA) changed this to default to '1' for Drew
	r_mode = ri.Cvar_Get("r_mode", "4", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_oldMode = ri.Cvar_Get( "r_oldMode", "", CVAR_ARCHIVE ); // ydnar: previous "good" video mode
	r_fullscreen = ri.Cvar_Get( "r_fullscreen", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_customwidth = ri.Cvar_Get( "r_customwidth", "1600", CVAR_ARCHIVE | CVAR_LATCH );
	r_customheight = ri.Cvar_Get( "r_customheight", "1024", CVAR_ARCHIVE | CVAR_LATCH );
	r_customaspect = ri.Cvar_Get( "r_customaspect", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_simpleMipMaps = ri.Cvar_Get( "r_simpleMipMaps", "1", CVAR_ARCHIVE | CVAR_LATCH );
	r_vertexLight = ri.Cvar_Get( "r_vertexLight", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_uiFullScreen = ri.Cvar_Get( "r_uifullscreen", "0", 0 );
	r_subdivisions = ri.Cvar_Get( "r_subdivisions", "4", CVAR_ARCHIVE | CVAR_LATCH );
	r_smp = ri.Cvar_Get("r_smp", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_UNSAFE);
	r_ignoreFastPath = ri.Cvar_Get("r_ignoreFastPath", "1", CVAR_ARCHIVE | CVAR_LATCH);

	//
	// temporary latched variables that can only change over a restart
	//

	r_displayRefresh = ri.Cvar_Get("r_displayRefresh", "0", CVAR_LATCH | CVAR_UNSAFE);
	r_fullbright = ri.Cvar_Get( "r_fullbright", "0", CVAR_LATCH | CVAR_CHEAT );
	r_mapOverBrightBits = ri.Cvar_Get( "r_mapOverBrightBits", "0", CVAR_LATCH );
	AssertCvarRange( r_mapOverBrightBits, 0, 3, qtrue );
	r_intensity = ri.Cvar_Get( "r_intensity", "1", CVAR_LATCH );
	AssertCvarRange( r_intensity, 0, 1.5, qfalse );
	r_singleShader = ri.Cvar_Get( "r_singleShader", "0", CVAR_CHEAT | CVAR_LATCH );

	//
	// archived variables that can change at any time
	//
	r_lodCurveError = ri.Cvar_Get( "r_lodCurveError", "250", CVAR_ARCHIVE );
	r_lodbias = ri.Cvar_Get( "r_lodbias", "0", CVAR_ARCHIVE );
	r_flares = ri.Cvar_Get( "r_flares", "1", CVAR_ARCHIVE );
#if !defined RTCW_ET
	r_znear = ri.Cvar_Get( "r_znear", "4", CVAR_CHEAT );
#else
	r_znear = ri.Cvar_Get( "r_znear", "3", CVAR_CHEAT );  // ydnar: changed it to 3 (from 4) because of lean/fov cheats
#endif // RTCW_XX
	AssertCvarRange( r_znear, 0.001f, 200, qtrue );
//----(SA)	added
	r_zfar = ri.Cvar_Get( "r_zfar", "0", CVAR_CHEAT );
//----(SA)	end
	r_ignoreGLErrors = ri.Cvar_Get( "r_ignoreGLErrors", "1", CVAR_ARCHIVE );
	r_fastsky = ri.Cvar_Get( "r_fastsky", "0", CVAR_ARCHIVE );
	r_inGameVideo = ri.Cvar_Get( "r_inGameVideo", "1", CVAR_ARCHIVE );
	r_drawSun = ri.Cvar_Get( "r_drawSun", "1", CVAR_ARCHIVE );
	r_dynamiclight = ri.Cvar_Get( "r_dynamiclight", "1", CVAR_ARCHIVE );
	r_dlightScale = ri.Cvar_Get( "r_dlightScale", "1.0", CVAR_ARCHIVE );   //----(SA)	added
	r_dlightBacks = ri.Cvar_Get( "r_dlightBacks", "1", CVAR_ARCHIVE );
	r_finish = ri.Cvar_Get( "r_finish", "0", CVAR_ARCHIVE );
	r_textureMode = ri.Cvar_Get( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE );
	r_textureAnisotropy = ri.Cvar_Get( "r_textureAnisotropy", "0", CVAR_ARCHIVE );
	r_swapInterval = ri.Cvar_Get( "r_swapInterval", "1", CVAR_ARCHIVE );
	r_gamma = ri.Cvar_Get( "r_gamma", "1", CVAR_ARCHIVE );
	r_facePlaneCull = ri.Cvar_Get( "r_facePlaneCull", "1", CVAR_ARCHIVE );
	r_railWidth = ri.Cvar_Get( "r_railWidth", "16", CVAR_ARCHIVE );
	r_railCoreWidth = ri.Cvar_Get( "r_railCoreWidth", "1", CVAR_ARCHIVE );
	r_railSegmentLength = ri.Cvar_Get( "r_railSegmentLength", "32", CVAR_ARCHIVE );
	r_waterFogColor = ri.Cvar_Get( "r_waterFogColor", "0", CVAR_ROM );  //----(SA)	added
	r_mapFogColor = ri.Cvar_Get( "r_mapFogColor", "0", CVAR_ROM );  //----(SA)	added
	r_savegameFogColor = ri.Cvar_Get( "r_savegameFogColor", "0", CVAR_ROM );    //----(SA)	added
	r_primitives = ri.Cvar_Get( "r_primitives", "0", CVAR_ARCHIVE );
	r_ambientScale = ri.Cvar_Get( "r_ambientScale", "0.5", CVAR_CHEAT );
	r_directedScale = ri.Cvar_Get( "r_directedScale", "1", CVAR_CHEAT );

	//
	// temporary variables that can change at any time
	//

	r_showImages = ri.Cvar_Get( "r_showImages", "0", CVAR_TEMP );

	r_debugLight = ri.Cvar_Get( "r_debuglight", "0", CVAR_TEMP );
	r_debugSort = ri.Cvar_Get( "r_debugSort", "0", CVAR_CHEAT );
	r_printShaders = ri.Cvar_Get( "r_printShaders", "0", 0 );
	r_saveFontData = ri.Cvar_Get( "r_saveFontData", "0", 0 );

	// Ridah

	// TTimo show_bug.cgi?id=440
	//   with r_cache enabled, non-win32 OSes were leaking 24Mb per R_Init..
	r_cache = ri.Cvar_Get( "r_cache", "1", CVAR_LATCH );  // leaving it as this for backwards compability. but it caches models and shaders also
	// TTimo show_bug.cgi?id=570
	r_cacheShaders = ri.Cvar_Get( "r_cacheShaders", "1", CVAR_LATCH );
	r_cacheModels = ri.Cvar_Get( "r_cacheModels", "1", CVAR_LATCH );
	r_compressModels = ri.Cvar_Get( "r_compressModels", "0", 0 );     // converts MD3 -> MDC at run-time
	r_exportCompressedModels = ri.Cvar_Get( "r_exportCompressedModels", "0", 0 ); // saves compressed models
	r_cacheGathering = ri.Cvar_Get( "cl_cacheGathering", "0", 0 );
	r_buildScript = ri.Cvar_Get( "com_buildscript", "0", 0 );
	r_bonesDebug = ri.Cvar_Get( "r_bonesDebug", "0", CVAR_CHEAT );
	// done.

	// Rafael - wolf fog
	r_wolffog = ri.Cvar_Get(
		"r_wolffog",
		"1",
#ifdef RTCW_SP
		0
#else // RTCW_SP
		CVAR_CHEAT
#endif // RTCW_SP
	); // JPW NERVE cheat protected per id request

	// done

	r_nocurves = ri.Cvar_Get( "r_nocurves", "0", CVAR_CHEAT );
	r_drawworld = ri.Cvar_Get( "r_drawworld", "1", CVAR_CHEAT );
	r_drawfoliage = ri.Cvar_Get( "r_drawfoliage", "1", CVAR_CHEAT );  // ydnar
	r_lightmap = ri.Cvar_Get( "r_lightmap", "0", CVAR_CHEAT ); // DHM - NERVE :: cheat protect
	r_portalOnly = ri.Cvar_Get( "r_portalOnly", "0", CVAR_CHEAT );
	r_flareSize = ri.Cvar_Get( "r_flareSize", "40", CVAR_CHEAT );
#ifndef RTCW_SP
	ri.Cvar_Set( "r_flareFade", "5" ); // to force this when people already have "7" in their config
#endif // RTCW_SP
	r_flareFade = ri.Cvar_Get( "r_flareFade", "5", CVAR_CHEAT );
	r_showSmp = ri.Cvar_Get( "r_showSmp", "0", CVAR_CHEAT );
	r_skipBackEnd = ri.Cvar_Get( "r_skipBackEnd", "0", CVAR_CHEAT );
	r_measureOverdraw = ri.Cvar_Get( "r_measureOverdraw", "0", CVAR_CHEAT );
	r_lodscale = ri.Cvar_Get( "r_lodscale", "5", CVAR_CHEAT );
	r_norefresh = ri.Cvar_Get( "r_norefresh", "0", CVAR_CHEAT );
	r_drawentities = ri.Cvar_Get( "r_drawentities", "1", CVAR_CHEAT );
	r_ignore = ri.Cvar_Get( "r_ignore", "1", CVAR_CHEAT );
	r_nocull = ri.Cvar_Get( "r_nocull", "0", CVAR_CHEAT );
	r_novis = ri.Cvar_Get( "r_novis", "0", CVAR_CHEAT );
	r_showcluster = ri.Cvar_Get( "r_showcluster", "0", CVAR_CHEAT );
	r_speeds = ri.Cvar_Get( "r_speeds", "0", CVAR_CHEAT );
	r_verbose = ri.Cvar_Get( "r_verbose", "0", CVAR_CHEAT );
	r_logFile = ri.Cvar_Get( "r_logFile", "0", CVAR_CHEAT );
	r_debugSurface = ri.Cvar_Get( "r_debugSurface", "0", CVAR_CHEAT );
	r_nobind = ri.Cvar_Get( "r_nobind", "0", CVAR_CHEAT );
	r_showtris = ri.Cvar_Get( "r_showtris", "0", CVAR_CHEAT );
	r_trisColor = ri.Cvar_Get( "r_trisColor", "1.0 1.0 1.0 1.0", CVAR_ARCHIVE );
	r_showsky = ri.Cvar_Get( "r_showsky", "0", CVAR_CHEAT );
	r_shownormals = ri.Cvar_Get( "r_shownormals", "0", CVAR_CHEAT );
	r_normallength = ri.Cvar_Get( "r_normallength", "0.5", CVAR_ARCHIVE );
	r_showmodelbounds = ri.Cvar_Get( "r_showmodelbounds", "0", CVAR_CHEAT );
	r_clear = ri.Cvar_Get( "r_clear", "0", CVAR_CHEAT );
	r_offsetFactor = ri.Cvar_Get( "r_offsetfactor", "-1", CVAR_CHEAT );
	r_offsetUnits = ri.Cvar_Get( "r_offsetunits", "-2", CVAR_CHEAT );
	r_drawBuffer = ri.Cvar_Get( "r_drawBuffer", "GL_BACK", CVAR_CHEAT );
	r_lockpvs = ri.Cvar_Get( "r_lockpvs", "0", CVAR_CHEAT );
	r_noportals = ri.Cvar_Get( "r_noportals", "0", CVAR_CHEAT );
	r_shadows = ri.Cvar_Get( "cg_shadows", "1", 0 );
	r_portalsky = ri.Cvar_Get( "cg_skybox", "1", 0 );
	r_maxpolys = ri.Cvar_Get( "r_maxpolys", va( "%d", MAX_POLYS ), 0 );
	r_maxpolyverts = ri.Cvar_Get( "r_maxpolyverts", va( "%d", MAX_POLYVERTS ), 0 );
	r_highQualityVideo = ri.Cvar_Get( "r_highQualityVideo", "1", CVAR_ARCHIVE);
	r_dbg_use_glsl_shader_files = ri.Cvar_Get("r_dbg_use_glsl_shader_files", "0", CVAR_ARCHIVE | CVAR_CHEAT);
	r_hdr = ri.Cvar_Get("r_hdr", "1", CVAR_ARCHIVE);
	r_hdr_detection = ri.Cvar_Get("r_hdr_detection", "auto", CVAR_ARCHIVE);
	r_hdr_cctf = ri.Cvar_Get("r_hdr_cctf", "srgb", CVAR_ARCHIVE);
	r_hdr_cctf_gamma = ri.Cvar_Get("r_hdr_cctf_gamma", "2.2", CVAR_ARCHIVE);
	r_hdr_sdr_white_level = ri.Cvar_Get("r_hdr_sdr_white_level", "1.0", CVAR_ARCHIVE);
	r_hdr_override_sdr_white_level = ri.Cvar_Get("r_hdr_override_sdr_white_level", "0", CVAR_ARCHIVE);

	// make sure all the commands added here are also
	// removed in R_Shutdown
	ri.Cmd_AddCommand( "imagelist", R_ImageList_f );
	ri.Cmd_AddCommand( "shaderlist", R_ShaderList_f );
	ri.Cmd_AddCommand( "skinlist", R_SkinList_f );
	ri.Cmd_AddCommand( "modellist", R_Modellist_f );
	ri.Cmd_AddCommand( "modelist", R_ModeList_f );
	ri.Cmd_AddCommand( "screenshot", R_ScreenShot_f );
	ri.Cmd_AddCommand( "screenshotJPEG", R_ScreenShotJPEG_f );
	ri.Cmd_AddCommand( "gfxinfo", GfxInfo_f );
	ri.Cmd_AddCommand( "taginfo", R_TagInfo_f );

	// Ridah
	ri.Cmd_AddCommand( "cropimages", R_CropImages_f );

	// BBi
	ri.Cmd_AddCommand ("r_reload_programs", r_reload_programs_f);
	// BBi

	// done.
}

/*
===============
R_Init
===============
*/
void R_Init( void ) {
	int err;
	int i;

	ri.Printf( PRINT_ALL, "----- R_Init -----\n" );

	r_hdr_mgr_uptr.reset(rtcw::make_hdr_mgr());

	// clear all our internal state
	memset( &tr, 0, sizeof( tr ) );
	memset( &backEnd, 0, sizeof( backEnd ) );
	memset( &tess, 0, sizeof( tess ) );

	tess.xyz =              tess_xyz;
	tess.texCoords0 =       tess_texCoords0;
	tess.texCoords1 =       tess_texCoords1;
	tess.indexes =          tess_indexes;
	tess.normal =           tess_normal;
	tess.vertexColors =     tess_vertexColors;

	tess.maxShaderVerts =       SHADER_MAX_VERTEXES;
	tess.maxShaderIndicies =    SHADER_MAX_INDEXES;

#ifndef RTCW_ET
	tess.vertexDlightBits = tess_vertexDlightBits;
#endif // RTCW_X

	// BBi
	//Swap_Init();
	// BBi

	if ((reinterpret_cast<uintptr_t>(tess.xyz) & 15) != 0) {
		Com_Printf( "WARNING: tess.xyz not 16 byte aligned\n" );
	}
	memset( tess.constantColor255, 255, sizeof( tess.constantColor255 ) );

	//
	// init function tables
	//
	for ( i = 0; i < FUNCTABLE_SIZE; i++ )
	{
		tr.sinTable[i]      = c::sin( DEG2RAD( i * 360.0f / ( ( float ) ( FUNCTABLE_SIZE - 1 ) ) ) );
		tr.squareTable[i]   = ( i < FUNCTABLE_SIZE / 2 ) ? 1.0f : -1.0f;
		tr.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
		tr.inverseSawToothTable[i] = 1.0f - tr.sawToothTable[i];

		if ( i < FUNCTABLE_SIZE / 2 ) {
			if ( i < FUNCTABLE_SIZE / 4 ) {
				tr.triangleTable[i] = ( float ) i / ( FUNCTABLE_SIZE / 4 );
			} else
			{
				tr.triangleTable[i] = 1.0f - tr.triangleTable[i - FUNCTABLE_SIZE / 4];
			}
		} else
		{
			tr.triangleTable[i] = -tr.triangleTable[i - FUNCTABLE_SIZE / 2];
		}
	}

#if !defined RTCW_SP
	// Ridah, init the virtual memory
	R_Hunk_Begin();
#endif // RTCW_XX

	R_InitFogTable();

	R_NoiseInit();

	R_Register();

#if defined RTCW_SP
	// Ridah, init the virtual memory
	R_Hunk_Begin();
#endif // RTCW_XX

	max_polys = r_maxpolys->integer;
	if ( max_polys < MAX_POLYS ) {
		max_polys = MAX_POLYS;
	}

	max_polyverts = r_maxpolyverts->integer;
	if ( max_polyverts < MAX_POLYVERTS ) {
		max_polyverts = MAX_POLYVERTS;
	}

#if 0
//	backEndData[0] = ri.Hunk_Alloc( sizeof( *backEndData[0] ), h_low );
	backEndData[0] = static_cast<backEndData_t*> (ri.Hunk_Alloc( sizeof( *backEndData[0] ) + sizeof( srfPoly_t ) * max_polys + sizeof( polyVert_t ) * max_polyverts, h_low ));

	if ( r_smp->integer ) {
//		backEndData[1] = ri.Hunk_Alloc( sizeof( *backEndData[1] ), h_low );
		backEndData[1] = static_cast<backEndData_t*> (ri.Hunk_Alloc( sizeof( *backEndData[1] ) + sizeof( srfPoly_t ) * max_polys + sizeof( polyVert_t ) * max_polyverts, h_low ));
	} else {
		backEndData[1] = NULL;
	}
#endif // 0

	backEndData = static_cast<backEndData_t*>(ri.Hunk_Alloc(
		sizeof(*backEndData) + (sizeof(srfPoly_t) * max_polys) +
		(sizeof(polyVert_t) * max_polyverts), h_low));

	R_ToggleSmpFrame();

	InitOpenGL();

	R_InitImages();

	R_InitShaders();

	R_InitSkins();

	R_ModelInit();

	R_InitFreeType();

#if defined RTCW_SP
	RB_ZombieFXInit();
#endif // RTCW_XX

	err = glGetError();
	if ( err != GL_NO_ERROR ) {
		ri.Printf( PRINT_ALL, "glGetError() = 0x%x\n", err );
	}

	ri.Printf( PRINT_ALL, "----- finished R_Init -----\n" );
}

#if defined RTCW_ET
void R_PurgeCache( void ) {
	R_PurgeShaders( 9999999 );
	R_PurgeBackupImages( 9999999 );
	R_PurgeModels( 9999999 );
}
#endif // RTCW_XX

// BBi
static void r_shutdown_programs ()
{
	delete ogl_tess_program;
	ogl_tess_program = NULL;
}
// BBi

/*
===============
RE_Shutdown
===============
*/
void RE_Shutdown( qboolean destroyWindow ) {

	ri.Printf( PRINT_ALL, "RE_Shutdown( %i )\n", destroyWindow );

	ri.Cmd_RemoveCommand( "modellist" );
	ri.Cmd_RemoveCommand( "screenshotJPEG" );
	ri.Cmd_RemoveCommand( "screenshot" );
	ri.Cmd_RemoveCommand( "imagelist" );
	ri.Cmd_RemoveCommand( "shaderlist" );
	ri.Cmd_RemoveCommand( "skinlist" );
	ri.Cmd_RemoveCommand( "gfxinfo" );
	ri.Cmd_RemoveCommand( "modelist" );
	ri.Cmd_RemoveCommand( "shaderstate" );
	ri.Cmd_RemoveCommand( "taginfo" );

	// Ridah
	ri.Cmd_RemoveCommand( "cropimages" );
	// done.

	// BBi
	ri.Cmd_RemoveCommand ("r_reload_programs");
	// BBi

	R_ShutdownCommandBuffers();

	// Ridah, keep a backup of the current images if possible
	// clean out any remaining unused media from the last backup

#if !defined RTCW_ET
	R_PurgeShaders( 9999999 );
	R_PurgeBackupImages( 9999999 );
	R_PurgeModels( 9999999 );
#else
	R_PurgeCache();
#endif // RTCW_XX

	if ( r_cache->integer ) {
		if ( tr.registered ) {
			if ( destroyWindow ) {
				R_SyncRenderThread();
				R_ShutdownCommandBuffers();
				R_DeleteTextures();
			} else {
				// backup the current media
				R_ShutdownCommandBuffers();

				R_BackupModels();
				R_BackupShaders();
				R_BackupImages();
			}
		}
	} else if ( tr.registered ) {
		R_SyncRenderThread();
		R_ShutdownCommandBuffers();
		R_DeleteTextures();
	}

	R_DoneFreeType();

	// BBi
	if (!glConfigEx.is_path_ogl_1_x ()) {
		r_shutdown_programs ();
		r_tess_uninitialize ();
		r_terminate_offscreen();
	}
	// BBi

	// shut down platform specific OpenGL stuff
	if ( destroyWindow ) {
		GLimp_Shutdown();

		// Ridah, release the virtual memory
		R_Hunk_End();
		R_FreeImageBuffer();

#if defined RTCW_SP
		//ri.Tag_Free();	// wipe all render alloc'd zone memory
#else
		ri.Tag_Free();  // wipe all render alloc'd zone memory
#endif // RTCW_XX

	}

	// BBi
	r_hdr_mgr_uptr.reset();
	// BBi

	tr.registered = qfalse;
}


/*
=============
RE_EndRegistration

Touch all images to make sure they are resident
=============
*/
void RE_EndRegistration( void ) {
	R_SyncRenderThread();
	if ( !Sys_LowPhysicalMemory() ) {

#if !defined RTCW_ET
		RB_ShowImages();
#else
//		RB_ShowImages();
#endif // RTCW_XX

	}
}

#if defined RTCW_ET
void R_DebugPolygon( int color, int numPoints, float *points );
#endif // RTCW_XX

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
refexport_t *GetRefAPI( int apiVersion, refimport_t *rimp ) {
	static refexport_t re;

	ri = *rimp;

	memset( &re, 0, sizeof( re ) );

	if ( apiVersion != REF_API_VERSION ) {
		ri.Printf( PRINT_ALL, "Mismatched REF_API_VERSION: expected %i, got %i\n",
				   REF_API_VERSION, apiVersion );
		return NULL;
	}

	// the RE_ functions are Renderer Entry points

	re.Shutdown = RE_Shutdown;

	re.BeginRegistration = RE_BeginRegistration;
	re.RegisterModel    = RE_RegisterModel;
	re.RegisterSkin     = RE_RegisterSkin;
//----(SA) added
	re.GetSkinModel         = RE_GetSkinModel;
	re.GetShaderFromModel   = RE_GetShaderFromModel;
//----(SA) end
	re.RegisterShader   = RE_RegisterShader;
	re.RegisterShaderNoMip = RE_RegisterShaderNoMip;
	re.LoadWorld        = RE_LoadWorldMap;
	re.SetWorldVisData  = RE_SetWorldVisData;
	re.EndRegistration  = RE_EndRegistration;

	re.BeginFrame       = RE_BeginFrame;
	re.EndFrame         = RE_EndFrame;

	re.MarkFragments    = R_MarkFragments;

#if defined RTCW_ET
	re.ProjectDecal     = RE_ProjectDecal;
	re.ClearDecals      = RE_ClearDecals;
#endif // RTCW_XX

	re.LerpTag          = R_LerpTag;
	re.ModelBounds      = R_ModelBounds;

	re.ClearScene       = RE_ClearScene;
	re.AddRefEntityToScene = RE_AddRefEntityToScene;
	re.AddPolyToScene   = RE_AddPolyToScene;
	// Ridah
	re.AddPolysToScene  = RE_AddPolysToScene;
	// done.
	re.AddLightToScene  = RE_AddLightToScene;
//----(SA)
	re.AddCoronaToScene = RE_AddCoronaToScene;
	re.SetFog           = R_SetFog;
//----(SA)
	re.RenderScene      = RE_RenderScene;

#if defined RTCW_ET
	re.SaveViewParms    = RE_SaveViewParms;
	re.RestoreViewParms = RE_RestoreViewParms;
#endif // RTCW_XX

	re.SetColor         = RE_SetColor;
	re.DrawStretchPic   = RE_StretchPic;

#if !defined RTCW_SP
	re.DrawRotatedPic   = RE_RotatedPic;        // NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
	re.Add2dPolys       = RE_2DPolyies;
#endif // RTCW_XX

	re.DrawStretchPicGradient   = RE_StretchPicGradient;
	re.DrawStretchRaw   = RE_StretchRaw;
	re.UploadCinematic  = RE_UploadCinematic;
	re.RegisterFont     = RE_RegisterFont;
	re.RemapShader      = R_RemapShader;
	re.GetEntityToken   = R_GetEntityToken;

#if defined RTCW_SP
#ifdef BLAH // MrE __USEA3D
	re.A3D_RenderGeometry = RE_A3D_RenderGeometry;
#endif

	// RF
	re.ZombieFXAddNewHit = RB_ZombieFXAddNewHit;
#endif // RTCW_XX

#if defined RTCW_ET
	re.DrawDebugPolygon = R_DebugPolygon;
	re.DrawDebugText    = R_DebugText;

	re.AddPolyBufferToScene =   RE_AddPolyBufferToScene;

	re.SetGlobalFog     = RE_SetGlobalFog;

	re.inPVS = R_inPVS;

	re.purgeCache       = R_PurgeCache;

	//bani
	re.LoadDynamicShader = RE_LoadDynamicShader;
	re.GetTextureId = R_GetTextureId;
	// fretn
	re.RenderToTexture = RE_RenderToTexture;
	//bani
	re.Finish = RE_Finish;
#endif // RTCW_XX


	return &re;
}

