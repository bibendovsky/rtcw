/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

/*
 * name:		tr_main.c
 *
 * desc:		main control flow for each frame
 *
*/

// BBi
#include <algorithm>
// BBi

#include "tr_local.h"

trGlobals_t tr;

static float s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};


refimport_t ri;

// entities that will have procedurally generated surfaces will just
// point at this for their sorting surface
surfaceType_t entitySurface = SF_ENTITY;

// fog stuff
glfog_t glfogsettings[NUM_FOGS];
glfogType_t glfogNum = FOG_NONE;
qboolean fogIsOn = qfalse;


/*
=================
R_Fog (void)
=================
*/
void R_Fog( glfog_t *curfog ) {
	static glfog_t setfog;

	if ( !r_wolffog->integer ) {
		R_FogOff();
		return;
	}

	if ( !curfog->registered ) {   //----(SA)
		R_FogOff();
		return;
	}

	//----(SA) assme values of '0' for these parameters means 'use default'
	if ( !curfog->density ) {
		curfog->density = 1;
	}
	if ( !curfog->hint ) {
		curfog->hint = GL_DONT_CARE;
	}
	if ( !curfog->mode ) {
		curfog->mode = GL_LINEAR;
	}
	//----(SA)	end


	R_FogOn();

	// only send changes if necessary

	// BBi
	if (!glConfigEx.is_path_ogl_1_x ()) {
		float fog_start = curfog->start;
		float fog_end = curfog->end;

#if defined RTCW_SP
		if ((backEnd.refdef.rdflags & RDF_SNOOPERVIEW) != 0)
			fog_start = curfog->end;
#endif // RTCW_XX

		if (r_zfar->value)
			fog_end = r_zfar->value;
		else {
#if defined RTCW_SP
			if ((backEnd.refdef.rdflags & RDF_SNOOPERVIEW) != 0)
				fog_end = curfog->end + 1000.0F;
#endif // RTCW_XX
		}

		ogl_tess_state.fog_mode = curfog->mode;
		ogl_tess_state.fog_dist_mode = glConfig.NVFogMode;
		ogl_tess_state.fog_hint = curfog->hint;
		ogl_tess_state.fog_density = curfog->density;
		ogl_tess_state.fog_start = fog_start;
		ogl_tess_state.fog_end = fog_end;

		ogl_tess_state.fog_color = rtcw::cgm::Vec4(
			curfog->color[0],
			curfog->color[1],
			curfog->color[2],
			curfog->color[3]);
	} else {
	// BBi

//	if(curfog->mode != setfog.mode || !setfog.registered) {
	glFogi( GL_FOG_MODE, curfog->mode );
//		setfog.mode = curfog->mode;
//	}
//	if(curfog->color[0] != setfog.color[0] || curfog->color[1] != setfog.color[1] || curfog->color[2] != setfog.color[2] || !setfog.registered) {
	glFogfv( GL_FOG_COLOR, curfog->color );
//		VectorCopy(setfog.color, curfog->color);
//	}
//	if(curfog->density != setfog.density || !setfog.registered) {
	glFogf( GL_FOG_DENSITY, curfog->density );
//		setfog.density = curfog->density;
//	}
//	if(curfog->hint != setfog.hint || !setfog.registered) {
	glHint( GL_FOG_HINT, curfog->hint );
//		setfog.hint = curfog->hint;
//	}
//	if(curfog->start != setfog.start || !setfog.registered) {

#if defined RTCW_SP
	if ( backEnd.refdef.rdflags & RDF_SNOOPERVIEW ) {
		glFogf( GL_FOG_START, curfog->end );       // snooper starts GL fog out further
	} else {
		glFogf( GL_FOG_START, curfog->start );
	}
#else
	glFogf( GL_FOG_START, curfog->start );
#endif // RTCW_XX

//		setfog.start = curfog->start;
//	}

	if ( r_zfar->value ) {             // (SA) allow override for helping level designers test fog distances
//		if(setfog.end != r_zfar->value || !setfog.registered) {
		glFogf( GL_FOG_END, r_zfar->value );
//			setfog.end = r_zfar->value;
//		}
	} else {
//		if(curfog->end != setfog.end || !setfog.registered) {

#if defined RTCW_SP
		if ( backEnd.refdef.rdflags & RDF_SNOOPERVIEW ) {
			glFogf( GL_FOG_END, curfog->end + 1000 );      // snooper ends GL fog out further.  this works fine with our maps, but could be 'funky' with later maps
		}
//				glFogf (GL_FOG_END, curfog->end);
		else {
			glFogf( GL_FOG_END, curfog->end );
		}
#else
		glFogf( GL_FOG_END, curfog->end );
#endif // RTCW_XX

//			setfog.end = curfog->end;
//		}
	}

#if defined RTCW_SP
//----(SA)	added
#else
// TTimo - from SP NV fog code
#endif // RTCW_XX

	// NV fog mode
	if ( glConfig.NVFogAvailable ) {
		glFogi( GL_FOG_DISTANCE_MODE_NV, glConfig.NVFogMode );
	}

#if defined RTCW_SP
//----(SA)	end
#else
// end
#endif // RTCW_XX

	// BBi
	}
	// BBi

	setfog.registered = qtrue;

	glClearColor( curfog->color[0], curfog->color[1], curfog->color[2], curfog->color[3] );
}

// Ridah, allow disabling fog temporarily
void R_FogOff( void ) {
	if ( !fogIsOn ) {
		return;
	}

	// BBi
	if (!glConfigEx.is_path_ogl_1_x ()) {
		ogl_tess_state.use_fog = false;
	} else {
	// BBi

	glDisable( GL_FOG );

	// BBi
	}
	// BBi

	fogIsOn = qfalse;
}

void R_FogOn( void ) {
	if ( fogIsOn ) {
		return;
	}

#if !defined RTCW_MP
//	if(r_uiFullScreen->integer) {	// don't fog in the menu
//		R_FogOff();
//		return;
//	}

#if !defined RTCW_ET
	if ( backEnd.projection2D ) {  // no fog in 2d
#endif // RTCW_XX

#else
	if ( r_uiFullScreen->integer ) {   // don't fog in the menu
#endif // RTCW_XX

#if !defined RTCW_ET
		R_FogOff();
		return;
	}
#endif // RTCW_XX

	if ( !r_wolffog->integer ) {
		return;
	}

//	if(backEnd.viewParms.isGLFogged) {
//		if(!(backEnd.viewParms.glFog.registered))
//			return;
//	}

	if ( backEnd.refdef.rdflags & RDF_SKYBOXPORTAL ) { // don't force world fog on portal sky
		if ( !( glfogsettings[FOG_PORTALVIEW].registered ) ) {
			return;
		}
	} else if ( !glfogNum )     {
		return;
	}

	// BBi
	if (!glConfigEx.is_path_ogl_1_x ()) {
		ogl_tess_state.use_fog = true;
	} else {
	// BBi

	glEnable( GL_FOG );

	// BBi
	}
	// BBi

	fogIsOn = qtrue;
}
// done.



//----(SA)
/*
==============
R_SetFog

  if fogvar == FOG_CMD_SWITCHFOG {
	fogvar is the command
	var1 is the fog to switch to
	var2 is the time to transition
  }
  else {
	fogvar is the fog that's being set
	var1 is the near fog z value
	var2 is the far fog z value
	rgb = color
	density is density, and is used to derive the values of 'mode', 'drawsky', and 'clearscreen'
  }
==============
*/
void R_SetFog( int fogvar, int var1, int var2, float r, float g, float b, float density ) {
	if ( fogvar != FOG_CMD_SWITCHFOG ) {   // just set the parameters and return

		if ( var1 == 0 && var2 == 0 ) {    // clear this fog
			glfogsettings[fogvar].registered = qfalse;
			return;
		}

#if !defined RTCW_ET
		glfogsettings[fogvar].color[0]      = r;
		glfogsettings[fogvar].color[1]      = g;
		glfogsettings[fogvar].color[2]      = b;
#else
		glfogsettings[fogvar].color[0]      = r * tr.identityLight;
		glfogsettings[fogvar].color[1]      = g * tr.identityLight;
		glfogsettings[fogvar].color[2]      = b * tr.identityLight;
#endif // RTCW_XX

		glfogsettings[fogvar].color[3]      = 1;
		glfogsettings[fogvar].start         = var1;
		glfogsettings[fogvar].end           = var2;

#if defined RTCW_SP
		if ( density >= 1 ) {
#else
		if ( density > 1 ) {
#endif // RTCW_XX

			glfogsettings[fogvar].mode          = GL_LINEAR;
			glfogsettings[fogvar].drawsky       = qfalse;
			glfogsettings[fogvar].clearscreen   = qtrue;
			glfogsettings[fogvar].density       = 1.0;
		} else
		{
			glfogsettings[fogvar].mode          = GL_EXP;
			glfogsettings[fogvar].drawsky       = qtrue;
			glfogsettings[fogvar].clearscreen   = qfalse;
			glfogsettings[fogvar].density       = density;
		}
		glfogsettings[fogvar].hint          = GL_DONT_CARE;
		glfogsettings[fogvar].registered    = qtrue;

		return;
	}

#if defined RTCW_SP
	// FOG_MAP now used to mean 'no fog'
	if ( var1 == FOG_MAP ) {

		// transitioning from...
		if ( glfogsettings[FOG_CURRENT].registered ) {
			memcpy( &glfogsettings[FOG_LAST], &glfogsettings[FOG_CURRENT], sizeof( glfog_t ) );
		}

		memcpy( &glfogsettings[FOG_TARGET], &glfogsettings[glfogNum], sizeof( glfog_t ) );


		// clear, clear, clear
		memset( &glfogsettings[FOG_MAP], 0, sizeof( glfog_t ) );
//		memset(&glfogsettings[FOG_CURRENT], 0, sizeof(glfog_t));
		memset( &glfogsettings[FOG_TARGET], 0, sizeof( glfog_t ) );
//		glfogsettings[FOG_CURRENT].registered = qfalse;
//		glfogsettings[FOG_TARGET].registered = qfalse;
		glfogNum = FOG_NONE;
		return;
	}
#endif // RTCW_XX

	// don't switch to invalid fogs
	if ( glfogsettings[var1].registered != qtrue ) {
		return;
	}

	glfogNum = glfogType_t (var1);

	// transitioning to new fog, store the current values as the 'from'

	if ( glfogsettings[FOG_CURRENT].registered ) {
		memcpy( &glfogsettings[FOG_LAST], &glfogsettings[FOG_CURRENT], sizeof( glfog_t ) );
	} else {
		// if no current fog fall back to world fog
		// FIXME: handle transition if there is no FOG_MAP fog

#if defined RTCW_SP
//		memcpy(&glfogsettings[FOG_LAST], &glfogsettings[FOG_MAP], sizeof(glfog_t));
		memcpy( &glfogsettings[FOG_LAST], &glfogsettings[glfogNum], sizeof( glfog_t ) );
#else
		memcpy( &glfogsettings[FOG_LAST], &glfogsettings[FOG_MAP], sizeof( glfog_t ) );
#endif // RTCW_XX

	}

	memcpy( &glfogsettings[FOG_TARGET], &glfogsettings[glfogNum], sizeof( glfog_t ) );

#if defined RTCW_SP
	if ( !var2 ) { // instant
		glfogsettings[FOG_TARGET].startTime = 0;
		glfogsettings[FOG_TARGET].finishTime = 0;
		glfogsettings[FOG_TARGET].dirty = 1;
		glfogsettings[FOG_CURRENT].dirty = 1;
	} else {
		// setup transition times
		glfogsettings[FOG_TARGET].startTime = tr.refdef.time;
		glfogsettings[FOG_TARGET].finishTime = tr.refdef.time + var2;
	}
#else
	// setup transition times
	glfogsettings[FOG_TARGET].startTime = tr.refdef.time;
	glfogsettings[FOG_TARGET].finishTime = tr.refdef.time + var2;
#endif // RTCW_XX

}

//----(SA) end

/*
=================
R_CullLocalBox

Returns CULL_IN, CULL_CLIP, or CULL_OUT
=================
*/
int R_CullLocalBox( vec3_t bounds[2] ) {
	int i, j;
	vec3_t transformed[8];
	float dists[8];
	vec3_t v;
	cplane_t    *frust;
	int anyBack;
	int front, back;

	if ( r_nocull->integer ) {
		return CULL_CLIP;
	}

	// transform into world space
	for ( i = 0 ; i < 8 ; i++ ) {
		v[0] = bounds[i & 1][0];
		v[1] = bounds[( i >> 1 ) & 1][1];
		v[2] = bounds[( i >> 2 ) & 1][2];

// BBi
//#if !defined RTCW_ET
//		VectorCopy( tr.or.origin, transformed[i] );
//		VectorMA( transformed[i], v[0], tr.or.axis[0], transformed[i] );
//		VectorMA( transformed[i], v[1], tr.or.axis[1], transformed[i] );
//		VectorMA( transformed[i], v[2], tr.or.axis[2], transformed[i] );
//#else
//		VectorCopy( tr.orientation.origin, transformed[i] );
//		VectorMA( transformed[i], v[0], tr.orientation.axis[0], transformed[i] );
//		VectorMA( transformed[i], v[1], tr.orientation.axis[1], transformed[i] );
//		VectorMA( transformed[i], v[2], tr.orientation.axis[2], transformed[i] );
//#endif // RTCW_XX
		VectorCopy (tr.orientation.origin, transformed[i]);
		VectorMA (transformed[i], v[0], tr.orientation.axis[0], transformed[i]);
		VectorMA (transformed[i], v[1], tr.orientation.axis[1], transformed[i]);
		VectorMA (transformed[i], v[2], tr.orientation.axis[2], transformed[i]);
// BBi
	}

	// check against frustum planes
	anyBack = 0;

#if !defined RTCW_ET
	for ( i = 0 ; i < 4 ; i++ ) {
#else
	for ( i = 0 ; i < 5 ; i++ ) {
#endif // RTCW_XX

		frust = &tr.viewParms.frustum[i];

		front = back = 0;
		for ( j = 0 ; j < 8 ; j++ ) {
			dists[j] = DotProduct( transformed[j], frust->normal );
			if ( dists[j] > frust->dist ) {
				front = 1;
				if ( back ) {
					break;      // a point is in front
				}
			} else {
				back = 1;
			}
		}
		if ( !front ) {
			// all points were behind one of the planes
			return CULL_OUT;
		}
		anyBack |= back;
	}

	if ( !anyBack ) {
		return CULL_IN;     // completely inside frustum
	}

	return CULL_CLIP;       // partially clipped
}

/*
** R_CullLocalPointAndRadius
*/
int R_CullLocalPointAndRadius( vec3_t pt, float radius ) {
	vec3_t transformed;

	R_LocalPointToWorld( pt, transformed );

	return R_CullPointAndRadius( transformed, radius );
}

/*
** R_CullPointAndRadius
*/
int R_CullPointAndRadius( vec3_t pt, float radius ) {
	int i;
	float dist;
	cplane_t    *frust;
	qboolean mightBeClipped = qfalse;

	if ( r_nocull->integer ) {
		return CULL_CLIP;
	}

	// check against frustum planes

#if !defined RTCW_ET
	for ( i = 0 ; i < 4 ; i++ )
#else
	for ( i = 0 ; i < 5 ; i++ )
#endif // RTCW_XX

	{
		frust = &tr.viewParms.frustum[i];

		dist = DotProduct( pt, frust->normal ) - frust->dist;
		if ( dist < -radius ) {
			return CULL_OUT;
		} else if ( dist <= radius )   {
			mightBeClipped = qtrue;
		}
	}

	if ( mightBeClipped ) {
		return CULL_CLIP;
	}

	return CULL_IN;     // completely inside frustum
}


/*
=================
R_LocalNormalToWorld

=================
*/
void R_LocalNormalToWorld( vec3_t local, vec3_t world ) {
// BBi
//#if !defined RTCW_ET
//	world[0] = local[0] * tr.or.axis[0][0] + local[1] * tr.or.axis[1][0] + local[2] * tr.or.axis[2][0];
//	world[1] = local[0] * tr.or.axis[0][1] + local[1] * tr.or.axis[1][1] + local[2] * tr.or.axis[2][1];
//	world[2] = local[0] * tr.or.axis[0][2] + local[1] * tr.or.axis[1][2] + local[2] * tr.or.axis[2][2];
//#else
//	world[0] = local[0] * tr.orientation.axis[0][0] + local[1] * tr.orientation.axis[1][0] + local[2] * tr.orientation.axis[2][0];
//	world[1] = local[0] * tr.orientation.axis[0][1] + local[1] * tr.orientation.axis[1][1] + local[2] * tr.orientation.axis[2][1];
//	world[2] = local[0] * tr.orientation.axis[0][2] + local[1] * tr.orientation.axis[1][2] + local[2] * tr.orientation.axis[2][2];
//#endif // RTCW_XX
	world[0] = (local[0] * tr.orientation.axis[0][0]) +
		(local[1] * tr.orientation.axis[1][0]) +
		(local[2] * tr.orientation.axis[2][0]);

	world[1] = (local[0] * tr.orientation.axis[0][1]) +
		(local[1] * tr.orientation.axis[1][1]) +
		(local[2] * tr.orientation.axis[2][1]);

	world[2] = (local[0] * tr.orientation.axis[0][2]) +
		(local[1] * tr.orientation.axis[1][2]) +
		(local[2] * tr.orientation.axis[2][2]);
// BBi
}

/*
=================
R_LocalPointToWorld

=================
*/
void R_LocalPointToWorld( vec3_t local, vec3_t world ) {
// BBi
//#if !defined RTCW_ET
//	world[0] = local[0] * tr.or.axis[0][0] + local[1] * tr.or.axis[1][0] + local[2] * tr.or.axis[2][0] + tr.or.origin[0];
//	world[1] = local[0] * tr.or.axis[0][1] + local[1] * tr.or.axis[1][1] + local[2] * tr.or.axis[2][1] + tr.or.origin[1];
//	world[2] = local[0] * tr.or.axis[0][2] + local[1] * tr.or.axis[1][2] + local[2] * tr.or.axis[2][2] + tr.or.origin[2];
//#else
//	world[0] = local[0] * tr.orientation.axis[0][0] + local[1] * tr.orientation.axis[1][0] + local[2] * tr.orientation.axis[2][0] + tr.orientation.origin[0];
//	world[1] = local[0] * tr.orientation.axis[0][1] + local[1] * tr.orientation.axis[1][1] + local[2] * tr.orientation.axis[2][1] + tr.orientation.origin[1];
//	world[2] = local[0] * tr.orientation.axis[0][2] + local[1] * tr.orientation.axis[1][2] + local[2] * tr.orientation.axis[2][2] + tr.orientation.origin[2];
//#endif // RTCW_XX
	world[0] = (local[0] * tr.orientation.axis[0][0]) +
		(local[1] * tr.orientation.axis[1][0]) +
		(local[2] * tr.orientation.axis[2][0]) +
		tr.orientation.origin[0];

	world[1] = (local[0] * tr.orientation.axis[0][1]) +
		(local[1] * tr.orientation.axis[1][1]) +
		(local[2] * tr.orientation.axis[2][1]) +
		tr.orientation.origin[1];

	world[2] = (local[0] * tr.orientation.axis[0][2]) +
		(local[1] * tr.orientation.axis[1][2]) +
		(local[2] * tr.orientation.axis[2][2]) +
		tr.orientation.origin[2];
// BBi
}

/*
=================
R_WorldToLocal

=================
*/
void R_WorldToLocal( vec3_t world, vec3_t local ) {
// BBi
//#if !defined RTCW_ET
//	local[0] = DotProduct( world, tr.or.axis[0] );
//	local[1] = DotProduct( world, tr.or.axis[1] );
//	local[2] = DotProduct( world, tr.or.axis[2] );
//#else
//	local[0] = DotProduct( world, tr.orientation.axis[0] );
//	local[1] = DotProduct( world, tr.orientation.axis[1] );
//	local[2] = DotProduct( world, tr.orientation.axis[2] );
//#endif // RTCW_XX
	local[0] = DotProduct (world, tr.orientation.axis[0]);
	local[1] = DotProduct (world, tr.orientation.axis[1]);
	local[2] = DotProduct (world, tr.orientation.axis[2]);
// BBi
}

/*
==========================
R_TransformModelToClip

==========================
*/
void R_TransformModelToClip( const vec3_t src, const float *modelMatrix, const float *projectionMatrix,
							 vec4_t eye, vec4_t dst ) {
	int i;

	for ( i = 0 ; i < 4 ; i++ ) {
		eye[i] =
			src[0] * modelMatrix[ i + 0 * 4 ] +
			src[1] * modelMatrix[ i + 1 * 4 ] +
			src[2] * modelMatrix[ i + 2 * 4 ] +
			1 * modelMatrix[ i + 3 * 4 ];
	}

	for ( i = 0 ; i < 4 ; i++ ) {
		dst[i] =
			eye[0] * projectionMatrix[ i + 0 * 4 ] +
			eye[1] * projectionMatrix[ i + 1 * 4 ] +
			eye[2] * projectionMatrix[ i + 2 * 4 ] +
			eye[3] * projectionMatrix[ i + 3 * 4 ];
	}
}

/*
==========================
R_TransformClipToWindow

==========================
*/
void R_TransformClipToWindow( const vec4_t clip, const viewParms_t *view, vec4_t normalized, vec4_t window ) {
	normalized[0] = clip[0] / clip[3];
	normalized[1] = clip[1] / clip[3];
	normalized[2] = ( clip[2] + clip[3] ) / ( 2 * clip[3] );

	window[0] = 0.5f * ( 1.0f + normalized[0] ) * view->viewportWidth;
	window[1] = 0.5f * ( 1.0f + normalized[1] ) * view->viewportHeight;
	window[2] = normalized[2];

	window[0] = (int) ( window[0] + 0.5 );
	window[1] = (int) ( window[1] + 0.5 );
}


/*
==========================
myGlMultMatrix

==========================
*/
void myGlMultMatrix( const float *a, const float *b, float *out ) {
	int i, j;

	for ( i = 0 ; i < 4 ; i++ ) {
		for ( j = 0 ; j < 4 ; j++ ) {
			out[ i * 4 + j ] =
				a [ i * 4 + 0 ] * b [ 0 * 4 + j ]
				+ a [ i * 4 + 1 ] * b [ 1 * 4 + j ]
				+ a [ i * 4 + 2 ] * b [ 2 * 4 + j ]
				+ a [ i * 4 + 3 ] * b [ 3 * 4 + j ];
		}
	}
}

/*
=================
R_RotateForEntity

Generates an orientation for an entity and viewParms
Does NOT produce any GL calls
Called by both the front end and the back end
=================
*/
void R_RotateForEntity( const trRefEntity_t *ent, const viewParms_t *viewParms,
						orientationr_t *orient ) {
	float glMatrix[16];
	vec3_t delta;
	float axisLength;

	if ( ent->e.reType != RT_MODEL ) {
		*orient = viewParms->world;
		return;
	}

	VectorCopy( ent->e.origin, orient->origin );

	VectorCopy( ent->e.axis[0], orient->axis[0] );
	VectorCopy( ent->e.axis[1], orient->axis[1] );
	VectorCopy( ent->e.axis[2], orient->axis[2] );

	glMatrix[0] = orient->axis[0][0];
	glMatrix[4] = orient->axis[1][0];
	glMatrix[8] = orient->axis[2][0];
	glMatrix[12] = orient->origin[0];

	glMatrix[1] = orient->axis[0][1];
	glMatrix[5] = orient->axis[1][1];
	glMatrix[9] = orient->axis[2][1];
	glMatrix[13] = orient->origin[1];

	glMatrix[2] = orient->axis[0][2];
	glMatrix[6] = orient->axis[1][2];
	glMatrix[10] = orient->axis[2][2];
	glMatrix[14] = orient->origin[2];

	glMatrix[3] = 0;
	glMatrix[7] = 0;
	glMatrix[11] = 0;
	glMatrix[15] = 1;

	myGlMultMatrix( glMatrix, viewParms->world.modelMatrix, orient->modelMatrix );

	// calculate the viewer origin in the model's space
	// needed for fog, specular, and environment mapping

// BBi
//#if !defined RTCW_ET
//	VectorSubtract( viewParms->or.origin, or->origin, delta );
//#else
//	VectorSubtract( viewParms->orientation.origin, or->origin, delta );
//#endif // RTCW_XX
	VectorSubtract (viewParms->orientation.origin, orient->origin, delta);
// BBi

	// compensate for scale in the axes if necessary
	if ( ent->e.nonNormalizedAxes ) {
		axisLength = VectorLength( ent->e.axis[0] );
		if ( !axisLength ) {
			axisLength = 0;
		} else {
			axisLength = 1.0f / axisLength;
		}
	} else {
		axisLength = 1.0f;
	}

	orient->viewOrigin[0] = DotProduct( delta, orient->axis[0] ) * axisLength;
	orient->viewOrigin[1] = DotProduct( delta, orient->axis[1] ) * axisLength;
	orient->viewOrigin[2] = DotProduct( delta, orient->axis[2] ) * axisLength;
}

/*
=================
R_RotateForViewer

Sets up the modelview matrix for a given viewParm
=================
*/
void R_RotateForViewer( void ) {
	float viewerMatrix[16];
	vec3_t origin;

// BBi
//#if !defined RTCW_ET
//	memset( &tr.or, 0, sizeof( tr.or ) );
//	tr.or.axis[0][0] = 1;
//	tr.or.axis[1][1] = 1;
//	tr.or.axis[2][2] = 1;
//	VectorCopy( tr.viewParms.or.origin, tr.or.viewOrigin );
//
//	// transform by the camera placement
//	VectorCopy( tr.viewParms.or.origin, origin );
//
//	viewerMatrix[0] = tr.viewParms.or.axis[0][0];
//	viewerMatrix[4] = tr.viewParms.or.axis[0][1];
//	viewerMatrix[8] = tr.viewParms.or.axis[0][2];
//	viewerMatrix[12] = -origin[0] * viewerMatrix[0] + - origin[1] * viewerMatrix[4] + - origin[2] * viewerMatrix[8];
//
//	viewerMatrix[1] = tr.viewParms.or.axis[1][0];
//	viewerMatrix[5] = tr.viewParms.or.axis[1][1];
//	viewerMatrix[9] = tr.viewParms.or.axis[1][2];
//	viewerMatrix[13] = -origin[0] * viewerMatrix[1] + - origin[1] * viewerMatrix[5] + - origin[2] * viewerMatrix[9];
//
//	viewerMatrix[2] = tr.viewParms.or.axis[2][0];
//	viewerMatrix[6] = tr.viewParms.or.axis[2][1];
//	viewerMatrix[10] = tr.viewParms.or.axis[2][2];
//#else
//	memset( &tr.orientation, 0, sizeof( tr.orientation ) );
//	tr.orientation.axis[0][0] = 1;
//	tr.orientation.axis[1][1] = 1;
//	tr.orientation.axis[2][2] = 1;
//	VectorCopy( tr.viewParms.orientation.origin, tr.orientation.viewOrigin );
//
//	// transform by the camera placement
//	VectorCopy( tr.viewParms.orientation.origin, origin );
//
//	viewerMatrix[0] = tr.viewParms.orientation.axis[0][0];
//	viewerMatrix[4] = tr.viewParms.orientation.axis[0][1];
//	viewerMatrix[8] = tr.viewParms.orientation.axis[0][2];
//	viewerMatrix[12] = -origin[0] * viewerMatrix[0] + - origin[1] * viewerMatrix[4] + - origin[2] * viewerMatrix[8];
//
//	viewerMatrix[1] = tr.viewParms.orientation.axis[1][0];
//	viewerMatrix[5] = tr.viewParms.orientation.axis[1][1];
//	viewerMatrix[9] = tr.viewParms.orientation.axis[1][2];
//	viewerMatrix[13] = -origin[0] * viewerMatrix[1] + - origin[1] * viewerMatrix[5] + - origin[2] * viewerMatrix[9];
//
//	viewerMatrix[2] = tr.viewParms.orientation.axis[2][0];
//	viewerMatrix[6] = tr.viewParms.orientation.axis[2][1];
//	viewerMatrix[10] = tr.viewParms.orientation.axis[2][2];
//#endif // RTCW_XX
	memset (&::tr.orientation, 0, sizeof (tr.orientation));
	tr.orientation.axis[0][0] = 1;
	tr.orientation.axis[1][1] = 1;
	tr.orientation.axis[2][2] = 1;
	VectorCopy (tr.viewParms.orientation.origin,
		tr.orientation.viewOrigin);

	// transform by the camera placement
	VectorCopy (tr.viewParms.orientation.origin, origin);

	viewerMatrix[0] = tr.viewParms.orientation.axis[0][0];
	viewerMatrix[4] = tr.viewParms.orientation.axis[0][1];
	viewerMatrix[8] = tr.viewParms.orientation.axis[0][2];
	viewerMatrix[12] = (-origin[0] * viewerMatrix[0]) +
		(-origin[1] * viewerMatrix[4]) + (-origin[2] * viewerMatrix[8]);

	viewerMatrix[1] = tr.viewParms.orientation.axis[1][0];
	viewerMatrix[5] = tr.viewParms.orientation.axis[1][1];
	viewerMatrix[9] = tr.viewParms.orientation.axis[1][2];
	viewerMatrix[13] = (-origin[0] * viewerMatrix[1]) +
		(-origin[1] * viewerMatrix[5]) + (-origin[2] * viewerMatrix[9]);

	viewerMatrix[2] = tr.viewParms.orientation.axis[2][0];
	viewerMatrix[6] = tr.viewParms.orientation.axis[2][1];
	viewerMatrix[10] = tr.viewParms.orientation.axis[2][2];
// BBi

	viewerMatrix[14] = -origin[0] * viewerMatrix[2] + - origin[1] * viewerMatrix[6] + - origin[2] * viewerMatrix[10];

	viewerMatrix[3] = 0;
	viewerMatrix[7] = 0;
	viewerMatrix[11] = 0;
	viewerMatrix[15] = 1;

	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)

// BBi
//#if !defined RTCW_ET
//	myGlMultMatrix( viewerMatrix, s_flipMatrix, tr.or.modelMatrix );
//
//	tr.viewParms.world = tr.or;
//#else
//	myGlMultMatrix( viewerMatrix, s_flipMatrix, tr.orientation.modelMatrix );
//
//	tr.viewParms.world = tr.orientation;
//#endif // RTCW_XX
	myGlMultMatrix (viewerMatrix, s_flipMatrix, tr.orientation.modelMatrix);
	tr.viewParms.world = tr.orientation;
// BBi
}



/*
==============
R_SetFrameFog
==============
*/
void R_SetFrameFog( void ) {

#if defined RTCW_ET
	// Arnout: new style global fog transitions
	if ( tr.world->globalFogTransEndTime ) {
		if ( tr.world->globalFogTransEndTime >= tr.refdef.time ) {
			float lerpPos;
			int fadeTime;

			fadeTime = tr.world->globalFogTransEndTime - tr.world->globalFogTransStartTime;
			lerpPos = (float)( tr.refdef.time - tr.world->globalFogTransStartTime ) / (float)fadeTime;
			if ( lerpPos > 1 ) {
				lerpPos = 1;
			}

			tr.world->fogs[tr.world->globalFog].shader->fogParms.color[ 0 ] = tr.world->globalTransStartFog[ 0 ] + ( ( tr.world->globalTransEndFog[ 0 ] - tr.world->globalTransStartFog[ 0 ] ) * lerpPos );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.color[ 1 ] = tr.world->globalTransStartFog[ 1 ] + ( ( tr.world->globalTransEndFog[ 1 ] - tr.world->globalTransStartFog[ 1 ] ) * lerpPos );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.color[ 2 ] = tr.world->globalTransStartFog[ 2 ] + ( ( tr.world->globalTransEndFog[ 2 ] - tr.world->globalTransStartFog[ 2 ] ) * lerpPos );

			tr.world->fogs[tr.world->globalFog].shader->fogParms.colorInt = ColorBytes4( tr.world->fogs[tr.world->globalFog].shader->fogParms.color[ 0 ] * tr.identityLight,
																						 tr.world->fogs[tr.world->globalFog].shader->fogParms.color[ 1 ] * tr.identityLight,
																						 tr.world->fogs[tr.world->globalFog].shader->fogParms.color[ 2 ] * tr.identityLight, 1.0 );

			tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque = tr.world->globalTransStartFog[ 3 ] + ( ( tr.world->globalTransEndFog[ 3 ] - tr.world->globalTransStartFog[ 3 ] ) * lerpPos );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.tcScale = 1.0f / ( tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque * 8 );
		} else {
			// transition complete
			VectorCopy( tr.world->globalTransEndFog, tr.world->fogs[tr.world->globalFog].shader->fogParms.color );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.colorInt = ColorBytes4( tr.world->globalTransEndFog[ 0 ] * tr.identityLight,
																						 tr.world->globalTransEndFog[ 1 ] * tr.identityLight,
																						 tr.world->globalTransEndFog[ 2 ] * tr.identityLight, 1.0 );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque = tr.world->globalTransEndFog[ 3 ];
			tr.world->fogs[tr.world->globalFog].shader->fogParms.tcScale = 1.0f / ( tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque * 8 );

			tr.world->globalFogTransEndTime = 0;
		}
	}
#endif // RTCW_XX

	if ( r_speeds->integer == 5 ) {
		if ( !glfogsettings[FOG_TARGET].registered ) {
			ri.Printf( PRINT_ALL, "no fog - calc zFar: %0.1f\n", tr.viewParms.zFar );
			return;
		}
	}

#if !defined RTCW_SP
	// DHM - Nerve :: If fog is not valid, don't use it
	if ( !glfogsettings[FOG_TARGET].registered ) {
		return;
	}
#endif // RTCW_XX

	// still fading
	if ( glfogsettings[FOG_TARGET].finishTime && glfogsettings[FOG_TARGET].finishTime >= tr.refdef.time ) {
		float lerpPos;
		int fadeTime;

		// transitioning from density to distance
		if ( glfogsettings[FOG_LAST].mode == GL_EXP && glfogsettings[FOG_TARGET].mode == GL_LINEAR ) {
			// for now just fast transition to the target when dissimilar fogs are
			memcpy( &glfogsettings[FOG_CURRENT], &glfogsettings[FOG_TARGET], sizeof( glfog_t ) );
			glfogsettings[FOG_TARGET].finishTime = 0;
		}
		// transitioning from distance to density
		else if ( glfogsettings[FOG_LAST].mode == GL_LINEAR && glfogsettings[FOG_TARGET].mode == GL_EXP ) {
			memcpy( &glfogsettings[FOG_CURRENT], &glfogsettings[FOG_TARGET], sizeof( glfog_t ) );
			glfogsettings[FOG_TARGET].finishTime = 0;
		}
		// transitioning like fog modes
		else {

			fadeTime = glfogsettings[FOG_TARGET].finishTime - glfogsettings[FOG_TARGET].startTime;
			if ( fadeTime <= 0 ) {
				fadeTime = 1;   // avoid divide by zero


			}
			lerpPos = (float)( tr.refdef.time - glfogsettings[FOG_TARGET].startTime ) / (float)fadeTime;
			if ( lerpPos > 1 ) {
				lerpPos = 1;
			}

			// lerp near/far
			glfogsettings[FOG_CURRENT].start        = glfogsettings[FOG_LAST].start + ( ( glfogsettings[FOG_TARGET].start - glfogsettings[FOG_LAST].start ) * lerpPos );
			glfogsettings[FOG_CURRENT].end          = glfogsettings[FOG_LAST].end + ( ( glfogsettings[FOG_TARGET].end - glfogsettings[FOG_LAST].end ) * lerpPos );

			// lerp color
			glfogsettings[FOG_CURRENT].color[0]     = glfogsettings[FOG_LAST].color[0] + ( ( glfogsettings[FOG_TARGET].color[0] - glfogsettings[FOG_LAST].color[0] ) * lerpPos );
			glfogsettings[FOG_CURRENT].color[1]     = glfogsettings[FOG_LAST].color[1] + ( ( glfogsettings[FOG_TARGET].color[1] - glfogsettings[FOG_LAST].color[1] ) * lerpPos );
			glfogsettings[FOG_CURRENT].color[2]     = glfogsettings[FOG_LAST].color[2] + ( ( glfogsettings[FOG_TARGET].color[2] - glfogsettings[FOG_LAST].color[2] ) * lerpPos );

			glfogsettings[FOG_CURRENT].density      = glfogsettings[FOG_TARGET].density;
			glfogsettings[FOG_CURRENT].mode         = glfogsettings[FOG_TARGET].mode;
			glfogsettings[FOG_CURRENT].registered   = qtrue;

			// if either fog in the transition clears the screen, clear the background this frame to avoid hall of mirrors
			glfogsettings[FOG_CURRENT].clearscreen  = ( glfogsettings[FOG_TARGET].clearscreen || glfogsettings[FOG_LAST].clearscreen );
		}

#if defined RTCW_SP
		glfogsettings[FOG_CURRENT].dirty = 1;
#endif // RTCW_XX

	} else {

#if !defined RTCW_SP
		// probably usually not necessary to copy the whole thing.
#endif // RTCW_XX

		// potential FIXME: since this is the most common occurance, diff first and only set changes

#if defined RTCW_SP
//		if(glfogsettings[FOG_CURRENT].dirty) {
#endif // RTCW_XX

		memcpy( &glfogsettings[FOG_CURRENT], &glfogsettings[FOG_TARGET], sizeof( glfog_t ) );

#if defined RTCW_SP
		glfogsettings[FOG_CURRENT].dirty = 0;
//		}
#endif // RTCW_XX

	}


	// shorten the far clip if the fog opaque distance is closer than the procedural farcip dist

	if ( glfogsettings[FOG_CURRENT].mode == GL_LINEAR ) {
		if ( glfogsettings[FOG_CURRENT].end < tr.viewParms.zFar ) {
			tr.viewParms.zFar = glfogsettings[FOG_CURRENT].end;
		}

#if defined RTCW_SP
		if ( backEnd.refdef.rdflags & RDF_SNOOPERVIEW ) {
			tr.viewParms.zFar += 1000;  // zfar out slightly further for snooper.  this works fine with our maps, but could be 'funky' with later maps

		}
#endif // RTCW_XX

	}
//	else
//		glfogsettings[FOG_CURRENT].end = 5;


	if ( r_speeds->integer == 5 ) {
		if ( glfogsettings[FOG_CURRENT].mode == GL_LINEAR ) {
			ri.Printf( PRINT_ALL, "farclip fog - den: %0.1f  calc zFar: %0.1f  fog zfar: %0.1f\n", glfogsettings[FOG_CURRENT].density, tr.viewParms.zFar, glfogsettings[FOG_CURRENT].end );
		} else {

#if defined RTCW_SP
			ri.Printf( PRINT_ALL, "density fog - den: %0.6f  calc zFar: %0.1f  fog zFar: %0.1f\n", glfogsettings[FOG_CURRENT].density, tr.viewParms.zFar, glfogsettings[FOG_CURRENT].end );
#else
			ri.Printf( PRINT_ALL, "density fog - den: %0.4f  calc zFar: %0.1f  fog zFar: %0.1f\n", glfogsettings[FOG_CURRENT].density, tr.viewParms.zFar, glfogsettings[FOG_CURRENT].end );
#endif // RTCW_XX

		}
	}
}


/*
==============
SetFarClip
==============
*/
static void SetFarClip( void ) {
	float farthestCornerDistance = 0;
	int i;

	// if not rendering the world (icons, menus, etc)
	// set a 2k far clip plane
	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		tr.viewParms.zFar = 2048;
		return;
	}

	//----(SA)	this lets you use r_zfar from the command line to experiment with different
	//			distances, but setting it back to 0 uses the map (or procedurally generated) default
	if ( r_zfar->value ) {

		tr.viewParms.zFar = r_zfar->integer;
		R_SetFrameFog();

		if ( r_speeds->integer == 5 ) {
			ri.Printf( PRINT_ALL, "r_zfar value forcing farclip at: %f\n", tr.viewParms.zFar );
		}

		return;
	}

	//
	// set far clipping planes dynamically
	//
	farthestCornerDistance = 0;
	for ( i = 0; i < 8; i++ )
	{
		vec3_t v;
		vec3_t vecTo;
		float distance;

		if ( i & 1 ) {
			v[0] = tr.viewParms.visBounds[0][0];
		} else
		{
			v[0] = tr.viewParms.visBounds[1][0];
		}

		if ( i & 2 ) {
			v[1] = tr.viewParms.visBounds[0][1];
		} else
		{
			v[1] = tr.viewParms.visBounds[1][1];
		}

		if ( i & 4 ) {
			v[2] = tr.viewParms.visBounds[0][2];
		} else
		{
			v[2] = tr.viewParms.visBounds[1][2];
		}

// BBi
//#if !defined RTCW_ET
//		VectorSubtract( v, tr.viewParms.or.origin, vecTo );
//#else
//		VectorSubtract( v, tr.viewParms.orientation.origin, vecTo );
//#endif // RTCW_XX
		VectorSubtract (v, tr.viewParms.orientation.origin, vecTo);
// BBi

		distance = vecTo[0] * vecTo[0] + vecTo[1] * vecTo[1] + vecTo[2] * vecTo[2];

		if ( distance > farthestCornerDistance ) {
			farthestCornerDistance = distance;
		}
	}

	tr.viewParms.zFar = c::sqrt( farthestCornerDistance );

#if defined RTCW_ET
	// ydnar: add global q3 fog
	if ( tr.world != NULL && tr.world->globalFog >= 0 && tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque < tr.viewParms.zFar ) {
		tr.viewParms.zFar = tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque;
	}
#endif // RTCW_XX

	R_SetFrameFog();

}

#if defined RTCW_ET
/*
=================
R_SetupFrustum

Setup that culling frustum planes for the current view
=================
*/
void R_SetupFrustum( void ) {
	int i;
	float xs, xc;
	float ang;

	ang = tr.viewParms.fovX / 180 * M_PI * 0.5f;
	xs = c::sin( ang );
	xc = c::cos( ang );

	VectorScale( tr.viewParms.orientation.axis[0], xs, tr.viewParms.frustum[0].normal );
	VectorMA( tr.viewParms.frustum[0].normal, xc, tr.viewParms.orientation.axis[1], tr.viewParms.frustum[0].normal );

	VectorScale( tr.viewParms.orientation.axis[0], xs, tr.viewParms.frustum[1].normal );
	VectorMA( tr.viewParms.frustum[1].normal, -xc, tr.viewParms.orientation.axis[1], tr.viewParms.frustum[1].normal );

	ang = tr.viewParms.fovY / 180 * M_PI * 0.5f;
	xs = c::sin( ang );
	xc = c::cos( ang );

	VectorScale( tr.viewParms.orientation.axis[0], xs, tr.viewParms.frustum[2].normal );
	VectorMA( tr.viewParms.frustum[2].normal, xc, tr.viewParms.orientation.axis[2], tr.viewParms.frustum[2].normal );

	VectorScale( tr.viewParms.orientation.axis[0], xs, tr.viewParms.frustum[3].normal );
	VectorMA( tr.viewParms.frustum[3].normal, -xc, tr.viewParms.orientation.axis[2], tr.viewParms.frustum[3].normal );

	for ( i = 0; i < 4; i++ )
	{
		tr.viewParms.frustum[i].type = PLANE_NON_AXIAL;
		tr.viewParms.frustum[i].dist = DotProduct( tr.viewParms.orientation.origin, tr.viewParms.frustum[i].normal );
		SetPlaneSignbits( &tr.viewParms.frustum[i] );
	}

	// ydnar: farplane (testing! use farplane for real)
	VectorScale( tr.viewParms.orientation.axis[ 0 ], -1, tr.viewParms.frustum[ 4 ].normal );
	tr.viewParms.frustum[ 4 ].dist = DotProduct( tr.viewParms.orientation.origin, tr.viewParms.frustum[ 4 ].normal ) - tr.viewParms.zFar;
	tr.viewParms.frustum[ 4 ].type = PLANE_NON_AXIAL;
	SetPlaneSignbits( &tr.viewParms.frustum[ 4 ] );
}
#endif // RTCW_XX



/*
===============
R_SetupProjection
===============
*/
void R_SetupProjection( void ) {
	float xmin, xmax, ymin, ymax;
	float width, height, depth;
	float zNear, zFar;

	// dynamically compute far clip plane distance
	SetFarClip();

#if defined RTCW_ET
	// ydnar: set frustum planes (this happens here because of zfar manipulation)
	R_SetupFrustum();
#endif // RTCW_XX

	//
	// set up projection matrix
	//
	zNear   = r_znear->value;

#if defined RTCW_ET
	// ydnar: high fov values let players see through walls
	// solution is to move z near plane inward, which decreases zbuffer precision
	// but if a player wants to play with fov 160, then they can deal with z-fighting
	// assume fov 90 = scale 1, fov 180 = scale 1/16th
	if ( tr.refdef.fov_x > 90.0f ) {
		zNear /= ( ( tr.refdef.fov_x - 90.0f ) * 0.09f + 1.0f );
	}
#endif // RTCW_XX

	if ( r_zfar->value ) {
		zFar = r_zfar->value;   // (SA) allow override for helping level designers test fog distances
	} else {
		zFar = tr.viewParms.zFar;
	}

	ymax = zNear * c::tan( tr.refdef.fov_y * M_PI / 360.0f );
	ymin = -ymax;

	xmax = zNear * c::tan( tr.refdef.fov_x * M_PI / 360.0f );
	xmin = -xmax;

	width = xmax - xmin;
	height = ymax - ymin;
	depth = zFar - zNear;

	tr.viewParms.projectionMatrix[0] = 2 * zNear / width;
	tr.viewParms.projectionMatrix[4] = 0;
	tr.viewParms.projectionMatrix[8] = ( xmax + xmin ) / width; // normally 0
	tr.viewParms.projectionMatrix[12] = 0;

	tr.viewParms.projectionMatrix[1] = 0;
	tr.viewParms.projectionMatrix[5] = 2 * zNear / height;
	tr.viewParms.projectionMatrix[9] = ( ymax + ymin ) / height;    // normally 0
	tr.viewParms.projectionMatrix[13] = 0;

	tr.viewParms.projectionMatrix[2] = 0;
	tr.viewParms.projectionMatrix[6] = 0;
	tr.viewParms.projectionMatrix[10] = -( zFar + zNear ) / depth;
	tr.viewParms.projectionMatrix[14] = -2 * zFar * zNear / depth;

	tr.viewParms.projectionMatrix[3] = 0;
	tr.viewParms.projectionMatrix[7] = 0;
	tr.viewParms.projectionMatrix[11] = -1;
	tr.viewParms.projectionMatrix[15] = 0;
}

#if !defined RTCW_ET
/*
=================
R_SetupFrustum

Setup that culling frustum planes for the current view
=================
*/
void R_SetupFrustum( void ) {
	int i;
	float xs, xc;
	float ang;

	ang = tr.viewParms.fovX / 180 * M_PI * 0.5f;
	xs = c::sin( ang );
	xc = c::cos( ang );

	// BBi
	//VectorScale( tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[0].normal );
	//VectorMA( tr.viewParms.frustum[0].normal, xc, tr.viewParms.or.axis[1], tr.viewParms.frustum[0].normal );

	//VectorScale( tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[1].normal );
	//VectorMA( tr.viewParms.frustum[1].normal, -xc, tr.viewParms.or.axis[1], tr.viewParms.frustum[1].normal );
	VectorScale (tr.viewParms.orientation.axis[0], xs,
		tr.viewParms.frustum[0].normal);

	VectorMA (tr.viewParms.frustum[0].normal, xc,
		tr.viewParms.orientation.axis[1], tr.viewParms.frustum[0].normal);

	VectorScale (tr.viewParms.orientation.axis[0], xs,
		tr.viewParms.frustum[1].normal);

	VectorMA (tr.viewParms.frustum[1].normal, -xc,
		tr.viewParms.orientation.axis[1], tr.viewParms.frustum[1].normal);
	// BBi

	ang = tr.viewParms.fovY / 180 * M_PI * 0.5f;
	xs = c::sin( ang );
	xc = c::cos( ang );

	// BBi
	//VectorScale( tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[2].normal );
	//VectorMA( tr.viewParms.frustum[2].normal, xc, tr.viewParms.or.axis[2], tr.viewParms.frustum[2].normal );

	//VectorScale( tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[3].normal );
	//VectorMA( tr.viewParms.frustum[3].normal, -xc, tr.viewParms.or.axis[2], tr.viewParms.frustum[3].normal );
	VectorScale (tr.viewParms.orientation.axis[0], xs,
		tr.viewParms.frustum[2].normal);

	VectorMA (tr.viewParms.frustum[2].normal, xc,
		tr.viewParms.orientation.axis[2], tr.viewParms.frustum[2].normal);

	VectorScale (tr.viewParms.orientation.axis[0], xs,
		tr.viewParms.frustum[3].normal);

	VectorMA (tr.viewParms.frustum[3].normal, -xc,
		tr.viewParms.orientation.axis[2], tr.viewParms.frustum[3].normal);
	// BBi

	for ( i = 0 ; i < 4 ; i++ ) {
		tr.viewParms.frustum[i].type = PLANE_NON_AXIAL;

		// BBi
		//tr.viewParms.frustum[i].dist = DotProduct( tr.viewParms.or.origin, tr.viewParms.frustum[i].normal );
		tr.viewParms.frustum[i].dist = DotProduct (
			tr.viewParms.orientation.origin, tr.viewParms.frustum[i].normal);
		// BBi

		SetPlaneSignbits( &tr.viewParms.frustum[i] );
	}
}
#endif // RTCW_XX

/*
=================
R_MirrorPoint
=================
*/
void R_MirrorPoint( vec3_t in, orientation_t *surface, orientation_t *camera, vec3_t out ) {
	int i;
	vec3_t local;
	vec3_t transformed;
	float d;

	VectorSubtract( in, surface->origin, local );

	VectorClear( transformed );
	for ( i = 0 ; i < 3 ; i++ ) {
		d = DotProduct( local, surface->axis[i] );
		VectorMA( transformed, d, camera->axis[i], transformed );
	}

	VectorAdd( transformed, camera->origin, out );
}

void R_MirrorVector( vec3_t in, orientation_t *surface, orientation_t *camera, vec3_t out ) {
	int i;
	float d;

	VectorClear( out );
	for ( i = 0 ; i < 3 ; i++ ) {
		d = DotProduct( in, surface->axis[i] );
		VectorMA( out, d, camera->axis[i], out );
	}
}


/*
=============
R_PlaneForSurface
=============
*/
void R_PlaneForSurface( surfaceType_t *surfType, cplane_t *plane ) {
	srfTriangles_t  *tri;
	srfPoly_t       *poly;
	drawVert_t      *v1, *v2, *v3;
	vec4_t plane4;

	if ( !surfType ) {
		memset( plane, 0, sizeof( *plane ) );
		plane->normal[0] = 1;
		return;
	}
	switch ( *surfType ) {
	case SF_FACE:
		*plane = ( (srfSurfaceFace_t *)surfType )->plane;
		return;
	case SF_TRIANGLES:
		tri = (srfTriangles_t *)surfType;
		v1 = tri->verts + tri->indexes[0];
		v2 = tri->verts + tri->indexes[1];
		v3 = tri->verts + tri->indexes[2];
		PlaneFromPoints( plane4, v1->xyz, v2->xyz, v3->xyz );
		VectorCopy( plane4, plane->normal );
		plane->dist = plane4[3];
		return;
	case SF_POLY:
		poly = (srfPoly_t *)surfType;
		PlaneFromPoints( plane4, poly->verts[0].xyz, poly->verts[1].xyz, poly->verts[2].xyz );
		VectorCopy( plane4, plane->normal );
		plane->dist = plane4[3];
		return;
	default:
		memset( plane, 0, sizeof( *plane ) );
		plane->normal[0] = 1;
		return;
	}
}

/*
=================
R_GetPortalOrientation

entityNum is the entity that the portal surface is a part of, which may
be moving and rotating.

Returns qtrue if it should be mirrored
=================
*/
qboolean R_GetPortalOrientations( drawSurf_t *drawSurf, int entityNum,
								  orientation_t *surface, orientation_t *camera,
								  vec3_t pvsOrigin, qboolean *mirror ) {
	int i;
	cplane_t originalPlane, plane;
	trRefEntity_t   *e;
	float d;
	vec3_t transformed;

	// create plane axis for the portal we are seeing
	R_PlaneForSurface( drawSurf->surface, &originalPlane );

	// rotate the plane if necessary
	if ( entityNum != ENTITYNUM_WORLD ) {
		tr.currentEntityNum = entityNum;
		tr.currentEntity = &tr.refdef.entities[entityNum];

		// get the orientation of the entity
// BBi
//#if !defined RTCW_ET
//		R_RotateForEntity( tr.currentEntity, &tr.viewParms, &tr.or );
//#else
//		R_RotateForEntity( tr.currentEntity, &tr.viewParms, &tr.orientation );
//#endif // RTCW_XX
		R_RotateForEntity (tr.currentEntity, &::tr.viewParms,
			&::tr.orientation);
// BBi

		// rotate the plane, but keep the non-rotated version for matching
		// against the portalSurface entities
		R_LocalNormalToWorld( originalPlane.normal, plane.normal );

// BBi
//#if !defined RTCW_ET
//		plane.dist = originalPlane.dist + DotProduct( plane.normal, tr.or.origin );
//#else
//		plane.dist = originalPlane.dist + DotProduct( plane.normal, tr.orientation.origin );
//#endif // RTCW_XX
		plane.dist = originalPlane.dist +
			DotProduct (plane.normal, tr.orientation.origin);
// BBi

		// translate the original plane
// BBi
//#if !defined RTCW_ET
//		originalPlane.dist = originalPlane.dist + DotProduct( originalPlane.normal, tr.or.origin );
//#else
//		originalPlane.dist = originalPlane.dist + DotProduct( originalPlane.normal, tr.orientation.origin );
//#endif // RTCW_XX
		originalPlane.dist = originalPlane.dist + DotProduct (
			originalPlane.normal, tr.orientation.origin);
// BBi
	} else {
		plane = originalPlane;
	}

	VectorCopy( plane.normal, surface->axis[0] );
	PerpendicularVector( surface->axis[1], surface->axis[0] );
	CrossProduct( surface->axis[0], surface->axis[1], surface->axis[2] );

	// locate the portal entity closest to this plane.
	// origin will be the origin of the portal, origin2 will be
	// the origin of the camera
	for ( i = 0 ; i < tr.refdef.num_entities ; i++ ) {
		e = &tr.refdef.entities[i];
		if ( e->e.reType != RT_PORTALSURFACE ) {
			continue;
		}

		d = DotProduct( e->e.origin, originalPlane.normal ) - originalPlane.dist;
		if ( d > 64 || d < -64 ) {
			continue;
		}

		// get the pvsOrigin from the entity
		VectorCopy( e->e.oldorigin, pvsOrigin );

		// if the entity is just a mirror, don't use as a camera point
		if ( e->e.oldorigin[0] == e->e.origin[0] &&
			 e->e.oldorigin[1] == e->e.origin[1] &&
			 e->e.oldorigin[2] == e->e.origin[2] ) {
			VectorScale( plane.normal, plane.dist, surface->origin );
			VectorCopy( surface->origin, camera->origin );
			VectorSubtract( vec3_origin, surface->axis[0], camera->axis[0] );
			VectorCopy( surface->axis[1], camera->axis[1] );
			VectorCopy( surface->axis[2], camera->axis[2] );

			*mirror = qtrue;
			return qtrue;
		}

		// project the origin onto the surface plane to get
		// an origin point we can rotate around
		d = DotProduct( e->e.origin, plane.normal ) - plane.dist;
		VectorMA( e->e.origin, -d, surface->axis[0], surface->origin );

		// now get the camera origin and orientation
		VectorCopy( e->e.oldorigin, camera->origin );
		AxisCopy( e->e.axis, camera->axis );
		VectorSubtract( vec3_origin, camera->axis[0], camera->axis[0] );
		VectorSubtract( vec3_origin, camera->axis[1], camera->axis[1] );

		// optionally rotate
		if ( e->e.oldframe ) {
			// if a speed is specified
			if ( e->e.frame ) {
				// continuous rotate
				d = ( tr.refdef.time / 1000.0f ) * e->e.frame;
				VectorCopy( camera->axis[1], transformed );
				RotatePointAroundVector( camera->axis[1], camera->axis[0], transformed, d );
				CrossProduct( camera->axis[0], camera->axis[1], camera->axis[2] );
			} else {
				// bobbing rotate, with skinNum being the rotation offset
				d = c::sin( tr.refdef.time * 0.003f );
				d = e->e.skinNum + d * 4;
				VectorCopy( camera->axis[1], transformed );
				RotatePointAroundVector( camera->axis[1], camera->axis[0], transformed, d );
				CrossProduct( camera->axis[0], camera->axis[1], camera->axis[2] );
			}
		} else if ( e->e.skinNum )   {
			d = e->e.skinNum;
			VectorCopy( camera->axis[1], transformed );
			RotatePointAroundVector( camera->axis[1], camera->axis[0], transformed, d );
			CrossProduct( camera->axis[0], camera->axis[1], camera->axis[2] );
		}
		*mirror = qfalse;
		return qtrue;
	}

	// if we didn't locate a portal entity, don't render anything.
	// We don't want to just treat it as a mirror, because without a
	// portal entity the server won't have communicated a proper entity set
	// in the snapshot

	// unfortunately, with local movement prediction it is easily possible
	// to see a surface before the server has communicated the matching
	// portal surface entity, so we don't want to print anything here...

	//ri.Printf( PRINT_ALL, "Portal surface without a portal entity\n" );

	return qfalse;
}

static qboolean IsMirror( const drawSurf_t *drawSurf, int entityNum ) {
	int i;
	cplane_t originalPlane, plane;
	trRefEntity_t   *e;
	float d;

	// create plane axis for the portal we are seeing
	R_PlaneForSurface( drawSurf->surface, &originalPlane );

	// rotate the plane if necessary
	if ( entityNum != ENTITYNUM_WORLD ) {
		tr.currentEntityNum = entityNum;
		tr.currentEntity = &tr.refdef.entities[entityNum];

		// get the orientation of the entity
// BBi
//#if !defined RTCW_ET
//		R_RotateForEntity( tr.currentEntity, &tr.viewParms, &tr.or );
//#else
//		R_RotateForEntity( tr.currentEntity, &tr.viewParms, &tr.orientation );
//#endif // RTCW_XX
		R_RotateForEntity (tr.currentEntity, &::tr.viewParms,
			&::tr.orientation);
// BBi

		// rotate the plane, but keep the non-rotated version for matching
		// against the portalSurface entities
		R_LocalNormalToWorld( originalPlane.normal, plane.normal );

// BBi
//#if !defined RTCW_ET
//		plane.dist = originalPlane.dist + DotProduct( plane.normal, tr.or.origin );
//#else
//		plane.dist = originalPlane.dist + DotProduct( plane.normal, tr.orientation.origin );
//#endif // RTCW_XX
		plane.dist = originalPlane.dist + DotProduct (
			plane.normal, tr.orientation.origin);
// BBi

		// translate the original plane
// BBi
//#if !defined RTCW_ET
//		originalPlane.dist = originalPlane.dist + DotProduct( originalPlane.normal, tr.or.origin );
//#else
//		originalPlane.dist = originalPlane.dist + DotProduct( originalPlane.normal, tr.orientation.origin );
//#endif // RTCW_XX
		originalPlane.dist = originalPlane.dist + DotProduct (
			originalPlane.normal, tr.orientation.origin);
// BBi
	} else
	{
		plane = originalPlane;
	}

	// locate the portal entity closest to this plane.
	// origin will be the origin of the portal, origin2 will be
	// the origin of the camera
	for ( i = 0 ; i < tr.refdef.num_entities ; i++ )
	{
		e = &tr.refdef.entities[i];
		if ( e->e.reType != RT_PORTALSURFACE ) {
			continue;
		}

		d = DotProduct( e->e.origin, originalPlane.normal ) - originalPlane.dist;
		if ( d > 64 || d < -64 ) {
			continue;
		}

		// if the entity is just a mirror, don't use as a camera point
		if ( e->e.oldorigin[0] == e->e.origin[0] &&
			 e->e.oldorigin[1] == e->e.origin[1] &&
			 e->e.oldorigin[2] == e->e.origin[2] ) {
			return qtrue;
		}

		return qfalse;
	}
	return qfalse;
}

/*
** SurfIsOffscreen
**
** Determines if a surface is completely offscreen.
*/
static qboolean SurfIsOffscreen( const drawSurf_t *drawSurf, vec4_t clipDest[128] ) {
	float shortest = 100000000;
	int entityNum;
	int numTriangles;
	shader_t *shader;
	int fogNum;

#if defined RTCW_ET
	int frontFace;
#endif // RTCW_XX

	int dlighted;

#if defined RTCW_SP
// GR - tessellation flag
	int atiTess;
#endif // RTCW_XX

	vec4_t clip, eye;
	int i;
	unsigned int pointOr = 0;
	unsigned int pointAnd = (unsigned int)~0;

// BBi
//	if ( glConfig.smpActive ) {     // FIXME!  we can't do RB_BeginSurface/RB_EndSurface stuff with smp!
//		return qfalse;
//	}
// BBi

	R_RotateForViewer();

#if defined RTCW_SP
// GR - decompose with tessellation flag
	R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted, &atiTess );
#elif defined RTCW_MP
	R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted );
#else
	R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &frontFace, &dlighted );
#endif // RTCW_XX

	RB_BeginSurface( shader, fogNum );
	rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );

	assert( tess.numVertexes < 128 );

	for ( i = 0; i < tess.numVertexes; i++ )
	{
		int j;
		unsigned int pointFlags = 0;

		R_TransformModelToClip( tess.xyz[i].v, tr.orientation.modelMatrix, tr.viewParms.projectionMatrix, eye, clip );

		for ( j = 0; j < 3; j++ )
		{
			if ( clip[j] >= clip[3] ) {
				pointFlags |= ( 1 << ( j * 2 ) );
			} else if ( clip[j] <= -clip[3] )   {
				pointFlags |= ( 1 << ( j * 2 + 1 ) );
			}
		}
		pointAnd &= pointFlags;
		pointOr |= pointFlags;
	}

	// trivially reject
	if ( pointAnd ) {
		return qtrue;
	}

	// determine if this surface is backfaced and also determine the distance
	// to the nearest vertex so we can cull based on portal range.  Culling
	// based on vertex distance isn't 100% correct (we should be checking for
	// range to the surface), but it's good enough for the types of portals
	// we have in the game right now.
	numTriangles = tess.numIndexes / 3;

	for ( i = 0; i < tess.numIndexes; i += 3 )
	{
		vec3_t normal;
		float dot;
		float len;

		VectorSubtract( tess.xyz[tess.indexes[i]].v, tr.viewParms.orientation.origin, normal );

		len = VectorLengthSquared( normal );            // lose the sqrt
		if ( len < shortest ) {
			shortest = len;
		}

		if ( ( dot = DotProduct( normal, tess.normal[tess.indexes[i]].v ) ) >= 0 ) {
			numTriangles--;
		}
	}
	if ( !numTriangles ) {
		return qtrue;
	}

	// mirrors can early out at this point, since we don't do a fade over distance
	// with them (although we could)
	if ( IsMirror( drawSurf, entityNum ) ) {
		return qfalse;
	}

	if ( shortest > ( tess.shader->portalRange * tess.shader->portalRange ) ) {
		return qtrue;
	}

	return qfalse;
}

/*
========================
R_MirrorViewBySurface

Returns qtrue if another view has been rendered
========================
*/
qboolean R_MirrorViewBySurface( drawSurf_t *drawSurf, int entityNum ) {
	vec4_t clipDest[128];
	viewParms_t newParms;
	viewParms_t oldParms;
	orientation_t surface, camera;

	// don't recursively mirror
	if ( tr.viewParms.isPortal ) {
		ri.Printf( PRINT_DEVELOPER, "WARNING: recursive mirror/portal found\n" );
		return qfalse;
	}

//	if ( r_noportals->integer || r_fastsky->integer || tr.levelGLFog) {
	if ( r_noportals->integer || r_fastsky->integer ) {
		return qfalse;
	}

	// trivially reject portal/mirror
	if ( SurfIsOffscreen( drawSurf, clipDest ) ) {
		return qfalse;
	}

	// save old viewParms so we can return to it after the mirror view
	oldParms = tr.viewParms;

	newParms = tr.viewParms;
	newParms.isPortal = qtrue;
	if ( !R_GetPortalOrientations( drawSurf, entityNum, &surface, &camera,
								   newParms.pvsOrigin, &newParms.isMirror ) ) {
		return qfalse;      // bad portal, no portalentity
	}

// BBi
//#if !defined RTCW_ET
//	R_MirrorPoint( oldParms.or.origin, &surface, &camera, newParms.or.origin );
//#else
//	R_MirrorPoint( oldParms.orientation.origin, &surface, &camera, newParms.orientation.origin );
//#endif // RTCW_XX
	R_MirrorPoint (oldParms.orientation.origin, &surface, &camera,
		newParms.orientation.origin);
// BBi

	VectorSubtract( vec3_origin, camera.axis[0], newParms.portalPlane.normal );
	newParms.portalPlane.dist = DotProduct( camera.origin, newParms.portalPlane.normal );

// BBi
//#if !defined RTCW_ET
//	R_MirrorVector( oldParms.or.axis[0], &surface, &camera, newParms.or.axis[0] );
//	R_MirrorVector( oldParms.or.axis[1], &surface, &camera, newParms.or.axis[1] );
//	R_MirrorVector( oldParms.or.axis[2], &surface, &camera, newParms.or.axis[2] );
//#else
//	R_MirrorVector( oldParms.orientation.axis[0], &surface, &camera, newParms.orientation.axis[0] );
//	R_MirrorVector( oldParms.orientation.axis[1], &surface, &camera, newParms.orientation.axis[1] );
//	R_MirrorVector( oldParms.orientation.axis[2], &surface, &camera, newParms.orientation.axis[2] );
//#endif // RTCW_XX
	R_MirrorVector (oldParms.orientation.axis[0], &surface, &camera,
		newParms.orientation.axis[0]);

	R_MirrorVector (oldParms.orientation.axis[1], &surface, &camera,
		newParms.orientation.axis[1]);

	R_MirrorVector (oldParms.orientation.axis[2], &surface, &camera,
		newParms.orientation.axis[2]);
// BBi

	// OPTIMIZE: restrict the viewport on the mirrored view

	// render the mirror view
	R_RenderView( &newParms );

	tr.viewParms = oldParms;

	return qtrue;
}

/*
=================
R_SpriteFogNum

See if a sprite is inside a fog volume
=================
*/
int R_SpriteFogNum( trRefEntity_t *ent ) {
	int i, j;
	fog_t           *fog;

	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return 0;
	}

	for ( i = 1 ; i < tr.world->numfogs ; i++ ) {
		fog = &tr.world->fogs[i];
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( ent->e.origin[j] - ent->e.radius >= fog->bounds[1][j] ) {
				break;
			}
			if ( ent->e.origin[j] + ent->e.radius <= fog->bounds[0][j] ) {
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
==========================================================================================

DRAWSURF SORTING

==========================================================================================
*/

/*
=================
qsort replacement

=================
*/

// BBi
namespace {

//#define SWAP_DRAW_SURF( a,b ) temp = ( (int *)a )[0]; ( (int *)a )[0] = ( (int *)b )[0]; ( (int *)b )[0] = temp; temp = ( (int *)a )[1]; ( (int *)a )[1] = ( (int *)b )[1]; ( (int *)b )[1] = temp;
void SWAP_DRAW_SURF (drawSurf_t* a, drawSurf_t* b)
{
	std::swap(*a, *b);
}

void SWAP_DRAW_SURF (char* a, char* b)
{
	SWAP_DRAW_SURF(reinterpret_cast<drawSurf_t*>(a), reinterpret_cast<drawSurf_t*>(b));
}

} // namespace
// BBi

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8            /* testing shows that this is good value */

static void shortsort( drawSurf_t *lo, drawSurf_t *hi ) {
	drawSurf_t  *p, *max;
	int temp;

	while ( hi > lo ) {
		max = lo;
		for ( p = lo + 1; p <= hi; p++ ) {
			if ( p->sort > max->sort ) {
				max = p;
			}
		}
		SWAP_DRAW_SURF( max, hi );
		hi--;
	}
}


/* sort the array between lo and hi (inclusive)
FIXME: this was lifted and modified from the microsoft lib source...
 */

void qsortFast(
	void *base,
	unsigned num,
	unsigned width
	) {
	char *lo, *hi;              /* ends of sub-array currently sorting */
	char *mid;                  /* points to middle of subarray */
	char *loguy, *higuy;        /* traveling pointers for partition step */
	unsigned size;              /* size of the sub-array */
	char *lostk[30], *histk[30];
	int stkptr;                 /* stack for saving sub-array to be processed */
	int temp;

	// BBi
	//if ( sizeof( drawSurf_t ) != 8 ) {
	//	ri.Error( ERR_DROP, "change SWAP_DRAW_SURF macro" );
	//}
	// BBi

	/* Note: the number of stack entries required is no more than
	   1 + log2(size), so 30 is sufficient for any array */

	if ( num < 2 || width == 0 ) {
		return;                 /* nothing to do */

	}
	stkptr = 0;                 /* initialize stack */

	lo = static_cast<char*> (base);
	hi = static_cast<char*> (base) + width * ( num - 1 );        /* initialize limits */

	/* this entry point is for pseudo-recursion calling: setting
	   lo and hi and jumping to here is like recursion, but stkptr is
	   prserved, locals aren't, so we preserve stuff on the stack */
recurse:

	size = ( hi - lo ) / width + 1;        /* number of el's to sort */

	/* below a certain size, it is faster to use a O(n^2) sorting method */
	if ( size <= CUTOFF ) {
		shortsort( (drawSurf_t *)lo, (drawSurf_t *)hi );
	} else {
		/* First we pick a partititioning element.  The efficiency of the
		   algorithm demands that we find one that is approximately the
		   median of the values, but also that we select one fast.  Using
		   the first one produces bad performace if the array is already
		   sorted, so we use the middle one, which would require a very
		   wierdly arranged array for worst case performance.  Testing shows
		   that a median-of-three algorithm does not, in general, increase
		   performance. */

		mid = lo + ( size / 2 ) * width;      /* find middle element */
		SWAP_DRAW_SURF( mid, lo );               /* swap it to beginning of array */

		/* We now wish to partition the array into three pieces, one
		   consisiting of elements <= partition element, one of elements
		   equal to the parition element, and one of element >= to it.  This
		   is done below; comments indicate conditions established at every
		   step. */

		loguy = lo;
		higuy = hi + width;

		/* Note that higuy decreases and loguy increases on every iteration,
		   so loop must terminate. */
		for (;; ) {
			/* lo <= loguy < hi, lo < higuy <= hi + 1,
			   A[i] <= A[lo] for lo <= i <= loguy,
			   A[i] >= A[lo] for higuy <= i <= hi */

			do  {
				loguy += width;
			} while ( loguy <= hi &&
					  ( ( (drawSurf_t *)loguy )->sort <= ( (drawSurf_t *)lo )->sort ) );

			/* lo < loguy <= hi+1, A[i] <= A[lo] for lo <= i < loguy,
			   either loguy > hi or A[loguy] > A[lo] */

			do  {
				higuy -= width;
			} while ( higuy > lo &&
					  ( ( (drawSurf_t *)higuy )->sort >= ( (drawSurf_t *)lo )->sort ) );

			/* lo-1 <= higuy <= hi, A[i] >= A[lo] for higuy < i <= hi,
			   either higuy <= lo or A[higuy] < A[lo] */

			if ( higuy < loguy ) {
				break;
			}

			/* if loguy > hi or higuy <= lo, then we would have exited, so
			   A[loguy] > A[lo], A[higuy] < A[lo],
			   loguy < hi, highy > lo */

			SWAP_DRAW_SURF( loguy, higuy );

			/* A[loguy] < A[lo], A[higuy] > A[lo]; so condition at top
			   of loop is re-established */
		}

		/*     A[i] >= A[lo] for higuy < i <= hi,
			   A[i] <= A[lo] for lo <= i < loguy,
			   higuy < loguy, lo <= higuy <= hi
		   implying:
			   A[i] >= A[lo] for loguy <= i <= hi,
			   A[i] <= A[lo] for lo <= i <= higuy,
			   A[i] = A[lo] for higuy < i < loguy */

		SWAP_DRAW_SURF( lo, higuy );     /* put partition element in place */

		/* OK, now we have the following:
			  A[i] >= A[higuy] for loguy <= i <= hi,
			  A[i] <= A[higuy] for lo <= i < higuy
			  A[i] = A[lo] for higuy <= i < loguy    */

		/* We've finished the partition, now we want to sort the subarrays
		   [lo, higuy-1] and [loguy, hi].
		   We do the smaller one first to minimize stack usage.
		   We only sort arrays of length 2 or more.*/

		if ( higuy - 1 - lo >= hi - loguy ) {
			if ( lo + width < higuy ) {
				lostk[stkptr] = lo;
				histk[stkptr] = higuy - width;
				++stkptr;
			}                           /* save big recursion for later */

			if ( loguy < hi ) {
				lo = loguy;
				goto recurse;           /* do small recursion */
			}
		} else {
			if ( loguy < hi ) {
				lostk[stkptr] = loguy;
				histk[stkptr] = hi;
				++stkptr;               /* save big recursion for later */
			}

			if ( lo + width < higuy ) {
				hi = higuy - width;
				goto recurse;           /* do small recursion */
			}
		}
	}

	/* We have sorted the array, except for any pending sorts on the stack.
	   Check if there are any, and do them. */

	--stkptr;
	if ( stkptr >= 0 ) {
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto recurse;           /* pop subarray from stack */
	} else {
		return;                 /* all subarrays done */
	}
}

//==========================================================================================

/*
=================
R_AddDrawSurf
=================
*/

#if defined RTCW_SP
void R_AddDrawSurf( surfaceType_t *surface, shader_t *shader,
					int fogIndex, int dlightMap, int atiTess ) {
#elif defined RTCW_MP
void R_AddDrawSurf( surfaceType_t *surface, shader_t *shader,
					int fogIndex, int dlightMap ) {
#else
void R_AddDrawSurf( surfaceType_t *surface, shader_t *shader, int fogNum, int frontFace, int dlightMap ) {
#endif // RTCW_XX

	int index;

#if defined RTCW_ET
	// we can't just wrap around here, because the surface count will be
	// off.  Check for overflow, and drop new surfaces on overflow.
	if ( tr.refdef.numDrawSurfs >= MAX_DRAWSURFS ) {
		return;
	}
#endif // RTCW_XX

#if !defined RTCW_ET
	// instead of checking for overflow, we just mask the index
	// so it wraps around
	index = tr.refdef.numDrawSurfs & DRAWSURF_MASK;
#else
	index = tr.refdef.numDrawSurfs;
#endif // RTCW_XX

	// the sort data is packed into a single 32 bit value so it can be
	// compared quickly during the qsorting process

#if defined RTCW_SP
// GR - add tesselation flag to the sort
#endif // RTCW_XX

	tr.refdef.drawSurfs[index].sort = ( shader->sortedIndex << QSORT_SHADERNUM_SHIFT )

#if defined RTCW_SP
									  | ( atiTess << QSORT_ATI_TESS_SHIFT )
#endif // RTCW_XX

#if !defined RTCW_ET
									  | tr.shiftedEntityNum | ( fogIndex << QSORT_FOGNUM_SHIFT ) | (int)dlightMap;
#else
									  | tr.shiftedEntityNum | ( fogNum << QSORT_FOGNUM_SHIFT ) | ( frontFace << QSORT_FRONTFACE_SHIFT ) | dlightMap;
#endif // RTCW_XX

	tr.refdef.drawSurfs[index].surface = surface;
	tr.refdef.numDrawSurfs++;
}

/*
=================
R_DecomposeSort
=================
*/

#if defined RTCW_SP
// GR - decompose  with tessellation flag
void R_DecomposeSort( unsigned sort, int *entityNum, shader_t **shader,
					  int *fogNum, int *dlightMap, int *atiTess ) {
#elif defined RTCW_MP
void R_DecomposeSort( unsigned sort, int *entityNum, shader_t **shader,
					  int *fogNum, int *dlightMap ) {
#else
void R_DecomposeSort( unsigned sort, int *entityNum, shader_t **shader, int *fogNum, int *frontFace, int *dlightMap ) {
#endif // RTCW_XX

	*fogNum = ( sort >> QSORT_FOGNUM_SHIFT ) & 31;
	*shader = tr.sortedShaders[ ( sort >> QSORT_SHADERNUM_SHIFT ) & ( MAX_SHADERS - 1 ) ];
//	*entityNum = ( sort >> QSORT_ENTITYNUM_SHIFT ) & 1023;
	*entityNum = ( sort >> QSORT_ENTITYNUM_SHIFT ) & ( MAX_GENTITIES - 1 );   // (SA) uppded entity count for Wolf to 11 bits

#if defined RTCW_ET
	*frontFace = ( sort >> QSORT_FRONTFACE_SHIFT ) & 1;
#endif // RTCW_XX

#if !defined RTCW_ET
	*dlightMap = sort & 3;
#else
	*dlightMap = sort & 1;
#endif // RTCW_XX

#if defined RTCW_SP
//GR - extract tessellation flag
	*atiTess = ( sort >> QSORT_ATI_TESS_SHIFT ) & 1;
#endif // RTCW_XX

}


/*
=================
R_SortDrawSurfs
=================
*/
void R_SortDrawSurfs( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	shader_t        *shader;
	int fogNum;
	int entityNum;

#if defined RTCW_ET
	int frontFace;
#endif // RTCW_XX

	int dlighted;
	int i;

#if defined RTCW_SP
// GR - tessellation flag
	int atiTess;
#endif // RTCW_XX

	// it is possible for some views to not have any surfaces
	if ( numDrawSurfs < 1 ) {
		// we still need to add it for hyperspace cases
		R_AddDrawSurfCmd( drawSurfs, numDrawSurfs );
		return;
	}

	// if we overflowed MAX_DRAWSURFS, the drawsurfs
	// wrapped around in the buffer and we will be missing
	// the first surfaces, not the last ones
	if ( numDrawSurfs > MAX_DRAWSURFS ) {
		numDrawSurfs = MAX_DRAWSURFS;
	}

	// sort the drawsurfs by sort type, then orientation, then shader
	qsortFast( drawSurfs, numDrawSurfs, sizeof( drawSurf_t ) );

	// check for any pass through drawing, which
	// may cause another view to be rendered first
	for ( i = 0 ; i < numDrawSurfs ; i++ ) {

#if defined RTCW_SP
// GR - decompose with tessellation flag
		R_DecomposeSort( ( drawSurfs + i )->sort, &entityNum, &shader, &fogNum, &dlighted, &atiTess );
#elif defined RTCW_MP
		R_DecomposeSort( ( drawSurfs + i )->sort, &entityNum, &shader, &fogNum, &dlighted );
#else
		R_DecomposeSort( ( drawSurfs + i )->sort, &entityNum, &shader, &fogNum, &frontFace, &dlighted );
#endif // RTCW_XX

		if ( shader->sort > SS_PORTAL ) {
			break;
		}

		// no shader should ever have this sort type
		if ( shader->sort == SS_BAD ) {
			ri.Error( ERR_DROP, "Shader '%s'with sort == SS_BAD", shader->name );
		}

		// if the mirror was completely clipped away, we may need to check another surface
		if ( R_MirrorViewBySurface( ( drawSurfs + i ), entityNum ) ) {
			// this is a debug option to see exactly what is being mirrored
			if ( r_portalOnly->integer ) {
				return;
			}
			break;      // only one mirror view at a time
		}
	}

	R_AddDrawSurfCmd( drawSurfs, numDrawSurfs );
}

/*
=============
R_AddEntitySurfaces
=============
*/
void R_AddEntitySurfaces( void ) {
	trRefEntity_t   *ent;
	shader_t        *shader;

	if ( !r_drawentities->integer ) {
		return;
	}

	for ( tr.currentEntityNum = 0;
		  tr.currentEntityNum < tr.refdef.num_entities;
		  tr.currentEntityNum++ ) {
		ent = tr.currentEntity = &tr.refdef.entities[tr.currentEntityNum];

		ent->needDlights = qfalse;

		// preshift the value we are going to OR into the drawsurf sort
		tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

		//
		// the weapon model must be handled special --
		// we don't want the hacked weapon position showing in
		// mirrors, because the true body position will already be drawn
		//
		if ( ( ent->e.renderfx & RF_FIRST_PERSON ) && tr.viewParms.isPortal ) {
			continue;
		}

		// simple generated models, like sprites and beams, are not culled
		switch ( ent->e.reType ) {
		case RT_PORTALSURFACE:
			break;      // don't draw anything
		case RT_SPRITE:
		case RT_SPLASH:
		case RT_BEAM:
		case RT_LIGHTNING:
		case RT_RAIL_CORE:
		case RT_RAIL_CORE_TAPER:
		case RT_RAIL_RINGS:
			// self blood sprites, talk balloons, etc should not be drawn in the primary
			// view.  We can't just do this check for all entities, because md3
			// entities may still want to cast shadows from them
			if ( ( ent->e.renderfx & RF_THIRD_PERSON ) && !tr.viewParms.isPortal ) {
				continue;
			}
			shader = R_GetShaderByHandle( ent->e.customShader );

#if defined RTCW_SP
// GR - these entities are not tessellated
			R_AddDrawSurf( &entitySurface, shader, R_SpriteFogNum( ent ), 0, ATI_TESS_NONE );
#elif defined RTCW_MP
			R_AddDrawSurf( &entitySurface, shader, R_SpriteFogNum( ent ), 0 );
#else
			R_AddDrawSurf( &entitySurface, shader, R_SpriteFogNum( ent ), 0, 0 );
#endif // RTCW_XX

			break;

		case RT_MODEL:
			// we must set up parts of tr.or for model culling
// BBi
//#if !defined RTCW_ET
//			R_RotateForEntity( ent, &tr.viewParms, &tr.or );
//#else
//			R_RotateForEntity( ent, &tr.viewParms, &tr.orientation );
//#endif // RTCW_XX
			R_RotateForEntity (ent, &::tr.viewParms, &::tr.orientation);
// BBi

			tr.currentModel = R_GetModelByHandle( ent->e.hModel );
			if ( !tr.currentModel ) {

#if defined RTCW_SP
// GR - not tessellated
				R_AddDrawSurf( &entitySurface, tr.defaultShader, 0, 0, ATI_TESS_NONE );
#elif defined RTCW_MP
				R_AddDrawSurf( &entitySurface, tr.defaultShader, 0, 0 );
#else
				R_AddDrawSurf( &entitySurface, tr.defaultShader, 0, 0, 0 );
#endif // RTCW_XX

			} else {
				switch ( tr.currentModel->type ) {
				case MOD_MESH:
					R_AddMD3Surfaces( ent );
					break;
					// Ridah
				case MOD_MDC:
					R_AddMDCSurfaces( ent );
					break;
					// done.
				case MOD_MDS:
					R_AddAnimSurfaces( ent );
					break;

#if defined RTCW_ET
				case MOD_MDM:
					R_MDM_AddAnimSurfaces( ent );
					break;
#endif // RTCW_XX

				case MOD_BRUSH:
					R_AddBrushModelSurfaces( ent );
					break;
				case MOD_BAD:       // null model axis
					if ( ( ent->e.renderfx & RF_THIRD_PERSON ) && !tr.viewParms.isPortal ) {
						break;
					}
					shader = R_GetShaderByHandle( ent->e.customShader );

#if defined RTCW_SP
// GR - not tessellated
					R_AddDrawSurf( &entitySurface, tr.defaultShader, 0, 0, ATI_TESS_NONE );
#elif defined RTCW_MP
					R_AddDrawSurf( &entitySurface, tr.defaultShader, 0, 0 );
#else
					R_AddDrawSurf( &entitySurface, tr.defaultShader, 0, 0, 0 );
#endif // RTCW_XX

					break;
				default:
					ri.Error( ERR_DROP, "R_AddEntitySurfaces: Bad modeltype" );
					break;
				}
			}
			break;
		default:
			ri.Error( ERR_DROP, "R_AddEntitySurfaces: Bad reType" );
		}
	}

}


/*
====================
R_GenerateDrawSurfs
====================
*/
void R_GenerateDrawSurfs( void ) {

#if defined RTCW_ET
	// set the projection matrix (and view frustum) here
	// first with max or fog distance so we can have proper
	// arbitrary frustum farplane culling optimization
	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL || tr.world == NULL ) {
		VectorSet( tr.viewParms.visBounds[ 0 ], MIN_WORLD_COORD, MIN_WORLD_COORD, MIN_WORLD_COORD );
		VectorSet( tr.viewParms.visBounds[ 1 ], MAX_WORLD_COORD, MAX_WORLD_COORD, MAX_WORLD_COORD );
	} else
	{
		VectorCopy( tr.world->nodes->mins, tr.viewParms.visBounds[ 0 ] );
		VectorCopy( tr.world->nodes->maxs, tr.viewParms.visBounds[ 1 ] );
	}


	R_SetupProjection();

	R_CullDecalProjectors();
	R_CullDlights();
#endif // RTCW_XX

	R_AddWorldSurfaces();

#if !defined RTCW_ET
	R_AddPolygonSurfaces();
#endif // RTCW_XX

	// set the projection matrix with the minimum zfar
	// now that we have the world bounded
	// this needs to be done before entities are
	// added, because they use the projection
	// matrix for lod calculation
	R_SetupProjection();

	R_AddEntitySurfaces();

#if defined RTCW_ET
	R_AddPolygonSurfaces();

	R_AddPolygonBufferSurfaces();
#endif // RTCW_XX

}

/*
================
R_DebugPolygon
================
*/
void R_DebugPolygon( int color, int numPoints, float *points ) {
	int i;

	GL_State( GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

	// draw solid shade

	// BBi
	if (!glConfigEx.is_path_ogl_1_x ()) {
		ogl_tess_state.primary_color = rtcw::cgm::Vec4(
			static_cast<float>(color % 2),
			static_cast<float>((color / 2) % 2),
			static_cast<float>((color / 4) % 2),
			1.0F);

		for (i = 0; i < numPoints; ++i) {
			const float* v = points + (3 * i);
			ogl_tess2.position[i] = rtcw::cgm::Vec4(v[0], v[1], v[2], 1.0F);
		}

		ogl_tess2_draw (GL_TRIANGLE_FAN, numPoints, false, false);
	} else {
	// BBi

	glColor3f( color & 1, ( color >> 1 ) & 1, ( color >> 2 ) & 1 );
	glBegin( GL_POLYGON );
	for ( i = 0 ; i < numPoints ; i++ ) {
		glVertex3fv( points + i * 3 );
	}
	glEnd();

	// BBi
	}
	// BBi

	// draw wireframe outline
	GL_State( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
	glDepthRange( 0, 0 );

	// BBi
	if (!glConfigEx.is_path_ogl_1_x ()) {
		ogl_tess_state.primary_color = rtcw::cgm::Vec4(1.0F, 1.0F, 1.0F, 1.0F);

		for (i = 0; i < numPoints; ++i) {
			const float* v = points + (3 * i);
			ogl_tess2.position[i] = rtcw::cgm::Vec4(v[0], v[1], v[2], 1.0F);
		}

		ogl_tess2_draw (GL_TRIANGLE_FAN, numPoints, false, false);
	} else {
	// BBi

	glColor3f( 1, 1, 1 );
	glBegin( GL_POLYGON );
	for ( i = 0 ; i < numPoints ; i++ ) {
		glVertex3fv( points + i * 3 );
	}
	glEnd();

	// BBi
	}
	// BBi

	glDepthRange( 0, 1 );
}

#if defined RTCW_ET
/*
================
R_DebugText
================
*/
void R_DebugText( const vec3_t org, float r, float g, float b, const char *text, qboolean neverOcclude ) {

	if ( neverOcclude ) {
		glDepthRange( 0, 0 );  // never occluded

	}

	// BBi FIXME 2.1+ path
	if (glConfigEx.is_path_ogl_1_x ()) {
	// BBi

	glColor3f( r, g, b );
	glRasterPos3fv( org );
	glPushAttrib( GL_LIST_BIT );
	glListBase( gl_NormalFontBase );
	glCallLists( strlen( text ), GL_UNSIGNED_BYTE, text );
	glListBase( 0 );
	glPopAttrib();

	// BBi
	}
	// BBi

	if ( neverOcclude ) {
		glDepthRange( 0, 1 );
	}
}
#endif // RTCW_XX

/*
====================
R_DebugGraphics

Visualization aid for movement clipping debugging
====================
*/
void R_DebugGraphics( void ) {
	if ( !r_debugSurface->integer ) {
		return;
	}

	R_FogOff(); // moved this in here to keep from /always/ doing the fog state change

	// the render thread can't make callbacks to the main thread
	R_SyncRenderThread();

	GL_Bind( tr.whiteImage );
	GL_Cull( CT_FRONT_SIDED );
	ri.CM_DrawDebugSurface( R_DebugPolygon );
}


/*
================
R_RenderView

A view may be either the actual camera view,
or a mirror / remote location
================
*/
void R_RenderView( viewParms_t *parms ) {
	int firstDrawSurf;

	if ( parms->viewportWidth <= 0 || parms->viewportHeight <= 0 ) {
		return;
	}

	// Ridah, purge media that were left over from the last level
	if ( r_cache->integer ) {
		extern void R_PurgeBackupImages( int purgeCount );
		static int lastTime;

		if ( ( lastTime > tr.refdef.time ) || ( lastTime < ( tr.refdef.time - 200 ) ) ) {
			R_FreeImageBuffer();    // clear all image buffers
			R_PurgeShaders( 10 );
			R_PurgeBackupImages( 1 );
			R_PurgeModels( 1 );
			lastTime = tr.refdef.time;
		}
	}
	// done.

	tr.viewCount++;

	tr.viewParms = *parms;
	tr.viewParms.frameSceneNum = tr.frameSceneNum;
	tr.viewParms.frameCount = tr.frameCount;

	firstDrawSurf = tr.refdef.numDrawSurfs;

	tr.viewCount++;

	// set viewParms.world
	R_RotateForViewer();

#if !defined RTCW_ET
	R_SetupFrustum();
#else
	// ydnar: removed this because we do a full frustum/projection setup
	// based on world bounds zfar or fog bounds
	// R_SetupFrustum ();
#endif // RTCW_XX

	R_GenerateDrawSurfs();

	R_SortDrawSurfs( tr.refdef.drawSurfs + firstDrawSurf, tr.refdef.numDrawSurfs - firstDrawSurf );

	// draw main system development information (surface outlines, etc)
	R_FogOff();
	R_DebugGraphics();
	R_FogOn();

}

// BBi
GLenum r_get_best_wrap_clamp ()
{
	if (glConfigEx.use_arb_framebuffer_object_)
		return GL_CLAMP_TO_EDGE;
	else
		return GL_CLAMP;
}
// BBi
