/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// tr_map.c

#include "tr_local.h"
#include "rtcw_endian.h"

/*

Loads and prepares a map file for scene rendering.

A single entry point:

void RE_LoadWorldMap( const char *name );

*/

static world_t s_worldData;
static byte        *fileBase;

int c_subdivisions;
int c_gridVerts;

#if defined RTCW_ET
surfaceType_t skipData = SF_SKIP;
#endif // RTCW_XX

//===============================================================================

static void HSVtoRGB( float h, float s, float v, float rgb[3] ) {
	int i;
	float f;
	float p, q, t;

	h *= 5;

	i = c::floor( h );
	f = h - i;

	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );

	switch ( i )
	{
	case 0:
		rgb[0] = v;
		rgb[1] = t;
		rgb[2] = p;
		break;
	case 1:
		rgb[0] = q;
		rgb[1] = v;
		rgb[2] = p;
		break;
	case 2:
		rgb[0] = p;
		rgb[1] = v;
		rgb[2] = t;
		break;
	case 3:
		rgb[0] = p;
		rgb[1] = q;
		rgb[2] = v;
		break;
	case 4:
		rgb[0] = t;
		rgb[1] = p;
		rgb[2] = v;
		break;
	case 5:
		rgb[0] = v;
		rgb[1] = p;
		rgb[2] = q;
		break;
	}
}

/*
===============
R_ColorShiftLightingBytes

===============
*/
static void R_ColorShiftLightingBytes( byte in[4], byte out[4] ) {
	int shift, r, g, b;

	// shift the color data based on overbright range
#ifdef RTCW_VANILLA
	shift = r_mapOverBrightBits->integer - tr.overbrightBits;
#else // RTCW_VANILLA
	shift = std::max(r_mapOverBrightBits->integer - tr.overbrightBits, 0);
#endif // RTCW_VANILLA

	// shift the data based on overbright range
	r = in[0] << shift;
	g = in[1] << shift;
	b = in[2] << shift;

	// normalize by color instead of saturating to white
	if ( ( r | g | b ) > 255 ) {
		int max;

		max = r > g ? r : g;
		max = max > b ? max : b;
		r = r * 255 / max;
		g = g * 255 / max;
		b = b * 255 / max;
	}

	out[0] = r;
	out[1] = g;
	out[2] = b;
	out[3] = in[3];
}

#if defined RTCW_ET
/*
===============
R_ProcessLightmap

	returns maxIntensity
===============
*/
float R_ProcessLightmap( byte **pic, int in_padding, int width, int height, byte **pic_out ) {
	int j;
	float maxIntensity = 0;
	double sumIntensity = 0;

	if ( r_lightmap->integer > 1 ) { // color code by intensity as development tool	(FIXME: check range)
		for ( j = 0; j < width * height; j++ )
		{
			float r = ( *pic )[j * in_padding + 0];
			float g = ( *pic )[j * in_padding + 1];
			float b = ( *pic )[j * in_padding + 2];
			float intensity;
			float out[3];

			intensity = 0.33f * r + 0.685f * g + 0.063f * b;

			if ( intensity > 255 ) {
				intensity = 1.0f;
			} else {
				intensity /= 255.0f;
			}

			if ( intensity > maxIntensity ) {
				maxIntensity = intensity;
			}

			HSVtoRGB( intensity, 1.00, 0.50, out );

			if ( r_lightmap->integer == 3 ) {
				// Arnout: artists wanted the colours to be inversed
				( *pic_out )[j * 4 + 0] = out[2] * 255;
				( *pic_out )[j * 4 + 1] = out[1] * 255;
				( *pic_out )[j * 4 + 2] = out[0] * 255;
			} else {
				( *pic_out )[j * 4 + 0] = out[0] * 255;
				( *pic_out )[j * 4 + 1] = out[1] * 255;
				( *pic_out )[j * 4 + 2] = out[2] * 255;
			}
			( *pic_out )[j * 4 + 3] = 255;

			sumIntensity += intensity;
		}
	} else {
		for ( j = 0 ; j < width * height; j++ ) {
			R_ColorShiftLightingBytes( &( *pic )[j * in_padding], &( *pic_out )[j * 4] );
			( *pic_out )[j * 4 + 3] = 255;
		}
	}

	return maxIntensity;
}
#endif // RTCW_XX

/*
===============
R_LoadLightmaps

===============
*/
#define LIGHTMAP_SIZE   128
static void R_LoadLightmaps( lump_t *l ) {

#if !defined RTCW_ET
	byte        *buf, *buf_p;
#else
	byte        *buf, *buf_p, *image_p;
#endif // RTCW_XX

	int len;
	byte image[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 4];

#if !defined RTCW_ET
	int i, j;
	float maxIntensity = 0;
	double sumIntensity = 0;
#else
	int i /*, j*/;
	float intensity, maxIntensity = 0;
//	double sumIntensity = 0;
#endif // RTCW_XX

#if defined RTCW_ET
	// ydnar: clear lightmaps first
	tr.numLightmaps = 0;
	memset( tr.lightmaps, 0, sizeof( *tr.lightmaps ) * MAX_LIGHTMAPS );
#endif // RTCW_XX

	len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	// we are about to upload textures
	R_SyncRenderThread();

	// create all the lightmaps
	tr.numLightmaps = len / ( LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3 );
	if ( tr.numLightmaps == 1 ) {
		//FIXME: HACK: maps with only one lightmap turn up fullbright for some reason.
		//this avoids this, but isn't the correct solution.
		tr.numLightmaps++;
	}

// BBi
//#if !defined RTCW_ET
//	// if we are in r_vertexLight mode, we don't need the lightmaps at all
//	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
//#else
//	// permedia doesn't support lightmaps
//	if ( glConfig.hardwareType == GLHW_PERMEDIA2 ) {
//#endif // RTCW_XX

#if !defined RTCW_ET
	if (r_vertexLight->integer != 0) {
#else
	if (false) {
#endif // RTCW_XX
// BBi

		return;
	}

	for ( i = 0 ; i < tr.numLightmaps ; i++ ) {
		// expand the 24 bit on-disk to 32 bit
		buf_p = buf + i * LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3;

#if !defined RTCW_ET
		if ( r_lightmap->integer == 2 ) { // color code by intensity as development tool	(FIXME: check range)
			for ( j = 0; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ )
			{
				float r = buf_p[j * 3 + 0];
				float g = buf_p[j * 3 + 1];
				float b = buf_p[j * 3 + 2];
				float intensity;
				float out[3];

				intensity = 0.33f * r + 0.685f * g + 0.063f * b;

				if ( intensity > 255 ) {
					intensity = 1.0f;
				} else {
					intensity /= 255.0f;
				}

				if ( intensity > maxIntensity ) {
					maxIntensity = intensity;
				}

				HSVtoRGB( intensity, 1.00, 0.50, out );

				image[j * 4 + 0] = out[0] * 255;
				image[j * 4 + 1] = out[1] * 255;
				image[j * 4 + 2] = out[2] * 255;
				image[j * 4 + 3] = 255;

				sumIntensity += intensity;
			}
		} else {
			for ( j = 0 ; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ ) {
				R_ColorShiftLightingBytes( &buf_p[j * 3], &image[j * 4] );
				image[j * 4 + 3] = 255;
			}
		}
#else
		image_p = image;

		intensity = R_ProcessLightmap( &buf_p, 3, LIGHTMAP_SIZE, LIGHTMAP_SIZE, &image_p );
		if ( intensity > maxIntensity ) {
			maxIntensity = intensity;
		}
#endif // RTCW_XX

		// BBi
		//tr.lightmaps[i] = R_CreateImage( va( "*lightmap%d",i ), image,
		//								 LIGHTMAP_SIZE, LIGHTMAP_SIZE, qfalse, qfalse, GL_CLAMP );
		tr.lightmaps[i] = R_CreateImage (va ("*lightmap%d", i), image,
			LIGHTMAP_SIZE, LIGHTMAP_SIZE, false, false, r_get_best_wrap_clamp ());
		// BBi
	}

#if !defined RTCW_ET
	if ( r_lightmap->integer == 2 ) {
#else
	if ( r_lightmap->integer > 1 ) {
#endif // RTCW_XX

		ri.Printf( PRINT_ALL, "Brightest lightmap value: %d\n", ( int ) ( maxIntensity * 255 ) );
	}
}


/*
=================
RE_SetWorldVisData

This is called by the clipmodel subsystem so we can share the 1.8 megs of
space in big maps...
=================
*/
void        RE_SetWorldVisData( const byte *vis ) {
	tr.externalVisData = vis;
}


/*
=================
R_LoadVisibility
=================
*/
static void R_LoadVisibility( lump_t *l ) {
	int len;
	byte    *buf;

	len = ( s_worldData.numClusters + 63 ) & ~63;
	s_worldData.novis = static_cast<byte*> (ri.Hunk_Alloc( len, h_low ));
	memset( s_worldData.novis, 0xff, len );

	len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	s_worldData.numClusters = rtcw::Endian::le( ( (int *)buf )[0] );
	s_worldData.clusterBytes = rtcw::Endian::le( ( (int *)buf )[1] );

	// CM_Load should have given us the vis data to share, so
	// we don't need to allocate another copy
	if ( tr.externalVisData ) {
		s_worldData.vis = tr.externalVisData;
	} else {
		byte    *dest;

		dest = static_cast<byte*> (ri.Hunk_Alloc( len - 8, h_low ));
		memcpy( dest, buf + 8, len - 8 );
		s_worldData.vis = dest;
	}
}

//===============================================================================


/*
===============
ShaderForShaderNum
===============
*/
static shader_t *ShaderForShaderNum( int shaderNum, int lightmapNum ) {
	shader_t    *shader;
	dshader_t   *dsh;

	rtcw::Endian::lei(shaderNum);
	if ( shaderNum < 0 || shaderNum >= s_worldData.numShaders ) {
		ri.Error( ERR_DROP, "ShaderForShaderNum: bad num %i", shaderNum );
	}
	dsh = &s_worldData.shaders[ shaderNum ];

// BBi
//#if !defined RTCW_ET
//	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
//#else
//	if ( glConfig.hardwareType == GLHW_PERMEDIA2 ) {
//#endif // RTCW_XX

#if !defined RTCW_ET
	if (r_vertexLight->integer != 0) {
#else
	if (false) {
#endif // RTCW_XX
// BBi

		lightmapNum = LIGHTMAP_BY_VERTEX;
	}

#if defined RTCW_SP
	if ( r_fullbright->integer ) {
		lightmapNum = LIGHTMAP_WHITEIMAGE;
	}
#elif defined RTCW_MP
// JPW NERVE removed per atvi request
/*
	if ( r_fullbright->integer ) {
		lightmapNum = LIGHTMAP_WHITEIMAGE;
	}
*/
#endif // RTCW_XX

	shader = R_FindShader( dsh->shader, lightmapNum, qtrue );

	// if the shader had errors, just use default shader
	if ( shader->defaultShader ) {
		return tr.defaultShader;
	}

	return shader;
}

// Ridah, optimizations here
// memory block for use by surfaces
static byte *surfHunkPtr;
static int surfHunkSize;
#define SURF_HUNK_MAXSIZE 0x40000
#define LL( x ) rtcw::Endian::le( x )

/*
==============
R_InitSurfMemory
==============
*/
void R_InitSurfMemory( void ) {
	// allocate a new chunk
	surfHunkPtr = static_cast<byte*> (ri.Hunk_Alloc( SURF_HUNK_MAXSIZE, h_low ));
	surfHunkSize = 0;
}

/*
==============
R_GetSurfMemory
==============
*/
void *R_GetSurfMemory( int size ) {
	byte *retval;

	// round to cacheline
	size = ( size + 31 ) & ~31;

	surfHunkSize += size;
	if ( surfHunkSize >= SURF_HUNK_MAXSIZE ) {
		// allocate a new chunk
		R_InitSurfMemory();
		surfHunkSize += size;   // since it just got reset
	}
	retval = surfHunkPtr;
	surfHunkPtr += size;

	return (void *)retval;
}

#if !defined RTCW_ET
/*
===============
ParseFace
===============
*/
static void ParseFace( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes  ) {
	int i, j;
	srfSurfaceFace_t    *cv;
	int numPoints, numIndexes;
	int lightmapNum;
	int sfaceSize, ofsIndexes;

	lightmapNum = rtcw::Endian::le( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = rtcw::Endian::le( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numPoints = rtcw::Endian::le( ds->numVerts );
	if ( numPoints > MAX_FACE_POINTS ) {
		ri.Printf( PRINT_WARNING, "WARNING: MAX_FACE_POINTS exceeded: %i\n", numPoints );
		numPoints = MAX_FACE_POINTS;
		surf->shader = tr.defaultShader;
	}

	numIndexes = rtcw::Endian::le( ds->numIndexes );

	// create the srfSurfaceFace_t
#if FIXME
	sfaceSize = ( int ) &( (srfSurfaceFace_t *)0 )->points[numPoints];
#else
	sfaceSize = static_cast<int>(reinterpret_cast<intptr_t>(&(static_cast<srfSurfaceFace_t*>(NULL))->points[numPoints]));
#endif // FIXME

	ofsIndexes = sfaceSize;
	sfaceSize += sizeof( int ) * numIndexes;

	//cv = ri.Hunk_Alloc( sfaceSize );
	cv = static_cast<srfSurfaceFace_t*> (R_GetSurfMemory( sfaceSize ));

	cv->surfaceType = SF_FACE;
	cv->numPoints = numPoints;
	cv->numIndices = numIndexes;
	cv->ofsIndices = ofsIndexes;

	verts += rtcw::Endian::le( ds->firstVert );
	for ( i = 0 ; i < numPoints ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			cv->points[i][j] = rtcw::Endian::le( verts[i].xyz[j] );
		}
		for ( j = 0 ; j < 2 ; j++ ) {
			cv->points[i][3 + j] = rtcw::Endian::le( verts[i].st[j] );
			cv->points[i][5 + j] = rtcw::Endian::le( verts[i].lightmap[j] );
		}
		R_ColorShiftLightingBytes( verts[i].color, (byte *)&cv->points[i][7] );
	}

	indexes += rtcw::Endian::le( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		( ( int * )( (byte *)cv + cv->ofsIndices ) )[i] = rtcw::Endian::le( indexes[ i ] );
	}

	// take the plane information from the lightmap vector
	for ( i = 0 ; i < 3 ; i++ ) {
		cv->plane.normal[i] = rtcw::Endian::le( ds->lightmapVecs[2][i] );
	}
	cv->plane.dist = DotProduct( cv->points[0], cv->plane.normal );
	SetPlaneSignbits( &cv->plane );
	cv->plane.type = PlaneTypeForNormal( cv->plane.normal );

	surf->data = (surfaceType_t *)cv;
}
#endif // RTCW_XX

#if defined RTCW_ET
/*
SphereFromBounds() - ydnar
creates a bounding sphere from a bounding box
*/

static void SphereFromBounds( vec3_t mins, vec3_t maxs, vec3_t origin, float *radius ) {
	vec3_t temp;

	VectorAdd( mins, maxs, origin );
	VectorScale( origin, 0.5, origin );
	VectorSubtract( maxs, origin, temp );
	*radius = VectorLength( temp );
}



/*
FinishGenericSurface() - ydnar
handles final surface classification
*/

static void FinishGenericSurface( dsurface_t *ds, srfGeneric_t *gen, vec3_t pt ) {
	// set bounding sphere
	SphereFromBounds( gen->bounds[ 0 ], gen->bounds[ 1 ], gen->origin, &gen->radius );

	// take the plane normal from the lightmap vector and classify it
	gen->plane.normal[ 0 ] = rtcw::Endian::le( ds->lightmapVecs[ 2 ][ 0 ] );
	gen->plane.normal[ 1 ] = rtcw::Endian::le( ds->lightmapVecs[ 2 ][ 1 ] );
	gen->plane.normal[ 2 ] = rtcw::Endian::le( ds->lightmapVecs[ 2 ][ 2 ] );
	gen->plane.dist = DotProduct( pt, gen->plane.normal );
	SetPlaneSignbits( &gen->plane );
	gen->plane.type = PlaneTypeForNormal( gen->plane.normal );
}
#endif // RTCW_XX

/*
===============
ParseMesh
===============
*/
static void ParseMesh( dsurface_t *ds, drawVert_t *verts, msurface_t *surf ) {
	srfGridMesh_t   *grid;
	int i, j;
	int width, height, numPoints;
	drawVert_t points[MAX_PATCH_SIZE * MAX_PATCH_SIZE];
	int lightmapNum;
	vec3_t bounds[2];
	vec3_t tmpVec;

#if !defined RTCW_ET
	static surfaceType_t skipData = SF_SKIP;
#endif // RTCW_XX

	lightmapNum = rtcw::Endian::le( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = rtcw::Endian::le( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	// we may have a nodraw surface, because they might still need to
	// be around for movement clipping
	if ( s_worldData.shaders[ rtcw::Endian::le( ds->shaderNum ) ].surfaceFlags & SURF_NODRAW ) {
		surf->data = &skipData;
		return;
	}

	width = rtcw::Endian::le( ds->patchWidth );
	height = rtcw::Endian::le( ds->patchHeight );

	verts += rtcw::Endian::le( ds->firstVert );
	numPoints = width * height;
	for ( i = 0 ; i < numPoints ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			points[i].xyz[j] = rtcw::Endian::le( verts[i].xyz[j] );
			points[i].normal[j] = rtcw::Endian::le( verts[i].normal[j] );
		}
		for ( j = 0 ; j < 2 ; j++ ) {
			points[i].st[j] = rtcw::Endian::le( verts[i].st[j] );
			points[i].lightmap[j] = rtcw::Endian::le( verts[i].lightmap[j] );
		}
		R_ColorShiftLightingBytes( verts[i].color, points[i].color );
	}

	// pre-tesseleate
	grid = R_SubdividePatchToGrid( width, height, points );
	surf->data = (surfaceType_t *)grid;

	// copy the level of detail origin, which is the center
	// of the group of all curves that must subdivide the same
	// to avoid cracking
	for ( i = 0 ; i < 3 ; i++ ) {
		bounds[0][i] = rtcw::Endian::le( ds->lightmapVecs[0][i] );
		bounds[1][i] = rtcw::Endian::le( ds->lightmapVecs[1][i] );
	}
	VectorAdd( bounds[0], bounds[1], bounds[1] );
	VectorScale( bounds[1], 0.5f, grid->lodOrigin );
	VectorSubtract( bounds[0], grid->lodOrigin, tmpVec );
	grid->lodRadius = VectorLength( tmpVec );

#if defined RTCW_ET
	// finish surface
	FinishGenericSurface( ds, (srfGeneric_t*) grid, grid->verts[ 0 ].xyz );
#endif // RTCW_XX

}

#if defined RTCW_ET
/*
===============
ParseFace
===============
*/
#if 0 // rain - unused
static void ParseFace( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes  ) {
	int i, j;
	srfSurfaceFace_t    *cv;
	int numPoints, numIndexes;
	int lightmapNum;
	int sfaceSize, ofsIndexes;


	lightmapNum = LittleLong( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numPoints = LittleLong( ds->numVerts );
	if ( numPoints > MAX_FACE_POINTS ) {
		ri.Printf( PRINT_WARNING, "WARNING: MAX_FACE_POINTS exceeded: %i\n", numPoints );
		numPoints = MAX_FACE_POINTS;
		surf->shader = tr.defaultShader;
	}

	numIndexes = LittleLong( ds->numIndexes );

	// create the srfSurfaceFace_t
	sfaceSize = ( int ) &( (srfSurfaceFace_t *)0 )->points[numPoints];
	ofsIndexes = sfaceSize;
	sfaceSize += sizeof( int ) * numIndexes;

	//cv = ri.Hunk_Alloc( sfaceSize );
	cv = R_GetSurfMemory( sfaceSize );

	cv->surfaceType = SF_FACE;
	cv->numPoints = numPoints;
	cv->numIndices = numIndexes;
	cv->ofsIndices = ofsIndexes;

	ClearBounds( cv->bounds[0], cv->bounds[1] );
	verts += LittleLong( ds->firstVert );
	for ( i = 0 ; i < numPoints ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			cv->points[i][j] = LittleFloat( verts[i].xyz[j] );
		}
		AddPointToBounds( cv->points[ i ], cv->bounds[ 0 ], cv->bounds[ 1 ] );
		for ( j = 0 ; j < 2 ; j++ ) {
			cv->points[i][3 + j] = LittleFloat( verts[i].st[j] );
			cv->points[i][5 + j] = LittleFloat( verts[i].lightmap[j] );
		}
		R_ColorShiftLightingBytes( verts[i].color, (byte *)&cv->points[i][7] );
	}

	indexes += LittleLong( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		( ( int * )( (byte *)cv + cv->ofsIndices ) )[i] = LittleLong( indexes[ i ] );
	}

	#if 0
	// take the plane information from the lightmap vector
	for ( i = 0 ; i < 3 ; i++ ) {
		cv->plane.normal[i] = LittleFloat( ds->lightmapVecs[2][i] );
	}
	cv->plane.dist = DotProduct( cv->points[0], cv->plane.normal );
	SetPlaneSignbits( &cv->plane );
	cv->plane.type = PlaneTypeForNormal( cv->plane.normal );
	#endif

	surf->data = (surfaceType_t *) cv;

	// finish surface
	FinishGenericSurface( ds, (srfGeneric_t*) cv, cv->points[ 0 ] );
}
#endif
#endif // RTCW_XX


/*
===============
ParseTriSurf
===============
*/

#if defined RTCW_ET
#if 0
static void ParseTriSurf( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfTriangles2_t     *tri;
	int i, j;
	int numVerts, numIndexes;
	int lightmapNum;


	// get lightmap num
	lightmapNum = LittleLong( ds->lightmapNum );

	// get fog volume
	surf->fogIndex = LittleLong( ds->fogNum ) + 1;

	// get shader
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );    //%	LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numVerts = LittleLong( ds->numVerts );
	numIndexes = LittleLong( ds->numIndexes );


	tri = R_GetSurfMemory( sizeof( *tri ) + ( numVerts + 1 ) * ( sizeof( vec4hack_t ) + sizeof( vec2hack_t ) + sizeof( vec2hack_t ) + sizeof( vec4hack_t ) + sizeof( color4ubhack_t ) ) + numIndexes * sizeof( tri->indexes[0] ) );

	tri->surfaceType = SF_TRIANGLES;
	tri->numVerts = numVerts;
	tri->numIndexes = numIndexes;

	tri->xyz =      ( vec4hack_t* )( tri +              1 );
	tri->st =       ( vec2hack_t* )( tri->xyz +         tri->numVerts + 1 );
	tri->lightmap = ( vec2hack_t* )( tri->st +          tri->numVerts + 1 );
	tri->normal =   ( vec4hack_t* )( tri->lightmap +    tri->numVerts + 1 );
	tri->color =    ( color4ubhack_t* )( tri->normal +      tri->numVerts + 1 );
	tri->indexes =  ( int* )( tri->color +       tri->numVerts + 1 );

	surf->data = (surfaceType_t *)tri;

	// copy vertexes
	ClearBounds( tri->bounds[0], tri->bounds[1] );
	verts += LittleLong( ds->firstVert );


	if ( LittleLong( 256 ) != 256 ) {
		for ( i = 0 ; i < numVerts ; i++ ) {
			for ( j = 0 ; j < 3 ; j++ ) {
				tri->xyz[i].v[j] =      LittleFloat( verts[i].xyz[j] );
				tri->normal[i].v[j] =   LittleFloat( verts[i].normal[j] );
			}
			AddPointToBounds( tri->xyz[i].v, tri->bounds[0], tri->bounds[1] );
			for ( j = 0 ; j < 2 ; j++ ) {
				tri->st[i].v[j] =       LittleFloat( verts[i].st[j] );
				tri->lightmap[i].v[j] = LittleFloat( verts[i].lightmap[j] );
			}

			R_ColorShiftLightingBytes( verts[i].color, tri->color[i].v );
		}
	} else { // Gordon: OPT: removed the littlefloats from when they aint needed
		for ( i = 0 ; i < numVerts ; i++ ) {
			for ( j = 0 ; j < 3 ; j++ ) {
				tri->xyz[i].v[j] =      verts[i].xyz[j];
				tri->normal[i].v[j] =   verts[i].normal[j];
			}
			AddPointToBounds( tri->xyz[i].v, tri->bounds[0], tri->bounds[1] );
			for ( j = 0 ; j < 2 ; j++ ) {
				tri->st[i].v[j] =       verts[i].st[j];
				tri->lightmap[i].v[j] = verts[i].lightmap[j];
			}

			R_ColorShiftLightingBytes( verts[i].color, tri->color[i].v );
		}
	}

	// copy indexes
	indexes += LittleLong( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		tri->indexes[i] = LittleLong( indexes[i] );
		if ( tri->indexes[i] < 0 || tri->indexes[i] >= numVerts ) {
			ri.Error( ERR_DROP, "Bad index in triangle surface" );
		}
	}

	// finish surface
	FinishGenericSurface( ds, (srfGeneric_t*) tri, tri->xyz[0].v );
}
#endif
#endif // RTCW_XX

static void ParseTriSurf( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfTriangles_t  *tri;
	int i, j;
	int numVerts, numIndexes;

#if defined RTCW_ET
	int lightmapNum;


	// get lightmap num
	lightmapNum = rtcw::Endian::le( ds->lightmapNum );
#endif // RTCW_XX

	// get fog volume
	surf->fogIndex = rtcw::Endian::le( ds->fogNum ) + 1;

	// get shader

#if !defined RTCW_ET
	surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
#else
	surf->shader = ShaderForShaderNum( ds->shaderNum, lightmapNum );    //%	LIGHTMAP_BY_VERTEX );
#endif // RTCW_XX

	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numVerts = rtcw::Endian::le( ds->numVerts );
	numIndexes = rtcw::Endian::le( ds->numIndexes );

	//tri = ri.Hunk_Alloc( sizeof( *tri ) + numVerts * sizeof( tri->verts[0] )
	//	+ numIndexes * sizeof( tri->indexes[0] ) );
	tri = static_cast<srfTriangles_t*> (R_GetSurfMemory( sizeof( *tri ) + numVerts * sizeof( tri->verts[0] )
						   + numIndexes * sizeof( tri->indexes[0] ) ));

	tri->surfaceType = SF_TRIANGLES;
	tri->numVerts = numVerts;
	tri->numIndexes = numIndexes;
	tri->verts = ( drawVert_t * )( tri + 1 );
	tri->indexes = ( int * )( tri->verts + tri->numVerts );

	surf->data = (surfaceType_t *)tri;

	// copy vertexes
	ClearBounds( tri->bounds[0], tri->bounds[1] );
	verts += rtcw::Endian::le( ds->firstVert );
	for ( i = 0 ; i < numVerts ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			tri->verts[i].xyz[j] = rtcw::Endian::le( verts[i].xyz[j] );
			tri->verts[i].normal[j] = rtcw::Endian::le( verts[i].normal[j] );
		}
		AddPointToBounds( tri->verts[i].xyz, tri->bounds[0], tri->bounds[1] );
		for ( j = 0 ; j < 2 ; j++ ) {
			tri->verts[i].st[j] = rtcw::Endian::le( verts[i].st[j] );
			tri->verts[i].lightmap[j] = rtcw::Endian::le( verts[i].lightmap[j] );
		}

		R_ColorShiftLightingBytes( verts[i].color, tri->verts[i].color );
	}

	// copy indexes
	indexes += rtcw::Endian::le( ds->firstIndex );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		tri->indexes[i] = rtcw::Endian::le( indexes[i] );
		if ( tri->indexes[i] < 0 || tri->indexes[i] >= numVerts ) {
			ri.Error( ERR_DROP, "Bad index in triangle surface" );
		}
	}

#if defined RTCW_ET
	// finish surface
	FinishGenericSurface( ds, (srfGeneric_t*) tri, tri->verts[ 0 ].xyz );
#endif // RTCW_XX

}

#if defined RTCW_ET
/*
ParseFoliage() - ydnar
parses a foliage drawsurface
*/

static void ParseFoliage( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfFoliage_t    *foliage;
	int i, j, numVerts, numIndexes, numInstances, size;
	vec4_t          *xyz, *normal /*, *origin*/;
//	fcolor4ub_t		*color;
	vec3_t bounds[ 2 ], boundsTranslated[ 2 ];
	float scale;


	// get fog volume
	surf->fogIndex = rtcw::Endian::le( ds->fogNum ) + 1;

	// get shader
	surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	// foliage surfaces have their actual vert count in patchHeight
	// and the instance count in patchWidth
	// the instances are just additional drawverts

	// get counts
	numVerts = rtcw::Endian::le( ds->patchHeight );
	numIndexes = rtcw::Endian::le( ds->numIndexes );
	numInstances = rtcw::Endian::le( ds->patchWidth );

	// calculate size
	size = sizeof( *foliage ) +
		   numVerts * ( sizeof( foliage->xyz[ 0 ] ) + sizeof( foliage->normal[ 0 ] ) + sizeof( foliage->texCoords[ 0 ] ) + sizeof( foliage->lmTexCoords[ 0 ] ) ) +
		   numIndexes * sizeof( foliage->indexes[ 0 ] ) +
		   numInstances * sizeof( foliage->instances[ 0 ] );

	// get memory
	foliage = static_cast<srfFoliage_t*> (R_GetSurfMemory( size ));

	// set up surface
	foliage->surfaceType = SF_FOLIAGE;
	foliage->numVerts = numVerts;
	foliage->numIndexes = numIndexes;
	foliage->numInstances = numInstances;
	foliage->xyz = ( vec4_t* )( foliage + 1 );
	foliage->normal = ( vec4_t* )( foliage->xyz + foliage->numVerts );
	foliage->texCoords = ( vec2_t* )( foliage->normal + foliage->numVerts );
	foliage->lmTexCoords = ( vec2_t* )( foliage->texCoords + foliage->numVerts );
	foliage->indexes = reinterpret_cast<glIndex_t*> ( foliage->lmTexCoords + foliage->numVerts );
	foliage->instances = ( foliageInstance_t* )( foliage->indexes + foliage->numIndexes );

	surf->data = (surfaceType_t*) foliage;

	// get foliage drawscale
	scale = r_drawfoliage->value;
	if ( scale < 0.0f ) {
		scale = 1.0f;
	} else if ( scale > 2.0f ) {
		scale = 2.0f;
	}

	// copy vertexes
	ClearBounds( bounds[ 0 ], bounds[ 1 ] );
	verts += rtcw::Endian::le( ds->firstVert );
	xyz = foliage->xyz;
	normal = foliage->normal;
	for ( i = 0; i < numVerts; i++ )
	{
		// copy xyz and normal
		for ( j = 0; j < 3; j++ )
		{
			foliage->xyz[ i ][ j ] = rtcw::Endian::le( verts[ i ].xyz[ j ] );
			foliage->normal[ i ][ j ] = rtcw::Endian::le( verts[ i ].normal[ j ] );
		}

		// scale height
		foliage->xyz[ i ][ 2 ] *= scale;

		// finish
		foliage->xyz[ i ][ 3 ] = foliage->normal[ i ][ 3 ] = 0;
		AddPointToBounds( foliage->xyz[ i ], bounds[ 0 ], bounds[ 1 ] );

		// copy texture coordinates
		for ( j = 0; j < 2; j++ )
		{
			foliage->texCoords[ i ][ j ] = rtcw::Endian::le( verts[ i ].st[ j ] );
			foliage->lmTexCoords[ i ][ j ] = rtcw::Endian::le( verts[ i ].lightmap[ j ] );
		}
	}

	// copy indexes
	indexes += rtcw::Endian::le( ds->firstIndex );
	for ( i = 0; i < numIndexes; i++ )
	{
		foliage->indexes[ i ] = rtcw::Endian::le( indexes[ i ] );
		if ( foliage->indexes[ i ] < 0 || foliage->indexes[ i ] >= numVerts ) {
			ri.Error( ERR_DROP, "Bad index in triangle surface" );
		}
	}

	// copy origins and colors
	ClearBounds( foliage->bounds[ 0 ], foliage->bounds[ 1 ] );
	verts += numVerts;
	for ( i = 0; i < numInstances; i++ )
	{
		// copy xyz
		for ( j = 0; j < 3; j++ )
			foliage->instances[ i ].origin[ j ] = rtcw::Endian::le( verts[ i ].xyz[ j ] );
		VectorAdd( bounds[ 0 ], foliage->instances[ i ].origin, boundsTranslated[ 0 ] );
		VectorAdd( bounds[ 1 ], foliage->instances[ i ].origin, boundsTranslated[ 1 ] );
		AddPointToBounds( boundsTranslated[ 0 ], foliage->bounds[ 0 ], foliage->bounds[ 1 ] );
		AddPointToBounds( boundsTranslated[ 1 ], foliage->bounds[ 0 ], foliage->bounds[ 1 ] );

		// copy color
		R_ColorShiftLightingBytes( verts[ i ].color, foliage->instances[ i ].color );
	}

	// finish surface
	FinishGenericSurface( ds, (srfGeneric_t*) foliage, foliage->xyz[ 0 ] );
}
#endif // RTCW_XX


/*
===============
ParseFlare
===============
*/
static void ParseFlare( dsurface_t *ds, drawVert_t *verts, msurface_t *surf, int *indexes ) {
	srfFlare_t      *flare;
	int i;

	// get fog volume
	surf->fogIndex = rtcw::Endian::le( ds->fogNum ) + 1;

	// get shader
	surf->shader = ShaderForShaderNum( ds->shaderNum, LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	flare = static_cast<srfFlare_t*> (ri.Hunk_Alloc( sizeof( *flare ), h_low ));
	flare->surfaceType = SF_FLARE;

	surf->data = (surfaceType_t *)flare;

	for ( i = 0 ; i < 3 ; i++ ) {
		flare->origin[i] = rtcw::Endian::le( ds->lightmapOrigin[i] );
		flare->color[i] = rtcw::Endian::le( ds->lightmapVecs[0][i] );
		flare->normal[i] = rtcw::Endian::le( ds->lightmapVecs[2][i] );
	}
}


/*
=================
R_MergedWidthPoints

returns true if there are grid points merged on a width edge
=================
*/
int R_MergedWidthPoints( srfGridMesh_t *grid, int offset ) {
	int i, j;

	for ( i = 1; i < grid->width - 1; i++ ) {
		for ( j = i + 1; j < grid->width - 1; j++ ) {

#if !defined RTCW_MP
			if ( Q_fabs( grid->verts[i + offset].xyz[0] - grid->verts[j + offset].xyz[0] ) > .1 ) {
				continue;
			}
			if ( Q_fabs( grid->verts[i + offset].xyz[1] - grid->verts[j + offset].xyz[1] ) > .1 ) {
				continue;
			}
			if ( Q_fabs( grid->verts[i + offset].xyz[2] - grid->verts[j + offset].xyz[2] ) > .1 ) {
#else
			if ( c::fabs( grid->verts[i + offset].xyz[0] - grid->verts[j + offset].xyz[0] ) > .1 ) {
				continue;
			}
			if ( c::fabs( grid->verts[i + offset].xyz[1] - grid->verts[j + offset].xyz[1] ) > .1 ) {
				continue;
			}
			if ( c::fabs( grid->verts[i + offset].xyz[2] - grid->verts[j + offset].xyz[2] ) > .1 ) {
#endif // RTCW_XX

				continue;
			}
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_MergedHeightPoints

returns true if there are grid points merged on a height edge
=================
*/
int R_MergedHeightPoints( srfGridMesh_t *grid, int offset ) {
	int i, j;

	for ( i = 1; i < grid->height - 1; i++ ) {
		for ( j = i + 1; j < grid->height - 1; j++ ) {

#if !defined RTCW_MP
			if ( Q_fabs( grid->verts[grid->width * i + offset].xyz[0] - grid->verts[grid->width * j + offset].xyz[0] ) > .1 ) {
				continue;
			}
			if ( Q_fabs( grid->verts[grid->width * i + offset].xyz[1] - grid->verts[grid->width * j + offset].xyz[1] ) > .1 ) {
				continue;
			}
			if ( Q_fabs( grid->verts[grid->width * i + offset].xyz[2] - grid->verts[grid->width * j + offset].xyz[2] ) > .1 ) {
#else
			if ( c::fabs( grid->verts[grid->width * i + offset].xyz[0] - grid->verts[grid->width * j + offset].xyz[0] ) > .1 ) {
				continue;
			}
			if ( c::fabs( grid->verts[grid->width * i + offset].xyz[1] - grid->verts[grid->width * j + offset].xyz[1] ) > .1 ) {
				continue;
			}
			if ( c::fabs( grid->verts[grid->width * i + offset].xyz[2] - grid->verts[grid->width * j + offset].xyz[2] ) > .1 ) {
#endif // RTCW_XX

				continue;
			}
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_FixSharedVertexLodError_r

NOTE: never sync LoD through grid edges with merged points!

FIXME: write generalized version that also avoids cracks between a patch and one that meets half way?
=================
*/
void R_FixSharedVertexLodError_r( int start, srfGridMesh_t *grid1 ) {
	int j, k, l, m, n, offset1, offset2, touch;
	srfGridMesh_t *grid2;

	for ( j = start; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if ( grid2->surfaceType != SF_GRID ) {
			continue;
		}
		// if the LOD errors are already fixed for this patch
		if ( grid2->lodFixed == 2 ) {
			continue;
		}
		// grids in the same LOD group should have the exact same lod radius
		if ( grid1->lodRadius != grid2->lodRadius ) {
			continue;
		}
		// grids in the same LOD group should have the exact same lod origin
		if ( grid1->lodOrigin[0] != grid2->lodOrigin[0] ) {
			continue;
		}
		if ( grid1->lodOrigin[1] != grid2->lodOrigin[1] ) {
			continue;
		}
		if ( grid1->lodOrigin[2] != grid2->lodOrigin[2] ) {
			continue;
		}
		//
		touch = qfalse;
		for ( n = 0; n < 2; n++ ) {
			//
			if ( n ) {
				offset1 = ( grid1->height - 1 ) * grid1->width;
			} else { offset1 = 0;}
			if ( R_MergedWidthPoints( grid1, offset1 ) ) {
				continue;
			}
			for ( k = 1; k < grid1->width - 1; k++ ) {
				for ( m = 0; m < 2; m++ ) {

					if ( m ) {
						offset2 = ( grid2->height - 1 ) * grid2->width;
					} else { offset2 = 0;}
					if ( R_MergedWidthPoints( grid2, offset2 ) ) {
						continue;
					}
					for ( l = 1; l < grid2->width - 1; l++ ) {
						//

#if !defined RTCW_MP
						if ( Q_fabs( grid1->verts[k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0] ) > .1 ) {
							continue;
						}
						if ( Q_fabs( grid1->verts[k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1] ) > .1 ) {
							continue;
						}
						if ( Q_fabs( grid1->verts[k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2] ) > .1 ) {
#else
						if ( c::fabs( grid1->verts[k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0] ) > .1 ) {
							continue;
						}
						if ( c::fabs( grid1->verts[k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1] ) > .1 ) {
							continue;
						}
						if ( c::fabs( grid1->verts[k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2] ) > .1 ) {
#endif // RTCW_XX

							continue;
						}
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
				for ( m = 0; m < 2; m++ ) {

					if ( m ) {
						offset2 = grid2->width - 1;
					} else { offset2 = 0;}
					if ( R_MergedHeightPoints( grid2, offset2 ) ) {
						continue;
					}
					for ( l = 1; l < grid2->height - 1; l++ ) {
						//

#if !defined RTCW_MP
						if ( Q_fabs( grid1->verts[k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0] ) > .1 ) {
							continue;
						}
						if ( Q_fabs( grid1->verts[k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1] ) > .1 ) {
							continue;
						}
						if ( Q_fabs( grid1->verts[k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2] ) > .1 ) {
#else
						if ( c::fabs( grid1->verts[k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0] ) > .1 ) {
							continue;
						}
						if ( c::fabs( grid1->verts[k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1] ) > .1 ) {
							continue;
						}
						if ( c::fabs( grid1->verts[k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2] ) > .1 ) {
#endif // RTCW_XX

							continue;
						}
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		for ( n = 0; n < 2; n++ ) {
			//
			if ( n ) {
				offset1 = grid1->width - 1;
			} else { offset1 = 0;}
			if ( R_MergedHeightPoints( grid1, offset1 ) ) {
				continue;
			}
			for ( k = 1; k < grid1->height - 1; k++ ) {
				for ( m = 0; m < 2; m++ ) {

					if ( m ) {
						offset2 = ( grid2->height - 1 ) * grid2->width;
					} else { offset2 = 0;}
					if ( R_MergedWidthPoints( grid2, offset2 ) ) {
						continue;
					}
					for ( l = 1; l < grid2->width - 1; l++ ) {
						//

#if !defined RTCW_MP
						if ( Q_fabs( grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0] ) > .1 ) {
							continue;
						}
						if ( Q_fabs( grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1] ) > .1 ) {
							continue;
						}
						if ( Q_fabs( grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2] ) > .1 ) {
#else
						if ( c::fabs( grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0] ) > .1 ) {
							continue;
						}
						if ( c::fabs( grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1] ) > .1 ) {
							continue;
						}
						if ( c::fabs( grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2] ) > .1 ) {
#endif // RTCW_XX

							continue;
						}
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
				for ( m = 0; m < 2; m++ ) {

					if ( m ) {
						offset2 = grid2->width - 1;
					} else { offset2 = 0;}
					if ( R_MergedHeightPoints( grid2, offset2 ) ) {
						continue;
					}
					for ( l = 1; l < grid2->height - 1; l++ ) {
						//

#if !defined RTCW_MP
						if ( Q_fabs( grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0] ) > .1 ) {
							continue;
						}
						if ( Q_fabs( grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1] ) > .1 ) {
							continue;
						}
						if ( Q_fabs( grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2] ) > .1 ) {
#else
						if ( c::fabs( grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0] ) > .1 ) {
							continue;
						}
						if ( c::fabs( grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1] ) > .1 ) {
							continue;
						}
						if ( c::fabs( grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2] ) > .1 ) {
#endif // RTCW_XX

							continue;
						}
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		if ( touch ) {
			grid2->lodFixed = 2;
			R_FixSharedVertexLodError_r( start, grid2 );
			//NOTE: this would be correct but makes things really slow
			//grid2->lodFixed = 1;
		}
	}
}

/*
=================
R_FixSharedVertexLodError

This function assumes that all patches in one group are nicely stitched together for the highest LoD.
If this is not the case this function will still do its job but won't fix the highest LoD cracks.
=================
*/
void R_FixSharedVertexLodError( void ) {
	int i;
	srfGridMesh_t *grid1;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if ( grid1->surfaceType != SF_GRID ) {
			continue;
		}
		//
		if ( grid1->lodFixed ) {
			continue;
		}
		//
		grid1->lodFixed = 2;
		// recursively fix other patches in the same LOD group
		R_FixSharedVertexLodError_r( i + 1, grid1 );
	}
}


/*
===============
R_StitchPatches
===============
*/
int R_StitchPatches( int grid1num, int grid2num ) {
	int k, l, m, n, offset1, offset2, row, column;
	srfGridMesh_t *grid1, *grid2;
	float *v1, *v2;

	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	grid2 = (srfGridMesh_t *) s_worldData.surfaces[grid2num].data;
	for ( n = 0; n < 2; n++ ) {
		//
		if ( n ) {
			offset1 = ( grid1->height - 1 ) * grid1->width;
		} else { offset1 = 0;}
		if ( R_MergedWidthPoints( grid1, offset1 ) ) {
			continue;
		}
		for ( k = 0; k < grid1->width - 2; k += 2 ) {

			for ( m = 0; m < 2; m++ ) {

				if ( grid2->width >= MAX_GRID_SIZE ) {
					break;
				}
				if ( m ) {
					offset2 = ( grid2->height - 1 ) * grid2->width;
				} else { offset2 = 0;}
				//if (R_MergedWidthPoints(grid2, offset2))
				//	continue;
				for ( l = 0; l < grid2->width - 1; l++ ) {
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) < .01 &&
						 Q_fabs( v1[1] - v2[1] ) < .01 &&
						 Q_fabs( v1[2] - v2[2] ) < .01 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) < .01 &&
						 c::fabs( v1[1] - v2[1] ) < .01 &&
						 c::fabs( v1[2] - v2[2] ) < .01 ) {
#endif // RTCW_XX

						continue;
					}
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if ( m ) {
						row = grid2->height - 1;
					} else { row = 0;}
					grid2 = R_GridInsertColumn( grid2, l + 1, row,
												grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k + 1] );
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = reinterpret_cast<surfaceType_t*> (grid2);
					return qtrue;
				}
			}
			for ( m = 0; m < 2; m++ ) {

				if ( grid2->height >= MAX_GRID_SIZE ) {
					break;
				}
				if ( m ) {
					offset2 = grid2->width - 1;
				} else { offset2 = 0;}
				//if (R_MergedHeightPoints(grid2, offset2))
				//	continue;
				for ( l = 0; l < grid2->height - 1; l++ ) {
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * ( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * ( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) < .01 &&
						 Q_fabs( v1[1] - v2[1] ) < .01 &&
						 Q_fabs( v1[2] - v2[2] ) < .01 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) < .01 &&
						 c::fabs( v1[1] - v2[1] ) < .01 &&
						 c::fabs( v1[2] - v2[2] ) < .01 ) {
#endif // RTCW_XX

						continue;
					}
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if ( m ) {
						column = grid2->width - 1;
					} else { column = 0;}
					grid2 = R_GridInsertRow( grid2, l + 1, column,
											 grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k + 1] );
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = reinterpret_cast<surfaceType_t*> (grid2);
					return qtrue;
				}
			}
		}
	}
	for ( n = 0; n < 2; n++ ) {
		//
		if ( n ) {
			offset1 = grid1->width - 1;
		} else { offset1 = 0;}
		if ( R_MergedHeightPoints( grid1, offset1 ) ) {
			continue;
		}
		for ( k = 0; k < grid1->height - 2; k += 2 ) {
			for ( m = 0; m < 2; m++ ) {

				if ( grid2->width >= MAX_GRID_SIZE ) {
					break;
				}
				if ( m ) {
					offset2 = ( grid2->height - 1 ) * grid2->width;
				} else { offset2 = 0;}
				//if (R_MergedWidthPoints(grid2, offset2))
				//	continue;
				for ( l = 0; l < grid2->width - 1; l++ ) {
					//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}

					v1 = grid1->verts[grid1->width * ( k + 2 ) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) < .01 &&
						 Q_fabs( v1[1] - v2[1] ) < .01 &&
						 Q_fabs( v1[2] - v2[2] ) < .01 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) < .01 &&
						 c::fabs( v1[1] - v2[1] ) < .01 &&
						 c::fabs( v1[2] - v2[2] ) < .01 ) {
#endif // RTCW_XX

						continue;
					}
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if ( m ) {
						row = grid2->height - 1;
					} else { row = 0;}
					grid2 = R_GridInsertColumn( grid2, l + 1, row,
												grid1->verts[grid1->width * ( k + 1 ) + offset1].xyz, grid1->heightLodError[k + 1] );
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = reinterpret_cast<surfaceType_t*> (grid2);
					return qtrue;
				}
			}
			for ( m = 0; m < 2; m++ ) {

				if ( grid2->height >= MAX_GRID_SIZE ) {
					break;
				}
				if ( m ) {
					offset2 = grid2->width - 1;
				} else { offset2 = 0;}
				//if (R_MergedHeightPoints(grid2, offset2))
				//	continue;
				for ( l = 0; l < grid2->height - 1; l++ ) {
					//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}

					v1 = grid1->verts[grid1->width * ( k + 2 ) + offset1].xyz;
					v2 = grid2->verts[grid2->width * ( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * ( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) < .01 &&
						 Q_fabs( v1[1] - v2[1] ) < .01 &&
						 Q_fabs( v1[2] - v2[2] ) < .01 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) < .01 &&
						 c::fabs( v1[1] - v2[1] ) < .01 &&
						 c::fabs( v1[2] - v2[2] ) < .01 ) {
#endif // RTCW_XX

						continue;
					}
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if ( m ) {
						column = grid2->width - 1;
					} else { column = 0;}
					grid2 = R_GridInsertRow( grid2, l + 1, column,
											 grid1->verts[grid1->width * ( k + 1 ) + offset1].xyz, grid1->heightLodError[k + 1] );
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = reinterpret_cast<surfaceType_t*> (grid2);
					return qtrue;
				}
			}
		}
	}
	for ( n = 0; n < 2; n++ ) {
		//
		if ( n ) {
			offset1 = ( grid1->height - 1 ) * grid1->width;
		} else { offset1 = 0;}
		if ( R_MergedWidthPoints( grid1, offset1 ) ) {
			continue;
		}
		for ( k = grid1->width - 1; k > 1; k -= 2 ) {

			for ( m = 0; m < 2; m++ ) {

				if ( grid2->width >= MAX_GRID_SIZE ) {
					break;
				}
				if ( m ) {
					offset2 = ( grid2->height - 1 ) * grid2->width;
				} else { offset2 = 0;}
				//if (R_MergedWidthPoints(grid2, offset2))
				//	continue;
				for ( l = 0; l < grid2->width - 1; l++ ) {
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) < .01 &&
						 Q_fabs( v1[1] - v2[1] ) < .01 &&
						 Q_fabs( v1[2] - v2[2] ) < .01 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) < .01 &&
						 c::fabs( v1[1] - v2[1] ) < .01 &&
						 c::fabs( v1[2] - v2[2] ) < .01 ) {
#endif // RTCW_XX

						continue;
					}
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if ( m ) {
						row = grid2->height - 1;
					} else { row = 0;}
					grid2 = R_GridInsertColumn( grid2, l + 1, row,
												grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k + 1] );
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = reinterpret_cast<surfaceType_t*> (grid2);
					return qtrue;
				}
			}
			for ( m = 0; m < 2; m++ ) {

				if ( grid2->height >= MAX_GRID_SIZE ) {
					break;
				}
				if ( m ) {
					offset2 = grid2->width - 1;
				} else { offset2 = 0;}
				//if (R_MergedHeightPoints(grid2, offset2))
				//	continue;
				for ( l = 0; l < grid2->height - 1; l++ ) {
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * ( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * ( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) < .01 &&
						 Q_fabs( v1[1] - v2[1] ) < .01 &&
						 Q_fabs( v1[2] - v2[2] ) < .01 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) < .01 &&
						 c::fabs( v1[1] - v2[1] ) < .01 &&
						 c::fabs( v1[2] - v2[2] ) < .01 ) {
#endif // RTCW_XX

						continue;
					}
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if ( m ) {
						column = grid2->width - 1;
					} else { column = 0;}
					grid2 = R_GridInsertRow( grid2, l + 1, column,
											 grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k + 1] );
					if ( !grid2 ) {
						break;
					}
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = reinterpret_cast<surfaceType_t*> (grid2);
					return qtrue;
				}
			}
		}
	}
	for ( n = 0; n < 2; n++ ) {
		//
		if ( n ) {
			offset1 = grid1->width - 1;
		} else { offset1 = 0;}
		if ( R_MergedHeightPoints( grid1, offset1 ) ) {
			continue;
		}
		for ( k = grid1->height - 1; k > 1; k -= 2 ) {
			for ( m = 0; m < 2; m++ ) {

				if ( grid2->width >= MAX_GRID_SIZE ) {
					break;
				}
				if ( m ) {
					offset2 = ( grid2->height - 1 ) * grid2->width;
				} else { offset2 = 0;}
				//if (R_MergedWidthPoints(grid2, offset2))
				//	continue;
				for ( l = 0; l < grid2->width - 1; l++ ) {
					//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}

					v1 = grid1->verts[grid1->width * ( k - 2 ) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) < .01 &&
						 Q_fabs( v1[1] - v2[1] ) < .01 &&
						 Q_fabs( v1[2] - v2[2] ) < .01 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) < .01 &&
						 c::fabs( v1[1] - v2[1] ) < .01 &&
						 c::fabs( v1[2] - v2[2] ) < .01 ) {
#endif // RTCW_XX

						continue;
					}
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if ( m ) {
						row = grid2->height - 1;
					} else { row = 0;}
					grid2 = R_GridInsertColumn( grid2, l + 1, row,
												grid1->verts[grid1->width * ( k - 1 ) + offset1].xyz, grid1->heightLodError[k + 1] );
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = reinterpret_cast<surfaceType_t*> (grid2);
					return qtrue;
				}
			}
			for ( m = 0; m < 2; m++ ) {

				if ( grid2->height >= MAX_GRID_SIZE ) {
					break;
				}
				if ( m ) {
					offset2 = grid2->width - 1;
				} else { offset2 = 0;}
				//if (R_MergedHeightPoints(grid2, offset2))
				//	continue;
				for ( l = 0; l < grid2->height - 1; l++ ) {
					//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}

					v1 = grid1->verts[grid1->width * ( k - 2 ) + offset1].xyz;
					v2 = grid2->verts[grid2->width * ( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( Q_fabs( v1[2] - v2[2] ) > .1 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[1] - v2[1] ) > .1 ) {
						continue;
					}
					if ( c::fabs( v1[2] - v2[2] ) > .1 ) {
#endif // RTCW_XX

						continue;
					}
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * ( l + 1 ) + offset2].xyz;

#if !defined RTCW_MP
					if ( Q_fabs( v1[0] - v2[0] ) < .01 &&
						 Q_fabs( v1[1] - v2[1] ) < .01 &&
						 Q_fabs( v1[2] - v2[2] ) < .01 ) {
#else
					if ( c::fabs( v1[0] - v2[0] ) < .01 &&
						 c::fabs( v1[1] - v2[1] ) < .01 &&
						 c::fabs( v1[2] - v2[2] ) < .01 ) {
#endif // RTCW_XX

						continue;
					}
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if ( m ) {
						column = grid2->width - 1;
					} else { column = 0;}
					grid2 = R_GridInsertRow( grid2, l + 1, column,
											 grid1->verts[grid1->width * ( k - 1 ) + offset1].xyz, grid1->heightLodError[k + 1] );
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = reinterpret_cast<surfaceType_t*> (grid2);
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
===============
R_TryStitchPatch

This function will try to stitch patches in the same LoD group together for the highest LoD.

Only single missing vertice cracks will be fixed.

Vertices will be joined at the patch side a crack is first found, at the other side
of the patch (on the same row or column) the vertices will not be joined and cracks
might still appear at that side.
===============
*/
int R_TryStitchingPatch( int grid1num ) {
	int j, numstitches;
	srfGridMesh_t *grid1, *grid2;

	numstitches = 0;
	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	for ( j = 0; j < s_worldData.numsurfaces; j++ ) {
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if ( grid2->surfaceType != SF_GRID ) {
			continue;
		}
		// grids in the same LOD group should have the exact same lod radius
		if ( grid1->lodRadius != grid2->lodRadius ) {
			continue;
		}
		// grids in the same LOD group should have the exact same lod origin
		if ( grid1->lodOrigin[0] != grid2->lodOrigin[0] ) {
			continue;
		}
		if ( grid1->lodOrigin[1] != grid2->lodOrigin[1] ) {
			continue;
		}
		if ( grid1->lodOrigin[2] != grid2->lodOrigin[2] ) {
			continue;
		}
		//
		while ( R_StitchPatches( grid1num, j ) )
		{
			numstitches++;
		}
	}
	return numstitches;
}

/*
===============
R_StitchAllPatches
===============
*/
void R_StitchAllPatches( void ) {
	int i, stitched, numstitches;
	srfGridMesh_t *grid1;

	numstitches = 0;
	do
	{
		stitched = qfalse;
		for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
			//
			grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
			// if this surface is not a grid
			if ( grid1->surfaceType != SF_GRID ) {
				continue;
			}
			//
			if ( grid1->lodStitched ) {
				continue;
			}
			//
			grid1->lodStitched = qtrue;
			stitched = qtrue;
			//
			numstitches += R_TryStitchingPatch( i );
		}
	}
	while ( stitched );
	ri.Printf( PRINT_ALL, "stitched %d LoD cracks\n", numstitches );
}

/*
===============
R_MovePatchSurfacesToHunk
===============
*/
void R_MovePatchSurfacesToHunk( void ) {
	int i, size;
	srfGridMesh_t *grid, *hunkgrid;

	for ( i = 0; i < s_worldData.numsurfaces; i++ ) {
		//
		grid = (srfGridMesh_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if ( grid->surfaceType != SF_GRID ) {
			continue;
		}
		//
		size = ( grid->width * grid->height - 1 ) * sizeof( drawVert_t ) + sizeof( *grid );
		hunkgrid = static_cast<srfGridMesh_t*> (ri.Hunk_Alloc( size, h_low ));
		Com_Memcpy( hunkgrid, grid, size );

		hunkgrid->widthLodError = static_cast<float*> (ri.Hunk_Alloc( grid->width * 4, h_low ));
		Com_Memcpy( hunkgrid->widthLodError, grid->widthLodError, grid->width * 4 );

		hunkgrid->heightLodError = static_cast<float*> (ri.Hunk_Alloc( grid->height * 4, h_low ));

#if !defined RTCW_ET
		Com_Memcpy( grid->heightLodError, grid->heightLodError, grid->height * 4 );
#else
		// rain - copy into hunkgrid instead of copying grid overtop of itself
		Com_Memcpy( hunkgrid->heightLodError, grid->heightLodError, grid->height * 4 );
#endif // RTCW_XX

		R_FreeSurfaceGridMesh( grid );

		s_worldData.surfaces[i].data = reinterpret_cast<surfaceType_t*> (hunkgrid);
	}
}

/*
===============
R_LoadSurfaces
===============
*/
static void R_LoadSurfaces( lump_t *surfs, lump_t *verts, lump_t *indexLump ) {
	dsurface_t  *in;
	msurface_t  *out;
	drawVert_t  *dv;
	int         *indexes;
	int count;

#if !defined RTCW_ET
	int numFaces, numMeshes, numTriSurfs, numFlares;
#else
	int numFaces, numMeshes, numTriSurfs, numFlares, numFoliage;
#endif // RTCW_XX

	int i;

	numFaces = 0;
	numMeshes = 0;
	numTriSurfs = 0;
	numFlares = 0;

#if defined RTCW_ET
	numFoliage = 0;
#endif // RTCW_XX

	in = reinterpret_cast<dsurface_t*> ( fileBase + surfs->fileofs );
	if ( surfs->filelen % sizeof( *in ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}
	count = surfs->filelen / sizeof( *in );

	dv = reinterpret_cast<drawVert_t*> ( fileBase + verts->fileofs );
	if ( verts->filelen % sizeof( *dv ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}

	indexes = reinterpret_cast<int*> ( fileBase + indexLump->fileofs );
	if ( indexLump->filelen % sizeof( *indexes ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}

	out = static_cast<msurface_t*> (ri.Hunk_Alloc( count * sizeof( *out ), h_low ));

	s_worldData.surfaces = out;
	s_worldData.numsurfaces = count;

	// Ridah, init the surface memory. This is optimization, so we don't have to
	// look for memory for each surface, we allocate a big block and just chew it up
	// as we go
	R_InitSurfMemory();

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		switch ( rtcw::Endian::le( in->surfaceType ) ) {
		case MST_PATCH:
			ParseMesh( in, dv, out );
			numMeshes++;
			break;
		case MST_TRIANGLE_SOUP:
			ParseTriSurf( in, dv, out, indexes );
			numTriSurfs++;
			break;
		case MST_PLANAR:

#if !defined RTCW_ET
			ParseFace( in, dv, out, indexes );
#else
			// ydnar: faces and triangle surfaces are now homogenous
			//%	ParseFace( in, dv, out, indexes );
			ParseTriSurf( in, dv, out, indexes );
#endif // RTCW_XX

			numFaces++;
			break;
		case MST_FLARE:
			ParseFlare( in, dv, out, indexes );
			numFlares++;
			break;

#if defined RTCW_ET
		case MST_FOLIAGE:   // ydnar
			ParseFoliage( in, dv, out, indexes );
			numFoliage++;
			break;
#endif // RTCW_XX

		default:
			ri.Error( ERR_DROP, "Bad surfaceType" );
		}
	}

#ifdef PATCH_STITCHING
	R_StitchAllPatches();
#endif

	R_FixSharedVertexLodError();

#ifdef PATCH_STITCHING
	R_MovePatchSurfacesToHunk();
#endif

#if !defined RTCW_ET
	ri.Printf( PRINT_ALL, "...loaded %d faces, %i meshes, %i trisurfs, %i flares\n",
			   numFaces, numMeshes, numTriSurfs, numFlares );
#else
	ri.Printf( PRINT_ALL, "...loaded %d faces, %i meshes, %i trisurfs, %i flares %i foliage\n",
			   numFaces, numMeshes, numTriSurfs, numFlares, numFoliage );
#endif // RTCW_XX

}



/*
=================
R_LoadSubmodels
=================
*/
static void R_LoadSubmodels( lump_t *l ) {
	dmodel_t    *in;
	bmodel_t    *out;
	int i, j, count;

	in = reinterpret_cast<dmodel_t*> ( fileBase + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}
	count = l->filelen / sizeof( *in );

#if defined RTCW_ET
	s_worldData.numBModels = count;
#endif // RTCW_XX

	s_worldData.bmodels = out = static_cast<bmodel_t*> (ri.Hunk_Alloc( count * sizeof( *out ), h_low ));

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		model_t *model;

		model = R_AllocModel();

		assert( model != NULL );            // this should never happen

		model->type = MOD_BRUSH;

#if !defined RTCW_ET
		model->bmodel = out;
#else
		model->model.bmodel = out;
#endif // RTCW_XX

		Com_sprintf( model->name, sizeof( model->name ), "*%d", i );

		for ( j = 0 ; j < 3 ; j++ ) {
			out->bounds[0][j] = rtcw::Endian::le( in->mins[j] );
			out->bounds[1][j] = rtcw::Endian::le( in->maxs[j] );
		}

		out->firstSurface = s_worldData.surfaces + rtcw::Endian::le( in->firstSurface );
		out->numSurfaces = rtcw::Endian::le( in->numSurfaces );

#if defined RTCW_ET
		// ydnar: for attaching fog brushes to models
		out->firstBrush = rtcw::Endian::le( in->firstBrush );
		out->numBrushes = rtcw::Endian::le( in->numBrushes );

		// ydnar: allocate decal memory
		j = ( i == 0 ? MAX_WORLD_DECALS : MAX_ENTITY_DECALS );
		out->decals = static_cast<decal_t*> (ri.Hunk_Alloc( j * sizeof( *out->decals ), h_low ));
		memset( out->decals, 0, j * sizeof( *out->decals ) );
#endif // RTCW_XX

	}
}



//==================================================================

/*
=================
R_SetParent
=================
*/
static void R_SetParent( mnode_t *node, mnode_t *parent ) {

#if defined RTCW_ET
	//  set parent
#endif // RTCW_XX

	node->parent = parent;

#if defined RTCW_ET
	// handle leaf nodes
#endif // RTCW_XX

	if ( node->contents != -1 ) {

#if defined RTCW_ET
		// add node surfaces to bounds
		if ( node->nummarksurfaces > 0 ) {
			int c;
			msurface_t      **mark;
			srfGeneric_t    *gen;


			// add node surfaces to bounds
			mark = node->firstmarksurface;
			c = node->nummarksurfaces;
			while ( c-- )
			{
				gen = ( srfGeneric_t* )( **mark ).data;
				if ( gen->surfaceType != SF_FACE &&
					 gen->surfaceType != SF_GRID &&
					 gen->surfaceType != SF_TRIANGLES &&
					 gen->surfaceType != SF_FOLIAGE ) {
					continue;
				}
				AddPointToBounds( gen->bounds[ 0 ], node->surfMins, node->surfMaxs );
				AddPointToBounds( gen->bounds[ 1 ], node->surfMins, node->surfMaxs );
				mark++;
			}
		}

		// go back
#endif // RTCW_XX

		return;
	}

#if defined RTCW_ET
	// recurse to child nodes
#endif // RTCW_XX

	R_SetParent( node->children[0], node );
	R_SetParent( node->children[1], node );

#if defined RTCW_ET
	// ydnar: surface bounds
	AddPointToBounds( node->children[ 0 ]->surfMins, node->surfMins, node->surfMaxs );
	AddPointToBounds( node->children[ 0 ]->surfMins, node->surfMins, node->surfMaxs );
	AddPointToBounds( node->children[ 1 ]->surfMins, node->surfMins, node->surfMaxs );
	AddPointToBounds( node->children[ 1 ]->surfMaxs, node->surfMins, node->surfMaxs );
#endif // RTCW_XX

}

/*
=================
R_LoadNodesAndLeafs
=================
*/
static void R_LoadNodesAndLeafs( lump_t *nodeLump, lump_t *leafLump ) {
	int i, j, p;
	dnode_t     *in;
	dleaf_t     *inLeaf;
	mnode_t     *out;
	int numNodes, numLeafs;

	in = reinterpret_cast<dnode_t*> ( fileBase + nodeLump->fileofs );
	if ( nodeLump->filelen % sizeof( dnode_t ) ||
		 leafLump->filelen % sizeof( dleaf_t ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}
	numNodes = nodeLump->filelen / sizeof( dnode_t );
	numLeafs = leafLump->filelen / sizeof( dleaf_t );

	out = static_cast<mnode_t*> (ri.Hunk_Alloc( ( numNodes + numLeafs ) * sizeof( *out ), h_low ));

	s_worldData.nodes = out;
	s_worldData.numnodes = numNodes + numLeafs;
	s_worldData.numDecisionNodes = numNodes;

#if defined RTCW_ET
	// ydnar: skybox optimization
	s_worldData.numSkyNodes = 0;
	s_worldData.skyNodes = static_cast<mnode_t**> (ri.Hunk_Alloc( WORLD_MAX_SKY_NODES * sizeof( *s_worldData.skyNodes ), h_low ));
#endif // RTCW_XX

	// load nodes
	for ( i = 0 ; i < numNodes; i++, in++, out++ )
	{
		for ( j = 0 ; j < 3 ; j++ )
		{
			out->mins[j] = rtcw::Endian::le( in->mins[j] );
			out->maxs[j] = rtcw::Endian::le( in->maxs[j] );
		}

#if defined RTCW_ET
		// ydnar: surface bounds
		VectorCopy( out->mins, out->surfMins );
		VectorCopy( out->maxs, out->surfMaxs );
#endif // RTCW_XX

		p = rtcw::Endian::le( in->planeNum );
		out->plane = s_worldData.planes + p;

		out->contents = CONTENTS_NODE;  // differentiate from leafs

		for ( j = 0 ; j < 2 ; j++ )
		{
			p = rtcw::Endian::le( in->children[j] );
			if ( p >= 0 ) {
				out->children[j] = s_worldData.nodes + p;
			} else {
				out->children[j] = s_worldData.nodes + numNodes + ( -1 - p );
			}
		}
	}

	// load leafs
	inLeaf = reinterpret_cast<dleaf_t*> ( fileBase + leafLump->fileofs );
	for ( i = 0 ; i < numLeafs ; i++, inLeaf++, out++ )
	{
		for ( j = 0 ; j < 3 ; j++ )
		{
			out->mins[j] = rtcw::Endian::le( inLeaf->mins[j] );
			out->maxs[j] = rtcw::Endian::le( inLeaf->maxs[j] );
		}

#if defined RTCW_ET
		// ydnar: surface bounds
		ClearBounds( out->surfMins, out->surfMaxs );
#endif // RTCW_XX

		out->cluster = rtcw::Endian::le( inLeaf->cluster );
		out->area = rtcw::Endian::le( inLeaf->area );

		if ( out->cluster >= s_worldData.numClusters ) {
			s_worldData.numClusters = out->cluster + 1;
		}

		out->firstmarksurface = s_worldData.marksurfaces +
								rtcw::Endian::le( inLeaf->firstLeafSurface );
		out->nummarksurfaces = rtcw::Endian::le( inLeaf->numLeafSurfaces );
	}

	// chain decendants
	R_SetParent( s_worldData.nodes, NULL );
}

//=============================================================================

/*
=================
R_LoadShaders
=================
*/
static void R_LoadShaders( lump_t *l ) {
	int i, count;
	dshader_t   *in, *out;

	in = reinterpret_cast<dshader_t*> ( fileBase + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}
	count = l->filelen / sizeof( *in );
	out = static_cast<dshader_t*> (ri.Hunk_Alloc( count * sizeof( *out ), h_low ));

	s_worldData.shaders = out;
	s_worldData.numShaders = count;

	memcpy( out, in, count * sizeof( *out ) );

	for ( i = 0 ; i < count ; i++ ) {
		out[i].surfaceFlags = rtcw::Endian::le( out[i].surfaceFlags );
		out[i].contentFlags = rtcw::Endian::le( out[i].contentFlags );
	}
}


/*
=================
R_LoadMarksurfaces
=================
*/
static void R_LoadMarksurfaces( lump_t *l ) {
	int i, j, count;
	int     *in;
	msurface_t **out;

	in = reinterpret_cast<int*> ( fileBase + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}
	count = l->filelen / sizeof( *in );
	out = static_cast<msurface_t**> (ri.Hunk_Alloc( count * sizeof( *out ), h_low ));

	s_worldData.marksurfaces = out;
	s_worldData.nummarksurfaces = count;

	for ( i = 0 ; i < count ; i++ )
	{
		j = rtcw::Endian::le( in[i] );
		out[i] = s_worldData.surfaces + j;
	}
}


/*
=================
R_LoadPlanes
=================
*/
static void R_LoadPlanes( lump_t *l ) {
	int i, j;
	cplane_t    *out;
	dplane_t    *in;
	int count;
	int bits;

	in = reinterpret_cast<dplane_t*> ( fileBase + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}
	count = l->filelen / sizeof( *in );
	out = static_cast<cplane_t*> (ri.Hunk_Alloc( count * 2 * sizeof( *out ), h_low ));

	s_worldData.planes = out;
	s_worldData.numplanes = count;

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		bits = 0;
		for ( j = 0 ; j < 3 ; j++ ) {
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
R_LoadFogs

=================
*/
static void R_LoadFogs( lump_t *l, lump_t *brushesLump, lump_t *sidesLump ) {

#if !defined RTCW_ET
	int i;
#else
	int i, j;
#endif // RTCW_XX

	fog_t       *out;
	dfog_t      *fogs;
	dbrush_t    *brushes, *brush;
	dbrushside_t    *sides;
	int count, brushesCount, sidesCount;
	int sideNum;
	int planeNum;
	shader_t    *shader;

#if !defined RTCW_ET
	float d;
	int firstSide;
#else
	int firstSide = 0;
#endif // RTCW_XX

	fogs = reinterpret_cast<dfog_t*> ( fileBase + l->fileofs );
	if ( l->filelen % sizeof( *fogs ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}
	count = l->filelen / sizeof( *fogs );

	// create fog strucutres for them
	s_worldData.numfogs = count + 1;
	s_worldData.fogs = static_cast<fog_t*> (ri.Hunk_Alloc( s_worldData.numfogs * sizeof( *out ), h_low ));
	out = s_worldData.fogs + 1;

#if defined RTCW_ET
	// ydnar: reset global fog
	s_worldData.globalFog = -1;
#endif // RTCW_XX

	if ( !count ) {
		return;
	}

	brushes = reinterpret_cast<dbrush_t*> ( fileBase + brushesLump->fileofs );
	if ( brushesLump->filelen % sizeof( *brushes ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}
	brushesCount = brushesLump->filelen / sizeof( *brushes );

	sides = reinterpret_cast<dbrushside_t*> ( fileBase + sidesLump->fileofs );
	if ( sidesLump->filelen % sizeof( *sides ) ) {
		ri.Error( ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name );
	}
	sidesCount = sidesLump->filelen / sizeof( *sides );

	for ( i = 0 ; i < count ; i++, fogs++ ) {
		out->originalBrushNumber = rtcw::Endian::le( fogs->brushNum );


#if defined RTCW_ET
		// ydnar: global fog has a brush number of -1, and no visible side
		if ( out->originalBrushNumber == -1 ) {
			VectorSet( out->bounds[ 0 ], MIN_WORLD_COORD, MIN_WORLD_COORD, MIN_WORLD_COORD );
			VectorSet( out->bounds[ 1 ], MAX_WORLD_COORD, MAX_WORLD_COORD, MAX_WORLD_COORD );
		} else
		{
#endif // RTCW_XX

		if ( (unsigned)out->originalBrushNumber >= brushesCount ) {
			ri.Error( ERR_DROP, "fog brushNumber out of range" );
		}

#if defined RTCW_ET
			// ydnar: find which bsp submodel the fog volume belongs to
			for ( j = 0; j < s_worldData.numBModels; j++ )
			{
				if ( out->originalBrushNumber >= s_worldData.bmodels[ j ].firstBrush &&
					 out->originalBrushNumber < ( s_worldData.bmodels[ j ].firstBrush + s_worldData.bmodels[ j ].numBrushes ) ) {
					out->modelNum = j;
					break;
				}
			}
#endif // RTCW_XX

		brush = brushes + out->originalBrushNumber;

		firstSide = rtcw::Endian::le( brush->firstSide );

		if ( (unsigned)firstSide > sidesCount - 6 ) {
			ri.Error( ERR_DROP, "fog brush sideNumber out of range" );
		}

		// brushes are always sorted with the axial sides first
		sideNum = firstSide + 0;
		planeNum = rtcw::Endian::le( sides[ sideNum ].planeNum );
		out->bounds[0][0] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 1;
		planeNum = rtcw::Endian::le( sides[ sideNum ].planeNum );
		out->bounds[1][0] = s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 2;
		planeNum = rtcw::Endian::le( sides[ sideNum ].planeNum );
		out->bounds[0][1] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 3;
		planeNum = rtcw::Endian::le( sides[ sideNum ].planeNum );
		out->bounds[1][1] = s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 4;
		planeNum = rtcw::Endian::le( sides[ sideNum ].planeNum );
		out->bounds[0][2] = -s_worldData.planes[ planeNum ].dist;

		sideNum = firstSide + 5;
		planeNum = rtcw::Endian::le( sides[ sideNum ].planeNum );
		out->bounds[1][2] = s_worldData.planes[ planeNum ].dist;

#if defined RTCW_ET
		}
#endif // RTCW_XX

		// get information from the shader for fog parameters
		shader = R_FindShader( fogs->shader, LIGHTMAP_NONE, qtrue );

		out->parms = shader->fogParms;

#if !defined RTCW_ET
		out->colorInt = ColorBytes4( shader->fogParms.color[0] * tr.identityLight,
									 shader->fogParms.color[1] * tr.identityLight,
									 shader->fogParms.color[2] * tr.identityLight, 1.0 );

		d = shader->fogParms.depthForOpaque < 1 ? 1 : shader->fogParms.depthForOpaque;
		out->tcScale = 1.0f / ( d * 8 );
#endif // RTCW_XX

#if defined RTCW_ET
		// Arnout: colorInt is now set in the shader so we can modify it
		out->shader = shader;

		// ydnar: global fog sets clearcolor/zfar
		if ( out->originalBrushNumber == -1 ) {
			s_worldData.globalFog = i + 1;
			VectorCopy( shader->fogParms.color, s_worldData.globalOriginalFog );
			s_worldData.globalOriginalFog[ 3 ] =  shader->fogParms.depthForOpaque;
		}
#endif // RTCW_XX

		// set the gradient vector
		sideNum = rtcw::Endian::le( fogs->visibleSide );

#if !defined RTCW_ET
		if ( sideNum == -1 ) {
#else
		// ydnar: made this check a little more strenuous (was sideNum == -1)
		if ( sideNum < 0 || sideNum >= sidesCount ) {
#endif // RTCW_XX

			out->hasSurface = qfalse;
		} else {
			out->hasSurface = qtrue;
			planeNum = rtcw::Endian::le( sides[ firstSide + sideNum ].planeNum );
			VectorSubtract( vec3_origin, s_worldData.planes[ planeNum ].normal, out->surface );
			out->surface[3] = -s_worldData.planes[ planeNum ].dist;
		}

		out++;
	}

}


/*
==============
R_FindLightGridBounds
==============
*/
void R_FindLightGridBounds( vec3_t mins, vec3_t maxs ) {
	world_t *w;
	msurface_t  *surf;
	srfSurfaceFace_t *surfFace;
//	cplane_t	*plane;
	struct shader_s     *shd;

	qboolean foundGridBrushes = qfalse;
	int i,j;

	w = &s_worldData;

//----(SA)	temp - disable this whole thing for now
	VectorCopy( w->bmodels[0].bounds[0], mins );
	VectorCopy( w->bmodels[0].bounds[1], maxs );
	return;
//----(SA)	temp



#ifdef RTCW_VANILLA
	ClearBounds( mins, maxs );

// wrong!
	for ( i = 0; i < w->bmodels[0].numSurfaces; i++ ) {
		surf = w->bmodels[0].firstSurface + i;
		shd = surf->shader;

		if ( !( *surf->data == SF_FACE ) ) {
			continue;
		}

		if ( !( shd->contentFlags & CONTENTS_LIGHTGRID ) ) {
			continue;
		}

		foundGridBrushes = qtrue;
	}


// wrong!
	for ( i = 0; i < w->numsurfaces; i++ ) {
		surf = &w->surfaces[i];
		shd = surf->shader;
		if ( !( *surf->data == SF_FACE ) ) {
			continue;
		}

		if ( !( shd->contentFlags & CONTENTS_LIGHTGRID ) ) {
			continue;
		}

		foundGridBrushes = qtrue;

		surfFace = ( srfSurfaceFace_t * )surf->data;

		for ( j = 0; j < surfFace->numPoints; j++ ) {
			AddPointToBounds( surfFace->points[j], mins, maxs );
		}

	}

	// go through brushes looking for lightgrid
//	for ( i = 0 ; i < numbrushes ; i++ ) {
//		db = &dbrushes[i];
//
//		if (!(dshaders[db->shaderNum].contentFlags & CONTENTS_LIGHTGRID)) {
//			continue;
//		}
//
//		foundGridBrushes = qtrue;
//
//		// go through light grid surfaces for bounds
//		for ( j = 0 ; j < db->numSides ; j++ ) {
//			s = &dbrushsides[ db->firstSide + j ];
//
//			surfmin[0] = -dplanes[ dbrushsides[ db->firstSide + 0 ].planeNum ].dist - 1;
//			surfmin[1] = -dplanes[ dbrushsides[ db->firstSide + 2 ].planeNum ].dist - 1;
//			surfmin[2] = -dplanes[ dbrushsides[ db->firstSide + 4 ].planeNum ].dist - 1;
//			surfmax[0] = dplanes[ dbrushsides[ db->firstSide + 1 ].planeNum ].dist + 1;
//			surfmax[1] = dplanes[ dbrushsides[ db->firstSide + 3 ].planeNum ].dist + 1;
//			surfmax[2] = dplanes[ dbrushsides[ db->firstSide + 5 ].planeNum ].dist + 1;
//			AddPointToBounds (surfmin, mins, maxs);
//			AddPointToBounds (surfmax, mins, maxs);
//		}
//	}


//----(SA)	temp
	foundGridBrushes = qfalse;  // disable this whole thing for now
//----(SA)	temp

	if ( !foundGridBrushes ) {
		VectorCopy( w->bmodels[0].bounds[0], mins );
		VectorCopy( w->bmodels[0].bounds[1], maxs );
	}
#endif // RTCW_VANILLA
}

/*
================
R_LoadLightGrid

================
*/
void R_LoadLightGrid( lump_t *l ) {
	int i;
	vec3_t maxs;
	int numGridPoints;
	world_t *w;
//	float	*wMins, *wMaxs;
	vec3_t wMins, wMaxs;

	w = &s_worldData;

	w->lightGridInverseSize[0] = 1.0 / w->lightGridSize[0];
	w->lightGridInverseSize[1] = 1.0 / w->lightGridSize[1];
	w->lightGridInverseSize[2] = 1.0 / w->lightGridSize[2];

//----(SA)	modified
	R_FindLightGridBounds( wMins, wMaxs );
//	wMins = w->bmodels[0].bounds[0];
//	wMaxs = w->bmodels[0].bounds[1];
//----(SA)	end

	for ( i = 0 ; i < 3 ; i++ ) {
		w->lightGridOrigin[i] = w->lightGridSize[i] * c::ceil( wMins[i] / w->lightGridSize[i] );
		maxs[i] = w->lightGridSize[i] * c::floor( wMaxs[i] / w->lightGridSize[i] );
		w->lightGridBounds[i] = ( maxs[i] - w->lightGridOrigin[i] ) / w->lightGridSize[i] + 1;
	}

	numGridPoints = w->lightGridBounds[0] * w->lightGridBounds[1] * w->lightGridBounds[2];

	if ( l->filelen != numGridPoints * 8 ) {
		ri.Printf( PRINT_WARNING, "WARNING: light grid mismatch\n" );
		w->lightGridData = NULL;
		return;
	}

	w->lightGridData = static_cast<byte*> (ri.Hunk_Alloc( l->filelen, h_low ));
	memcpy( w->lightGridData, ( void * )( fileBase + l->fileofs ), l->filelen );

	// deal with overbright bits
	for ( i = 0 ; i < numGridPoints ; i++ ) {
		R_ColorShiftLightingBytes( &w->lightGridData[i * 8], &w->lightGridData[i * 8] );
		R_ColorShiftLightingBytes( &w->lightGridData[i * 8 + 3], &w->lightGridData[i * 8 + 3] );
	}
}

/*
================
R_LoadEntities
================
*/
void R_LoadEntities( lump_t *l ) {
	const char* p;
	char* token;
	char* s;
	char keyname[MAX_TOKEN_CHARS];
	char value[MAX_TOKEN_CHARS];
	world_t *w;

	w = &s_worldData;
	w->lightGridSize[0] = 64;
	w->lightGridSize[1] = 64;
	w->lightGridSize[2] = 128;

	p = ( char * )( fileBase + l->fileofs );

	// store for reference by the cgame
	w->entityString = static_cast<char*> (ri.Hunk_Alloc( l->filelen + 1, h_low ));
	strcpy( w->entityString, p );
	w->entityParsePoint = w->entityString;

	token = COM_ParseExt( &p, qtrue );
	if ( !*token || *token != '{' ) {
		return;
	}

	// only parse the world spawn
	while ( 1 ) {
		// parse key
		token = COM_ParseExt( &p, qtrue );

		if ( !*token || *token == '}' ) {
			break;
		}
		Q_strncpyz( keyname, token, sizeof( keyname ) );

		// parse value
		token = COM_ParseExt( &p, qtrue );

		if ( !*token || *token == '}' ) {
			break;
		}
		Q_strncpyz( value, token, sizeof( value ) );

#if !defined RTCW_ET
		// check for remapping of shaders for vertex lighting
		s = const_cast<char*>("vertexremapshader");
		if ( !Q_strncmp( keyname, s, strlen( s ) ) ) {
			s = strchr( value, ';' );
			if ( !s ) {
				ri.Printf( PRINT_WARNING, "WARNING: no semi colon in vertexshaderremap '%s'\n", value );
				break;
			}
			*s++ = 0;
#endif // RTCW_XX

#if defined RTCW_SP
			if ( r_vertexLight->integer ) {
				R_RemapShader( value, s, "0" );
			}
#elif defined RTCW_MP
			// NERVE - SMF - temp fix, don't allow remapping of shader
			//  - fixes not drawing terrain surfaces when r_vertexLight is true even when remapped shader is present
//			if (r_vertexLight->integer) {
//				R_RemapShader(value, s, "0");
//			}
#endif // RTCW_XX

#if !defined RTCW_ET
			continue;
		}
#endif // RTCW_XX

		// check for remapping of shaders
		s = const_cast<char*>("remapshader");
		if ( !Q_strncmp( keyname, s, strlen( s ) ) ) {
			s = strchr( value, ';' );
			if ( !s ) {
				ri.Printf( PRINT_WARNING, "WARNING: no semi colon in shaderremap '%s'\n", value );
				break;
			}
			*s++ = 0;
			R_RemapShader( value, s, "0" );
			continue;
		}
		// check for a different grid size
		if ( !Q_stricmp( keyname, "gridsize" ) ) {
			sscanf( value, "%f %f %f", &w->lightGridSize[0], &w->lightGridSize[1], &w->lightGridSize[2] );
			continue;
		}
	}
}

/*
=================
R_GetEntityToken
=================
*/
qboolean R_GetEntityToken( char *buffer, int size ) {
	const char  *s;

	s = COM_Parse( const_cast<const char**>(&s_worldData.entityParsePoint) );
	Q_strncpyz( buffer, s, size );
	if ( !s_worldData.entityParsePoint || !s[0] ) {
		s_worldData.entityParsePoint = s_worldData.entityString;
		return qfalse;
	} else {
		return qtrue;
	}
}

/*
=================
RE_LoadWorldMap

Called directly from cgame
=================
*/
void RE_LoadWorldMap( const char *name ) {
	int i;
	dheader_t   *header;
	byte        *buffer;
	byte        *startMarker;

	skyboxportal = 0;

	if ( tr.worldMapLoaded ) {
		ri.Error( ERR_DROP, "ERROR: attempted to redundantly load world map\n" );
	}

	// set default sun direction to be used if it isn't
	// overridden by a shader
	tr.sunDirection[0] = 0.45;
	tr.sunDirection[1] = 0.3;
	tr.sunDirection[2] = 0.9;

	tr.sunShader = 0;   // clear sunshader so it's not there if the level doesn't specify it

	// invalidate fogs (likely to be re-initialized to new values by the current map)
	// TODO:(SA)this is sort of silly.  I'm going to do a general cleanup on fog stuff
	//			now that I can see how it's been used.  (functionality can narrow since
	//			it's not used as much as it's designed for.)
	R_SetFog( FOG_SKY,       0, 0, 0, 0, 0, 0 );
	R_SetFog( FOG_PORTALVIEW,0, 0, 0, 0, 0, 0 );
	R_SetFog( FOG_HUD,       0, 0, 0, 0, 0, 0 );
	R_SetFog( FOG_MAP,       0, 0, 0, 0, 0, 0 );
	R_SetFog( FOG_CURRENT,   0, 0, 0, 0, 0, 0 );
	R_SetFog( FOG_TARGET,    0, 0, 0, 0, 0, 0 );
	R_SetFog( FOG_WATER,     0, 0, 0, 0, 0, 0 );
	R_SetFog( FOG_SERVER,    0, 0, 0, 0, 0, 0 );

	VectorNormalize( tr.sunDirection );

	tr.worldMapLoaded = qtrue;

#if defined RTCW_ET
	tr.worldDir = NULL;
#endif // RTCW_XX

	// load it
	ri.FS_ReadFile( name, (void **)&buffer );
	if ( !buffer ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: %s not found", name );
	}

#if defined RTCW_ET
	// ydnar: set map meta dir
	tr.worldDir = CopyString( name );
	COM_StripExtension( tr.worldDir, tr.worldDir );
#endif // RTCW_XX

	// clear tr.world so if the level fails to load, the next
	// try will not look at the partially loaded version
	tr.world = NULL;

	memset( &s_worldData, 0, sizeof( s_worldData ) );
	Q_strncpyz( s_worldData.name, name, sizeof( s_worldData.name ) );

	Q_strncpyz( s_worldData.baseName, COM_SkipPath( s_worldData.name ), sizeof( s_worldData.name ) );
	COM_StripExtension( s_worldData.baseName, s_worldData.baseName );

	startMarker = static_cast<byte*> (ri.Hunk_Alloc( 0, h_low ));
	c_gridVerts = 0;

	header = (dheader_t *)buffer;
	fileBase = (byte *)header;

	i = rtcw::Endian::le( header->version );

#if defined RTCW_SP
#ifndef _SKIP_BSP_CHECK
	if ( i != BSP_VERSION ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: %s has wrong version number (%i should be %i)",
				  name, i, BSP_VERSION );
	}
#endif
#else
	if ( i != BSP_VERSION ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: %s has wrong version number (%i should be %i)",
				  name, i, BSP_VERSION );
	}
#endif // RTCW_XX

	// swap all the lumps
	for ( i = 0 ; i < sizeof( dheader_t ) / 4 ; i++ ) {
		( (int *)header )[i] = rtcw::Endian::le( ( (int *)header )[i] );
	}

	// load into heap
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
	R_LoadShaders( &header->lumps[LUMP_SHADERS] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
	R_LoadLightmaps( &header->lumps[LUMP_LIGHTMAPS] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
	R_LoadPlanes( &header->lumps[LUMP_PLANES] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );

#if !defined RTCW_ET
	R_LoadFogs( &header->lumps[LUMP_FOGS], &header->lumps[LUMP_BRUSHES], &header->lumps[LUMP_BRUSHSIDES] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
#else
	//%	R_LoadFogs( &header->lumps[LUMP_FOGS], &header->lumps[LUMP_BRUSHES], &header->lumps[LUMP_BRUSHSIDES] );
	//%	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
#endif // RTCW_XX

	R_LoadSurfaces( &header->lumps[LUMP_SURFACES], &header->lumps[LUMP_DRAWVERTS], &header->lumps[LUMP_DRAWINDEXES] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
	R_LoadMarksurfaces( &header->lumps[LUMP_LEAFSURFACES] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
	R_LoadNodesAndLeafs( &header->lumps[LUMP_NODES], &header->lumps[LUMP_LEAFS] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
	R_LoadSubmodels( &header->lumps[LUMP_MODELS] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );

#if defined RTCW_ET
	// moved fog lump loading here, so fogs can be tagged with a model num
	R_LoadFogs( &header->lumps[LUMP_FOGS], &header->lumps[LUMP_BRUSHES], &header->lumps[LUMP_BRUSHSIDES] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
#endif // RTCW_XX

	R_LoadVisibility( &header->lumps[LUMP_VISIBILITY] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
	R_LoadEntities( &header->lumps[LUMP_ENTITIES] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );
	R_LoadLightGrid( &header->lumps[LUMP_LIGHTGRID] );
	ri.Cmd_ExecuteText( EXEC_NOW, "updatescreen\n" );

	s_worldData.dataSize = (byte *)ri.Hunk_Alloc( 0, h_low ) - startMarker;

	// only set tr.world now that we know the entire level has loaded properly
	tr.world = &s_worldData;

	// reset fog to world fog (if present)

#if defined RTCW_SP
//	R_SetFog(FOG_CMD_SWITCHFOG, FOG_MAP,20,0,0,0,0);
#else
	R_SetFog( FOG_CMD_SWITCHFOG, FOG_MAP,20,0,0,0,0 );
#endif // RTCW_XX

//----(SA)	set the sun shader if there is one
	if ( tr.sunShaderName ) {
		tr.sunShader = R_FindShader( tr.sunShaderName, LIGHTMAP_NONE, qtrue );
	}

//----(SA)	end
	ri.FS_FreeFile( buffer );
}

