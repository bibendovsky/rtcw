/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// tr_light.c

#include "tr_local.h"

#define DLIGHT_AT_RADIUS        16
// at the edge of a dlight's influence, this amount of light will be added

#define DLIGHT_MINIMUM_RADIUS   16
// never calculate a range less than this to prevent huge light numbers


/*
===============
R_TransformDlights

Transforms the origins of an array of dlights.
Used by both the front end (for DlightBmodel) and
the back end (before doing the lighting calculation)
===============
*/
void R_TransformDlights( int count, dlight_t *dl, orientationr_t *orient ) {
	int i;
	vec3_t temp;

	for ( i = 0 ; i < count ; i++, dl++ ) {
		VectorSubtract( dl->origin, orient->origin, temp );
		dl->transformed[0] = DotProduct( temp, orient->axis[0] );
		dl->transformed[1] = DotProduct( temp, orient->axis[1] );
		dl->transformed[2] = DotProduct( temp, orient->axis[2] );
	}
}

#if defined RTCW_ET
/*
R_CullDlights()
frustum culls dynamic lights
*/

void R_CullDlights( void ) {
	int i, numDlights, dlightBits;
	dlight_t    *dl;


	/* limit */
	if ( tr.refdef.num_dlights > MAX_DLIGHTS ) {
		tr.refdef.num_dlights = MAX_DLIGHTS;
	}

	/* walk dlight list */
	numDlights = 0;
	dlightBits = 0;
	for ( i = 0, dl = tr.refdef.dlights; i < tr.refdef.num_dlights; i++, dl++ )
	{
		if ( ( dl->flags & REF_DIRECTED_DLIGHT ) || R_CullPointAndRadius( dl->origin, dl->radius ) != CULL_OUT ) {
			numDlights = i + 1;
			dlightBits |= ( 1 << i );
		}
	}

	/* reset count */
	tr.refdef.num_dlights = numDlights;

	/* set bits */
	tr.refdef.dlightBits = dlightBits;
}
#endif // RTCW_XX

/*
=============
R_DlightBmodel

Determine which dynamic lights may effect this bmodel
=============
*/
void R_DlightBmodel( bmodel_t *bmodel ) {
	int i, j;
	dlight_t    *dl;
	int mask;
	msurface_t  *surf;

	// transform all the lights

// BBi
//#if !defined RTCW_ET
//	R_TransformDlights( tr.refdef.num_dlights, tr.refdef.dlights, &tr.or );
//#else
//	R_TransformDlights( tr.refdef.num_dlights, tr.refdef.dlights, &tr.orientation );
//#endif // RTCW_XX
	R_TransformDlights (
		tr.refdef.num_dlights, tr.refdef.dlights, &::tr.orientation);
// BBi

	mask = 0;
	for ( i = 0 ; i < tr.refdef.num_dlights ; i++ ) {
		dl = &tr.refdef.dlights[i];

#if defined RTCW_ET
		// ydnar: parallel dlights affect all entities
		if ( !( dl->flags & REF_DIRECTED_DLIGHT ) ) {
#endif // RTCW_XX

		// see if the point is close enough to the bounds to matter
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( dl->transformed[j] - bmodel->bounds[1][j] > dl->radius ) {
				break;
			}
			if ( bmodel->bounds[0][j] - dl->transformed[j] > dl->radius ) {
				break;
			}
		}
		if ( j < 3 ) {
			continue;
		}

#if defined RTCW_ET
		}
#endif // RTCW_XX

		// we need to check this light
		mask |= 1 << i;
	}

	// RF, this is why some dlights wouldn't light up bmodels

	//tr.currentEntity->needDlights = (mask != 0);

	// (SA) isn't this dangerous to do to an enumerated type? (setting it to an int)
	//		meaning, shouldn't ->needDlights be changed to an int rather than a qbool?

	tr.currentEntity->needDlights = mask;


	// set the dlight bits in all the surfaces
	for ( i = 0 ; i < bmodel->numSurfaces ; i++ ) {
		surf = bmodel->firstSurface + i;

#if 0
		if ( *surf->data == SF_FACE ) {
			( (srfSurfaceFace_t *)surf->data )->dlightBits[ tr.smpFrame ] = mask;
		} else if ( *surf->data == SF_GRID ) {
			( (srfGridMesh_t *)surf->data )->dlightBits[ tr.smpFrame ] = mask;
		} else if ( *surf->data == SF_TRIANGLES ) {

#if !defined RTCW_ET
			( (srfTriangles_t *)surf->data )->dlightBits[ tr.smpFrame ] = mask;
#else
//			((srfTriangles_t *)surf->data)->dlightBits[ tr.smpFrame ] = mask;
			( (srfTriangles2_t *)surf->data )->dlightBits[ tr.smpFrame ] = mask;
#endif // RTCW_XX

#if defined RTCW_ET
		} else if ( *surf->data == SF_FOLIAGE ) {   // ydnar
			( (srfFoliage_t *)surf->data )->dlightBits[ tr.smpFrame ] = mask;
#endif // RTCW_XX

		}
#endif // 0

		if (*surf->data == SF_FACE)
			reinterpret_cast<srfSurfaceFace_t*>(surf->data)->dlightBits = mask;
		else if (*surf->data == SF_GRID)
			reinterpret_cast<srfGridMesh_t*>(surf->data)->dlightBits = mask;
		else if ( *surf->data == SF_TRIANGLES ) {

#if !defined RTCW_ET
			reinterpret_cast<srfTriangles_t*>(surf->data)->dlightBits = mask;
#else
			reinterpret_cast<srfTriangles2_t*>(surf->data)->dlightBits = mask;
#endif // RTCW_XX

#if defined RTCW_ET
		} else if (*surf->data == SF_FOLIAGE) { // ydnar
			reinterpret_cast<srfFoliage_t*>(surf->data)->dlightBits = mask;
#endif // RTCW_XX

		}
	}
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

extern cvar_t  *r_ambientScale;
extern cvar_t  *r_directedScale;
extern cvar_t  *r_debugLight;

/*
=================
R_SetupEntityLightingGrid

=================
*/
static void R_SetupEntityLightingGrid( trRefEntity_t *ent ) {
	vec3_t lightOrigin;
	int pos[3];
	int i, j;
	byte    *gridData;
	float frac[3];
	int gridStep[3];
	vec3_t direction;
	float totalFactor;

	if ( ent->e.renderfx & RF_LIGHTING_ORIGIN ) {
		// seperate lightOrigins are needed so an object that is
		// sinking into the ground can still be lit, and so
		// multi-part models can be lit identically
		VectorCopy( ent->e.lightingOrigin, lightOrigin );
	} else {
		VectorCopy( ent->e.origin, lightOrigin );
	}

	VectorSubtract( lightOrigin, tr.world->lightGridOrigin, lightOrigin );
	for ( i = 0 ; i < 3 ; i++ ) {
		float v;

		v = lightOrigin[i] * tr.world->lightGridInverseSize[i];
		pos[i] = c::floor( v );
		frac[i] = v - pos[i];
		if ( pos[i] < 0 ) {
			pos[i] = 0;
		} else if ( pos[i] >= tr.world->lightGridBounds[i] - 1 ) {
			pos[i] = tr.world->lightGridBounds[i] - 1;
		}
	}

	VectorClear( ent->ambientLight );
	VectorClear( ent->directedLight );
	VectorClear( direction );

	assert( tr.world->lightGridData ); // bk010103 - NULL with -nolight maps

	// trilerp the light value
	gridStep[0] = 8;
	gridStep[1] = 8 * tr.world->lightGridBounds[0];
	gridStep[2] = 8 * tr.world->lightGridBounds[0] * tr.world->lightGridBounds[1];
	gridData = tr.world->lightGridData + pos[0] * gridStep[0]
			   + pos[1] * gridStep[1] + pos[2] * gridStep[2];

	totalFactor = 0;
	for ( i = 0 ; i < 8 ; i++ ) {
		float factor;
		byte    *data;
		int lat, lng;
		vec3_t normal;

		factor = 1.0;
		data = gridData;
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( i & ( 1 << j ) ) {
				factor *= frac[j];
				data += gridStep[j];
			} else {
				factor *= ( 1.0f - frac[j] );
			}
		}

		if ( !( data[0] + data[1] + data[2] ) ) {
			continue;   // ignore samples in walls
		}
		totalFactor += factor;

		ent->ambientLight[0] += factor * data[0];
		ent->ambientLight[1] += factor * data[1];
		ent->ambientLight[2] += factor * data[2];

		ent->directedLight[0] += factor * data[3];
		ent->directedLight[1] += factor * data[4];
		ent->directedLight[2] += factor * data[5];

		lat = data[7];
		lng = data[6];
		lat *= ( FUNCTABLE_SIZE / 256 );
		lng *= ( FUNCTABLE_SIZE / 256 );

		// decode X as cos( lat ) * sin( long )
		// decode Y as sin( lat ) * sin( long )
		// decode Z as cos( long )

		normal[0] = tr.sinTable[( lat + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK] * tr.sinTable[lng];
		normal[1] = tr.sinTable[lat] * tr.sinTable[lng];
		normal[2] = tr.sinTable[( lng + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK];

		VectorMA( direction, factor, normal, direction );

#if defined RTCW_ET
		// ydnar: test code
		//%	if( strstr( tr.models[ ent->e.hModel ]->name, ".mdm" ) && i == 0 )
		//%		ri.Printf( PRINT_ALL, "lat: %3d lng: %3d dir: %2.3f %2.3f %2.3f\n",
		//%			data[ 7 ], data[ 8 ], normal[ 0 ], normal[ 1 ], normal[ 2 ] );
#endif // RTCW_XX

	}

	if ( totalFactor > 0 && totalFactor < 0.99 ) {
		totalFactor = 1.0f / totalFactor;
		VectorScale( ent->ambientLight, totalFactor, ent->ambientLight );
		VectorScale( ent->directedLight, totalFactor, ent->directedLight );
	}

	VectorScale( ent->ambientLight, r_ambientScale->value, ent->ambientLight );
	VectorScale( ent->directedLight, r_directedScale->value, ent->directedLight );

//----(SA)	added
	// cheats?  check for single player?
	if ( tr.lightGridMulDirected ) {
		VectorScale( ent->directedLight, tr.lightGridMulDirected, ent->directedLight );
	}
	if ( tr.lightGridMulAmbient ) {
		VectorScale( ent->ambientLight, tr.lightGridMulAmbient, ent->ambientLight );
	}
//----(SA)	end

	VectorNormalize2( direction, ent->lightDir );

#if defined RTCW_ET
	// ydnar: debug hack
	//%	VectorSubtract( vec3_origin, direction, ent->lightDir );
#endif // RTCW_XX

}


/*
===============
LogLight
===============
*/
static void LogLight( trRefEntity_t *ent ) {
	int max1, max2;

	if ( !( ent->e.renderfx & RF_FIRST_PERSON ) ) {
		return;
	}

	max1 = ent->ambientLight[0];
	if ( ent->ambientLight[1] > max1 ) {
		max1 = ent->ambientLight[1];
	} else if ( ent->ambientLight[2] > max1 ) {
		max1 = ent->ambientLight[2];
	}

	max2 = ent->directedLight[0];
	if ( ent->directedLight[1] > max2 ) {
		max2 = ent->directedLight[1];
	} else if ( ent->directedLight[2] > max2 ) {
		max2 = ent->directedLight[2];
	}

	ri.Printf( PRINT_ALL, "amb:%i  dir:%i\n", max1, max2 );
}

/*
=================
R_SetupEntityLighting

Calculates all the lighting values that will be used
by the Calc_* functions
=================
*/
void R_SetupEntityLighting( const trRefdef_t *refdef, trRefEntity_t *ent ) {
	int i;
	dlight_t        *dl;

#if !defined RTCW_ET
	float power;
#endif // RTCW_XX

	vec3_t dir;

#if !defined RTCW_ET
	float d;
#else
	float d, modulate;
#endif // RTCW_XX

	vec3_t lightDir;
	vec3_t lightOrigin;

#if !defined RTCW_ET
//	qboolean		highlighted = qfalse; // TTimo: unused
#endif // RTCW_XX

#if defined RTCW_ET
	vec3_t lightValue;
	byte            *entityLight;
#endif // RTCW_XX


	// lighting calculations
	if ( ent->lightingCalculated ) {
		return;
	}
	ent->lightingCalculated = qtrue;

	//
	// trace a sample point down to find ambient light
	//
	if ( ent->e.renderfx & RF_LIGHTING_ORIGIN ) {
		// seperate lightOrigins are needed so an object that is
		// sinking into the ground can still be lit, and so
		// multi-part models can be lit identically
		VectorCopy( ent->e.lightingOrigin, lightOrigin );
	} else {
		VectorCopy( ent->e.origin, lightOrigin );
	}

	// if NOWORLDMODEL, only use dynamic lights (menu system, etc)

#if !defined RTCW_ET
	if ( !( refdef->rdflags & RDF_NOWORLDMODEL )
		 && tr.world->lightGridData ) {
		R_SetupEntityLightingGrid( ent );
	} else {
		ent->ambientLight[0] = ent->ambientLight[1] =
								   ent->ambientLight[2] = tr.identityLight * 150;
		ent->directedLight[0] = ent->directedLight[1] =
									ent->directedLight[2] = tr.identityLight * 150;
		VectorCopy( tr.sunDirection, ent->lightDir );
#else
	if ( tr.world && tr.world->lightGridData &&
		 ( !( refdef->rdflags & RDF_NOWORLDMODEL ) ||
		   ( ( refdef->rdflags & RDF_NOWORLDMODEL ) && ( ent->e.renderfx & RF_LIGHTING_ORIGIN ) ) ) ) {
		R_SetupEntityLightingGrid( ent );
	} else
	{
		//%	ent->ambientLight[0] = ent->ambientLight[1] = ent->ambientLight[2] = tr.identityLight * 150;
		//%	ent->directedLight[0] = ent->directedLight[1] = ent->directedLight[2] = tr.identityLight * 150;
		//%	VectorCopy( tr.sunDirection, ent->lightDir );
		ent->ambientLight[ 0 ] = tr.identityLight * 64;
		ent->ambientLight[ 1 ] = tr.identityLight * 64;
		ent->ambientLight[ 2 ] = tr.identityLight * 96;
		ent->directedLight[ 0 ] = tr.identityLight * 255;
		ent->directedLight[ 1 ] = tr.identityLight * 232;
		ent->directedLight[ 2 ] = tr.identityLight * 224;
		VectorSet( ent->lightDir, -1, 1, 1.25 );
		VectorNormalize( ent->lightDir );
#endif // RTCW_XX

	}

	if ( ent->e.hilightIntensity ) {
		// level of intensity was set because the item was looked at
		ent->ambientLight[0] += tr.identityLight * 128 * ent->e.hilightIntensity;
		ent->ambientLight[1] += tr.identityLight * 128 * ent->e.hilightIntensity;
		ent->ambientLight[2] += tr.identityLight * 128 * ent->e.hilightIntensity;
	} else if ( ent->e.renderfx & RF_MINLIGHT ) {
		// give everything a minimum light add
		ent->ambientLight[0] += tr.identityLight * 32;
		ent->ambientLight[1] += tr.identityLight * 32;
		ent->ambientLight[2] += tr.identityLight * 32;
	}


	if ( ent->e.entityNum < MAX_CLIENTS && ( refdef->rdflags & RDF_SNOOPERVIEW ) ) {
		VectorSet( ent->ambientLight, 245, 245, 245 );    // allow a little room for flicker from directed light
	}


	//
	// modify the light by dynamic lights
	//
	d = VectorLength( ent->directedLight );
	VectorScale( ent->lightDir, d, lightDir );

	for ( i = 0 ; i < refdef->num_dlights ; i++ ) {
		dl = &refdef->dlights[i];

#if !defined RTCW_ET
		if ( dl->dlshader ) {  //----(SA)	if the dlight has a diff shader specified, you don't know what it does, so don't let it affect entities lighting
#else
		if ( dl->shader ) { //----(SA)	if the dlight has a diff shader specified, you don't know what it does, so don't let it affect entities lighting
#endif // RTCW_XX

			continue;
		}

#if !defined RTCW_ET
		VectorSubtract( dl->origin, lightOrigin, dir );
		d = VectorNormalize( dir );

		power = DLIGHT_AT_RADIUS * ( dl->radius * dl->radius );
		if ( d < DLIGHT_MINIMUM_RADIUS ) {
			d = DLIGHT_MINIMUM_RADIUS;
		}

		d = power / ( d * d );

		VectorMA( ent->directedLight, d, dl->color, ent->directedLight );
		VectorMA( lightDir, d, dir, lightDir );
#else
		#if 0
		VectorSubtract( dl->origin, lightOrigin, dir );
		d = VectorNormalize( dir );
		modulate = DLIGHT_AT_RADIUS * ( dl->radius * dl->radius );
		if ( d < DLIGHT_MINIMUM_RADIUS ) {
			d = DLIGHT_MINIMUM_RADIUS;
		}

		modulate = modulate / ( d * d );
		#else
		// directional dlight, origin is a directional normal
		if ( dl->flags & REF_DIRECTED_DLIGHT ) {
			modulate = dl->intensity * 255.0;
			VectorCopy( dl->origin, dir );
		}
		// ball dlight
		else
		{
			VectorSubtract( dl->origin, lightOrigin, dir );
			d = dl->radius - VectorNormalize( dir );
			if ( d <= 0.0 ) {
				modulate = 0;
			} else {
				modulate = dl->intensity * d;
			}
		}
		#endif

		VectorMA( ent->directedLight, modulate, dl->color, ent->directedLight );
		VectorMA( lightDir, modulate, dir, lightDir );
#endif // RTCW_XX

	}

	// clamp ambient
	for ( i = 0 ; i < 3 ; i++ ) {
		if ( ent->ambientLight[i] > tr.identityLightByte ) {
			ent->ambientLight[i] = tr.identityLightByte;
		}
	}

	if ( r_debugLight->integer ) {
		LogLight( ent );
	}

#if defined RTCW_ET
	// ydnar: test code
	//%	VectorClear( ent->ambientLight );
#endif // RTCW_XX

	// save out the byte packet version
	( (byte *)&ent->ambientLightInt )[0] = myftol( ent->ambientLight[0] );
	( (byte *)&ent->ambientLightInt )[1] = myftol( ent->ambientLight[1] );
	( (byte *)&ent->ambientLightInt )[2] = myftol( ent->ambientLight[2] );
	( (byte *)&ent->ambientLightInt )[3] = 0xff;

#if defined RTCW_ET
	// ydnar: save out the light table
	d = 0.0f;
	entityLight = (byte*) ent->entityLightInt;
	modulate = 1.0f / ( ENTITY_LIGHT_STEPS - 1 );
	for ( i = 0; i < ENTITY_LIGHT_STEPS; i++ )
	{
		VectorMA( ent->ambientLight, d, ent->directedLight, lightValue );
		entityLight[ 0 ] = lightValue[ 0 ] > 255.0f ? 255 : myftol( lightValue[ 0 ] );
		entityLight[ 1 ] = lightValue[ 1 ] > 255.0f ? 255 : myftol( lightValue[ 1 ] );
		entityLight[ 2 ] = lightValue[ 2 ] > 255.0f ? 255 : myftol( lightValue[ 2 ] );
		entityLight[ 3 ] = 0xFF;

		d += modulate;
		entityLight += 4;
	}

	// ydnar: test code
	//%	VectorSet( lightDir, 0, 0, 1 );
#endif // RTCW_XX

	// transform the direction to local space
	VectorNormalize( lightDir );
	ent->lightDir[0] = DotProduct( lightDir, ent->e.axis[0] );
	ent->lightDir[1] = DotProduct( lightDir, ent->e.axis[1] );
	ent->lightDir[2] = DotProduct( lightDir, ent->e.axis[2] );

#if defined RTCW_ET
	// ydnar: renormalize if necessary
	if ( ent->e.nonNormalizedAxes ) {
		VectorNormalize( ent->lightDir );
	}

	// ydnar: test code
	//%	if( strstr( tr.models[ ent->e.hModel ]->name, ".mdm" ) )
	//%		ri.Printf( PRINT_ALL, "vec: %f %f %f   localvec: %f %f %f\n",
	//%			lightDir[ 0 ], lightDir[ 1 ], lightDir[ 2 ],
	//%			ent->lightDir[ 0 ], ent->lightDir[ 1 ], ent->lightDir[ 2 ] );
#endif // RTCW_XX

}

/*
=================
R_LightForPoint
=================
*/
int R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir ) {
	trRefEntity_t ent;

	// bk010103 - this segfaults with -nolight maps
	if ( tr.world->lightGridData == NULL ) {
		return qfalse;
	}

	Com_Memset( &ent, 0, sizeof( ent ) );
	VectorCopy( point, ent.e.origin );
	R_SetupEntityLightingGrid( &ent );
	VectorCopy( ent.ambientLight, ambientLight );
	VectorCopy( ent.directedLight, directedLight );
	VectorCopy( ent.lightDir, lightDir );

	return qtrue;
}
