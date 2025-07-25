/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/



#ifndef TR_LOCAL_H
#define TR_LOCAL_H

#include "q_shared.h"
#include "qfiles.h"
#include "qcommon.h"
#include "tr_public.h"
#include "qgl.h"

// BBi GLSL
#include "rtcw_cgm_vec.h"
#include "rtcw_ogl_tess_program.h"
#include "rtcw_ogl_tess_state.h"
#include "rtcw_ogl_matrix_stack.h"
// BBi

#define GL_INDEX_TYPE       GL_UNSIGNED_INT
typedef unsigned int glIndex_t;

// BBi
//// everything that is needed by the backend needs
//// to be double buffered to allow it to run in
//// parallel on a dual cpu machine
//#define SMP_FRAMES      2
// BBi

#if defined RTCW_SP
#define MAX_SHADERS             2048
#else
#define MAX_SHADERS             8192
#endif // RTCW_XX

#define MAX_SHADER_STATES 2048
#define MAX_STATES_PER_SHADER 32
#define MAX_STATE_NAME 32

// can't be increased without changing bit packing for drawsurfs


// a trRefEntity_t has all the information passed in by
// the client game, as well as some locally derived info

#if defined RTCW_ET
// ydnar: optimizing diffuse lighting calculation with a table lookup
#define ENTITY_LIGHT_STEPS      16
#endif // RTCW_XX

typedef struct {
	refEntity_t e;

	float axisLength;           // compensate for non-normalized axis

	qboolean needDlights;       // true for bmodels that touch a dlight
	qboolean lightingCalculated;
	vec3_t lightDir;            // normalized direction towards light
	vec3_t ambientLight;        // color normalized to 0-255
	int ambientLightInt;            // 32 bit rgba packed
	vec3_t directedLight;

#if defined RTCW_ET
	int entityLightInt[ ENTITY_LIGHT_STEPS ];
#endif // RTCW_XX

	float brightness;
} trRefEntity_t;

typedef struct {
	vec3_t origin;              // in world coordinates
	vec3_t axis[3];             // orientation in world
	vec3_t viewOrigin;          // viewParms->or.origin in local coordinates
	float modelMatrix[16];
} orientationr_t;

typedef struct image_s {
	char imgName[MAX_QPATH];            // game path, including extension
	int width, height;                      // source image
	int uploadWidth, uploadHeight;          // after power of two and picmip but not including clamp to MAX_TEXTURE_SIZE
	GLuint texnum;                      // gl texture binding

	int frameUsed;                  // for texture usage in frame statistics

	int internalFormat;
	int TMU;                        // only needed for voodoo2

	qboolean mipmap;
	qboolean allowPicmip;

	int wrapClampMode;              // GL_CLAMP or GL_REPEAT

	int hash;           // for fast building of the backupHash

	struct image_s* next;
} image_t;

//===============================================================================

typedef enum {
	SS_BAD,
	SS_PORTAL,          // mirrors, portals, viewscreens
	SS_ENVIRONMENT,     // sky box
	SS_OPAQUE,          // opaque

	SS_DECAL,           // scorch marks, etc.
	SS_SEE_THROUGH,     // ladders, grates, grills that may have small blended edges
						// in addition to alpha test
	SS_BANNER,

	SS_FOG,

	SS_UNDERWATER,      // for items that should be drawn in front of the water plane

	SS_BLEND0,          // regular transparency and filters
	SS_BLEND1,          // generally only used for additive type effects
	SS_BLEND2,
	SS_BLEND3,

	SS_BLEND6,
	SS_STENCIL_SHADOW,
	SS_ALMOST_NEAREST,  // gun smoke puffs

	SS_NEAREST          // blood blobs
} shaderSort_t;


#define MAX_SHADER_STAGES 8

typedef enum {
	GF_NONE,

	GF_SIN,
	GF_SQUARE,
	GF_TRIANGLE,
	GF_SAWTOOTH,
	GF_INVERSE_SAWTOOTH,

	GF_NOISE

} genFunc_t;


typedef enum {
	DEFORM_NONE,
	DEFORM_WAVE,
	DEFORM_NORMALS,
	DEFORM_BULGE,
	DEFORM_MOVE,
	DEFORM_PROJECTION_SHADOW,
	DEFORM_AUTOSPRITE,
	DEFORM_AUTOSPRITE2,
	DEFORM_TEXT0,
	DEFORM_TEXT1,
	DEFORM_TEXT2,
	DEFORM_TEXT3,
	DEFORM_TEXT4,
	DEFORM_TEXT5,
	DEFORM_TEXT6,
	DEFORM_TEXT7
} deform_t;

typedef enum {
	AGEN_IDENTITY,
	AGEN_SKIP,
	AGEN_ENTITY,
	AGEN_ONE_MINUS_ENTITY,
	AGEN_NORMALZFADE,   // Ridah
	AGEN_VERTEX,
	AGEN_ONE_MINUS_VERTEX,
	AGEN_LIGHTING_SPECULAR,
	AGEN_WAVEFORM,
	AGEN_PORTAL,
	AGEN_CONST
} alphaGen_t;

typedef enum {
	CGEN_BAD,
	CGEN_IDENTITY_LIGHTING, // tr.identityLight
	CGEN_IDENTITY,          // always (1,1,1,1)
	CGEN_ENTITY,            // grabbed from entity's modulate field
	CGEN_ONE_MINUS_ENTITY,  // grabbed from 1 - entity.modulate
	CGEN_EXACT_VERTEX,      // tess.vertexColors
	CGEN_VERTEX,            // tess.vertexColors * tr.identityLight
	CGEN_ONE_MINUS_VERTEX,
	CGEN_WAVEFORM,          // programmatically generated
	CGEN_LIGHTING_DIFFUSE,
	CGEN_FOG,               // standard fog
	CGEN_CONST              // fixed color
} colorGen_t;

typedef enum {
	TCGEN_BAD,
	TCGEN_IDENTITY,         // clear to 0,0
	TCGEN_LIGHTMAP,
	TCGEN_TEXTURE,
	TCGEN_ENVIRONMENT_MAPPED,
	TCGEN_FIRERISEENV_MAPPED,
	TCGEN_FOG,
	TCGEN_VECTOR            // S and T from world coordinates
} texCoordGen_t;

typedef enum {
	ACFF_NONE,
	ACFF_MODULATE_RGB,
	ACFF_MODULATE_RGBA,
	ACFF_MODULATE_ALPHA
} acff_t;

typedef struct {
	genFunc_t func;

	float base;
	float amplitude;
	float phase;
	float frequency;
} waveForm_t;

#define TR_MAX_TEXMODS 4

typedef enum {
	TMOD_NONE,
	TMOD_TRANSFORM,
	TMOD_TURBULENT,
	TMOD_SCROLL,
	TMOD_SCALE,
	TMOD_STRETCH,
	TMOD_ROTATE,
	TMOD_ENTITY_TRANSLATE,
	TMOD_SWAP
} texMod_t;

#define MAX_SHADER_DEFORMS  3
typedef struct {
	deform_t deformation;               // vertex coordinate modification type

	vec3_t moveVector;
	waveForm_t deformationWave;
	float deformationSpread;

	float bulgeWidth;
	float bulgeHeight;
	float bulgeSpeed;
} deformStage_t;


typedef struct {
	texMod_t type;

	// used for TMOD_TURBULENT and TMOD_STRETCH
	waveForm_t wave;

	// used for TMOD_TRANSFORM
	float matrix[2][2];                 // s' = s * m[0][0] + t * m[1][0] + trans[0]
	float translate[2];                 // t' = s * m[0][1] + t * m[0][1] + trans[1]

	// used for TMOD_SCALE
	float scale[2];                     // s *= scale[0]
										// t *= scale[1]

	// used for TMOD_SCROLL
	float scroll[2];                    // s' = s + scroll[0] * time
										// t' = t + scroll[1] * time

	// + = clockwise
	// - = counterclockwise
	float rotateSpeed;

} texModInfo_t;


// RF increased this for onfire animation
//#define	MAX_IMAGE_ANIMATIONS	8
#define MAX_IMAGE_ANIMATIONS    16

#if defined RTCW_ET
// Arnout: FIXME change the is* qbooleans to a type index
#endif // RTCW_XX

typedef struct {
	image_t         *image[MAX_IMAGE_ANIMATIONS];
	int numImageAnimations;
	float imageAnimationSpeed;

	texCoordGen_t tcGen;
	vec3_t tcGenVectors[2];

	int numTexMods;
	texModInfo_t    *texMods;

	int videoMapHandle;
	qboolean isLightmap;

#if !defined RTCW_ET
	qboolean vertexLightmap;
#endif // RTCW_XX

	qboolean isVideoMap;
} textureBundle_t;

#define NUM_TEXTURE_BUNDLES 2

typedef struct {
	qboolean active;

	textureBundle_t bundle[NUM_TEXTURE_BUNDLES];

	waveForm_t rgbWave;
	colorGen_t rgbGen;

	waveForm_t alphaWave;
	alphaGen_t alphaGen;

	byte constantColor[4];                      // for CGEN_CONST and AGEN_CONST

	unsigned stateBits;                         // GLS_xxxx mask

	acff_t adjustColorsForFog;

	// Ridah
	float zFadeBounds[2];

	qboolean isDetail;
	qboolean isFogged;              // used only for shaders that have fog disabled, so we can enable it for individual stages
} shaderStage_t;

struct shaderCommands_s;

#define LIGHTMAP_2D         -4      // shader is for 2D rendering
#define LIGHTMAP_BY_VERTEX  -3      // pre-lit triangle models
#define LIGHTMAP_WHITEIMAGE -2
#define LIGHTMAP_NONE       -1

typedef enum {
	CT_FRONT_SIDED,
	CT_BACK_SIDED,
	CT_TWO_SIDED
} cullType_t;

typedef enum {
	FP_NONE,        // surface is translucent and will just be adjusted properly
	FP_EQUAL,       // surface is opaque but possibly alpha tested
	FP_LE           // surface is trnaslucent, but still needs a fog pass (fog surface)
} fogPass_t;

typedef struct {
	float cloudHeight;
	image_t     *outerbox[6], *innerbox[6];
} skyParms_t;

typedef struct {
	vec3_t color;
	float depthForOpaque;

#if defined RTCW_ET
	unsigned colorInt;                  // in packed byte format
	float tcScale;                      // texture coordinate vector scales
#endif // RTCW_XX

} fogParms_t;


typedef struct shader_s {
	char name[MAX_QPATH];               // game path, including extension
	int lightmapIndex;                  // for a shader to match, both name and lightmapIndex must match

	int index;                          // this shader == tr.shaders[index]
	int sortedIndex;                    // this shader == tr.sortedShaders[sortedIndex]

	float sort;                         // lower numbered shaders draw before higher numbered

	qboolean defaultShader;             // we want to return index 0 if the shader failed to
										// load for some reason, but R_FindShader should
										// still keep a name allocated for it, so if
										// something calls RE_RegisterShader again with
										// the same name, we don't try looking for it again

	qboolean explicitlyDefined;         // found in a .shader file

	int surfaceFlags;                   // if explicitlyDefined, this will have SURF_* flags
	int contentFlags;

	qboolean entityMergable;            // merge across entites optimizable (smoke, blood)

	qboolean isSky;
	skyParms_t sky;
	fogParms_t fogParms;

	float portalRange;                  // distance to fog out at

#if defined RTCW_ET
	vec4_t distanceCull;                // ydnar: opaque alpha range for foliage (inner, outer, alpha threshold, 1/(outer-inner))
#endif // RTCW_XX

	int multitextureEnv;                // 0, GL_MODULATE, GL_ADD (FIXME: put in stage)

	cullType_t cullType;                // CT_FRONT_SIDED, CT_BACK_SIDED, or CT_TWO_SIDED
	qboolean polygonOffset;             // set for decals and other items that must be offset
	qboolean noMipMaps;                 // for console fonts, 2D elements, etc.
	qboolean noPicMip;                  // for images that must always be full resolution

#if defined RTCW_SP
	qboolean characterMip;              // use r_picmip2 rather than r_picmip
#endif // RTCW_XX

	fogPass_t fogPass;                  // draw a blended pass, possibly with depth test equals

	qboolean needsNormal;               // not all shaders will need all data to be gathered
	qboolean needsST1;
	qboolean needsST2;
	qboolean needsColor;

	// Ridah
	qboolean noFog;

	int numDeforms;
	deformStage_t deforms[MAX_SHADER_DEFORMS];

	int numUnfoggedPasses;
	shaderStage_t   *stages[MAX_SHADER_STAGES];

	void ( *optimalStageIteratorFunc )( void );

	float clampTime;                                    // time this shader is clamped to
	float timeOffset;                                   // current time offset for this shader

	int numStates;                                      // if non-zero this is a state shader
	struct shader_s *currentShader;                     // current state if this is a state shader
	struct shader_s *parentShader;                      // current state if this is a state shader
	int currentState;                                   // current state index for cycle purposes
	int32_t expireTime;                                    // time in milliseconds this expires

	struct shader_s *remappedShader;                    // current shader this one is remapped too

	int shaderStates[MAX_STATES_PER_SHADER];            // index to valid shader states

	struct shader_s *next;
} shader_t;

typedef struct corona_s {
	vec3_t origin;
	vec3_t color;               // range from 0.0 to 1.0, should be color normalized
	vec3_t transformed;         // origin in local coordinate system
	float scale;                // uses r_flaresize as the baseline (1.0)
	int id;

// BBi
//#if defined RTCW_SP
//	int flags;                  // '1' is 'visible'
//								// still send the corona request, even if not visible, for proper fading
//#else
//	qboolean visible;           // still send the corona request, even if not visible, for proper fading
//#endif // RTCW_XX
	int flags; // '1' is 'visible'
			   // still send the corona request, even if not visible,
			   // for proper fading
// BBi
} corona_t;

typedef struct dlight_s {
	vec3_t origin;
	vec3_t color;               // range from 0.0 to 1.0, should be color normalized
	float radius;

#if defined RTCW_ET
	float radiusInverseCubed;       // ydnar: attenuation optimization
	float intensity;                // 1.0 = fullbright, > 1.0 = overbright
	shader_t    *shader;
	int flags;
#endif // RTCW_XX

	vec3_t transformed;         // origin in local coordinate system

#if !defined RTCW_ET
	// Ridah
	int overdraw;
	// done.

	shader_t    *dlshader;  //----(SA) adding a shader to dlights, so, if desired, we can change the blend or texture of a dlight

	qboolean forced;        //----(SA)	use this dlight when r_dynamiclight is either 1 or 2 (rather than just 1) for "important" gameplay lights (alarm lights, etc)
	//done
#endif // RTCW_XX

} dlight_t;

#if defined RTCW_ET
// ydnar: decal projection
typedef struct decalProjector_s
{
	shader_t    *shader;
	byte color[ 4 ];
	int fadeStartTime, fadeEndTime;
	vec3_t mins, maxs;
	vec3_t center;
	float radius, radius2;
	qboolean omnidirectional;
	int numPlanes;                  // either 5 or 6, for quad or triangle projectors
	vec4_t planes[ 6 ];
	vec4_t texMat[ 3 ][ 2 ];
}
decalProjector_t;
#endif // RTCW_XX

// trRefdef_t holds everything that comes in refdef_t,
// as well as the locally generated scene information
typedef struct {
	int x, y, width, height;
	float fov_x, fov_y;
	vec3_t vieworg;
	vec3_t viewaxis[3];             // transformation matrix

	int time;                       // time in milliseconds for shader effects and other time dependent rendering issues
	int rdflags;                    // RDF_NOWORLDMODEL, etc

	// 1 bits will prevent the associated area from rendering at all
	byte areamask[MAX_MAP_AREA_BYTES];
	qboolean areamaskModified;      // qtrue if areamask changed since last scene

	float floatTime;                // tr.refdef.time / 1000.0

	// text messages for deform text shaders
	char text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];

	int num_entities;
	trRefEntity_t   *entities;

#if defined RTCW_ET
	int dlightBits;                 // ydnar: optimization
#endif // RTCW_XX

	int num_dlights;

#if !defined RTCW_ET
	struct dlight_s *dlights;
#else
	dlight_t    *dlights;
#endif // RTCW_XX

	int num_coronas;

#if !defined RTCW_ET
	struct corona_s *coronas;
#else
	corona_t    *coronas;
#endif // RTCW_XX

	int numPolys;
	struct srfPoly_s    *polys;

#if defined RTCW_ET
	int numPolyBuffers;
	struct srfPolyBuffer_s  *polybuffers;

	int decalBits;                  // ydnar: optimization
	int numDecalProjectors;
	decalProjector_t    *decalProjectors;

	int numDecals;
	struct srfDecal_s   *decals;
#endif // RTCW_XX

	int numDrawSurfs;
	struct drawSurf_s   *drawSurfs;
} trRefdef_t;


//=================================================================================

// skins allow models to be retextured without modifying the model file
typedef struct {
	char name[MAX_QPATH];

#if defined RTCW_ET
	int hash;
#endif // RTCW_XX

	shader_t    *shader;
} skinSurface_t;

//----(SA) modified
#define MAX_PART_MODELS 5

typedef struct {
	char type[MAX_QPATH];           // md3_lower, md3_lbelt, md3_rbelt, etc.
	char model[MAX_QPATH];          // lower.md3, belt1.md3, etc.

#if defined RTCW_ET
	int hash;
#endif // RTCW_XX

} skinModel_t;

typedef struct skin_s {
	char name[MAX_QPATH];               // game path, including extension
	int numSurfaces;
	int numModels;
	skinSurface_t   *surfaces[MD3_MAX_SURFACES];
	skinModel_t     *models[MAX_PART_MODELS];

#if !defined RTCW_ET
	vec3_t scale;       //----(SA)	added
#endif // RTCW_XX

} skin_t;
//----(SA) end

typedef struct {

#if defined RTCW_ET
	int modelNum;                   // ydnar: bsp model the fog belongs to
#endif // RTCW_XX

	int originalBrushNumber;
	vec3_t bounds[2];

#if !defined RTCW_ET
	unsigned colorInt;                  // in packed byte format
	float tcScale;                      // texture coordinate vector scales
#endif // RTCW_XX

#if defined RTCW_ET
	shader_t    *shader;            // fog shader to get colorInt and tcScale from
#endif // RTCW_XX

	fogParms_t parms;

	// for clipping distance in fog when outside
	qboolean hasSurface;
	float surface[4];
} fog_t;

typedef struct {

// BBi
//#if !defined RTCW_ET
//	orientationr_t  or;
//#else
//	orientationr_t orientation;
//#endif // RTCW_XX
	orientationr_t orientation;
// BBi

	orientationr_t world;
	vec3_t pvsOrigin;               // may be different than or.origin for portals
	qboolean isPortal;              // true if this view is through a portal
	qboolean isMirror;              // the portal is a mirror, invert the face culling
	int frameSceneNum;              // copied from tr.frameSceneNum
	int frameCount;                 // copied from tr.frameCount
	cplane_t portalPlane;           // clip anything behind this if mirroring
	int viewportX, viewportY, viewportWidth, viewportHeight;
	float fovX, fovY;
	float projectionMatrix[16];

#if !defined RTCW_ET
	cplane_t frustum[4];
#else
	cplane_t frustum[5];            // ydnar: added farplane
#endif // RTCW_XX

	vec3_t visBounds[2];
	float zFar;

#if defined RTCW_SP
	int dirty;
#endif // RTCW_XX

	glfog_t glFog;                  // fog parameters	//----(SA)	added

} viewParms_t;


/*
==============================================================================

SURFACES

==============================================================================
*/

// any changes in surfaceType must be mirrored in rb_surfaceTable[]

#if !defined RTCW_ET
// NOTE: also mirror changes to max2skl.c
#else
// NOTE: also mirror changes to max2skl.c - Arnout: not anymore
#endif // RTCW_XX

typedef enum {
	SF_BAD,
	SF_SKIP,                // ignore
	SF_FACE,
	SF_GRID,
	SF_TRIANGLES,

#if defined RTCW_ET
	SF_FOLIAGE,
#endif // RTCW_XX

	SF_POLY,
	SF_MD3,
	SF_MDC,
	SF_MDS,

#if defined RTCW_ET
	SF_MDM,
#endif // RTCW_XX

	SF_FLARE,
	SF_ENTITY,              // beams, rails, lightning, etc that can be determined by entity
	SF_DISPLAY_LIST,

#if defined RTCW_ET
	SF_POLYBUFFER,
	SF_DECAL,               // ydnar: decal surfaces
#endif // RTCW_XX

	SF_NUM_SURFACE_TYPES,
	SF_MAX = 0xffffffff         // ensures that sizeof( surfaceType_t ) == sizeof( int )
} surfaceType_t;

typedef struct drawSurf_s {
	unsigned sort;                      // bit combination for fast compares
	surfaceType_t       *surface;       // any of surface*_t
} drawSurf_t;

#if !defined RTCW_ET
#define MAX_FACE_POINTS     64
#else
#define MAX_FACE_POINTS     1024
#endif // RTCW_XX

#define MAX_PATCH_SIZE      32          // max dimensions of a patch mesh in map file
#define MAX_GRID_SIZE       65          // max dimensions of a grid mesh in memory

typedef byte color4ub_t[4];

typedef struct color4ubhack_s {
	color4ub_t v;
} color4ubhack_t;

typedef struct vec4hack_s {
	vec4_t v;
} vec4hack_t;

typedef struct vec2hack_s {
	vec2_t v;
} vec2hack_t;

// when cgame directly specifies a polygon, it becomes a srfPoly_t
// as soon as it is called
typedef struct srfPoly_s {
	surfaceType_t surfaceType;
	qhandle_t hShader;
	int fogIndex;
	int numVerts;
	polyVert_t      *verts;
} srfPoly_t;

#if defined RTCW_ET
typedef struct srfPolyBuffer_s {
	surfaceType_t surfaceType;
	int fogIndex;
	polyBuffer_t*   pPolyBuffer;
} srfPolyBuffer_t;

// ydnar: decals
#define MAX_DECAL_VERTS         10  // worst case is triangle clipped by 6 planes
#define MAX_WORLD_DECALS        1024
#define MAX_ENTITY_DECALS       128
typedef struct srfDecal_s
{
	surfaceType_t surfaceType;
	int numVerts;
	polyVert_t verts[ MAX_DECAL_VERTS ];
}
srfDecal_t;
#endif // RTCW_XX

typedef struct srfDisplayList_s {
	surfaceType_t surfaceType;
	int listNum;
} srfDisplayList_t;


typedef struct srfFlare_s {
	surfaceType_t surfaceType;
	vec3_t origin;
	vec3_t normal;
	vec3_t color;
} srfFlare_t;

#if defined RTCW_ET
// ydnar: normal map drawsurfaces must match this header
typedef struct srfGeneric_s
{
	surfaceType_t surfaceType;

	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;

	// dynamic lighting information

// BBi
	//int dlightBits[ SMP_FRAMES ];
	int dlightBits;
// BBi
}
srfGeneric_t;
#endif // RTCW_XX

typedef struct srfGridMesh_s {
	surfaceType_t surfaceType;

#if defined RTCW_ET
	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;
#endif // RTCW_XX

	// dynamic lighting information

// BBi
	//int dlightBits[SMP_FRAMES];
	int dlightBits;
// BBi

#if !defined RTCW_ET
	// culling information
	vec3_t meshBounds[2];
	vec3_t localOrigin;
	float meshRadius;
#endif // RTCW_XX

	// lod information, which may be different
	// than the culling information to allow for
	// groups of curves that LOD as a unit
	vec3_t lodOrigin;
	float lodRadius;
	int lodFixed;
	int lodStitched;

	// vertexes
	int width, height;
	float           *widthLodError;
	float           *heightLodError;
	drawVert_t verts[1];            // variable sized
} srfGridMesh_t;



#define VERTEXSIZE  8

#if !defined RTCW_ET
typedef struct {
#else
typedef struct srfSurfaceFace_s
{
#endif // RTCW_XX

	surfaceType_t surfaceType;

#if defined RTCW_ET
	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
#endif // RTCW_XX

	cplane_t plane;

	// dynamic lighting information

// BBi
//	int dlightBits[SMP_FRAMES];
	int dlightBits;
// BBi

	// triangle definitions (no normals at points)
	int numPoints;
	int numIndices;
	int ofsIndices;
	float points[1][VERTEXSIZE];        // variable sized
										// there is a variable length list of indices here also
} srfSurfaceFace_t;


// misc_models in maps are turned into direct geometry by q3map

#if !defined RTCW_ET
typedef struct {
#else
typedef struct srfTriangles_s
{
#endif // RTCW_XX

	surfaceType_t surfaceType;

#if !defined RTCW_ET
	// dynamic lighting information

// BBi
//	int dlightBits[SMP_FRAMES];
	int dlightBits;
// BBi
#endif // RTCW_XX

	// culling information (FIXME: use this!)
	vec3_t bounds[2];

#if !defined RTCW_ET
	vec3_t localOrigin;
#endif // RTCW_XX

#if defined RTCW_ET
	vec3_t origin;
#endif // RTCW_XX

	float radius;

#if defined RTCW_ET
	cplane_t plane;

	// dynamic lighting information

// BBi
//	int dlightBits[ SMP_FRAMES ];
	int dlightBits;
// BBi
#endif // RTCW_XX

	// triangle definitions
	int numIndexes;
	int             *indexes;

	int numVerts;
	drawVert_t      *verts;
} srfTriangles_t;

#if defined RTCW_ET
// Gordon: Test
typedef struct srfTriangles2_s
{
	surfaceType_t surfaceType;

	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;

	// dynamic lighting information

// BBi
//	int dlightBits[ SMP_FRAMES ];
	int dlightBits;
// BBi

	// triangle definitions
	int numIndexes;
	int             *indexes;

	int numVerts;


	vec4hack_t*     xyz;
	vec2hack_t*     st;
	vec2hack_t*     lightmap;
	vec4hack_t*     normal;
	color4ubhack_t* color;
} srfTriangles2_t;
//

// ydnar: foliage surfaces are autogenerated from models into geometry lists by q3map2
typedef byte fcolor4ub_t[4];

typedef struct
{
	vec3_t origin;
	fcolor4ub_t color;
}
foliageInstance_t;

typedef struct
{
	surfaceType_t surfaceType;

	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;

	// dynamic lighting information

// BBi
//	int dlightBits[ SMP_FRAMES ];
	int dlightBits;
// BBi

	// triangle definitions
	int numIndexes;
	glIndex_t       *indexes;

	int numVerts;
	vec4_t          *xyz;
	vec4_t          *normal;
	vec2_t          *texCoords;
	vec2_t          *lmTexCoords;

	// origins
	int numInstances;
	foliageInstance_t   *instances;
}
srfFoliage_t;
#endif // RTCW_XX



extern void( *rb_surfaceTable[SF_NUM_SURFACE_TYPES] ) ( void * );

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//

#define SIDE_FRONT  0
#define SIDE_BACK   1
#define SIDE_ON     2

typedef struct msurface_s {
	int viewCount;                      // if == tr.viewCount, already added

#if !defined RTCW_ET
	struct shader_s     *shader;
#else
	shader_t        *shader;
#endif // RTCW_XX

	int fogIndex;

	surfaceType_t       *data;          // any of srf*_t
} msurface_t;

#if defined RTCW_ET
// ydnar: bsp model decal surfaces
typedef struct decal_s
{
	msurface_t      *parent;
	shader_t        *shader;
	float fadeStartTime, fadeEndTime;
	int fogIndex;
	int numVerts;
	polyVert_t verts[ MAX_DECAL_VERTS ];
}
decal_t;
#endif // RTCW_XX


#define CONTENTS_NODE       -1
typedef struct mnode_s {
	// common with leaf and node
	int contents;               // -1 for nodes, to differentiate from leafs
	int visframe;               // node needs to be traversed if current
	vec3_t mins, maxs;          // for bounding box culling

#if defined RTCW_ET
	vec3_t surfMins, surfMaxs;      // ydnar: bounding box including surfaces
#endif // RTCW_XX

	struct mnode_s  *parent;

	// node specific
	cplane_t    *plane;
	struct mnode_s  *children[2];

	// leaf specific
	int cluster;
	int area;

	msurface_t  **firstmarksurface;
	int nummarksurfaces;
} mnode_t;

#if !defined RTCW_ET
typedef struct {
#else
typedef struct bmodel_s {
#endif // RTCW_XX

	vec3_t bounds[2];           // for culling
	msurface_t  *firstSurface;
	int numSurfaces;

#if defined RTCW_ET
	// ydnar: decals
	decal_t         *decals;

	// ydnar: for fog volumes
	int firstBrush;
	int numBrushes;

#if 0
	orientation_t orientation[ SMP_FRAMES ];
	qboolean visible[ SMP_FRAMES ];
	int entityNum[ SMP_FRAMES ];
#endif // 0

	orientation_t orientation;
	qboolean visible;
	int entityNum;
// BBi
#endif // RTCW_XX

} bmodel_t;

#if defined RTCW_ET
// ydnar: optimization
#define WORLD_MAX_SKY_NODES 32
#endif // RTCW_XX

typedef struct {
	char name[MAX_QPATH];               // ie: maps/tim_dm2.bsp
	char baseName[MAX_QPATH];           // ie: tim_dm2

	int dataSize;

	int numShaders;
	dshader_t   *shaders;

#if defined RTCW_ET
	int numBModels;
#endif // RTCW_XX

	bmodel_t    *bmodels;

	int numplanes;
	cplane_t    *planes;

	int numnodes;               // includes leafs
	int numDecisionNodes;
	mnode_t     *nodes;

#if defined RTCW_ET
	int numSkyNodes;
	mnode_t     **skyNodes;     // ydnar: don't walk the entire bsp when rendering sky
#endif // RTCW_XX

	int numsurfaces;
	msurface_t  *surfaces;

	int nummarksurfaces;
	msurface_t  **marksurfaces;

	int numfogs;
	fog_t       *fogs;

#if defined RTCW_ET
	int globalFog;                      // Arnout: index of global fog
	vec4_t globalOriginalFog;           // Arnout: to be able to restore original global fog
	vec4_t globalTransStartFog;         // Arnout: start fog for switch fog transition
	vec4_t globalTransEndFog;           // Arnout: end fog for switch fog transition
	int globalFogTransStartTime;
	int globalFogTransEndTime;
#endif // RTCW_XX

	vec3_t lightGridOrigin;
	vec3_t lightGridSize;
	vec3_t lightGridInverseSize;
	int lightGridBounds[3];
	byte        *lightGridData;

	int numClusters;
	int clusterBytes;
	const byte  *vis;           // may be passed in by CM_LoadMap to save space

	byte        *novis;         // clusterBytes of 0xff

	char        *entityString;
	char        *entityParsePoint;
} world_t;

//======================================================================

typedef enum {
	MOD_BAD,
	MOD_BRUSH,
	MOD_MESH,
	MOD_MDS,

#if !defined RTCW_ET
	MOD_MDC // Ridah
#else
	MOD_MDC,
	MOD_MDM,
	MOD_MDX
#endif // RTCW_XX

} modtype_t;

#if defined RTCW_ET
typedef union {
	bmodel_t    *bmodel;            // only if type == MOD_BRUSH
	md3Header_t *md3[MD3_MAX_LODS]; // only if type == MOD_MESH
	mdsHeader_t *mds;               // only if type == MOD_MDS
	mdcHeader_t *mdc[MD3_MAX_LODS]; // only if type == MOD_MDC
	mdmHeader_t *mdm;               // only if type == MOD_MDM
	mdxHeader_t *mdx;               // only if type == MOD_MDX
} model_u;
#endif // RTCW_XX

typedef struct model_s {
	char name[MAX_QPATH];
	modtype_t type;
	int index;                      // model = tr.models[model->index]

	int dataSize;                   // just for listing purposes

#if !defined RTCW_ET
	bmodel_t    *bmodel;            // only if type == MOD_BRUSH
	md3Header_t *md3[MD3_MAX_LODS]; // only if type == MOD_MESH
	mdsHeader_t *mds;               // only if type == MOD_MDS
	mdcHeader_t *mdc[MD3_MAX_LODS]; // only if type == MOD_MDC
#endif // RTCW_XX

#if defined RTCW_ET
	// show_bug.cgi?id=575
	model_u model;
#endif // RTCW_XX

	int numLods;

#if defined RTCW_SP
// GR - model tessellation capability flag
	int ATI_tess;
#endif // RTCW_XX

#if defined RTCW_ET
	qhandle_t shadowShader;
	float shadowParms[ 6 ];         // x, y, width, height, depth, z offset
#endif // RTCW_XX

} model_t;


#define MAX_MOD_KNOWN   2048

void        R_ModelInit( void );
model_t     *R_GetModelByHandle( qhandle_t hModel );
int         R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
void        R_ModelBounds( qhandle_t handle, vec3_t mins, vec3_t maxs );

void        R_Modellist_f( void );

//====================================================
extern refimport_t ri;

#define MAX_DRAWIMAGES          2048
#define MAX_LIGHTMAPS           256
#define MAX_SKINS               1024

#if !defined RTCW_ET
#define MAX_DRAWSURFS           0x10000
#else
#define MAX_DRAWSURFS           0x40000
#endif // RTCW_XX

#define DRAWSURF_MASK           ( MAX_DRAWSURFS - 1 )

#if defined RTCW_SP
/*

the drawsurf sort data is packed into a single 32 bit value so it can be
compared quickly during the qsorting process

the bits are allocated as follows:

(SA) modified for Wolf (11 bits of entity num)

old:

22 - 31	: sorted shader index
12 - 21	: entity index
3 - 7	: fog index
2		: used to be clipped flag
0 - 1	: dlightmap index

#define	QSORT_SHADERNUM_SHIFT	22
#define	QSORT_ENTITYNUM_SHIFT	12
#define	QSORT_FOGNUM_SHIFT		3

new:

22 - 31	: sorted shader index
11 - 21	: entity index
2 - 6	: fog index
removed	: used to be clipped flag
0 - 1	: dlightmap index

*/
#else
/*

the drawsurf sort data is packed into a single 32 bit value so it can be
compared quickly during the qsorting process

the bits are allocated as follows:

(SA) modified for Wolf (11 bits of entity num)

old:

22 - 31	: sorted shader index
12 - 21	: entity index
3 - 7	: fog index
2		: used to be clipped flag
0 - 1	: dlightmap index

#define	QSORT_SHADERNUM_SHIFT	22
#define	QSORT_ENTITYNUM_SHIFT	12
#define	QSORT_FOGNUM_SHIFT		3

new:

22 - 31	: sorted shader index
11 - 21	: entity index
2 - 6	: fog index
removed	: used to be clipped flag
0 - 1	: dlightmap index

newest: (fixes shader index not having enough bytes)

18 - 31	: sorted shader index
7 - 17	: entity index
2 - 6	: fog index
0 - 1	: dlightmap index
*/
#endif // RTCW_XX

#if defined RTCW_SP
#define QSORT_SHADERNUM_SHIFT   22
#define QSORT_ENTITYNUM_SHIFT   11
#else
#define QSORT_SHADERNUM_SHIFT   18
#define QSORT_ENTITYNUM_SHIFT   7
#endif // RTCW_XX

#define QSORT_FOGNUM_SHIFT      2

#if defined RTCW_ET
#define QSORT_FRONTFACE_SHIFT   1
#endif // RTCW_XX

#if defined RTCW_SP
// GR - tessellation flag in bit 8
#define QSORT_ATI_TESS_SHIFT    8
// GR - TruForm flags
#define ATI_TESS_TRUFORM    1
#define ATI_TESS_NONE       0
#endif // RTCW_XX

extern int gl_filter_min, gl_filter_max;

/*
** performanceCounters_t
*/
typedef struct {

#if defined RTCW_ET
	int c_sphere_cull_in, c_sphere_cull_out;
	int c_plane_cull_in, c_plane_cull_out;
#endif // RTCW_XX

	int c_sphere_cull_patch_in, c_sphere_cull_patch_clip, c_sphere_cull_patch_out;
	int c_box_cull_patch_in, c_box_cull_patch_clip, c_box_cull_patch_out;
	int c_sphere_cull_md3_in, c_sphere_cull_md3_clip, c_sphere_cull_md3_out;
	int c_box_cull_md3_in, c_box_cull_md3_clip, c_box_cull_md3_out;

	int c_leafs;
	int c_dlightSurfaces;
	int c_dlightSurfacesCulled;

#if defined RTCW_ET
	int c_decalProjectors, c_decalTestSurfaces, c_decalClipSurfaces, c_decalSurfaces, c_decalSurfacesCreated;
#endif // RTCW_XX

} frontEndCounters_t;

#define FOG_TABLE_SIZE      256

#if !defined RTCW_ET
#define FUNCTABLE_SIZE      1024
#define FUNCTABLE_SIZE2     10
#else
#define FUNCTABLE_SIZE      4096    //%	1024
#define FUNCTABLE_SIZE2     12      //%	10
#endif // RTCW_XX

#define FUNCTABLE_MASK      ( FUNCTABLE_SIZE - 1 )

// the renderer front end should never modify glstate_t
typedef struct {
	int currenttextures[2];
	int currenttmu;
	qboolean finishCalled;
	int texEnv[2];
	int faceCulling;
	uint32_t glStateBits;
} glstate_t;


typedef struct {
	int c_surfaces, c_shaders, c_vertexes, c_indexes, c_totalIndexes;
	float c_overDraw;

	int c_dlightVertexes;
	int c_dlightIndexes;

	int c_flareAdds;
	int c_flareTests;
	int c_flareRenders;

	int msec;               // total msec for backend run
} backEndCounters_t;

// all state modified by the back end is seperated
// from the front end state
typedef struct {
// BBi
	//int smpFrame;
// BBi

	trRefdef_t refdef;
	viewParms_t viewParms;

// BBi
//#if !defined RTCW_ET
//	orientationr_t  or;
//#else
//	orientationr_t orientation;
//#endif // RTCW_XX
	orientationr_t orientation;
// BBi

	backEndCounters_t pc;
	qboolean isHyperspace;
	trRefEntity_t   *currentEntity;
	qboolean skyRenderedThisView;       // flag for drawing sun

	qboolean projection2D;      // if qtrue, drawstretchpic doesn't need to change modes
	byte color2D[4];
	qboolean vertexes2D;        // shader needs to be finished
	trRefEntity_t entity2D;     // currentEntity will point at this when doing 2D rendering
} backEndState_t;

/*
** trGlobals_t
**
** Most renderer globals are defined here.
** backend functions should never modify any of these fields,
** but may read fields that aren't dynamically modified
** by the frontend.
*/
typedef struct {
	qboolean registered;                    // cleared at shutdown, set at beginRegistration

	int visCount;                           // incremented every time a new vis cluster is entered
	int frameCount;                         // incremented every frame
	int sceneCount;                         // incremented every scene
	int viewCount;                          // incremented every view (twice a scene if portaled)
											// and every R_MarkFragments call

	int smpFrame;                           // toggles from 0 to 1 every endFrame

	int frameSceneNum;                      // zeroed at RE_BeginFrame

	qboolean worldMapLoaded;
	world_t                 *world;

#if defined RTCW_ET
	char                    *worldDir;      // ydnar: for referencing external lightmaps
#endif // RTCW_XX

	const byte              *externalVisData;   // from RE_SetWorldVisData, shared with CM_Load

	image_t                 *defaultImage;
	image_t                 *scratchImage[32];
	image_t                 *fogImage;
	image_t                 *dlightImage;   // inverse-square highlight for projective adding
	image_t                 *flareImage;
	image_t                 *whiteImage;            // full of 0xff
	image_t                 *identityLightImage;    // full of tr.identityLightByte

	shader_t                *defaultShader;
	shader_t                *shadowShader;

#if defined RTCW_SP
//	shader_t				*projectionShadowShader;
#else
	shader_t                *projectionShadowShader;
#endif // RTCW_XX

	shader_t                *dlightShader;      //----(SA) added

	shader_t                *flareShader;

#if defined RTCW_SP
	shader_t                *spotFlareShader;
#endif // RTCW_XX

	char                    *sunShaderName;
	shader_t                *sunShader;
	shader_t                *sunflareShader[6];  //----(SA) for the camera lens flare effect for sun

	int numLightmaps;
	image_t                 *lightmaps[MAX_LIGHTMAPS];

	trRefEntity_t           *currentEntity;
	trRefEntity_t worldEntity;                  // point currentEntity at this when rendering world
	int currentEntityNum;
	int shiftedEntityNum;                       // currentEntityNum << QSORT_ENTITYNUM_SHIFT
	model_t                 *currentModel;

#if defined RTCW_ET
	bmodel_t                *currentBModel;     // only valid when rendering brush models
#endif // RTCW_XX

	viewParms_t viewParms;

	float identityLight;                        // 1.0 / ( 1 << overbrightBits )
	int identityLightByte;                      // identityLight * 255
	int overbrightBits;                         // r_overbrightBits->integer, but set to 0 if no hw gamma

// BBi
//#if !defined RTCW_ET
//	orientationr_t          or;                 // for current entity
//#else
//	orientationr_t orientation;                 // for current entity
//#endif // RTCW_XX
	orientationr_t orientation; // for current entity
// BBi

	trRefdef_t refdef;

	int viewCluster;

	vec3_t sunLight;                            // from the sky shader for this level
	vec3_t sunDirection;

//----(SA)	added
	float lightGridMulAmbient;          // lightgrid multipliers specified in sky shader
	float lightGridMulDirected;         //
//----(SA)	end

//	qboolean				levelGLFog;

	frontEndCounters_t pc;
	int frontEndMsec;                           // not in pc due to clearing issue

	//
	// put large tables at the end, so most elements will be
	// within the +/32K indexed range on risc processors
	//
	model_t                 *models[MAX_MOD_KNOWN];
	int numModels;

	int numImages;
	image_t                 *images[MAX_DRAWIMAGES];
	// Ridah
	int numCacheImages;

	// shader indexes from other modules will be looked up in tr.shaders[]
	// shader indexes from drawsurfs will be looked up in sortedShaders[]
	// lower indexed sortedShaders must be rendered first (opaque surfaces before translucent)
	int numShaders;
	shader_t                *shaders[MAX_SHADERS];
	shader_t                *sortedShaders[MAX_SHADERS];

	int numSkins;
	skin_t                  *skins[MAX_SKINS];

	float sinTable[FUNCTABLE_SIZE];
	float squareTable[FUNCTABLE_SIZE];
	float triangleTable[FUNCTABLE_SIZE];
	float sawToothTable[FUNCTABLE_SIZE];
	float inverseSawToothTable[FUNCTABLE_SIZE];
	float fogTable[FOG_TABLE_SIZE];

	// RF, temp var used while parsing shader only
	int allowCompress;

} trGlobals_t;

extern backEndState_t backEnd;
extern trGlobals_t tr;
extern glconfig_t glConfig;         // outside of TR since it shouldn't be cleared during ref re-init

// BBi
extern GlConfigEx glConfigEx;
// BBi

extern glstate_t glState;           // outside of TR since it shouldn't be cleared during ref re-init


//
// cvars
//
extern cvar_t   *r_flareSize;
extern cvar_t   *r_flareFade;

extern cvar_t   *r_railWidth;
extern cvar_t   *r_railCoreWidth;
extern cvar_t   *r_railSegmentLength;

extern cvar_t   *r_ignore;              // used for debugging anything
extern cvar_t   *r_verbose;             // used for verbose debug spew
extern cvar_t   *r_ignoreFastPath;      // allows us to ignore our Tess fast paths

extern cvar_t   *r_znear;               // near Z clip plane
extern cvar_t   *r_zfar;                // far Z clip plane

extern cvar_t   *r_stencilbits;         // number of desired stencil bits
extern cvar_t   *r_depthbits;           // number of desired depth bits
extern cvar_t   *r_colorbits;           // number of desired color bits, only relevant for fullscreen
extern cvar_t   *r_stereo;              // desired pixelformat stereo flag
extern cvar_t   *r_texturebits;         // number of desired texture bits
										// 0 = use framebuffer depth
										// 16 = use 16-bit textures
										// 32 = use 32-bit textures
										// all else = error

extern cvar_t   *r_measureOverdraw;     // enables stencil buffer overdraw measurement

extern cvar_t   *r_lodbias;             // push/pull LOD transitions
extern cvar_t   *r_lodscale;

extern cvar_t   *r_primitives;          // "0" = based on compiled vertex array existance
										// "1" = glDrawElemet tristrips
										// "2" = glDrawElements triangles
										// "-1" = no drawing

extern cvar_t   *r_inGameVideo;             // controls whether in game video should be draw
extern cvar_t   *r_fastsky;             // controls whether sky should be cleared or drawn
extern cvar_t   *r_drawSun;             // controls drawing of sun quad
										// "0" no sun
										// "1" draw sun
										// "2" also draw lens flare effect centered on sun
extern cvar_t   *r_dynamiclight;        // dynamic lights enabled/disabled

// BBi
//#if defined RTCW_SP
// BBi
extern cvar_t   *r_dlightScale;         // global user attenuation of dlights
// BBi
//#endif // RTCW_XX
// BBi

extern cvar_t   *r_dlightBacks;         // dlight non-facing surfaces for continuity

extern cvar_t  *r_norefresh;            // bypasses the ref rendering
extern cvar_t  *r_drawentities;         // disable/enable entity rendering
extern cvar_t  *r_drawworld;            // disable/enable world rendering

#if defined RTCW_ET
extern cvar_t  *r_drawfoliage;          // ydnar: disable/enable foliage rendering
#endif // RTCW_XX

extern cvar_t  *r_speeds;               // various levels of information display
extern cvar_t  *r_detailTextures;       // enables/disables detail texturing stages
extern cvar_t  *r_novis;                // disable/enable usage of PVS
extern cvar_t  *r_nocull;
extern cvar_t  *r_facePlaneCull;        // enables culling of planar surfaces with back side test
extern cvar_t  *r_nocurves;
extern cvar_t  *r_showcluster;

extern cvar_t   *r_mode;                // video mode

#if defined RTCW_ET
extern cvar_t   *r_oldMode;             // ydnar: previous "good" video mode
#endif // RTCW_XX

extern cvar_t   *r_fullscreen;
extern cvar_t   *r_gamma;
extern cvar_t   *r_displayRefresh;      // optional display refresh option
extern cvar_t   *r_ignorehwgamma;       // overrides hardware gamma capabilities

extern cvar_t   *r_allowExtensions;             // global enable/disable of OpenGL extensions
extern cvar_t   *r_ext_compressed_textures;     // these control use of specific extensions
extern cvar_t   *r_ext_gamma_control;
extern cvar_t   *r_ext_texenv_op;
extern cvar_t   *r_ext_multitexture;
extern cvar_t   *r_ext_compiled_vertex_array;
extern cvar_t   *r_ext_texture_env_add;

// BBi
//#if !defined RTCW_SP
// BBi

extern cvar_t   *r_ext_texture_filter_anisotropic;  //DAJ from EF

// BBi
//#endif // RTCW_XX
// BBi

//----(SA)	added

#if defined RTCW_SP
extern cvar_t   *r_ext_ATI_pntriangles;
extern cvar_t   *r_ati_truform_tess;
extern cvar_t   *r_ati_truform_pointmode;   //----(SA)
extern cvar_t   *r_ati_truform_normalmode;  //----(SA)
extern cvar_t   *r_ati_fsaa_samples;        //DAJ

// BBi
//extern cvar_t   *r_ext_texture_filter_anisotropic;
// BBi
#endif // RTCW_XX

#if defined RTCW_ET
extern cvar_t   *r_ati_fsaa_samples;                //DAJ

extern cvar_t   *r_clampToEdge;         // ydnar: opengl 1.2 GL_CLAMP_TO_EDGE support

// TTimo
#endif // RTCW_XX

extern cvar_t   *r_ext_NV_fog_dist;
extern cvar_t   *r_nv_fogdist_mode;

#if defined RTCW_MP
extern cvar_t   *r_ext_ATI_pntriangles;
extern cvar_t   *r_ati_truform_tess;        //
extern cvar_t   *r_ati_truform_normalmode;  // linear/quadratic
extern cvar_t   *r_ati_truform_pointmode;   // linear/cubic
//----(SA)	end

extern cvar_t   *r_ati_fsaa_samples;                //DAJ
#endif // RTCW_XX

// BBi
//#if defined RTCW_SP
// BBi
//----(SA)	end

extern cvar_t  *r_waterFogColor;        //----(SA)	added
extern cvar_t  *r_mapFogColor;          //----(SA)	added
extern cvar_t  *r_savegameFogColor;     //----(SA)	added
// BBi
//#endif // RTCW_XX
// BBi

extern cvar_t  *r_nobind;                       // turns off binding to appropriate textures
extern cvar_t  *r_singleShader;                 // make most world faces use default shader
extern cvar_t  *r_roundImagesDown;

#if defined RTCW_SP
extern cvar_t  *r_lowMemTextureSize;
extern cvar_t  *r_lowMemTextureThreshold;
#endif // RTCW_XX

extern cvar_t  *r_rmse;                         // reduces textures to this root mean square error
extern cvar_t  *r_colorMipLevels;               // development aid to see texture mip usage
extern cvar_t  *r_picmip;                       // controls picmip values

#if defined RTCW_SP
extern cvar_t  *r_picmip2;                      // controls picmip values for designated (character skin) textures
#endif // RTCW_XX

extern cvar_t  *r_finish;
extern cvar_t  *r_drawBuffer;
extern cvar_t  *r_glDriver;
extern cvar_t  *r_glIgnoreWicked3D;
extern cvar_t  *r_swapInterval;
extern cvar_t  *r_textureMode;

// BBi
//#if defined RTCW_ET
// BBi

extern cvar_t  *r_textureAnisotropy;

// BBi
//#endif // RTCW_XX
// BBi

extern cvar_t  *r_offsetFactor;
extern cvar_t  *r_offsetUnits;
extern cvar_t* r_fullbright; // avoid lightmap pass // JPW NERVE removed per atvi request
extern cvar_t  *r_lightmap;                     // render lightmaps only

// BBi
//#if !defined RTCW_ET
// BBi
extern cvar_t  *r_vertexLight;                  // vertex lighting mode for better performance
// BBi
//#endif // RTCW_XX
// BBi

extern cvar_t  *r_uiFullScreen;                 // ui is running fullscreen

extern cvar_t  *r_logFile;                      // number of frames to emit GL logs
extern cvar_t  *r_showtris;                     // enables wireframe rendering of the world

#if defined RTCW_ET
extern cvar_t  *r_trisColor;                    // enables modifying of the wireframe colour (in 0xRRGGBB[AA] format, alpha defaults to FF)
#endif // RTCW_XX

extern cvar_t  *r_showsky;                      // forces sky in front of all surfaces
extern cvar_t  *r_shownormals;                  // draws wireframe normals

#if defined RTCW_ET
extern cvar_t  *r_normallength;                 // length of the normals
extern cvar_t  *r_showmodelbounds;
#endif // RTCW_XX

extern cvar_t  *r_clear;                        // force screen clear every frame

extern cvar_t  *r_shadows;                      // controls shadows: 0 = none, 1 = blur, 2 = stencil, 3 = black planar projection
extern cvar_t  *r_flares;                       // light flares

extern cvar_t  *r_portalsky;    // (SA) added
extern cvar_t  *r_intensity;

extern cvar_t  *r_lockpvs;
extern cvar_t  *r_noportals;
extern cvar_t  *r_portalOnly;

extern cvar_t  *r_subdivisions;
extern cvar_t  *r_lodCurveError;
extern cvar_t  *r_smp;
extern cvar_t  *r_showSmp;
extern cvar_t  *r_skipBackEnd;

extern cvar_t  *r_ignoreGLErrors;

extern cvar_t  *r_overBrightBits;
extern cvar_t  *r_mapOverBrightBits;

extern cvar_t  *r_debugSurface;
extern cvar_t  *r_simpleMipMaps;

extern cvar_t  *r_showImages;
extern cvar_t  *r_debugSort;

extern cvar_t  *r_printShaders;
extern cvar_t  *r_saveFontData;

// Ridah
extern cvar_t  *r_cache;
extern cvar_t  *r_cacheShaders;
extern cvar_t  *r_cacheModels;

extern cvar_t  *r_cacheGathering;

extern cvar_t  *r_bonesDebug;
// done.

// Rafael - wolf fog
extern cvar_t   *r_wolffog;
// done

extern cvar_t  *r_highQualityVideo;
//====================================================================

float R_NoiseGet4f( float x, float y, float z, float t );
void  R_NoiseInit( void );

void R_SwapBuffers( int );

void R_RenderView( viewParms_t *parms );

void R_AddMD3Surfaces( trRefEntity_t *e );
void R_AddNullModelSurfaces( trRefEntity_t *e );
void R_AddBeamSurfaces( trRefEntity_t *e );
void R_AddRailSurfaces( trRefEntity_t *e, qboolean isUnderwater );
void R_AddLightningBoltSurfaces( trRefEntity_t *e );

void R_TagInfo_f( void );

void R_AddPolygonSurfaces( void );

#if defined RTCW_SP
// GR - add tessellation flag
void R_DecomposeSort( unsigned sort, int *entityNum, shader_t **shader,
					  int *fogNum, int *dlightMap, int *atiTess );

// GR - add tessellation flag
void R_AddDrawSurf( surfaceType_t *surface, shader_t *shader, int fogIndex, int dlightMap, int atiTess );
#elif defined RTCW_MP
void R_DecomposeSort( unsigned sort, int *entityNum, shader_t **shader,
					  int *fogNum, int *dlightMap );

void R_AddDrawSurf( surfaceType_t *surface, shader_t *shader, int fogIndex, int dlightMap );
#else
void R_AddPolygonBufferSurfaces( void );


void R_DecomposeSort( unsigned sort, int *entityNum, shader_t **shader,
					  int *fogNum, int *frontFace, int *dlightMap );

void R_AddDrawSurf( surfaceType_t *surface, shader_t *shader, int fogNum, int frontFace, int dlightMap );
#endif // RTCW_XX


#define CULL_IN     0       // completely unclipped
#define CULL_CLIP   1       // clipped by one or more planes
#define CULL_OUT    2       // completely outside the clipping planes
void R_LocalNormalToWorld( vec3_t local, vec3_t world );
void R_LocalPointToWorld( vec3_t local, vec3_t world );
int R_CullLocalBox( vec3_t bounds[2] );
int R_CullPointAndRadius( vec3_t origin, float radius );
int R_CullLocalPointAndRadius( vec3_t origin, float radius );

// BBi
//#if !defined RTCW_ET
//void R_RotateForEntity( const trRefEntity_t * ent, const viewParms_t * viewParms, orientationr_t * or );
//#else
//void R_RotateForEntity( const trRefEntity_t *ent, const viewParms_t *viewParms, orientationr_t *orientation );
//#endif // RTCW_XX
void R_RotateForEntity (const trRefEntity_t* ent, const viewParms_t* viewParms, orientationr_t* orientation);
// BBi

/*
** GL wrapper/helper functions
*/
void    GL_Bind( image_t *image );
void    GL_SetDefaultState( void );
void    GL_SelectTexture( int unit );
void    GL_TextureMode( const char *string );

// BBi
//#if defined RTCW_ET
// BBi

void    GL_TextureAnisotropy( float anisotropy );

// BBi
//#endif // RTCW_XX
// BBi

void    GL_CheckErrors( void );
void    GL_State( uint32_t stateVector );
void    GL_TexEnv( int env );
void    GL_Cull( int cullType );

#define GLS_SRCBLEND_ZERO                       0x00000001
#define GLS_SRCBLEND_ONE                        0x00000002
#define GLS_SRCBLEND_DST_COLOR                  0x00000003
#define GLS_SRCBLEND_ONE_MINUS_DST_COLOR        0x00000004
#define GLS_SRCBLEND_SRC_ALPHA                  0x00000005
#define GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA        0x00000006
#define GLS_SRCBLEND_DST_ALPHA                  0x00000007
#define GLS_SRCBLEND_ONE_MINUS_DST_ALPHA        0x00000008
#define GLS_SRCBLEND_ALPHA_SATURATE             0x00000009
#define     GLS_SRCBLEND_BITS                   0x0000000f

#define GLS_DSTBLEND_ZERO                       0x00000010
#define GLS_DSTBLEND_ONE                        0x00000020
#define GLS_DSTBLEND_SRC_COLOR                  0x00000030
#define GLS_DSTBLEND_ONE_MINUS_SRC_COLOR        0x00000040
#define GLS_DSTBLEND_SRC_ALPHA                  0x00000050
#define GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA        0x00000060
#define GLS_DSTBLEND_DST_ALPHA                  0x00000070
#define GLS_DSTBLEND_ONE_MINUS_DST_ALPHA        0x00000080
#define     GLS_DSTBLEND_BITS                   0x000000f0

#define GLS_DEPTHMASK_TRUE                      0x00000100

#define GLS_POLYMODE_LINE                       0x00001000

#define GLS_DEPTHTEST_DISABLE                   0x00010000
#define GLS_DEPTHFUNC_EQUAL                     0x00020000

#if defined RTCW_SP
#define GLS_FOG_DISABLE                         0x00020000  //----(SA)	added
#endif // RTCW_XX

#define GLS_ATEST_GT_0                          0x10000000
#define GLS_ATEST_LT_80                         0x20000000
#define GLS_ATEST_GE_80                         0x40000000
#define     GLS_ATEST_BITS                      0x70000000

#define GLS_DEFAULT         GLS_DEPTHMASK_TRUE

void    RE_StretchRaw( int x, int y, int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty );
void    RE_UploadCinematic( int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty );

void        RE_BeginFrame( stereoFrame_t stereoFrame );
void        RE_BeginRegistration( glconfig_t *glconfig );
void        RE_LoadWorldMap( const char *mapname );
void        RE_SetWorldVisData( const byte *vis );
qhandle_t   RE_RegisterModel( const char *name );
qhandle_t   RE_RegisterSkin( const char *name );
void        RE_Shutdown( qboolean destroyWindow );

qboolean    R_GetEntityToken( char *buffer, int size );

#if defined RTCW_ET
float       R_ProcessLightmap( byte **pic, int in_padding, int width, int height, byte **pic_out ); // Arnout
#endif // RTCW_XX

//----(SA)
qboolean    RE_GetSkinModel( qhandle_t skinid, const char *type, char *name );
qhandle_t   RE_GetShaderFromModel( qhandle_t modelid, int surfnum, int withlightmap );    //----(SA)
//----(SA) end

model_t     *R_AllocModel( void );

void        R_Init( void );

#if !defined RTCW_ET
image_t     *R_FindImageFile( const char *name, qboolean mipmap, qboolean allowPicmip, int glWrapClampMode );
#else
image_t     *R_FindImageFile( const char *name, qboolean mipmap, qboolean allowPicmip, int glWrapClampMode, qboolean lightmap );
#endif // RTCW_XX

#if defined RTCW_SP
image_t     *R_FindImageFileExt( const char *name, qboolean mipmap, qboolean allowPicmip, qboolean characterMip, int glWrapClampMode ); //----(SA)	added
#endif // RTCW_XX

image_t     *R_CreateImage( const char *name, const byte *pic, int width, int height, qboolean mipmap
							, qboolean allowPicmip, int wrapClampMode );

#if defined RTCW_SP
//----(SA)	added (didn't want to modify all instances of R_CreateImage()
image_t     *R_CreateImageExt( const char *name, const byte *pic, int width, int height, qboolean mipmap
							   , qboolean allowPicmip, qboolean characterMip, int wrapClampMode );
//----(SA)	end
#endif // RTCW_XX

qboolean    R_GetModeInfo( int *width, int *height, float *windowAspect, int mode );

void        R_SetColorMappings( void );
void        R_GammaCorrect( byte *buffer, int bufSize );

void    R_ImageList_f( void );
void    R_SkinList_f( void );
void    R_ScreenShot_f( void );
void    R_ScreenShotJPEG_f( void );

void    R_InitFogTable( void );
float   R_FogFactor( float s, float t );
void    R_InitImages( void );
void    R_DeleteTextures( void );
int     R_SumOfUsedImages( void );
void    R_InitSkins( void );
skin_t  *R_GetSkinByHandle( qhandle_t hSkin );

#if defined RTCW_ET
void    R_DebugText( const vec3_t org, float r, float g, float b, const char *text, qboolean neverOcclude );
#endif // RTCW_XX

//
// tr_shader.c
//
qhandle_t        RE_RegisterShaderLightMap( const char *name, int lightmapIndex );
qhandle_t        RE_RegisterShader( const char *name );
qhandle_t        RE_RegisterShaderNoMip( const char *name );
qhandle_t RE_RegisterShaderFromImage( const char *name, int lightmapIndex, image_t *image, qboolean mipRawImage );

shader_t    *R_FindShader( const char *name, int lightmapIndex, qboolean mipRawImage );
shader_t    *R_GetShaderByHandle( qhandle_t hShader );
shader_t    *R_GetShaderByState( int index, int32_t *cycleTime );
shader_t *R_FindShaderByName( const char *name );
void        R_InitShaders( void );
void        R_ShaderList_f( void );
void    R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );

#if defined RTCW_ET
//bani
qboolean RE_LoadDynamicShader( const char *shadername, const char *shadertext );

// fretn - renderToTexture
void RE_RenderToTexture( int textureid, int x, int y, int w, int h );
// bani
void RE_Finish( void );
int R_GetTextureId( const char *name );
#endif // RTCW_XX

/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

#if defined RTCW_ET
extern int gl_NormalFontBase;
#endif // RTCW_XX

void        GLimp_Init( void );
void        GLimp_Shutdown( void );
void        GLimp_EndFrame( void );

// BBi
//qboolean GLimp_SpawnRenderThread( void ( *function )( void ) );
//void        *GLimp_RendererSleep( void );
//void        GLimp_FrontEndSleep( void );
//void        GLimp_WakeRenderer( void *data );
// BBi

void        GLimp_LogComment( char *comment );

void GLimp_SetGamma( unsigned char red[256],
					 unsigned char green[256],
					 unsigned char blue[256] );


/*
====================================================================

TESSELATOR/SHADER DECLARATIONS

====================================================================
*/

#if !defined RTCW_ET
typedef byte color4ub_t[4];
#endif // RTCW_XX

typedef struct stageVars
{
	color4ub_t colors[SHADER_MAX_VERTEXES];
	vec2_t texcoords[NUM_TEXTURE_BUNDLES][SHADER_MAX_VERTEXES];
} stageVars_t;

extern vec4hack_t tess_xyz[SHADER_MAX_VERTEXES];
extern vec4hack_t tess_normal[SHADER_MAX_VERTEXES];
extern vec2hack_t tess_texCoords0[SHADER_MAX_VERTEXES];
extern vec2hack_t tess_texCoords1[SHADER_MAX_VERTEXES];
extern glIndex_t tess_indexes[SHADER_MAX_INDEXES];
extern color4ubhack_t tess_vertexColors[SHADER_MAX_VERTEXES];

#ifndef RTCW_ET
extern int vertexDlightBits[SHADER_MAX_VERTEXES];
#endif // RTCW_X

typedef struct shaderCommands_s
{
	glIndex_t* indexes;
	vec4hack_t* normal;
	color4ubhack_t* vertexColors;
	vec4hack_t* xyz;
	vec2hack_t* texCoords0;
	vec2hack_t* texCoords1;

#if !defined RTCW_ET
	int* vertexDlightBits;
#endif // RTCW_X

	stageVars_t svars;

	color4ub_t constantColor255[SHADER_MAX_VERTEXES];

	int maxShaderVerts;
	int maxShaderIndicies;

	shader_t    *shader;
	float shaderTime;
	int fogNum;

	int dlightBits;         // or together of all vertexDlightBits

	int numIndexes;
	int numVertexes;

#if defined RTCW_SP
	qboolean ATI_tess;
#endif // RTCW_XX

	// info extracted from current shader
	int numPasses;
	void ( *currentStageIteratorFunc )( void );
	shaderStage_t   **xstages;
} shaderCommands_t;

extern shaderCommands_t tess;

void RB_BeginSurface( shader_t *shader, int fogNum );
void RB_EndSurface( void );
void RB_CheckOverflow( int verts, int indexes );

#if !defined RTCW_ET
#define RB_CHECKOVERFLOW( v,i ) if ( tess.numVertexes + ( v ) >= SHADER_MAX_VERTEXES || tess.numIndexes + ( i ) >= SHADER_MAX_INDEXES ) {RB_CheckOverflow( v,i );}
#else
#define RB_CHECKOVERFLOW( v,i ) if ( tess.numVertexes + ( v ) >= tess.maxShaderVerts || tess.numIndexes + ( i ) >= SHADER_MAX_INDEXES ) {RB_CheckOverflow( v,i );}
#endif // RTCW_XX

void RB_StageIteratorGeneric( void );
void RB_StageIteratorSky( void );
void RB_StageIteratorVertexLitTexture( void );
void RB_StageIteratorLightmappedMultitexture( void );

void RB_AddQuadStamp( vec3_t origin, vec3_t left, vec3_t up, byte *color );
void RB_AddQuadStampExt( vec3_t origin, vec3_t left, vec3_t up, byte *color, float s1, float t1, float s2, float t2 );
void RB_AddQuadStampFadingCornersExt( vec3_t origin, vec3_t left, vec3_t up, byte *color, float s1, float t1, float s2, float t2 );

void RB_ShowImages( void );

#if defined RTCW_ET
void RB_DrawBounds( vec3_t mins, vec3_t maxs ); // ydnar
#endif // RTCW_XX


/*
============================================================

WORLD MAP

============================================================
*/

void R_AddBrushModelSurfaces( trRefEntity_t *e );
void R_AddWorldSurfaces( void );


/*
============================================================

FLARES

============================================================
*/

void R_ClearFlares( void );

// BBi
//#if defined RTCW_SP
//void RB_AddFlare( void *surface, int fogNum, vec3_t point, vec3_t color, float scale, vec3_t normal, int id, int flags ); // TTimo updated prototype
//#else
//void RB_AddFlare( void *surface, int fogNum, vec3_t point, vec3_t color, float scale, vec3_t normal, int id, qboolean visible );    //----(SA)	added scale.  added id.  added visible
//#endif // RTCW_XX
void RB_AddFlare (void* surface, int fogNum, vec3_t point, vec3_t color,
	float scale, vec3_t normal, int id, int flags);
// BBi

void RB_AddDlightFlares( void );
void RB_RenderFlares( void );

/*
============================================================

LIGHTS

============================================================
*/

#if defined RTCW_ET
void R_TransformDlights( int count, dlight_t *dl, orientationr_t *orientation );
void R_CullDlights( void );
#endif // RTCW_XX

void R_DlightBmodel( bmodel_t *bmodel );
void R_SetupEntityLighting( const trRefdef_t *refdef, trRefEntity_t *ent );

#if !defined RTCW_ET
void R_TransformDlights( int count, dlight_t * dl, orientationr_t * orient );
#endif // RTCW_XX

int R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );


/*
============================================================

SHADOWS

============================================================
*/

void RB_ShadowTessEnd( void );
void RB_ShadowFinish( void );
void RB_ProjectionShadowDeform( void );

/*
============================================================

SKIES

============================================================
*/

void R_BuildCloudData( shaderCommands_t *shader );
void R_InitSkyTexCoords( float cloudLayerHeight );
void R_DrawSkyBox( shaderCommands_t *shader );
void RB_DrawSun( void );
void RB_ClipSkyPolygons( shaderCommands_t *shader );

/*
============================================================

CURVE TESSELATION

============================================================
*/

#define PATCH_STITCHING

srfGridMesh_t *R_SubdividePatchToGrid( int width, int height,
									   drawVert_t points[MAX_PATCH_SIZE * MAX_PATCH_SIZE] );
srfGridMesh_t *R_GridInsertColumn( srfGridMesh_t *grid, int column, int row, vec3_t point, float loderror );
srfGridMesh_t *R_GridInsertRow( srfGridMesh_t *grid, int row, int column, vec3_t point, float loderror );
void R_FreeSurfaceGridMesh( srfGridMesh_t *grid );

/*
============================================================

MARKERS, POLYGON PROJECTION ON WORLD POLYGONS

============================================================
*/

int R_MarkFragments( int orientation, const vec3_t *points, const vec3_t projection,
					 int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer );

#if defined RTCW_ET
/*
============================================================

DECALS - ydnar

============================================================
*/

void RE_ProjectDecal( qhandle_t hShader, int numPoints, vec3_t *points, vec4_t projection, vec4_t color, int lifeTime, int fadeTime );
void RE_ClearDecals( void );

void R_AddModelShadow( refEntity_t *ent );

void R_TransformDecalProjector( decalProjector_t * in, vec3_t axis[ 3 ], vec3_t origin, decalProjector_t * out );
qboolean R_TestDecalBoundingBox( decalProjector_t *dp, vec3_t mins, vec3_t maxs );
qboolean R_TestDecalBoundingSphere( decalProjector_t *dp, vec3_t center, float radius2 );

void R_ProjectDecalOntoSurface( decalProjector_t *dp, msurface_t *surf, bmodel_t *bmodel );

void R_AddDecalSurface( decal_t *decal );
void R_AddDecalSurfaces( bmodel_t *bmodel );
void R_CullDecalProjectors( void );
#endif // RTCW_XX

/*
============================================================

SCENE GENERATION

============================================================
*/

void R_ToggleSmpFrame( void );

void RE_ClearScene( void );
void RE_AddRefEntityToScene( const refEntity_t *ent );
void RE_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts );
// Ridah
void RE_AddPolysToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys );
// done.

#if defined RTCW_ET
void RE_AddPolyBufferToScene( polyBuffer_t* pPolyBuffer );
#endif // RTCW_XX

#if !defined RTCW_ET
// Ridah
void RE_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw );
// done.
#else
// ydnar: modified dlight system to support seperate radius & intensity
// void RE_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw );
void RE_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
#endif // RTCW_XX

//----(SA)
// BBi
//#if defined RTCW_SP
//void RE_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, int flags );
//#else
//void RE_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible );
//#endif // RTCW_XX
void RE_AddCoronaToScene (const vec3_t org, float r, float g, float b,
	float scale, int id, int flags);
// BBi
//----(SA)
void RE_RenderScene( const refdef_t *fd );

#if defined RTCW_ET
void RE_SaveViewParms();
void RE_RestoreViewParms();
#endif // RTCW_XX

/*
=============================================================

ANIMATED MODELS

=============================================================
*/

void R_MakeAnimModel( model_t *model );
void R_AddAnimSurfaces( trRefEntity_t *ent );
void RB_SurfaceAnim( mdsSurface_t *surfType );
int R_GetBoneTag( orientation_t *outTag, mdsHeader_t *mds, int startTagIndex, const refEntity_t *refent, const char *tagName );

#if defined RTCW_ET
//
// MDM / MDX
//
void R_MDM_MakeAnimModel( model_t *model );
void R_MDM_AddAnimSurfaces( trRefEntity_t *ent );
void RB_MDM_SurfaceAnim( mdmSurface_t *surfType );
int R_MDM_GetBoneTag( orientation_t *outTag, mdmHeader_t *mdm, int startTagIndex, const refEntity_t *refent, const char *tagName );
#endif // RTCW_XX

/*
=============================================================
=============================================================
*/
void    R_TransformModelToClip( const vec3_t src, const float *modelMatrix, const float *projectionMatrix,
								vec4_t eye, vec4_t dst );
void    R_TransformClipToWindow( const vec4_t clip, const viewParms_t *view, vec4_t normalized, vec4_t window );

void    RB_DeformTessGeometry( void );

void    RB_CalcEnvironmentTexCoords( float *dstTexCoords );
void    RB_CalcFireRiseEnvTexCoords( float *st );
void    RB_CalcFogTexCoords( float *dstTexCoords );
void    RB_CalcScrollTexCoords( const float scroll[2], float *dstTexCoords );
void    RB_CalcRotateTexCoords( float rotSpeed, float *dstTexCoords );
void    RB_CalcScaleTexCoords( const float scale[2], float *dstTexCoords );
void    RB_CalcSwapTexCoords( float *dstTexCoords );
void    RB_CalcTurbulentTexCoords( const waveForm_t *wf, float *dstTexCoords );
void    RB_CalcTransformTexCoords( const texModInfo_t *tmi, float *dstTexCoords );
void    RB_CalcModulateColorsByFog( unsigned char *dstColors );
void    RB_CalcModulateAlphasByFog( unsigned char *dstColors );
void    RB_CalcModulateRGBAsByFog( unsigned char *dstColors );
void    RB_CalcWaveAlpha( const waveForm_t *wf, unsigned char *dstColors );
void    RB_CalcWaveColor( const waveForm_t *wf, unsigned char *dstColors );
void    RB_CalcAlphaFromEntity( unsigned char *dstColors );
void    RB_CalcAlphaFromOneMinusEntity( unsigned char *dstColors );
void    RB_CalcStretchTexCoords( const waveForm_t *wf, float *texCoords );
void    RB_CalcColorFromEntity( unsigned char *dstColors );
void    RB_CalcColorFromOneMinusEntity( unsigned char *dstColors );
void    RB_CalcSpecularAlpha( unsigned char *alphas );
void    RB_CalcDiffuseColor( unsigned char *colors );

#if defined RTCW_SP
void    RB_ZombieFXInit( void );
void    RB_ZombieFXAddNewHit( int entityNum, const vec3_t hitPos, const vec3_t hitDir );
#endif // RTCW_XX

/*
=============================================================

RENDERER BACK END FUNCTIONS

=============================================================
*/

void RB_RenderThread( void );
void RB_ExecuteRenderCommands( const void *data );

/*
=============================================================

RENDERER BACK END COMMAND QUEUE

=============================================================
*/

#define MAX_RENDER_COMMANDS 0x40000

typedef struct {
	byte cmds[MAX_RENDER_COMMANDS];
	int used;
} renderCommandList_t;

typedef struct {
	int commandId;
	float color[4];
} setColorCommand_t;

typedef struct {
	int commandId;
	int buffer;
} drawBufferCommand_t;

typedef struct {
	int commandId;
	image_t *image;
	int width;
	int height;
	void    *data;
} subImageCommand_t;

typedef struct {
	int commandId;
} swapBuffersCommand_t;

typedef struct {
	int commandId;
	int buffer;
} endFrameCommand_t;

typedef struct {
	int commandId;
	shader_t    *shader;
	float x, y;
	float w, h;
	float s1, t1;
	float s2, t2;

	byte gradientColor[4];      // color values 0-255
	int gradientType;       //----(SA)	added

#if !defined RTCW_SP
	float angle;            // NERVE - SMF
#endif // RTCW_XX

} stretchPicCommand_t;

#if defined RTCW_ET
typedef struct {
	int commandId;
	polyVert_t* verts;
	int numverts;
	shader_t*   shader;
} poly2dCommand_t;
#endif // RTCW_XX

typedef struct {
	int commandId;
	trRefdef_t refdef;
	viewParms_t viewParms;
	drawSurf_t *drawSurfs;
	int numDrawSurfs;
} drawSurfsCommand_t;

#if defined RTCW_ET
//bani
typedef struct {
	int commandId;
	image_t *image;
	int x;
	int y;
	int w;
	int h;
} renderToTextureCommand_t;

//bani
typedef struct {
	int commandId;
} renderFinishCommand_t;
#endif // RTCW_XX

typedef enum {
	RC_END_OF_LIST,
	RC_SET_COLOR,
	RC_STRETCH_PIC,

#if defined RTCW_ET
	RC_2DPOLYS,
#endif // RTCW_XX

#if !defined RTCW_SP
	RC_ROTATED_PIC,
#endif // RTCW_XX

	RC_STRETCH_PIC_GRADIENT,    // (SA) added
	RC_DRAW_SURFS,
	RC_DRAW_BUFFER,

#if !defined RTCW_ET
	RC_SWAP_BUFFERS
#else
	RC_SWAP_BUFFERS,
	RC_RENDERTOTEXTURE, //bani
	RC_FINISH   //bani
#endif // RTCW_XX

} renderCommand_t;


// these are sort of arbitrary limits.
// the limits apply to the sum of all scenes in a frame --
// the main view, all the 3D icons, etc

// Ridah, these aren't enough for cool effects
//#define	MAX_POLYS		256
//#define	MAX_POLYVERTS	1024
#define MAX_POLYS       4096
#define MAX_POLYVERTS   8192
// done.

#if defined RTCW_ET
// Gordon: testing
#define MAX_POLYINDICIES 8192

// ydnar: max decal projectors per frame, each can generate lots of polys
#define MAX_DECAL_PROJECTORS    32  // uses bitmasks, don't increase
#define DECAL_PROJECTOR_MASK    ( MAX_DECAL_PROJECTORS - 1 )
#define MAX_DECALS              1024
#define DECAL_MASK              ( MAX_DECALS - 1 )
#endif // RTCW_XX

// all of the information needed by the back end must be
// contained in a backEndData_t.  This entire structure is
// duplicated so the front and back end can run in parallel
// on an SMP machine
typedef struct {
	drawSurf_t drawSurfs[MAX_DRAWSURFS];
	dlight_t dlights[MAX_DLIGHTS];
	corona_t coronas[MAX_CORONAS];          //----(SA)
	trRefEntity_t entities[MAX_ENTITIES];
	srfPoly_t polys[MAX_POLYS];

#if defined RTCW_ET
	srfPolyBuffer_t polybuffers[MAX_POLYS];
#endif // RTCW_XX

	polyVert_t polyVerts[MAX_POLYVERTS];

#if defined RTCW_ET
	decalProjector_t decalProjectors[ MAX_DECAL_PROJECTORS ];
	srfDecal_t decals[ MAX_DECALS ];
#endif // RTCW_XX

	renderCommandList_t commands;
} backEndData_t;

extern int max_polys;
extern int max_polyverts;

// BBi
//extern backEndData_t   *backEndData[SMP_FRAMES];    // the second one may not be allocated
extern backEndData_t* backEndData;
// BBi

extern volatile renderCommandList_t    *renderCommandList;

extern volatile qboolean renderThreadActive;


void *R_GetCommandBuffer( int bytes );
void RB_ExecuteRenderCommands( const void *data );

void R_InitCommandBuffers( void );
void R_ShutdownCommandBuffers( void );

void R_SyncRenderThread( void );

void R_AddDrawSurfCmd( drawSurf_t *drawSurfs, int numDrawSurfs );

void RE_SetColor( const float *rgba );
void RE_StretchPic( float x, float y, float w, float h,
					float s1, float t1, float s2, float t2, qhandle_t hShader );

#if !defined RTCW_SP
void RE_RotatedPic( float x, float y, float w, float h,
					float s1, float t1, float s2, float t2, qhandle_t hShader, float angle );       // NERVE - SMF
#endif // RTCW_XX

void RE_StretchPicGradient( float x, float y, float w, float h,
							float s1, float t1, float s2, float t2, qhandle_t hShader, const float *gradientColor, int gradientType );

#if defined RTCW_ET
void RE_2DPolyies( polyVert_t* verts, int numverts, qhandle_t hShader );
void RE_SetGlobalFog( qboolean restore, int duration, float r, float g, float b, float depthForOpaque );
#endif // RTCW_XX

void RE_BeginFrame( stereoFrame_t stereoFrame );
void RE_EndFrame( int *frontEndMsec, int *backEndMsec );
void SaveJPG (const char* file_name, int quality, int width, int height, uint8_t* src_data);

// font stuff
void R_InitFreeType();
void R_DoneFreeType();
void RE_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font );

// Ridah, caching system
// NOTE: to disable this for development, set "r_cache 0" in autoexec.cfg
void R_InitTexnumImages( qboolean force );

void *R_CacheModelAlloc( int size );
void R_CacheModelFree( void *ptr );
void R_PurgeModels( int count );
void R_BackupModels( void );
qboolean R_FindCachedModel( const char *name, model_t *newmod );
void R_LoadCacheModels( void );

void *R_CacheImageAlloc( int size );
void R_CacheImageFree( void *ptr );
qboolean R_TouchImage( image_t *inImage );
image_t *R_FindCachedImage( const char *name, int hash );
void R_FindFreeTexnum( image_t *image );
void R_LoadCacheImages( void );
void R_PurgeBackupImages( int purgeCount );
void R_BackupImages( void );

#if !defined RTCW_ET
void *R_CacheShaderAlloc( int size );
void R_CacheShaderFree( void *ptr );
#else
//void *R_CacheShaderAlloc( int size );
//void R_CacheShaderFree( void *ptr );
#endif // RTCW_XX

#if defined RTCW_ET
void R_CacheShaderFreeExt( const char* name, void *ptr, const char* file, int line );
void* R_CacheShaderAllocExt( const char* name, int size, const char* file, int line );

#define R_CacheShaderAlloc( name, size ) R_CacheShaderAllocExt( name, size, __FILE__, __LINE__ )
#define R_CacheShaderFree( name, ptr ) R_CacheShaderFreeExt( name, ptr, __FILE__, __LINE__ )
#endif // RTCW_XX

shader_t *R_FindCachedShader( const char *name, int lightmapIndex, int hash );
void R_BackupShaders( void );
void R_PurgeShaders( int count );

#if defined RTCW_ET
void R_PurgeLightmapShaders( void );
#endif // RTCW_XX

void R_LoadCacheShaders( void );
// done.

//------------------------------------------------------------------------------
// Ridah, mesh compression
#define NUMMDCVERTEXNORMALS  256

extern float r_anormals[NUMMDCVERTEXNORMALS][3];

// NOTE: MDC_MAX_ERROR is effectively the compression level. the lower this value, the higher
// the accuracy, but with lower compression ratios.
#define MDC_MAX_ERROR       0.1     // if any compressed vert is off by more than this from the
									// actual vert, make this a baseframe

#define MDC_DIST_SCALE      0.05    // lower for more accuracy, but less range

// note: we are locked in at 8 or less bits since changing to byte-encoded normals
#define MDC_BITS_PER_AXIS   8
#define MDC_MAX_OFS         127.0   // to be safe

#define MDC_MAX_DIST        ( MDC_MAX_OFS * MDC_DIST_SCALE )

#if 0
void R_MDC_DecodeXyzCompressed( mdcXyzCompressed_t *xyzComp, vec3_t out, vec3_t normal );
#else   // optimized version
#define R_MDC_DecodeXyzCompressed( ofsVec, out, normal ) \
	( out )[0] = ( (float)( ( ofsVec ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	( out )[1] = ( (float)( ( ofsVec >> 8 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	( out )[2] = ( (float)( ( ofsVec >> 16 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	VectorCopy( ( r_anormals )[( ofsVec >> 24 )], normal );
#endif

void R_AddMDCSurfaces( trRefEntity_t *ent );
// done.
//------------------------------------------------------------------------------

void R_LatLongToNormal( vec3_t outNormal, short latLong );


/*
============================================================

GL FOG

============================================================
*/

//extern glfog_t		glfogCurrent;
extern glfog_t glfogsettings[NUM_FOGS];         // [0] never used (FOG_NONE)
extern glfogType_t glfogNum;                    // fog type to use (from the fog_t enum list)

extern qboolean fogIsOn;

extern void         R_FogOff( void );
extern void         R_FogOn( void );

extern void R_SetFog( int fogvar, int var1, int var2, float r, float g, float b, float density );

extern int skyboxportal;

#if defined RTCW_SP
extern int drawskyboxportal;
#endif // RTCW_XX

// Ridah, virtual memory
void *R_Hunk_Begin( void );
void R_Hunk_End( void );
void R_FreeImageBuffer( void );

#if defined RTCW_ET
qboolean R_inPVS( const vec3_t p1, const vec3_t p2 );
#endif // RTCW_XX

// BBi
class OglTessLayout {
public:
	static const int MAX_VERTEX_COUNT = 2 * 4000;

	static const GLintptr POS_OFS;
	static const GLintptr TC0_OFS;
	static const GLintptr TC1_OFS;
	static const GLintptr COL_OFS;

	static const GLvoid* POS_PTR;
	static const GLvoid* TC0_PTR;
	static const GLvoid* TC1_PTR;
	static const GLvoid* COL_PTR;

	static const size_t POS_SIZE;
	static const size_t TC0_SIZE;
	static const size_t TC1_SIZE;
	static const size_t COL_SIZE;

	rtcw::cgm::Vec4 position[MAX_VERTEX_COUNT];
	rtcw::cgm::Vec2 texture_coords[2][MAX_VERTEX_COUNT];
	color4ub_t color[MAX_VERTEX_COUNT];
}; // class OglTessLayout

extern rtcw::OglTessState ogl_tess_state;

extern GLuint ogl_tess_vbo;
extern int ogl_tess_base_vertex;
extern rtcw::OglTessProgram* ogl_tess_program;

extern GLuint ogl_tess2_vbo;
extern int ogl_tess2_base_vertex;
extern OglTessLayout ogl_tess2;

const int ogl_tess_default_vao_index = 0;
const int ogl_tess_default_vao_count = 1;

const int ogl_tess_vao_base_index = ogl_tess_default_vao_index + ogl_tess_default_vao_count;
const int ogl_tess_vao_count = (1 << 3) - 1;

const int ogl_tess2_vao_base_index = ogl_tess_vao_base_index + ogl_tess_vao_count;
const int ogl_tess2_vao_count = (1 << 2) - 1;

const int ogl_tess_vao_total_count = ogl_tess_default_vao_count + ogl_tess_vao_count + ogl_tess2_vao_count;

typedef GLuint OglTessVaos[ogl_tess_vao_total_count];

extern bool ogl_tess_use_vao;
extern OglTessVaos ogl_tess_vaos;

void ogl_tess2_draw (GLenum mode, int vertex_count,
	bool use_texture_coords, bool use_color);


GLenum r_get_best_wrap_clamp ();
void r_reload_programs_f ();

extern rtcw::OglMatrixStack ogl_model_view_stack;
extern rtcw::OglMatrixStack ogl_projection_stack;

bool r_probe_programs ();
// BBi

#endif //TR_LOCAL_H (THIS MUST BE LAST!!)
