/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// tr_mesh.c: triangle model functions
//
// !!! NOTE: Any changes made here must be duplicated in tr_cmesh.c for MDC support

#include "tr_local.h"

static float ProjectRadius( float r, vec3_t location ) {
	float pr;
	float dist;
	float c;
	vec3_t p;
	float projected[4];

// BBi
//#if !defined RTCW_ET
//	c = DotProduct( tr.viewParms.or.axis[0], tr.viewParms.or.origin );
//	dist = DotProduct( tr.viewParms.or.axis[0], location ) - c;
//#else
//	c = DotProduct( tr.viewParms.orientation.axis[0], tr.viewParms.orientation.origin );
//	dist = DotProduct( tr.viewParms.orientation.axis[0], location ) - c;
//#endif // RTCW_XX
	c = DotProduct (tr.viewParms.orientation.axis[0],
		tr.viewParms.orientation.origin);

	dist = DotProduct (tr.viewParms.orientation.axis[0], location) - c;
// BBi

	if ( dist <= 0 ) {
		return 0;
	}

	p[0] = 0;

#if !defined RTCW_ET
	p[1] = c::fabs( r );
#else
	p[1] = Q_fabs( r );
#endif // RTCW_XX

	p[2] = -dist;

	projected[0] = p[0] * tr.viewParms.projectionMatrix[0] +
				   p[1] * tr.viewParms.projectionMatrix[4] +
				   p[2] * tr.viewParms.projectionMatrix[8] +
				   tr.viewParms.projectionMatrix[12];

	projected[1] = p[0] * tr.viewParms.projectionMatrix[1] +
				   p[1] * tr.viewParms.projectionMatrix[5] +
				   p[2] * tr.viewParms.projectionMatrix[9] +
				   tr.viewParms.projectionMatrix[13];

	projected[2] = p[0] * tr.viewParms.projectionMatrix[2] +
				   p[1] * tr.viewParms.projectionMatrix[6] +
				   p[2] * tr.viewParms.projectionMatrix[10] +
				   tr.viewParms.projectionMatrix[14];

	projected[3] = p[0] * tr.viewParms.projectionMatrix[3] +
				   p[1] * tr.viewParms.projectionMatrix[7] +
				   p[2] * tr.viewParms.projectionMatrix[11] +
				   tr.viewParms.projectionMatrix[15];


	pr = projected[1] / projected[3];

	if ( pr > 1.0f ) {
		pr = 1.0f;
	}

	return pr;
}

/*
=============
R_CullModel
=============
*/
static int R_CullModel( md3Header_t *header, trRefEntity_t *ent ) {
	vec3_t bounds[2];
	md3Frame_t  *oldFrame, *newFrame;
	int i;

#if defined RTCW_SP
	qboolean cullSphere;    //----(SA)	added
	float radScale;

	cullSphere = qtrue;
#endif // RTCW_XX

	// compute frame pointers
	newFrame = ( md3Frame_t * )( ( byte * ) header + header->ofsFrames ) + ent->e.frame;
	oldFrame = ( md3Frame_t * )( ( byte * ) header + header->ofsFrames ) + ent->e.oldframe;

#if defined RTCW_SP
	radScale = 1.0f;

	if ( ent->e.nonNormalizedAxes ) {
		cullSphere = qfalse;    // by defalut, cull bounding sphere ONLY if this is not an upscaled entity

		// but allow the radius to be scaled if specified
//		if(ent->e.reFlags & REFLAG_SCALEDSPHERECULL) {
//			cullSphere = qtrue;
//			radScale = ent->e.radius;
//		}
	}

	if ( cullSphere ) {
		if ( ent->e.frame == ent->e.oldframe ) {
			switch ( R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius * radScale ) )
#else
	// cull bounding sphere ONLY if this is not an upscaled entity
	if ( !ent->e.nonNormalizedAxes ) {
		if ( ent->e.frame == ent->e.oldframe ) {
			switch ( R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius ) )
#endif // RTCW_XX

			{
			case CULL_OUT:
				tr.pc.c_sphere_cull_md3_out++;
				return CULL_OUT;

			case CULL_IN:
				tr.pc.c_sphere_cull_md3_in++;
				return CULL_IN;

			case CULL_CLIP:
				tr.pc.c_sphere_cull_md3_clip++;
				break;
			}
		} else
		{
			int sphereCull, sphereCullB;

#if defined RTCW_SP
			sphereCull  = R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius * radScale );
#else
			sphereCull  = R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius );
#endif // RTCW_XX

			if ( newFrame == oldFrame ) {
				sphereCullB = sphereCull;
			} else {

#if defined RTCW_SP
				sphereCullB = R_CullLocalPointAndRadius( oldFrame->localOrigin, oldFrame->radius * radScale );
#else
				sphereCullB = R_CullLocalPointAndRadius( oldFrame->localOrigin, oldFrame->radius );
#endif // RTCW_XX

			}

			if ( sphereCull == sphereCullB ) {
				if ( sphereCull == CULL_OUT ) {
					tr.pc.c_sphere_cull_md3_out++;
					return CULL_OUT;
				} else if ( sphereCull == CULL_IN )   {
					tr.pc.c_sphere_cull_md3_in++;
					return CULL_IN;
				} else
				{
					tr.pc.c_sphere_cull_md3_clip++;
				}
			}
		}
	}

	// calculate a bounding box in the current coordinate system
	for ( i = 0 ; i < 3 ; i++ ) {
		bounds[0][i] = oldFrame->bounds[0][i] < newFrame->bounds[0][i] ? oldFrame->bounds[0][i] : newFrame->bounds[0][i];
		bounds[1][i] = oldFrame->bounds[1][i] > newFrame->bounds[1][i] ? oldFrame->bounds[1][i] : newFrame->bounds[1][i];

#if defined RTCW_SP
		bounds[0][i] *= radScale;   //----(SA)	added
		bounds[1][i] *= radScale;   //----(SA)	added
#endif // RTCW_XX

	}

	switch ( R_CullLocalBox( bounds ) )
	{
	case CULL_IN:
		tr.pc.c_box_cull_md3_in++;
		return CULL_IN;
	case CULL_CLIP:
		tr.pc.c_box_cull_md3_clip++;
		return CULL_CLIP;
	case CULL_OUT:
	default:
		tr.pc.c_box_cull_md3_out++;
		return CULL_OUT;
	}
}


/*
=================
R_ComputeLOD

=================
*/
int R_ComputeLOD( trRefEntity_t *ent ) {
	float radius;
	float flod, lodscale;
	float projectedRadius;
	md3Frame_t *frame;
	int lod;

	if ( tr.currentModel->numLods < 2 ) {
		// model has only 1 LOD level, skip computations and bias
		lod = 0;
	} else
	{
		// multiple LODs exist, so compute projected bounding sphere
		// and use that as a criteria for selecting LOD

		// RF, checked for a forced lowest LOD
		if ( ent->e.reFlags & REFLAG_FORCE_LOD ) {
			return ( tr.currentModel->numLods - 1 );
		}

#if !defined RTCW_ET
		frame = ( md3Frame_t * )( ( ( unsigned char * ) tr.currentModel->md3[0] ) + tr.currentModel->md3[0]->ofsFrames );
#else
		frame = ( md3Frame_t * )( ( ( unsigned char * ) tr.currentModel->model.md3[0] ) + tr.currentModel->model.md3[0]->ofsFrames );
#endif // RTCW_XX

		frame += ent->e.frame;

		radius = RadiusFromBounds( frame->bounds[0], frame->bounds[1] );

		//----(SA)	testing
		if ( ent->e.reFlags & REFLAG_ORIENT_LOD ) {
			// right now this is for trees, and pushes the lod distance way in.
			// this is not the intended purpose, but is helpful for the new
			// terrain level that has loads of trees
//			radius = radius/2.0f;
		}
		//----(SA)	end

		if ( ( projectedRadius = ProjectRadius( radius, ent->e.origin ) ) != 0 ) {
			lodscale = r_lodscale->value;
			if ( lodscale > 20 ) {
				lodscale = 20;
			}
			flod = 1.0f - projectedRadius * lodscale;
		} else
		{
			// object intersects near view plane, e.g. view weapon
			flod = 0;
		}

		flod *= tr.currentModel->numLods;
		lod = myftol( flod );

		if ( lod < 0 ) {
			lod = 0;
		} else if ( lod >= tr.currentModel->numLods )   {
			lod = tr.currentModel->numLods - 1;
		}
	}

	lod += r_lodbias->integer;

	if ( lod >= tr.currentModel->numLods ) {
		lod = tr.currentModel->numLods - 1;
	}
	if ( lod < 0 ) {
		lod = 0;
	}

	return lod;
}

/*
=================
R_ComputeFogNum

=================
*/
static int R_ComputeFogNum( md3Header_t *header, trRefEntity_t *ent ) {
	int i, j;
	fog_t           *fog;
	md3Frame_t      *md3Frame;
	vec3_t localOrigin;

	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return 0;
	}

	// FIXME: non-normalized axis issues
	md3Frame = ( md3Frame_t * )( ( byte * ) header + header->ofsFrames ) + ent->e.frame;
	VectorAdd( ent->e.origin, md3Frame->localOrigin, localOrigin );
	for ( i = 1 ; i < tr.world->numfogs ; i++ ) {
		fog = &tr.world->fogs[i];
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( localOrigin[j] - md3Frame->radius >= fog->bounds[1][j] ) {
				break;
			}
			if ( localOrigin[j] + md3Frame->radius <= fog->bounds[0][j] ) {
				break;
			}
		}
		if ( j == 3 ) {
			return i;
		}
	}

	return 0;
}

/*
=================
R_AddMD3Surfaces

=================
*/
void R_AddMD3Surfaces( trRefEntity_t *ent ) {
	int i;
	md3Header_t     *header = 0;
	md3Surface_t    *surface = 0;
	md3Shader_t     *md3Shader = 0;
	shader_t        *shader = 0;
	int cull;
	int lod;
	int fogNum;
	qboolean personalModel;

	// don't add third_person objects if not in a portal
	personalModel = ( ent->e.renderfx & RF_THIRD_PERSON ) && !tr.viewParms.isPortal;

	if ( ent->e.renderfx & RF_WRAP_FRAMES ) {

#if !defined RTCW_ET
		ent->e.frame %= tr.currentModel->md3[0]->numFrames;
		ent->e.oldframe %= tr.currentModel->md3[0]->numFrames;
#else
		ent->e.frame %= tr.currentModel->model.md3[0]->numFrames;
		ent->e.oldframe %= tr.currentModel->model.md3[0]->numFrames;
#endif // RTCW_XX
	}

#if defined RTCW_ET
	//
	// compute LOD
	//
	if ( ent->e.renderfx & RF_FORCENOLOD ) {
		lod = 0;
	} else {
		lod = R_ComputeLOD( ent );
	}
#endif // RTCW_XX

	//
	// Validate the frames so there is no chance of a crash.
	// This will write directly into the entity structure, so
	// when the surfaces are rendered, they don't need to be
	// range checked again.
	//

#if !defined RTCW_ET
	if ( ( ent->e.frame >= tr.currentModel->md3[0]->numFrames )
		 || ( ent->e.frame < 0 )
		 || ( ent->e.oldframe >= tr.currentModel->md3[0]->numFrames )
		 || ( ent->e.oldframe < 0 ) ) {
		ri.Printf( PRINT_DEVELOPER, "R_AddMD3Surfaces: no such frame %d to %d for '%s'\n",
				   ent->e.oldframe, ent->e.frame,
				   tr.currentModel->name );
#else
	if ( ( ent->e.frame >= tr.currentModel->model.md3[lod]->numFrames )
		 || ( ent->e.frame < 0 )
		 || ( ent->e.oldframe >= tr.currentModel->model.md3[lod]->numFrames )
		 || ( ent->e.oldframe < 0 ) ) {
		ri.Printf( PRINT_DEVELOPER, "R_AddMD3Surfaces: no such frame %d to %d for '%s' (%d)\n",
				   ent->e.oldframe, ent->e.frame,
				   tr.currentModel->name,
				   tr.currentModel->model.md3[ lod ]->numFrames );
#endif // RTCW_XX

		ent->e.frame = 0;
		ent->e.oldframe = 0;
	}

#if !defined RTCW_ET
	//
	// compute LOD
	//
	lod = R_ComputeLOD( ent );
#endif // RTCW_XX

#if !defined RTCW_ET
	header = tr.currentModel->md3[lod];
#else
	header = tr.currentModel->model.md3[lod];
#endif // RTCW_XX

	//
	// cull the entire model if merged bounding box of both frames
	// is outside the view frustum.
	//
	cull = R_CullModel( header, ent );
	if ( cull == CULL_OUT ) {
		return;
	}

	//
	// set up lighting now that we know we aren't culled
	//
	if ( !personalModel || r_shadows->integer > 1 ) {
		R_SetupEntityLighting( &tr.refdef, ent );
	}

	//
	// see if we are in a fog volume
	//
	fogNum = R_ComputeFogNum( header, ent );

	//
	// draw all surfaces
	//
	surface = ( md3Surface_t * )( (byte *)header + header->ofsSurfaces );
	for ( i = 0 ; i < header->numSurfaces ; i++ ) {

#if defined RTCW_SP
		int j;

//----(SA)	blink will change to be an overlay rather than replacing the head texture.
//		think of it like batman's mask.  the polygons that have eye texture are duplicated
//		and the 'lids' rendered with polygonoffset shader parm over the top of the open eyes.  this gives
//		minimal overdraw/alpha blending/texture use without breaking the model and causing seams
		if ( !Q_stricmp( surface->name, "h_blink" ) ) {
			if ( !( ent->e.renderfx & RF_BLINK ) ) {
				surface = ( md3Surface_t * )( (byte *)surface + surface->ofsEnd );
				continue;
			}
		}
//----(SA)	end
#endif // RTCW_XX

		if ( ent->e.customShader ) {
			shader = R_GetShaderByHandle( ent->e.customShader );
		} else if ( ent->e.customSkin > 0 && ent->e.customSkin < tr.numSkins ) {
			skin_t *skin;

#if defined RTCW_ET
			int hash;
#endif // RTCW_XX

#if !defined RTCW_SP
			int j;
#endif // RTCW_XX

			skin = R_GetSkinByHandle( ent->e.customSkin );

			// match the surface name to something in the skin file
			shader = tr.defaultShader;

#if defined RTCW_SP
			for ( j = 0 ; j < skin->numSurfaces ; j++ ) {
				// the names have both been lowercased
				if ( !strcmp( skin->surfaces[j]->name, surface->name ) ) {
					shader = skin->surfaces[j]->shader;
					break;
				}
			}
#else
//----(SA)	added blink
			if ( ent->e.renderfx & RF_BLINK ) {

#if !defined RTCW_ET
				const char *s = va( "%s_b", surface->name );   // append '_b' for 'blink'
#else
				char *s = va( "%s_b", surface->name ); // append '_b' for 'blink'
#endif // RTCW_XX

#if defined RTCW_ET
				hash = Com_HashKey( s, strlen( s ) );
#endif // RTCW_XX

				for ( j = 0 ; j < skin->numSurfaces ; j++ ) {

#if defined RTCW_ET
					if ( hash != skin->surfaces[j]->hash ) {
						continue;
					}
#endif // RTCW_XX

					if ( !strcmp( skin->surfaces[j]->name, s ) ) {
						shader = skin->surfaces[j]->shader;
						break;
					}
				}
			}

			if ( shader == tr.defaultShader ) {    // blink reference in skin was not found

#if defined RTCW_ET
				hash = Com_HashKey( surface->name, sizeof( surface->name ) );
#endif // RTCW_XX

				for ( j = 0 ; j < skin->numSurfaces ; j++ ) {
					// the names have both been lowercased

#if defined RTCW_ET
					if ( hash != skin->surfaces[j]->hash ) {
						continue;
					}
#endif // RTCW_XX

					if ( !strcmp( skin->surfaces[j]->name, surface->name ) ) {
						shader = skin->surfaces[j]->shader;
						break;
					}
				}
			}
//----(SA)	end
#endif // RTCW_XX

			if ( shader == tr.defaultShader ) {
				ri.Printf( PRINT_DEVELOPER, "WARNING: no shader for surface %s in skin %s\n", surface->name, skin->name );
			} else if ( shader->defaultShader )     {
				ri.Printf( PRINT_DEVELOPER, "WARNING: shader %s in skin %s not found\n", shader->name, skin->name );
			}
		} else if ( surface->numShaders <= 0 ) {
			shader = tr.defaultShader;
		} else {
			md3Shader = ( md3Shader_t * )( (byte *)surface + surface->ofsShaders );
			md3Shader += ent->e.skinNum % surface->numShaders;
			shader = tr.shaders[ md3Shader->shaderIndex ];
		}


		// we will add shadows even if the main object isn't visible in the view

		// stencil shadows can't do personal models unless I polyhedron clip
		if ( !personalModel
			 && r_shadows->integer == 2
			 && fogNum == 0
			 && !( ent->e.renderfx & ( RF_NOSHADOW | RF_DEPTHHACK ) )
			 && shader->sort == SS_OPAQUE ) {

#if defined RTCW_SP
// GR - tessellate according to model capabilities
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), tr.shadowShader, 0, qfalse, tr.currentModel->ATI_tess );
#elif defined RTCW_MP
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), tr.shadowShader, 0, qfalse );
#else
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), tr.shadowShader, 0, 0, 0 );
#endif // RTCW_XX

		}

		// projection shadows work fine with personal models

#if defined RTCW_SP
//		if ( r_shadows->integer == 3
//			&& fogNum == 0
//			&& (ent->e.renderfx & RF_SHADOW_PLANE )
//			&& shader->sort == SS_OPAQUE ) {
//			R_AddDrawSurf( (void *)surface, tr.projectionShadowShader, 0, qfalse );
//		}
#else
		if ( r_shadows->integer == 3
			 && fogNum == 0
			 && ( ent->e.renderfx & RF_SHADOW_PLANE )
			 && shader->sort == SS_OPAQUE ) {

#if !defined RTCW_ET
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), tr.projectionShadowShader, 0, qfalse );
#else
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), tr.projectionShadowShader, 0, 0, 0 );
#endif // RTCW_XX

		}
#endif // RTCW_XX

#if !defined RTCW_ET
		// for testing polygon shadows (on /all/ models)
#endif // RTCW_XX

#if defined RTCW_SP
//		if ( r_shadows->integer == 4)
//			R_AddDrawSurf( (void *)surface, tr.projectionShadowShader, 0, qfalse );
#else
		// for testing polygon shadows (on /all/ models)
		if ( r_shadows->integer == 4 ) {

#if !defined RTCW_ET
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), tr.projectionShadowShader, 0, qfalse );
#else
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), tr.projectionShadowShader, 0, 0, 0 );
#endif // RTCW_XX

		}
#endif // RTCW_XX


		// don't add third_person objects if not viewing through a portal
		if ( !personalModel ) {

#if defined RTCW_SP
// GR - tessellate according to model capabilities
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), shader, fogNum, qfalse, tr.currentModel->ATI_tess );
#elif defined RTCW_MP
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), shader, fogNum, qfalse );
#else
			R_AddDrawSurf( reinterpret_cast<surfaceType_t*> (surface), shader, fogNum, 0, 0 );
#endif // RTCW_XX

		}

		surface = ( md3Surface_t * )( (byte *)surface + surface->ofsEnd );
	}

}

