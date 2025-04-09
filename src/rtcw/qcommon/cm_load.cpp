/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

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

// cmodel.c -- model loading

#include "cm_local.h"
#include "rtcw_endian.h"

#ifdef BSPC

#include "../bspc/l_qfiles.h"

void SetPlaneSignbits( cplane_t *out ) {
	int bits, j;

	// for fast box on planeside test
	bits = 0;
	for ( j = 0 ; j < 3 ; j++ ) {
		if ( out->normal[j] < 0 ) {
			bits |= 1 << j;
		}
	}
	out->signbits = bits;
}
#endif //BSPC

// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map

#if defined RTCW_ET
#define BOX_LEAF_BRUSHES    1   // ydnar
#endif // RTCW_XX

#define BOX_BRUSHES     1
#define BOX_SIDES       6
#define BOX_LEAFS       2
#define BOX_PLANES      12

#define LL( x ) x = rtcw::Endian::le( x )


clipMap_t cm;
int c_pointcontents;
int c_traces, c_brush_traces, c_patch_traces;


byte        *cmod_base;

#ifndef BSPC
cvar_t      *cm_noAreas;
cvar_t      *cm_noCurves;
cvar_t      *cm_playerCurveClip;

#if defined RTCW_ET
cvar_t      *cm_optimize;
#endif // RTCW_XX

#endif

cmodel_t box_model;
cplane_t    *box_planes;
cbrush_t    *box_brush;



void    CM_InitBoxHull( void );
void    CM_FloodAreaConnections( void );


/*
===============================================================================

					MAP LOADING

===============================================================================
*/

/*
=================
CMod_LoadShaders
=================
*/
void CMod_LoadShaders( lump_t *l ) {
	dshader_t   *in, *out;
	int i, count;

	in = reinterpret_cast<dshader_t*> ( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "CMod_LoadShaders: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map with no shaders" );
	}
	cm.shaders = static_cast<dshader_t*> (Hunk_Alloc( count * sizeof( *cm.shaders ), h_high ));
	cm.numShaders = count;

#if defined RTCW_SP
	Com_Memcpy( cm.shaders, in, count * sizeof( *cm.shaders ) );
#else
	memcpy( cm.shaders, in, count * sizeof( *cm.shaders ) );
#endif // RTCW_XX

	if (!rtcw::Endian::is_little ()) {
		out = cm.shaders;
		for ( i = 0 ; i < count ; i++, in++, out++ ) {
			rtcw::Endian::lei(out->contentFlags);
			rtcw::Endian::lei(out->surfaceFlags);
		}
	}
}


/*
=================
CMod_LoadSubmodels
=================
*/
void CMod_LoadSubmodels( lump_t *l ) {
	dmodel_t    *in;
	cmodel_t    *out;
	int i, j, count;
	int         *indexes;

	in = reinterpret_cast<dmodel_t*> ( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "CMod_LoadSubmodels: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map with no models" );
	}
	cm.cmodels = static_cast<cmodel_t*> (Hunk_Alloc( count * sizeof( *cm.cmodels ), h_high ));
	cm.numSubModels = count;

	if ( count > MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "MAX_SUBMODELS exceeded" );
	}

	for ( i = 0 ; i < count ; i++, in++, out++ )
	{
		out = &cm.cmodels[i];

		for ( j = 0 ; j < 3 ; j++ )
		{   // spread the mins / maxs by a pixel
			out->mins[j] = rtcw::Endian::le( in->mins[j] ) - 1;
			out->maxs[j] = rtcw::Endian::le( in->maxs[j] ) + 1;
		}

		if ( i == 0 ) {
			continue;   // world model doesn't need other info
		}

		// make a "leaf" just to hold the model's brushes and surfaces
		out->leaf.numLeafBrushes = rtcw::Endian::le( in->numBrushes );
		indexes = static_cast<int*> (Hunk_Alloc( out->leaf.numLeafBrushes * 4, h_high ));
		out->leaf.firstLeafBrush = indexes - cm.leafbrushes;
		for ( j = 0 ; j < out->leaf.numLeafBrushes ; j++ ) {
			indexes[j] = rtcw::Endian::le( in->firstBrush ) + j;
		}

		out->leaf.numLeafSurfaces = rtcw::Endian::le( in->numSurfaces );
		indexes = static_cast<int*> (Hunk_Alloc( out->leaf.numLeafSurfaces * 4, h_high ));
		out->leaf.firstLeafSurface = indexes - cm.leafsurfaces;
		for ( j = 0 ; j < out->leaf.numLeafSurfaces ; j++ ) {
			indexes[j] = rtcw::Endian::le( in->firstSurface ) + j;
		}
	}
}


/*
=================
CMod_LoadNodes

=================
*/
void CMod_LoadNodes( lump_t *l ) {
	dnode_t     *in;
	int child;
	cNode_t     *out;
	int i, j, count;

	in = reinterpret_cast<dnode_t*> ( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map has no nodes" );
	}
	cm.nodes = static_cast<cNode_t*> (Hunk_Alloc( count * sizeof( *cm.nodes ), h_high ));
	cm.numNodes = count;

	out = cm.nodes;

	for ( i = 0 ; i < count ; i++, out++, in++ )
	{
		out->plane = cm.planes + rtcw::Endian::le( in->planeNum );
		for ( j = 0 ; j < 2 ; j++ )
		{
			child = rtcw::Endian::le( in->children[j] );
			out->children[j] = child;
		}
	}

}

/*
=================
CM_BoundBrush

=================
*/
void CM_BoundBrush( cbrush_t *b ) {
	b->bounds[0][0] = -b->sides[0].plane->dist;
	b->bounds[1][0] = b->sides[1].plane->dist;

	b->bounds[0][1] = -b->sides[2].plane->dist;
	b->bounds[1][1] = b->sides[3].plane->dist;

	b->bounds[0][2] = -b->sides[4].plane->dist;
	b->bounds[1][2] = b->sides[5].plane->dist;
}


/*
=================
CMod_LoadBrushes

=================
*/
void CMod_LoadBrushes( lump_t *l ) {
	dbrush_t    *in;
	cbrush_t    *out;
	int i, count;

	in = reinterpret_cast<dbrush_t*> ( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	cm.brushes = static_cast<cbrush_t*> (Hunk_Alloc( ( BOX_BRUSHES + count ) * sizeof( *cm.brushes ), h_high ));
	cm.numBrushes = count;

	out = cm.brushes;

	for ( i = 0 ; i < count ; i++, out++, in++ ) {
		out->sides = cm.brushsides + rtcw::Endian::le( in->firstSide );
		out->numsides = rtcw::Endian::le( in->numSides );

		out->shaderNum = rtcw::Endian::le( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushes: bad shaderNum: %i", out->shaderNum );
		}
		out->contents = cm.shaders[out->shaderNum].contentFlags;

		CM_BoundBrush( out );
	}

}

/*
=================
CMod_LoadLeafs
=================
*/
void CMod_LoadLeafs( lump_t *l ) {
	int i;
	cLeaf_t     *out;
	dleaf_t     *in;
	int count;

	in = reinterpret_cast<dleaf_t*> ( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map with no leafs" );
	}

	cm.leafs = static_cast<cLeaf_t*> (Hunk_Alloc( ( BOX_LEAFS + count ) * sizeof( *cm.leafs ), h_high ));
	cm.numLeafs = count;

	out = cm.leafs;
	for ( i = 0 ; i < count ; i++, in++, out++ )
	{
		out->cluster = rtcw::Endian::le( in->cluster );
		out->area = rtcw::Endian::le( in->area );
		out->firstLeafBrush = rtcw::Endian::le( in->firstLeafBrush );
		out->numLeafBrushes = rtcw::Endian::le( in->numLeafBrushes );
		out->firstLeafSurface = rtcw::Endian::le( in->firstLeafSurface );
		out->numLeafSurfaces = rtcw::Endian::le( in->numLeafSurfaces );

		if ( out->cluster >= cm.numClusters ) {
			cm.numClusters = out->cluster + 1;
		}
		if ( out->area >= cm.numAreas ) {
			cm.numAreas = out->area + 1;
		}
	}

	cm.areas = static_cast<cArea_t*> (Hunk_Alloc( cm.numAreas * sizeof( *cm.areas ), h_high ));
	cm.areaPortals = static_cast<int*> (Hunk_Alloc( cm.numAreas * cm.numAreas * sizeof( *cm.areaPortals ), h_high ));
}

/*
=================
CMod_LoadPlanes
=================
*/
void CMod_LoadPlanes( lump_t *l ) {
	int i, j;
	cplane_t    *out;
	dplane_t    *in;
	int count;
	int bits;

	in = reinterpret_cast<dplane_t*> ( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map with no planes" );
	}
	cm.planes = static_cast<cplane_t*> (Hunk_Alloc( ( BOX_PLANES + count ) * sizeof( *cm.planes ), h_high ));
	cm.numPlanes = count;

	out = cm.planes;

	for ( i = 0 ; i < count ; i++, in++, out++ )
	{
		bits = 0;
		for ( j = 0 ; j < 3 ; j++ )
		{
			out->normal[j] = rtcw::Endian::le( in->normal[j] );
			if ( out->normal[j] < 0 ) {
				bits |= 1 << j;
			}
		}

		out->dist = rtcw::Endian::le( in->dist );
		out->type = PlaneTypeForNormal( out->normal );
		out->signbits = bits;
	}
}

/*
=================
CMod_LoadLeafBrushes
=================
*/
void CMod_LoadLeafBrushes( lump_t *l ) {
	int i;
	int         *out;
	int         *in;
	int count;

	in = reinterpret_cast<int*> ( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

#if !defined RTCW_ET
	cm.leafbrushes = static_cast<int*> (Hunk_Alloc( count * sizeof( *cm.leafbrushes ), h_high ));
#else
	// ydnar: more than <count> brushes are stored in leafbrushes...
	cm.leafbrushes = static_cast<int*> (Hunk_Alloc( ( BOX_LEAF_BRUSHES + count ) * sizeof( *cm.leafbrushes ), h_high ));
#endif // RTCW_XX

	cm.numLeafBrushes = count;

	out = cm.leafbrushes;

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		*out = rtcw::Endian::le( *in );
	}
}

/*
=================
CMod_LoadLeafSurfaces
=================
*/
void CMod_LoadLeafSurfaces( lump_t *l ) {
	int i;
	int         *out;
	int         *in;
	int count;

	in = reinterpret_cast<int*> ( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	cm.leafsurfaces = static_cast<int*> (Hunk_Alloc( count * sizeof( *cm.leafsurfaces ), h_high ));
	cm.numLeafSurfaces = count;

	out = cm.leafsurfaces;

	rtcw::Endian::le(in, count, out);
}

/*
=================
CMod_LoadBrushSides
=================
*/
void CMod_LoadBrushSides( lump_t *l ) {
	int i;
	cbrushside_t    *out;
	dbrushside_t    *in;
	int count;
	int num;

	in = reinterpret_cast<dbrushside_t*> ( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	cm.brushsides = static_cast<cbrushside_t*> (Hunk_Alloc( ( BOX_SIDES + count ) * sizeof( *cm.brushsides ), h_high ));
	cm.numBrushSides = count;

	out = cm.brushsides;

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		num = rtcw::Endian::le( in->planeNum );
		out->plane = &cm.planes[num];
		out->shaderNum = rtcw::Endian::le( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushSides: bad shaderNum: %i", out->shaderNum );
		}
		out->surfaceFlags = cm.shaders[out->shaderNum].surfaceFlags;
	}
}


/*
=================
CMod_LoadEntityString
=================
*/
void CMod_LoadEntityString( lump_t *l ) {
	cm.entityString = static_cast<char*> (Hunk_Alloc( l->filelen, h_high ));
	cm.numEntityChars = l->filelen;

#if defined RTCW_SP
	Com_Memcpy( cm.entityString, cmod_base + l->fileofs, l->filelen );
#else
	memcpy( cm.entityString, cmod_base + l->fileofs, l->filelen );
#endif // RTCW_XX

}

/*
=================
CMod_LoadVisibility
=================
*/
#define VIS_HEADER  8
void CMod_LoadVisibility( lump_t *l ) {
	int len;
	byte    *buf;

	len = l->filelen;
	if ( !len ) {
		cm.clusterBytes = ( cm.numClusters + 31 ) & ~31;
		cm.visibility = static_cast<byte*> (Hunk_Alloc( cm.clusterBytes, h_high ));

#if defined RTCW_SP
		Com_Memset( cm.visibility, 255, cm.clusterBytes );
#else
		memset( cm.visibility, 255, cm.clusterBytes );
#endif // RTCW_XX

		return;
	}
	buf = cmod_base + l->fileofs;

	cm.vised = qtrue;
	cm.visibility = static_cast<byte*> (Hunk_Alloc( len, h_high ));
	cm.numClusters = rtcw::Endian::le( ( (int *)buf )[0] );
	cm.clusterBytes = rtcw::Endian::le( ( (int *)buf )[1] );

#if defined RTCW_SP
	Com_Memcpy( cm.visibility, buf + VIS_HEADER, len - VIS_HEADER );
#else
	memcpy( cm.visibility, buf + VIS_HEADER, len - VIS_HEADER );
#endif // RTCW_XX

}

//==================================================================


/*
=================
CMod_LoadPatches
=================
*/
#define MAX_PATCH_VERTS     1024
void CMod_LoadPatches( lump_t *surfs, lump_t *verts ) {
	drawVert_t  *dv, *dv_p;
	dsurface_t  *in;
	int count;
	int i, j;
	int c;
	cPatch_t    *patch;
	vec3_t points[MAX_PATCH_VERTS];
	int width, height;
	int shaderNum;

	in = reinterpret_cast<dsurface_t*> ( cmod_base + surfs->fileofs );
	if ( surfs->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	cm.numSurfaces = count = surfs->filelen / sizeof( *in );
	cm.surfaces = static_cast<cPatch_t**> (Hunk_Alloc( cm.numSurfaces * sizeof( cm.surfaces[0] ), h_high ));

	dv = reinterpret_cast<drawVert_t*> ( cmod_base + verts->fileofs );
	if ( verts->filelen % sizeof( *dv ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}

	// scan through all the surfaces, but only load patches,
	// not planar faces
	for ( i = 0 ; i < count ; i++, in++ ) {
		if ( rtcw::Endian::le( in->surfaceType ) != MST_PATCH ) {
			continue;       // ignore other surfaces
		}
		// FIXME: check for non-colliding patches

		cm.surfaces[ i ] = patch = static_cast<cPatch_t*> (Hunk_Alloc( sizeof( *patch ), h_high ));

		// load the full drawverts onto the stack
		width = rtcw::Endian::le( in->patchWidth );
		height = rtcw::Endian::le( in->patchHeight );
		c = width * height;
		if ( c > MAX_PATCH_VERTS ) {
			Com_Error( ERR_DROP, "ParseMesh: MAX_PATCH_VERTS" );
		}

		dv_p = dv + rtcw::Endian::le( in->firstVert );
		for ( j = 0 ; j < c ; j++, dv_p++ )
			rtcw::Endian::le(dv_p->xyz, 3, points[j]);

		shaderNum = rtcw::Endian::le( in->shaderNum );
		patch->contents = cm.shaders[shaderNum].contentFlags;
		patch->surfaceFlags = cm.shaders[shaderNum].surfaceFlags;

		// create the internal facet structure

#if !defined RTCW_ET
		patch->pc = CM_GeneratePatchCollide( width, height, points );
#else
		patch->pc = CM_GeneratePatchCollide( width, height, points, qtrue );
#endif // RTCW_XX

	}
}

//==================================================================


#if 0 //BSPC
/*
==================
CM_FreeMap

Free any loaded map and all submodels
==================
*/
void CM_FreeMap( void ) {

#if defined RTCW_SP
	Com_Memset( &cm, 0, sizeof( cm ) );
#else
	memset( &cm, 0, sizeof( cm ) );
#endif // RTCW_XX

	Hunk_ClearHigh();
	CM_ClearLevelPatches();
}
#endif //BSPC

unsigned CM_LumpChecksum( lump_t *lump ) {
	return rtcw::Endian::le( Com_BlockChecksum( cmod_base + lump->fileofs, lump->filelen ) );
}

unsigned CM_Checksum( dheader_t *header ) {
	unsigned checksums[16];
	checksums[0] = CM_LumpChecksum( &header->lumps[LUMP_SHADERS] );
	checksums[1] = CM_LumpChecksum( &header->lumps[LUMP_LEAFS] );
	checksums[2] = CM_LumpChecksum( &header->lumps[LUMP_LEAFBRUSHES] );
	checksums[3] = CM_LumpChecksum( &header->lumps[LUMP_LEAFSURFACES] );
	checksums[4] = CM_LumpChecksum( &header->lumps[LUMP_PLANES] );
	checksums[5] = CM_LumpChecksum( &header->lumps[LUMP_BRUSHSIDES] );
	checksums[6] = CM_LumpChecksum( &header->lumps[LUMP_BRUSHES] );
	checksums[7] = CM_LumpChecksum( &header->lumps[LUMP_MODELS] );
	checksums[8] = CM_LumpChecksum( &header->lumps[LUMP_NODES] );
	checksums[9] = CM_LumpChecksum( &header->lumps[LUMP_SURFACES] );
	checksums[10] = CM_LumpChecksum( &header->lumps[LUMP_DRAWVERTS] );

	return rtcw::Endian::le( Com_BlockChecksum( checksums, 11 * 4 ) );
}

/*
==================
CM_LoadMap

Loads in the map and all submodels
==================
*/
void CM_LoadMap( const char *name, qboolean clientload, int *checksum ) {
	int             *buf;
	int i;
	dheader_t header;
	int length;
	static unsigned last_checksum;

	if ( !name || !name[0] ) {
		Com_Error( ERR_DROP, "CM_LoadMap: NULL name" );
	}

#ifndef BSPC
	cm_noAreas = Cvar_Get( "cm_noAreas", "0", CVAR_CHEAT );
	cm_noCurves = Cvar_Get( "cm_noCurves", "0", CVAR_CHEAT );
	cm_playerCurveClip = Cvar_Get( "cm_playerCurveClip", "1", CVAR_ARCHIVE | CVAR_CHEAT );

#if defined RTCW_ET
	cm_optimize = Cvar_Get( "cm_optimize", "1", CVAR_CHEAT );
#endif // RTCW_XX

#endif
	Com_DPrintf( "CM_LoadMap( %s, %i )\n", name, clientload );

	if ( !strcmp( cm.name, name ) && clientload ) {
		*checksum = last_checksum;
		return;
	}

	// free old stuff

#if defined RTCW_SP
	Com_Memset( &cm, 0, sizeof( cm ) );
#else
	memset( &cm, 0, sizeof( cm ) );
#endif // RTCW_XX

	CM_ClearLevelPatches();

	if ( !name[0] ) {
		cm.numLeafs = 1;
		cm.numClusters = 1;
		cm.numAreas = 1;
		cm.cmodels = static_cast<cmodel_t*> (Hunk_Alloc( sizeof( *cm.cmodels ), h_high ));
		*checksum = 0;
		return;
	}

	//
	// load the file
	//
#ifndef BSPC
	length = FS_ReadFile( name, (void **)&buf );
#else
	length = LoadQuakeFile( (quakefile_t *) name, (void **)&buf );
#endif

	if ( !buf ) {
		Com_Error( ERR_DROP, "Couldn't load %s", name );
	}

	last_checksum = rtcw::Endian::le( Com_BlockChecksum( buf, length ) );
	*checksum = last_checksum;

	header = *(dheader_t *)buf;
	for ( i = 0 ; i < sizeof( dheader_t ) / 4 ; i++ ) {
		( (int *)&header )[i] = rtcw::Endian::le( ( (int *)&header )[i] );
	}

#if !defined _SKIP_BSP_CHECK || !defined RTCW_SP
	if ( header.version != BSP_VERSION ) {
		Com_Error( ERR_DROP, "CM_LoadMap: %s has wrong version number (%i should be %i)"
				   , name, header.version, BSP_VERSION );
	}
#endif // RTCW_XX

	cmod_base = (byte *)buf;

	// load into heap
	CMod_LoadShaders( &header.lumps[LUMP_SHADERS] );
	CMod_LoadLeafs( &header.lumps[LUMP_LEAFS] );
	CMod_LoadLeafBrushes( &header.lumps[LUMP_LEAFBRUSHES] );
	CMod_LoadLeafSurfaces( &header.lumps[LUMP_LEAFSURFACES] );
	CMod_LoadPlanes( &header.lumps[LUMP_PLANES] );
	CMod_LoadBrushSides( &header.lumps[LUMP_BRUSHSIDES] );
	CMod_LoadBrushes( &header.lumps[LUMP_BRUSHES] );
	CMod_LoadSubmodels( &header.lumps[LUMP_MODELS] );
	CMod_LoadNodes( &header.lumps[LUMP_NODES] );
	CMod_LoadEntityString( &header.lumps[LUMP_ENTITIES] );
	CMod_LoadVisibility( &header.lumps[LUMP_VISIBILITY] );
	CMod_LoadPatches( &header.lumps[LUMP_SURFACES], &header.lumps[LUMP_DRAWVERTS] );

	// we are NOT freeing the file, because it is cached for the ref
	FS_FreeFile( buf );

	CM_InitBoxHull();

	CM_FloodAreaConnections();

	// allow this to be cached if it is loaded by the server
	if ( !clientload ) {
		Q_strncpyz( cm.name, name, sizeof( cm.name ) );
	}
}

#if !defined RTCW_SP
/*
==================
CM_ClearMap
==================
*/
void CM_ClearMap( void ) {
	Com_Memset( &cm, 0, sizeof( cm ) );
	CM_ClearLevelPatches();
}
#endif // RTCW_XX

/*
==================
CM_ClipHandleToModel
==================
*/
cmodel_t    *CM_ClipHandleToModel( clipHandle_t handle ) {
	if ( handle < 0 ) {
		Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle );
	}
	if ( handle < cm.numSubModels ) {
		return &cm.cmodels[handle];
	}
	if ( handle == BOX_MODEL_HANDLE || handle == CAPSULE_MODEL_HANDLE ) {
		return &box_model;
	}
	if ( handle < MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i < %i < %i",
				   cm.numSubModels, handle, MAX_SUBMODELS );
	}
	Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle + MAX_SUBMODELS );

	return NULL;

}

/*
==================
CM_InlineModel
==================
*/
clipHandle_t    CM_InlineModel( int index ) {
	if ( index < 0 || index >= cm.numSubModels ) {
		Com_Error( ERR_DROP, "CM_InlineModel: bad number" );
	}
	return index;
}

int     CM_NumClusters( void ) {
	return cm.numClusters;
}

int     CM_NumInlineModels( void ) {
	return cm.numSubModels;
}

char    *CM_EntityString( void ) {
	return cm.entityString;
}

int     CM_LeafCluster( int leafnum ) {
	if ( leafnum < 0 || leafnum >= cm.numLeafs ) {
		Com_Error( ERR_DROP, "CM_LeafCluster: bad number" );
	}
	return cm.leafs[leafnum].cluster;
}

int     CM_LeafArea( int leafnum ) {
	if ( leafnum < 0 || leafnum >= cm.numLeafs ) {
		Com_Error( ERR_DROP, "CM_LeafArea: bad number" );
	}
	return cm.leafs[leafnum].area;
}

//=======================================================================


/*
===================
CM_InitBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
void CM_InitBoxHull( void ) {
	int i;
	int side;
	cplane_t    *p;
	cbrushside_t    *s;

	box_planes = &cm.planes[cm.numPlanes];

	box_brush = &cm.brushes[cm.numBrushes];
	box_brush->numsides = 6;
	box_brush->sides = cm.brushsides + cm.numBrushSides;
	box_brush->contents = CONTENTS_BODY;

	box_model.leaf.numLeafBrushes = 1;
//	box_model.leaf.firstLeafBrush = cm.numBrushes;
	box_model.leaf.firstLeafBrush = cm.numLeafBrushes;
	cm.leafbrushes[cm.numLeafBrushes] = cm.numBrushes;

	for ( i = 0 ; i < 6 ; i++ )
	{
		side = i & 1;

		// brush sides
		s = &cm.brushsides[cm.numBrushSides + i];
		s->plane =  cm.planes + ( cm.numPlanes + i * 2 + side );
		s->surfaceFlags = 0;

		// planes
		p = &box_planes[i * 2];
		p->type = i >> 1;
		p->signbits = 0;
		VectorClear( p->normal );
		p->normal[i >> 1] = 1;

		p = &box_planes[i * 2 + 1];
		p->type = 3 + ( i >> 1 );
		p->signbits = 0;
		VectorClear( p->normal );
		p->normal[i >> 1] = -1;

		SetPlaneSignbits( p );
	}
}

/*
===================
CM_TempBoxModel

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
Capsules are handled differently though.
===================
*/
clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int capsule ) {

	VectorCopy( mins, box_model.mins );
	VectorCopy( maxs, box_model.maxs );

#if !defined RTCW_SP
	if ( capsule ) {
		return CAPSULE_MODEL_HANDLE;
	}
#endif // RTCW_XX

	box_planes[0].dist = maxs[0];
	box_planes[1].dist = -maxs[0];
	box_planes[2].dist = mins[0];
	box_planes[3].dist = -mins[0];
	box_planes[4].dist = maxs[1];
	box_planes[5].dist = -maxs[1];
	box_planes[6].dist = mins[1];
	box_planes[7].dist = -mins[1];
	box_planes[8].dist = maxs[2];
	box_planes[9].dist = -maxs[2];
	box_planes[10].dist = mins[2];
	box_planes[11].dist = -mins[2];

	VectorCopy( mins, box_brush->bounds[0] );
	VectorCopy( maxs, box_brush->bounds[1] );

#if defined RTCW_SP
	if ( capsule ) {
		return CAPSULE_MODEL_HANDLE;
	}
#endif // RTCW_XX

	return BOX_MODEL_HANDLE;
}

#if !defined RTCW_SP
// DHM - Nerve
void CM_SetTempBoxModelContents( int contents ) {

	box_brush->contents = contents;
}
// dhm
#endif // RTCW_XX

/*
===================
CM_ModelBounds
===================
*/
void CM_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	cmodel_t    *cmod;

	cmod = CM_ClipHandleToModel( model );
	VectorCopy( cmod->mins, mins );
	VectorCopy( cmod->maxs, maxs );
}


