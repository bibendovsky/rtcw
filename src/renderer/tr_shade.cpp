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

// tr_shade.c

//BBi
#include <windows.h>
//BBi

#include "tr_local.h"

/*

  THIS ENTIRE FILE IS BACK END

  This file deals with applying shaders to surface data in the tess struct.
*/

/*
================
R_ArrayElementDiscrete

This is just for OpenGL conformance testing, it should never be the fastest
================
*/
static void APIENTRY R_ArrayElementDiscrete( GLint index ) {
	::glColor4ubv( tess.svars.colors[ index ] );
	if ( glState.currenttmu ) {
		::glMultiTexCoord2fARB( 0, tess.svars.texcoords[ 0 ][ index ][0], tess.svars.texcoords[ 0 ][ index ][1] );
		::glMultiTexCoord2fARB( 1, tess.svars.texcoords[ 1 ][ index ][0], tess.svars.texcoords[ 1 ][ index ][1] );
	} else {
		::glTexCoord2fv( tess.svars.texcoords[ 0 ][ index ] );
	}

#if !defined RTCW_ET
	::glVertex3fv( tess.xyz[ index ] );
#else
	::glVertex3fv( tess.xyz[ index ].v );
#endif // RTCW_XX

}

/*
===================
R_DrawStripElements

===================
*/
static int c_vertexes;          // for seeing how long our average strips are
static int c_begins;
static void R_DrawStripElements( int numIndexes, const glIndex_t *indexes, void ( APIENTRY *element )( GLint ) ) {
	int i;
	int last[3] = { -1, -1, -1 };
	qboolean even;

	c_begins++;

	if ( numIndexes <= 0 ) {
		return;
	}

	::glBegin( GL_TRIANGLE_STRIP );

	// prime the strip
	element( indexes[0] );
	element( indexes[1] );
	element( indexes[2] );
	c_vertexes += 3;

	last[0] = indexes[0];
	last[1] = indexes[1];
	last[2] = indexes[2];

	even = qfalse;

	for ( i = 3; i < numIndexes; i += 3 )
	{
		// odd numbered triangle in potential strip
		if ( !even ) {
			// check previous triangle to see if we're continuing a strip
			if ( ( indexes[i + 0] == last[2] ) && ( indexes[i + 1] == last[1] ) ) {
				element( indexes[i + 2] );
				c_vertexes++;
				assert( indexes[i + 2] < tess.numVertexes );
				even = qtrue;
			}
			// otherwise we're done with this strip so finish it and start
			// a new one
			else
			{
				::glEnd();

				::glBegin( GL_TRIANGLE_STRIP );
				c_begins++;

				element( indexes[i + 0] );
				element( indexes[i + 1] );
				element( indexes[i + 2] );

				c_vertexes += 3;

				even = qfalse;
			}
		} else
		{
			// check previous triangle to see if we're continuing a strip
			if ( ( last[2] == indexes[i + 1] ) && ( last[0] == indexes[i + 0] ) ) {
				element( indexes[i + 2] );
				c_vertexes++;

				even = qfalse;
			}
			// otherwise we're done with this strip so finish it and start
			// a new one
			else
			{
				::glEnd();

				::glBegin( GL_TRIANGLE_STRIP );
				c_begins++;

				element( indexes[i + 0] );
				element( indexes[i + 1] );
				element( indexes[i + 2] );
				c_vertexes += 3;

				even = qfalse;
			}
		}

		// cache the last three vertices
		last[0] = indexes[i + 0];
		last[1] = indexes[i + 1];
		last[2] = indexes[i + 2];
	}

	::glEnd();
}



/*
==================
R_DrawElements

Optionally performs our own glDrawElements that looks for strip conditions
instead of using the single glDrawElements call that may be inefficient
without compiled vertex arrays.
==================
*/
static void R_DrawElements( int numIndexes, const glIndex_t *indexes ) {
	int primitives;

	primitives = r_primitives->integer;

	// default is to use triangles if compiled vertex arrays are present
	if ( primitives == 0 ) {
		if ( glConfigEx.useExtCompiledVertexArray ) {
			primitives = 2;
		} else {
			primitives = 1;
		}
	}


	if ( primitives == 2 ) {
		::glDrawElements( GL_TRIANGLES,
						 numIndexes,
						 GL_INDEX_TYPE,
						 indexes );
		return;
	}

	if ( primitives == 1 ) {
		R_DrawStripElements( numIndexes,  indexes, ::glArrayElement );
		return;
	}

	if ( primitives == 3 ) {
		R_DrawStripElements( numIndexes,  indexes, R_ArrayElementDiscrete );
		return;
	}

	// anything else will cause no drawing
}


/*
=============================================================

SURFACE SHADERS

=============================================================
*/

shaderCommands_t tess;
static qboolean setArraysOnce;

/*
=================
R_BindAnimatedImage

=================
*/
static void R_BindAnimatedImage( textureBundle_t *bundle ) {
	int index;

	if ( bundle->isVideoMap ) {
		ri.CIN_RunCinematic( bundle->videoMapHandle );
		ri.CIN_UploadCinematic( bundle->videoMapHandle );
		return;
	}

	if ( bundle->numImageAnimations <= 1 ) {
		if ( bundle->isLightmap && ( backEnd.refdef.rdflags & RDF_SNOOPERVIEW ) ) {
			GL_Bind( tr.whiteImage );
		} else {
			GL_Bind( bundle->image[0] );
		}
		return;
	}

	// it is necessary to do this messy calc to make sure animations line up
	// exactly with waveforms of the same frequency
	index = myftol( tess.shaderTime * bundle->imageAnimationSpeed * FUNCTABLE_SIZE );
	index >>= FUNCTABLE_SIZE2;

	if ( index < 0 ) {
		index = 0;  // may happen with shader time offsets
	}
	index %= bundle->numImageAnimations;

	if ( bundle->isLightmap && ( backEnd.refdef.rdflags & RDF_SNOOPERVIEW ) ) {
		GL_Bind( tr.whiteImage );
	} else {
		GL_Bind( bundle->image[ index ] );
	}
}

/*
================
DrawTris

Draws triangle outlines for debugging
================
*/
static void DrawTris( shaderCommands_t *input ) {

#if !defined RTCW_ET
	GL_Bind( tr.whiteImage );
	::glColor3f( 1,1,1 );

	GL_State( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE );

#if !defined RTCW_MP
	if ( r_showtris->integer == 1 ) {
		::glDepthRange( 0, 0 );
	}
#else
	::glDepthRange( 0, 0 );
#endif // RTCW_XX
#else
	char            *s = r_trisColor->string;
	vec4_t trisColor = { 1, 1, 1, 1 };
	unsigned int stateBits = 0;

	GL_Bind( tr.whiteImage );

	if ( *s == '0' && ( *( s + 1 ) == 'x' || *( s + 1 ) == 'X' ) ) {
		s += 2;
		if ( Q_IsHexColorString( s ) ) {
			trisColor[0] = ( (float)( gethex( *( s ) ) * 16 + gethex( *( s + 1 ) ) ) ) / 255.00;
			trisColor[1] = ( (float)( gethex( *( s + 2 ) ) * 16 + gethex( *( s + 3 ) ) ) ) / 255.00;
			trisColor[2] = ( (float)( gethex( *( s + 4 ) ) * 16 + gethex( *( s + 5 ) ) ) ) / 255.00;

			if ( Q_HexColorStringHasAlpha( s ) ) {
				trisColor[3] = ( (float)( gethex( *( s + 6 ) ) * 16 + gethex( *( s + 7 ) ) ) ) / 255.00;
			}
		}
	} else {
		int i;
		char    *token;

		for ( i = 0 ; i < 4 ; i++ ) {
			token = COM_Parse( &s );
			if ( token ) {
				trisColor[i] = atof( token );
			} else {
				trisColor[i] = 1.f;
			}
		}

		if ( !trisColor[3] ) {
			trisColor[3] = 1.f;
		}
	}

	if ( trisColor[3] < 1.f ) {
		stateBits |= ( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
	}

	::glColor4fv( trisColor );

	// ydnar r_showtris 2
	if ( r_showtris->integer == 2 ) {
		stateBits |= ( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE );
		GL_State( stateBits );
		::glDepthRange( 0, 0 );
	}
	#ifdef CELSHADING_HACK
	else if ( r_showtris->integer == 3 ) {
		stateBits |= ( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE );
		GL_State( stateBits );
		::glEnable( GL_POLYGON_OFFSET_LINE );
		::glPolygonOffset( 4.0, 0.5 );
		::glLineWidth( 5.0 );
	}
	#endif
	else
	{
		stateBits |= ( GLS_POLYMODE_LINE );
		GL_State( stateBits );
		::glEnable( GL_POLYGON_OFFSET_LINE );
		::glPolygonOffset( r_offsetFactor->value, r_offsetUnits->value );
	}
#endif // RTCW_XX

	::glDisableClientState( GL_COLOR_ARRAY );
	::glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	::glVertexPointer( 3, GL_FLOAT, 16, input->xyz ); // padded for SIMD

	if ( glConfigEx.useExtCompiledVertexArray ) {
		::glLockArraysEXT( 0, input->numVertexes );
		//GLimp_LogComment( "glLockArraysEXT\n" );
	}

	R_DrawElements( input->numIndexes, input->indexes );

	if ( glConfigEx.useExtCompiledVertexArray ) {
		::glUnlockArraysEXT();
		//GLimp_LogComment( "glUnlockArraysEXT\n" );
	}
	::glDepthRange( 0, 1 );

#if defined RTCW_ET
	::glDisable( GL_POLYGON_OFFSET_LINE );
#endif // RTCW_XX

}


/*
================
DrawNormals

Draws vertex normals for debugging
================
*/
static void DrawNormals( shaderCommands_t *input ) {
	int i;
	vec3_t temp;

	GL_Bind( tr.whiteImage );
	::glColor3f( 1,1,1 );

#if defined RTCW_SP
	if ( r_shownormals->integer == 1 ) {
		::glDepthRange( 0, 0 );  // never occluded
	}
#else
	::glDepthRange( 0, 0 );  // never occluded
#endif // RTCW_XX

	GL_State( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE );

#if defined RTCW_ET
	// ydnar: light direction
	if ( r_shownormals->integer == 2 ) {
		trRefEntity_t   *ent = backEnd.currentEntity;
		vec3_t temp2;

		if ( ent->e.renderfx & RF_LIGHTING_ORIGIN ) {
			VectorSubtract( ent->e.lightingOrigin, backEnd.orientation.origin, temp2 );
		} else {
			VectorClear( temp2 );
		}
		temp[ 0 ] = DotProduct( temp2, backEnd.orientation.axis[ 0 ] );
		temp[ 1 ] = DotProduct( temp2, backEnd.orientation.axis[ 1 ] );
		temp[ 2 ] = DotProduct( temp2, backEnd.orientation.axis[ 2 ] );

		::glColor3f( ent->ambientLight[ 0 ] / 255, ent->ambientLight[ 1 ] / 255, ent->ambientLight[ 2 ] / 255 );
		::glPointSize( 5 );
		::glBegin( GL_POINTS );
		::glVertex3fv( temp );
		::glEnd();
		::glPointSize( 1 );

		if ( fabs( VectorLengthSquared( ent->lightDir ) - 1.0f ) > 0.2f ) {
			::glColor3f( 1, 0, 0 );
		} else {
			::glColor3f( ent->directedLight[ 0 ] / 255, ent->directedLight[ 1 ] / 255, ent->directedLight[ 2 ] / 255 );
		}
		::glLineWidth( 3 );
		::glBegin( GL_LINES );
		::glVertex3fv( temp );
		VectorMA( temp, 32, ent->lightDir, temp );
		::glVertex3fv( temp );
		::glEnd();
		::glLineWidth( 1 );
	}
	// ydnar: normals drawing
	else
	{
#endif // RTCW_XX

	::glBegin( GL_LINES );
	for ( i = 0 ; i < input->numVertexes ; i++ ) {

#if !defined RTCW_ET
		::glVertex3fv( input->xyz[i] );
		VectorMA( input->xyz[i], 2, input->normal[i], temp );
#else
			::glVertex3fv( input->xyz[i].v );
			VectorMA( input->xyz[i].v, r_normallength->value, input->normal[i].v, temp );
#endif // RTCW_XX

		::glVertex3fv( temp );
	}
	::glEnd();

#if defined RTCW_ET
	}
#endif // RTCW_XX

	::glDepthRange( 0, 1 );
}

/*
==============
RB_BeginSurface

We must set some things up before beginning any tesselation,
because a surface may be forced to perform a RB_End due
to overflow.
==============
*/
void RB_BeginSurface( shader_t *shader, int fogNum ) {

	shader_t *state = ( shader->remappedShader ) ? shader->remappedShader : shader;

#if defined RTCW_SP
	tess.ATI_tess = qfalse;     //----(SA)	added
#endif // RTCW_XX

	tess.numIndexes = 0;
	tess.numVertexes = 0;
	tess.shader = state;
	tess.fogNum = fogNum;
	tess.dlightBits = 0;        // will be OR'd in by surface functions
	tess.xstages = state->stages;
	tess.numPasses = state->numUnfoggedPasses;
	tess.currentStageIteratorFunc = state->optimalStageIteratorFunc;

	tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
	if ( tess.shader->clampTime && tess.shaderTime >= tess.shader->clampTime ) {
		tess.shaderTime = tess.shader->clampTime;
	}
	// done.
}

/*
===================
DrawMultitextured

output = t0 * t1 or t0 + t1

t0 = most upstream according to spec
t1 = most downstream according to spec
===================
*/
static void DrawMultitextured( shaderCommands_t *input, int stage ) {
	shaderStage_t   *pStage;

	pStage = tess.xstages[stage];

	// Ridah
	if ( tess.shader->noFog && pStage->isFogged ) {
		R_FogOn();
	} else if ( tess.shader->noFog && !pStage->isFogged ) {
		R_FogOff(); // turn it back off
	} else {    // make sure it's on
		R_FogOn();
	}
	// done.

	GL_State( pStage->stateBits );

	// this is an ugly hack to work around a GeForce driver
	// bug with multitexture and clip planes
	if ( backEnd.viewParms.isPortal ) {
		::glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}

	//
	// base
	//
	GL_SelectTexture( 0 );
	::glTexCoordPointer( 2, GL_FLOAT, 0, input->svars.texcoords[0] );
	R_BindAnimatedImage( &pStage->bundle[0] );

	//
	// lightmap/secondary pass
	//
	GL_SelectTexture( 1 );
	::glEnable( GL_TEXTURE_2D );
	::glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	if ( r_lightmap->integer ) {
		GL_TexEnv( GL_REPLACE );
	} else {
		GL_TexEnv( tess.shader->multitextureEnv );
	}

	::glTexCoordPointer( 2, GL_FLOAT, 0, input->svars.texcoords[1] );

	R_BindAnimatedImage( &pStage->bundle[1] );

	R_DrawElements( input->numIndexes, input->indexes );

	//
	// disable texturing on TEXTURE1, then select TEXTURE0
	//
	//::glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	::glDisable( GL_TEXTURE_2D );

	GL_SelectTexture( 0 );
}


#if !defined RTCW_ET
/*
===================
ProjectDlightTexture

Perform dynamic lighting with another rendering pass
===================
*/
static void ProjectDlightTexture( void ) {
	int i, l;
	vec3_t origin;
	float   *texCoords;
	byte    *colors;
	byte clipBits[SHADER_MAX_VERTEXES];
	MAC_STATIC float texCoordsArray[SHADER_MAX_VERTEXES][2];
	byte colorArray[SHADER_MAX_VERTEXES][4];
	unsigned hitIndexes[SHADER_MAX_INDEXES];
	int numIndexes;
	float scale;
	float radius;
	vec3_t floatColor;

	if ( !backEnd.refdef.num_dlights ) {
		return;
	}


	if ( backEnd.refdef.rdflags & RDF_SNOOPERVIEW ) {  // no dlights for snooper
		return;
	}


	for ( l = 0 ; l < backEnd.refdef.num_dlights ; l++ ) {
		dlight_t    *dl;

		if ( !( tess.dlightBits & ( 1 << l ) ) ) {
			continue;   // this surface definately doesn't have any of this light
		}
		texCoords = texCoordsArray[0];
		colors = colorArray[0];

		dl = &backEnd.refdef.dlights[l];
		VectorCopy( dl->transformed, origin );
		radius = dl->radius;
		scale = 1.0f / radius;
		floatColor[0] = dl->color[0] * 255.0f;
		floatColor[1] = dl->color[1] * 255.0f;
		floatColor[2] = dl->color[2] * 255.0f;

		for ( i = 0 ; i < tess.numVertexes ; i++, texCoords += 2, colors += 4 ) {
			vec3_t dist;
			int clip;
			float modulate;

			if ( 0 ) {
				clipBits[i] = 255;  // definately not dlighted
				continue;
			}

#if defined RTCW_SP
			VectorSubtract( origin, tess.xyz[i], dist );

//			if(!r_dlightBacks->integer) {
//				vec3_t	dir;
//				VectorNormalize2(dist, dir);
//				if( DotProduct( tess.normal[i], dir) < 0) {
//					clipBits[i] = 255;	// not lighted (backface)
//					continue;
//				}
//			}
#endif // RTCW_XX

			backEnd.pc.c_dlightVertexes++;

#if defined RTCW_MP
			VectorSubtract( origin, tess.xyz[i], dist );
#endif // RTCW_XX

			texCoords[0] = 0.5f + dist[0] * scale;
			texCoords[1] = 0.5f + dist[1] * scale;

			clip = 0;
			if ( texCoords[0] < 0.0f ) {
				clip |= 1;
			} else if ( texCoords[0] > 1.0f ) {
				clip |= 2;
			}
			if ( texCoords[1] < 0.0f ) {
				clip |= 4;
			} else if ( texCoords[1] > 1.0f ) {
				clip |= 8;
			}
			// modulate the strength based on the height and color
			if ( dist[2] > radius ) {
				clip |= 16;
				modulate = 0.0f;
			} else if ( dist[2] < -radius ) {
				clip |= 32;
				modulate = 0.0f;
			} else {
				dist[2] = Q_fabs( dist[2] );
				if ( dist[2] < radius * 0.5f ) {
					modulate = 1.0f;
				} else {
					modulate = 2.0f * ( radius - dist[2] ) * scale;
				}
			}
			clipBits[i] = clip;

			colors[0] = myftol( floatColor[0] * modulate );
			colors[1] = myftol( floatColor[1] * modulate );
			colors[2] = myftol( floatColor[2] * modulate );
			colors[3] = 255;
		}

		// build a list of triangles that need light
		numIndexes = 0;
		for ( i = 0 ; i < tess.numIndexes ; i += 3 ) {
			int a, b, c;

			a = tess.indexes[i];
			b = tess.indexes[i + 1];
			c = tess.indexes[i + 2];
			if ( clipBits[a] & clipBits[b] & clipBits[c] ) {
				continue;   // not lighted
			}

#if defined RTCW_SP
//			if(!r_dlightBacks->integer) {
//				vec3_t	dir;
//				VectorSubtract( origin, tess.xyz[a], dir );
//				VectorNormalize(dir);
//				if( DotProduct( tess.normal[i], dir) < 0) {
//					continue;	// not lighted (backface)
//				}
//			}
#endif // RTCW_XX

			hitIndexes[numIndexes] = a;
			hitIndexes[numIndexes + 1] = b;
			hitIndexes[numIndexes + 2] = c;
			numIndexes += 3;
		}

		if ( !numIndexes ) {
			continue;
		}

		::glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		::glTexCoordPointer( 2, GL_FLOAT, 0, texCoordsArray[0] );

		::glEnableClientState( GL_COLOR_ARRAY );
		::glColorPointer( 4, GL_UNSIGNED_BYTE, 0, colorArray );

		//----(SA) creating dlight shader to allow for special blends or alternate dlight texture
		{
			shader_t *dls = dl->dlshader;
			if ( dls ) {
//				if (!::glActiveTextureARB || dls->numUnfoggedPasses < 2) {
				for ( i = 0; i < dls->numUnfoggedPasses; i++ )
				{
					shaderStage_t *stage = dls->stages[i];
					R_BindAnimatedImage( &dls->stages[i]->bundle[0] );
					GL_State( stage->stateBits | GLS_DEPTHFUNC_EQUAL );
					R_DrawElements( numIndexes, hitIndexes );
					backEnd.pc.c_totalIndexes += numIndexes;
					backEnd.pc.c_dlightIndexes += numIndexes;
				}
/*
				} else {	// optimize for multitexture

					for(i=0;i<dls->numUnfoggedPasses;)
					{
						shaderStage_t *stage = dls->stages[i];

						GL_State(stage->stateBits | GLS_DEPTHFUNC_EQUAL);

						// setup each TMU
						for (tmu=0; tmu<glConfig.maxActiveTextures && i<dls->numUnfoggedPasses; tmu++, i++) {

							GL_SelectTexture( tmu );

							if (tmu) {
								::glEnable( GL_TEXTURE_2D );
							}

							R_BindAnimatedImage( &dls->stages[i]->bundle[0] );
						}

						// draw the elements
						R_DrawElements( numIndexes, hitIndexes );
						backEnd.pc.c_totalIndexes += numIndexes;
						backEnd.pc.c_dlightIndexes += numIndexes;
					}

					// turn off unused TMU's
					for (tmu=1; tmu<glConfig.maxActiveTextures; tmu++) {
						// set back to default state
						GL_SelectTexture( tmu );
						::glDisable( GL_TEXTURE_2D );
					}

					// return to TEXTURE0
					GL_SelectTexture( 0 );
				}
*/
			} else
			{
				R_FogOff();

//				if (!dl->overdraw || !::glActiveTextureARB) {
				GL_Bind( tr.dlightImage );
				// include GLS_DEPTHFUNC_EQUAL so alpha tested surfaces don't add light
				// where they aren't rendered
				GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
				R_DrawElements( numIndexes, hitIndexes );
				backEnd.pc.c_totalIndexes += numIndexes;
				backEnd.pc.c_dlightIndexes += numIndexes;

				// Ridah, overdraw lights several times, rather than sending
				//	multiple lights through
				for ( i = 0; i < dl->overdraw; i++ ) {
					R_DrawElements( numIndexes, hitIndexes );
					backEnd.pc.c_totalIndexes += numIndexes;
					backEnd.pc.c_dlightIndexes += numIndexes;
				}
/*
				} else {	// optimize for multitexture

					GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );

					// setup each TMU (use all available TMU's)
					for (tmu=0; tmu<glConfig.maxActiveTextures && tmu<(dl->overdraw+1); tmu++) {
						GL_SelectTexture( tmu );
						if (tmu) {
							::glEnable( GL_TEXTURE_2D );
							GL_TexEnv( GL_ADD );
							GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
							::glEnableClientState( GL_TEXTURE_COORD_ARRAY );
							::glTexCoordPointer( 2, GL_FLOAT, 0, texCoordsArray[0] );
							::glEnableClientState( GL_COLOR_ARRAY );
							::glColorPointer( 4, GL_UNSIGNED_BYTE, 0, colorArray );
						}
						GL_Bind( tr.dlightImage );
					}

					// draw each bundle
					for(i=0; i<(dl->overdraw+1); i+=glConfig.maxActiveTextures)
					{
						// make sure we dont draw with too many TMU's
						if (i+glConfig.maxActiveTextures>(dl->overdraw+1)) {
							for (tmu=0; tmu<glConfig.maxActiveTextures; tmu++) {
								if (tmu+i>=(dl->overdraw+1)) {
									GL_SelectTexture( tmu );
									::glDisable( GL_TEXTURE_2D );
								}
							}
						}
						// draw the elements
						R_DrawElements( numIndexes, hitIndexes );
						backEnd.pc.c_totalIndexes += numIndexes;
						backEnd.pc.c_dlightIndexes += numIndexes;
					}

					// turn off unused TMU's
					for (tmu=1; tmu<glConfig.maxActiveTextures; tmu++) {
						// set back to default state
						GL_SelectTexture( tmu );
						::glDisable( GL_TEXTURE_2D );
					}

					// return to TEXTURE0
					GL_SelectTexture( 0 );
				}
*/
				R_FogOn();
			}
		}
	}
}
#endif // RTCW_XX

#if defined RTCW_ET
/*
DynamicLightSinglePass()
perform all dynamic lighting with a single rendering pass
*/

static void DynamicLightSinglePass( void ) {
	int i, l, a, b, c, color, *intColors;
	vec3_t origin;
	byte        *colors;
	unsigned hitIndexes[ SHADER_MAX_INDEXES ];
	int numIndexes;
	float radius, radiusInverseCubed;
	float intensity, remainder, modulate;
	vec3_t floatColor, dir;
	dlight_t    *dl;


	// early out
	if ( backEnd.refdef.num_dlights == 0 ) {
		return;
	}

	// clear colors
	Com_Memset( tess.svars.colors, 0, sizeof( tess.svars.colors ) );

	// walk light list
	for ( l = 0; l < backEnd.refdef.num_dlights; l++ )
	{
		// early out
		if ( !( tess.dlightBits & ( 1 << l ) ) ) {
			continue;
		}

		// setup
		dl = &backEnd.refdef.dlights[ l ];
		VectorCopy( dl->transformed, origin );
		radius = dl->radius;
		radiusInverseCubed = dl->radiusInverseCubed;
		intensity = dl->intensity;
		floatColor[ 0 ] = dl->color[ 0 ] * 255.0f;
		floatColor[ 1 ] = dl->color[ 1 ] * 255.0f;
		floatColor[ 2 ] = dl->color[ 2 ] * 255.0f;

		// directional lights have max intensity and washout remainder intensity
		if ( dl->flags & REF_DIRECTED_DLIGHT ) {
			remainder = intensity * 0.125;
		} else {
			remainder = 0.0f;
		}

		// illuminate vertexes
		colors = tess.svars.colors[ 0 ];
		for ( i = 0; i < tess.numVertexes; i++, colors += 4 )
		{
			backEnd.pc.c_dlightVertexes++;

			// directional dlight, origin is a directional normal
			if ( dl->flags & REF_DIRECTED_DLIGHT ) {
				// twosided surfaces use absolute value of the calculated lighting
				modulate = intensity * DotProduct( dl->origin, tess.normal[ i ].v );
				if ( tess.shader->cullType == CT_TWO_SIDED ) {
					modulate = fabs( modulate );
				}
				modulate += remainder;
			}
			// ball dlight
			else
			{
				dir[ 0 ] = radius - fabs( origin[ 0 ] - tess.xyz[ i ].v[ 0 ] );
				if ( dir[ 0 ] <= 0.0f ) {
					continue;
				}
				dir[ 1 ] = radius - fabs( origin[ 1 ] - tess.xyz[ i ].v[ 1 ] );
				if ( dir[ 1 ] <= 0.0f ) {
					continue;
				}
				dir[ 2 ] = radius - fabs( origin[ 2 ] - tess.xyz[ i ].v[ 2 ] );
				if ( dir[ 2 ] <= 0.0f ) {
					continue;
				}

				modulate = intensity * dir[ 0 ] * dir[ 1 ] * dir[ 2 ] * radiusInverseCubed;
			}

			// optimizations
			if ( modulate < ( 1.0f / 128.0f ) ) {
				continue;
			} else if ( modulate > 1.0f ) {
				modulate = 1.0f;
			}

			// add to color
			color = colors[ 0 ] + myftol( floatColor[ 0 ] * modulate );
			colors[ 0 ] = color > 255 ? 255 : color;
			color = colors[ 1 ] + myftol( floatColor[ 1 ] * modulate );
			colors[ 1 ] = color > 255 ? 255 : color;
			color = colors[ 2 ] + myftol( floatColor[ 2 ] * modulate );
			colors[ 2 ] = color > 255 ? 255 : color;
		}
	}

	// build a list of triangles that need light
	intColors = (int*) tess.svars.colors;
	numIndexes = 0;
	for ( i = 0; i < tess.numIndexes; i += 3 )
	{
		a = tess.indexes[ i ];
		b = tess.indexes[ i + 1 ];
		c = tess.indexes[ i + 2 ];
		if ( !( intColors[ a ] | intColors[ b ] | intColors[ c ] ) ) {
			continue;
		}
		hitIndexes[ numIndexes++ ] = a;
		hitIndexes[ numIndexes++ ] = b;
		hitIndexes[ numIndexes++ ] = c;
	}

	if ( numIndexes == 0 ) {
		return;
	}

	// debug code
	//%	for( i = 0; i < numIndexes; i++ )
	//%		intColors[ hitIndexes[ i ] ] = 0x000000FF;

	::glEnableClientState( GL_COLOR_ARRAY );
	::glColorPointer( 4, GL_UNSIGNED_BYTE, 0, tess.svars.colors );

	// render the dynamic light pass
	R_FogOff();
	GL_Bind( tr.whiteImage );
	GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
	R_DrawElements( numIndexes, hitIndexes );
	backEnd.pc.c_totalIndexes += numIndexes;
	backEnd.pc.c_dlightIndexes += numIndexes;
	R_FogOn();
}



/*
DynamicLightPass()
perform dynamic lighting with multiple rendering passes
*/

static void DynamicLightPass( void ) {
	int i, l, a, b, c, color, *intColors;
	vec3_t origin;
	byte        *colors;
	unsigned hitIndexes[ SHADER_MAX_INDEXES ];
	int numIndexes;
	float radius, radiusInverseCubed;
	float intensity, remainder, modulate;
	vec3_t floatColor, dir;
	dlight_t    *dl;


	// early out
	if ( backEnd.refdef.num_dlights == 0 ) {
		return;
	}

	// walk light list
	for ( l = 0; l < backEnd.refdef.num_dlights; l++ )
	{
		// early out
		if ( !( tess.dlightBits & ( 1 << l ) ) ) {
			continue;
		}

		// clear colors
		Com_Memset( tess.svars.colors, 0, sizeof( tess.svars.colors ) );

		// setup
		dl = &backEnd.refdef.dlights[ l ];
		VectorCopy( dl->transformed, origin );
		radius = dl->radius;
		radiusInverseCubed = dl->radiusInverseCubed;
		intensity = dl->intensity;
		floatColor[ 0 ] = dl->color[ 0 ] * 255.0f;
		floatColor[ 1 ] = dl->color[ 1 ] * 255.0f;
		floatColor[ 2 ] = dl->color[ 2 ] * 255.0f;

		// directional lights have max intensity and washout remainder intensity
		if ( dl->flags & REF_DIRECTED_DLIGHT ) {
			remainder = intensity * 0.125;
		} else {
			remainder = 0.0f;
		}

		// illuminate vertexes
		colors = tess.svars.colors[ 0 ];
		for ( i = 0; i < tess.numVertexes; i++, colors += 4 )
		{
			backEnd.pc.c_dlightVertexes++;

			// directional dlight, origin is a directional normal
			if ( dl->flags & REF_DIRECTED_DLIGHT ) {
				// twosided surfaces use absolute value of the calculated lighting
				modulate = intensity * DotProduct( dl->origin, tess.normal[ i ].v );
				if ( tess.shader->cullType == CT_TWO_SIDED ) {
					modulate = fabs( modulate );
				}
				modulate += remainder;
			}
			// ball dlight
			else
			{
				dir[ 0 ] = radius - fabs( origin[ 0 ] - tess.xyz[ i ].v[ 0 ] );
				if ( dir[ 0 ] <= 0.0f ) {
					continue;
				}
				dir[ 1 ] = radius - fabs( origin[ 1 ] - tess.xyz[ i ].v[ 1 ] );
				if ( dir[ 1 ] <= 0.0f ) {
					continue;
				}
				dir[ 2 ] = radius - fabs( origin[ 2 ] - tess.xyz[ i ].v[ 2 ] );
				if ( dir[ 2 ] <= 0.0f ) {
					continue;
				}

				modulate = intensity * dir[ 0 ] * dir[ 1 ] * dir[ 2 ] * radiusInverseCubed;
			}

			// optimizations
			if ( modulate < ( 1.0f / 128.0f ) ) {
				continue;
			} else if ( modulate > 1.0f ) {
				modulate = 1.0f;
			}

			// set color
			color = myftol( floatColor[ 0 ] * modulate );
			colors[ 0 ] = color > 255 ? 255 : color;
			color = myftol( floatColor[ 1 ] * modulate );
			colors[ 1 ] = color > 255 ? 255 : color;
			color = myftol( floatColor[ 2 ] * modulate );
			colors[ 2 ] = color > 255 ? 255 : color;
		}

		// build a list of triangles that need light
		intColors = (int*) tess.svars.colors;
		numIndexes = 0;
		for ( i = 0; i < tess.numIndexes; i += 3 )
		{
			a = tess.indexes[ i ];
			b = tess.indexes[ i + 1 ];
			c = tess.indexes[ i + 2 ];
			if ( !( intColors[ a ] | intColors[ b ] | intColors[ c ] ) ) {
				continue;
			}
			hitIndexes[ numIndexes++ ] = a;
			hitIndexes[ numIndexes++ ] = b;
			hitIndexes[ numIndexes++ ] = c;
		}

		if ( numIndexes == 0 ) {
			continue;
		}

		// debug code (fixme, there's a bug in this function!)
		//%	for( i = 0; i < numIndexes; i++ )
		//%		intColors[ hitIndexes[ i ] ] = 0x000000FF;

		::glEnableClientState( GL_COLOR_ARRAY );
		::glColorPointer( 4, GL_UNSIGNED_BYTE, 0, tess.svars.colors );

		R_FogOff();
		GL_Bind( tr.whiteImage );
		GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
		R_DrawElements( numIndexes, hitIndexes );
		backEnd.pc.c_totalIndexes += numIndexes;
		backEnd.pc.c_dlightIndexes += numIndexes;
		R_FogOn();
	}
}
#endif // RTCW_XX

/*
===================
RB_FogPass

Blends a fog texture on top of everything else
===================
*/
static void RB_FogPass( void ) {
	fog_t       *fog;
	int i;

#if !defined RTCW_ET
	if ( tr.refdef.rdflags & RDF_SNOOPERVIEW ) { // no fog pass in snooper
#else
	// no fog pass in snooper
	if ( tr.refdef.rdflags & RDF_SNOOPERVIEW || tess.shader->noFog || !r_wolffog->integer ) {
#endif // RTCW_XX

		return;
	}

#if defined RTCW_ET
	// ydnar: no world, no fogging
	if ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return;
	}
#endif // RTCW_XX

	::glEnableClientState( GL_COLOR_ARRAY );
	::glColorPointer( 4, GL_UNSIGNED_BYTE, 0, tess.svars.colors );

	::glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	::glTexCoordPointer( 2, GL_FLOAT, 0, tess.svars.texcoords[0] );

	fog = tr.world->fogs + tess.fogNum;

	for ( i = 0; i < tess.numVertexes; i++ ) {

#if !defined RTCW_ET
		*( int * )&tess.svars.colors[i] = fog->colorInt;
#else
		*( int * )&tess.svars.colors[i] = fog->shader->fogParms.colorInt;
#endif // RTCW_XX

	}

	RB_CalcFogTexCoords( ( float * ) tess.svars.texcoords[0] );

	GL_Bind( tr.fogImage );

	if ( tess.shader->fogPass == FP_EQUAL ) {
		GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_EQUAL );
	} else {
		GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
	}

	R_DrawElements( tess.numIndexes, tess.indexes );
}

/*
===============
ComputeColors
===============
*/
static void ComputeColors( shaderStage_t *pStage ) {
	int i;

	//
	// rgbGen
	//
	switch ( pStage->rgbGen )
	{
	case CGEN_IDENTITY:
		memset( tess.svars.colors, 0xff, tess.numVertexes * 4 );
		break;
	default:
	case CGEN_IDENTITY_LIGHTING:
		memset( tess.svars.colors, tr.identityLightByte, tess.numVertexes * 4 );
		break;
	case CGEN_LIGHTING_DIFFUSE:
		RB_CalcDiffuseColor( ( unsigned char * ) tess.svars.colors );
		break;
	case CGEN_EXACT_VERTEX:

#if !defined RTCW_ET
		memcpy( tess.svars.colors, tess.vertexColors, tess.numVertexes * sizeof( tess.vertexColors[0] ) );
#else
		memcpy( tess.svars.colors, tess.vertexColors, tess.numVertexes * sizeof( tess.vertexColors[0].v ) );
#endif // RTCW_XX

		break;
	case CGEN_CONST:
		for ( i = 0; i < tess.numVertexes; i++ ) {
			*(int *)tess.svars.colors[i] = *(int *)pStage->constantColor;
		}
		break;
	case CGEN_VERTEX:
		if ( tr.identityLight == 1 ) {

#if !defined RTCW_ET
			memcpy( tess.svars.colors, tess.vertexColors, tess.numVertexes * sizeof( tess.vertexColors[0] ) );
#else
			memcpy( tess.svars.colors, tess.vertexColors, tess.numVertexes * sizeof( tess.vertexColors[0].v ) );
#endif // RTCW_XX

		} else
		{
			for ( i = 0; i < tess.numVertexes; i++ )
			{

#if !defined RTCW_ET
				tess.svars.colors[i][0] = tess.vertexColors[i][0] * tr.identityLight;
				tess.svars.colors[i][1] = tess.vertexColors[i][1] * tr.identityLight;
				tess.svars.colors[i][2] = tess.vertexColors[i][2] * tr.identityLight;
				tess.svars.colors[i][3] = tess.vertexColors[i][3];
#else
				tess.svars.colors[i][0] = tess.vertexColors[i].v[0] * tr.identityLight;
				tess.svars.colors[i][1] = tess.vertexColors[i].v[1] * tr.identityLight;
				tess.svars.colors[i][2] = tess.vertexColors[i].v[2] * tr.identityLight;
				tess.svars.colors[i][3] = tess.vertexColors[i].v[3];
#endif // RTCW_XX

			}
		}
		break;
	case CGEN_ONE_MINUS_VERTEX:
		if ( tr.identityLight == 1 ) {
			for ( i = 0; i < tess.numVertexes; i++ )
			{

#if !defined RTCW_ET
				tess.svars.colors[i][0] = 255 - tess.vertexColors[i][0];
				tess.svars.colors[i][1] = 255 - tess.vertexColors[i][1];
				tess.svars.colors[i][2] = 255 - tess.vertexColors[i][2];
#else
				tess.svars.colors[i][0] = 255 - tess.vertexColors[i].v[0];
				tess.svars.colors[i][1] = 255 - tess.vertexColors[i].v[1];
				tess.svars.colors[i][2] = 255 - tess.vertexColors[i].v[2];
#endif // RTCW_XX

			}
		} else
		{
			for ( i = 0; i < tess.numVertexes; i++ )
			{

#if !defined RTCW_ET
				tess.svars.colors[i][0] = ( 255 - tess.vertexColors[i][0] ) * tr.identityLight;
				tess.svars.colors[i][1] = ( 255 - tess.vertexColors[i][1] ) * tr.identityLight;
				tess.svars.colors[i][2] = ( 255 - tess.vertexColors[i][2] ) * tr.identityLight;
#else
				tess.svars.colors[i][0] = ( 255 - tess.vertexColors[i].v[0] ) * tr.identityLight;
				tess.svars.colors[i][1] = ( 255 - tess.vertexColors[i].v[1] ) * tr.identityLight;
				tess.svars.colors[i][2] = ( 255 - tess.vertexColors[i].v[2] ) * tr.identityLight;
#endif // RTCW_XX

			}
		}
		break;
	case CGEN_FOG:
	{
		fog_t       *fog;

		fog = tr.world->fogs + tess.fogNum;

		for ( i = 0; i < tess.numVertexes; i++ ) {

#if !defined RTCW_ET
			*( int * )&tess.svars.colors[i] = fog->colorInt;
#else
			*( int * )&tess.svars.colors[i] = fog->shader->fogParms.colorInt;
#endif // RTCW_XX

		}
	}
	break;
	case CGEN_WAVEFORM:
		RB_CalcWaveColor( &pStage->rgbWave, ( unsigned char * ) tess.svars.colors );
		break;
	case CGEN_ENTITY:
		RB_CalcColorFromEntity( ( unsigned char * ) tess.svars.colors );
		break;
	case CGEN_ONE_MINUS_ENTITY:
		RB_CalcColorFromOneMinusEntity( ( unsigned char * ) tess.svars.colors );
		break;
	}

	//
	// alphaGen
	//
	switch ( pStage->alphaGen )
	{
	case AGEN_SKIP:
		break;
	case AGEN_IDENTITY:
		if ( pStage->rgbGen != CGEN_IDENTITY ) {
			if ( ( pStage->rgbGen == CGEN_VERTEX && tr.identityLight != 1 ) ||
				 pStage->rgbGen != CGEN_VERTEX ) {
				for ( i = 0; i < tess.numVertexes; i++ ) {
					tess.svars.colors[i][3] = 0xff;
				}
			}
		}
		break;
	case AGEN_CONST:
		if ( pStage->rgbGen != CGEN_CONST ) {
			for ( i = 0; i < tess.numVertexes; i++ ) {
				tess.svars.colors[i][3] = pStage->constantColor[3];
			}
		}
		break;
	case AGEN_WAVEFORM:
		RB_CalcWaveAlpha( &pStage->alphaWave, ( unsigned char * ) tess.svars.colors );
		break;
	case AGEN_LIGHTING_SPECULAR:
		RB_CalcSpecularAlpha( ( unsigned char * ) tess.svars.colors );
		break;
	case AGEN_ENTITY:
		RB_CalcAlphaFromEntity( ( unsigned char * ) tess.svars.colors );
		break;
	case AGEN_ONE_MINUS_ENTITY:
		RB_CalcAlphaFromOneMinusEntity( ( unsigned char * ) tess.svars.colors );
		break;
		// Ridah
	case AGEN_NORMALZFADE:
	{
		float alpha, range, lowest, highest, dot;
		vec3_t worldUp;
		qboolean zombieEffect = qfalse;

		if ( VectorCompare( backEnd.currentEntity->e.fireRiseDir, vec3_origin ) ) {
			VectorSet( backEnd.currentEntity->e.fireRiseDir, 0, 0, 1 );
		}

		if ( backEnd.currentEntity->e.hModel ) {    // world surfaces dont have an axis
			VectorRotate( backEnd.currentEntity->e.fireRiseDir, backEnd.currentEntity->e.axis, worldUp );
		} else {
			VectorCopy( backEnd.currentEntity->e.fireRiseDir, worldUp );
		}

		lowest = pStage->zFadeBounds[0];
		if ( lowest == -1000 ) {    // use entity alpha
			lowest = backEnd.currentEntity->e.shaderTime;
			zombieEffect = qtrue;
		}
		highest = pStage->zFadeBounds[1];
		if ( highest == -1000 ) {   // use entity alpha
			highest = backEnd.currentEntity->e.shaderTime;
			zombieEffect = qtrue;
		}
		range = highest - lowest;
		for ( i = 0; i < tess.numVertexes; i++ ) {

#if !defined RTCW_ET
			dot = DotProduct( tess.normal[i], worldUp );
#else
			dot = DotProduct( tess.normal[i].v, worldUp );
#endif // RTCW_XX

			// special handling for Zombie fade effect
			if ( zombieEffect ) {
				alpha = (float)backEnd.currentEntity->e.shaderRGBA[3] * ( dot + 1.0 ) / 2.0;
				alpha += ( 2.0 * (float)backEnd.currentEntity->e.shaderRGBA[3] ) * ( 1.0 - ( dot + 1.0 ) / 2.0 );
				if ( alpha > 255.0 ) {
					alpha = 255.0;
				} else if ( alpha < 0.0 ) {
					alpha = 0.0;
				}
				tess.svars.colors[i][3] = (byte)( alpha );
				continue;
			}

			if ( dot < highest ) {
				if ( dot > lowest ) {
					if ( dot < lowest + range / 2 ) {
						alpha = ( (float)pStage->constantColor[3] * ( ( dot - lowest ) / ( range / 2 ) ) );
					} else {
						alpha = ( (float)pStage->constantColor[3] * ( 1.0 - ( ( dot - lowest - range / 2 ) / ( range / 2 ) ) ) );
					}
					if ( alpha > 255.0 ) {
						alpha = 255.0;
					} else if ( alpha < 0.0 ) {
						alpha = 0.0;
					}

					// finally, scale according to the entity's alpha
					if ( backEnd.currentEntity->e.hModel ) {
						alpha *= (float)backEnd.currentEntity->e.shaderRGBA[3] / 255.0;
					}

					tess.svars.colors[i][3] = (byte)( alpha );
				} else {
					tess.svars.colors[i][3] = 0;
				}
			} else {
				tess.svars.colors[i][3] = 0;
			}
		}
	}
	break;
		// done.
	case AGEN_VERTEX:
		if ( pStage->rgbGen != CGEN_VERTEX ) {
			for ( i = 0; i < tess.numVertexes; i++ ) {

#if !defined RTCW_ET
				tess.svars.colors[i][3] = tess.vertexColors[i][3];
#else
				tess.svars.colors[i][3] = tess.vertexColors[i].v[3];
#endif // RTCW_XX

			}
		}
		break;
	case AGEN_ONE_MINUS_VERTEX:
		for ( i = 0; i < tess.numVertexes; i++ )
		{

#if !defined RTCW_ET
			tess.svars.colors[i][3] = 255 - tess.vertexColors[i][3];
#else
			tess.svars.colors[i][3] = 255 - tess.vertexColors[i].v[3];
#endif // RTCW_XX

		}
		break;
	case AGEN_PORTAL:
	{
		unsigned char alpha;

		for ( i = 0; i < tess.numVertexes; i++ )
		{
			float len;
			vec3_t v;

#if !defined RTCW_ET
			VectorSubtract( tess.xyz[i], backEnd.viewParms.or.origin, v );
#else
			VectorSubtract( tess.xyz[i].v, backEnd.viewParms.orientation.origin, v );
#endif // RTCW_XX

			len = VectorLength( v );

			len /= tess.shader->portalRange;

			if ( len < 0 ) {
				alpha = 0;
			} else if ( len > 1 )   {
				alpha = 0xff;
			} else
			{
				alpha = len * 0xff;
			}

			tess.svars.colors[i][3] = alpha;
		}
	}
	break;
	}

	//
	// fog adjustment for colors to fade out as fog increases
	//

#if !defined RTCW_ET
	if ( tess.fogNum ) {
#else
	if ( tess.fogNum && !tess.shader->noFog ) {
#endif // RTCW_XX

		switch ( pStage->adjustColorsForFog )
		{
		case ACFF_MODULATE_RGB:
			RB_CalcModulateColorsByFog( ( unsigned char * ) tess.svars.colors );
			break;
		case ACFF_MODULATE_ALPHA:
			RB_CalcModulateAlphasByFog( ( unsigned char * ) tess.svars.colors );
			break;
		case ACFF_MODULATE_RGBA:
			RB_CalcModulateRGBAsByFog( ( unsigned char * ) tess.svars.colors );
			break;
		case ACFF_NONE:
			break;
		}
	}
}

/*
===============
ComputeTexCoords
===============
*/
static void ComputeTexCoords( shaderStage_t *pStage ) {
	int i;
	int b;

	for ( b = 0; b < NUM_TEXTURE_BUNDLES; b++ ) {
		int tm;

		//
		// generate the texture coordinates
		//
		switch ( pStage->bundle[b].tcGen )
		{
		case TCGEN_IDENTITY:
			memset( tess.svars.texcoords[b], 0, sizeof( float ) * 2 * tess.numVertexes );
			break;
		case TCGEN_TEXTURE:
			for ( i = 0 ; i < tess.numVertexes ; i++ ) {

#if !defined RTCW_ET
				tess.svars.texcoords[b][i][0] = tess.texCoords[i][0][0];
				tess.svars.texcoords[b][i][1] = tess.texCoords[i][0][1];
#else
				tess.svars.texcoords[b][i][0] = tess.texCoords0[i].v[0];
				tess.svars.texcoords[b][i][1] = tess.texCoords0[i].v[1];
#endif // RTCW_XX

			}
			break;
		case TCGEN_LIGHTMAP:
			for ( i = 0 ; i < tess.numVertexes ; i++ ) {

#if !defined RTCW_ET
				tess.svars.texcoords[b][i][0] = tess.texCoords[i][1][0];
				tess.svars.texcoords[b][i][1] = tess.texCoords[i][1][1];
#else
				tess.svars.texcoords[b][i][0] = tess.texCoords1[i].v[0];
				tess.svars.texcoords[b][i][1] = tess.texCoords1[i].v[1];
#endif // RTCW_XX

			}
			break;
		case TCGEN_VECTOR:
			for ( i = 0 ; i < tess.numVertexes ; i++ ) {

#if !defined RTCW_ET
				tess.svars.texcoords[b][i][0] = DotProduct( tess.xyz[i], pStage->bundle[b].tcGenVectors[0] );
				tess.svars.texcoords[b][i][1] = DotProduct( tess.xyz[i], pStage->bundle[b].tcGenVectors[1] );
#else
				tess.svars.texcoords[b][i][0] = DotProduct( tess.xyz[i].v, pStage->bundle[b].tcGenVectors[0] );
				tess.svars.texcoords[b][i][1] = DotProduct( tess.xyz[i].v, pStage->bundle[b].tcGenVectors[1] );
#endif // RTCW_XX

			}
			break;
		case TCGEN_FOG:
			RB_CalcFogTexCoords( ( float * ) tess.svars.texcoords[b] );
			break;
		case TCGEN_ENVIRONMENT_MAPPED:
			RB_CalcEnvironmentTexCoords( ( float * ) tess.svars.texcoords[b] );
			break;
		case TCGEN_FIRERISEENV_MAPPED:
			RB_CalcFireRiseEnvTexCoords( ( float * ) tess.svars.texcoords[b] );
			break;
		case TCGEN_BAD:
			return;
		}

		//
		// alter texture coordinates
		//
		for ( tm = 0; tm < pStage->bundle[b].numTexMods ; tm++ ) {
			switch ( pStage->bundle[b].texMods[tm].type )
			{
			case TMOD_NONE:
				tm = TR_MAX_TEXMODS;        // break out of for loop
				break;

			case TMOD_SWAP:
				RB_CalcSwapTexCoords( ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_TURBULENT:
				RB_CalcTurbulentTexCoords( &pStage->bundle[b].texMods[tm].wave,
										   ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_ENTITY_TRANSLATE:
				RB_CalcScrollTexCoords( backEnd.currentEntity->e.shaderTexCoord,
										( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_SCROLL:
				RB_CalcScrollTexCoords( pStage->bundle[b].texMods[tm].scroll,
										( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_SCALE:
				RB_CalcScaleTexCoords( pStage->bundle[b].texMods[tm].scale,
									   ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_STRETCH:
				RB_CalcStretchTexCoords( &pStage->bundle[b].texMods[tm].wave,
										 ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_TRANSFORM:
				RB_CalcTransformTexCoords( &pStage->bundle[b].texMods[tm],
										   ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_ROTATE:
				RB_CalcRotateTexCoords( pStage->bundle[b].texMods[tm].rotateSpeed,
										( float * ) tess.svars.texcoords[b] );
				break;

			default:
				ri.Error( ERR_DROP, "ERROR: unknown texmod '%d' in shader '%s'\n", pStage->bundle[b].texMods[tm].type, tess.shader->name );
				break;
			}
		}
	}
}



extern void R_Fog( glfog_t *curfog );

/*
==============
SetIteratorFog
	set the fog parameters for this pass
==============
*/
void SetIteratorFog( void ) {
	// changed for problem when you start the game with r_fastsky set to '1'
//	if(r_fastsky->integer || backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) {
	if ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) {
		R_FogOff();
		return;
	}

	if ( backEnd.refdef.rdflags & RDF_DRAWINGSKY ) {
		if ( glfogsettings[FOG_SKY].registered ) {
			R_Fog( &glfogsettings[FOG_SKY] );
		} else {
			R_FogOff();
		}

		return;
	}

	if ( skyboxportal && backEnd.refdef.rdflags & RDF_SKYBOXPORTAL ) {
		if ( glfogsettings[FOG_PORTALVIEW].registered ) {
			R_Fog( &glfogsettings[FOG_PORTALVIEW] );
		} else {
			R_FogOff();
		}
	} else {
		if ( glfogNum > FOG_NONE ) {
			R_Fog( &glfogsettings[FOG_CURRENT] );
		} else {
			R_FogOff();
		}
	}
}


/*
** RB_IterateStagesGeneric
*/
static void RB_IterateStagesGeneric( shaderCommands_t *input ) {
	int stage;

	for ( stage = 0; stage < MAX_SHADER_STAGES; stage++ )
	{
		shaderStage_t *pStage = tess.xstages[stage];

		if ( !pStage ) {
			break;
		}

		ComputeColors( pStage );
		ComputeTexCoords( pStage );

		if ( !setArraysOnce ) {
			::glEnableClientState( GL_COLOR_ARRAY );
			::glColorPointer( 4, GL_UNSIGNED_BYTE, 0, input->svars.colors );
		}

		//
		// do multitexture
		//
		if ( pStage->bundle[1].image[0] != 0 ) {
			DrawMultitextured( input, stage );
		} else
		{
			int fadeStart, fadeEnd;

			if ( !setArraysOnce ) {
				::glTexCoordPointer( 2, GL_FLOAT, 0, input->svars.texcoords[0] );
			}

			//
			// set state
			//

#if !defined RTCW_ET

            //BBi
			//if ( pStage->bundle[0].vertexLightmap && ( ( r_vertexLight->integer && !r_uiFullScreen->integer ) || glConfig.hardwareType == GLHW_PERMEDIA2 ) && r_lightmap->integer ) {
            if ((pStage->bundle[0].vertexLightmap) &&
                (r_vertexLight->integer != 0) &&
                (r_uiFullScreen->integer == 0) &&
                (r_lightmap->integer != 0))
            {
            //BBi
				GL_Bind( tr.whiteImage );
			} else {
#endif // RTCW_XX

				R_BindAnimatedImage( &pStage->bundle[0] );

#if !defined RTCW_ET
			}
#endif // RTCW_XX

			// Ridah, per stage fogging (detail textures)
			if ( tess.shader->noFog && pStage->isFogged ) {
				R_FogOn();
			} else if ( tess.shader->noFog && !pStage->isFogged ) {
				R_FogOff(); // turn it back off
			} else {    // make sure it's on
				R_FogOn();
			}
			// done.

			//----(SA)	fading model stuff
			fadeStart = backEnd.currentEntity->e.fadeStartTime;

			if ( fadeStart ) {
				fadeEnd = backEnd.currentEntity->e.fadeEndTime;
				if ( fadeStart > tr.refdef.time ) {       // has not started to fade yet
					GL_State( pStage->stateBits );
				} else
				{
					int i;
					unsigned int tempState;
					float alphaval;

					if ( fadeEnd < tr.refdef.time ) {     // entity faded out completely
						continue;
					}

					alphaval = (float)( fadeEnd - tr.refdef.time ) / (float)( fadeEnd - fadeStart );

					tempState = pStage->stateBits;
					// remove the current blend, and don't write to Z buffer
					tempState &= ~( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS | GLS_DEPTHMASK_TRUE );
					// set the blend to src_alpha, dst_one_minus_src_alpha
					tempState |= ( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
					GL_State( tempState );
					GL_Cull( CT_FRONT_SIDED );
					// modulate the alpha component of each vertex in the render list
					for ( i = 0; i < tess.numVertexes; i++ ) {
						tess.svars.colors[i][0] *= alphaval;
						tess.svars.colors[i][1] *= alphaval;
						tess.svars.colors[i][2] *= alphaval;
						tess.svars.colors[i][3] *= alphaval;
					}
				}

#if !defined RTCW_ET
			} else {
				GL_State( pStage->stateBits );
#endif // RTCW_XX

			}
			//----(SA)	end

#if defined RTCW_ET
			// ydnar: lightmap stages should be GL_ONE GL_ZERO so they can be seen
			else if ( r_lightmap->integer && ( pStage->bundle[0].isLightmap || pStage->bundle[1].isLightmap ) ) {
				unsigned int stateBits;


				stateBits = ( pStage->stateBits & ~( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) |
							( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );
				GL_State( stateBits );
			} else {
				GL_State( pStage->stateBits );
			}
#endif // RTCW_XX

			//
			// draw
			//
			R_DrawElements( input->numIndexes, input->indexes );
		}
		// allow skipping out to show just lightmaps during development

#if !defined RTCW_ET
		if ( r_lightmap->integer && ( pStage->bundle[0].isLightmap || pStage->bundle[1].isLightmap || pStage->bundle[0].vertexLightmap ) ) {
#else
		if ( r_lightmap->integer && ( pStage->bundle[0].isLightmap || pStage->bundle[1].isLightmap ) ) {
#endif // RTCW_XX

			break;
		}
	}
}


/*
** RB_StageIteratorGeneric
*/
void RB_StageIteratorGeneric( void ) {
	shaderCommands_t *input;

	input = &tess;

	RB_DeformTessGeometry();

    //BBi
	////
	//// log this call
	////
	//if ( r_logFile->integer ) {
	//	// don't just call LogComment, or we will get
	//	// a call to va() every frame!
	//	GLimp_LogComment( va( "--- RB_StageIteratorGeneric( %s ) ---\n", tess.shader->name ) );
	//}
    //BBi

	// set GL fog
	SetIteratorFog();

//BBi
//#if defined RTCW_SP
//	if ( ::glPNTrianglesiATI && tess.ATI_tess ) {
//		// RF< so we can send the normals as an array
//		::glEnableClientState( GL_NORMAL_ARRAY );
//#ifdef __MACOS__    //DAJ ATI
//		::glPNTrianglesiATI( GL_PN_TRIANGLES_ATI, 1 );
//#else
//		::glEnable( GL_PN_TRIANGLES_ATI ); // ATI PN-Triangles extension
//#endif
//	}
//#endif // RTCW_XX
//BBi

	//
	// set face culling appropriately
	//
	GL_Cull( input->shader->cullType );

	// set polygon offset if necessary
	if ( input->shader->polygonOffset ) {
		::glEnable( GL_POLYGON_OFFSET_FILL );
		::glPolygonOffset( r_offsetFactor->value, r_offsetUnits->value );
	}

	//
	// if there is only a single pass then we can enable color
	// and texture arrays before we compile, otherwise we need
	// to avoid compiling those arrays since they will change
	// during multipass rendering
	//
	if ( tess.numPasses > 1 || input->shader->multitextureEnv ) {
		setArraysOnce = qfalse;
		::glDisableClientState( GL_COLOR_ARRAY );
		::glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	} else
	{
		setArraysOnce = qtrue;

		::glEnableClientState( GL_COLOR_ARRAY );
		::glColorPointer( 4, GL_UNSIGNED_BYTE, 0, tess.svars.colors );

		::glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		::glTexCoordPointer( 2, GL_FLOAT, 0, tess.svars.texcoords[0] );
	}

//BBi
//#if defined RTCW_SP
//	// RF, send normals only if required
//	// This must be done first, since we can't change the arrays once they have been
//	// locked
//	if ( ::glPNTrianglesiATI && tess.ATI_tess ) {
//		::glNormalPointer( GL_FLOAT, 16, input->normal );
//	}
//#endif // RTCW_XX
//BBi

	//
	// lock XYZ
	//
	::glVertexPointer( 3, GL_FLOAT, 16, input->xyz ); // padded for SIMD
	if ( glConfigEx.useExtCompiledVertexArray ) {
		::glLockArraysEXT( 0, input->numVertexes );
		//GLimp_LogComment( "glLockArraysEXT\n" );
	}

	//
	// enable color and texcoord arrays after the lock if necessary
	//
	if ( !setArraysOnce ) {
		::glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		::glEnableClientState( GL_COLOR_ARRAY );
	}

	//
	// call shader function
	//
	RB_IterateStagesGeneric( input );

	//
	// now do any dynamic lighting needed
	//

#if !defined RTCW_ET
	if ( tess.dlightBits && tess.shader->sort <= SS_OPAQUE
		 && !( tess.shader->surfaceFlags & ( SURF_NODLIGHT | SURF_SKY ) ) ) {
		ProjectDlightTexture();
#else
	//%	tess.dlightBits = 255;	// HACK!
	//%	if( tess.dlightBits && tess.shader->sort <= SS_OPAQUE &&
	if ( tess.dlightBits && tess.shader->fogPass &&
		 !( tess.shader->surfaceFlags & ( SURF_NODLIGHT | SURF_SKY ) ) ) {
		if ( r_dynamiclight->integer == 2 ) {
			DynamicLightPass();
		} else {
			DynamicLightSinglePass();
		}
#endif // RTCW_XX

	}

	//
	// now do fog
	//
	if ( tess.fogNum && tess.shader->fogPass ) {
		RB_FogPass();
	}

	//
	// unlock arrays
	//
	if ( glConfigEx.useExtCompiledVertexArray ) {
		::glUnlockArraysEXT();
		//GLimp_LogComment( "glUnlockArraysEXT\n" );
	}

	//
	// reset polygon offset
	//
	if ( input->shader->polygonOffset ) {
		::glDisable( GL_POLYGON_OFFSET_FILL );
	}

//BBi
//#if defined RTCW_SP
//	// turn truform back off
//	if ( ::glPNTrianglesiATI && tess.ATI_tess ) {
//#ifdef __MACOS__    //DAJ ATI
//		::glPNTrianglesiATI( GL_PN_TRIANGLES_ATI, 0 );
//#else
//		::glDisable( GL_PN_TRIANGLES_ATI );    // ATI PN-Triangles extension
//#endif
//		::glDisableClientState( GL_NORMAL_ARRAY );
//	}
//#endif // RTCW_XX
//BBi

}


/*
** RB_StageIteratorVertexLitTexture
*/
void RB_StageIteratorVertexLitTexture( void ) {
	shaderCommands_t *input;
	shader_t        *shader;

	input = &tess;

	shader = input->shader;

	//
	// compute colors
	//
	RB_CalcDiffuseColor( ( unsigned char * ) tess.svars.colors );

    //BBi
	////
	//// log this call
	////
	//if ( r_logFile->integer ) {
	//	// don't just call LogComment, or we will get
	//	// a call to va() every frame!
	//	GLimp_LogComment( va( "--- RB_StageIteratorVertexLitTexturedUnfogged( %s ) ---\n", tess.shader->name ) );
	//}
    //BBi


	// set GL fog
	SetIteratorFog();

	//
	// set face culling appropriately
	//
	GL_Cull( input->shader->cullType );

	//
	// set arrays and lock
	//
	::glEnableClientState( GL_COLOR_ARRAY );
	::glEnableClientState( GL_TEXTURE_COORD_ARRAY );

//BBi
//#if defined RTCW_SP
//	if ( ::glPNTrianglesiATI && tess.ATI_tess ) {
//#ifdef __MACOS__    //DAJ ATI
//		::glPNTrianglesiATI( GL_PN_TRIANGLES_ATI, 1 );
//#else
//		::glEnable( GL_PN_TRIANGLES_ATI ); // ATI PN-Triangles extension
//#endif
//		::glEnableClientState( GL_NORMAL_ARRAY );         // RF< so we can send the normals as an array
//		::glNormalPointer( GL_FLOAT, 16, input->normal );
//	}
//#endif // RTCW_XX
//BBi

	::glColorPointer( 4, GL_UNSIGNED_BYTE, 0, tess.svars.colors );

#if !defined RTCW_ET
	::glTexCoordPointer( 2, GL_FLOAT, 16, tess.texCoords[0][0] );
#else
	::glTexCoordPointer( 2, GL_FLOAT, 8, tess.texCoords0 );
#endif // RTCW_XX

	::glVertexPointer( 3, GL_FLOAT, 16, input->xyz );

	if ( glConfigEx.useExtCompiledVertexArray ) {
		::glLockArraysEXT( 0, input->numVertexes );
		//GLimp_LogComment( "glLockArraysEXT\n" );
	}

	//
	// call special shade routine
	//
	R_BindAnimatedImage( &tess.xstages[0]->bundle[0] );
	GL_State( tess.xstages[0]->stateBits );
	R_DrawElements( input->numIndexes, input->indexes );

	//
	// now do any dynamic lighting needed
	//

#if !defined RTCW_ET
	if ( tess.dlightBits && tess.shader->sort <= SS_OPAQUE ) {
		ProjectDlightTexture();
	}
#else
	//%	if ( tess.dlightBits && tess.shader->sort <= SS_OPAQUE )
	if ( tess.dlightBits && tess.shader->fogPass &&
		 !( tess.shader->surfaceFlags & ( SURF_NODLIGHT | SURF_SKY ) ) ) {
		if ( r_dynamiclight->integer == 2 ) {
			DynamicLightPass();
		} else {
			DynamicLightSinglePass();
		}
	}
#endif // RTCW_XX

	//
	// now do fog
	//
	if ( tess.fogNum && tess.shader->fogPass ) {
		RB_FogPass();
	}

	//
	// unlock arrays
	//
	if ( glConfigEx.useExtCompiledVertexArray ) {
		::glUnlockArraysEXT();
		//GLimp_LogComment( "glUnlockArraysEXT\n" );
	}

//BBi
//#if defined RTCW_SP
//	if ( ::glPNTrianglesiATI && tess.ATI_tess )
//#ifdef __MACOS__ //DAJ ATI{
//		::glPNTrianglesiATI( GL_PN_TRIANGLES_ATI, 0 );
//	}
//#else
//	{ ::glDisable( GL_PN_TRIANGLES_ATI );    // ATI PN-Triangles extension
//	}
//#endif
//#endif // RTCW_XX
//BBi

}

//define	REPLACE_MODE

void RB_StageIteratorLightmappedMultitexture( void ) {
	shaderCommands_t *input;

	input = &tess;

    //BBi
	////
	//// log this call
	////
	//if ( r_logFile->integer ) {
	//	// don't just call LogComment, or we will get
	//	// a call to va() every frame!
	//	GLimp_LogComment( va( "--- RB_StageIteratorLightmappedMultitexture( %s ) ---\n", tess.shader->name ) );
	//}
    //BBi

	// set GL fog
	SetIteratorFog();

	//
	// set face culling appropriately
	//
	GL_Cull( input->shader->cullType );

	//
	// set color, pointers, and lock
	//
	GL_State( GLS_DEFAULT );
	::glVertexPointer( 3, GL_FLOAT, 16, input->xyz );

//BBi
//#if defined RTCW_SP
//	if ( ::glPNTrianglesiATI && tess.ATI_tess ) {
//#ifdef __MACOS__    //DAJ ATI
//		::glPNTrianglesiATI( GL_PN_TRIANGLES_ATI, 1 );
//#else
//		::glEnable( GL_PN_TRIANGLES_ATI ); // ATI PN-Triangles extension
//#endif
//		::glNormalPointer( GL_FLOAT, 16, input->normal );
//	}
//#endif // RTCW_XX
//BBi

#ifdef REPLACE_MODE
	::glDisableClientState( GL_COLOR_ARRAY );
	::glColor3f( 1, 1, 1 );
	::glShadeModel( GL_FLAT );
#else
	::glEnableClientState( GL_COLOR_ARRAY );
	::glColorPointer( 4, GL_UNSIGNED_BYTE, 0, tess.constantColor255 );
#endif

	//
	// select base stage
	//
	GL_SelectTexture( 0 );

	::glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	R_BindAnimatedImage( &tess.xstages[0]->bundle[0] );

#if !defined RTCW_ET
	::glTexCoordPointer( 2, GL_FLOAT, 16, tess.texCoords[0][0] );
#else
	::glTexCoordPointer( 2, GL_FLOAT, 8, tess.texCoords0 );
#endif // RTCW_XX

	//
	// configure second stage
	//
	GL_SelectTexture( 1 );
	::glEnable( GL_TEXTURE_2D );
	if ( r_lightmap->integer ) {
		GL_TexEnv( GL_REPLACE );
	} else {
		GL_TexEnv( GL_MODULATE );
	}

//----(SA)	modified for snooper
	if ( tess.xstages[0]->bundle[1].isLightmap && ( backEnd.refdef.rdflags & RDF_SNOOPERVIEW ) ) {
		GL_Bind( tr.whiteImage );
	} else {
		R_BindAnimatedImage( &tess.xstages[0]->bundle[1] );
	}

	::glEnableClientState( GL_TEXTURE_COORD_ARRAY );

#if !defined RTCW_ET
	::glTexCoordPointer( 2, GL_FLOAT, 16, tess.texCoords[0][1] );
#else
	::glTexCoordPointer( 2, GL_FLOAT, 8, tess.texCoords1 );
#endif // RTCW_XX

	//
	// lock arrays
	//
	if ( glConfigEx.useExtCompiledVertexArray ) {
		::glLockArraysEXT( 0, input->numVertexes );
		//GLimp_LogComment( "glLockArraysEXT\n" );
	}

	R_DrawElements( input->numIndexes, input->indexes );

	//
	// disable texturing on TEXTURE1, then select TEXTURE0
	//
	::glDisable( GL_TEXTURE_2D );
	::glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	GL_SelectTexture( 0 );
#ifdef REPLACE_MODE
	GL_TexEnv( GL_MODULATE );
	::glShadeModel( GL_SMOOTH );
#endif

	//
	// now do any dynamic lighting needed
	//

#if !defined RTCW_ET
	if ( tess.dlightBits && tess.shader->sort <= SS_OPAQUE ) {
		ProjectDlightTexture();
#else
	//%	if ( tess.dlightBits && tess.shader->sort <= SS_OPAQUE )
	if ( tess.dlightBits && tess.shader->fogPass &&
		 !( tess.shader->surfaceFlags & ( SURF_NODLIGHT | SURF_SKY ) ) ) {
		if ( r_dynamiclight->integer == 2 ) {
			DynamicLightPass();
		} else {
			DynamicLightSinglePass();
		}
#endif // RTCW_XX

	}

	//
	// now do fog
	//
	if ( tess.fogNum && tess.shader->fogPass ) {
		RB_FogPass();
	}

	//
	// unlock arrays
	//
	if ( glConfigEx.useExtCompiledVertexArray ) {
		::glUnlockArraysEXT();
		//GLimp_LogComment( "glUnlockArraysEXT\n" );
	}

//BBi
//#if defined RTCW_SP
//	if ( ::glPNTrianglesiATI && tess.ATI_tess )
//#ifdef __MACOS__ //DAJ ATI{
//		::glPNTrianglesiATI( GL_PN_TRIANGLES_ATI, 0 );
//	}
//#else
//	{ ::glDisable( GL_PN_TRIANGLES_ATI );    // ATI PN-Triangles extension
//	}
//#endif
//#endif // RTCW_XX
//BBi

}

/*
** RB_EndSurface
*/
void RB_EndSurface( void ) {
	shaderCommands_t *input;

	input = &tess;

	if ( input->numIndexes == 0 ) {
		return;
	}

#if !defined RTCW_ET
	if ( input->indexes[SHADER_MAX_INDEXES - 1] != 0 ) {
		ri.Error( ERR_DROP, "RB_EndSurface() - SHADER_MAX_INDEXES hit" );
	}
	if ( input->xyz[SHADER_MAX_VERTEXES - 1][0] != 0 ) {
		ri.Error( ERR_DROP, "RB_EndSurface() - SHADER_MAX_VERTEXES hit" );
	}
#else
	if ( input->indexes[input->maxShaderIndicies - 1] != 0 ) {
		ri.Error( ERR_DROP, "RB_EndSurface() - input->maxShaderIndicies(%i) hit", input->maxShaderIndicies );
	}
	if ( input->xyz[input->maxShaderVerts - 1].v[0] != 0 ) {
		ri.Error( ERR_DROP, "RB_EndSurface() - input->maxShaderVerts(%i) hit", input->maxShaderVerts );
	}
#endif // RTCW_XX

	if ( tess.shader == tr.shadowShader ) {
		RB_ShadowTessEnd();
		return;
	}

	// for debugging of sort order issues, stop rendering after a given sort value
	if ( r_debugSort->integer && r_debugSort->integer < tess.shader->sort ) {
		return;

#if defined RTCW_SP
	}

	if ( skyboxportal ) {
		// world
		if ( !( backEnd.refdef.rdflags & RDF_SKYBOXPORTAL ) ) {
			if ( tess.currentStageIteratorFunc == RB_StageIteratorSky ) {  // don't process these tris at all
				return;
			}
		}
		// portal sky
		else {
			if ( !drawskyboxportal ) {
				if ( !( tess.currentStageIteratorFunc == RB_StageIteratorSky ) ) {  // /only/ process sky tris
					return;
				}
			}
		}
#endif // RTCW_XX

	}

	//
	// update performance counters
	//
	backEnd.pc.c_shaders++;
	backEnd.pc.c_vertexes += tess.numVertexes;
	backEnd.pc.c_indexes += tess.numIndexes;
	backEnd.pc.c_totalIndexes += tess.numIndexes * tess.numPasses;

	//
	// call off to shader specific tess end function
	//
	tess.currentStageIteratorFunc();

	//
	// draw debugging stuff
	//
	if ( r_showtris->integer ) {
		DrawTris( input );
	}
	if ( r_shownormals->integer ) {
		DrawNormals( input );
	}


	// clear shader so we can tell we don't have any unclosed surfaces
	tess.numIndexes = 0;

	//GLimp_LogComment( "----------\n" );
}

