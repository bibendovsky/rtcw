/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "tr_local.h"

int r_firstSceneDrawSurf;

int r_numdlights;
int r_firstSceneDlight;

int r_numcoronas;
int r_firstSceneCorona;

int r_numentities;
int r_firstSceneEntity;

int r_numpolys;
int r_firstScenePoly;

int r_numpolyverts;

#if defined RTCW_ET
// Gordon: TESTING
int r_firstScenePolybuffer;
int r_numpolybuffers;

// ydnar: decals
int r_firstSceneDecalProjector;
int r_numDecalProjectors;
int r_firstSceneDecal;
int r_numDecals;
#endif // RTCW_XX

int skyboxportal;

#if defined RTCW_SP
int drawskyboxportal;
#endif // RTCW_XX


/*
====================
R_ToggleSmpFrame

====================
*/
void R_ToggleSmpFrame( void ) {
#if 0
	if ( r_smp->integer ) {
		// use the other buffers next frame, because another CPU
		// may still be rendering into the current ones
		tr.smpFrame ^= 1;
	} else {
		tr.smpFrame = 0;
	}

	backEndData[tr.smpFrame]->commands.used = 0;
#endif // 0

	tr.smpFrame = 0;
	backEndData->commands.used = 0;

	r_firstSceneDrawSurf = 0;

	r_numdlights = 0;
	r_firstSceneDlight = 0;

	r_numcoronas = 0;
	r_firstSceneCorona = 0;

	r_numentities = 0;
	r_firstSceneEntity = 0;

	r_numpolys = 0;
	r_firstScenePoly = 0;

	r_numpolyverts = 0;

#if defined RTCW_ET
	// Gordon: TESTING
	r_numpolybuffers = 0;
	r_firstScenePolybuffer = 0;

	// ydnar: decals
	r_numDecalProjectors = 0;
	r_firstSceneDecalProjector = 0;
	r_numDecals = 0;
	r_firstSceneDecal = 0;
#endif // RTCW_XX

}

/*
====================
RE_ClearScene

====================
*/
void RE_ClearScene( void ) {

#if defined RTCW_ET
	int i;


	// ydnar: clear model stuff for dynamic fog
	if ( tr.world != NULL ) {
		for ( i = 0; i < tr.world->numBModels; i++ )
#if 0
			tr.world->bmodels[ i ].visible[ tr.smpFrame ] = qfalse;
#endif // 0
			tr.world->bmodels[i].visible = qfalse;
	}

	// everything else
#endif // RTCW_XX

	r_firstSceneDlight = r_numdlights;
	r_firstSceneCorona = r_numcoronas;
	r_firstSceneEntity = r_numentities;
	r_firstScenePoly = r_numpolys;
}

/*
===========================================================================

DISCRETE POLYS

===========================================================================
*/

/*
=====================
R_AddPolygonSurfaces

Adds all the scene's polys into this view's drawsurf list
=====================
*/
void R_AddPolygonSurfaces( void ) {
	int i;
	shader_t    *sh;
	srfPoly_t   *poly;

	tr.currentEntityNum = ENTITYNUM_WORLD;
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

	for ( i = 0, poly = tr.refdef.polys; i < tr.refdef.numPolys ; i++, poly++ ) {
		sh = R_GetShaderByHandle( poly->hShader );

#if defined RTCW_SP
// GR - not tessellated
		R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (poly), sh, poly->fogIndex, qfalse, ATI_TESS_NONE );
#elif defined RTCW_MP
		R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (poly), sh, poly->fogIndex, qfalse );
#else
		R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (poly), sh, poly->fogIndex, 0, 0 );
#endif // RTCW_XX

	}
}

/*
=====================
RE_AddPolyToScene

=====================
*/
void RE_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	srfPoly_t   *poly;
	int i;
	int fogIndex;
	fog_t       *fog;
	vec3_t bounds[2];

	if ( !tr.registered ) {
		return;
	}

	if ( !hShader ) {
		ri.Printf( PRINT_WARNING, "WARNING: RE_AddPolyToScene: NULL poly shader\n" );
		return;
	}

	if ( ( ( r_numpolyverts + numVerts ) > max_polyverts ) || ( r_numpolys >= max_polys ) ) {
		return;
	}

// BBi
//	poly = &backEndData[tr.smpFrame]->polys[r_numpolys];
	poly = &backEndData->polys[r_numpolys];
// BBi

	poly->surfaceType = SF_POLY;
	poly->hShader = hShader;
	poly->numVerts = numVerts;

// BBi
//	poly->verts = &backEndData[tr.smpFrame]->polyVerts[r_numpolyverts];
	poly->verts = &backEndData->polyVerts[r_numpolyverts];
// BBi

	memcpy( poly->verts, verts, numVerts * sizeof( *verts ) );

	// BBi
	//// Ridah
	//if ( glConfig.hardwareType == GLHW_RAGEPRO ) {
	//	poly->verts->modulate[0] = 255;
	//	poly->verts->modulate[1] = 255;
	//	poly->verts->modulate[2] = 255;
	//	poly->verts->modulate[3] = 255;
	//}
	//// done.
	// BBi

	r_numpolys++;
	r_numpolyverts += numVerts;

	// see if it is in a fog volume
	if ( tr.world->numfogs == 1 ) {
		fogIndex = 0;
	} else {
		// find which fog volume the poly is in
		VectorCopy( poly->verts[0].xyz, bounds[0] );
		VectorCopy( poly->verts[0].xyz, bounds[1] );
		for ( i = 1 ; i < poly->numVerts ; i++ ) {
			AddPointToBounds( poly->verts[i].xyz, bounds[0], bounds[1] );
		}
		for ( fogIndex = 1 ; fogIndex < tr.world->numfogs ; fogIndex++ ) {
			fog = &tr.world->fogs[fogIndex];
			if ( bounds[1][0] >= fog->bounds[0][0]
				 && bounds[1][1] >= fog->bounds[0][1]
				 && bounds[1][2] >= fog->bounds[0][2]
				 && bounds[0][0] <= fog->bounds[1][0]
				 && bounds[0][1] <= fog->bounds[1][1]
				 && bounds[0][2] <= fog->bounds[1][2] ) {
				break;
			}
		}
		if ( fogIndex == tr.world->numfogs ) {
			fogIndex = 0;
		}
	}
	poly->fogIndex = fogIndex;
}

// Ridah
/*
=====================
RE_AddPolysToScene

=====================
*/
void RE_AddPolysToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys ) {
	srfPoly_t   *poly;
	int i;
	int fogIndex;
	fog_t       *fog;
	vec3_t bounds[2];
	int j;

	if ( !tr.registered ) {
		return;
	}

	if ( !hShader ) {
		ri.Printf( PRINT_WARNING, "WARNING: RE_AddPolysToScene: NULL poly shader\n" );
		return;
	}

	for ( j = 0; j < numPolys; j++ ) {
		if ( r_numpolyverts + numVerts > max_polyverts || r_numpolys >= max_polys ) {
//			ri.Printf( PRINT_WARNING, "WARNING: RE_AddPolysToScene: MAX_POLYS or MAX_POLYVERTS reached\n");
			return;
		}

// BBi
//		poly = &backEndData[tr.smpFrame]->polys[r_numpolys];
		poly = &backEndData->polys[r_numpolys];
// BBi

		poly->surfaceType = SF_POLY;
		poly->hShader = hShader;
		poly->numVerts = numVerts;

// BBi
//		poly->verts = &backEndData[tr.smpFrame]->polyVerts[r_numpolyverts];
		poly->verts = &backEndData->polyVerts[r_numpolyverts];
// BBi

		memcpy( poly->verts, &verts[numVerts * j], numVerts * sizeof( *verts ) );

		// BBi
		//// Ridah
		//if ( glConfig.hardwareType == GLHW_RAGEPRO ) {
		//	poly->verts->modulate[0] = 255;
		//	poly->verts->modulate[1] = 255;
		//	poly->verts->modulate[2] = 255;
		//	poly->verts->modulate[3] = 255;
		//}
		//// done.
		// BBi

		r_numpolys++;
		r_numpolyverts += numVerts;

		// if no world is loaded
		if ( tr.world == NULL ) {
			fogIndex = 0;
		}
		// see if it is in a fog volume
		else if ( tr.world->numfogs == 1 ) {
			fogIndex = 0;
		} else {
			// find which fog volume the poly is in
			VectorCopy( poly->verts[0].xyz, bounds[0] );
			VectorCopy( poly->verts[0].xyz, bounds[1] );
			for ( i = 1 ; i < poly->numVerts ; i++ ) {
				AddPointToBounds( poly->verts[i].xyz, bounds[0], bounds[1] );
			}
			for ( fogIndex = 1 ; fogIndex < tr.world->numfogs ; fogIndex++ ) {
				fog = &tr.world->fogs[fogIndex];
				if ( bounds[1][0] >= fog->bounds[0][0]
					 && bounds[1][1] >= fog->bounds[0][1]
					 && bounds[1][2] >= fog->bounds[0][2]
					 && bounds[0][0] <= fog->bounds[1][0]
					 && bounds[0][1] <= fog->bounds[1][1]
					 && bounds[0][2] <= fog->bounds[1][2] ) {
					break;
				}
			}
			if ( fogIndex == tr.world->numfogs ) {
				fogIndex = 0;
			}
		}
		poly->fogIndex = fogIndex;
	}
}
// done.

#if defined RTCW_ET
/*
=====================
R_AddPolygonSurfaces

Adds all the scene's polys into this view's drawsurf list
=====================
*/
void R_AddPolygonBufferSurfaces( void ) {
	int i;
	shader_t        *sh;
	srfPolyBuffer_t *polybuffer;

	tr.currentEntityNum = ENTITYNUM_WORLD;
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

	for ( i = 0, polybuffer = tr.refdef.polybuffers; i < tr.refdef.numPolyBuffers ; i++, polybuffer++ ) {
		sh = R_GetShaderByHandle( polybuffer->pPolyBuffer->shader );

		R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (polybuffer), sh, polybuffer->fogIndex, 0, 0 );
	}
}

/*
=====================
RE_AddPolyBufferToScene

=====================
*/
void RE_AddPolyBufferToScene( polyBuffer_t* pPolyBuffer ) {
	srfPolyBuffer_t*    pPolySurf;
	int fogIndex;
	fog_t*              fog;
	vec3_t bounds[2];
	int i;

	if ( r_numpolybuffers >= MAX_POLYS ) {
		return;
	}

#if 0
	pPolySurf = &backEndData[tr.smpFrame]->polybuffers[r_numpolybuffers];
#endif // 0
	pPolySurf = &backEndData->polybuffers[r_numpolybuffers];

	r_numpolybuffers++;

	pPolySurf->surfaceType = SF_POLYBUFFER;
	pPolySurf->pPolyBuffer = pPolyBuffer;

	VectorCopy( pPolyBuffer->xyz[0], bounds[0] );
	VectorCopy( pPolyBuffer->xyz[0], bounds[1] );
	for ( i = 1 ; i < pPolyBuffer->numVerts ; i++ ) {
		AddPointToBounds( pPolyBuffer->xyz[i], bounds[0], bounds[1] );
	}
	for ( fogIndex = 1 ; fogIndex < tr.world->numfogs ; fogIndex++ ) {
		fog = &tr.world->fogs[fogIndex];
		if ( bounds[1][0] >= fog->bounds[0][0]
			 && bounds[1][1] >= fog->bounds[0][1]
			 && bounds[1][2] >= fog->bounds[0][2]
			 && bounds[0][0] <= fog->bounds[1][0]
			 && bounds[0][1] <= fog->bounds[1][1]
			 && bounds[0][2] <= fog->bounds[1][2] ) {
			break;
		}
	}
	if ( fogIndex == tr.world->numfogs ) {
		fogIndex = 0;
	}

	pPolySurf->fogIndex = fogIndex;
}
#endif // RTCW_XX

//=================================================================================


/*
=====================
RE_AddRefEntityToScene

=====================
*/
void RE_AddRefEntityToScene( const refEntity_t *ent ) {
	if ( !tr.registered ) {
		return;
	}
	// show_bug.cgi?id=402
	if ( r_numentities >= ENTITYNUM_WORLD ) {
		return;
	}
	if ( ent->reType < 0 || ent->reType >= RT_MAX_REF_ENTITY_TYPE ) {
		ri.Error( ERR_DROP, "RE_AddRefEntityToScene: bad reType %i", ent->reType );
	}

// BBi
//	backEndData[tr.smpFrame]->entities[r_numentities].e = *ent;
//	backEndData[tr.smpFrame]->entities[r_numentities].lightingCalculated = qfalse;
	backEndData->entities[r_numentities].e = *ent;
	backEndData->entities[r_numentities].lightingCalculated = qfalse;
// BBi

	r_numentities++;

#if defined RTCW_ET
	// ydnar: add projected shadows for this model
	// Arnout: casting const away
	R_AddModelShadow( (refEntity_t*) ent );
#endif // RTCW_XX

}

#if !defined RTCW_ET
// Ridah, added support for overdraw field
/*
=====================
RE_AddLightToScene

=====================
*/
void RE_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw ) {
#else
/*
RE_AddLightToScene()
ydnar: modified dlight system to support seperate radius and intensity
*/

void RE_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags ) {
#endif // RTCW_XX

	dlight_t    *dl;

#if !defined RTCW_ET
	if ( !tr.registered ) {
#else
	// early out
	if ( !tr.registered || r_numdlights >= MAX_DLIGHTS || radius <= 0 || intensity <= 0 ) {
#endif // RTCW_XX

		return;
	}

#if !defined RTCW_ET
	if ( r_numdlights >= MAX_DLIGHTS ) {
		return;
	}
	if ( intensity <= 0 ) {
		return;
	}
#endif // RTCW_XX

	// BBi
	//// these cards don't have the correct blend mode
	//if ( glConfig.hardwareType == GLHW_RIVA128 || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
	//	return;
	//}
	// BBi

	// RF, allow us to force some dlights under all circumstances

#if !defined RTCW_ET
	if ( !( overdraw & REF_FORCE_DLIGHT ) ) {
#else
	if ( !( flags & REF_FORCE_DLIGHT ) ) {
#endif // RTCW_XX

		if ( r_dynamiclight->integer == 0 ) {
			return;
		}

#if !defined RTCW_ET
// BBi
#if 0
		if ( r_dynamiclight->integer == 2 && !( backEndData[tr.smpFrame]->dlights[r_numdlights].forced ) ) {
			return;
		}
#endif // 0

		if (r_dynamiclight->integer == 2 &&
			!backEndData->dlights[r_numdlights].forced)
		{
			return;
		}
// BBi
#endif // RTCW_XX

	}

#if defined RTCW_SP
	if ( r_dlightScale->value <= 0 ) { //----(SA)	added
		return;
	}
#endif // RTCW_XX

#if !defined RTCW_ET
	overdraw &= ~REF_FORCE_DLIGHT;
#endif // RTCW_XX

#if defined RTCW_SP
	overdraw &= ~REF_JUNIOR_DLIGHT;
#elif defined RTCW_MP
	overdraw &= ~REF_JUNIOR_DLIGHT; //----(SA)	added
#endif // RTCW_XX

// BBi
#if 0
	dl = &backEndData[tr.smpFrame]->dlights[r_numdlights++];
#endif // 0
	dl = &backEndData->dlights[r_numdlights++];
// BBi

	VectorCopy( org, dl->origin );

#if defined RTCW_MP
	dl->radius = intensity;
#endif // RTCW_XX

#if defined RTCW_SP
	dl->radius = intensity * r_dlightScale->value;  //----(SA)	modified
#endif // RTCW_XX

#if defined RTCW_ET
	VectorCopy( org, dl->transformed );
	dl->radius = radius;
	dl->radiusInverseCubed = ( 1.0 / dl->radius );
	dl->radiusInverseCubed = dl->radiusInverseCubed * dl->radiusInverseCubed * dl->radiusInverseCubed;
	dl->intensity = intensity;
#endif // RTCW_XX

	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;

#if !defined RTCW_ET
	dl->dlshader = NULL;
	dl->overdraw = 0;
#else
	dl->shader = R_GetShaderByHandle( hShader );
	if ( dl->shader == tr.defaultShader ) {
		dl->shader = NULL;
	}
	dl->flags = flags;
#endif // RTCW_XX

#if !defined RTCW_ET
	if ( overdraw == 10 ) { // sorry, hijacking 10 for a quick hack (SA)
		dl->dlshader = R_GetShaderByHandle( RE_RegisterShader( "negdlightshader" ) );
	} else if ( overdraw == 11 ) { // 11 is flames
		dl->dlshader = R_GetShaderByHandle( RE_RegisterShader( "flamedlightshader" ) );
	} else {
		dl->overdraw = overdraw;
	}
#endif // RTCW_XX

}
// done.


/*
==============
RE_AddCoronaToScene
==============
*/
// BBi
//#if defined RTCW_SP
//void RE_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, int flags ) {
//#else
//void RE_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible ) {
//#endif // RTCW_XX
void RE_AddCoronaToScene (const vec3_t org, float r, float g, float b,
	float scale, int id, int flags)
{
// BBi
	corona_t    *cor;

	if ( !tr.registered ) {
		return;
	}
	if ( r_numcoronas >= MAX_CORONAS ) {
		return;
	}

#if 0
	cor = &backEndData[tr.smpFrame]->coronas[r_numcoronas++];
#endif // 0

	cor = &backEndData->coronas[r_numcoronas++];

	VectorCopy( org, cor->origin );
	cor->color[0] = r;
	cor->color[1] = g;
	cor->color[2] = b;
	cor->scale = scale;
	cor->id = id;

// BBi
//#if defined RTCW_SP
//	cor->flags = flags;
//#else
//	cor->visible = visible;
//#endif // RTCW_XX
	cor->flags = flags;
// BBi
}

/*
@@@@@@@@@@@@@@@@@@@@@
RE_RenderScene

Draw a 3D view into a part of the window, then return
to 2D drawing.

Rendering a scene may require multiple views to be rendered
to handle mirrors,
@@@@@@@@@@@@@@@@@@@@@
*/
void RE_RenderScene( const refdef_t *fd ) {
	viewParms_t parms;
	int startTime;

	if ( !tr.registered ) {
		return;
	}
	//GLimp_LogComment( "====== RE_RenderScene =====\n" );

	if ( r_norefresh->integer ) {
		return;
	}

	startTime = ri.Milliseconds();

	if ( !tr.world && !( fd->rdflags & RDF_NOWORLDMODEL ) ) {
		ri.Error( ERR_DROP, "R_RenderScene: NULL worldmodel" );
	}

	memcpy( tr.refdef.text, fd->text, sizeof( tr.refdef.text ) );

	tr.refdef.x = fd->x;
	tr.refdef.y = fd->y;
	tr.refdef.width = fd->width;
	tr.refdef.height = fd->height;
	tr.refdef.fov_x = fd->fov_x;
	tr.refdef.fov_y = fd->fov_y;

	VectorCopy( fd->vieworg, tr.refdef.vieworg );
	VectorCopy( fd->viewaxis[0], tr.refdef.viewaxis[0] );
	VectorCopy( fd->viewaxis[1], tr.refdef.viewaxis[1] );
	VectorCopy( fd->viewaxis[2], tr.refdef.viewaxis[2] );

	tr.refdef.time = fd->time;
	tr.refdef.rdflags = fd->rdflags;

	if ( fd->rdflags & RDF_SKYBOXPORTAL ) {
		skyboxportal = 1;
	}

#if defined RTCW_SP
	if ( fd->rdflags & RDF_DRAWSKYBOX ) {
		drawskyboxportal = 1;
	} else {
		drawskyboxportal = 0;
	}
#endif // RTCW_XX

	// copy the areamask data over and note if it has changed, which
	// will force a reset of the visible leafs even if the view hasn't moved
	tr.refdef.areamaskModified = qfalse;
	if ( !( tr.refdef.rdflags & RDF_NOWORLDMODEL ) ) {
		int areaDiff;
		int i;

		// compare the area bits
		areaDiff = 0;
		for ( i = 0 ; i < MAX_MAP_AREA_BYTES / 4 ; i++ ) {
			areaDiff |= ( (int *)tr.refdef.areamask )[i] ^ ( (int *)fd->areamask )[i];
			( (int *)tr.refdef.areamask )[i] = ( (int *)fd->areamask )[i];
		}

		if ( areaDiff ) {
			// a door just opened or something
			tr.refdef.areamaskModified = qtrue;
		}
	}


	// derived info

#if 0
	tr.refdef.floatTime = tr.refdef.time * 0.001f;

	tr.refdef.numDrawSurfs = r_firstSceneDrawSurf;
	tr.refdef.drawSurfs = backEndData[tr.smpFrame]->drawSurfs;

	tr.refdef.num_entities = r_numentities - r_firstSceneEntity;
	tr.refdef.entities = &backEndData[tr.smpFrame]->entities[r_firstSceneEntity];

	tr.refdef.num_dlights = r_numdlights - r_firstSceneDlight;
	tr.refdef.dlights = &backEndData[tr.smpFrame]->dlights[r_firstSceneDlight];

#if defined RTCW_ET
	tr.refdef.dlightBits = 0;
#endif // RTCW_XX

	tr.refdef.num_coronas = r_numcoronas - r_firstSceneCorona;
	tr.refdef.coronas = &backEndData[tr.smpFrame]->coronas[r_firstSceneCorona];

	tr.refdef.numPolys = r_numpolys - r_firstScenePoly;
	tr.refdef.polys = &backEndData[tr.smpFrame]->polys[r_firstScenePoly];

#if defined RTCW_ET
	tr.refdef.numPolyBuffers = r_numpolybuffers - r_firstScenePolybuffer;
	tr.refdef.polybuffers = &backEndData[tr.smpFrame]->polybuffers[r_firstScenePolybuffer];

	tr.refdef.numDecalProjectors = r_numDecalProjectors - r_firstSceneDecalProjector;
	tr.refdef.decalProjectors = &backEndData[ tr.smpFrame ]->decalProjectors[ r_firstSceneDecalProjector ];

	tr.refdef.numDecals = r_numDecals - r_firstSceneDecal;
	tr.refdef.decals = &backEndData[ tr.smpFrame ]->decals[ r_firstSceneDecal ];
#endif // RTCW_XX
#endif // 0

	tr.refdef.floatTime = tr.refdef.time * 0.001F;

	tr.refdef.numDrawSurfs = r_firstSceneDrawSurf;
	tr.refdef.drawSurfs = backEndData->drawSurfs;

	tr.refdef.num_entities = r_numentities - r_firstSceneEntity;
	tr.refdef.entities = &backEndData->entities[r_firstSceneEntity];

	tr.refdef.num_dlights = r_numdlights - r_firstSceneDlight;
	tr.refdef.dlights = &backEndData->dlights[r_firstSceneDlight];

#if defined RTCW_ET
	tr.refdef.dlightBits = 0;
#endif // RTCW_XX

	tr.refdef.num_coronas = r_numcoronas - r_firstSceneCorona;
	tr.refdef.coronas = &backEndData->coronas[r_firstSceneCorona];

	tr.refdef.numPolys = r_numpolys - r_firstScenePoly;
	tr.refdef.polys = &backEndData->polys[r_firstScenePoly];

#if defined RTCW_ET
	tr.refdef.numPolyBuffers = r_numpolybuffers - r_firstScenePolybuffer;
	tr.refdef.polybuffers = &backEndData->polybuffers[r_firstScenePolybuffer];

	tr.refdef.numDecalProjectors = r_numDecalProjectors - r_firstSceneDecalProjector;
	tr.refdef.decalProjectors = &backEndData->decalProjectors[r_firstSceneDecalProjector];

	tr.refdef.numDecals = r_numDecals - r_firstSceneDecal;
	tr.refdef.decals = &backEndData->decals[r_firstSceneDecal];
#endif // RTCW_XX

	// turn off dynamic lighting globally by clearing all the

// BBi
//#if !defined RTCW_ET
//	// dlights if it needs to be disabled or if vertex lighting is enabled
//
//#if defined RTCW_SP
//	if ( /*r_dynamiclight->integer == 0 ||*/    // RF, disabled so we can force things like lightning dlights
//		r_vertexLight->integer == 1 ||
//#elif defined RTCW_MP
//	if ( /*r_dynamiclight->integer == 0 ||	// RF, disabled so we can force things like lightning dlights
//		 r_vertexLight->integer == 1 ||*/
//#endif // RTCW_XX
//
//		glConfig.hardwareType == GLHW_PERMEDIA2 ) {
//#else
//	// dlights if using permedia hw
//	if ( glConfig.hardwareType == GLHW_PERMEDIA2 ) {
//#endif // RTCW_XX

#if !defined RTCW_ET
	if (r_vertexLight->integer != 0) {
#else
	if (false) {
#endif // RTCW_XX
// BBi

		tr.refdef.num_dlights = 0;
	}

	// a single frame may have multiple scenes draw inside it --
	// a 3D game view, 3D status bar renderings, 3D menus, etc.
	// They need to be distinguished by the light flare code, because
	// the visibility state for a given surface may be different in
	// each scene / view.
	tr.frameSceneNum++;
	tr.sceneCount++;

	// setup view parms for the initial view
	//
	// set up viewport
	// The refdef takes 0-at-the-top y coordinates, so
	// convert to GL's 0-at-the-bottom space
	//
	memset( &parms, 0, sizeof( parms ) );
	parms.viewportX = tr.refdef.x;
	parms.viewportY = glConfig.vidHeight - ( tr.refdef.y + tr.refdef.height );
	parms.viewportWidth = tr.refdef.width;
	parms.viewportHeight = tr.refdef.height;
	parms.isPortal = qfalse;

	parms.fovX = tr.refdef.fov_x;
	parms.fovY = tr.refdef.fov_y;

// BBi
//#if !defined RTCW_ET
//	VectorCopy( fd->vieworg, parms.or.origin );
//	VectorCopy( fd->viewaxis[0], parms.or.axis[0] );
//	VectorCopy( fd->viewaxis[1], parms.or.axis[1] );
//	VectorCopy( fd->viewaxis[2], parms.or.axis[2] );
//#else
//	VectorCopy( fd->vieworg, parms.orientation.origin );
//	VectorCopy( fd->viewaxis[0], parms.orientation.axis[0] );
//	VectorCopy( fd->viewaxis[1], parms.orientation.axis[1] );
//	VectorCopy( fd->viewaxis[2], parms.orientation.axis[2] );
//#endif // RTCW_XX
	VectorCopy (fd->vieworg, parms.orientation.origin);
	VectorCopy (fd->viewaxis[0], parms.orientation.axis[0]);
	VectorCopy (fd->viewaxis[1], parms.orientation.axis[1]);
	VectorCopy (fd->viewaxis[2], parms.orientation.axis[2]);
// BBi

	VectorCopy( fd->vieworg, parms.pvsOrigin );

	R_RenderView( &parms );

	// the next scene rendered in this frame will tack on after this one
	r_firstSceneDrawSurf = tr.refdef.numDrawSurfs;
	r_firstSceneEntity = r_numentities;
	r_firstSceneDlight = r_numdlights;
	r_firstScenePoly = r_numpolys;

#if defined RTCW_ET
	r_firstScenePolybuffer = r_numpolybuffers;
#endif // RTCW_XX

	tr.frontEndMsec += ri.Milliseconds() - startTime;
}

#if defined RTCW_ET
// Temp storage for saving view paramters.  Drawing the animated head in the corner
// was creaming important view info.
viewParms_t g_oldViewParms;



/*
================
RE_SaveViewParms

Save out the old render info to a temp place so we don't kill the LOD system
when we do a second render.
================
*/
void RE_SaveViewParms() {
	// save old viewParms so we can return to it after the mirror view
	g_oldViewParms = tr.viewParms;
}


/*
================
RE_RestoreViewParms

Restore the old render info so we don't kill the LOD system
when we do a second render.
================
*/
void RE_RestoreViewParms() {

	// This was killing the LOD computation
	tr.viewParms = g_oldViewParms;


}
#endif // RTCW_XX

