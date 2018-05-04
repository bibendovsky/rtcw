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

#include "tr_local.h"

#if 0
backEndData_t   *backEndData[SMP_FRAMES];
#endif // 0
backEndData_t* backEndData;

backEndState_t backEnd;


static float s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};


// BBi
void ogl_tess2_draw (GLenum mode, int vertex_count,
    bool use_texture_coords, bool use_color)
{
    if (vertex_count <= 0)
        return;

    if (ogl_tess_program->program_ == 0)
        return;

    if (ogl_tess2_vbo == 0)
        return;

    bool use_pos_array = (ogl_tess_program->a_pos_vec4 >= 0);
    bool use_tc0_array = use_texture_coords &&
        (ogl_tess_program->a_pos_vec4 >= 0);
    bool use_col_array = use_color &&
        (ogl_tess_program->a_col_vec4 >= 0);

    if ((!use_pos_array) && (!use_tc0_array) && (!use_col_array))
        return;


    if ((ogl_tess2_base_vertex + vertex_count) >
        OglTessLayout::MAX_VERTEX_COUNT)
    {
        ogl_tess2_base_vertex = 0;
    }

    ::glUseProgram (ogl_tess_program->program_);

    ::glBindBuffer (GL_ARRAY_BUFFER, ogl_tess2_vbo);

    if (use_pos_array) {
        ::glBufferSubData (GL_ARRAY_BUFFER,
            OglTessLayout::POS_OFS +
                (ogl_tess2_base_vertex * OglTessLayout::POS_SIZE),
            vertex_count * OglTessLayout::POS_SIZE,
            ogl_tess2.position);
    }

    if (use_tc0_array) {
        ::glBufferSubData (GL_ARRAY_BUFFER,
            OglTessLayout::TC0_OFS +
                (ogl_tess2_base_vertex * OglTessLayout::TC0_SIZE),
            vertex_count * OglTessLayout::TC0_SIZE,
            ogl_tess2.texture_coords[0]);
    }

    if (use_col_array) {
        ::glBufferSubData (GL_ARRAY_BUFFER,
            OglTessLayout::COL_OFS +
                (ogl_tess2_base_vertex * OglTessLayout::COL_SIZE),
            vertex_count * OglTessLayout::COL_SIZE,
            ogl_tess2.color);
    }

    // position
    if (use_pos_array) {
        ::glVertexAttribPointer (ogl_tess_program->a_pos_vec4, 3, GL_FLOAT,
            GL_FALSE, OglTessLayout::POS_SIZE,
            OglTessLayout::POS_PTR);

        ::glEnableVertexAttribArray (ogl_tess_program->a_pos_vec4);
    }

    // texture coordinates (0)
    if (use_tc0_array) {
        ::glVertexAttribPointer (ogl_tess_program->a_tc0_vec2, 2, GL_FLOAT,
            GL_FALSE, 0, OglTessLayout::TC0_PTR);

        ::glEnableVertexAttribArray (ogl_tess_program->a_tc0_vec2);
    }

    // color
    if (use_col_array) {
        ::glVertexAttribPointer (ogl_tess_program->a_col_vec4, 4,
            GL_UNSIGNED_BYTE, GL_TRUE, 0,
            OglTessLayout::COL_PTR);

        ::glEnableVertexAttribArray (ogl_tess_program->a_col_vec4);
    }

    ::glDrawArrays (mode, ogl_tess2_base_vertex, vertex_count);

    ::glUseProgram (0);

    if (use_pos_array)
        ::glDisableVertexAttribArray (ogl_tess_program->a_pos_vec4);

    if (use_tc0_array)
        ::glDisableVertexAttribArray (ogl_tess_program->a_tc0_vec2);

    if (use_col_array)
        ::glDisableVertexAttribArray (ogl_tess_program->a_col_vec4);

    ::glBindBuffer (GL_ARRAY_BUFFER, 0);

    ogl_tess2_base_vertex += vertex_count;
}
// BBi


/*
** GL_Bind
*/
void GL_Bind( image_t *image ) {
	int texnum;

	if ( !image ) {
		ri.Printf( PRINT_WARNING, "GL_Bind: NULL image\n" );
		texnum = tr.defaultImage->texnum;
	} else {
		texnum = image->texnum;
	}

	if ( r_nobind->integer && tr.dlightImage ) {        // performance evaluation option
		texnum = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[glState.currenttmu] != texnum ) {
		image->frameUsed = tr.frameCount;
		glState.currenttextures[glState.currenttmu] = texnum;
		::glBindTexture( GL_TEXTURE_2D, texnum );
	}
}

/*
** GL_SelectTexture
*/
void GL_SelectTexture( int unit ) {
	if ( glState.currenttmu == unit ) {
		return;
	}

	if ( unit == 0 ) {
        // BBi
        if (!glConfigEx.is_path_ogl_1_x ()) {
            glActiveTexture (GL_TEXTURE0);
        } else {
        // BBi

		glActiveTexture( GL_TEXTURE0_ARB );
		//GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE0_ARB )\n" );
		glClientActiveTexture( GL_TEXTURE0_ARB );
		//GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE0_ARB )\n" );

        // BBi
        }
        // BBi
	} else if ( unit == 1 )   {
        // BBi
        if (!glConfigEx.is_path_ogl_1_x ()) {
            glActiveTexture (GL_TEXTURE1);
        } else {
        // BBi

		glActiveTexture( GL_TEXTURE1_ARB );
		//GLimp_LogComment( "glActiveTextureARB( GL_TEXTURE1_ARB )\n" );
		glClientActiveTexture( GL_TEXTURE1_ARB );
		//GLimp_LogComment( "glClientActiveTextureARB( GL_TEXTURE1_ARB )\n" );

        // BBi
        }
        // BBi
	} else {
		ri.Error( ERR_DROP, "GL_SelectTexture: unit = %i", unit );
	}

	glState.currenttmu = unit;
}


/*
** GL_BindMultitexture
*/
void GL_BindMultitexture( image_t *image0, GLuint env0, image_t *image1, GLuint env1 ) {
	int texnum0, texnum1;

	texnum0 = image0->texnum;
	texnum1 = image1->texnum;

	if ( r_nobind->integer && tr.dlightImage ) {        // performance evaluation option
		texnum0 = texnum1 = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[1] != texnum1 ) {
		GL_SelectTexture( 1 );
		image1->frameUsed = tr.frameCount;
		glState.currenttextures[1] = texnum1;
		::glBindTexture( GL_TEXTURE_2D, texnum1 );
	}
	if ( glState.currenttextures[0] != texnum0 ) {
		GL_SelectTexture( 0 );
		image0->frameUsed = tr.frameCount;
		glState.currenttextures[0] = texnum0;
		::glBindTexture( GL_TEXTURE_2D, texnum0 );
	}
}


/*
** GL_Cull
*/
void GL_Cull( int cullType ) {
	if ( glState.faceCulling == cullType ) {
		return;
	}

	glState.faceCulling = cullType;

	if ( cullType == CT_TWO_SIDED ) {
		::glDisable( GL_CULL_FACE );
	} else
	{
		::glEnable( GL_CULL_FACE );

		if ( cullType == CT_BACK_SIDED ) {
			if ( backEnd.viewParms.isMirror ) {
				::glCullFace( GL_FRONT );
			} else
			{
				::glCullFace( GL_BACK );
			}
		} else
		{
			if ( backEnd.viewParms.isMirror ) {
				::glCullFace( GL_BACK );
			} else
			{
				::glCullFace( GL_FRONT );
			}
		}
	}
}

/*
** GL_TexEnv
*/
void GL_TexEnv( int env ) {
	if ( env == glState.texEnv[glState.currenttmu] ) {
		return;
	}

	glState.texEnv[glState.currenttmu] = env;


    // BBi
    int tu = glState.currenttmu;
    assert ((tu == 0) || (tu == 1));
    // BBi

	switch ( env )
	{
	case GL_MODULATE:
        // BBi
        if (!glConfigEx.is_path_ogl_1_x ()) {
            ogl_tess_state.tex_env_mode[tu].set (GL_MODULATE);
        } else {
        // BBi

		::glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

        // BBi
        }
        // BBi

		break;
	case GL_REPLACE:
        // BBi
        if (!glConfigEx.is_path_ogl_1_x ()) {
            ogl_tess_state.tex_env_mode[tu].set (GL_REPLACE);
        } else {
        // BBi

		::glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

        // BBi
        }
        // BBi

		break;
	case GL_DECAL:
        // BBi
        if (!glConfigEx.is_path_ogl_1_x ()) {
            ogl_tess_state.tex_env_mode[tu].set (GL_DECAL);
        } else {
        // BBi

		::glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

        // BBi
        }
        // BBi

		break;
	case GL_ADD:
        // BBi
        if (!glConfigEx.is_path_ogl_1_x ()) {
            ogl_tess_state.tex_env_mode[tu].set (GL_ADD);
        } else {
        // BBi

		::glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );

        // BBi
        }
        // BBi

		break;
	default:
		ri.Error( ERR_DROP, "GL_TexEnv: invalid env '%d' passed\n", env );
		break;
	}

    // BBi
    if (!glConfigEx.is_path_ogl_1_x ())
        ogl_tess_state.commit_changes ();
    // BBi
}

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in Q3.
*/
void GL_State( unsigned long stateBits ) {
	unsigned long diff = stateBits ^ glState.glStateBits;

	if ( !diff ) {
		return;
	}

	//
	// check depthFunc bits
	//
	if ( diff & GLS_DEPTHFUNC_EQUAL ) {
		if ( stateBits & GLS_DEPTHFUNC_EQUAL ) {
			::glDepthFunc( GL_EQUAL );
		} else
		{
			::glDepthFunc( GL_LEQUAL );
		}
	}

	//
	// check blend bits
	//
	if ( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) {
		GLenum srcFactor, dstFactor;

		if ( stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) {
			switch ( stateBits & GLS_SRCBLEND_BITS )
			{
			case GLS_SRCBLEND_ZERO:
				srcFactor = GL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				srcFactor = GL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				srcFactor = GL_DST_COLOR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				srcFactor = GL_ONE_MINUS_DST_COLOR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				srcFactor = GL_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				srcFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				srcFactor = GL_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				srcFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ALPHA_SATURATE:
				srcFactor = GL_SRC_ALPHA_SATURATE;
				break;
			default:
				srcFactor = GL_ONE;     // to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid src blend state bits\n" );
				break;
			}

			switch ( stateBits & GLS_DSTBLEND_BITS )
			{
			case GLS_DSTBLEND_ZERO:
				dstFactor = GL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				dstFactor = GL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				dstFactor = GL_SRC_COLOR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				dstFactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				dstFactor = GL_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				dstFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				dstFactor = GL_DST_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				dstFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				dstFactor = GL_ONE;     // to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid dst blend state bits\n" );
				break;
			}

			::glEnable( GL_BLEND );
			::glBlendFunc( srcFactor, dstFactor );
		} else
		{
			::glDisable( GL_BLEND );
		}
	}

	//
	// check depthmask
	//
	if ( diff & GLS_DEPTHMASK_TRUE ) {
		if ( stateBits & GLS_DEPTHMASK_TRUE ) {
			::glDepthMask( GL_TRUE );
		} else
		{
			::glDepthMask( GL_FALSE );
		}
	}

	//
	// fill/line mode
	//
	if ( diff & GLS_POLYMODE_LINE ) {
		if ( stateBits & GLS_POLYMODE_LINE ) {
			::glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		} else
		{
			::glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	//
	// depthtest
	//
	if ( diff & GLS_DEPTHTEST_DISABLE ) {
		if ( stateBits & GLS_DEPTHTEST_DISABLE ) {
			::glDisable( GL_DEPTH_TEST );
		} else
		{
			::glEnable( GL_DEPTH_TEST );
		}
	}

	//
	// alpha test
	//
	if ( diff & GLS_ATEST_BITS ) {
		switch ( stateBits & GLS_ATEST_BITS )
		{
		case 0:
            // BBi
            if (!glConfigEx.is_path_ogl_1_x ()) {
                ogl_tess_state.use_alpha_test.set (false);
            } else {
            // BBi

			::glDisable( GL_ALPHA_TEST );

            // BBi
            }
            // BBi

			break;
		case GLS_ATEST_GT_0:
            // BBi
            if (!glConfigEx.is_path_ogl_1_x ()) {
                ogl_tess_state.use_alpha_test.set (true);
                ogl_tess_state.alpha_test_func.set (GL_GREATER);
                ogl_tess_state.alpha_test_ref.set (0.0F);
            } else {
            // BBi

			::glEnable( GL_ALPHA_TEST );
			::glAlphaFunc( GL_GREATER, 0.0f );

            // BBi
            }
            // BBi

			break;
		case GLS_ATEST_LT_80:
            // BBi
            if (!glConfigEx.is_path_ogl_1_x ()) {
                ogl_tess_state.use_alpha_test.set (true);
                ogl_tess_state.alpha_test_func.set (GL_LESS);
                ogl_tess_state.alpha_test_ref.set (0.5F);
            } else {
            // BBi

			::glEnable( GL_ALPHA_TEST );
			::glAlphaFunc( GL_LESS, 0.5f );

            // BBi
            }
            // BBi

			break;
		case GLS_ATEST_GE_80:
            // BBi
            if (!glConfigEx.is_path_ogl_1_x ()) {
                ogl_tess_state.use_alpha_test.set (true);
                ogl_tess_state.alpha_test_func.set (GL_GEQUAL);
                ogl_tess_state.alpha_test_ref.set (0.5F);
            } else {
            // BBi

			::glEnable( GL_ALPHA_TEST );
			::glAlphaFunc( GL_GEQUAL, 0.5f );

            // BBi
            }
            // BBi

			break;
		default:
			assert( 0 );
			break;
		}

        // BBi
        if (!glConfigEx.is_path_ogl_1_x ())
            ogl_tess_state.commit_changes ();
        // BBi
	}

	glState.glStateBits = stateBits;
}



/*
================
RB_Hyperspace

A player has predicted a teleport, but hasn't arrived yet
================
*/
static void RB_Hyperspace( void ) {
	float c;

	if ( !backEnd.isHyperspace ) {
		// do initialization shit
	}

	c = ( backEnd.refdef.time & 255 ) / 255.0f;
	::glClearColor( c, c, c, 1 );
	::glClear( GL_COLOR_BUFFER_BIT );

	backEnd.isHyperspace = qtrue;
}


static void SetViewportAndScissor( void ) {
    // BBi
    if (!glConfigEx.is_path_ogl_1_x ()) {
        ogl_projection_stack.set_current (backEnd.viewParms.projectionMatrix);

        ogl_tess_state.projection.set (ogl_projection_stack.get_current ());
        ogl_tess_state.commit_changes ();
    } else {
    // BBi

	::glMatrixMode( GL_PROJECTION );
	::glLoadMatrixf( backEnd.viewParms.projectionMatrix );
	::glMatrixMode( GL_MODELVIEW );

    // BBi
    }
    // BBi

	// set the window clipping

#if defined RTCW_SP
	::glViewport(    backEnd.viewParms.viewportX,
					backEnd.viewParms.viewportY,
					backEnd.viewParms.viewportWidth,
					backEnd.viewParms.viewportHeight );

// TODO: insert handling for widescreen?  (when looking through camera)
	::glScissor(     backEnd.viewParms.viewportX,
					backEnd.viewParms.viewportY,
					backEnd.viewParms.viewportWidth,
					backEnd.viewParms.viewportHeight );
#else
	::glViewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
				 backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
	::glScissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
				backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
#endif // RTCW_XX

}

/*
=================
RB_BeginDrawingView

Any mirrored or portaled views have already been drawn, so prepare
to actually render the visible surfaces for this view
=================
*/
void RB_BeginDrawingView( void ) {
	int clearBits = 0;

	// sync with gl if needed
	if ( r_finish->integer == 1 && !glState.finishCalled ) {
		::glFinish();
		glState.finishCalled = qtrue;
	}
	if ( r_finish->integer == 0 ) {
		glState.finishCalled = qtrue;
	}

	// we will need to change the projection matrix before drawing
	// 2D images again
	backEnd.projection2D = qfalse;

	//
	// set the modelview matrix for the viewer
	//
	SetViewportAndScissor();

	// ensures that depth writes are enabled for the depth clear
	GL_State( GLS_DEFAULT );


////////// (SA) modified to ensure one glclear() per frame at most

	// clear relevant buffers
	clearBits = 0;

	if ( r_measureOverdraw->integer || r_shadows->integer == 2 ) {
		clearBits |= GL_STENCIL_BUFFER_BIT;
	}

#if !defined RTCW_ET
	if ( r_uiFullScreen->integer ) {
		clearBits = GL_DEPTH_BUFFER_BIT;    // (SA) always just clear depth for menus

	} else if ( skyboxportal ) {
		if ( backEnd.refdef.rdflags & RDF_SKYBOXPORTAL ) {   // portal scene, clear whatever is necessary
#else
//	if(r_uiFullScreen->integer) {
//		clearBits = GL_DEPTH_BUFFER_BIT;	// (SA) always just clear depth for menus
//	}
	// ydnar: global q3 fog volume
	else if ( tr.world && tr.world->globalFog >= 0 ) {
		clearBits |= GL_DEPTH_BUFFER_BIT;
		clearBits |= GL_COLOR_BUFFER_BIT;
		//
		::glClearColor( tr.world->fogs[tr.world->globalFog].shader->fogParms.color[ 0 ] * tr.identityLight,
					   tr.world->fogs[tr.world->globalFog].shader->fogParms.color[ 1 ] * tr.identityLight,
					   tr.world->fogs[tr.world->globalFog].shader->fogParms.color[ 2 ] * tr.identityLight, 1.0 );
	} else if ( skyboxportal )      {
		if ( backEnd.refdef.rdflags & RDF_SKYBOXPORTAL ) { // portal scene, clear whatever is necessary
#endif // RTCW_XX

			clearBits |= GL_DEPTH_BUFFER_BIT;

			if ( r_fastsky->integer || backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) {  // fastsky: clear color

				// try clearing first with the portal sky fog color, then the world fog color, then finally a default
				clearBits |= GL_COLOR_BUFFER_BIT;
				if ( glfogsettings[FOG_PORTALVIEW].registered ) {
					::glClearColor( glfogsettings[FOG_PORTALVIEW].color[0], glfogsettings[FOG_PORTALVIEW].color[1], glfogsettings[FOG_PORTALVIEW].color[2], glfogsettings[FOG_PORTALVIEW].color[3] );
				} else if ( glfogNum > FOG_NONE && glfogsettings[FOG_CURRENT].registered )      {
					::glClearColor( glfogsettings[FOG_CURRENT].color[0], glfogsettings[FOG_CURRENT].color[1], glfogsettings[FOG_CURRENT].color[2], glfogsettings[FOG_CURRENT].color[3] );
				} else {
//					::glClearColor ( 1.0, 0.0, 0.0, 1.0 );	// red clear for testing portal sky clear
					::glClearColor( 0.5, 0.5, 0.5, 1.0 );
				}
			} else {                                                    // rendered sky (either clear color or draw quake sky)
				if ( glfogsettings[FOG_PORTALVIEW].registered ) {
					::glClearColor( glfogsettings[FOG_PORTALVIEW].color[0], glfogsettings[FOG_PORTALVIEW].color[1], glfogsettings[FOG_PORTALVIEW].color[2], glfogsettings[FOG_PORTALVIEW].color[3] );

					if ( glfogsettings[FOG_PORTALVIEW].clearscreen ) {    // portal fog requests a screen clear (distance fog rather than quake sky)
						clearBits |= GL_COLOR_BUFFER_BIT;
					}
				}

			}
		} else {                                        // world scene with portal sky, don't clear any buffers, just set the fog color if there is one

			clearBits |= GL_DEPTH_BUFFER_BIT;   // this will go when I get the portal sky rendering way out in the zbuffer (or not writing to zbuffer at all)

			if ( glfogNum > FOG_NONE && glfogsettings[FOG_CURRENT].registered ) {
				if ( backEnd.refdef.rdflags & RDF_UNDERWATER ) {
					if ( glfogsettings[FOG_CURRENT].mode == GL_LINEAR ) {
						clearBits |= GL_COLOR_BUFFER_BIT;
					}

				} else if ( !( r_portalsky->integer ) ) {    // portal skies have been manually turned off, clear bg color
					clearBits |= GL_COLOR_BUFFER_BIT;
				}

				::glClearColor( glfogsettings[FOG_CURRENT].color[0], glfogsettings[FOG_CURRENT].color[1], glfogsettings[FOG_CURRENT].color[2], glfogsettings[FOG_CURRENT].color[3] );

#if defined RTCW_ET
			} else if ( !( r_portalsky->integer ) ) {      // ydnar: portal skies have been manually turned off, clear bg color
				clearBits |= GL_COLOR_BUFFER_BIT;
				::glClearColor( 0.5, 0.5, 0.5, 1.0 );
#endif // RTCW_XX

			}
		}
	} else {                                              // world scene with no portal sky
		clearBits |= GL_DEPTH_BUFFER_BIT;

		// NERVE - SMF - we don't want to clear the buffer when no world model is specified
		if ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) {
			clearBits &= ~GL_COLOR_BUFFER_BIT;
		}
		// -NERVE - SMF

#if defined RTCW_SP
		// (SA) well, this is silly then
		else if ( r_fastsky->integer ) {   //  || backEnd.refdef.rdflags & RDF_NOWORLDMODEL
#else
		else if ( r_fastsky->integer || backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) {
#endif // RTCW_XX

			clearBits |= GL_COLOR_BUFFER_BIT;

			if ( glfogsettings[FOG_CURRENT].registered ) { // try to clear fastsky with current fog color
				::glClearColor( glfogsettings[FOG_CURRENT].color[0], glfogsettings[FOG_CURRENT].color[1], glfogsettings[FOG_CURRENT].color[2], glfogsettings[FOG_CURRENT].color[3] );
			} else {
//				::glClearColor ( 0.0, 0.0, 1.0, 1.0 );	// blue clear for testing world sky clear
#if defined RTCW_SP
				::glClearColor( 0.5, 0.5, 0.5, 1.0 );
#else
				::glClearColor( 0.05, 0.05, 0.05, 1.0 );  // JPW NERVE changed per id req was 0.5s
#endif
			}
		} else {        // world scene, no portal sky, not fastsky, clear color if fog says to, otherwise, just set the clearcolor
			if ( glfogsettings[FOG_CURRENT].registered ) { // try to clear fastsky with current fog color
				::glClearColor( glfogsettings[FOG_CURRENT].color[0], glfogsettings[FOG_CURRENT].color[1], glfogsettings[FOG_CURRENT].color[2], glfogsettings[FOG_CURRENT].color[3] );

				if ( glfogsettings[FOG_CURRENT].clearscreen ) {   // world fog requests a screen clear (distance fog rather than quake sky)
					clearBits |= GL_COLOR_BUFFER_BIT;
				}
			}
		}
	}


#if defined RTCW_ET
	// ydnar: don't clear the color buffer when no world model is specified
	if ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) {
		clearBits &= ~GL_COLOR_BUFFER_BIT;
	}
#endif // RTCW_XX

	if ( clearBits ) {
		::glClear( clearBits );
	}

//----(SA)	done

	if ( ( backEnd.refdef.rdflags & RDF_HYPERSPACE ) ) {
		RB_Hyperspace();
		return;
	} else
	{
		backEnd.isHyperspace = qfalse;
	}

	glState.faceCulling = -1;       // force face culling to set next time

	// we will only draw a sun if there was sky rendered in this view
	backEnd.skyRenderedThisView = qfalse;

	// clip to the plane of the portal
	if ( backEnd.viewParms.isPortal ) {
		float plane[4];
		double plane2[4];

		plane[0] = backEnd.viewParms.portalPlane.normal[0];
		plane[1] = backEnd.viewParms.portalPlane.normal[1];
		plane[2] = backEnd.viewParms.portalPlane.normal[2];
		plane[3] = backEnd.viewParms.portalPlane.dist;

// BBi
//#if !defined RTCW_ET
//		plane2[0] = DotProduct( backEnd.viewParms.or.axis[0], plane );
//		plane2[1] = DotProduct( backEnd.viewParms.or.axis[1], plane );
//		plane2[2] = DotProduct( backEnd.viewParms.or.axis[2], plane );
//		plane2[3] = DotProduct( plane, backEnd.viewParms.or.origin ) - plane[3];
//#else
//		plane2[0] = DotProduct( backEnd.viewParms.orientation.axis[0], plane );
//		plane2[1] = DotProduct( backEnd.viewParms.orientation.axis[1], plane );
//		plane2[2] = DotProduct( backEnd.viewParms.orientation.axis[2], plane );
//		plane2[3] = DotProduct( plane, backEnd.viewParms.orientation.origin ) - plane[3];
//#endif // RTCW_XX
        plane2[0] = DotProduct (
            ::backEnd.viewParms.orientation.axis[0], plane);
        plane2[1] = DotProduct (
            ::backEnd.viewParms.orientation.axis[1], plane);
        plane2[2] = DotProduct (
            ::backEnd.viewParms.orientation.axis[2], plane);
        plane2[3] = DotProduct (
            plane, ::backEnd.viewParms.orientation.origin) - plane[3];
// BBi

        // BBi
        if (!glConfigEx.is_path_ogl_1_x ()) {
            ogl_model_view_stack.set_current (s_flipMatrix);
            ogl_tess_state.model_view.set (ogl_model_view_stack.get_current ());
            ogl_tess_state.commit_changes ();
        } else {
        // BBi

		::glLoadMatrixf( s_flipMatrix );

        // BBi
        }
        // BBi

		::glClipPlane( GL_CLIP_PLANE0, plane2 );
		::glEnable( GL_CLIP_PLANE0 );
	} else {
		::glDisable( GL_CLIP_PLANE0 );
	}
}

#if defined RTCW_SP
/*
============
RB_ZombieFX

  This is post-tesselation filtering, made especially for the Zombie.
============
*/

extern void GlobalVectorToLocal( const vec3_t in, vec3_t out );
extern vec_t VectorLengthSquared( const vec3_t v );

#define ZOMBIEFX_MAX_VERTS              2048
#define ZOMBIEFX_FADEOUT_TIME_SEC       ( 0.001 * ZOMBIEFX_FADEOUT_TIME )
#define ZOMBIEFX_MAX_HITS               128
#define ZOMBIEFX_MAX_NEWHITS            4
#define ZOMBIEFX_HIT_OKRANGE_SQR        9   // all verts within this range will be hit
#define ZOMBIEFX_HIT_MAXRANGE_SQR       36  // each bullet that strikes the bounding box, will effect verts inside this range (allowing for projections onto the mesh)
#define ZOMBIEFX_PERHIT_TAKEALPHA       150
#define ZOMBIEFX_MAX_HITS_PER_VERT      2

static const char *zombieFxFleshHitSurfaceNames[2] = {"u_body","l_legs"};

// this stores each of the flesh hits for each of the zombies in the game
typedef struct {
	qboolean isHit;
	unsigned short numHits;
	unsigned short vertHits[ZOMBIEFX_MAX_HITS]; // bit flags to represent those verts that have been hit
	int numNewHits;
	vec3_t newHitPos[ZOMBIEFX_MAX_NEWHITS];
	vec3_t newHitDir[ZOMBIEFX_MAX_NEWHITS];
} trZombieFleshHitverts_t;
//
trZombieFleshHitverts_t zombieFleshHitVerts[MAX_SP_CLIENTS][2]; // one for upper, one for lower

void RB_ZombieFXInit( void ) {
	memset( zombieFleshHitVerts, 0, sizeof( zombieFleshHitVerts ) );
}

void RB_ZombieFXAddNewHit( int entityNum, const vec3_t hitPos, const vec3_t hitDir ) {
	int part = 0;

// disabled for E3, are we still going to use this?
	return;

	if ( entityNum == -1 ) {
		// hack, reset data
		RB_ZombieFXInit();
		return;
	}

	if ( entityNum & ( 1 << 30 ) ) {
		part = 1;
		entityNum &= ~( 1 << 30 );
	}

	if ( entityNum >= MAX_SP_CLIENTS ) {
		Com_Printf( "RB_ZombieFXAddNewHit: entityNum (%i) outside allowable range (%i)\n", entityNum, MAX_SP_CLIENTS );
		return;
	}
	if ( zombieFleshHitVerts[entityNum][part].numHits + zombieFleshHitVerts[entityNum][part].numNewHits >= ZOMBIEFX_MAX_HITS ) {
		// already full of hits
		return;
	}
	if ( zombieFleshHitVerts[entityNum][part].numNewHits >= ZOMBIEFX_MAX_NEWHITS ) {
		// just ignore this hit
		return;
	}
	// add it to the list
	VectorCopy( hitPos, zombieFleshHitVerts[entityNum][part].newHitPos[zombieFleshHitVerts[entityNum][part].numNewHits] );
	VectorCopy( hitDir, zombieFleshHitVerts[entityNum][part].newHitDir[zombieFleshHitVerts[entityNum][part].numNewHits] );
	zombieFleshHitVerts[entityNum][part].numNewHits++;
}

void RB_ZombieFXProcessNewHits( trZombieFleshHitverts_t *fleshHitVerts, int oldNumVerts, int numSurfVerts ) {
	float *xyzTrav, *normTrav;
	vec3_t hitPos, hitDir, v, testDir;
	float bestHitDist, thisDist;
	qboolean foundHit;
	int i, j, bestHit;
	unsigned short *hitTrav;
	byte hitCounts[ZOMBIEFX_MAX_VERTS];     // so we can quickly tell if a particular vert has been hit enough times already

// disabled for E3, are we still going to use this?
	return;

	// first build the hitCount list
	memset( hitCounts, 0, sizeof( hitCounts ) );
	for ( i = 0, hitTrav = fleshHitVerts->vertHits; i < fleshHitVerts->numHits; i++, hitTrav++ ) {
		hitCounts[*hitTrav]++;
	}

	// for each new hit
	for ( i = 0; i < fleshHitVerts->numNewHits; i++ ) {
		// calc the local hitPos
		VectorCopy( fleshHitVerts->newHitPos[i], v );
		VectorSubtract( v, backEnd.currentEntity->e.origin, v );
		GlobalVectorToLocal( v, hitPos );
		// calc the local hitDir
		VectorCopy( fleshHitVerts->newHitDir[i], v );
		GlobalVectorToLocal( v, hitDir );

		// look for close matches
		foundHit = qfalse;

		// for each vertex
		for (   j = 0, bestHitDist = -1, xyzTrav = tess.xyz[oldNumVerts].v, normTrav = tess.normal[oldNumVerts].v;
				j < numSurfVerts;
				j++, xyzTrav += 4, normTrav += 4 ) {

			// if this vert has been hit enough times already
			if ( hitCounts[j] > ZOMBIEFX_MAX_HITS_PER_VERT ) {
				continue;
			}
			// if this normal faces the wrong way, reject it
			if ( DotProduct( normTrav, hitDir ) > 0 ) {
				continue;
			}
			// get the diff vector
			VectorSubtract( xyzTrav, hitPos, testDir );
			// check for distance within range
			thisDist = VectorLengthSquared( testDir );
			if ( thisDist < ZOMBIEFX_HIT_OKRANGE_SQR ) {
				goto hitCheckDone;
			}
			thisDist = c::sqrt( thisDist );
			// check for the projection being inside range
			VectorMA( hitPos, thisDist, hitDir, v );
			VectorSubtract( xyzTrav, v, testDir );
			thisDist = VectorLengthSquared( testDir );
			if ( thisDist < ZOMBIEFX_HIT_OKRANGE_SQR ) {
				goto hitCheckDone;
			}
			// if we are still struggling to find a hit, then pick the closest outside the OK range
			if ( !foundHit ) {
				if ( thisDist < ZOMBIEFX_HIT_MAXRANGE_SQR && ( bestHitDist < 0 || thisDist < bestHitDist ) ) {
					bestHitDist = thisDist;
					bestHit = j;
				}
			}

			// if it gets to here, then it failed
			continue;

hitCheckDone:

			// this vertex was hit
			foundHit = qtrue;
			// set the appropriate bit-flag
			fleshHitVerts->isHit = qtrue;
			fleshHitVerts->vertHits[fleshHitVerts->numHits++] = (unsigned short)j;
			//if (fleshHitVerts->numHits == ZOMBIEFX_MAX_HITS)
			//	break;	// only find one close match per shot
			if ( fleshHitVerts->numHits == ZOMBIEFX_MAX_HITS ) {
				break;
			}
		}

		if ( fleshHitVerts->numHits == ZOMBIEFX_MAX_HITS ) {
			break;
		}

		// if we didn't find a hit vertex, grab the closest acceptible match
		if ( !foundHit && bestHitDist >= 0 ) {
			// set the appropriate bit-flag
			fleshHitVerts->isHit = qtrue;
			fleshHitVerts->vertHits[fleshHitVerts->numHits++] = (unsigned short)bestHit;
			if ( fleshHitVerts->numHits == ZOMBIEFX_MAX_HITS ) {
				break;
			}
		}
	}

	// we've processed any new hits
	fleshHitVerts->numNewHits = 0;
}

void RB_ZombieFXShowFleshHits( trZombieFleshHitverts_t *fleshHitVerts, int oldNumVerts, int numSurfVerts ) {
	byte *vertColors;
	unsigned short *vertHits;
	int i;

// disabled for E3, are we still going to use this?
	return;

	vertColors = tess.vertexColors[oldNumVerts].v;
	vertHits = fleshHitVerts->vertHits;

	// for each hit entry, adjust that verts alpha component
	for ( i = 0; i < fleshHitVerts->numHits; i++, vertHits++ ) {
		if ( vertColors[( *vertHits ) * 4 + 3] < ZOMBIEFX_PERHIT_TAKEALPHA ) {
			vertColors[( *vertHits ) * 4 + 3] = 0;
		} else {
			vertColors[( *vertHits ) * 4 + 3] -= ZOMBIEFX_PERHIT_TAKEALPHA;
		}
	}
}

void RB_ZombieFXDecompose( int oldNumVerts, int numSurfVerts, float deltaTimeScale ) {
	byte *vertColors;
	float   *xyz, *norm;
	int i;
	float alpha;

// disabled for E3, are we still going to use this?
	return;

	vertColors = tess.vertexColors[oldNumVerts].v;
	xyz = tess.xyz[oldNumVerts].v;
	norm = tess.normal[oldNumVerts].v;

	for ( i = 0; i < numSurfVerts; i++, vertColors += 4, xyz += 4, norm += 4 ) {
		alpha = 255.0 * ( (float)( 1 + i % 3 ) / 3.0 ) * deltaTimeScale * 2;
		if ( alpha > 255.0 ) {
			alpha = 255.0;
		}
		if ( (float)vertColors[3] - alpha < 0 ) {
			vertColors[3] = 0;
		} else {
			vertColors[3] -= (byte)alpha;
		}

		// skin shrinks with age
		VectorMA( xyz, -2.0 * deltaTimeScale, norm, xyz );
	}
}

void RB_ZombieFXFullAlpha( int oldNumVerts, int numSurfVerts ) {
	byte *vertColors;
	int i;

	vertColors = tess.vertexColors[oldNumVerts].v;

	for ( i = 0; i < numSurfVerts; i++, vertColors += 4 ) {
		vertColors[3] = 255;
	}
}

void RB_ZombieFX( int part, drawSurf_t *drawSurf, int oldNumVerts, int oldNumIndex ) {
	int numSurfVerts;
	float deltaTime;
	char    *surfName;
	trZombieFleshHitverts_t *fleshHitVerts;

	// Central point for Zombie post-tess processing. Various effects can be added from this point

// disabled for E3, are we still going to use this?
	return;

	if ( *drawSurf->surface == SF_MD3 ) {
		surfName = ( (md3Surface_t *)drawSurf->surface )->name;
	} else if ( *drawSurf->surface == SF_MDC ) {
		surfName = ( (mdcSurface_t *)drawSurf->surface )->name;
	} else {
		Com_Printf( "RB_ZombieFX: unknown surface type\n" );
		return;
	}

	// ignore all surfaces starting with u_sk (skeleton)
	if ( !Q_strncmp( surfName, "u_sk", 4 ) ) {
		return;
	}
	// legs
	if ( !Q_strncmp( surfName, "l_sk", 4 ) ) {
		return;
	}
	// head
	if ( !Q_strncmp( surfName, "h_sk", 4 ) ) {
		return;
	}

	numSurfVerts = tess.numVertexes - oldNumVerts;

	if ( numSurfVerts > ZOMBIEFX_MAX_VERTS ) {
		Com_Printf( "RB_ZombieFX: exceeded ZOMBIEFX_MAX_VERTS\n" );
		return;
	}

	deltaTime = backEnd.currentEntity->e.shaderTime;
	if ( ZOMBIEFX_FADEOUT_TIME_SEC < deltaTime ) {
		// nothing to do, it's done fading out
		tess.numVertexes = oldNumVerts;
		tess.numIndexes = oldNumIndex;
		return;
	}

	fleshHitVerts = &zombieFleshHitVerts[backEnd.currentEntity->e.entityNum][part];

	// set everything to full alpha
	RB_ZombieFXFullAlpha( oldNumVerts, numSurfVerts );

	// if this is the chest surface, do flesh hits
	if ( !Q_stricmp( surfName, zombieFxFleshHitSurfaceNames[part] ) ) {

		// check for any new bullet impacts that need to be scanned for triangle collisions
		if ( fleshHitVerts->numNewHits ) {
			RB_ZombieFXProcessNewHits( fleshHitVerts, oldNumVerts, numSurfVerts );
		}

		// hide vertices marked as being torn off
		if ( fleshHitVerts->isHit ) {
			RB_ZombieFXShowFleshHits( fleshHitVerts, oldNumVerts, numSurfVerts );
		}
	}

	// decompose?
	if ( deltaTime ) {
		RB_ZombieFXDecompose( oldNumVerts, numSurfVerts, deltaTime / ZOMBIEFX_FADEOUT_TIME_SEC );
	}

}
#endif // RTCW_XX


#define MAC_EVENT_PUMP_MSEC     5

/*
==================
RB_RenderDrawSurfList
==================
*/
void RB_RenderDrawSurfList( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	shader_t        *shader, *oldShader;
	int fogNum, oldFogNum;
	int entityNum, oldEntityNum;

#if defined RTCW_ET
	int frontFace;
#endif // RTCW_XX

	int dlighted, oldDlighted;
	qboolean depthRange, oldDepthRange;
	int i;
	drawSurf_t      *drawSurf;
	int oldSort;
	float originalTime;

#if defined RTCW_SP
	int oldNumVerts, oldNumIndex;
//GR - tessellation flag
	int atiTess = 0, oldAtiTess;
#endif // RTCW_XX

#ifdef __MACOS__
	int macEventTime;

	Sys_PumpEvents();       // crutch up the mac's limited buffer queue size

	// we don't want to pump the event loop too often and waste time, so
	// we are going to check every shader change
	macEventTime = ri.Milliseconds() + MAC_EVENT_PUMP_MSEC;
#endif

	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;

	// clear the z buffer, set the modelview, etc
	RB_BeginDrawingView();

	// draw everything
	oldEntityNum = -1;
	backEnd.currentEntity = &tr.worldEntity;
	oldShader = NULL;
	oldFogNum = -1;
	oldDepthRange = qfalse;
	oldDlighted = qfalse;
	oldSort = -1;
	depthRange = qfalse;

#if defined RTCW_SP
// GR - tessellation also forces to draw everything
	oldAtiTess = -1;
#endif // RTCW_XX

	backEnd.pc.c_surfaces += numDrawSurfs;

	for ( i = 0, drawSurf = drawSurfs ; i < numDrawSurfs ; i++, drawSurf++ ) {
		if ( drawSurf->sort == oldSort ) {
			// fast path, same as previous sort

#if defined RTCW_SP
			oldNumVerts = tess.numVertexes;
			oldNumIndex = tess.numIndexes;
#endif // RTCW_XX

			rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );

#if defined RTCW_SP
/*
			// RF, convert the newly created vertexes into dust particles, and overwrite
			if (backEnd.currentEntity->e.reFlags & REFLAG_ZOMBIEFX) {
				RB_ZombieFX( 0, drawSurf, oldNumVerts, oldNumIndex );
			}
			else if (backEnd.currentEntity->e.reFlags & REFLAG_ZOMBIEFX2) {
				RB_ZombieFX( 1, drawSurf, oldNumVerts, oldNumIndex );
			}
*/
#endif // RTCW_XX

			continue;
		}
		oldSort = drawSurf->sort;

#if defined RTCW_SP
// GR - also extract tesselation flag
		R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted, &atiTess );
#elif defined RTCW_MP
		R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted );
#else
		R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &frontFace, &dlighted );
#endif // RTCW_XX

		//
		// change the tess parameters if needed
		// a "entityMergable" shader is a shader that can have surfaces from seperate
		// entities merged into a single batch, like smoke and blood puff sprites
		if ( shader != oldShader || fogNum != oldFogNum || dlighted != oldDlighted

#if defined RTCW_SP
// GR - force draw on tessellation flag change
			 || ( atiTess != oldAtiTess )
#endif // RTCW_XX

			 || ( entityNum != oldEntityNum && !shader->entityMergable ) ) {
			if ( oldShader != NULL ) {
#ifdef __MACOS__    // crutch up the mac's limited buffer queue size
				int t;

				t = ri.Milliseconds();
				if ( t > macEventTime ) {
					macEventTime = t + MAC_EVENT_PUMP_MSEC;
					Sys_PumpEvents();
				}
#endif

#if defined RTCW_SP
// GR - pass tessellation flag to the shader command
//		make sure to use oldAtiTess!!!
				tess.ATI_tess = ( oldAtiTess == ATI_TESS_TRUFORM );
#endif // RTCW_XX

				RB_EndSurface();
			}
			RB_BeginSurface( shader, fogNum );
			oldShader = shader;
			oldFogNum = fogNum;
			oldDlighted = dlighted;

#if defined RTCW_SP
// GR - update old tessellation flag
			oldAtiTess = atiTess;
#endif // RTCW_XX

		}

		//
		// change the modelview matrix if needed
		//
		if ( entityNum != oldEntityNum ) {
			depthRange = qfalse;

			if ( entityNum != ENTITYNUM_WORLD ) {
				backEnd.currentEntity = &backEnd.refdef.entities[entityNum];

#if defined RTCW_SP
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
#else
				backEnd.refdef.floatTime = originalTime; // - backEnd.currentEntity->e.shaderTime; // JPW NERVE pulled this to match q3ta
#endif // RTCW_XX

				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
//				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

				// set up the transformation matrix

// BBi
//#if !defined RTCW_ET
//				R_RotateForEntity( backEnd.currentEntity, &backEnd.viewParms, &backEnd.or );
//#else
//				R_RotateForEntity( backEnd.currentEntity, &backEnd.viewParms, &backEnd.orientation );
//#endif // RTCW_XX
                ::R_RotateForEntity (
                    ::backEnd.currentEntity,
                    &::backEnd.viewParms,
                    &::backEnd.orientation);
// BBi

				// set up the dynamic lighting if needed
				if ( backEnd.currentEntity->needDlights ) {

// BBi
//#if !defined RTCW_ET
//					R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
//#else
//					R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.orientation );
//#endif // RTCW_XX
                    ::R_TransformDlights (
                        ::backEnd.refdef.num_dlights,
                        ::backEnd.refdef.dlights,
                        &::backEnd.orientation);
// BBi
				}

				if ( backEnd.currentEntity->e.renderfx & RF_DEPTHHACK ) {
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			} else {
				backEnd.currentEntity = &tr.worldEntity;
				backEnd.refdef.floatTime = originalTime;

// BBi
//#if !defined RTCW_ET
//				backEnd.or = backEnd.viewParms.world;
//#else
//				backEnd.orientation = backEnd.viewParms.world;
//#endif // RTCW_XX
                ::backEnd.orientation = ::backEnd.viewParms.world;
// BBi

				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
//				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

// BBi
//#if !defined RTCW_ET
//				R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
//#else
//				R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.orientation );
//#endif // RTCW_XX
                ::R_TransformDlights (
                    ::backEnd.refdef.num_dlights,
                    ::backEnd.refdef.dlights,
                    &::backEnd.orientation);
// BBi
			}

// BBi
            const float* matrix = ::backEnd.orientation.modelMatrix;
// BBi

// BBi
//#if !defined RTCW_ET
//			::glLoadMatrixf( backEnd.or.modelMatrix );
//#else
//			::glLoadMatrixf( backEnd.orientation.modelMatrix );
//#endif // RTCW_XX
            if (!glConfigEx.is_path_ogl_1_x ()) {
                ogl_model_view_stack.set_current (matrix);

                ogl_tess_state.model_view.set (ogl_model_view_stack.get_current ());
                ogl_tess_state.commit_changes ();
            } else
                ::glLoadMatrixf (matrix);
// BBi

			//
			// change depthrange if needed
			//
			if ( oldDepthRange != depthRange ) {
				if ( depthRange ) {
					::glDepthRange( 0, 0.3 );
				} else {
					::glDepthRange( 0, 1 );
				}
				oldDepthRange = depthRange;
			}

			oldEntityNum = entityNum;
		}

#if defined RTCW_SP
		// RF, ZOMBIEFX, store the tess indexes, so we can grab the calculated
		// vertex positions and normals, and convert them into dust particles
		oldNumVerts = tess.numVertexes;
		oldNumIndex = tess.numIndexes;
#endif // RTCW_XX

		// add the triangles for this surface
		rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );

#if defined RTCW_SP
		// RF, convert the newly created vertexes into dust particles, and overwrite
		if ( backEnd.currentEntity->e.reFlags & REFLAG_ZOMBIEFX ) {
			RB_ZombieFX( 0, drawSurf, oldNumVerts, oldNumIndex );
		} else if ( backEnd.currentEntity->e.reFlags & REFLAG_ZOMBIEFX2 )     {
			RB_ZombieFX( 1, drawSurf, oldNumVerts, oldNumIndex );
		}
#endif // RTCW_XX

	}

	// draw the contents of the last shader batch
	if ( oldShader != NULL ) {

#if defined RTCW_SP
// GR - pass tessellation flag to the shader command
//		make sure to use oldAtiTess!!!
		tess.ATI_tess = ( oldAtiTess == ATI_TESS_TRUFORM );
#endif // RTCW_XX

		RB_EndSurface();
	}

	// go back to the world modelview matrix
	backEnd.currentEntity = &tr.worldEntity;
	backEnd.refdef.floatTime = originalTime;

// BBi
//#if !defined RTCW_ET
//	backEnd.or = backEnd.viewParms.world;
//	R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
//#else
//	backEnd.orientation = backEnd.viewParms.world;
//	R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.orientation );
//#endif // RTCW_XX
    ::backEnd.orientation = ::backEnd.viewParms.world;

    ::R_TransformDlights (
        ::backEnd.refdef.num_dlights,
        ::backEnd.refdef.dlights,
        &::backEnd.orientation);
// BBi

    // BBi
    if (!glConfigEx.is_path_ogl_1_x ()) {
        ogl_model_view_stack.set_current (backEnd.viewParms.world.modelMatrix);
        ogl_tess_state.model_view.set (ogl_model_view_stack.get_current ());
        ogl_tess_state.commit_changes ();
    } else {
    // BBi

	::glLoadMatrixf( backEnd.viewParms.world.modelMatrix );

    // BBi
    }
    // BBi

	if ( depthRange ) {
		::glDepthRange( 0, 1 );
	}

	// (SA) draw sun
	RB_DrawSun();


	// darken down any stencil shadows
	RB_ShadowFinish();

	// add light flares on lights that aren't obscured
	RB_RenderFlares();

#ifdef __MACOS__
	Sys_PumpEvents();       // crutch up the mac's limited buffer queue size
#endif
}


/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/

/*
================
RB_SetGL2D

================
*/
void    RB_SetGL2D( void ) {
	backEnd.projection2D = qtrue;

	// set 2D virtual screen size
	::glViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
	::glScissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );

    // BBi
    if (!glConfigEx.is_path_ogl_1_x ()) {
        ogl_model_view_stack.set_current (glm::mat4 (1.0F));
        ogl_projection_stack.set_current (glm::ortho (
            0.0F, static_cast<float> (glConfig.vidWidth),
            static_cast<float> (glConfig.vidHeight), 0.0F, 0.0F, 1.0F));

        ogl_tess_state.model_view.set (ogl_model_view_stack.get_current ());
        ogl_tess_state.projection.set (ogl_projection_stack.get_current ());
        ogl_tess_state.commit_changes ();
    } else {
    // BBi

	::glMatrixMode( GL_PROJECTION );
	::glLoadIdentity();
	::glOrtho( 0, glConfig.vidWidth, glConfig.vidHeight, 0, 0, 1 );
	::glMatrixMode( GL_MODELVIEW );
	::glLoadIdentity();

    // BBi
    }
    // BBi

	GL_State( GLS_DEPTHTEST_DISABLE |
			  GLS_SRCBLEND_SRC_ALPHA |
			  GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

#if defined RTCW_SP
    // BBi
    if (!glConfigEx.is_path_ogl_1_x ()) {
        ogl_tess_state.use_fog.set (false);
        ogl_tess_state.commit_changes ();
    } else {
    // BBi

	::glDisable( GL_FOG ); //----(SA)	added

    // BBi
    }
    // BBi
#endif // RTCW_XX

	::glDisable( GL_CULL_FACE );
	::glDisable( GL_CLIP_PLANE0 );

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
}


/*
=============
RE_StretchRaw

FIXME: not exactly backend
Stretches a raw 32 bit power of 2 bitmap image over the given screen rectangle.
Used for cinematics.
=============
*/
void RE_StretchRaw( int x, int y, int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty ) {
	int i, j;
	int start, end;

	if ( !tr.registered ) {
		return;
	}
	R_SyncRenderThread();

	// we definately want to sync every frame for the cinematics
	::glFinish();

	start = end = 0;
	if ( r_speeds->integer ) {
		start = ri.Milliseconds();
	}

	// make sure rows and cols are powers of 2
	for ( i = 0 ; ( 1 << i ) < cols ; i++ ) {
	}
	for ( j = 0 ; ( 1 << j ) < rows ; j++ ) {
	}
	if ( ( 1 << i ) != cols || ( 1 << j ) != rows ) {
		ri.Error( ERR_DROP, "Draw_StretchRaw: size not a power of 2: %i by %i", cols, rows );
	}

	GL_Bind( tr.scratchImage[client] );

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		::glTexImage2D( GL_TEXTURE_2D, 0, 3, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

        //BBi
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

        GLenum clamp_mode = ::r_get_best_wrap_clamp ();

        ::glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        ::glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ::glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp_mode);
        ::glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp_mode);
        //BBi
	} else {
		if ( dirty ) {
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			::glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
		}
	}

	if ( r_speeds->integer ) {
		end = ri.Milliseconds();
		ri.Printf( PRINT_ALL, "::glTexSubImage2D %i, %i: %i msec\n", cols, rows, end - start );
	}

	RB_SetGL2D();

    // BBi
    if (!glConfigEx.is_path_ogl_1_x ()) {
        ogl_tess_state.primary_color.set (glm::vec4 (
            tr.identityLight, tr.identityLight, tr.identityLight, 1.0F));
        ogl_tess_state.commit_changes ();
    } else {
    // BBi

	::glColor3f( tr.identityLight, tr.identityLight, tr.identityLight );

    // BBi
    }
    // BBi

    // BBi
    if (!glConfigEx.is_path_ogl_1_x ()) {
        ogl_tess2.texture_coords[0][0] =
            glm::vec2 (0.0F, 0.0F);
        ogl_tess2.position[0] = glm::vec4 (
            static_cast<float> (x), static_cast<float> (y),
            0.0F, 1.0F);

        ogl_tess2.texture_coords[0][1] =
            glm::vec2 (1.0F, 0.0F);
        ogl_tess2.position[1] = glm::vec4 (
            static_cast<float> (x + w), static_cast<float> (y),
            0.0F, 1.0F);

        ogl_tess2.texture_coords[0][2] =
            glm::vec2 (0.0F, 1.0F);
        ogl_tess2.position[2] = glm::vec4 (
            static_cast<float> (x), static_cast<float> (y + h),
            0.0F, 1.0F);

        ogl_tess2.texture_coords[0][3] =
            glm::vec2 (1.0F, 1.0F);
        ogl_tess2.position[3] = glm::vec4 (
            static_cast<float> (x + w), static_cast<float> (y + h),
            0.0F, 1.0F);

        ::ogl_tess2_draw (GL_TRIANGLE_STRIP, 4, true, false);
    } else {
    // BBi

	::glBegin( GL_QUADS );
	::glTexCoord2f( 0.5f / cols,  0.5f / rows );
	::glVertex2f( x, y );
	::glTexCoord2f( ( cols - 0.5f ) / cols,  0.5f / rows );
	::glVertex2f( x + w, y );
	::glTexCoord2f( ( cols - 0.5f ) / cols, ( rows - 0.5f ) / rows );
	::glVertex2f( x + w, y + h );
	::glTexCoord2f( 0.5f / cols, ( rows - 0.5f ) / rows );
	::glVertex2f( x, y + h );
	::glEnd();

    // BBi
    }
    // BBi
}

void RE_UploadCinematic( int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty ) {

	GL_Bind( tr.scratchImage[client] );

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		::glTexImage2D( GL_TEXTURE_2D, 0, 3, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

        //BBi
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

        GLenum clamp_mode = ::r_get_best_wrap_clamp ();

        ::glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        ::glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ::glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp_mode);
        ::glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp_mode);
        //BBi
	} else {
		if ( dirty ) {
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			::glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
		}
	}
}


/*
=============
RB_SetColor

=============
*/
const void  *RB_SetColor( const void *data ) {
	const setColorCommand_t *cmd;

	cmd = (const setColorCommand_t *)data;

	backEnd.color2D[0] = cmd->color[0] * 255;
	backEnd.color2D[1] = cmd->color[1] * 255;
	backEnd.color2D[2] = cmd->color[2] * 255;
	backEnd.color2D[3] = cmd->color[3] * 255;

	return (const void *)( cmd + 1 );
}

/*
=============
RB_StretchPic
=============
*/
const void *RB_StretchPic( const void *data ) {
	const stretchPicCommand_t   *cmd;
	shader_t *shader;
	int numVerts, numIndexes;

	cmd = (const stretchPicCommand_t *)data;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( 4, 6 );
	numVerts = tess.numVertexes;
	numIndexes = tess.numIndexes;

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[ numIndexes ] = numVerts + 3;
	tess.indexes[ numIndexes + 1 ] = numVerts + 0;
	tess.indexes[ numIndexes + 2 ] = numVerts + 2;
	tess.indexes[ numIndexes + 3 ] = numVerts + 2;
	tess.indexes[ numIndexes + 4 ] = numVerts + 0;
	tess.indexes[ numIndexes + 5 ] = numVerts + 1;

	*(int *)tess.vertexColors[ numVerts ].v =
		*(int *)tess.vertexColors[ numVerts + 1 ].v =
			*(int *)tess.vertexColors[ numVerts + 2 ].v =
				*(int *)tess.vertexColors[ numVerts + 3 ].v = *(int *)backEnd.color2D;

	tess.xyz[ numVerts ].v[0] = cmd->x;
	tess.xyz[ numVerts ].v[1] = cmd->y;
	tess.xyz[ numVerts ].v[2] = 0;

	tess.texCoords0[ numVerts ].v[0] = cmd->s1;
	tess.texCoords0[ numVerts ].v[1] = cmd->t1;

	tess.xyz[ numVerts + 1 ].v[0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 1 ].v[1] = cmd->y;
	tess.xyz[ numVerts + 1 ].v[2] = 0;

	tess.texCoords0[ numVerts + 1 ].v[0] = cmd->s2;
	tess.texCoords0[ numVerts + 1 ].v[1] = cmd->t1;

	tess.xyz[ numVerts + 2 ].v[0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 2 ].v[1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 2 ].v[2] = 0;

	tess.texCoords0[ numVerts + 2 ].v[0] = cmd->s2;
	tess.texCoords0[ numVerts + 2 ].v[1] = cmd->t2;

	tess.xyz[ numVerts + 3 ].v[0] = cmd->x;
	tess.xyz[ numVerts + 3 ].v[1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 3 ].v[2] = 0;

	tess.texCoords0[ numVerts + 3 ].v[0] = cmd->s1;
	tess.texCoords0[ numVerts + 3 ].v[1] = cmd->t2;

	return (const void *)( cmd + 1 );
}

#if defined RTCW_ET
const void* RB_Draw2dPolys( const void* data ) {
	const poly2dCommand_t* cmd;
	shader_t *shader;
	int i;

	cmd = (const poly2dCommand_t* )data;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( cmd->numverts, ( cmd->numverts - 2 ) * 3 );

	for ( i = 0; i < cmd->numverts - 2; i++ ) {
		tess.indexes[tess.numIndexes + 0] = tess.numVertexes;
		tess.indexes[tess.numIndexes + 1] = tess.numVertexes + i + 1;
		tess.indexes[tess.numIndexes + 2] = tess.numVertexes + i + 2;
		tess.numIndexes += 3;
	}

	for ( i = 0; i < cmd->numverts; i++ ) {
		tess.xyz[ tess.numVertexes ].v[0] = cmd->verts[i].xyz[0];
		tess.xyz[ tess.numVertexes ].v[1] = cmd->verts[i].xyz[1];
		tess.xyz[ tess.numVertexes ].v[2] = 0;

		tess.texCoords0[ tess.numVertexes ].v[0] = cmd->verts[i].st[0];
		tess.texCoords0[ tess.numVertexes ].v[1] = cmd->verts[i].st[1];

		tess.vertexColors[ tess.numVertexes ].v[0] = cmd->verts[i].modulate[0];
		tess.vertexColors[ tess.numVertexes ].v[1] = cmd->verts[i].modulate[1];
		tess.vertexColors[ tess.numVertexes ].v[2] = cmd->verts[i].modulate[2];
		tess.vertexColors[ tess.numVertexes ].v[3] = cmd->verts[i].modulate[3];
		tess.numVertexes++;
	}

	return (const void *)( cmd + 1 );
}
#endif // RTCW_XX

#if !defined RTCW_SP
// NERVE - SMF
/*
=============
RB_RotatedPic
=============
*/
const void *RB_RotatedPic( const void *data ) {
	const stretchPicCommand_t   *cmd;
	shader_t *shader;
	int numVerts, numIndexes;
	float angle;
	float pi2 = M_PI * 2;

	cmd = (const stretchPicCommand_t *)data;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( 4, 6 );
	numVerts = tess.numVertexes;
	numIndexes = tess.numIndexes;

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[ numIndexes ] = numVerts + 3;
	tess.indexes[ numIndexes + 1 ] = numVerts + 0;
	tess.indexes[ numIndexes + 2 ] = numVerts + 2;
	tess.indexes[ numIndexes + 3 ] = numVerts + 2;
	tess.indexes[ numIndexes + 4 ] = numVerts + 0;
	tess.indexes[ numIndexes + 5 ] = numVerts + 1;

	*(int *)tess.vertexColors[ numVerts ].v =
		*(int *)tess.vertexColors[ numVerts + 1 ].v =
			*(int *)tess.vertexColors[ numVerts + 2 ].v =
				*(int *)tess.vertexColors[ numVerts + 3 ].v = *(int *)backEnd.color2D;

	angle = cmd->angle * pi2;
	tess.xyz[ numVerts ].v[0] = cmd->x + ( c::cos( angle ) * cmd->w );
	tess.xyz[ numVerts ].v[1] = cmd->y + ( c::sin( angle ) * cmd->h );
	tess.xyz[ numVerts ].v[2] = 0;

	tess.texCoords0[ numVerts ].v[0] = cmd->s1;
	tess.texCoords0[ numVerts ].v[1] = cmd->t1;

	angle = cmd->angle * pi2 + 0.25 * pi2;
	tess.xyz[ numVerts + 1 ].v[0] = cmd->x + ( c::cos( angle ) * cmd->w );
	tess.xyz[ numVerts + 1 ].v[1] = cmd->y + ( c::sin( angle ) * cmd->h );
	tess.xyz[ numVerts + 1 ].v[2] = 0;

	tess.texCoords0[ numVerts + 1 ].v[0] = cmd->s2;
	tess.texCoords0[ numVerts + 1 ].v[1] = cmd->t1;

	angle = cmd->angle * pi2 + 0.50 * pi2;
	tess.xyz[ numVerts + 2 ].v[0] = cmd->x + ( c::cos( angle ) * cmd->w );
	tess.xyz[ numVerts + 2 ].v[1] = cmd->y + ( c::sin( angle ) * cmd->h );
	tess.xyz[ numVerts + 2 ].v[2] = 0;

	tess.texCoords0[ numVerts + 2 ].v[0] = cmd->s2;
	tess.texCoords0[ numVerts + 2 ].v[1] = cmd->t2;

	angle = cmd->angle * pi2 + 0.75 * pi2;
	tess.xyz[ numVerts + 3 ].v[0] = cmd->x + ( c::cos( angle ) * cmd->w );
	tess.xyz[ numVerts + 3 ].v[1] = cmd->y + ( c::sin( angle ) * cmd->h );
	tess.xyz[ numVerts + 3 ].v[2] = 0;

	tess.texCoords0[ numVerts + 3 ].v[0] = cmd->s1;
	tess.texCoords0[ numVerts + 3 ].v[1] = cmd->t2;

	return (const void *)( cmd + 1 );
}
// -NERVE - SMF
#endif // RTCW_XX

/*
==============
RB_StretchPicGradient
==============
*/
const void *RB_StretchPicGradient( const void *data ) {
	const stretchPicCommand_t   *cmd;
	shader_t *shader;
	int numVerts, numIndexes;

	cmd = (const stretchPicCommand_t *)data;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( 4, 6 );
	numVerts = tess.numVertexes;
	numIndexes = tess.numIndexes;

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[ numIndexes ] = numVerts + 3;
	tess.indexes[ numIndexes + 1 ] = numVerts + 0;
	tess.indexes[ numIndexes + 2 ] = numVerts + 2;
	tess.indexes[ numIndexes + 3 ] = numVerts + 2;
	tess.indexes[ numIndexes + 4 ] = numVerts + 0;
	tess.indexes[ numIndexes + 5 ] = numVerts + 1;

//	*(int *)tess.vertexColors[ numVerts ].v =
//		*(int *)tess.vertexColors[ numVerts + 1 ].v =
//		*(int *)tess.vertexColors[ numVerts + 2 ].v =
//		*(int *)tess.vertexColors[ numVerts + 3 ].v = *(int *)backEnd.color2D;

	*(int *)tess.vertexColors[ numVerts ].v =
		*(int *)tess.vertexColors[ numVerts + 1 ].v = *(int *)backEnd.color2D;

	*(int *)tess.vertexColors[ numVerts + 2 ].v =
		*(int *)tess.vertexColors[ numVerts + 3 ].v = *(int *)cmd->gradientColor;

	tess.xyz[ numVerts ].v[0] = cmd->x;
	tess.xyz[ numVerts ].v[1] = cmd->y;
	tess.xyz[ numVerts ].v[2] = 0;

	tess.texCoords0[ numVerts ].v[0] = cmd->s1;
	tess.texCoords0[ numVerts ].v[1] = cmd->t1;

	tess.xyz[ numVerts + 1 ].v[0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 1 ].v[1] = cmd->y;
	tess.xyz[ numVerts + 1 ].v[2] = 0;

	tess.texCoords0[ numVerts + 1 ].v[0] = cmd->s2;
	tess.texCoords0[ numVerts + 1 ].v[1] = cmd->t1;

	tess.xyz[ numVerts + 2 ].v[0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 2 ].v[1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 2 ].v[2] = 0;

	tess.texCoords0[ numVerts + 2 ].v[0] = cmd->s2;
	tess.texCoords0[ numVerts + 2 ].v[1] = cmd->t2;

	tess.xyz[ numVerts + 3 ].v[0] = cmd->x;
	tess.xyz[ numVerts + 3 ].v[1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 3 ].v[2] = 0;

	tess.texCoords0[ numVerts + 3 ].v[0] = cmd->s1;
	tess.texCoords0[ numVerts + 3 ].v[1] = cmd->t2;

	return (const void *)( cmd + 1 );
}


/*
=============
RB_DrawSurfs

=============
*/
const void  *RB_DrawSurfs( const void *data ) {
	const drawSurfsCommand_t    *cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	cmd = (const drawSurfsCommand_t *)data;

	backEnd.refdef = cmd->refdef;
	backEnd.viewParms = cmd->viewParms;

	RB_RenderDrawSurfList( cmd->drawSurfs, cmd->numDrawSurfs );

	return (const void *)( cmd + 1 );
}


/*
=============
RB_DrawBuffer

=============
*/
const void  *RB_DrawBuffer( const void *data ) {
	const drawBufferCommand_t   *cmd;

	cmd = (const drawBufferCommand_t *)data;

	::glDrawBuffer( cmd->buffer );

	// clear screen for debugging
	if ( r_clear->integer ) {
		::glClearColor( 1, 0, 0.5, 1 );
		::glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	return (const void *)( cmd + 1 );
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
void RB_ShowImages( void ) {
	int i;
	image_t *image;
	float x, y, w, h;
	int start, end;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	::glClear( GL_COLOR_BUFFER_BIT );

	::glFinish();


	start = ri.Milliseconds();

	for ( i = 0 ; i < tr.numImages ; i++ ) {
		image = tr.images[i];

		w = glConfig.vidWidth / 40;
		h = glConfig.vidHeight / 30;

		x = i % 40 * w;
		y = i / 30 * h;

		// show in proportional size in mode 2
		if ( r_showImages->integer == 2 ) {
			w *= image->uploadWidth / 512.0f;
			h *= image->uploadHeight / 512.0f;
		}

		GL_Bind( image );

        // BBi
        if (!glConfigEx.is_path_ogl_1_x ()) {
            ogl_tess2.texture_coords[0][0] =
                glm::vec2 (0.0F, 0.0F);
            ogl_tess2.position[0] = glm::vec4 (
                static_cast<float> (x), static_cast<float> (y),
                0.0F, 1.0F);

            ogl_tess2.texture_coords[0][1] =
                glm::vec2 (1.0F, 0.0F);
            ogl_tess2.position[1] = glm::vec4 (
                static_cast<float> (x + w), static_cast<float> (y),
                0.0F, 1.0F);

            ogl_tess2.texture_coords[0][2] =
                glm::vec2 (0.0F, 1.0F);
            ogl_tess2.position[2] = glm::vec4 (
                static_cast<float> (x), static_cast<float> (y + h),
                0.0F, 1.0F);

            ogl_tess2.texture_coords[0][3] =
                glm::vec2 (1.0F, 1.0F);
            ogl_tess2.position[3] = glm::vec4 (
                static_cast<float> (x + w), static_cast<float> (y + h),
                0.0F, 1.0F);

            ::ogl_tess2_draw (GL_TRIANGLE_STRIP, 4, true, false);
        } else {
        // BBi

		::glBegin( GL_QUADS );
		::glTexCoord2f( 0, 0 );
		::glVertex2f( x, y );
		::glTexCoord2f( 1, 0 );
		::glVertex2f( x + w, y );
		::glTexCoord2f( 1, 1 );
		::glVertex2f( x + w, y + h );
		::glTexCoord2f( 0, 1 );
		::glVertex2f( x, y + h );
		::glEnd();

        // BBi
        }
        // BBi
	}

	::glFinish();

	end = ri.Milliseconds();
	ri.Printf( PRINT_ALL, "%i msec to draw all images\n", end - start );

}

#if defined RTCW_ET
/*
=============
RB_DrawBounds - ydnar
=============
*/

void RB_DrawBounds( vec3_t mins, vec3_t maxs ) {
	vec3_t center;


	GL_Bind( tr.whiteImage );
	GL_State( GLS_POLYMODE_LINE );

	// box corners

    // BBi
    if (!glConfigEx.is_path_ogl_1_x ()) {
        uint8_t* col;

        //
        col = ogl_tess2.color[0];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[0] = glm::vec4 (mins[0], mins[1], mins[2], 1.0F);

        //
        col = ogl_tess2.color[1];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[1] = glm::vec4 (maxs[0], mins[1], mins[2], 1.0F);

        //
        col = ogl_tess2.color[2];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[2] = glm::vec4 (mins[0], mins[1], mins[2], 1.0F);

        //
        col = ogl_tess2.color[3];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[3] = glm::vec4 (mins[0], maxs[1], mins[2], 1.0F);

        //
        col = ogl_tess2.color[4];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[4] = glm::vec4 (mins[0], mins[1], mins[2], 1.0F);

        //
        col = ogl_tess2.color[5];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[5] = glm::vec4 (mins[0], mins[1], maxs[2], 1.0F);

        // ***

        //
        col = ogl_tess2.color[6];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[6] = glm::vec4 (maxs[0], maxs[1], maxs[2], 1.0F);

        //
        col = ogl_tess2.color[7];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[7] = glm::vec4 (mins[0], maxs[1], maxs[2], 1.0F);

        //
        col = ogl_tess2.color[8];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[8] = glm::vec4 (maxs[0], maxs[1], maxs[2], 1.0F);

        //
        col = ogl_tess2.color[9];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[9] = glm::vec4 (maxs[0], mins[1], maxs[2], 1.0F);

        //
        col = ogl_tess2.color[10];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[10] = glm::vec4 (maxs[0], maxs[1], maxs[2], 1.0F);

        //
        col = ogl_tess2.color[11];
        col[0] = 255;
        col[1] = 255;
        col[2] = 255;
        col[3] = 255;

        ogl_tess2.position[11] = glm::vec4 (maxs[0], maxs[1], mins[2], 1.0F);

        ::ogl_tess2_draw (GL_LINES, 12, false, true);
    } else {
    // BBi

	::glBegin( GL_LINES );
	::glColor3f( 1, 1, 1 );

	::glVertex3f( mins[ 0 ], mins[ 1 ], mins[ 2 ] );
	::glVertex3f( maxs[ 0 ], mins[ 1 ], mins[ 2 ] );
	::glVertex3f( mins[ 0 ], mins[ 1 ], mins[ 2 ] );
	::glVertex3f( mins[ 0 ], maxs[ 1 ], mins[ 2 ] );
	::glVertex3f( mins[ 0 ], mins[ 1 ], mins[ 2 ] );
	::glVertex3f( mins[ 0 ], mins[ 1 ], maxs[ 2 ] );

	::glVertex3f( maxs[ 0 ], maxs[ 1 ], maxs[ 2 ] );
	::glVertex3f( mins[ 0 ], maxs[ 1 ], maxs[ 2 ] );
	::glVertex3f( maxs[ 0 ], maxs[ 1 ], maxs[ 2 ] );
	::glVertex3f( maxs[ 0 ], mins[ 1 ], maxs[ 2 ] );
	::glVertex3f( maxs[ 0 ], maxs[ 1 ], maxs[ 2 ] );
	::glVertex3f( maxs[ 0 ], maxs[ 1 ], mins[ 2 ] );
	::glEnd();

    // BBi
    }
    // BBi

	center[ 0 ] = ( mins[ 0 ] + maxs[ 0 ] ) * 0.5;
	center[ 1 ] = ( mins[ 1 ] + maxs[ 1 ] ) * 0.5;
	center[ 2 ] = ( mins[ 2 ] + maxs[ 2 ] ) * 0.5;

	// center axis

    // BBi
    if (!glConfigEx.is_path_ogl_1_x ()) {
        uint8_t* col;

        //
        col = ogl_tess2.color[0];
        col[0] = 255;
        col[1] = 216;
        col[2] = 0;
        col[3] = 255;

        ogl_tess2.position[0] = glm::vec4 (
            mins[0], center[1], center[2], 1.0F);

        //
        col = ogl_tess2.color[1];
        col[0] = 255;
        col[1] = 216;
        col[2] = 0;
        col[3] = 255;

        ogl_tess2.position[1] = glm::vec4 (
            maxs[0], center[1], center[2], 1.0F);

        //
        col = ogl_tess2.color[2];
        col[0] = 255;
        col[1] = 216;
        col[2] = 0;
        col[3] = 255;

        ogl_tess2.position[2] = glm::vec4 (
            center[0], mins[1], center[2], 1.0F);

        //
        col = ogl_tess2.color[3];
        col[0] = 255;
        col[1] = 216;
        col[2] = 0;
        col[3] = 255;

        ogl_tess2.position[3] = glm::vec4 (
            center[0], maxs[1], center[2], 1.0F);

        //
        col = ogl_tess2.color[4];
        col[0] = 255;
        col[1] = 216;
        col[2] = 0;
        col[3] = 255;

        ogl_tess2.position[4] = glm::vec4 (
            center[0], center[1], mins[2], 1.0F);

        //
        col = ogl_tess2.color[5];
        col[0] = 255;
        col[1] = 216;
        col[2] = 0;
        col[3] = 255;

        ogl_tess2.position[5] = glm::vec4 (
            center[0], center[1], maxs[2], 1.0F);

        ::ogl_tess2_draw (GL_LINES, 6, false, true);
    } else {
    // BBi

	::glBegin( GL_LINES );
	::glColor3f( 1, 0.85, 0 );

	::glVertex3f( mins[ 0 ], center[ 1 ], center[ 2 ] );
	::glVertex3f( maxs[ 0 ], center[ 1 ], center[ 2 ] );
	::glVertex3f( center[ 0 ], mins[ 1 ], center[ 2 ] );
	::glVertex3f( center[ 0 ], maxs[ 1 ], center[ 2 ] );
	::glVertex3f( center[ 0 ], center[ 1 ], mins[ 2 ] );
	::glVertex3f( center[ 0 ], center[ 1 ], maxs[ 2 ] );
	::glEnd();

    // BBi
    }
    // BBi
}
#endif // RTCW_XX


/*
=============
RB_SwapBuffers

=============
*/
const void  *RB_SwapBuffers( const void *data ) {
	const swapBuffersCommand_t  *cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	// texture swapping test
	if ( r_showImages->integer ) {
		RB_ShowImages();
	}

	cmd = (const swapBuffersCommand_t *)data;

	// we measure overdraw by reading back the stencil buffer and
	// counting up the number of increments that have happened
	if ( r_measureOverdraw->integer ) {
		int i;
		long sum = 0;
		unsigned char *stencilReadback;

		stencilReadback = static_cast<byte*> (ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight ));
		::glReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

		for ( i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++ ) {
			sum += stencilReadback[i];
		}

		backEnd.pc.c_overDraw += sum;
		ri.Hunk_FreeTempMemory( stencilReadback );
	}


	if ( !glState.finishCalled ) {
		::glFinish();
	}

	//GLimp_LogComment( "***************** RB_SwapBuffers *****************\n\n\n" );

	GLimp_EndFrame();

	backEnd.projection2D = qfalse;

	return (const void *)( cmd + 1 );
}

#if defined RTCW_ET
//bani
/*
=============
RB_RenderToTexture

=============
*/
const void  *RB_RenderToTexture( const void *data ) {
	const renderToTextureCommand_t  *cmd;

//	ri.Printf( PRINT_ALL, "RB_RenderToTexture\n" );

	cmd = (const renderToTextureCommand_t *)data;

	GL_Bind( cmd->image );
	::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR );
	::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR );
	::glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
	::glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, cmd->x, cmd->y, cmd->w, cmd->h, 0 );
//	::glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cmd->x, cmd->y, cmd->w, cmd->h );

	return (const void *)( cmd + 1 );
}

//bani
/*
=============
RB_Finish

=============
*/
const void  *RB_Finish( const void *data ) {
	const renderFinishCommand_t *cmd;

//	ri.Printf( PRINT_ALL, "RB_Finish\n" );

	cmd = (const renderFinishCommand_t *)data;

	::glFinish();

	return (const void *)( cmd + 1 );
}
#endif // RTCW_XX

/*
====================
RB_ExecuteRenderCommands

This function will be called synchronously if running without
smp extensions, or asynchronously by another thread.
====================
*/
void RB_ExecuteRenderCommands( const void *data ) {
	int t1, t2;

	t1 = ri.Milliseconds();

// BBi
#if 0
	if ( !r_smp->integer || data == backEndData[0]->commands.cmds ) {
		backEnd.smpFrame = 0;
	} else {
		backEnd.smpFrame = 1;
	}
#endif // 0
// BBi

	while ( 1 ) {
		switch ( *(const int *)data ) {
		case RC_SET_COLOR:
			data = RB_SetColor( data );
			break;
		case RC_STRETCH_PIC:
			data = RB_StretchPic( data );
			break;

#if defined RTCW_ET
		case RC_2DPOLYS:
			data = RB_Draw2dPolys( data );
			break;
#endif // RTCW_XX

#if !defined RTCW_SP
		case RC_ROTATED_PIC:
			data = RB_RotatedPic( data );
			break;
#endif // RTCW_XX

		case RC_STRETCH_PIC_GRADIENT:
			data = RB_StretchPicGradient( data );
			break;
		case RC_DRAW_SURFS:
			data = RB_DrawSurfs( data );
			break;
		case RC_DRAW_BUFFER:
			data = RB_DrawBuffer( data );
			break;
		case RC_SWAP_BUFFERS:
			data = RB_SwapBuffers( data );
			break;

#if defined RTCW_ET
			//bani
		case RC_RENDERTOTEXTURE:
			data = RB_RenderToTexture( data );
			break;
			//bani
		case RC_FINISH:
			data = RB_Finish( data );
			break;
#endif // RTCW_XX

		case RC_END_OF_LIST:
		default:
			// stop rendering on this thread
			t2 = ri.Milliseconds();
			backEnd.pc.msec = t2 - t1;
			return;
		}
	}

}

// BBi
#if 0
/*
================
RB_RenderThread
================
*/
void RB_RenderThread( void ) {
	const void  *data;

	// wait for either a rendering command or a quit command
	while ( 1 ) {
		// sleep until we have work to do
		data = GLimp_RendererSleep();

		if ( !data ) {
			return; // all done, renderer is shutting down
		}

		renderThreadActive = qtrue;

		RB_ExecuteRenderCommands( data );

		renderThreadActive = qfalse;
	}
}
#endif // 0
// BBi

