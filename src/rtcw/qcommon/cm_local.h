/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "q_shared.h"
#include "qcommon.h"
#include "cm_polylib.h"


//	(SA) DM needs more than 256 since this includes func_static and func_explosives
//#define	MAX_SUBMODELS		256
//#define	BOX_MODEL_HANDLE	255

#define MAX_SUBMODELS           512
#define BOX_MODEL_HANDLE        511
#define CAPSULE_MODEL_HANDLE    510

#if defined RTCW_ET
// enable to make the collision detection a bunch faster
#define MRE_OPTIMIZE
#endif // RTCW_XX

typedef struct {
	cplane_t    *plane;
	int children[2];                // negative numbers are leafs
} cNode_t;

typedef struct {
	int cluster;
	int area;

	int firstLeafBrush;
	int numLeafBrushes;

	int firstLeafSurface;
	int numLeafSurfaces;
} cLeaf_t;

typedef struct cmodel_s {
	vec3_t mins, maxs;
	cLeaf_t leaf;               // submodels don't reference the main tree
} cmodel_t;

typedef struct {
	cplane_t    *plane;
	int surfaceFlags;
	int shaderNum;
} cbrushside_t;

typedef struct {
	int shaderNum;              // the shader that determined the contents
	int contents;
	vec3_t bounds[2];
	int numsides;
	cbrushside_t    *sides;
	int checkcount;             // to avoid repeated testings
} cbrush_t;


typedef struct {
	int checkcount;                     // to avoid repeated testings
	int surfaceFlags;
	int contents;
	struct patchCollide_s   *pc;
} cPatch_t;


typedef struct {
	int floodnum;
	int floodvalid;
} cArea_t;

typedef struct {
	char name[MAX_QPATH];

	int numShaders;
	dshader_t   *shaders;

	int numBrushSides;
	cbrushside_t *brushsides;

	int numPlanes;
	cplane_t    *planes;

	int numNodes;
	cNode_t     *nodes;

	int numLeafs;
	cLeaf_t     *leafs;

	int numLeafBrushes;
	int         *leafbrushes;

	int numLeafSurfaces;
	int         *leafsurfaces;

	int numSubModels;
	cmodel_t    *cmodels;

	int numBrushes;
	cbrush_t    *brushes;

	int numClusters;
	int clusterBytes;
	byte        *visibility;
	qboolean vised;             // if false, visibility is just a single cluster of ffs

	int numEntityChars;
	char        *entityString;

	int numAreas;
	cArea_t     *areas;
	int         *areaPortals;   // [ numAreas*numAreas ] reference counts

	int numSurfaces;
	cPatch_t    **surfaces;         // non-patches will be NULL

	int floodvalid;
	int checkcount;                         // incremented on each trace
} clipMap_t;


// keep 1/8 unit away to keep the position valid before network snapping
// and to avoid various numeric issues
#define SURFACE_CLIP_EPSILON    ( 0.125 )

extern clipMap_t cm;
extern int c_pointcontents;
extern int c_traces, c_brush_traces, c_patch_traces;
extern cvar_t      *cm_noAreas;
extern cvar_t      *cm_noCurves;
extern cvar_t      *cm_playerCurveClip;

#if defined RTCW_ET
extern cvar_t      *cm_optimize;
#endif // RTCW_XX

// cm_test.c

// Used for oriented capsule collision detection
typedef struct
{
	qboolean use;
	float radius;
	float halfheight;
	vec3_t offset;
} sphere_t;

typedef struct {
	vec3_t start;
	vec3_t end;
	vec3_t size[2];         // size of the box being swept through the model
	vec3_t offsets[8];      // [signbits][x] = either size[0][x] or size[1][x]
	float maxOffset;        // longest corner length from origin
	vec3_t extents;         // greatest of abs(size[0]) and abs(size[1])
	vec3_t bounds[2];       // enclosing box of start and end surrounding by size
	vec3_t modelOrigin;     // origin of the model tracing through
	int contents;           // ored contents of the model tracing through
	qboolean isPoint;       // optimized case
	trace_t trace;          // returned from trace call
	sphere_t sphere;        // sphere for oriendted capsule collision

#if defined RTCW_ET
#ifdef MRE_OPTIMIZE
	cplane_t tracePlane1;
	cplane_t tracePlane2;
	float traceDist1;
	float traceDist2;
	vec3_t dir;
#endif
#endif // RTCW_XX

} traceWork_t;

typedef struct leafList_s {
	int count;
	int maxcount;
	qboolean overflowed;
	int     *list;
	vec3_t bounds[2];
	int lastLeaf;           // for overflows where each leaf can't be stored individually
	void ( *storeLeafs )( struct leafList_s *ll, int nodenum );
} leafList_t;


int CM_BoxBrushes( const vec3_t mins, const vec3_t maxs, cbrush_t **list, int listsize );

void CM_StoreLeafs( leafList_t *ll, int nodenum );
void CM_StoreBrushes( leafList_t *ll, int nodenum );

void CM_BoxLeafnums_r( leafList_t *ll, int nodenum );

cmodel_t    *CM_ClipHandleToModel( clipHandle_t handle );

// cm_patch.c

#if !defined RTCW_ET
struct patchCollide_s   *CM_GeneratePatchCollide( int width, int height, vec3_t *points );
#else
struct patchCollide_s   *CM_GeneratePatchCollide( int width, int height, vec3_t *points, qboolean addBevels );
#endif // RTCW_XX

void CM_TraceThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
qboolean CM_PositionTestInPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
void CM_ClearLevelPatches( void );
