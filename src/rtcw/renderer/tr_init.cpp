/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// tr_init.c -- functions that are not called every frame

#include "tr_local.h"


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


GlConfigEx glConfigEx;

rtcw::OglTessState ogl_tess_state;

GLuint ogl_tess_vbo = 0;
int ogl_tess_base_vertex = 0;

rtcw::OglTessProgram* ogl_tess_program = NULL;

OglTessLayout ogl_tess2;
GLuint ogl_tess2_vbo = 0;
int ogl_tess2_base_vertex = 0;

rtcw::OglMatrixStack ogl_model_view_stack(rtcw::OglMatrixStack::model_view_max_depth);
rtcw::OglMatrixStack ogl_projection_stack(rtcw::OglMatrixStack::projection_max_depth);
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

#if FIXME
#if defined RTCW_SP
cvar_t  *r_fullbright;
#else
//cvar_t	*r_fullbright; // JPW NERVE removed per atvi request
#endif // RTCW_XX
#else
cvar_t  *r_fullbright;
#endif // FIXME

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

	if (ogl_tess_program != NULL)
	{
		is_try_successfull &= ogl_tess_program->try_reload();
	}
	else
	{
		is_try_successfull = false;
		ri.Printf(PRINT_ALL, "Out of memory.\n");
	}

	if (is_try_successfull)
	{
		ogl_tess_program->reload();
	}

	ogl_tess_state.set_program(ogl_tess_program);
	ogl_tess_state.invalidate_and_commit();

	ri.Printf(PRINT_ALL, "======== GLSL (debug) ========\n");
}


static const char* r_get_embeded_vertex_shader()
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
		"\n"
		"//#version 110\n"
		"\n"
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
		"}\n";

	return result;
}

static const char* r_get_embeded_fragment_shader()
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
		"\n"
		"//#version 110\n"
		"\n"
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
		"    gl_FragColor = apply_gamma(frag_color);\n"
		"}\n"
		"\n"
	;

	return result;
}

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

	rtcw::OglTessProgram tess_program(r_get_embeded_vertex_shader(), r_get_embeded_fragment_shader());

	ri.Printf(PRINT_ALL, "\n======== GLSL probe ========\n");
	ri.Printf(PRINT_ALL, "%s...\n", "Trying to reload programs");

	is_try_successfull &= tess_program.try_reload();

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
		ogl_tess_program = new rtcw::OglTessProgram(r_get_embeded_vertex_shader(), r_get_embeded_fragment_shader());
	}

	if (ogl_tess_program != NULL)
	{
		is_try_successfull &= ogl_tess_program->try_reload();
	}
	else
	{
		is_try_successfull = false;
		ri.Printf(PRINT_ALL, "Out of memory.\n");
	}

	if (is_try_successfull)
	{
		ogl_tess_program->reload();
	}

	ogl_tess_state.set_program(ogl_tess_program);
	ogl_tess_state.invalidate_and_commit();

	ri.Printf(PRINT_ALL, "======== GLSL ========\n");
}

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
}

static void r_tess_uninitialize ()
{
	glDeleteBuffers (1, &ogl_tess_vbo);
	ogl_tess_vbo = 0;

	glDeleteBuffers (1, &ogl_tess2_vbo);
	ogl_tess2_vbo = 0;
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
	ogl_tess_state.set_default_values ();
	// BBi

	glClearDepth( 1.0f );

	glCullFace( GL_FRONT );

	// BBi
	if (!glConfigEx.is_path_ogl_1_x ()) {
		ogl_tess_state.primary_color.set (rtcw::cgm::Vec4(1.0F, 1.0F, 1.0F, 1.0F));
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
		ogl_tess_state.tex_2d[0].set (0);
		ogl_tess_state.tex_2d[1].set (1);
		ogl_tess_state.use_multitexturing.set (false);
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

// BBi
//	// ATI pn_triangles
//	if ( qglPNTrianglesiATI ) {
//		int maxtess;
//		// get max supported tesselation
//		qglGetIntegerv( GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI, (GLint*)&maxtess );
//		glConfig.ATIMaxTruformTess = maxtess;
//		// cap if necessary
//		if ( r_ati_truform_tess->value > maxtess ) {
//			ri.Cvar_Set( "r_ati_truform_tess", va( "%d", maxtess ) );
//		}
//
//		// set Wolf defaults
//		qglPNTrianglesiATI( GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI, r_ati_truform_tess->value );
//	}
// BBi

//----(SA)	end

	// BBi
	ogl_tess_state.commit_changes ();
	// BBi
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

// BBi
//#if defined RTCW_SP
//	ri.Printf( PRINT_ALL, "ATI truform: %s\n", enablestrings[qglPNTrianglesiATI != 0] );
//	if ( qglPNTrianglesiATI ) {
////DAJ bogus at this point		ri.Printf( PRINT_ALL, "MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI: %d\n", glConfig.ATIMaxTruformTess );
//		ri.Printf( PRINT_ALL, "Truform Tess: %d\n", r_ati_truform_tess->integer );
//		ri.Printf( PRINT_ALL, "Truform Point Mode: %s\n", r_ati_truform_pointmode->string );
//		ri.Printf( PRINT_ALL, "Truform Normal Mode: %s\n", r_ati_truform_normalmode->string );
//	}
//#endif // RTCW_XX
// BBi

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

