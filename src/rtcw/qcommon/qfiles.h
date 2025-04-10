/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef __QFILES_H__
#define __QFILES_H__

//
// qfiles.h: quake file formats
// This file must be identical in the quake and utils directories
//

// surface geometry should not exceed these limits

#if defined RTCW_SP
#define SHADER_MAX_VERTEXES 4000
#elif defined RTCW_MP
#define SHADER_MAX_VERTEXES 1000 // JPW NERVE was 4000, 1000 in q3ta
#else
#define SHADER_MAX_VERTEXES 1025 // Arnout: 1024+1 (1 buffer for RB_EndSurface overflow check) // JPW NERVE was 4000, 1000 in q3ta
#endif // RTCW_XX

#define SHADER_MAX_INDEXES  ( 6 * SHADER_MAX_VERTEXES )


// the maximum size of game reletive pathnames
#define MAX_QPATH       64

/*
========================================================================

QVM files

========================================================================
*/

#define VM_MAGIC    0x12721444
typedef struct {
	int vmMagic;

	int instructionCount;

	int codeOffset;
	int codeLength;

	int dataOffset;
	int dataLength;
	int litLength;              // ( dataLength - litLength ) should be byteswapped on load
	int bssLength;              // zero filled memory appended to datalength
} vmHeader_t;


/*
========================================================================

PCX files are used for 8 bit images

========================================================================
*/

// BBi
//typedef struct {
//	char manufacturer;
//	char version;
//	char encoding;
//	char bits_per_pixel;
//	unsigned short xmin,ymin,xmax,ymax;
//	unsigned short hres,vres;
//	unsigned char palette[48];
//	char reserved;
//	char color_planes;
//	unsigned short bytes_per_line;
//	unsigned short palette_type;
//	char filler[58];
//	unsigned char data;             // unbounded
//} pcx_t;

struct pcx_t {
	char manufacturer;
	char version;
	char encoding;
	char bits_per_pixel;
	uint16_t xmin;
	uint16_t ymin;
	uint16_t xmax;
	uint16_t ymax;
	uint16_t hres;
	uint16_t vres;
	uint8_t palette[48];
	char reserved;
	char color_planes;
	uint16_t bytes_per_line;
	uint16_t palette_type;
	char filler[58];
	uint8_t data; // unbounded
}; // struct pcx_t
// BBi


/*
========================================================================

TGA files are used for 24/32 bit images

========================================================================
*/

// BBi
//typedef struct _TargaHeader {
//	unsigned char id_length, colormap_type, image_type;
//	unsigned short colormap_index, colormap_length;
//	unsigned char colormap_size;
//	unsigned short x_origin, y_origin, width, height;
//	unsigned char pixel_size, attributes;
//} TargaHeader;

struct TargaHeader {
	uint8_t id_length;
	uint8_t colormap_type;
	uint8_t image_type;
	uint16_t colormap_index;
	uint16_t colormap_length;
	uint8_t colormap_size;
	uint16_t x_origin;
	uint16_t y_origin;
	uint16_t width;
	uint16_t height;
	uint8_t pixel_size;
	uint8_t attributes;
}; // struct TargaHeader
// BBi



/*
========================================================================

.MD3 triangle model file format

========================================================================
*/

#define MD3_IDENT           ( ( '3' << 24 ) + ( 'P' << 16 ) + ( 'D' << 8 ) + 'I' )
#define MD3_VERSION         15

// limits

#if defined RTCW_SP
#define MD3_MAX_LODS        3
#elif defined RTCW_MP
#define MD3_MAX_LODS        1
#else
#define MD3_MAX_LODS        4
#endif // RTCW_XX

#define MD3_MAX_TRIANGLES   8192    // per surface
#define MD3_MAX_VERTS       4096    // per surface
#define MD3_MAX_SHADERS     256     // per surface
#define MD3_MAX_FRAMES      1024    // per model
#define MD3_MAX_SURFACES    32      // per model
#define MD3_MAX_TAGS        16      // per frame

// vertex scales
#define MD3_XYZ_SCALE       ( 1.0 / 64 )

// BBi
//typedef struct md3Frame_s {
//	vec3_t bounds[2];
//	vec3_t localOrigin;
//	float radius;
//	char name[16];
//} md3Frame_t;

struct md3Frame_t {
	vec3_t bounds[2];
	vec3_t localOrigin;
	float radius;
	char name[16];
}; // struct md3Frame_t
// BBi

// BBi
//typedef struct md3Tag_s {
//	char name[MAX_QPATH];           // tag name
//	vec3_t origin;
//	vec3_t axis[3];
//} md3Tag_t;

struct md3Tag_t {
	char name[MAX_QPATH]; // tag name
	vec3_t origin;
	vec3_t axis[3];
}; // struct md3Tag_t
// BBi

/*
** md3Surface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numFrames
*/

// BBi
//typedef struct {
//	int ident;                  //
//
//	char name[MAX_QPATH];       // polyset name
//
//	int flags;
//	int numFrames;              // all surfaces in a model should have the same
//
//	int numShaders;             // all surfaces in a model should have the same
//	int numVerts;
//
//	int numTriangles;
//	int ofsTriangles;
//
//	int ofsShaders;             // offset from start of md3Surface_t
//	int ofsSt;                  // texture coords are common for all frames
//	int ofsXyzNormals;          // numVerts * numFrames
//
//	int ofsEnd;                 // next surface follows
//} md3Surface_t;

struct md3Surface_t {
	int32_t ident;

	char name[MAX_QPATH]; // polyset name

	int32_t flags;
	int32_t numFrames; // all surfaces in a model should have the same

	int32_t numShaders; // all surfaces in a model should have the same
	int32_t numVerts;

	int32_t numTriangles;
	int32_t ofsTriangles;

	int32_t ofsShaders; // offset from start of md3Surface_t
	int32_t ofsSt; // texture coords are common for all frames
	int32_t ofsXyzNormals; // numVerts * numFrames

	int32_t ofsEnd; // next surface follows
}; // struct md3Surface_t
// BBi

// BBi
//typedef struct {
//	char name[MAX_QPATH];
//	int shaderIndex;                // for in-game use
//} md3Shader_t;

struct md3Shader_t {
	char name[MAX_QPATH];
	int32_t shaderIndex;                // for in-game use
}; // struct md3Shader_t
// BBi

// BBi
//typedef struct {
//	int indexes[3];
//} md3Triangle_t;

struct md3Triangle_t {
	int32_t indexes[3];
}; // struct md3Triangle_t
// BBi

typedef struct {
	float st[2];
} md3St_t;

// BBi
//typedef struct {
//	short xyz[3];
//	short normal;
//} md3XyzNormal_t;

struct md3XyzNormal_t {
	int16_t xyz[3];
	int16_t normal;
}; // struct md3XyzNormal_t
// BBi

// BBi
//typedef struct {
//	int ident;
//	int version;
//
//	char name[MAX_QPATH];           // model name
//
//	int flags;
//
//	int numFrames;
//	int numTags;
//	int numSurfaces;
//
//	int numSkins;
//
//	int ofsFrames;                  // offset for first frame
//	int ofsTags;                    // numFrames * numTags
//	int ofsSurfaces;                // first surface, others follow
//
//	int ofsEnd;                     // end of file
//} md3Header_t;

struct md3Header_t {
	int32_t ident;
	int32_t version;

	char name[MAX_QPATH];           // model name

	int32_t flags;

	int32_t numFrames;
	int32_t numTags;
	int32_t numSurfaces;

	int32_t numSkins;

	int32_t ofsFrames;                  // offset for first frame
	int32_t ofsTags;                    // numFrames * numTags
	int32_t ofsSurfaces;                // first surface, others follow

	int32_t ofsEnd;                     // end of file
}; // struct md3Header_t
// BBi

#if defined RTCW_ET
/*
========================================================================

.tag tag file format

========================================================================
*/

#define TAG_IDENT           ( ( '1' << 24 ) + ( 'G' << 16 ) + ( 'A' << 8 ) + 'T' )
#define TAG_VERSION         1

// BBi
//typedef struct {
//	int ident;
//	int version;
//
//	int numTags;
//
//	int ofsEnd;
//} tagHeader_t;

struct tagHeader_t {
	int32_t ident;
	int32_t version;
	int32_t numTags;
	int32_t ofsEnd;
}; // struct tagHeader_t
// BBi

// BBi
//typedef struct {
//	char filename[MAX_QPATH];
//	int start;
//	int count;
//} tagHeaderExt_t;

struct tagHeaderExt_t {
	char filename[MAX_QPATH];
	int32_t start;
	int32_t count;
}; // struct tagHeaderExt_t
// BBi
#endif // RTCW_XX

// Ridah, mesh compression
/*
==============================================================================

MDC file format

==============================================================================
*/

#define MDC_IDENT           ( ( 'C' << 24 ) + ( 'P' << 16 ) + ( 'D' << 8 ) + 'I' )
#define MDC_VERSION         2

// version history:
// 1 - original
// 2 - changed tag structure so it only lists the names once

// BBi
//typedef struct {
//	unsigned int ofsVec;                    // offset direction from the last base frame
////	unsigned short	ofsVec;
//} mdcXyzCompressed_t;

struct mdcXyzCompressed_t {
	uint32_t ofsVec; // offset direction from the last base frame
};
// BBi

typedef struct {
	char name[MAX_QPATH];           // tag name
} mdcTagName_t;

#define MDC_TAG_ANGLE_SCALE ( 360.0 / 32700.0 )

// BBi
//typedef struct {
//	short xyz[3];
//	short angles[3];
//} mdcTag_t;

struct mdcTag_t {
	int16_t xyz[3];
	int16_t angles[3];
}; // struct mdcTag_t
// BBi

/*
** mdcSurface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numBaseFrames
** XyzCompressed	sizeof( mdcXyzCompressed ) * numVerts * numCompFrames
** frameBaseFrames	sizeof( short ) * numFrames
** frameCompFrames	sizeof( short ) * numFrames (-1 if frame is a baseFrame)
*/

// BBi
//typedef struct {
//	int ident;                  //
//
//	char name[MAX_QPATH];       // polyset name
//
//	int flags;
//	int numCompFrames;          // all surfaces in a model should have the same
//	int numBaseFrames;          // ditto
//
//	int numShaders;             // all surfaces in a model should have the same
//	int numVerts;
//
//	int numTriangles;
//	int ofsTriangles;
//
//	int ofsShaders;             // offset from start of md3Surface_t
//	int ofsSt;                  // texture coords are common for all frames
//	int ofsXyzNormals;          // numVerts * numBaseFrames
//	int ofsXyzCompressed;       // numVerts * numCompFrames
//
//	int ofsFrameBaseFrames;     // numFrames
//	int ofsFrameCompFrames;     // numFrames
//
//	int ofsEnd;                 // next surface follows
//} mdcSurface_t;

struct mdcSurface_t {
	int32_t ident;                  //

	char name[MAX_QPATH];       // polyset name

	int32_t flags;
	int32_t numCompFrames;          // all surfaces in a model should have the same
	int32_t numBaseFrames;          // ditto

	int32_t numShaders;             // all surfaces in a model should have the same
	int32_t numVerts;

	int32_t numTriangles;
	int32_t ofsTriangles;

	int32_t ofsShaders;             // offset from start of md3Surface_t
	int32_t ofsSt;                  // texture coords are common for all frames
	int32_t ofsXyzNormals;          // numVerts * numBaseFrames
	int32_t ofsXyzCompressed;       // numVerts * numCompFrames

	int32_t ofsFrameBaseFrames;     // numFrames
	int32_t ofsFrameCompFrames;     // numFrames

	int32_t ofsEnd;                 // next surface follows
}; // struct mdcSurface_t
// BBi

// BBi
//typedef struct {
//	int ident;
//	int version;
//
//	char name[MAX_QPATH];           // model name
//
//	int flags;
//
//	int numFrames;
//	int numTags;
//	int numSurfaces;
//
//	int numSkins;
//
//	int ofsFrames;                  // offset for first frame, stores the bounds and localOrigin
//	int ofsTagNames;                // numTags
//	int ofsTags;                    // numFrames * numTags
//	int ofsSurfaces;                // first surface, others follow
//
//	int ofsEnd;                     // end of file
//} mdcHeader_t;

struct mdcHeader_t {
	int32_t ident;
	int32_t version;

	char name[MAX_QPATH]; // model name

	int32_t flags;

	int32_t numFrames;
	int32_t numTags;
	int32_t numSurfaces;

	int32_t numSkins;

	int32_t ofsFrames; // offset for first frame, stores the bounds and localOrigin
	int32_t ofsTagNames; // numTags
	int32_t ofsTags; // numFrames * numTags
	int32_t ofsSurfaces; // first surface, others follow

	int32_t ofsEnd; // end of file
}; // struct mdcHeader_t
// BBi
// done.

/*
==============================================================================

MD4 file format

==============================================================================
*/

#define MD4_IDENT           ( ( '4' << 24 ) + ( 'P' << 16 ) + ( 'D' << 8 ) + 'I' )
#define MD4_VERSION         1
#define MD4_MAX_BONES       128

// BBi
//typedef struct {
//	int boneIndex;              // these are indexes into the boneReferences,
//	float boneWeight;           // not the global per-frame bone list
//	vec3_t offset;
//} md4Weight_t;

struct md4Weight_t {
	int32_t boneIndex; // these are indexes into the boneReferences,
	float boneWeight; // not the global per-frame bone list
	vec3_t offset;
}; // struct md4Weight_t
// BBi

// BBi
//typedef struct {
//	vec3_t normal;
//	vec2_t texCoords;
//	int numWeights;
//	md4Weight_t weights[1];     // variable sized
//} md4Vertex_t;

struct md4Vertex_t {
	vec3_t normal;
	vec2_t texCoords;
	int32_t numWeights;
	md4Weight_t weights[1]; // variable sized
}; // struct md4Vertex_t
// BBi

// BBi
//typedef struct {
//	int indexes[3];
//} md4Triangle_t;

struct md4Triangle_t {
	int32_t indexes[3];
}; // struct md4Triangle_t
// BBi

// BBi
//typedef struct {
//	int ident;
//
//	char name[MAX_QPATH];           // polyset name
//	char shader[MAX_QPATH];
//	int shaderIndex;                // for in-game use
//
//	int ofsHeader;                  // this will be a negative number
//
//	int numVerts;
//	int ofsVerts;
//
//	int numTriangles;
//	int ofsTriangles;
//
//	// Bone references are a set of ints representing all the bones
//	// present in any vertex weights for this surface.  This is
//	// needed because a model may have surfaces that need to be
//	// drawn at different sort times, and we don't want to have
//	// to re-interpolate all the bones for each surface.
//	int numBoneReferences;
//	int ofsBoneReferences;
//
//	int ofsEnd;                     // next surface follows
//} md4Surface_t;

struct md4Surface_t {
	int32_t ident;

	char name[MAX_QPATH]; // polyset name
	char shader[MAX_QPATH];
	int32_t shaderIndex; // for in-game use

	int32_t ofsHeader; // this will be a negative number

	int32_t numVerts;
	int32_t ofsVerts;

	int32_t numTriangles;
	int32_t ofsTriangles;

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int32_t numBoneReferences;
	int32_t ofsBoneReferences;

	int32_t ofsEnd; // next surface follows
}; // struct md4Surface_t
// BBi

typedef struct {
	float matrix[3][4];
} md4Bone_t;

typedef struct {
	vec3_t bounds[2];               // bounds of all surfaces of all LOD's for this frame
	vec3_t localOrigin;             // midpoint of bounds, used for sphere cull
	float radius;                   // dist from localOrigin to corner
	char name[16];
	md4Bone_t bones[1];             // [numBones]
} md4Frame_t;

// BBi
//typedef struct {
//	int numSurfaces;
//	int ofsSurfaces;                // first surface, others follow
//	int ofsEnd;                     // next lod follows
//} md4LOD_t;

struct md4LOD_t {
	int32_t numSurfaces;
	int32_t ofsSurfaces;                // first surface, others follow
	int32_t ofsEnd;                     // next lod follows
}; // struct md4LOD_t
// BBi

// BBi
//typedef struct {
//	int ident;
//	int version;
//
//	char name[MAX_QPATH];           // model name
//
//	// frames and bones are shared by all levels of detail
//	int numFrames;
//	int numBones;
//	int ofsFrames;                  // md4Frame_t[numFrames]
//
//	// each level of detail has completely separate sets of surfaces
//	int numLODs;
//	int ofsLODs;
//
//	int ofsEnd;                     // end of file
//} md4Header_t;

typedef struct {
	int32_t ident;
	int32_t version;

	char name[MAX_QPATH]; // model name

	// frames and bones are shared by all levels of detail
	int32_t numFrames;
	int32_t numBones;
	int32_t ofsFrames; // md4Frame_t[numFrames]

	// each level of detail has completely separate sets of surfaces
	int32_t numLODs;
	int32_t ofsLODs;

	int32_t ofsEnd; // end of file
} md4Header_t;
// BBi

/*
==============================================================================

MDS file format (Wolfenstein Skeletal Format)

==============================================================================
*/

#define MDS_IDENT           ( ( 'W' << 24 ) + ( 'S' << 16 ) + ( 'D' << 8 ) + 'M' )
#define MDS_VERSION         4
#define MDS_MAX_VERTS       6000
#define MDS_MAX_TRIANGLES   8192
#define MDS_MAX_BONES       128
#define MDS_MAX_SURFACES    32
#define MDS_MAX_TAGS        128

#define MDS_TRANSLATION_SCALE   ( 1.0 / 64 )

// BBi
//typedef struct {
//	int boneIndex;              // these are indexes into the boneReferences,
//	float boneWeight;           // not the global per-frame bone list
//	vec3_t offset;
//} mdsWeight_t;

struct mdsWeight_t {
	int32_t boneIndex; // these are indexes into the boneReferences,
	float boneWeight; // not the global per-frame bone list
	vec3_t offset;
}; // struct mdsWeight_t
// BBi

// BBi
//typedef struct {
//	vec3_t normal;
//	vec2_t texCoords;
//	int numWeights;
//	int fixedParent;            // stay equi-distant from this parent
//	float fixedDist;
//	mdsWeight_t weights[1];     // variable sized
//} mdsVertex_t;

struct mdsVertex_t {
	vec3_t normal;
	vec2_t texCoords;
	int32_t numWeights;
	int32_t fixedParent; // stay equi-distant from this parent
	float fixedDist;
	mdsWeight_t weights[1]; // variable sized
}; // struct mdsVertex_t
// BBi

// BBi
//typedef struct {
//	int indexes[3];
//} mdsTriangle_t;

struct mdsTriangle_t {
	int32_t indexes[3];
}; // struct mdsTriangle_t
// BBi

// BBi
//typedef struct {
//	int ident;
//
//	char name[MAX_QPATH];           // polyset name
//	char shader[MAX_QPATH];
//	int shaderIndex;                // for in-game use
//
//	int minLod;
//
//	int ofsHeader;                  // this will be a negative number
//
//	int numVerts;
//	int ofsVerts;
//
//	int numTriangles;
//	int ofsTriangles;
//
//	int ofsCollapseMap;           // numVerts * int
//
//	// Bone references are a set of ints representing all the bones
//	// present in any vertex weights for this surface.  This is
//	// needed because a model may have surfaces that need to be
//	// drawn at different sort times, and we don't want to have
//	// to re-interpolate all the bones for each surface.
//	int numBoneReferences;
//	int ofsBoneReferences;
//
//	int ofsEnd;                     // next surface follows
//} mdsSurface_t;

struct mdsSurface_t {
	int32_t ident;

	char name[MAX_QPATH]; // polyset name
	char shader[MAX_QPATH];
	int32_t shaderIndex; // for in-game use

	int32_t minLod;

	int32_t ofsHeader; // this will be a negative number

	int32_t numVerts;
	int32_t ofsVerts;

	int32_t numTriangles;
	int32_t ofsTriangles;

	int32_t ofsCollapseMap; // numVerts * int32_t

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int32_t numBoneReferences;
	int32_t ofsBoneReferences;

	int32_t ofsEnd; // next surface follows
}; // struct mdsSurface_t
// BBi

// BBi
//typedef struct {
//	//float		angles[3];
//	//float		ofsAngles[2];
//	short angles[4];            // to be converted to axis at run-time (this is also better for lerping)
//	short ofsAngles[2];         // PITCH/YAW, head in this direction from parent to go to the offset position
//} mdsBoneFrameCompressed_t;

struct mdsBoneFrameCompressed_t {
	int16_t angles[4]; // to be converted to axis at run-time (this is also better for lerping)
	int16_t ofsAngles[2]; // PITCH/YAW, head in this direction from parent to go to the offset position
}; // struct mdsBoneFrameCompressed_t
// BBi

// NOTE: this only used at run-time
typedef struct {
	float matrix[3][3];             // 3x3 rotation
	vec3_t translation;             // translation vector
} mdsBoneFrame_t;

typedef struct {
	vec3_t bounds[2];               // bounds of all surfaces of all LOD's for this frame
	vec3_t localOrigin;             // midpoint of bounds, used for sphere cull
	float radius;                   // dist from localOrigin to corner
	vec3_t parentOffset;            // one bone is an ascendant of all other bones, it starts the hierachy at this position
	mdsBoneFrameCompressed_t bones[1];              // [numBones]
} mdsFrame_t;

// BBi
//typedef struct {
//	int numSurfaces;
//	int ofsSurfaces;                // first surface, others follow
//	int ofsEnd;                     // next lod follows
//} mdsLOD_t;

struct mdsLOD_t {
	int32_t numSurfaces;
	int32_t ofsSurfaces; // first surface, others follow
	int32_t ofsEnd; // next lod follows
}; // struct mdsLOD_t
// BBi

// BBi
//typedef struct {
//	char name[MAX_QPATH];           // name of tag
//	float torsoWeight;
//	int boneIndex;                  // our index in the bones
//} mdsTag_t;

struct mdsTag_t {
	char name[MAX_QPATH]; // name of tag
	float torsoWeight;
	int32_t boneIndex; // our index in the bones
}; // struct mdsTag_t
// BBi

#define BONEFLAG_TAG        1       // this bone is actually a tag

// BBi
//typedef struct {
//	char name[MAX_QPATH];           // name of bone
//	int parent;                     // not sure if this is required, no harm throwing it in
//	float torsoWeight;              // scale torso rotation about torsoParent by this
//	float parentDist;
//	int flags;
//} mdsBoneInfo_t;

struct mdsBoneInfo_t {
	char name[MAX_QPATH]; // name of bone
	int32_t parent; // not sure if this is required, no harm throwing it in
	float torsoWeight; // scale torso rotation about torsoParent by this
	float parentDist;
	int32_t flags;
}; // struct mdsBoneInfo_t
// BBi

// BBi
//typedef struct {
//	int ident;
//	int version;
//
//	char name[MAX_QPATH];           // model name
//
//	float lodScale;
//	float lodBias;
//
//	// frames and bones are shared by all levels of detail
//	int numFrames;
//	int numBones;
//	int ofsFrames;                  // md4Frame_t[numFrames]
//	int ofsBones;                   // mdsBoneInfo_t[numBones]
//	int torsoParent;                // index of bone that is the parent of the torso
//
//	int numSurfaces;
//	int ofsSurfaces;
//
//	// tag data
//	int numTags;
//	int ofsTags;                    // mdsTag_t[numTags]
//
//	int ofsEnd;                     // end of file
//} mdsHeader_t;

struct mdsHeader_t {
	int32_t ident;
	int32_t version;

	char name[MAX_QPATH]; // model name

	float lodScale;
	float lodBias;

	// frames and bones are shared by all levels of detail
	int32_t numFrames;
	int32_t numBones;
	int32_t ofsFrames; // md4Frame_t[numFrames]
	int32_t ofsBones; // mdsBoneInfo_t[numBones]
	int32_t torsoParent; // index of bone that is the parent of the torso

	int32_t numSurfaces;
	int32_t ofsSurfaces;

	// tag data
	int32_t numTags;
	int32_t ofsTags; // mdsTag_t[numTags]

	int32_t ofsEnd; // end of file
}; // struct mdsHeader_t
// BBi

#if defined RTCW_ET
/*
==============================================================================

MDM file format (Wolfenstein Skeletal Mesh)

version history:
	2 - initial version
	3 - removed all frame data, this format is pure mesh and bone references now

==============================================================================
*/

#define MDM_IDENT           ( ( 'W' << 24 ) + ( 'M' << 16 ) + ( 'D' << 8 ) + 'M' )
#define MDM_VERSION         3
#define MDM_MAX_VERTS       6000
#define MDM_MAX_TRIANGLES   8192
#define MDM_MAX_SURFACES    32
#define MDM_MAX_TAGS        128

#define MDM_TRANSLATION_SCALE   ( 1.0 / 64 )

// BBi
//typedef struct {
//	int boneIndex;              // these are indexes into the boneReferences,
//	float boneWeight;           // not the global per-frame bone list
//	vec3_t offset;
//} mdmWeight_t;

struct mdmWeight_t {
	int32_t boneIndex; // these are indexes into the boneReferences,
	float boneWeight; // not the global per-frame bone list
	vec3_t offset;
}; // struct mdmWeight_t
// BBi

// BBi
//typedef struct {
//	vec3_t normal;
//	vec2_t texCoords;
//	int numWeights;
//	mdmWeight_t weights[1];     // variable sized
//} mdmVertex_t;

struct mdmVertex_t {
	vec3_t normal;
	vec2_t texCoords;
	int32_t numWeights;
	mdmWeight_t weights[1];     // variable sized
}; // struct mdmVertex_t
// BBi

// BBi
//typedef struct {
//	int indexes[3];
//} mdmTriangle_t;

struct mdmTriangle_t {
	int32_t indexes[3];
}; // struct mdmTriangle_t
// BBi

// BBi
//typedef struct {
//	int ident;
//
//	char name[MAX_QPATH];           // polyset name
//	char shader[MAX_QPATH];
//	int shaderIndex;                // for in-game use
//
//	int minLod;
//
//	int ofsHeader;                  // this will be a negative number
//
//	int numVerts;
//	int ofsVerts;
//
//	int numTriangles;
//	int ofsTriangles;
//
//	int ofsCollapseMap;           // numVerts * int
//
//	// Bone references are a set of ints representing all the bones
//	// present in any vertex weights for this surface.  This is
//	// needed because a model may have surfaces that need to be
//	// drawn at different sort times, and we don't want to have
//	// to re-interpolate all the bones for each surface.
//	int numBoneReferences;
//	int ofsBoneReferences;
//
//	int ofsEnd;                     // next surface follows
//} mdmSurface_t;

struct mdmSurface_t {
	int32_t ident;

	char name[MAX_QPATH]; // polyset name
	char shader[MAX_QPATH];
	int32_t shaderIndex; // for in-game use

	int32_t minLod;

	int32_t ofsHeader; // this will be a negative number

	int32_t numVerts;
	int32_t ofsVerts;

	int32_t numTriangles;
	int32_t ofsTriangles;

	int32_t ofsCollapseMap; // numVerts * int32_t

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int32_t numBoneReferences;
	int32_t ofsBoneReferences;

	int32_t ofsEnd; // next surface follows
}; // struct mdmSurface_t
// BBi

/*typedef struct {
	vec3_t		bounds[2];			// bounds of all surfaces of all LOD's for this frame
	vec3_t		localOrigin;		// midpoint of bounds, used for sphere cull
	float		radius;				// dist from localOrigin to corner
	vec3_t		parentOffset;		// one bone is an ascendant of all other bones, it starts the hierachy at this position
} mdmFrame_t;*/

// BBi
//typedef struct {
//	int numSurfaces;
//	int ofsSurfaces;                // first surface, others follow
//	int ofsEnd;                     // next lod follows
//} mdmLOD_t;

struct mdmLOD_t {
	int32_t numSurfaces;
	int32_t ofsSurfaces;                // first surface, others follow
	int32_t ofsEnd;                     // next lod follows
}; // struct mdmLOD_t
// BBi

/*typedef struct {
	char		name[MAX_QPATH];	// name of tag
	float		torsoWeight;
	int			boneIndex;			// our index in the bones

	int			numBoneReferences;
	int			ofsBoneReferences;

	int			ofsEnd;				// next tag follows
} mdmTag_t;*/

// Tags always only have one parent bone

// BBi
//typedef struct {
//	char name[MAX_QPATH];           // name of tag
//	vec3_t axis[3];
//
//	int boneIndex;
//	vec3_t offset;
//
//	int numBoneReferences;
//	int ofsBoneReferences;
//
//	int ofsEnd;                     // next tag follows
//} mdmTag_t;

typedef struct {
	char name[MAX_QPATH]; // name of tag
	vec3_t axis[3];

	int32_t boneIndex;
	vec3_t offset;

	int32_t numBoneReferences;
	int32_t ofsBoneReferences;

	int32_t ofsEnd; // next tag follows
} mdmTag_t;
// BBi

// BBi
//typedef struct {
//	int ident;
//	int version;
//
//	char name[MAX_QPATH];           // model name
///*	char		bonesfile[MAX_QPATH];	// bone file
//
//#ifdef UTILS
//	int			skel;
//#else
//	// dummy in file, set on load to link to MDX
//	qhandle_t	skel;
//#endif // UTILS
//*/
//	float lodScale;
//	float lodBias;
//
//	// frames and bones are shared by all levels of detail
///*	int			numFrames;
//	int			ofsFrames;			// mdmFrame_t[numFrames]
//*/
//	int numSurfaces;
//	int ofsSurfaces;
//
//	// tag data
//	int numTags;
//	int ofsTags;
//
//	int ofsEnd;                     // end of file
//} mdmHeader_t;

struct mdmHeader_t {
	int32_t ident;
	int32_t version;

	char name[MAX_QPATH]; // model name
	float lodScale;
	float lodBias;

	// frames and bones are shared by all levels of detail
	int32_t numSurfaces;
	int32_t ofsSurfaces;

	// tag data
	int32_t numTags;
	int32_t ofsTags;

	int32_t ofsEnd; // end of file
}; // struct mdmHeader_t
// BBi

/*
==============================================================================

MDX file format (Wolfenstein Skeletal Data)

version history:
	1 - initial version
	2 - moved parentOffset from the mesh to the skeletal data file

==============================================================================
*/

#define MDX_IDENT           ( ( 'W' << 24 ) + ( 'X' << 16 ) + ( 'D' << 8 ) + 'M' )
#define MDX_VERSION         2
#define MDX_MAX_BONES       128

typedef struct {
	vec3_t bounds[2];               // bounds of this frame
	vec3_t localOrigin;             // midpoint of bounds, used for sphere cull
	float radius;                   // dist from localOrigin to corner
	vec3_t parentOffset;            // one bone is an ascendant of all other bones, it starts the hierachy at this position
} mdxFrame_t;

// BBi
//typedef struct {
//	//float		angles[3];
//	//float		ofsAngles[2];
//	short angles[4];                // to be converted to axis at run-time (this is also better for lerping)
//	short ofsAngles[2];             // PITCH/YAW, head in this direction from parent to go to the offset position
//} mdxBoneFrameCompressed_t;

struct mdxBoneFrameCompressed_t {
	int16_t angles[4]; // to be converted to axis at run-time (this is also better for lerping)
	int16_t ofsAngles[2]; // PITCH/YAW, head in this direction from parent to go to the offset position
}; // mdxBoneFrameCompressed_t
// BBi

// NOTE: this only used at run-time
// FIXME: do we really need this?
typedef struct {
	float matrix[3][3];             // 3x3 rotation
	vec3_t translation;             // translation vector
} mdxBoneFrame_t;

// BBi
//typedef struct {
//	char name[MAX_QPATH];           // name of bone
//	int parent;                     // not sure if this is required, no harm throwing it in
//	float torsoWeight;              // scale torso rotation about torsoParent by this
//	float parentDist;
//	int flags;
//} mdxBoneInfo_t;

struct mdxBoneInfo_t {
	char name[MAX_QPATH]; // name of bone
	int32_t parent; // not sure if this is required, no harm throwing it in
	float torsoWeight; // scale torso rotation about torsoParent by this
	float parentDist;
	int32_t flags;
}; // struct mdxBoneInfo_t
// BBi

// BBi
//typedef struct {
//	int ident;
//	int version;
//
//	char name[MAX_QPATH];           // model name
//
//	// bones are shared by all levels of detail
//	int numFrames;
//	int numBones;
//	int ofsFrames;                  // (mdxFrame_t + mdxBoneFrameCompressed_t[numBones]) * numframes
//	int ofsBones;                   // mdxBoneInfo_t[numBones]
//	int torsoParent;                // index of bone that is the parent of the torso
//
//	int ofsEnd;                     // end of file
//} mdxHeader_t;

struct mdxHeader_t {
	int32_t ident;
	int32_t version;

	char name[MAX_QPATH]; // model name

	// bones are shared by all levels of detail
	int32_t numFrames;
	int32_t numBones;
	int32_t ofsFrames; // (mdxFrame_t + mdxBoneFrameCompressed_t[numBones]) * numframes
	int32_t ofsBones; // mdxBoneInfo_t[numBones]
	int32_t torsoParent; // index of bone that is the parent of the torso

	int32_t ofsEnd; // end of file
}; // struct mdxHeader_t
// BBi
#endif // RTCW_XX

/*
==============================================================================

  .BSP file format

==============================================================================
*/


#define BSP_IDENT   ( ( 'P' << 24 ) + ( 'S' << 16 ) + ( 'B' << 8 ) + 'I' )
// little-endian "IBSP"

#define BSP_VERSION         47  // updated (9/12/2001) to sync everything up pre-beta


// there shouldn't be any problem with increasing these values at the
// expense of more memory allocation in the utilities
//#define	MAX_MAP_MODELS		0x400
#define MAX_MAP_MODELS      0x800

#if !defined RTCW_ET
#define MAX_MAP_BRUSHES     0x8000
#define MAX_MAP_ENTITIES    0x800
#else
#define MAX_MAP_BRUSHES     16384
#define MAX_MAP_ENTITIES    4096
#endif // RTCW_XX

#define MAX_MAP_ENTSTRING   0x40000
#define MAX_MAP_SHADERS     0x400

#define MAX_MAP_AREAS       0x100   // MAX_MAP_AREA_BYTES in q_shared must match!
#define MAX_MAP_FOGS        0x100

#if !defined RTCW_ET
#define MAX_MAP_PLANES      0x20000
#else
#define MAX_MAP_PLANES      0x40000
#endif // RTCW_XX

#define MAX_MAP_NODES       0x20000

#if !defined RTCW_ET
#define MAX_MAP_BRUSHSIDES  0x20000
#else
#define MAX_MAP_BRUSHSIDES  0x100000
#endif // RTCW_XX

#define MAX_MAP_LEAFS       0x20000
#define MAX_MAP_LEAFFACES   0x20000
#define MAX_MAP_LEAFBRUSHES 0x40000
#define MAX_MAP_PORTALS     0x20000
#define MAX_MAP_LIGHTING    0x800000
#define MAX_MAP_LIGHTGRID   0x800000
#define MAX_MAP_VISIBILITY  0x200000

#define MAX_MAP_DRAW_SURFS  0x20000
#define MAX_MAP_DRAW_VERTS  0x80000
#define MAX_MAP_DRAW_INDEXES    0x80000


// key / value pair sizes in the entities lump
#define MAX_KEY             32
#define MAX_VALUE           1024

// the editor uses these predefined yaw angles to orient entities up or down
#define ANGLE_UP            -1
#define ANGLE_DOWN          -2

#define LIGHTMAP_WIDTH      128
#define LIGHTMAP_HEIGHT     128

#define MAX_WORLD_COORD     ( 128 * 1024 )
#define MIN_WORLD_COORD     ( -128 * 1024 )
#define WORLD_SIZE          ( MAX_WORLD_COORD - MIN_WORLD_COORD )

//=============================================================================


// BBi
//typedef struct {
//	int fileofs, filelen;
//} lump_t;

struct lump_t {
	int32_t fileofs;
	int32_t filelen;
}; // struct lump_t
// BBi

#define LUMP_ENTITIES       0
#define LUMP_SHADERS        1
#define LUMP_PLANES         2
#define LUMP_NODES          3
#define LUMP_LEAFS          4
#define LUMP_LEAFSURFACES   5
#define LUMP_LEAFBRUSHES    6
#define LUMP_MODELS         7
#define LUMP_BRUSHES        8
#define LUMP_BRUSHSIDES     9
#define LUMP_DRAWVERTS      10
#define LUMP_DRAWINDEXES    11
#define LUMP_FOGS           12
#define LUMP_SURFACES       13
#define LUMP_LIGHTMAPS      14
#define LUMP_LIGHTGRID      15
#define LUMP_VISIBILITY     16
#define HEADER_LUMPS        17

// BBi
//typedef struct {
//	int ident;
//	int version;
//
//	lump_t lumps[HEADER_LUMPS];
//} dheader_t;

struct dheader_t {
	int32_t ident;
	int32_t version;

	lump_t lumps[HEADER_LUMPS];
}; // struct dheader_t
// BBi

// BBi
//typedef struct {
//	float mins[3], maxs[3];
//	int firstSurface, numSurfaces;
//	int firstBrush, numBrushes;
//} dmodel_t;

struct dmodel_t {
	float mins[3];
	float maxs[3];
	int32_t firstSurface;
	int32_t numSurfaces;
	int32_t firstBrush;
	int32_t numBrushes;
}; // struct dmodel_t
// BBi

// BBi
//typedef struct {
//	char shader[MAX_QPATH];
//	int surfaceFlags;
//	int contentFlags;
//} dshader_t;

struct dshader_t {
	char shader[MAX_QPATH];
	int32_t surfaceFlags;
	int32_t contentFlags;
}; // struct dshader_t
// BBi

// planes x^1 is allways the opposite of plane x

typedef struct {
	float normal[3];
	float dist;
} dplane_t;

// BBi
//typedef struct {
//	int planeNum;
//	int children[2];            // negative numbers are -(leafs+1), not nodes
//	int mins[3];                // for frustom culling
//	int maxs[3];
//} dnode_t;

struct dnode_t {
	int32_t planeNum;
	int32_t children[2]; // negative numbers are -(leafs+1), not nodes
	int32_t mins[3]; // for frustom culling
	int32_t maxs[3];
}; // struct dnode_t
// BBi

// BBi
//typedef struct {
//	int cluster;                    // -1 = opaque cluster (do I still store these?)
//	int area;
//
//	int mins[3];                    // for frustum culling
//	int maxs[3];
//
//	int firstLeafSurface;
//	int numLeafSurfaces;
//
//	int firstLeafBrush;
//	int numLeafBrushes;
//} dleaf_t;

struct dleaf_t {
	int32_t cluster; // -1 = opaque cluster (do I still store these?)
	int32_t area;

	int32_t mins[3]; // for frustum culling
	int32_t maxs[3];

	int32_t firstLeafSurface;
	int32_t numLeafSurfaces;

	int32_t firstLeafBrush;
	int32_t numLeafBrushes;
}; // struct dleaf_t
// BBi

// BBi
//typedef struct {
//	int planeNum;                   // positive plane side faces out of the leaf
//	int shaderNum;
//} dbrushside_t;

struct dbrushside_t {
	int32_t planeNum; // positive plane side faces out of the leaf
	int32_t shaderNum;
}; // struct dbrushside_t
// BBi

// BBi
//typedef struct {
//	int firstSide;
//	int numSides;
//	int shaderNum;              // the shader that determines the contents flags
//} dbrush_t;

struct dbrush_t {
	int32_t firstSide;
	int32_t numSides;
	int32_t shaderNum; // the shader that determines the contents flags
}; // struct dbrush_t
// BBi

// BBi
//typedef struct {
//	char shader[MAX_QPATH];
//	int brushNum;
//	int visibleSide;            // the brush side that ray tests need to clip against (-1 == none)
//} dfog_t;

struct dfog_t {
	char shader[MAX_QPATH];
	int32_t brushNum;
	int32_t visibleSide; // the brush side that ray tests need to clip against (-1 == none)
}; // struct dfog_t
// BBi

// BBi
//typedef struct {
//	vec3_t xyz;
//	float st[2];
//	float lightmap[2];
//	vec3_t normal;
//	byte color[4];
//} drawVert_t;

struct drawVert_t {
	vec3_t xyz;
	float st[2];
	float lightmap[2];
	vec3_t normal;
	uint8_t color[4];
}; // struct drawVert_t
// BBi

typedef enum {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,

#if !defined RTCW_ET
	MST_FLARE
#else
	MST_FLARE,
	MST_FOLIAGE
#endif // RTCW_XX

} mapSurfaceType_t;

// BBi
//typedef struct {
//	int shaderNum;
//	int fogNum;
//	int surfaceType;
//
//	int firstVert;
//	int numVerts;
//
//	int firstIndex;
//	int numIndexes;
//
//	int lightmapNum;
//	int lightmapX, lightmapY;
//	int lightmapWidth, lightmapHeight;
//
//	vec3_t lightmapOrigin;
//	vec3_t lightmapVecs[3];         // for patches, [0] and [1] are lodbounds
//
//	int patchWidth;
//	int patchHeight;
//} dsurface_t;

struct dsurface_t {
	int32_t shaderNum;
	int32_t fogNum;
	int32_t surfaceType;

	int32_t firstVert;
	int32_t numVerts;

	int32_t firstIndex;
	int32_t numIndexes;

	int32_t lightmapNum;
	int32_t lightmapX, lightmapY;
	int32_t lightmapWidth, lightmapHeight;

	vec3_t lightmapOrigin;
	vec3_t lightmapVecs[3]; // for patches, [0] and [1] are lodbounds

	int32_t patchWidth;
	int32_t patchHeight;
}; // struct dsurface_t
// BBi

//----(SA) added so I didn't change the dsurface_t struct (and thereby the bsp format) for something that doesn't need to be stored in the bsp
typedef struct {
	char        *lighttarg;
} drsurfaceInternal_t;
//----(SA) end

#endif
