/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "qfiles.h"

#include "tr_types.h"

void        CM_LoadMap( const char *name, qboolean clientload, int *checksum );

#if !defined RTCW_SP
void        CM_ClearMap( void );
#endif // RTCW_XX

clipHandle_t CM_InlineModel( int index );       // 0 = world, 1 + are bmodels
clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int capsule );

#if !defined RTCW_SP
void        CM_SetTempBoxModelContents( int contents );     // DHM - Nerve
#endif // RTCW_XX

void        CM_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );

int         CM_NumClusters( void );
int         CM_NumInlineModels( void );
char        *CM_EntityString( void );

// returns an ORed contents mask
int         CM_PointContents( const vec3_t p, clipHandle_t model );
int         CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );

void        CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						 const vec3_t mins, const vec3_t maxs,
						 clipHandle_t model, int brushmask, int capsule );
void        CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
									const vec3_t mins, const vec3_t maxs,
									clipHandle_t model, int brushmask,
									const vec3_t origin, const vec3_t angles, int capsule );

byte        *CM_ClusterPVS( int cluster );

int         CM_PointLeafnum( const vec3_t p );

// only returns non-solid leafs
// overflow if return listsize and if *lastLeaf != list[listsize-1]
int         CM_BoxLeafnums( const vec3_t mins, const vec3_t maxs, int *list,
							int listsize, int *lastLeaf );

int         CM_LeafCluster( int leafnum );
int         CM_LeafArea( int leafnum );

void        CM_AdjustAreaPortalState( int area1, int area2, qboolean open );
qboolean    CM_AreasConnected( int area1, int area2 );

int         CM_WriteAreaBits( byte *buffer, int area );

// cm_tag.c
int         CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );


// cm_marks.c
int CM_MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection,
					  int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer );

// cm_patch.c
void CM_DrawDebugSurface( void ( *drawPoly )( int color, int numPoints, float *points ) );
