/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// tr_models.c -- model loading and caching

#include "tr_local.h"
#include "rtcw_endian.h"

#define LL( x ) x = rtcw::Endian::le( x )

// Ridah
static qboolean R_LoadMDC( model_t *mod, int lod, void *buffer, const char *mod_name );
// done.
static qboolean R_LoadMD3( model_t *mod, int lod, void *buffer, const char *name );
static qboolean R_LoadMDS( model_t *mod, void *buffer, const char *name );

#if defined RTCW_ET
static qboolean R_LoadMDM( model_t *mod, void *buffer, const char *name );
static qboolean R_LoadMDX( model_t *mod, void *buffer, const char *name );
#endif // RTCW_XX

model_t *loadmodel;

// BBi
//#if !defined RTCW_ET
// BBi
extern cvar_t *r_compressModels;
extern cvar_t *r_exportCompressedModels;
// BBi
//#endif // RTCW_XX
// BBi

extern cvar_t *r_buildScript;

/*
** R_GetModelByHandle
*/
model_t *R_GetModelByHandle( qhandle_t index ) {
	model_t     *mod;

	// out of range gets the defualt model
	if ( index < 1 || index >= tr.numModels ) {
		return tr.models[0];
	}

	mod = tr.models[index];

	return mod;
}

//===============================================================================

/*
** R_AllocModel
*/
model_t *R_AllocModel( void ) {
	model_t     *mod;

	if ( tr.numModels == MAX_MOD_KNOWN ) {
		return NULL;
	}

	mod = static_cast<model_t*> (ri.Hunk_Alloc( sizeof( *tr.models[tr.numModels] ), h_low ));
	mod->index = tr.numModels;
	tr.models[tr.numModels] = mod;
	tr.numModels++;

	return mod;
}

#if defined RTCW_ET
/*
R_LoadModelShadow()
loads a model's shadow script
*/

void R_LoadModelShadow( model_t *mod ) {
	unsigned    *buf;
	char filename[ 1024 ];
	shader_t    *sh;

	// set default shadow
	mod->shadowShader = 0;

	// build name
	COM_StripExtension2( mod->name, filename, sizeof( filename ) );
	COM_DefaultExtension( filename, 1024, ".shadow" );

	// load file
	ri.FS_ReadFile( filename, (void**) &buf );
	if ( buf != NULL ) {
		char    *shadowBits;

		shadowBits = strchr( (char*) buf, ' ' );
		if ( shadowBits != NULL ) {
			*shadowBits = '\0';
			shadowBits++;

			if ( strlen( (char*) buf ) >= MAX_QPATH ) {
				Com_Printf( "R_LoadModelShadow: Shader name exceeds MAX_QPATH\n" );
				mod->shadowShader = 0;
			} else {
				sh = R_FindShader( (char*) buf, LIGHTMAP_NONE, qtrue );

				if ( sh->defaultShader ) {
					mod->shadowShader = 0;
				} else {
					mod->shadowShader = sh->index;
				}
			}
			sscanf( shadowBits, "%f %f %f %f %f %f",
					&mod->shadowParms[ 0 ], &mod->shadowParms[ 1 ], &mod->shadowParms[ 2 ],
					&mod->shadowParms[ 3 ], &mod->shadowParms[ 4 ], &mod->shadowParms[ 5 ] );
		}
		ri.FS_FreeFile( buf );
	}
}
#endif // RTCW_XX

/*
====================
RE_RegisterModel

Loads in a model for the given name

Zero will be returned if the model fails to load.
An entry will be retained for failed models as an
optimization to prevent disk rescanning if they are
asked for again.
====================
*/
qhandle_t RE_RegisterModel( const char *name ) {
	model_t     *mod;
	unsigned    *buf;
	int lod;
	int ident = 0;         // TTimo: init
	qboolean loaded;
	qhandle_t hModel;
	int numLoaded;

#if defined RTCW_ET
	char filename[1024];
#endif // RTCW_XX

	if ( !name || !name[0] ) {
		// Ridah, disabled this, we can see models that can't be found because they won't be there
		//ri.Printf( PRINT_ALL, "RE_RegisterModel: NULL name\n" );
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Printf( "Model name exceeds MAX_QPATH\n" );
		return 0;
	}

	// Ridah, caching
	if ( r_cacheGathering->integer ) {
		ri.Cmd_ExecuteText( EXEC_NOW, va( "cache_usedfile model %s\n", name ) );
	}

	//
	// search the currently loaded models
	//
	for ( hModel = 1 ; hModel < tr.numModels; hModel++ ) {
		mod = tr.models[hModel];
		if ( !Q_stricmp( mod->name, name ) ) {
			if ( mod->type == MOD_BAD ) {
				return 0;
			}
			return hModel;
		}
	}

	// allocate a new model_t

	if ( ( mod = R_AllocModel() ) == NULL ) {
		ri.Printf( PRINT_WARNING, "RE_RegisterModel: R_AllocModel() failed for '%s'\n", name );
		return 0;
	}

	// only set the name after the model has been successfully loaded
	Q_strncpyz( mod->name, name, sizeof( mod->name ) );

#if defined RTCW_SP
// GR - by default models are not tessellated
	mod->ATI_tess = qfalse;
// GR - check if can be tessellated...
//		make sure to tessellate model heads
	if ( strstr( name, "head" ) ) {
		mod->ATI_tess = qtrue;
	}
#endif // RTCW_XX

	// make sure the render thread is stopped
	R_SyncRenderThread();

	// Ridah, look for it cached
	if ( R_FindCachedModel( name, mod ) ) {

#if defined RTCW_ET
		R_LoadModelShadow( mod );
#endif // RTCW_XX

		return mod->index;
	}
	// done.

#if defined RTCW_ET
	R_LoadModelShadow( mod );
#endif // RTCW_XX

	mod->numLods = 0;

	//
	// load the files
	//
	numLoaded = 0;

#if !defined RTCW_ET
	if ( strstr( name, ".mds" ) ) {  // try loading skeletal file
#else
	if ( strstr( name, ".mds" ) || strstr( name, ".mdm" ) || strstr( name, ".mdx" ) ) {    // try loading skeletal file
#endif // RTCW_XX

		loaded = qfalse;
		ri.FS_ReadFile( name, (void **)&buf );
		if ( buf ) {
			loadmodel = mod;

			ident = rtcw::Endian::le( *(unsigned *)buf );
			if ( ident == MDS_IDENT ) {
				loaded = R_LoadMDS( mod, buf, name );

#if defined RTCW_ET
			} else if ( ident == MDM_IDENT ) {
				loaded = R_LoadMDM( mod, buf, name );
			} else if ( ident == MDX_IDENT ) {
				loaded = R_LoadMDX( mod, buf, name );
#endif // RTCW_XX

			}

			ri.FS_FreeFile( buf );
		}

		if ( loaded ) {
			return mod->index;
		}
	}

	for ( lod = MD3_MAX_LODS - 1 ; lod >= 0 ; lod-- ) {

#if !defined RTCW_ET
		char filename[1024];
#endif // RTCW_XX

		strcpy( filename, name );

		if ( lod != 0 ) {
			char namebuf[80];

			if ( strrchr( filename, '.' ) ) {
				*strrchr( filename, '.' ) = 0;
			}
			sprintf( namebuf, "_%d.md3", lod );
			strcat( filename, namebuf );
		}

#if !defined RTCW_ET
		if ( r_compressModels->integer ) {
			filename[strlen( filename ) - 1] = '3';  // try MD3 first
		} else {
#endif // RTCW_XX

			filename[strlen( filename ) - 1] = 'c';  // try MDC first

#if !defined RTCW_ET
		}
#endif // RTCW_XX

		ri.FS_ReadFile( filename, (void **)&buf );

		if ( !buf ) {

#if !defined RTCW_ET
			if ( r_compressModels->integer ) {
				filename[strlen( filename ) - 1] = 'c';  // try MDC second
			} else {
#endif // RTCW_XX

				filename[strlen( filename ) - 1] = '3';  // try MD3 second

#if !defined RTCW_ET
			}
#endif // RTCW_XX

			ri.FS_ReadFile( filename, (void **)&buf );
			if ( !buf ) {
				continue;
			}
		}

		loadmodel = mod;

		ident = rtcw::Endian::le( *(unsigned *)buf );
		// Ridah, mesh compression
		if ( ident != MD3_IDENT && ident != MDC_IDENT ) {
			ri.Printf( PRINT_WARNING,"RE_RegisterModel: unknown fileid for %s\n", name );
			goto fail;
		}

		if ( ident == MD3_IDENT ) {
			loaded = R_LoadMD3( mod, lod, buf, name );

#if !defined RTCW_ET
			if ( r_compressModels->integer && r_exportCompressedModels->integer && mod->mdc[lod] ) {
				// save it out
				filename[strlen( filename ) - 1] = 'c';
				ri.FS_WriteFile( filename, mod->mdc[lod], mod->mdc[lod]->ofsEnd );
				// if building, open the file so it gets copied
				if ( r_buildScript->integer ) {
					ri.FS_ReadFile( filename, NULL );
				}
			}
#endif // RTCW_XX

		} else {
			loaded = R_LoadMDC( mod, lod, buf, name );
		}
		// done.

		ri.FS_FreeFile( buf );

		if ( !loaded ) {
			if ( lod == 0 ) {
				goto fail;
			} else {
				break;
			}
		} else {
			mod->numLods++;
			numLoaded++;
			// if we have a valid model and are biased
			// so that we won't see any higher detail ones,
			// stop loading them

#if !defined RTCW_ET
			if ( lod <= r_lodbias->integer ) {
				break;
			}
#else
// Arnout: don't need this anymore,
//			if ( lod <= r_lodbias->integer ) {
//				break;
//			}
#endif // RTCW_XX

		}
	}


	if ( numLoaded ) {
		// duplicate into higher lod spots that weren't
		// loaded, in case the user changes r_lodbias on the fly
		for ( lod-- ; lod >= 0 ; lod-- ) {
			mod->numLods++;
			// Ridah, mesh compression
			//	this check for mod->md3[0] could leave mod->md3[0] == 0x0000000 if r_lodbias is set to anything except '0'
			//	which causes trouble in tr_mesh.c in R_AddMD3Surfaces() and other locations since it checks md3[0]
			//	for various things.
			if ( ident == MD3_IDENT ) { //----(SA)	modified
//			if (mod->md3[0])		//----(SA)	end

#if !defined RTCW_ET
				mod->md3[lod] = mod->md3[lod + 1];
			} else {
				mod->mdc[lod] = mod->mdc[lod + 1];
#else
				mod->model.md3[lod] = mod->model.md3[lod + 1];
			} else {
				mod->model.mdc[lod] = mod->model.mdc[lod + 1];
#endif // RTCW_XX

			}
			// done.
		}

		return mod->index;
	}

fail:
	// we still keep the model_t around, so if the model name is asked for
	// again, we won't bother scanning the filesystem
	mod->type = MOD_BAD;
	return 0;
}

//-------------------------------------------------------------------------------
// Ridah, mesh compression
float r_anormals[NUMMDCVERTEXNORMALS][3] = {
#include "anorms256.h"
};

/*
=============
R_MDC_GetVec
=============
*/
void R_MDC_GetVec( unsigned char anorm, vec3_t dir ) {
	VectorCopy( r_anormals[anorm], dir );
}

/*
=============
R_MDC_GetAnorm
=============
*/
unsigned char R_MDC_GetAnorm( const vec3_t dir ) {

#if defined RTCW_SP
	int i, best_start_i[3], next_start, next_end, best = 0;     // TTimo: init
#else
	int i, best_start_i[3], next_start, next_end;
	int best = 0; // TTimo: init
#endif // RTCW_XX

	float best_diff, group_val, this_val, diff;
	float   *this_norm;

	// find best Z match

	if ( dir[2] > 0.097545f ) {
		next_start = 144;
		next_end = NUMMDCVERTEXNORMALS;
	} else
	{
		next_start = 0;
		next_end = 144;
	}

	best_diff = 999;
	this_val = -999;

	for ( i = next_start ; i < next_end ; i++ )
	{
		if ( r_anormals[i][2] == this_val ) {
			continue;
		} else {
			this_val = r_anormals[i][2];
		}

#if !defined RTCW_ET
		if ( ( diff = c::fabs( dir[2] - r_anormals[i][2] ) ) < best_diff ) {
#else
		if ( ( diff = Q_fabs( dir[2] - r_anormals[i][2] ) ) < best_diff ) {
#endif // RTCW_XX

			best_diff = diff;
			best_start_i[2] = i;

		}

		if ( next_start ) {
			if ( r_anormals[i][2] > dir[2] ) {
				break;  // we've gone passed the dir[2], so we can't possibly find a better match now
			}
		} else {
			if ( r_anormals[i][2] < dir[2] ) {
				break;  // we've gone passed the dir[2], so we can't possibly find a better match now
			}
		}
	}

	best_diff = -999;

	// find best match within the Z group

	for ( i = best_start_i[2], group_val = r_anormals[i][2]; i < NUMMDCVERTEXNORMALS; i++ )
	{
		this_norm = r_anormals[i];

		if ( this_norm[2] != group_val ) {
			break; // done checking the group
		}
/*
		if (	(this_norm[0] < 0 && dir[0] > 0)
			||	(this_norm[0] > 0 && dir[0] < 0)
			||	(this_norm[1] < 0 && dir[1] > 0)
			||	(this_norm[1] > 0 && dir[1] < 0))
			continue;
*/
		diff = DotProduct( dir, this_norm );

		if ( diff > best_diff ) {
			best_diff = diff;
			best = i;
		}
	}

	return (unsigned char)best;
}

/*
=================
R_MDC_EncodeOfsVec
=================
*/
qboolean R_MDC_EncodeXyzCompressed( const vec3_t vec, const vec3_t normal, mdcXyzCompressed_t *out ) {
	mdcXyzCompressed_t retval;
	int i;
	unsigned char anorm;

	i = sizeof( mdcXyzCompressed_t );

	retval.ofsVec = 0;
	for ( i = 0; i < 3; i++ ) {

#if !defined RTCW_ET
		if ( c::fabs( vec[i] ) >= MDC_MAX_DIST ) {
#else
		if ( Q_fabs( vec[i] ) >= MDC_MAX_DIST ) {
#endif // RTCW_XX

			return qfalse;
		}

#if !defined RTCW_ET
		retval.ofsVec += ( ( (int)c::fabs( ( vec[i] + MDC_DIST_SCALE * 0.5 ) * ( 1.0 / MDC_DIST_SCALE ) + MDC_MAX_OFS ) ) << ( i * MDC_BITS_PER_AXIS ) );
#else
		retval.ofsVec += ( ( (int)Q_fabs( ( vec[i] + MDC_DIST_SCALE * 0.5 ) * ( 1.0 / MDC_DIST_SCALE ) + MDC_MAX_OFS ) ) << ( i * MDC_BITS_PER_AXIS ) );
#endif // RTCW_XX

	}
	anorm = R_MDC_GetAnorm( normal );
	retval.ofsVec |= ( (int)anorm ) << 24;

	*out = retval;
	return qtrue;
}

/*
=================
R_MDC_DecodeXyzCompressed
=================
*/
#if 0   // unoptimized version, used for finding right settings
void R_MDC_DecodeXyzCompressed( mdcXyzCompressed_t *xyzComp, vec3_t out, vec3_t normal ) {
	int i;

	for ( i = 0; i < 3; i++ ) {
		out[i] = ( (float)( ( xyzComp->ofsVec >> ( i * MDC_BITS_PER_AXIS ) ) & ( ( 1 << MDC_BITS_PER_AXIS ) - 1 ) ) - MDC_MAX_OFS ) * MDC_DIST_SCALE;
	}
	R_MDC_GetVec( ( unsigned char )( xyzComp->ofsVec >> 24 ), normal );
}
#endif

#if !defined RTCW_ET
/*
=================
R_MDC_GetXyzCompressed
=================
*/
static qboolean R_MDC_GetXyzCompressed( md3Header_t *md3, md3XyzNormal_t *newXyz, vec3_t oldPos, mdcXyzCompressed_t *out, qboolean verify ) {
	vec3_t newPos, vec;
	int i;
	vec3_t pos, dir, norm, outnorm;

	for ( i = 0; i < 3; i++ ) {
		newPos[i] = (float)newXyz->xyz[i] * MD3_XYZ_SCALE;
	}

	VectorSubtract( newPos, oldPos, vec );
	R_LatLongToNormal( norm, newXyz->normal );
	if ( !R_MDC_EncodeXyzCompressed( vec, norm, out ) ) {
		return qfalse;
	}

	// calculate the uncompressed position
	R_MDC_DecodeXyzCompressed( out->ofsVec, dir, outnorm );
	VectorAdd( oldPos, dir, pos );

	if ( verify ) {
		if ( Distance( newPos, pos ) > MDC_MAX_ERROR ) {
			return qfalse;
		}
	}

	return qtrue;
}


/*
=================
R_MDC_CompressSurfaceFrame
=================
*/
static qboolean R_MDC_CompressSurfaceFrame( md3Header_t *md3, md3Surface_t *surf, int frame, int lastBaseFrame, mdcXyzCompressed_t *out ) {
	int i, j;
	md3XyzNormal_t  *xyz, *baseXyz;
	vec3_t oldPos;

	xyz = ( md3XyzNormal_t * )( (byte *)surf + surf->ofsXyzNormals );
	baseXyz = xyz + ( lastBaseFrame * surf->numVerts );
	xyz += ( frame * surf->numVerts );

	for ( i = 0; i < surf->numVerts; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			oldPos[j] = (float)baseXyz[i].xyz[j] * MD3_XYZ_SCALE;
		}
		if ( !R_MDC_GetXyzCompressed( md3, &xyz[i], oldPos, &out[i], qtrue ) ) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
=================
R_MDC_CanCompressSurfaceFrame
=================
*/
static qboolean R_MDC_CanCompressSurfaceFrame( md3Header_t *md3, md3Surface_t *surf, int frame, int lastBaseFrame ) {
	int i, j;
	md3XyzNormal_t  *xyz, *baseXyz;
	mdcXyzCompressed_t xyzComp;
	vec3_t oldPos;

	xyz = ( md3XyzNormal_t * )( (byte *)surf + surf->ofsXyzNormals );
	baseXyz = xyz + ( lastBaseFrame * surf->numVerts );
	xyz += ( frame * surf->numVerts );

	for ( i = 0; i < surf->numVerts; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			oldPos[j] = (float)baseXyz[i].xyz[j] * MD3_XYZ_SCALE;
		}
		if ( !R_MDC_GetXyzCompressed( md3, &xyz[i], oldPos, &xyzComp, qtrue ) ) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
=================
R_MD3toMDC

  Converts a model_t from md3 to mdc format
=================
*/
static qboolean R_MDC_ConvertMD3( model_t *mod, int lod, const char *mod_name ) {
	int i, j, f, c, k;
	md3Surface_t        *surf;
	md3Header_t         *md3;
	int                 *baseFrames;
	int numBaseFrames;

	qboolean foundBase;

	mdcHeader_t         *mdc, mdcHeader;
	mdcSurface_t        *cSurf;
	short               *frameBaseFrames, *frameCompFrames;

	mdcTag_t            *mdcTag;
	md3Tag_t            *md3Tag;

	vec3_t axis[3], angles;
	float ftemp;

	md3 = mod->md3[lod];

	baseFrames = static_cast<int*> (ri.Hunk_AllocateTempMemory( sizeof( *baseFrames ) * md3->numFrames ));

	// the first frame is always a base frame
	numBaseFrames = 0;
	memset( baseFrames, 0, sizeof( *baseFrames ) * md3->numFrames );
	baseFrames[numBaseFrames++] = 0;

	// first calculate how many baseframes we need, and which frames they are on
	// we need to treat the entire model as a single surface, if we compress some surfaces, and not others,
	// we may get tearing between surfaces

	for ( f = 1; f < md3->numFrames; f++ ) {

		surf = ( md3Surface_t * )( (byte *)md3 + md3->ofsSurfaces );
		foundBase = qfalse;
		for ( i = 0 ; i < md3->numSurfaces ; i++ ) {

			// process the verts in this surface, checking to see if the compressed
			// version will be close enough to the actual vert
			if ( !foundBase && !R_MDC_CanCompressSurfaceFrame( md3, surf, f, baseFrames[numBaseFrames - 1] ) ) {
				baseFrames[numBaseFrames++] = f;
				foundBase = qtrue;
			}

			// find the next surface
			surf = ( md3Surface_t * )( (byte *)surf + surf->ofsEnd );
		}

	}

	// success, so fill in the necessary data to the model_t
	mdcHeader.ident = MDC_IDENT;
	mdcHeader.version = MDC_VERSION;
	Q_strncpyz( mdcHeader.name, md3->name, sizeof( mdcHeader.name ) );
	mdcHeader.flags = md3->flags;
	mdcHeader.numFrames = md3->numFrames;
	mdcHeader.numTags = md3->numTags;
	mdcHeader.numSurfaces = md3->numSurfaces;
	mdcHeader.numSkins = md3->numSkins;
	mdcHeader.ofsFrames = sizeof( mdcHeader_t );
	mdcHeader.ofsTagNames = mdcHeader.ofsFrames + mdcHeader.numFrames * sizeof( md3Frame_t );
	mdcHeader.ofsTags = mdcHeader.ofsTagNames + mdcHeader.numTags * sizeof( mdcTagName_t );
	mdcHeader.ofsSurfaces = mdcHeader.ofsTags + mdcHeader.numTags * mdcHeader.numFrames * sizeof( mdcTag_t );
	mdcHeader.ofsEnd = mdcHeader.ofsSurfaces;

	surf = ( md3Surface_t * )( (byte *)md3 + md3->ofsSurfaces );
	for ( f = 0; f < md3->numSurfaces; f++ ) {
		mdcHeader.ofsEnd += sizeof( mdcSurface_t )
							+ surf->numShaders * sizeof( md3Shader_t )
							+ surf->numTriangles * sizeof( md3Triangle_t )
							+ surf->numVerts * sizeof( md3St_t )
							+ surf->numVerts * numBaseFrames * sizeof( md3XyzNormal_t )
							+ surf->numVerts * ( md3->numFrames - numBaseFrames ) * sizeof( mdcXyzCompressed_t )
							+ sizeof( short ) * md3->numFrames
							+ sizeof( short ) * md3->numFrames;

		surf = ( md3Surface_t * )( (byte *)surf + surf->ofsEnd );
	}

	// report the memory differences
	Com_Printf( "Compressed %s. Old = %i, New = %i\n", mod_name, md3->ofsEnd, mdcHeader.ofsEnd );

	mdc = static_cast<mdcHeader_t*> (ri.Hunk_Alloc( mdcHeader.ofsEnd, h_low ));
	mod->mdc[lod] = mdc;

	// we have the memory allocated, so lets fill it in

	// header info
	*mdc = mdcHeader;
	// frames
	memcpy( ( md3Frame_t * )( (byte *)mdc + mdc->ofsFrames ), ( md3Frame_t * )( (byte *)md3 + md3->ofsFrames ), mdcHeader.numFrames * sizeof( md3Frame_t ) );
	// tag names
	for ( j = 0; j < md3->numTags; j++ ) {
		memcpy( ( mdcTagName_t * )( (byte *)mdc + mdc->ofsTagNames ) + j, ( ( md3Tag_t * )( (byte *)md3 + md3->ofsTags ) + j )->name, sizeof( mdcTagName_t ) );
	}
	// tags
	mdcTag = ( ( mdcTag_t * )( (byte *)mdc + mdc->ofsTags ) );
	md3Tag = ( ( md3Tag_t * )( (byte *)md3 + md3->ofsTags ) );
	for ( f = 0; f < md3->numFrames; f++ ) {
		for ( j = 0; j < md3->numTags; j++, mdcTag++, md3Tag++ ) {
			for ( k = 0; k < 3; k++ ) {
				// origin
				ftemp = md3Tag->origin[k] / MD3_XYZ_SCALE;
				mdcTag->xyz[k] = (short)ftemp;
				// axis
				VectorCopy( md3Tag->axis[k], axis[k] );
			}
			// convert the axis to angles
			AxisToAngles( axis, angles );
			// copy them into the new tag
			for ( k = 0; k < 3; k++ ) {
				mdcTag->angles[k] = angles[k] / MDC_TAG_ANGLE_SCALE;
			}
		}
	}
	// surfaces
	surf = ( md3Surface_t * )( (byte *)md3 + md3->ofsSurfaces );
	cSurf = ( mdcSurface_t * )( (byte *)mdc + mdc->ofsSurfaces );
	for ( j = 0 ; j < md3->numSurfaces ; j++ ) {

		cSurf->ident = SF_MDC;
		Q_strncpyz( cSurf->name, surf->name, sizeof( cSurf->name ) );
		cSurf->flags = surf->flags;
		cSurf->numCompFrames = ( mdc->numFrames - numBaseFrames );
		cSurf->numBaseFrames = numBaseFrames;
		cSurf->numShaders = surf->numShaders;
		cSurf->numVerts = surf->numVerts;
		cSurf->numTriangles = surf->numTriangles;
		cSurf->ofsTriangles = sizeof( mdcSurface_t );
		cSurf->ofsShaders = cSurf->ofsTriangles + cSurf->numTriangles * sizeof( md3Triangle_t );
		cSurf->ofsSt = cSurf->ofsShaders + cSurf->numShaders * sizeof( md3Shader_t );
		cSurf->ofsXyzNormals = cSurf->ofsSt + cSurf->numVerts * sizeof( md3St_t );
		cSurf->ofsXyzCompressed = cSurf->ofsXyzNormals + cSurf->numVerts * numBaseFrames * sizeof( md3XyzNormal_t );
		cSurf->ofsFrameBaseFrames = cSurf->ofsXyzCompressed + cSurf->numVerts * ( mdc->numFrames - numBaseFrames ) * sizeof( mdcXyzCompressed_t );
		cSurf->ofsFrameCompFrames = cSurf->ofsFrameBaseFrames + mdc->numFrames * sizeof( short );
		cSurf->ofsEnd = cSurf->ofsFrameCompFrames + mdc->numFrames * sizeof( short );

		// triangles
		memcpy( (byte *)cSurf + cSurf->ofsTriangles, (byte *)surf + surf->ofsTriangles, cSurf->numTriangles * sizeof( md3Triangle_t ) );
		// shaders
		memcpy( (byte *)cSurf + cSurf->ofsShaders, (byte *)surf + surf->ofsShaders, cSurf->numShaders * sizeof( md3Shader_t ) );
		// st
		memcpy( (byte *)cSurf + cSurf->ofsSt, (byte *)surf + surf->ofsSt, cSurf->numVerts * sizeof( md3St_t ) );

		// rest
		frameBaseFrames = ( short * )( (byte *)cSurf + cSurf->ofsFrameBaseFrames );
		frameCompFrames = ( short * )( (byte *)cSurf + cSurf->ofsFrameCompFrames );
		for ( f = 0, i = 0, c = 0; f < md3->numFrames; f++ ) {
			if ( i < numBaseFrames && f == baseFrames[i] ) {
				// copy this baseFrame from the md3
				memcpy( (byte *)cSurf + cSurf->ofsXyzNormals + ( sizeof( md3XyzNormal_t ) * cSurf->numVerts * i ),
						(byte *)surf + surf->ofsXyzNormals + ( sizeof( md3XyzNormal_t ) * cSurf->numVerts * f ),
						sizeof( md3XyzNormal_t ) * cSurf->numVerts );
				i++;
				frameCompFrames[f] = -1;
				frameBaseFrames[f] = i - 1;
			} else {
				if ( !R_MDC_CompressSurfaceFrame( md3, surf, f, baseFrames[i - 1], ( mdcXyzCompressed_t * )( (byte *)cSurf + cSurf->ofsXyzCompressed + sizeof( mdcXyzCompressed_t ) * cSurf->numVerts * c ) ) ) {
					ri.Error( ERR_DROP, "R_MDC_ConvertMD3: tried to compress an unsuitable frame\n" );
				}
				frameCompFrames[f] = c;
				frameBaseFrames[f] = i - 1;
				c++;
			}
		}

		// find the next surface
		surf = ( md3Surface_t * )( (byte *)surf + surf->ofsEnd );
		cSurf = ( mdcSurface_t * )( (byte *)cSurf + cSurf->ofsEnd );
	}

	mod->type = MOD_MDC;

	// free allocated memory
	ri.Hunk_FreeTempMemory( baseFrames );

	// kill the md3 memory
	ri.Hunk_FreeTempMemory( md3 );
	mod->md3[lod] = NULL;

	return qtrue;
}
#endif // RTCW_XX

/*
=================
R_LoadMDC
=================
*/
static qboolean R_LoadMDC( model_t *mod, int lod, void *buffer, const char *mod_name ) {
	int i, j;
	mdcHeader_t         *pinmodel;
	md3Frame_t          *frame;
	mdcSurface_t        *surf;
	md3Shader_t         *shader;
	md3Triangle_t       *tri;
	md3St_t             *st;
	md3XyzNormal_t      *xyz;
	mdcXyzCompressed_t  *xyzComp;
	mdcTag_t            *tag;
	short               *ps;
	int version;
	int size;

	pinmodel = (mdcHeader_t *)buffer;

	version = rtcw::Endian::le( pinmodel->version );
	if ( version != MDC_VERSION ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDC: %s has wrong version (%i should be %i)\n",
				   mod_name, version, MDC_VERSION );
		return qfalse;
	}

	mod->type = MOD_MDC;
	size = rtcw::Endian::le( pinmodel->ofsEnd );
	mod->dataSize += size;

#if !defined RTCW_ET
	mod->mdc[lod] = static_cast<mdcHeader_t*> (ri.Hunk_Alloc( size, h_low ));

	memcpy( mod->mdc[lod], buffer, rtcw::Endian::le( pinmodel->ofsEnd ) );

	LL( mod->mdc[lod]->ident );
	LL( mod->mdc[lod]->version );
	LL( mod->mdc[lod]->numFrames );
	LL( mod->mdc[lod]->numTags );
	LL( mod->mdc[lod]->numSurfaces );
	LL( mod->mdc[lod]->ofsFrames );
	LL( mod->mdc[lod]->ofsTagNames );
	LL( mod->mdc[lod]->ofsTags );
	LL( mod->mdc[lod]->ofsSurfaces );
	LL( mod->mdc[lod]->ofsEnd );
	LL( mod->mdc[lod]->flags );
	LL( mod->mdc[lod]->numSkins );


	if ( mod->mdc[lod]->numFrames < 1 ) {
#else
	mod->model.mdc[lod] = static_cast<mdcHeader_t*> (ri.Hunk_Alloc( size, h_low ));

	memcpy( mod->model.mdc[lod], buffer, rtcw::Endian::le( pinmodel->ofsEnd ) );

	LL( mod->model.mdc[lod]->ident );
	LL( mod->model.mdc[lod]->version );
	LL( mod->model.mdc[lod]->numFrames );
	LL( mod->model.mdc[lod]->numTags );
	LL( mod->model.mdc[lod]->numSurfaces );
	LL( mod->model.mdc[lod]->ofsFrames );
	LL( mod->model.mdc[lod]->ofsTagNames );
	LL( mod->model.mdc[lod]->ofsTags );
	LL( mod->model.mdc[lod]->ofsSurfaces );
	LL( mod->model.mdc[lod]->ofsEnd );
	LL( mod->model.mdc[lod]->flags );
	LL( mod->model.mdc[lod]->numSkins );


	if ( mod->model.mdc[lod]->numFrames < 1 ) {
#endif // RTCW_XX

		ri.Printf( PRINT_WARNING, "R_LoadMDC: %s has no frames\n", mod_name );
		return qfalse;
	}

	// swap all the frames

#if !defined RTCW_ET
	frame = ( md3Frame_t * )( (byte *)mod->mdc[lod] + mod->mdc[lod]->ofsFrames );
	for ( i = 0 ; i < mod->mdc[lod]->numFrames ; i++, frame++ ) {
#else
	frame = ( md3Frame_t * )( (byte *)mod->model.mdc[lod] + mod->model.mdc[lod]->ofsFrames );
	for ( i = 0 ; i < mod->model.mdc[lod]->numFrames ; i++, frame++ ) {
#endif // RTCW_XX

		rtcw::Endian::lei(frame->radius);
		if ( strstr( mod->name,"sherman" ) || strstr( mod->name, "mg42" ) ) {
			frame->radius = 256;
			for ( j = 0 ; j < 3 ; j++ ) {
				frame->bounds[0][j] = 128;
				frame->bounds[1][j] = -128;
				frame->localOrigin[j] = rtcw::Endian::le( frame->localOrigin[j] );
			}
		} else
		{
			for ( j = 0 ; j < 3 ; j++ ) {
				frame->bounds[0][j] = rtcw::Endian::le( frame->bounds[0][j] );
				frame->bounds[1][j] = rtcw::Endian::le( frame->bounds[1][j] );
				frame->localOrigin[j] = rtcw::Endian::le( frame->localOrigin[j] );
			}
		}
	}

	// swap all the tags

#if !defined RTCW_ET
	tag = ( mdcTag_t * )( (byte *)mod->mdc[lod] + mod->mdc[lod]->ofsTags );
	if (!rtcw::Endian::is_little ()) {
		for ( i = 0 ; i < mod->mdc[lod]->numTags * mod->mdc[lod]->numFrames ; i++, tag++ ) {
#else
	tag = ( mdcTag_t * )( (byte *)mod->model.mdc[lod] + mod->model.mdc[lod]->ofsTags );
	if (!rtcw::Endian::is_little ()) {
		for ( i = 0 ; i < mod->model.mdc[lod]->numTags * mod->model.mdc[lod]->numFrames ; i++, tag++ ) {
#endif // RTCW_XX

			rtcw::Endian::lei(tag->xyz);
			rtcw::Endian::lei(tag->angles);
		}
	}

	// swap all the surfaces

#if !defined RTCW_ET
	surf = ( mdcSurface_t * )( (byte *)mod->mdc[lod] + mod->mdc[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->mdc[lod]->numSurfaces ; i++ ) {
#else
	surf = ( mdcSurface_t * )( (byte *)mod->model.mdc[lod] + mod->model.mdc[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->model.mdc[lod]->numSurfaces ; i++ ) {
#endif // RTCW_XX

		LL( surf->ident );
		LL( surf->flags );
		LL( surf->numBaseFrames );
		LL( surf->numCompFrames );
		LL( surf->numShaders );
		LL( surf->numTriangles );
		LL( surf->ofsTriangles );
		LL( surf->numVerts );
		LL( surf->ofsShaders );
		LL( surf->ofsSt );
		LL( surf->ofsXyzNormals );
		LL( surf->ofsXyzCompressed );
		LL( surf->ofsFrameBaseFrames );
		LL( surf->ofsFrameCompFrames );
		LL( surf->ofsEnd );

#if !defined RTCW_ET
		if ( surf->numVerts > SHADER_MAX_VERTEXES ) {
			ri.Error( ERR_DROP, "R_LoadMDC: %s has more than %i verts on a surface (%i)",
					  mod_name, SHADER_MAX_VERTEXES, surf->numVerts );
		}
		if ( surf->numTriangles * 3 > SHADER_MAX_INDEXES ) {
			ri.Error( ERR_DROP, "R_LoadMDC: %s has more than %i triangles on a surface (%i)",
					  mod_name, SHADER_MAX_INDEXES / 3, surf->numTriangles );
#else
		if ( surf->numVerts > tess.maxShaderVerts ) {
			ri.Error( ERR_DROP, "R_LoadMDC: %s has more than %i verts on a surface (%i)",
					  mod_name, tess.maxShaderVerts, surf->numVerts );
		}
		if ( surf->numTriangles * 3 > tess.maxShaderIndicies ) {
			ri.Error( ERR_DROP, "R_LoadMDC: %s has more than %i triangles on a surface (%i)",
					  mod_name, tess.maxShaderIndicies / 3, surf->numTriangles );
#endif // RTCW_XX

		}

		// change to surface identifier
		surf->ident = SF_MDC;

		// lowercase the surface name so skin compares are faster
		Q_strlwr( surf->name );

		// strip off a trailing _1 or _2
		// this is a crutch for q3data being a mess
		j = strlen( surf->name );
		if ( j > 2 && surf->name[j - 2] == '_' ) {
			surf->name[j - 2] = 0;
		}

		// register the shaders
		shader = ( md3Shader_t * )( (byte *)surf + surf->ofsShaders );
		for ( j = 0 ; j < surf->numShaders ; j++, shader++ ) {
			shader_t    *sh;

			sh = R_FindShader( shader->name, LIGHTMAP_NONE, qtrue );
			if ( sh->defaultShader ) {
				shader->shaderIndex = 0;
			} else {
				shader->shaderIndex = sh->index;
			}
		}

		// Ridah, optimization, only do the swapping if we really need to
		if (!rtcw::Endian::is_little ()) {

			// swap all the triangles
			tri = ( md3Triangle_t * )( (byte *)surf + surf->ofsTriangles );
			for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
				LL( tri->indexes[0] );
				LL( tri->indexes[1] );
				LL( tri->indexes[2] );
			}

			// swap all the ST
			st = ( md3St_t * )( (byte *)surf + surf->ofsSt );
			for ( j = 0 ; j < surf->numVerts ; j++, st++ ) {
				rtcw::Endian::lei(st->st);
			}

			// swap all the XyzNormals
			xyz = ( md3XyzNormal_t * )( (byte *)surf + surf->ofsXyzNormals );
			for ( j = 0 ; j < surf->numVerts * surf->numBaseFrames ; j++, xyz++ )
			{
				rtcw::Endian::lei(xyz->xyz);
				rtcw::Endian::lei(xyz->normal);
			}

			// swap all the XyzCompressed
			xyzComp = ( mdcXyzCompressed_t * )( (byte *)surf + surf->ofsXyzCompressed );
			for ( j = 0 ; j < surf->numVerts * surf->numCompFrames ; j++, xyzComp++ )
			{
				LL( xyzComp->ofsVec );
			}

			// swap the frameBaseFrames
			ps = ( short * )( (byte *)surf + surf->ofsFrameBaseFrames );

#if !defined RTCW_ET
			for ( j = 0; j < mod->mdc[lod]->numFrames; j++, ps++ )
#else
			for ( j = 0; j < mod->model.mdc[lod]->numFrames; j++, ps++ )
#endif // RTCW_XX

			{
				rtcw::Endian::lei(*ps);
			}

			// swap the frameCompFrames
			ps = ( short * )( (byte *)surf + surf->ofsFrameCompFrames );

#if !defined RTCW_ET
			for ( j = 0; j < mod->mdc[lod]->numFrames; j++, ps++ )
#else
			for ( j = 0; j < mod->model.mdc[lod]->numFrames; j++, ps++ )
#endif // RTCW_XX

			{
				rtcw::Endian::lei(*ps);
			}
		}
		// done.

		// find the next surface
		surf = ( mdcSurface_t * )( (byte *)surf + surf->ofsEnd );
	}

	return qtrue;
}

// done.
//-------------------------------------------------------------------------------

/*
=================
R_LoadMD3
=================
*/
static qboolean R_LoadMD3( model_t *mod, int lod, void *buffer, const char *mod_name ) {
	int i, j;
	md3Header_t         *pinmodel;
	md3Frame_t          *frame;
	md3Surface_t        *surf;
	md3Shader_t         *shader;
	md3Triangle_t       *tri;
	md3St_t             *st;
	md3XyzNormal_t      *xyz;
	md3Tag_t            *tag;
	int version;
	int size;
	qboolean fixRadius = qfalse;

	pinmodel = (md3Header_t *)buffer;

	version = rtcw::Endian::le( pinmodel->version );
	if ( version != MD3_VERSION ) {
		ri.Printf( PRINT_WARNING, "R_LoadMD3: %s has wrong version (%i should be %i)\n",
				   mod_name, version, MD3_VERSION );
		return qfalse;
	}

	mod->type = MOD_MESH;
	size = rtcw::Endian::le( pinmodel->ofsEnd );
	mod->dataSize += size;

#if !defined RTCW_ET
	// Ridah, convert to compressed format
	if ( !r_compressModels->integer ) {
		mod->md3[lod] = static_cast<md3Header_t*> (ri.Hunk_Alloc( size, h_low ));
	} else {
		mod->md3[lod] = static_cast<md3Header_t*> (ri.Hunk_AllocateTempMemory( size ));
	}
	// done.

	memcpy( mod->md3[lod], buffer, rtcw::Endian::le( pinmodel->ofsEnd ) );

	LL( mod->md3[lod]->ident );
	LL( mod->md3[lod]->version );
	LL( mod->md3[lod]->numFrames );
	LL( mod->md3[lod]->numTags );
	LL( mod->md3[lod]->numSurfaces );
	LL( mod->md3[lod]->ofsFrames );
	LL( mod->md3[lod]->ofsTags );
	LL( mod->md3[lod]->ofsSurfaces );
	LL( mod->md3[lod]->ofsEnd );

	if ( mod->md3[lod]->numFrames < 1 ) {
#else
	mod->model.md3[lod] = static_cast<md3Header_t*> (ri.Hunk_Alloc( size, h_low ));

	memcpy( mod->model.md3[lod], buffer, rtcw::Endian::le( pinmodel->ofsEnd ) );

	LL( mod->model.md3[lod]->ident );
	LL( mod->model.md3[lod]->version );
	LL( mod->model.md3[lod]->numFrames );
	LL( mod->model.md3[lod]->numTags );
	LL( mod->model.md3[lod]->numSurfaces );
	LL( mod->model.md3[lod]->ofsFrames );
	LL( mod->model.md3[lod]->ofsTags );
	LL( mod->model.md3[lod]->ofsSurfaces );
	LL( mod->model.md3[lod]->ofsEnd );

	if ( mod->model.md3[lod]->numFrames < 1 ) {
#endif // RTCW_XX

		ri.Printf( PRINT_WARNING, "R_LoadMD3: %s has no frames\n", mod_name );
		return qfalse;
	}

	if ( strstr( mod->name,"sherman" ) || strstr( mod->name, "mg42" ) ) {
		fixRadius = qtrue;
	}

	// swap all the frames

#if !defined RTCW_ET
	frame = ( md3Frame_t * )( (byte *)mod->md3[lod] + mod->md3[lod]->ofsFrames );
	for ( i = 0 ; i < mod->md3[lod]->numFrames ; i++, frame++ ) {
#else
	frame = ( md3Frame_t * )( (byte *)mod->model.md3[lod] + mod->model.md3[lod]->ofsFrames );
	for ( i = 0 ; i < mod->model.md3[lod]->numFrames ; i++, frame++ ) {
#endif // RTCW_XX

		rtcw::Endian::lei(frame->radius);
		if ( fixRadius ) {
			frame->radius = 256;
			for ( j = 0 ; j < 3 ; j++ ) {
				frame->bounds[0][j] = 128;
				frame->bounds[1][j] = -128;
				frame->localOrigin[j] = rtcw::Endian::le( frame->localOrigin[j] );
			}
		}
		// Hack for Bug using plugin generated model
		else if ( frame->radius == 1 ) {
			frame->radius = 256;
			for ( j = 0 ; j < 3 ; j++ ) {
				frame->bounds[0][j] = 128;
				frame->bounds[1][j] = -128;
				frame->localOrigin[j] = rtcw::Endian::le( frame->localOrigin[j] );
			}
		} else
		{
			rtcw::Endian::lei(frame->bounds[0]);
			rtcw::Endian::lei(frame->bounds[1]);
			rtcw::Endian::lei(frame->localOrigin);
		}
	}

	// swap all the tags

#if !defined RTCW_ET
	tag = ( md3Tag_t * )( (byte *)mod->md3[lod] + mod->md3[lod]->ofsTags );
	for ( i = 0 ; i < mod->md3[lod]->numTags * mod->md3[lod]->numFrames ; i++, tag++ ) {
#else
	tag = ( md3Tag_t * )( (byte *)mod->model.md3[lod] + mod->model.md3[lod]->ofsTags );
	for ( i = 0 ; i < mod->model.md3[lod]->numTags * mod->model.md3[lod]->numFrames ; i++, tag++ ) {
#endif // RTCW_XX

		rtcw::Endian::lei(tag->origin);
		rtcw::Endian::lei(tag->axis[0]);
		rtcw::Endian::lei(tag->axis[1]);
		rtcw::Endian::lei(tag->axis[2]);
	}

	// swap all the surfaces

#if !defined RTCW_ET
	surf = ( md3Surface_t * )( (byte *)mod->md3[lod] + mod->md3[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->md3[lod]->numSurfaces ; i++ ) {
#else
	surf = ( md3Surface_t * )( (byte *)mod->model.md3[lod] + mod->model.md3[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->model.md3[lod]->numSurfaces ; i++ ) {
#endif // RTCW_XX

		LL( surf->ident );
		LL( surf->flags );
		LL( surf->numFrames );
		LL( surf->numShaders );
		LL( surf->numTriangles );
		LL( surf->ofsTriangles );
		LL( surf->numVerts );
		LL( surf->ofsShaders );
		LL( surf->ofsSt );
		LL( surf->ofsXyzNormals );
		LL( surf->ofsEnd );

#if !defined RTCW_ET
		if ( surf->numVerts > SHADER_MAX_VERTEXES ) {
			ri.Error( ERR_DROP, "R_LoadMD3: %s has more than %i verts on a surface (%i)",
					  mod_name, SHADER_MAX_VERTEXES, surf->numVerts );
		}
		if ( surf->numTriangles * 3 > SHADER_MAX_INDEXES ) {
			ri.Error( ERR_DROP, "R_LoadMD3: %s has more than %i triangles on a surface (%i)",
					  mod_name, SHADER_MAX_INDEXES / 3, surf->numTriangles );
#else
		if ( surf->numVerts > tess.maxShaderVerts ) {
			ri.Error( ERR_DROP, "R_LoadMD3: %s has more than %i verts on a surface (%i)",
					  mod_name, tess.maxShaderVerts, surf->numVerts );
		}
		if ( surf->numTriangles * 3 > tess.maxShaderIndicies ) {
			ri.Error( ERR_DROP, "R_LoadMD3: %s has more than %i triangles on a surface (%i)",
					  mod_name, tess.maxShaderIndicies / 3, surf->numTriangles );
#endif // RTCW_XX

		}

		// change to surface identifier
		surf->ident = SF_MD3;

		// lowercase the surface name so skin compares are faster
		Q_strlwr( surf->name );

		// strip off a trailing _1 or _2
		// this is a crutch for q3data being a mess
		j = strlen( surf->name );
		if ( j > 2 && surf->name[j - 2] == '_' ) {
			surf->name[j - 2] = 0;
		}

		// register the shaders
		shader = ( md3Shader_t * )( (byte *)surf + surf->ofsShaders );
		for ( j = 0 ; j < surf->numShaders ; j++, shader++ ) {
			shader_t    *sh;

			sh = R_FindShader( shader->name, LIGHTMAP_NONE, qtrue );
			if ( sh->defaultShader ) {
				shader->shaderIndex = 0;
			} else {
				shader->shaderIndex = sh->index;
			}
		}

		// Ridah, optimization, only do the swapping if we really need to
		if (!rtcw::Endian::is_little ()) {

			// swap all the triangles
			tri = ( md3Triangle_t * )( (byte *)surf + surf->ofsTriangles );
			for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
				LL( tri->indexes[0] );
				LL( tri->indexes[1] );
				LL( tri->indexes[2] );
			}

			// swap all the ST
			st = ( md3St_t * )( (byte *)surf + surf->ofsSt );
			for ( j = 0 ; j < surf->numVerts ; j++, st++ )
				rtcw::Endian::lei(st->st);

			// swap all the XyzNormals
			xyz = ( md3XyzNormal_t * )( (byte *)surf + surf->ofsXyzNormals );
			for ( j = 0 ; j < surf->numVerts * surf->numFrames ; j++, xyz++ )
			{
				rtcw::Endian::lei(xyz->xyz);
				rtcw::Endian::lei(xyz->normal);
			}

		}
		// done.

		// find the next surface
		surf = ( md3Surface_t * )( (byte *)surf + surf->ofsEnd );
	}

#if !defined RTCW_ET
	// Ridah, convert to compressed format
	if ( r_compressModels->integer ) {
		R_MDC_ConvertMD3( mod, lod, mod_name );
	}
	// done.
#endif // RTCW_XX

	return qtrue;
}



/*
=================
R_LoadMDS
=================
*/
static qboolean R_LoadMDS( model_t *mod, void *buffer, const char *mod_name ) {
	int i, j, k;
	mdsHeader_t         *pinmodel, *mds;
	mdsFrame_t          *frame;
	mdsSurface_t        *surf;
	mdsTriangle_t       *tri;
	mdsVertex_t         *v;
	mdsBoneInfo_t       *bi;
	mdsTag_t            *tag;
	int version;
	int size;
	shader_t            *sh;
	int frameSize;
	int                 *collapseMap, *boneref;

	pinmodel = (mdsHeader_t *)buffer;

	version = rtcw::Endian::le( pinmodel->version );
	if ( version != MDS_VERSION ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDS: %s has wrong version (%i should be %i)\n",
				   mod_name, version, MDS_VERSION );
		return qfalse;
	}

	mod->type = MOD_MDS;
	size = rtcw::Endian::le( pinmodel->ofsEnd );
	mod->dataSize += size;

#if !defined RTCW_ET
	mds = mod->mds = static_cast<mdsHeader_t*> (ri.Hunk_Alloc( size, h_low ));
#else
	mds = mod->model.mds = static_cast<mdsHeader_t*> (ri.Hunk_Alloc( size, h_low ));
#endif // RTCW_XX

	memcpy( mds, buffer, rtcw::Endian::le( pinmodel->ofsEnd ) );

	LL( mds->ident );
	LL( mds->version );
	LL( mds->numFrames );
	LL( mds->numBones );
	LL( mds->numTags );
	LL( mds->numSurfaces );
	LL( mds->ofsFrames );
	LL( mds->ofsBones );
	LL( mds->ofsTags );
	LL( mds->ofsEnd );
	LL( mds->ofsSurfaces );
	rtcw::Endian::lei(mds->lodBias);
	rtcw::Endian::lei(mds->lodScale);
	LL( mds->torsoParent );

	if ( mds->numFrames < 1 ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDS: %s has no frames\n", mod_name );
		return qfalse;
	}

	if (!rtcw::Endian::is_little ()) {
		// swap all the frames
		//frameSize = (int)( &((mdsFrame_t *)0)->bones[ mds->numBones ] );
		frameSize = (int) ( sizeof( mdsFrame_t ) - sizeof( mdsBoneFrameCompressed_t ) + mds->numBones * sizeof( mdsBoneFrameCompressed_t ) );
		for ( i = 0 ; i < mds->numFrames ; i++, frame++ ) {
			frame = ( mdsFrame_t * )( (byte *)mds + mds->ofsFrames + i * frameSize );
			rtcw::Endian::lei(frame->radius);
			rtcw::Endian::lei(frame->bounds[0]);
			rtcw::Endian::lei(frame->bounds[1]);
			rtcw::Endian::lei(frame->localOrigin);
			rtcw::Endian::lei(frame->parentOffset);

			for ( j = 0 ; j < mds->numBones * sizeof( mdsBoneFrameCompressed_t ) / sizeof( short ) ; j++ ) {
				( (short *)frame->bones )[j] = rtcw::Endian::le( ( (short *)frame->bones )[j] );
			}
		}

		// swap all the tags
		tag = ( mdsTag_t * )( (byte *)mds + mds->ofsTags );
		for ( i = 0 ; i < mds->numTags ; i++, tag++ ) {
			LL( tag->boneIndex );
			rtcw::Endian::lei(tag->torsoWeight);
		}

		// swap all the bones
		for ( i = 0 ; i < mds->numBones ; i++, bi++ ) {
			bi = ( mdsBoneInfo_t * )( (byte *)mds + mds->ofsBones + i * sizeof( mdsBoneInfo_t ) );
			LL( bi->parent );
			rtcw::Endian::lei(bi->torsoWeight);
			rtcw::Endian::lei(bi->parentDist);
			LL( bi->flags );
		}
	}

	// swap all the surfaces
	surf = ( mdsSurface_t * )( (byte *)mds + mds->ofsSurfaces );
	for ( i = 0 ; i < mds->numSurfaces ; i++ ) {
		if (!rtcw::Endian::is_little ()) {

#if !defined RTCW_ET
			LL( surf->ident );
#else
			//LL(surf->ident);
#endif // RTCW_XX

			LL( surf->shaderIndex );
			LL( surf->minLod );
			LL( surf->ofsHeader );
			LL( surf->ofsCollapseMap );
			LL( surf->numTriangles );
			LL( surf->ofsTriangles );
			LL( surf->numVerts );
			LL( surf->ofsVerts );
			LL( surf->numBoneReferences );
			LL( surf->ofsBoneReferences );
			LL( surf->ofsEnd );
		}

#if !defined RTCW_ET
		if ( surf->numVerts > SHADER_MAX_VERTEXES ) {
			ri.Error( ERR_DROP, "R_LoadMDS: %s has more than %i verts on a surface (%i)",
					  mod_name, SHADER_MAX_VERTEXES, surf->numVerts );
		}
		if ( surf->numTriangles * 3 > SHADER_MAX_INDEXES ) {
			ri.Error( ERR_DROP, "R_LoadMDS: %s has more than %i triangles on a surface (%i)",
					  mod_name, SHADER_MAX_INDEXES / 3, surf->numTriangles );
#else
		// change to surface identifier
		surf->ident = SF_MDS;

		if ( surf->numVerts > tess.maxShaderVerts ) {
			ri.Error( ERR_DROP, "R_LoadMDS: %s has more than %i verts on a surface (%i)",
					  mod_name, tess.maxShaderVerts, surf->numVerts );
		}
		if ( surf->numTriangles * 3 > tess.maxShaderIndicies ) {
			ri.Error( ERR_DROP, "R_LoadMDS: %s has more than %i triangles on a surface (%i)",
					  mod_name, tess.maxShaderIndicies / 3, surf->numTriangles );
#endif // RTCW_XX

		}

		// register the shaders
		if ( surf->shader[0] ) {
			sh = R_FindShader( surf->shader, LIGHTMAP_NONE, qtrue );
			if ( sh->defaultShader ) {
				surf->shaderIndex = 0;
			} else {
				surf->shaderIndex = sh->index;
			}
		} else {
			surf->shaderIndex = 0;
		}

		if (!rtcw::Endian::is_little ()) {
			// swap all the triangles
			tri = ( mdsTriangle_t * )( (byte *)surf + surf->ofsTriangles );
			for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
				LL( tri->indexes[0] );
				LL( tri->indexes[1] );
				LL( tri->indexes[2] );
			}

			// swap all the vertexes
			v = ( mdsVertex_t * )( (byte *)surf + surf->ofsVerts );
			for ( j = 0 ; j < surf->numVerts ; j++ ) {
				rtcw::Endian::lei(v->normal);
				rtcw::Endian::lei(v->texCoords);

				rtcw::Endian::lei(v->numWeights);

				for ( k = 0 ; k < v->numWeights ; k++ ) {
					rtcw::Endian::lei(v->weights[k].boneIndex);
					rtcw::Endian::lei(v->weights[k].boneWeight);
					rtcw::Endian::lei(v->weights[k].offset);
				}

#if !defined RTCW_SP
				// find the fixedParent for this vert (if exists)
				v->fixedParent = -1;
				if ( v->numWeights == 2 ) {
					// find the closest parent
					if ( VectorLength( v->weights[0].offset ) < VectorLength( v->weights[1].offset ) ) {
						v->fixedParent = 0;
					} else {
						v->fixedParent = 1;
					}
					v->fixedDist = VectorLength( v->weights[v->fixedParent].offset );
				}
#endif // RTCW_XX

				v = (mdsVertex_t *)&v->weights[v->numWeights];
			}

			// swap the collapse map
			collapseMap = ( int * )( (byte *)surf + surf->ofsCollapseMap );
			for ( j = 0; j < surf->numVerts; j++, collapseMap++ ) {
				rtcw::Endian::lei(*collapseMap);
			}

			// swap the bone references
			boneref = ( int * )( ( byte *)surf + surf->ofsBoneReferences );
			for ( j = 0; j < surf->numBoneReferences; j++, boneref++ ) {
				rtcw::Endian::lei(*boneref);
			}
		}

		// find the next surface
		surf = ( mdsSurface_t * )( (byte *)surf + surf->ofsEnd );
	}

	return qtrue;
}

#if defined RTCW_ET
/*
=================
R_LoadMDM
=================
*/
static qboolean R_LoadMDM( model_t *mod, void *buffer, const char *mod_name ) {
	int i, j, k;
	mdmHeader_t         *pinmodel, *mdm;
//    mdmFrame_t			*frame;
	mdmSurface_t        *surf;
	mdmTriangle_t       *tri;
	mdmVertex_t         *v;
	mdmTag_t            *tag;
	int version;
	int size;
	shader_t            *sh;
	int                 *collapseMap, *boneref;

	pinmodel = (mdmHeader_t *)buffer;

	version = rtcw::Endian::le( pinmodel->version );
	if ( version != MDM_VERSION ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDM: %s has wrong version (%i should be %i)\n",
				   mod_name, version, MDM_VERSION );
		return qfalse;
	}

	mod->type = MOD_MDM;
	size = rtcw::Endian::le( pinmodel->ofsEnd );
	mod->dataSize += size;
	mdm = mod->model.mdm = static_cast<mdmHeader_t*> (ri.Hunk_Alloc( size, h_low ));

	memcpy( mdm, buffer, rtcw::Endian::le( pinmodel->ofsEnd ) );

	LL( mdm->ident );
	LL( mdm->version );
//    LL(mdm->numFrames);
	LL( mdm->numTags );
	LL( mdm->numSurfaces );
//    LL(mdm->ofsFrames);
	LL( mdm->ofsTags );
	LL( mdm->ofsEnd );
	LL( mdm->ofsSurfaces );
	rtcw::Endian::lei(mdm->lodBias);
	rtcw::Endian::lei(mdm->lodScale);

/*	mdm->skel = RE_RegisterModel(mdm->bonesfile);
	if ( !mdm->skel ) {
		ri.Error (ERR_DROP, "R_LoadMDM: %s skeleton not found\n", mdm->bonesfile );
	}

	if ( mdm->numFrames < 1 ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDM: %s has no frames\n", mod_name );
		return qfalse;
	}*/

	if (!rtcw::Endian::is_little ()) {
		// swap all the frames
		/*frameSize = (int) ( sizeof( mdmFrame_t ) );
		for ( i = 0 ; i < mdm->numFrames ; i++, frame++) {
			frame = (mdmFrame_t *) ( (byte *)mdm + mdm->ofsFrames + i * frameSize );
			frame->radius = LittleFloat( frame->radius );
			for ( j = 0 ; j < 3 ; j++ ) {
				frame->bounds[0][j] = LittleFloat( frame->bounds[0][j] );
				frame->bounds[1][j] = LittleFloat( frame->bounds[1][j] );
				frame->localOrigin[j] = LittleFloat( frame->localOrigin[j] );
				frame->parentOffset[j] = LittleFloat( frame->parentOffset[j] );
			}
		}*/

		// swap all the tags
		tag = ( mdmTag_t * )( (byte *)mdm + mdm->ofsTags );
		for ( i = 0 ; i < mdm->numTags ; i++ ) {
			int ii;
			rtcw::Endian::lei(tag->axis[ii]);

			LL( tag->boneIndex );
			//tag->torsoWeight = LittleFloat( tag->torsoWeight );
			rtcw::Endian::lei(tag->offset);

			LL( tag->numBoneReferences );
			LL( tag->ofsBoneReferences );
			LL( tag->ofsEnd );

			// swap the bone references
			boneref = ( int * )( ( byte *)tag + tag->ofsBoneReferences );
			for ( j = 0; j < tag->numBoneReferences; j++, boneref++ ) {
				rtcw::Endian::lei(*boneref);
			}

			// find the next tag
			tag = ( mdmTag_t * )( (byte *)tag + tag->ofsEnd );
		}
	}

	// swap all the surfaces
	surf = ( mdmSurface_t * )( (byte *)mdm + mdm->ofsSurfaces );
	for ( i = 0 ; i < mdm->numSurfaces ; i++ ) {
		if (!rtcw::Endian::is_little ()) {
			//LL(surf->ident);
			LL( surf->shaderIndex );
			LL( surf->minLod );
			LL( surf->ofsHeader );
			LL( surf->ofsCollapseMap );
			LL( surf->numTriangles );
			LL( surf->ofsTriangles );
			LL( surf->numVerts );
			LL( surf->ofsVerts );
			LL( surf->numBoneReferences );
			LL( surf->ofsBoneReferences );
			LL( surf->ofsEnd );
		}

		// change to surface identifier
		surf->ident = SF_MDM;

		if ( surf->numVerts > tess.maxShaderVerts ) {
			ri.Error( ERR_DROP, "R_LoadMDM: %s has more than %i verts on a surface (%i)",
					  mod_name, tess.maxShaderVerts, surf->numVerts );
		}
		if ( surf->numTriangles * 3 > tess.maxShaderIndicies ) {
			ri.Error( ERR_DROP, "R_LoadMDM: %s has more than %i triangles on a surface (%i)",
					  mod_name, tess.maxShaderIndicies / 3, surf->numTriangles );
		}

		// register the shaders
		if ( surf->shader[0] ) {
			sh = R_FindShader( surf->shader, LIGHTMAP_NONE, qtrue );
			if ( sh->defaultShader ) {
				surf->shaderIndex = 0;
			} else {
				surf->shaderIndex = sh->index;
			}
		} else {
			surf->shaderIndex = 0;
		}

		if (!rtcw::Endian::is_little ()) {
			// swap all the triangles
			tri = ( mdmTriangle_t * )( (byte *)surf + surf->ofsTriangles );
			for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
				LL( tri->indexes[0] );
				LL( tri->indexes[1] );
				LL( tri->indexes[2] );
			}

			// swap all the vertexes
			v = ( mdmVertex_t * )( (byte *)surf + surf->ofsVerts );
			for ( j = 0 ; j < surf->numVerts ; j++ ) {
				rtcw::Endian::lei(v->normal);
				rtcw::Endian::lei(v->texCoords);
				rtcw::Endian::lei(v->numWeights);

				for ( k = 0 ; k < v->numWeights ; k++ ) {
					rtcw::Endian::lei(v->weights[k].boneIndex);
					rtcw::Endian::lei(v->weights[k].boneWeight);
					rtcw::Endian::lei(v->weights[k].offset);
				}

				v = (mdmVertex_t *)&v->weights[v->numWeights];
			}

			// swap the collapse map
			collapseMap = ( int * )( (byte *)surf + surf->ofsCollapseMap );
			for ( j = 0; j < surf->numVerts; j++, collapseMap++ ) {
				rtcw::Endian::lei(*collapseMap);
			}

			// swap the bone references
			boneref = ( int * )( ( byte *)surf + surf->ofsBoneReferences );
			for ( j = 0; j < surf->numBoneReferences; j++, boneref++ ) {
				rtcw::Endian::lei(*boneref);
			}
		}

		// find the next surface
		surf = ( mdmSurface_t * )( (byte *)surf + surf->ofsEnd );
	}

	return qtrue;
}

/*
=================
R_LoadMDX
=================
*/
static qboolean R_LoadMDX( model_t *mod, void *buffer, const char *mod_name ) {
	int i, j;
	mdxHeader_t                 *pinmodel, *mdx;
	mdxFrame_t                  *frame;
	short                       *bframe;
	mdxBoneInfo_t               *bi;
	int version;
	int size;
	int frameSize;

	pinmodel = (mdxHeader_t *)buffer;

	version = rtcw::Endian::le( pinmodel->version );
	if ( version != MDX_VERSION ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDX: %s has wrong version (%i should be %i)\n",
				   mod_name, version, MDX_VERSION );
		return qfalse;
	}

	mod->type = MOD_MDX;
	size = rtcw::Endian::le( pinmodel->ofsEnd );
	mod->dataSize += size;
	mdx = mod->model.mdx = static_cast<mdxHeader_t*> (ri.Hunk_Alloc( size, h_low ));

	memcpy( mdx, buffer, rtcw::Endian::le( pinmodel->ofsEnd ) );

	LL( mdx->ident );
	LL( mdx->version );
	LL( mdx->numFrames );
	LL( mdx->numBones );
	LL( mdx->ofsFrames );
	LL( mdx->ofsBones );
	LL( mdx->ofsEnd );
	LL( mdx->torsoParent );

	if (!rtcw::Endian::is_little ()) {
		// swap all the frames
		frameSize = (int) ( sizeof( mdxBoneFrameCompressed_t ) ) * mdx->numBones;
		for ( i = 0 ; i < mdx->numFrames ; i++ ) {
			frame = ( mdxFrame_t * )( (byte *)mdx + mdx->ofsFrames + i * frameSize + i * sizeof( mdxFrame_t ) );
			rtcw::Endian::lei(frame->radius);
			rtcw::Endian::lei(frame->bounds[0]);
			rtcw::Endian::lei(frame->bounds[1]);
			rtcw::Endian::lei(frame->localOrigin);
			rtcw::Endian::lei(frame->parentOffset);

			bframe = ( short * )( (byte *)mdx + mdx->ofsFrames + i * frameSize + ( ( i + 1 ) * sizeof( mdxFrame_t ) ) );
			for ( j = 0 ; j < mdx->numBones * sizeof( mdxBoneFrameCompressed_t ) / sizeof( short ) ; j++ ) {
				( (short *)bframe )[j] = rtcw::Endian::le( ( (short *)bframe )[j] );
			}
		}

		// swap all the bones
		for ( i = 0 ; i < mdx->numBones ; i++ ) {
			bi = ( mdxBoneInfo_t * )( (byte *)mdx + mdx->ofsBones + i * sizeof( mdxBoneInfo_t ) );
			LL( bi->parent );
			rtcw::Endian::lei(bi->torsoWeight);
			rtcw::Endian::lei(bi->parentDist);
			LL( bi->flags );
		}
	}

	return qtrue;
}
#endif // RTCW_XX



//=============================================================================

/*
** RE_BeginRegistration
*/
void RE_BeginRegistration( glconfig_t *glconfigOut ) {
	ri.Hunk_Clear();    // (SA) MEM NOTE: not in missionpack

	R_Init();
	*glconfigOut = glConfig;

	R_SyncRenderThread();

	tr.viewCluster = -1;        // force markleafs to regenerate
	R_ClearFlares();
	RE_ClearScene();

	tr.registered = qtrue;

	// NOTE: this sucks, for some reason the first stretch pic is never drawn
	// without this we'd see a white flash on a level load because the very
	// first time the level shot would not be drawn
	RE_StretchPic( 0, 0, 0, 0, 0, 0, 1, 1, 0 );
}

/*
===============
R_ModelInit
===============
*/
void R_ModelInit( void ) {
	model_t     *mod;

	// leave a space for NULL model
	tr.numModels = 0;

	mod = R_AllocModel();
	mod->type = MOD_BAD;

	// Ridah, load in the cacheModels
	R_LoadCacheModels();
	// done.
}


/*
================
R_Modellist_f
================
*/

void R_Modellist_f( void ) {
	int i, j;
	model_t *mod;
	int total;
	int lods;

	total = 0;
	for ( i = 1 ; i < tr.numModels; i++ ) {
		mod = tr.models[i];
		lods = 1;
		for ( j = 1 ; j < MD3_MAX_LODS ; j++ ) {

#if !defined RTCW_ET
			if ( mod->md3[j] && mod->md3[j] != mod->md3[j - 1] ) {
#else
			if ( mod->model.md3[j] && mod->model.md3[j] != mod->model.md3[j - 1] ) {
#endif // RTCW_XX

				lods++;
			}
		}
		ri.Printf( PRINT_ALL, "%8i : (%i) %s\n",mod->dataSize, lods, mod->name );
		total += mod->dataSize;
	}
	ri.Printf( PRINT_ALL, "%8i : Total models\n", total );

#if 0       // not working right with new hunk
	if ( tr.world ) {
		ri.Printf( PRINT_ALL, "\n%8i : %s\n", tr.world->dataSize, tr.world->name );
	}
#endif
}


//=============================================================================


/*
================
R_GetTag
================
*/
static int R_GetTag( byte *mod, int frame, const char *tagName, int startTagIndex, md3Tag_t **outTag ) {
	md3Tag_t        *tag;
	int i;
	md3Header_t     *md3;

	md3 = (md3Header_t *) mod;

	if ( frame >= md3->numFrames ) {
		// it is possible to have a bad frame while changing models, so don't error
		frame = md3->numFrames - 1;
	}

	if ( startTagIndex > md3->numTags ) {
		*outTag = NULL;
		return -1;
	}

	tag = ( md3Tag_t * )( (byte *)mod + md3->ofsTags ) + frame * md3->numTags;
	for ( i = 0 ; i < md3->numTags ; i++, tag++ ) {
		if ( ( i >= startTagIndex ) && !strcmp( tag->name, tagName ) ) {

			// if we are looking for an indexed tag, wait until we find the correct number of matches
			//if (startTagIndex) {
			//	startTagIndex--;
			//	continue;
			//}

			*outTag = tag;
			return i;   // found it
		}
	}

	*outTag = NULL;
	return -1;
}

/*
================
R_GetMDCTag
================
*/
static int R_GetMDCTag( byte *mod, int frame, const char *tagName, int startTagIndex, mdcTag_t **outTag ) {
	mdcTag_t        *tag;
	mdcTagName_t    *pTagName;
	int i;
	mdcHeader_t     *mdc;

	mdc = (mdcHeader_t *) mod;

	if ( frame >= mdc->numFrames ) {
		// it is possible to have a bad frame while changing models, so don't error
		frame = mdc->numFrames - 1;
	}

	if ( startTagIndex > mdc->numTags ) {
		*outTag = NULL;
		return -1;
	}

	pTagName = ( mdcTagName_t * )( (byte *)mod + mdc->ofsTagNames );
	for ( i = 0 ; i < mdc->numTags ; i++, pTagName++ ) {
		if ( ( i >= startTagIndex ) && !strcmp( pTagName->name, tagName ) ) {
			break;  // found it
		}
	}

	if ( i >= mdc->numTags ) {
		*outTag = NULL;
		return -1;
	}

	tag = ( mdcTag_t * )( (byte *)mod + mdc->ofsTags ) + frame * mdc->numTags + i;
	*outTag = tag;
	return i;
}

/*
================
R_GetMDSTag
================
*/
// TTimo: unused
/*
static int R_GetMDSTag( byte *mod, const char *tagName, int startTagIndex, mdsTag_t **outTag ) {
	mdsTag_t		*tag;
	int				i;
	mdsHeader_t		*mds;

	mds = (mdsHeader_t *) mod;

	if (startTagIndex > mds->numTags) {
		*outTag = NULL;
		return -1;
	}

	tag = (mdsTag_t *)((byte *)mod + mds->ofsTags);
	for ( i = 0 ; i < mds->numTags ; i++ ) {
		if ( (i >= startTagIndex) && !strcmp( tag->name, tagName ) ) {
			break;	// found it
		}

		tag = (mdsTag_t *) ((byte *)tag + sizeof(mdsTag_t) - sizeof(mdsBoneFrameCompressed_t) + mds->numFrames * sizeof(mdsBoneFrameCompressed_t) );
	}

	if (i >= mds->numTags) {
		*outTag = NULL;
		return -1;
	}

	*outTag = tag;
	return i;
}
*/

/*
================
R_LerpTag

  returns the index of the tag it found, for cycling through tags with the same name
================
*/
int R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagNameIn, int startIndex ) {
	md3Tag_t    *start, *end;
	md3Tag_t ustart, uend;
	int i;
	float frontLerp, backLerp;
	model_t     *model;
	vec3_t sangles, eangles;
	char tagName[MAX_QPATH];       //, *ch;
	int retval;
	qhandle_t handle;
	int startFrame, endFrame;
	float frac;

	handle = refent->hModel;
	startFrame = refent->oldframe;
	endFrame = refent->frame;
	frac = 1.0 - refent->backlerp;

	Q_strncpyz( tagName, tagNameIn, MAX_QPATH );
/*
	// if the tagName has a space in it, then it is passing through the starting tag number
	if (ch = strrchr(tagName, ' ')) {
		*ch = 0;
		ch++;
		startIndex = atoi(ch);
	}
*/
	model = R_GetModelByHandle( handle );

#if !defined RTCW_ET
	if ( !model->md3[0] && !model->mdc[0] && !model->mds ) {
#else
	if ( !model->model.md3[0] && !model->model.mdc[0] && !model->model.mds ) {
#endif // RTCW_XX

		AxisClear( tag->axis );
		VectorClear( tag->origin );
		return -1;
	}

	frontLerp = frac;
	backLerp = 1.0 - frac;

	if ( model->type == MOD_MESH ) {
		// old MD3 style

#if !defined RTCW_ET
		retval = R_GetTag( (byte *)model->md3[0], startFrame, tagName, startIndex, &start );
		retval = R_GetTag( (byte *)model->md3[0], endFrame, tagName, startIndex, &end );
#else
		retval = R_GetTag( (byte *)model->model.md3[0], startFrame, tagName, startIndex, &start );
		retval = R_GetTag( (byte *)model->model.md3[0], endFrame, tagName, startIndex, &end );
#endif // RTCW_XX

	} else if ( model->type == MOD_MDS ) {    // use bone lerping

#if !defined RTCW_ET
		retval = R_GetBoneTag( tag, model->mds, startIndex, refent, tagNameIn );
#else
		retval = R_GetBoneTag( tag, model->model.mds, startIndex, refent, tagNameIn );
#endif // RTCW_XX

#if defined RTCW_ET
		if ( retval >= 0 ) {
			return retval;
		}

		// failed
		return -1;

	} else if ( model->type == MOD_MDM ) {    // use bone lerping

		retval = R_MDM_GetBoneTag( tag, model->model.mdm, startIndex, refent, tagNameIn );
#endif // RTCW_XX

		if ( retval >= 0 ) {
			return retval;
		}

		// failed
		return -1;

	} else {
		// psuedo-compressed MDC tags
		mdcTag_t    *cstart, *cend;

#if !defined RTCW_ET
		retval = R_GetMDCTag( (byte *)model->mdc[0], startFrame, tagName, startIndex, &cstart );
		retval = R_GetMDCTag( (byte *)model->mdc[0], endFrame, tagName, startIndex, &cend );
#else
		retval = R_GetMDCTag( (byte *)model->model.mdc[0], startFrame, tagName, startIndex, &cstart );
		retval = R_GetMDCTag( (byte *)model->model.mdc[0], endFrame, tagName, startIndex, &cend );
#endif // RTCW_XX

		// uncompress the MDC tags into MD3 style tags
		if ( cstart && cend ) {
			for ( i = 0; i < 3; i++ ) {
				ustart.origin[i] = (float)cstart->xyz[i] * MD3_XYZ_SCALE;
				uend.origin[i] = (float)cend->xyz[i] * MD3_XYZ_SCALE;
				sangles[i] = (float)cstart->angles[i] * MDC_TAG_ANGLE_SCALE;
				eangles[i] = (float)cend->angles[i] * MDC_TAG_ANGLE_SCALE;
			}

			AnglesToAxis( sangles, ustart.axis );
			AnglesToAxis( eangles, uend.axis );

			start = &ustart;
			end = &uend;
		} else {
			start = NULL;
			end = NULL;
		}

	}

	if ( !start || !end ) {
		AxisClear( tag->axis );
		VectorClear( tag->origin );
		return -1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		tag->origin[i] = start->origin[i] * backLerp +  end->origin[i] * frontLerp;
		tag->axis[0][i] = start->axis[0][i] * backLerp +  end->axis[0][i] * frontLerp;
		tag->axis[1][i] = start->axis[1][i] * backLerp +  end->axis[1][i] * frontLerp;
		tag->axis[2][i] = start->axis[2][i] * backLerp +  end->axis[2][i] * frontLerp;
	}

	VectorNormalize( tag->axis[0] );
	VectorNormalize( tag->axis[1] );
	VectorNormalize( tag->axis[2] );

	return retval;
}

/*
===============
R_TagInfo_f
===============
*/
void R_TagInfo_f( void ) {

	Com_Printf( "command not functional\n" );

/*
	int handle;
	orientation_t tag;
	int frame = -1;

	if (ri.Cmd_Argc() < 3) {
		Com_Printf("usage: taginfo <model> <tag>\n");
		return;
	}

	handle = RE_RegisterModel( ri.Cmd_Argv(1) );

	if (handle) {
		Com_Printf("found model %s..\n", ri.Cmd_Argv(1));
	} else {
		Com_Printf("cannot find model %s\n", ri.Cmd_Argv(1));
		return;
	}

	if (ri.Cmd_Argc() < 3) {
		frame = 0;
	} else {
		frame = atoi(ri.Cmd_Argv(3));
	}

	Com_Printf("using frame %i..\n", frame);

	R_LerpTag( &tag, handle, frame, frame, 0.0, (const char *)ri.Cmd_Argv(2) );

	Com_Printf("%s at position: %.1f %.1f %.1f\n", ri.Cmd_Argv(2), tag.origin[0], tag.origin[1], tag.origin[2] );
*/
}

/*
====================
R_ModelBounds
====================
*/
void R_ModelBounds( qhandle_t handle, vec3_t mins, vec3_t maxs ) {
	model_t     *model;
	md3Header_t *header;
	md3Frame_t  *frame;

	model = R_GetModelByHandle( handle );

#if !defined RTCW_ET
	if ( model->bmodel ) {
		VectorCopy( model->bmodel->bounds[0], mins );
		VectorCopy( model->bmodel->bounds[1], maxs );
		return;
	}

	// Ridah
	if ( model->md3[0] ) {
		header = model->md3[0];
#else
	// Gordon: fixing now that it's a union
	switch ( model->type ) {
	case MOD_BRUSH:
		VectorCopy( model->model.bmodel->bounds[0], mins );
		VectorCopy( model->model.bmodel->bounds[1], maxs );
		return;
	case MOD_MESH:
		header = model->model.md3[0];
#endif // RTCW_XX

		frame = ( md3Frame_t * )( (byte *)header + header->ofsFrames );

		VectorCopy( frame->bounds[0], mins );
		VectorCopy( frame->bounds[1], maxs );
		return;

#if !defined RTCW_ET
	} else if ( model->mdc[0] ) {
		frame = ( md3Frame_t * )( (byte *)model->mdc[0] + model->mdc[0]->ofsFrames );
#else
	case MOD_MDC:
		frame = ( md3Frame_t * )( (byte *)model->model.mdc[0] + model->model.mdc[0]->ofsFrames );
#endif // RTCW_XX

		VectorCopy( frame->bounds[0], mins );
		VectorCopy( frame->bounds[1], maxs );
		return;

#if defined RTCW_ET
	default:
		break;
#endif // RTCW_XX

	}

	VectorClear( mins );
	VectorClear( maxs );
	// done.
}

//---------------------------------------------------------------------------
// Virtual Memory, used for model caching, since we can't allocate them
// in the main Hunk (since it gets cleared on level changes), and they're
// too large to go into the Zone, we have a special memory chunk just for
// caching models in between levels.
//
// Optimized for Win32 systems, so that they'll grow the swapfile at startup
// if needed, but won't actually commit it until it's needed.
//
// GOAL: reserve a big chunk of virtual memory for the media cache, and only
// use it when we actually need it. This will make sure the swap file grows
// at startup if needed, rather than each allocation we make.
byte    *membase = NULL;
int hunkmaxsize;

// BBi
//#if defined RTCW_SP
//int hunkcursize;        //DAJ renamed from cursize
//#else
//int cursize;
//#endif // RTCW_XX
//
//#define R_HUNK_MEGS     24
//#define R_HUNK_SIZE     ( R_HUNK_MEGS*1024*1024 )

static int hunkcursize;
static const int R_HUNK_MEGS = 24;
static const int R_HUNK_SIZE = R_HUNK_MEGS * 1024 * 1024;
// BBi

// BBi
//void *R_Hunk_Begin( void ) {
//	int maxsize = R_HUNK_SIZE;
//
//	//Com_Printf("R_Hunk_Begin\n");
//
//#if defined RTCW_SP
//	if ( !r_cache->integer ) {
//		return NULL;
//	}
//
//	// reserve a huge chunk of memory, but don't commit any yet
//	hunkcursize = 0;
//#else
//	// reserve a huge chunk of memory, but don't commit any yet
//	cursize = 0;
//#endif // RTCW_XX
//
//	hunkmaxsize = maxsize;
//
//#if defined RTCW_SP
//#ifdef _WIN32
//
//	// this will "reserve" a chunk of memory for use by this application
//	// it will not be "committed" just yet, but the swap file will grow
//	// now if needed
//	membase = static_cast<byte*> (VirtualAlloc( NULL, maxsize, MEM_RESERVE, PAGE_NOACCESS ));
//
//#elif defined( __MACOS__ )
//
//	return; //DAJ FIXME memory leak
//
//#else   // just allocate it now
//
//	// show_bug.cgi?id=440
//	// it is possible that we have been allocated already, in case we don't do anything
//	// NOTE: in SP we currently default to r_cache 0, meaning this code is not used at all
//	//   I am merging the MP way of doing things though, just in case (since we have r_cache 1 on linux MP)
//	if ( !membase ) {
//		membase = malloc( maxsize );
//		// TTimo NOTE: initially, I was doing the memset even if we had an existing membase
//		// but this breaks some shaders (i.e. /map mp_beach, then go back to the main menu .. some shaders are missing)
//		// I assume the shader missing is because we don't clear memory either on win32
//		// meaning even on win32 we are using memory that is still reserved but was uncommited .. it works out of pure luck
//		memset( membase, 0, maxsize );
//	}
//
//#endif
//#elif defined RTCW_MP
//#ifdef _WIN32
//
//	// this will "reserve" a chunk of memory for use by this application
//	// it will not be "committed" just yet, but the swap file will grow
//	// now if needed
//	membase = static_cast<byte*> (VirtualAlloc( NULL, maxsize, MEM_RESERVE, PAGE_NOACCESS ));
//
//#else
//
//	// show_bug.cgi?id=440
//	// if not win32, then just allocate it now
//	// it is possible that we have been allocated already, in case we don't do anything
//	if ( !membase ) {
//		membase = malloc( maxsize );
//		// TTimo NOTE: initially, I was doing the memset even if we had an existing membase
//		// but this breaks some shaders (i.e. /map mp_beach, then go back to the main menu .. some shaders are missing)
//		// I assume the shader missing is because we don't clear memory either on win32
//		// meaning even on win32 we are using memory that is still reserved but was uncommited .. it works out of pure luck
//		memset( membase, 0, maxsize );
//	}
//
//#endif
//#else
//	// reserve a huge chunk of memory, but don't commit any yet
//	cursize = 0;
//	hunkmaxsize = maxsize;
//
//	if ( !membase ) {
//#ifdef _WIN32
//		// this will "reserve" a chunk of memory for use by this application
//		// it will not be "committed" just yet, but the swap file will grow
//		// now if needed
//		membase = static_cast<byte*> (VirtualAlloc( NULL, maxsize, MEM_RESERVE, PAGE_NOACCESS ));
//#else
//		// show_bug.cgi?id=440
//		// if not win32, then just allocate it now
//		// it is possible that we have been allocated already, in case we don't do anything
//		membase = malloc( maxsize );
//		// TTimo NOTE: initially, I was doing the memset even if we had an existing membase
//		// but this breaks some shaders (i.e. /map mp_beach, then go back to the main menu .. some shaders are missing)
//		// I assume the shader missing is because we don't clear memory either on win32
//		// meaning even on win32 we are using memory that is still reserved but was uncommited .. it works out of pure luck
//		memset( membase, 0, maxsize );
//#endif
//	}
//#endif // RTCW_XX
//
//	if ( !membase ) {
//		ri.Error( ERR_DROP, "R_Hunk_Begin: reserve failed" );
//	}
//
//	return (void *)membase;
//}

void* R_Hunk_Begin ()
{
#if defined RTCW_SP
	if (r_cache->integer == 0)
		return 0;
#endif // RTCW_XX

	// reserve a huge chunk of memory, but don't commit any yet
	hunkcursize = 0;
	hunkmaxsize = R_HUNK_SIZE;

	if (membase == 0) {
		membase = static_cast<byte*>(malloc(R_HUNK_SIZE));

		if (membase == 0)
			ri.Error (ERR_DROP, "R_Hunk_Begin: reserve failed");

		memset (membase, 0, R_HUNK_SIZE);
	}

	return membase;
}
// BBi

// BBi
//void *R_Hunk_Alloc( int size ) {
//#ifdef _WIN32
//	void    *buf;
//#endif
//
//	//Com_Printf("R_Hunk_Alloc(%d)\n", size);
//
//	// round to cacheline
//	size = ( size + 31 ) & ~31;
//
//#if defined RTCW_SP
//#ifdef _WIN32
//
//	// commit pages as needed
//	buf = VirtualAlloc( membase, hunkcursize + size, MEM_COMMIT, PAGE_READWRITE );
//
//	if ( !buf ) {
//		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR) &buf, 0, NULL );
//		ri.Error( ERR_DROP, "VirtualAlloc commit failed.\n%s", buf );
//	}
//
//#elif defined( __MACOS__ )
//
//	return NULL;    //DAJ
//
//#endif
//#else
//#ifdef _WIN32
//
//	// commit pages as needed
//	buf = VirtualAlloc( membase, cursize + size, MEM_COMMIT, PAGE_READWRITE );
//
//	if ( !buf ) {
//		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR) &buf, 0, NULL );
//		ri.Error( ERR_DROP, "VirtualAlloc commit failed.\n%s", buf );
//	}
//
//#endif
//#endif // RTCW_XX
//
//#if defined RTCW_SP
//	hunkcursize += size;
//	if ( hunkcursize > hunkmaxsize ) {
//#else
//	cursize += size;
//	if ( cursize > hunkmaxsize ) {
//#endif // RTCW_XX
//
//		ri.Error( ERR_DROP, "R_Hunk_Alloc overflow" );
//	}
//
//#if defined RTCW_SP
//	return ( void * )( membase + hunkcursize - size );
//#else
//	return ( void * )( membase + cursize - size );
//#endif // RTCW_XX
//
//}

void* R_Hunk_Alloc (
	int size)
{
	static const int ROUND_MASK = (8 * sizeof (int)) - 1;

	// round to cacheline
	size = (size + ROUND_MASK) & (~ROUND_MASK);

	hunkcursize += size;

	if (hunkcursize > hunkmaxsize)
		ri.Error (ERR_DROP, "R_Hunk_Alloc overflow");

	return membase + hunkcursize - size;
}
// BBi

// BBi
//// this is only called when we shutdown GL
//void R_Hunk_End( void ) {
//	//Com_Printf("R_Hunk_End\n");
//
//#if defined RTCW_SP
//	if ( !r_cache->integer ) {
//		return;
//	}
//#endif // RTCW_XX
//
//	if ( membase ) {
//
//#if defined RTCW_SP
//#ifdef _WIN32
//		VirtualFree( membase, 0, MEM_RELEASE );
//#elif defined( __MACOS__ )
//		//DAJ FIXME free (membase);
//#else
//		free( membase );
//#endif
//#else
//#ifdef _WIN32
//		VirtualFree( membase, 0, MEM_RELEASE );
//#else
//		free( membase );
//#endif
//#endif // RTCW_XX
//
//	}
//
//	membase = NULL;
//}

// this is only called when we shutdown GL
void R_Hunk_End ()
{
#if defined RTCW_SP
	if (r_cache->integer == 0)
		return;
#endif // RTCW_XX

	if (membase != 0) {
		free(membase);
		membase = 0;
	}
}
// BBi

// BBi
//void R_Hunk_Reset( void ) {
//	//Com_Printf("R_Hunk_Reset\n");
//
//#if defined RTCW_SP
//	if ( !r_cache->integer ) {
//		return;
//	}
//#endif // RTCW_XX
//
//	if ( !membase ) {
//		ri.Error( ERR_DROP, "R_Hunk_Reset called without a membase!" );
//	}
//
//#ifdef _WIN32
//	// mark the existing committed pages as reserved, but not committed
//
//#if defined RTCW_SP
//	VirtualFree( membase, hunkcursize, MEM_DECOMMIT );
//#else
//	VirtualFree( membase, cursize, MEM_DECOMMIT );
//#endif // RTCW_XX
//
//#endif
//	// on non win32 OS, we keep the allocated chunk as is, just start again to curzise = 0
//
//	// start again at the top
//
//#if defined RTCW_SP
//	hunkcursize = 0;
//#else
//	cursize = 0;
//#endif // RTCW_XX
//
//}

void R_Hunk_Reset ()
{
#if defined RTCW_SP
	if (r_cache->integer == 0)
		return;
#endif // RTCW_XX

	if (membase == 0)
		ri.Error (ERR_DROP, "R_Hunk_Reset called without a membase!");

	hunkcursize = 0;
}
// BBi

//=============================================================================
// Ridah, model caching

// TODO: convert the Hunk_Alloc's in the model loading to malloc's, so we don't have
// to move so much memory around during transitions

static model_t backupModels[MAX_MOD_KNOWN];
static int numBackupModels = 0;

/*
===============
R_CacheModelAlloc
===============
*/
void *R_CacheModelAlloc( int size ) {
	if ( r_cache->integer && r_cacheModels->integer ) {
		return R_Hunk_Alloc (size);
	} else {

#if defined RTCW_SP
		// if r_cache 0, this function is not supposed to get called
		Com_Printf( "FIXME: unexpected R_CacheModelAlloc call with r_cache 0\n" );
#endif // RTCW_XX

		return ri.Hunk_Alloc( size, h_low );
	}
}

/*
===============
R_CacheModelFree
===============
*/
void R_CacheModelFree( void *ptr ) {
	if ( r_cache->integer && r_cacheModels->integer ) {
		// TTimo: it's in the hunk, leave it there, next R_Hunk_Begin will clear it all
	} else
	{

#if defined RTCW_SP
		// if r_cache 0, this function is not supposed to get called
		Com_Printf( "FIXME: unexpected R_CacheModelFree call with r_cache 0\n" );
#else
		// if r_cache 0, this is never supposed to get called anyway
		Com_Printf( "FIXME: unexpected R_CacheModelFree call (r_cache 0)\n" );
#endif // RTCW_XX

	}
}

/*
===============
R_PurgeModels
===============
*/
void R_PurgeModels( int count ) {
	static int lastPurged = 0;

#if defined RTCW_SP
	//Com_Printf("R_PurgeModels\n");
#endif // RTCW_XX

	if ( !numBackupModels ) {
		return;
	}

	lastPurged = 0;
	numBackupModels = 0;

	// note: we can only do this since we only use the virtual memory for the model caching!
	R_Hunk_Reset();
}

/*
===============
R_BackupModels
===============
*/
void R_BackupModels( void ) {
	int i, j;
	model_t *mod, *modBack;

#if defined RTCW_SP
	//Com_Printf("R_BackupModels\n");
#endif // RTCW_XX

	if ( !r_cache->integer ) {
		return;
	}
	if ( !r_cacheModels->integer ) {
		return;
	}

	if ( numBackupModels ) {
		R_PurgeModels( numBackupModels + 1 ); // get rid of them all
	}

	// copy each model in memory across to the backupModels
	modBack = backupModels;
	for ( i = 0; i < tr.numModels; i++ ) {
		mod = tr.models[i];

		if ( mod->type && mod->type != MOD_BRUSH && mod->type != MOD_MDS ) {
			memcpy( modBack, mod, sizeof( *mod ) );
			switch ( mod->type ) {
			case MOD_MESH:
				for ( j = MD3_MAX_LODS - 1; j >= 0; j-- ) {

#if !defined RTCW_ET
					if ( j < mod->numLods && mod->md3[j] ) {
						if ( ( j == MD3_MAX_LODS - 1 ) || ( mod->md3[j] != mod->md3[j + 1] ) ) {
							modBack->md3[j] = static_cast<md3Header_t*> (R_CacheModelAlloc( mod->md3[j]->ofsEnd ));
							memcpy( modBack->md3[j], mod->md3[j], mod->md3[j]->ofsEnd );
						} else {
							modBack->md3[j] = modBack->md3[j + 1];
#else
					if ( j < mod->numLods && mod->model.md3[j] ) {
						if ( ( j == MD3_MAX_LODS - 1 ) || ( mod->model.md3[j] != mod->model.md3[j + 1] ) ) {
							modBack->model.md3[j] = static_cast<md3Header_t*> (R_CacheModelAlloc( mod->model.md3[j]->ofsEnd ));
							memcpy( modBack->model.md3[j], mod->model.md3[j], mod->model.md3[j]->ofsEnd );
						} else {
							modBack->model.md3[j] = modBack->model.md3[j + 1];
#endif // RTCW_XX

						}
					}
				}
				break;
			case MOD_MDC:
				for ( j = MD3_MAX_LODS - 1; j >= 0; j-- ) {

#if !defined RTCW_ET
					if ( j < mod->numLods && mod->mdc[j] ) {
						if ( ( j == MD3_MAX_LODS - 1 ) || ( mod->mdc[j] != mod->mdc[j + 1] ) ) {
							modBack->mdc[j] = static_cast<mdcHeader_t*> (R_CacheModelAlloc( mod->mdc[j]->ofsEnd ));
							memcpy( modBack->mdc[j], mod->mdc[j], mod->mdc[j]->ofsEnd );
						} else {
							modBack->mdc[j] = modBack->mdc[j + 1];
#else
					if ( j < mod->numLods && mod->model.mdc[j] ) {
						if ( ( j == MD3_MAX_LODS - 1 ) || ( mod->model.mdc[j] != mod->model.mdc[j + 1] ) ) {
							modBack->model.mdc[j] = static_cast<mdcHeader_t*> (R_CacheModelAlloc( mod->model.mdc[j]->ofsEnd ));
							memcpy( modBack->model.mdc[j], mod->model.mdc[j], mod->model.mdc[j]->ofsEnd );
						} else {
							modBack->model.mdc[j] = modBack->model.mdc[j + 1];
#endif // RTCW_XX

						}
					}
				}
				break;
			default:

#if defined RTCW_SP
				break; // MOD_BAD, MOD_BRUSH, MOD_MDS
#else
				break; // MOD_BAD MOD_BRUSH MOD_MDS not handled
#endif // RTCW_XX

			}
			modBack++;
			numBackupModels++;
		}
	}
}


/*
=================
R_RegisterMDCShaders
=================
*/
static void R_RegisterMDCShaders( model_t *mod, int lod ) {
	mdcSurface_t        *surf;
	md3Shader_t         *shader;
	int i, j;

	// swap all the surfaces

#if !defined RTCW_ET
	surf = ( mdcSurface_t * )( (byte *)mod->mdc[lod] + mod->mdc[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->mdc[lod]->numSurfaces ; i++ ) {
#else
	surf = ( mdcSurface_t * )( (byte *)mod->model.mdc[lod] + mod->model.mdc[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->model.mdc[lod]->numSurfaces ; i++ ) {
#endif // RTCW_XX

		// register the shaders
		shader = ( md3Shader_t * )( (byte *)surf + surf->ofsShaders );
		for ( j = 0 ; j < surf->numShaders ; j++, shader++ ) {
			shader_t    *sh;

			sh = R_FindShader( shader->name, LIGHTMAP_NONE, qtrue );
			if ( sh->defaultShader ) {
				shader->shaderIndex = 0;
			} else {
				shader->shaderIndex = sh->index;
			}
		}
		// find the next surface
		surf = ( mdcSurface_t * )( (byte *)surf + surf->ofsEnd );
	}
}

/*
=================
R_RegisterMD3Shaders
=================
*/
static void R_RegisterMD3Shaders( model_t *mod, int lod ) {
	md3Surface_t        *surf;
	md3Shader_t         *shader;
	int i, j;

	// swap all the surfaces

#if !defined RTCW_ET
	surf = ( md3Surface_t * )( (byte *)mod->md3[lod] + mod->md3[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->md3[lod]->numSurfaces ; i++ ) {
#else
	surf = ( md3Surface_t * )( (byte *)mod->model.md3[lod] + mod->model.md3[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->model.md3[lod]->numSurfaces ; i++ ) {
#endif // RTCW_XX

		// register the shaders
		shader = ( md3Shader_t * )( (byte *)surf + surf->ofsShaders );
		for ( j = 0 ; j < surf->numShaders ; j++, shader++ ) {
			shader_t    *sh;

			sh = R_FindShader( shader->name, LIGHTMAP_NONE, qtrue );
			if ( sh->defaultShader ) {
				shader->shaderIndex = 0;
			} else {
				shader->shaderIndex = sh->index;
			}
		}
		// find the next surface
		surf = ( md3Surface_t * )( (byte *)surf + surf->ofsEnd );
	}
}

/*
===============
R_FindCachedModel

  look for the given model in the list of backupModels
===============
*/
qboolean R_FindCachedModel( const char *name, model_t *newmod ) {
	int i,j, index;
	model_t *mod;

	// NOTE TTimo
	// would need an r_cache check here too?

	if ( !r_cacheModels->integer ) {
		return qfalse;
	}

	if ( !numBackupModels ) {
		return qfalse;
	}

	mod = backupModels;
	for ( i = 0; i < numBackupModels; i++, mod++ ) {
		if ( !Q_strncmp( mod->name, name, sizeof( mod->name ) ) ) {
			// copy it to a new slot
			index = newmod->index;
			memcpy( newmod, mod, sizeof( model_t ) );
			newmod->index = index;
			switch ( mod->type ) {
			case MOD_MDS:
				return qfalse;  // not supported yet

#if defined RTCW_ET
			case MOD_MDM:
				return qfalse;  // not supported yet
			case MOD_MDX:
				return qfalse;  // not supported yet
#endif // RTCW_XX

			case MOD_MESH:
				for ( j = MD3_MAX_LODS - 1; j >= 0; j-- ) {

#if !defined RTCW_ET
					if ( j < mod->numLods && mod->md3[j] ) {
						if ( ( j == MD3_MAX_LODS - 1 ) || ( mod->md3[j] != mod->md3[j + 1] ) ) {
							newmod->md3[j] = static_cast<md3Header_t*> (ri.Hunk_Alloc( mod->md3[j]->ofsEnd, h_low ));
							memcpy( newmod->md3[j], mod->md3[j], mod->md3[j]->ofsEnd );
							R_RegisterMD3Shaders( newmod, j );
							R_CacheModelFree( mod->md3[j] );
						} else {
							newmod->md3[j] = mod->md3[j + 1];
#else
					if ( j < mod->numLods && mod->model.md3[j] ) {
						if ( ( j == MD3_MAX_LODS - 1 ) || ( mod->model.md3[j] != mod->model.md3[j + 1] ) ) {
							newmod->model.md3[j] = static_cast<md3Header_t*> (ri.Hunk_Alloc( mod->model.md3[j]->ofsEnd, h_low ));
							memcpy( newmod->model.md3[j], mod->model.md3[j], mod->model.md3[j]->ofsEnd );
							R_RegisterMD3Shaders( newmod, j );
							R_CacheModelFree( mod->model.md3[j] );
						} else {
							newmod->model.md3[j] = mod->model.md3[j + 1];
#endif // RTCW_XX

						}
					}
				}
				break;
			case MOD_MDC:
				for ( j = MD3_MAX_LODS - 1; j >= 0; j-- ) {

#if !defined RTCW_ET
					if ( j < mod->numLods && mod->mdc[j] ) {
						if ( ( j == MD3_MAX_LODS - 1 ) || ( mod->mdc[j] != mod->mdc[j + 1] ) ) {
							newmod->mdc[j] = static_cast<mdcHeader_t*> (ri.Hunk_Alloc( mod->mdc[j]->ofsEnd, h_low ));
							memcpy( newmod->mdc[j], mod->mdc[j], mod->mdc[j]->ofsEnd );
							R_RegisterMDCShaders( newmod, j );
							R_CacheModelFree( mod->mdc[j] );
						} else {
							newmod->mdc[j] = mod->mdc[j + 1];
#else
					if ( j < mod->numLods && mod->model.mdc[j] ) {
						if ( ( j == MD3_MAX_LODS - 1 ) || ( mod->model.mdc[j] != mod->model.mdc[j + 1] ) ) {
							newmod->model.mdc[j] = static_cast<mdcHeader_t*> (ri.Hunk_Alloc( mod->model.mdc[j]->ofsEnd, h_low ));
							memcpy( newmod->model.mdc[j], mod->model.mdc[j], mod->model.mdc[j]->ofsEnd );
							R_RegisterMDCShaders( newmod, j );
							R_CacheModelFree( mod->model.mdc[j] );
						} else {
							newmod->model.mdc[j] = mod->model.mdc[j + 1];
#endif // RTCW_XX

						}
					}
				}
				break;
			default:

#if defined RTCW_SP
				break; // MOD_BAD, MOD_BRUSH
#else
				break; // MOD_BAD MOD_BRUSH
#endif // RTCW_XX

			}

			mod->type = MOD_BAD;    // don't try and purge it later
			mod->name[0] = 0;
			return qtrue;
		}
	}

	return qfalse;
}

/*
===============
R_LoadCacheModels
===============
*/
void R_LoadCacheModels( void ) {
	int len;
	byte *buf;
	char    *token;
	const char* pString;
	char name[MAX_QPATH];

#if defined RTCW_SP
	//Com_Printf("R_LoadCachedModels\n");
#endif // RTCW_XX

	if ( !r_cacheModels->integer ) {
		return;
	}

	// don't load the cache list in between level loads, only on startup, or after a vid_restart
	if ( numBackupModels > 0 ) {
		return;
	}

	len = ri.FS_ReadFile( "model.cache", NULL );

	if ( len <= 0 ) {
		return;
	}

	buf = (byte *)ri.Hunk_AllocateTempMemory( len );
	ri.FS_ReadFile( "model.cache", (void **)&buf );

#if defined RTCW_SP
	pString = reinterpret_cast<const char*> (buf);       //DAJ added (char*)
#else
	pString = reinterpret_cast<const char*> (buf);
#endif // RTCW_XX

	while ( ( token = COM_ParseExt( &pString, qtrue ) ) && token[0] ) {
		Q_strncpyz( name, token, sizeof( name ) );
		RE_RegisterModel( name );
	}

	ri.Hunk_FreeTempMemory( buf );
}
// done.
//========================================================================
