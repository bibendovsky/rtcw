/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef __TR_TYPES_H
#define __TR_TYPES_H


#define MAX_CORONAS     32          //----(SA)	not really a reason to limit this other than trying to keep a reasonable count
#define MAX_DLIGHTS     32          // can't be increased, because bit flags are used on surfaces
#define MAX_ENTITIES    1023        // can't be increased without changing drawsurf bit packing

// renderfx flags
#define RF_MINLIGHT         1       // allways have some light (viewmodel, some items)
#define RF_THIRD_PERSON     2       // don't draw through eyes, only mirrors (player bodies, chat sprites)
#define RF_FIRST_PERSON     4       // only draw through eyes (view weapon, damage blood blob)
#define RF_DEPTHHACK        8       // for view weapon Z crunching

#if !defined RTCW_ET
#define RF_NOSHADOW         64      // don't add stencil shadows

#define RF_LIGHTING_ORIGIN  128     // use refEntity->lightingOrigin instead of refEntity->origin
									// for lighting.  This allows entities to sink into the floor
									// with their origin going solid, and allows all parts of a
									// player to get the same lighting
#define RF_SHADOW_PLANE     256     // use refEntity->shadowPlane
#define RF_WRAP_FRAMES      512     // mod the model frames by the maxframes to allow continuous
									// animation without needing to know the frame count

#define RF_HILIGHT          ( 1 << 8 )  // more than RF_MINLIGHT.  For when an object is "Highlighted" (looked at/training identification/etc)
#define RF_BLINK            ( 1 << 9 )  // eyes in 'blink' state
#else
#define RF_NOSHADOW         0x000010        // don't add stencil shadows

#define RF_LIGHTING_ORIGIN  0x000020        // use refEntity->lightingOrigin instead of refEntity->origin
											// for lighting.  This allows entities to sink into the floor
											// with their origin going solid, and allows all parts of a
											// player to get the same lighting
#define RF_SHADOW_PLANE     0x000040        // use refEntity->shadowPlane
#define RF_WRAP_FRAMES      0x000080        // mod the model frames by the maxframes to allow continuous
											// animation without needing to know the frame count
#define RF_HILIGHT          0x000100        // more than RF_MINLIGHT.  For when an object is "Highlighted" (looked at/training identification/etc)
#define RF_BLINK            0x000200        // eyes in 'blink' state
#endif // RTCW_XX

#if defined RTCW_ET
#define RF_FORCENOLOD       0x000400
#endif // RTCW_XX

// refdef flags
#define RDF_NOWORLDMODEL    1       // used for player configuration screen
#define RDF_HYPERSPACE      4       // teleportation effect

// Rafael
#define RDF_SKYBOXPORTAL    8

#if defined RTCW_SP
#define RDF_DRAWSKYBOX      16      // the above marks a scene as being a 'portal sky'.  this flag says to draw it or not
#endif // RTCW_XX

//----(SA)
#define RDF_UNDERWATER      ( 1 << 4 )  // so the renderer knows to use underwater fog when the player is underwater
#define RDF_DRAWINGSKY      ( 1 << 5 )
#define RDF_SNOOPERVIEW     ( 1 << 6 )  //----(SA)	added


typedef struct {
	vec3_t xyz;
	float st[2];
	byte modulate[4];
} polyVert_t;

typedef struct poly_s {
	qhandle_t hShader;
	int numVerts;
	polyVert_t          *verts;
} poly_t;

typedef enum {
	RT_MODEL,
	RT_POLY,
	RT_SPRITE,
	RT_SPLASH,  // ripple effect
	RT_BEAM,
	RT_RAIL_CORE,
	RT_RAIL_CORE_TAPER, // a modified core that creates a properly texture mapped core that's wider at one end
	RT_RAIL_RINGS,
	RT_LIGHTNING,
	RT_PORTALSURFACE,       // doesn't draw anything, just info for portals

	RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;

#define ZOMBIEFX_FADEOUT_TIME   10000

#define REFLAG_ONLYHAND     1   // only draw hand surfaces

#if defined RTCW_SP
#define REFLAG_ZOMBIEFX     2   // special post-tesselation processing for zombie skin
#define REFLAG_ZOMBIEFX2    4   // special post-tesselation processing for zombie skin
#endif // RTCW_XX

#define REFLAG_FORCE_LOD    8   // force a low lod
#define REFLAG_ORIENT_LOD   16  // on LOD switch, align the model to the player's camera
#define REFLAG_DEAD_LOD     32  // allow the LOD to go lower than recommended

#if defined RTCW_SP
#define REFLAG_SCALEDSPHERECULL 64  // on LOD switch, align the model to the player's camera
#define REFLAG_FULL_LOD     8   // force a FULL lod
#endif // RTCW_XX

typedef struct {
	refEntityType_t reType;
	int renderfx;

	qhandle_t hModel;               // opaque type outside refresh

	// most recent data
	vec3_t lightingOrigin;          // so multi-part models can be lit identically (RF_LIGHTING_ORIGIN)
	float shadowPlane;              // projection shadows go here, stencils go slightly lower

	vec3_t axis[3];                 // rotation vectors
	vec3_t torsoAxis[3];            // rotation vectors for torso section of skeletal animation
	qboolean nonNormalizedAxes;     // axis are not normalized, i.e. they have scale
	float origin[3];                // also used as MODEL_BEAM's "from"
	int frame;                      // also used as MODEL_BEAM's diameter

#if defined RTCW_ET
	qhandle_t frameModel;
#endif // RTCW_XX

	int torsoFrame;                 // skeletal torso can have frame independant of legs frame

#if defined RTCW_ET
	qhandle_t torsoFrameModel;
#endif // RTCW_XX

#if defined RTCW_SP
	vec3_t scale;       //----(SA)	added
#endif // RTCW_XX

	// previous data for frame interpolation
	float oldorigin[3];             // also used as MODEL_BEAM's "to"
	int oldframe;

#if defined RTCW_ET
	qhandle_t oldframeModel;
#endif // RTCW_XX

	int oldTorsoFrame;

#if defined RTCW_ET
	qhandle_t oldTorsoFrameModel;
#endif // RTCW_XX

	float backlerp;                 // 0.0 = current, 1.0 = old
	float torsoBacklerp;

	// texturing
	int skinNum;                    // inline skin index
	qhandle_t customSkin;           // NULL for default skin
	qhandle_t customShader;         // use one image for the entire thing

	// misc
	byte shaderRGBA[4];             // colors used by rgbgen entity shaders
	float shaderTexCoord[2];        // texture coordinates used by tcMod entity modifiers
	float shaderTime;               // subtracted from refdef time to control effect start times

	// extra sprite information
	float radius;
	float rotation;

	// Ridah
	vec3_t fireRiseDir;

	// Ridah, entity fading (gibs, debris, etc)
	int fadeStartTime, fadeEndTime;

	float hilightIntensity;         //----(SA)	added

	int reFlags;

	int entityNum;                  // currentState.number, so we can attach rendering effects to specific entities (Zombie)

} refEntity_t;

//----(SA)

//                                                                  //
// WARNING:: synch FOG_SERVER in sv_ccmds.c if you change anything	//
//                                                                  //
typedef enum {
	FOG_NONE,       //	0

	FOG_SKY,        //	1	fog values to apply to the sky when using density fog for the world (non-distance clipping fog) (only used if(glfogsettings[FOG_MAP].registered) or if(glfogsettings[FOG_MAP].registered))
	FOG_PORTALVIEW, //	2	used by the portal sky scene
	FOG_HUD,        //	3	used by the 3D hud scene

	//		The result of these for a given frame is copied to the scene.glFog when the scene is rendered

	// the following are fogs applied to the main world scene
	FOG_MAP,        //	4	use fog parameter specified using the "fogvars" in the sky shader
	FOG_WATER,      //	5	used when underwater
	FOG_SERVER,     //	6	the server has set my fog (probably a target_fog) (keep synch in sv_ccmds.c !!!)
	FOG_CURRENT,    //	7	stores the current values when a transition starts
	FOG_LAST,       //	8	stores the current values when a transition starts
	FOG_TARGET,     //	9	the values it's transitioning to.

	FOG_CMD_SWITCHFOG,  // 10	transition to the fog specified in the second parameter of R_SetFog(...) (keep synch in sv_ccmds.c !!!)

	NUM_FOGS
} glfogType_t;


typedef struct {
	int mode;                   // GL_LINEAR, GL_EXP
	int hint;                   // GL_DONT_CARE
	int startTime;              // in ms
	int finishTime;             // in ms
	float color[4];
	float start;                // near
	float end;                  // far
	qboolean useEndForClip;     // use the 'far' value for the far clipping plane
	float density;              // 0.0-1.0
	qboolean registered;        // has this fog been set up?
	qboolean drawsky;           // draw skybox
	qboolean clearscreen;       // clear the GL color buffer

#if defined RTCW_SP
	int dirty;
#endif // RTCW_XX

} glfog_t;

//----(SA)	end


#define MAX_RENDER_STRINGS          8
#define MAX_RENDER_STRING_LENGTH    32

typedef struct {
	int x, y, width, height;
	float fov_x, fov_y;
	vec3_t vieworg;
	vec3_t viewaxis[3];             // transformation matrix

	int time;           // time in milliseconds for shader effects and other time dependent rendering issues
	int rdflags;                    // RDF_NOWORLDMODEL, etc

	// 1 bits will prevent the associated area from rendering at all
	byte areamask[MAX_MAP_AREA_BYTES];




	// text messages for deform text shaders
	char text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];


//----(SA)	added (needed to pass fog infos into the portal sky scene)
	glfog_t glfog;
//----(SA)	end

} refdef_t;


typedef enum {
	STEREO_CENTER,
	STEREO_LEFT,
	STEREO_RIGHT
} stereoFrame_t;


/*
** glconfig_t
**
** Contains variables specific to the OpenGL configuration
** being run right now.  These are constant once the OpenGL
** subsystem is initialized.
*/
typedef enum {
	TC_NONE,
	TC_S3TC,
	TC_EXT_COMP_S3TC,

// BBi
	TC_ARB
// BBi
} textureCompression_t;

typedef enum {
	GLDRV_ICD                  // driver is integrated with window system
								// WARNING: there are tests that check for
								// > GLDRV_ICD for minidriverness, so this
								// should always be the lowest value in this
								// enum set

	// BBi
	//GLDRV_STANDALONE,           // driver is a non-3Dfx standalone driver
	//GLDRV_VOODOO                // driver is a 3Dfx standalone driver
	// BBi
} glDriverType_t;

typedef enum {
	GLHW_GENERIC           // where everthing works the way it should

	// BBi
	//GLHW_3DFX_2D3D,         // Voodoo Banshee or Voodoo3, relevant since if this is
	//						// the hardware type then there can NOT exist a secondary
	//						// display adapter
	//GLHW_RIVA128,           // where you can't interpolate alpha
	//GLHW_RAGEPRO,           // where you can't modulate alpha on alpha textures
	//GLHW_PERMEDIA2          // where you don't have src*dst
	// BBi

} glHardwareType_t;


// BBi
// For your mental safety do not touch this cursed structure.
// BBi
typedef struct {
	char renderer_string[MAX_STRING_CHARS];
	char vendor_string[MAX_STRING_CHARS];
	char version_string[MAX_STRING_CHARS];

	// this is actually too short for many current cards/drivers
	// (SA) doubled from 2x to 4x MAX_STRING_CHARS
	// TTimo - bumping, some cards have a big extension string
	char extensions_string[4 * MAX_STRING_CHARS];

	int maxTextureSize;                             // queried from GL
	int maxActiveTextures;                          // multitexture ability

	int colorBits, depthBits, stencilBits;

	glDriverType_t driverType;
	glHardwareType_t hardwareType;

	qboolean deviceSupportsGamma;
	textureCompression_t textureCompression;
	qboolean textureEnvAddAvailable;
	qboolean anisotropicAvailable;                  //----(SA)	added
	float maxAnisotropy;                            //----(SA)	added

	// vendor-specific support
	// NVidia

	qboolean NVFogAvailable;                    //----(SA)	added

	int NVFogMode;                                  //----(SA)	added
	// ATI
	int ATIMaxTruformTess;                          // for truform support
	int ATINormalMode;                          // for truform support
	int ATIPointMode;                           // for truform support

	int vidWidth, vidHeight;
	// aspect is the screen's physical width / height, which may be different
	// than scrWidth / scrHeight if the pixels are non-square
	// normal screens should be 4/3, but wide aspect monitors may be 16/9
	float windowAspect;

	int displayFrequency;

	// synonymous with "does rendering consume the entire screen?", therefore
	// a Voodoo or Voodoo2 will have this set to TRUE, as will a Win32 ICD that
	// used CDS.
	qboolean isFullscreen;
	qboolean stereoEnabled;
	qboolean smpActive;                     // dual processor

#if !defined RTCW_ET
	qboolean textureFilterAnisotropicAvailable;                 //DAJ
#endif // RTCW_XX

} glconfig_t;

// BBi
enum RendererPath {
	RENDERER_PATH_NONE,
	// Fixed OpenGL.
	RENDERER_PATH_OGL_1_X,
	// Programmable OpenGL (2.x desktop).
	RENDERER_PATH_OGL_2_X
}; // enum RendererPath

// Extends glconfig_t structure.
class GlConfigEx
{
public:
	bool is_native_resolution_;
	bool use_arb_multitexture_;
	bool use_ext_compiled_vertex_array_;
	bool use_ext_bgra_;
	bool use_arb_framebuffer_object_; // glGenerateMipmap, etc.
	bool use_arb_texture_non_power_of_two_;
	bool use_arb_draw_elements_base_vertex;
	bool has_swap_control_;
	bool has_adaptive_swap_control_;
	bool use_gl_arb_vertex_array_object;
	bool is_2_x_capable_;
	RendererPath renderer_path_;

	void reset()
	{
		is_native_resolution_ = false;
		use_arb_multitexture_ = false;
		use_ext_compiled_vertex_array_ = false;
		use_ext_bgra_ = false;
		use_arb_framebuffer_object_ = false;
		use_arb_texture_non_power_of_two_ = false;
		use_arb_draw_elements_base_vertex = false;
		has_swap_control_ = false;
		has_adaptive_swap_control_ = false;
		is_2_x_capable_ = false;
		renderer_path_ = RENDERER_PATH_NONE;
	}

	bool is_path_ogl_1_x() const
	{
		return renderer_path_ == RENDERER_PATH_OGL_1_X;
	}

	bool is_path_ogl_2_x() const
	{
		return renderer_path_ == RENDERER_PATH_OGL_2_X;
	}
}; // GlConfigEx
// BBi

#if !_WIN32

#define _3DFX_DRIVER_NAME   "libMesaVoodooGL.so.3.1"

#if defined RTCW_MP
// show_bug.cgi?id=524
#endif // RTCW_XX

#define OPENGL_DRIVER_NAME  "libGL.so.1"

#else

#define _3DFX_DRIVER_NAME   "3dfxvgl"
#define OPENGL_DRIVER_NAME  "opengl32"
#define WICKED3D_V5_DRIVER_NAME "gl/openglv5.dll"
#define WICKED3D_V3_DRIVER_NAME "gl/openglv3.dll"

#endif  // !defined _WIN32


#if defined RTCW_ET
// =========================================
// Gordon, these MUST NOT exceed the values for SHADER_MAX_VERTEXES/SHADER_MAX_INDEXES
#define MAX_PB_VERTS    1025
#define MAX_PB_INDICIES ( MAX_PB_VERTS * 6 )

typedef struct polyBuffer_s {
	vec4_t xyz[MAX_PB_VERTS];
	vec2_t st[MAX_PB_VERTS];
	byte color[MAX_PB_VERTS][4];
	int numVerts;

	int indicies[MAX_PB_INDICIES];
	int numIndicies;

	qhandle_t shader;
} polyBuffer_t;
// =========================================
#endif // RTCW_XX

#endif  // __TR_TYPES_H
