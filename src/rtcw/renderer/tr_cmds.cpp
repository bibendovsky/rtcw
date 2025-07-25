/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "tr_local.h"

volatile renderCommandList_t    *renderCommandList;

volatile qboolean renderThreadActive;


/*
=====================
R_PerformanceCounters
=====================
*/
void R_PerformanceCounters( void ) {
	if ( !r_speeds->integer ) {
		// clear the counters even if we aren't printing
		memset( &tr.pc, 0, sizeof( tr.pc ) );
		memset( &backEnd.pc, 0, sizeof( backEnd.pc ) );
		return;
	}

#if !defined RTCW_ET
	if ( r_speeds->integer == 1 ) {
#else
	if ( r_speeds->integer ) { //%	== 1)
#endif // RTCW_XX

		ri.Printf( PRINT_ALL, "%i/%i shaders/surfs %i leafs %i verts %i/%i tris %.2f mtex %.2f dc\n",
				   backEnd.pc.c_shaders, backEnd.pc.c_surfaces, tr.pc.c_leafs, backEnd.pc.c_vertexes,
				   backEnd.pc.c_indexes / 3, backEnd.pc.c_totalIndexes / 3,
				   R_SumOfUsedImages() / ( 1000000.0f ), backEnd.pc.c_overDraw / (float)( glConfig.vidWidth * glConfig.vidHeight ) );

#if !defined RTCW_ET
	} else if ( r_speeds->integer == 2 ) {
#else
	}

	if ( r_speeds->integer == 2 ) {
#endif // RTCW_XX

		ri.Printf( PRINT_ALL, "(patch) %i sin %i sclip  %i sout %i bin %i bclip %i bout\n",
				   tr.pc.c_sphere_cull_patch_in, tr.pc.c_sphere_cull_patch_clip, tr.pc.c_sphere_cull_patch_out,
				   tr.pc.c_box_cull_patch_in, tr.pc.c_box_cull_patch_clip, tr.pc.c_box_cull_patch_out );
		ri.Printf( PRINT_ALL, "(md3) %i sin %i sclip  %i sout %i bin %i bclip %i bout\n",
				   tr.pc.c_sphere_cull_md3_in, tr.pc.c_sphere_cull_md3_clip, tr.pc.c_sphere_cull_md3_out,
				   tr.pc.c_box_cull_md3_in, tr.pc.c_box_cull_md3_clip, tr.pc.c_box_cull_md3_out );

#if defined RTCW_ET
		ri.Printf( PRINT_ALL, "(gen) %i sin %i sout %i pin %i pout\n",
				   tr.pc.c_sphere_cull_in, tr.pc.c_sphere_cull_out,
				   tr.pc.c_plane_cull_in, tr.pc.c_plane_cull_out );
#endif // RTCW_XX

	} else if ( r_speeds->integer == 3 ) {
		ri.Printf( PRINT_ALL, "viewcluster: %i\n", tr.viewCluster );
	} else if ( r_speeds->integer == 4 ) {

#if !defined RTCW_ET
		if ( backEnd.pc.c_dlightVertexes ) {
#endif // RTCW_XX

			ri.Printf( PRINT_ALL, "dlight srf:%i  culled:%i  verts:%i  tris:%i\n",
					   tr.pc.c_dlightSurfaces, tr.pc.c_dlightSurfacesCulled,
					   backEnd.pc.c_dlightVertexes, backEnd.pc.c_dlightIndexes / 3 );

#if !defined RTCW_ET
		}
	}
//----(SA)	this is unnecessary since it will always show 2048.  I moved this to where it is accurate for the world
//	else if (r_speeds->integer == 5 )
//	{
//		ri.Printf( PRINT_ALL, "zFar: %.0f\n", tr.viewParms.zFar );
//	}
	else if ( r_speeds->integer == 6 ) {
#else
	} else if ( r_speeds->integer == 6 )    {
#endif // RTCW_XX

		ri.Printf( PRINT_ALL, "flare adds:%i tests:%i renders:%i\n",
				   backEnd.pc.c_flareAdds, backEnd.pc.c_flareTests, backEnd.pc.c_flareRenders );

#if defined RTCW_ET
	} else if ( r_speeds->integer == 7 )    {
		ri.Printf( PRINT_ALL, "decal projectors: %d test surfs: %d clip surfs: %d decal surfs: %d created: %d\n",
				   tr.pc.c_decalProjectors, tr.pc.c_decalTestSurfaces, tr.pc.c_decalClipSurfaces, tr.pc.c_decalSurfaces, tr.pc.c_decalSurfacesCreated );
#endif // RTCW_XX

	}

	memset( &tr.pc, 0, sizeof( tr.pc ) );
	memset( &backEnd.pc, 0, sizeof( backEnd.pc ) );
}


/*
====================
R_InitCommandBuffers
====================
*/
void R_InitCommandBuffers( void ) {
// BBi
#if 0
	glConfig.smpActive = qfalse;
	if ( r_smp->integer ) {
		ri.Printf( PRINT_ALL, "Trying SMP acceleration...\n" );
		if ( GLimp_SpawnRenderThread( RB_RenderThread ) ) {
			ri.Printf( PRINT_ALL, "...succeeded.\n" );
			glConfig.smpActive = qtrue;
		} else {
			ri.Printf( PRINT_ALL, "...failed.\n" );
		}
	}
#endif // 0
// BBi
}

/*
====================
R_ShutdownCommandBuffers
====================
*/
void R_ShutdownCommandBuffers( void ) {
// BBi
#if 0
	// kill the rendering thread
	if ( glConfig.smpActive ) {
		GLimp_WakeRenderer( NULL );
		glConfig.smpActive = qfalse;
	}
#endif // 0
// BBi
}

/*
====================
R_IssueRenderCommands
====================
*/
int c_blockedOnRender;
int c_blockedOnMain;

void R_IssueRenderCommands( qboolean runPerformanceCounters ) {
	renderCommandList_t *cmdList;

#if defined RTCW_MP
	if ( !tr.registered ) {  //DAJ BUGFIX
		return;
	}
#endif // RTCW_XX

// BBi
//	cmdList = &backEndData[tr.smpFrame]->commands;
	cmdList = &backEndData->commands;
// BBi

	assert( cmdList ); // bk001205
	// add an end-of-list command
	*( int * )( cmdList->cmds + cmdList->used ) = RC_END_OF_LIST;

	// clear it out, in case this is a sync and not a buffer flip
	cmdList->used = 0;

	if ( glConfig.smpActive ) {
		// if the render thread is not idle, wait for it
		if ( renderThreadActive ) {
			c_blockedOnRender++;
			if ( r_showSmp->integer ) {
				ri.Printf( PRINT_ALL, "R" );
			}
		} else {
			c_blockedOnMain++;
			if ( r_showSmp->integer ) {
				ri.Printf( PRINT_ALL, "." );
			}
		}

// BBi
//		// sleep until the renderer has completed
//		GLimp_FrontEndSleep();
// BBi
	}

	// at this point, the back end thread is idle, so it is ok
	// to look at it's performance counters
	if ( runPerformanceCounters ) {
		R_PerformanceCounters();
	}

	// actually start the commands going
	if ( !r_skipBackEnd->integer ) {
		// let it start on the new batch
// BBi
//		if ( !glConfig.smpActive ) {
//			RB_ExecuteRenderCommands( cmdList->cmds );
//		} else {
//			GLimp_WakeRenderer( cmdList );
//		}

		RB_ExecuteRenderCommands (cmdList->cmds);
// BBi
	}
}


/*
====================
R_SyncRenderThread

Issue any pending commands and wait for them to complete.
After exiting, the render thread will have completed its work
and will remain idle and the main thread is free to issue
OpenGL calls until R_IssueRenderCommands is called.
====================
*/
void R_SyncRenderThread( void ) {
	if ( !tr.registered ) {
		return;
	}
	R_IssueRenderCommands( qfalse );

// BBi
#if 0
	if ( !glConfig.smpActive ) {
		return;
	}
	GLimp_FrontEndSleep();
#endif // 0
// BBi
}

/*
============
R_GetCommandBuffer

make sure there is enough command space, waiting on the
render thread if needed.
============
*/
void *R_GetCommandBuffer( int bytes ) {
	renderCommandList_t *cmdList;

// BBi See #BUG0001
//#if defined RTCW_MP
//	if ( !tr.registered ) {  //DAJ BUGFIX
//		return NULL;
//	}
//#endif // RTCW_XX

	if (!tr.registered) //DAJ BUGFIX
		return 0;
// BBi

// BBi
//	cmdList = &backEndData[tr.smpFrame]->commands;
	cmdList = &backEndData->commands;
// BBi

#if !defined RTCW_ET
	// always leave room for the end of list command
	if ( cmdList->used + bytes + 4 > MAX_RENDER_COMMANDS ) {
		if ( bytes > MAX_RENDER_COMMANDS - 4 ) {
#else
	// always leave room for the swap buffers and end of list commands
	if ( cmdList->used + bytes + ( sizeof( swapBuffersCommand_t ) + sizeof( int ) ) > MAX_RENDER_COMMANDS ) {
		if ( bytes > MAX_RENDER_COMMANDS - ( sizeof( swapBuffersCommand_t ) + sizeof( int ) ) ) {
#endif // RTCW_XX

			ri.Error( ERR_FATAL, "R_GetCommandBuffer: bad size %i", bytes );
		}
		// if we run out of room, just start dropping commands
		return NULL;
	}

	cmdList->used += bytes;

	return cmdList->cmds + cmdList->used - bytes;
}


/*
=============
R_AddDrawSurfCmd

=============
*/
void    R_AddDrawSurfCmd( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	drawSurfsCommand_t  *cmd;

	cmd = static_cast<drawSurfsCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_DRAW_SURFS;

	cmd->drawSurfs = drawSurfs;
	cmd->numDrawSurfs = numDrawSurfs;

	cmd->refdef = tr.refdef;
	cmd->viewParms = tr.viewParms;
}


/*
=============
RE_SetColor

Passing NULL will set the color to white
=============
*/
void    RE_SetColor( const float *rgba ) {
	setColorCommand_t   *cmd;

	cmd = static_cast<setColorCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SET_COLOR;
	if ( !rgba ) {
		static float colorWhite[4] = { 1, 1, 1, 1 };

		rgba = colorWhite;
	}

	cmd->color[0] = rgba[0];
	cmd->color[1] = rgba[1];
	cmd->color[2] = rgba[2];
	cmd->color[3] = rgba[3];
}


/*
=============
RE_StretchPic
=============
*/
void RE_StretchPic( float x, float y, float w, float h,
					float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	stretchPicCommand_t *cmd;

	cmd = static_cast<stretchPicCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_STRETCH_PIC;
	cmd->shader = R_GetShaderByHandle( hShader );
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
	cmd->s1 = s1;
	cmd->t1 = t1;
	cmd->s2 = s2;
	cmd->t2 = t2;
}

#if defined RTCW_ET
/*
=============
RE_2DPolyies
=============
*/
extern int r_numpolyverts;

void RE_2DPolyies( polyVert_t* verts, int numverts, qhandle_t hShader ) {
	poly2dCommand_t* cmd;

	if ( r_numpolyverts + numverts > max_polyverts ) {
		return;
	}

	cmd = static_cast<poly2dCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}

	cmd->commandId =    RC_2DPOLYS;

#if 0
	cmd->verts =        &backEndData[tr.smpFrame]->polyVerts[r_numpolyverts];
#endif // 0
	cmd->verts = &backEndData->polyVerts[r_numpolyverts];

	cmd->numverts =     numverts;
	memcpy( cmd->verts, verts, sizeof( polyVert_t ) * numverts );
	cmd->shader =       R_GetShaderByHandle( hShader );

	r_numpolyverts += numverts;
}
#endif // RTCW_XX

#if !defined RTCW_SP
/*
=============
RE_RotatedPic
=============
*/
void RE_RotatedPic( float x, float y, float w, float h,
					float s1, float t1, float s2, float t2, qhandle_t hShader, float angle ) {
	stretchPicCommand_t *cmd;

	cmd = static_cast<stretchPicCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_ROTATED_PIC;
	cmd->shader = R_GetShaderByHandle( hShader );
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;

	// fixup
	cmd->w /= 2;
	cmd->h /= 2;
	cmd->x += cmd->w;
	cmd->y += cmd->h;
	cmd->w = c::sqrt( ( cmd->w * cmd->w ) + ( cmd->h * cmd->h ) );
	cmd->h = cmd->w;

	cmd->angle = angle;
	cmd->s1 = s1;
	cmd->t1 = t1;
	cmd->s2 = s2;
	cmd->t2 = t2;
}
#endif // RTCW_XX

//----(SA)	added
/*
==============
RE_StretchPicGradient
==============
*/
void RE_StretchPicGradient( float x, float y, float w, float h,
							float s1, float t1, float s2, float t2, qhandle_t hShader, const float *gradientColor, int gradientType ) {
	stretchPicCommand_t *cmd;

	cmd = static_cast<stretchPicCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_STRETCH_PIC_GRADIENT;
	cmd->shader = R_GetShaderByHandle( hShader );
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
	cmd->s1 = s1;
	cmd->t1 = t1;
	cmd->s2 = s2;
	cmd->t2 = t2;

	if ( !gradientColor ) {
		static float colorWhite[4] = { 1, 1, 1, 1 };

		gradientColor = colorWhite;
	}

	cmd->gradientColor[0] = gradientColor[0] * 255;
	cmd->gradientColor[1] = gradientColor[1] * 255;
	cmd->gradientColor[2] = gradientColor[2] * 255;
	cmd->gradientColor[3] = gradientColor[3] * 255;
	cmd->gradientType = gradientType;
}
//----(SA)	end

#if defined RTCW_ET
/*
====================
RE_SetGlobalFog
	rgb = colour
	depthForOpaque is depth for opaque

	the restore flag can be used to restore the original level fog
	duration can be set to fade over a certain period
====================
*/
void RE_SetGlobalFog( qboolean restore, int duration, float r, float g, float b, float depthForOpaque ) {
	if ( restore ) {
		if ( duration > 0 ) {
			VectorCopy( tr.world->fogs[tr.world->globalFog].shader->fogParms.color, tr.world->globalTransStartFog );
			tr.world->globalTransStartFog[ 3 ] = tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque;

			Vector4Copy( tr.world->globalOriginalFog, tr.world->globalTransEndFog );

			tr.world->globalFogTransStartTime = tr.refdef.time;
			tr.world->globalFogTransEndTime = tr.refdef.time + duration;
		} else {
			VectorCopy( tr.world->globalOriginalFog, tr.world->fogs[tr.world->globalFog].shader->fogParms.color );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.colorInt = ColorBytes4( tr.world->globalOriginalFog[ 0 ] * tr.identityLight,
																						 tr.world->globalOriginalFog[ 1 ] * tr.identityLight,
																						 tr.world->globalOriginalFog[ 2 ] * tr.identityLight, 1.0 );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque = tr.world->globalOriginalFog[ 3 ];
			tr.world->fogs[tr.world->globalFog].shader->fogParms.tcScale = 1.0f / ( tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque );
		}
	} else {
		if ( duration > 0 ) {
			VectorCopy( tr.world->fogs[tr.world->globalFog].shader->fogParms.color, tr.world->globalTransStartFog );
			tr.world->globalTransStartFog[ 3 ] = tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque;

			VectorSet( tr.world->globalTransEndFog, r, g, b );
			tr.world->globalTransEndFog[ 3 ] = depthForOpaque < 1 ? 1 : depthForOpaque;

			tr.world->globalFogTransStartTime = tr.refdef.time;
			tr.world->globalFogTransEndTime = tr.refdef.time + duration;
		} else {
			VectorSet( tr.world->fogs[tr.world->globalFog].shader->fogParms.color, r, g, b );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.colorInt = ColorBytes4( r * tr.identityLight,
																						 g * tr.identityLight,
																						 b * tr.identityLight, 1.0 );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque = depthForOpaque < 1 ? 1 : depthForOpaque;
			tr.world->fogs[tr.world->globalFog].shader->fogParms.tcScale = 1.0f / ( tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque );
		}
	}
}
#endif // RTCW_XX


/*
====================
RE_BeginFrame

If running in stereo, RE_BeginFrame will be called twice
for each RE_EndFrame
====================
*/
void RE_BeginFrame( stereoFrame_t stereoFrame ) {
	drawBufferCommand_t *cmd;

	if ( !tr.registered ) {
		return;
	}
	glState.finishCalled = qfalse;

	tr.frameCount++;
	tr.frameSceneNum = 0;

	//
	// do overdraw measurement
	//
	if ( r_measureOverdraw->integer ) {
		if ( glConfig.stencilBits < 4 ) {
			ri.Printf( PRINT_ALL, "Warning: not enough stencil bits to measure overdraw: %d\n", glConfig.stencilBits );
			ri.Cvar_Set( "r_measureOverdraw", "0" );
			r_measureOverdraw->modified = qfalse;
		} else if ( r_shadows->integer == 2 )   {
			ri.Printf( PRINT_ALL, "Warning: stencil shadows and overdraw measurement are mutually exclusive\n" );
			ri.Cvar_Set( "r_measureOverdraw", "0" );
			r_measureOverdraw->modified = qfalse;
		} else
		{
			R_SyncRenderThread();
			glEnable( GL_STENCIL_TEST );
			glStencilMask( ~0U );
			glClearStencil( 0U );
			glStencilFunc( GL_ALWAYS, 0U, ~0U );
			glStencilOp( GL_KEEP, GL_INCR, GL_INCR );
		}
		r_measureOverdraw->modified = qfalse;
	} else
	{
		// this is only reached if it was on and is now off
		if ( r_measureOverdraw->modified ) {
			R_SyncRenderThread();
			glDisable( GL_STENCIL_TEST );
		}
		r_measureOverdraw->modified = qfalse;
	}

	//
	// texturemode stuff
	//
	if ( r_textureMode->modified ) {
		R_SyncRenderThread();
		GL_TextureMode( r_textureMode->string );
		r_textureMode->modified = qfalse;
	}

#ifdef RTCW_SP
	//
	// ATI stuff
	//

	// TRUFORM
	if ( glPNTrianglesiATI ) {

		// tess
		if ( r_ati_truform_tess->modified ) {
			r_ati_truform_tess->modified = qfalse;
			// cap if necessary
			if ( r_ati_truform_tess->value > glConfig.ATIMaxTruformTess ) {
				ri.Cvar_Set( "r_ati_truform_tess", va( "%d",glConfig.ATIMaxTruformTess ) );
			}

			glPNTrianglesiATI( GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI, r_ati_truform_tess->value );
		}

		// point mode
		if ( r_ati_truform_pointmode->modified ) {
			r_ati_truform_pointmode->modified = qfalse;
			// GR - shorten the mode name
			if ( Q_stricmp( r_ati_truform_pointmode->string, "LINEAR" ) == 0 ) {
				glConfig.ATIPointMode = GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI;
				// GR - fix point mode change
			} else if ( Q_stricmp( r_ati_truform_pointmode->string, "CUBIC" ) == 0 ) {
				glConfig.ATIPointMode = GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI;
			} else {
				// bogus value, set to valid
				glConfig.ATIPointMode = GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI;
				ri.Cvar_Set( "r_ati_truform_pointmode", "LINEAR" );
			}
			glPNTrianglesiATI( GL_PN_TRIANGLES_POINT_MODE_ATI, glConfig.ATIPointMode );
		}

		// normal mode
		if ( r_ati_truform_normalmode->modified ) {
			r_ati_truform_normalmode->modified = qfalse;
			// GR - shorten the mode name
			if ( !Q_stricmp( r_ati_truform_normalmode->string, "LINEAR" ) ) {
				glConfig.ATINormalMode = GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI;
				// GR - fix normal mode change
			} else if ( !Q_stricmp( r_ati_truform_normalmode->string, "QUADRATIC" ) ) {
				glConfig.ATINormalMode = GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI;
			} else {
				// bogus value, set to valid
				glConfig.ATINormalMode = GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI;
				ri.Cvar_Set( "r_ati_truform_normalmode", "LINEAR" );
			}
			glPNTrianglesiATI( GL_PN_TRIANGLES_NORMAL_MODE_ATI, glConfig.ATINormalMode );
		}
	}
#endif // RTCW_SP

// BBi
//#if defined RTCW_ET
// BBi

	//
	// anisotropic filtering stuff
	//
	if ( r_textureAnisotropy->modified ) {
		R_SyncRenderThread();
		GL_TextureAnisotropy( r_textureAnisotropy->value );
		r_textureAnisotropy->modified = qfalse;
	}

// BBi
//#endif // RTCW_XX
// BBi

	//
	// NVidia stuff
	//

	// fog control
	if ( glConfig.NVFogAvailable && r_nv_fogdist_mode->modified ) {
		r_nv_fogdist_mode->modified = qfalse;
		if ( !Q_stricmp( r_nv_fogdist_mode->string, "GL_EYE_PLANE_ABSOLUTE_NV" ) ) {
			glConfig.NVFogMode = (int)GL_EYE_PLANE_ABSOLUTE_NV;
		} else if ( !Q_stricmp( r_nv_fogdist_mode->string, "GL_EYE_PLANE" ) ) {
			glConfig.NVFogMode = (int)GL_EYE_PLANE;
		} else if ( !Q_stricmp( r_nv_fogdist_mode->string, "GL_EYE_RADIAL_NV" ) ) {
			glConfig.NVFogMode = (int)GL_EYE_RADIAL_NV;
		} else {
			// in case this was really 'else', store a valid value for next time
			glConfig.NVFogMode = (int)GL_EYE_RADIAL_NV;
			ri.Cvar_Set( "r_nv_fogdist_mode", "GL_EYE_RADIAL_NV" );
		}
	}

	//
	// gamma stuff
	//
	if ( r_gamma->modified ) {
		r_gamma->modified = qfalse;

		R_SyncRenderThread();
		R_SetColorMappings();
	}

	// check for errors
	if ( !r_ignoreGLErrors->integer ) {
		int err;

		R_SyncRenderThread();
		if ( ( err = glGetError() ) != GL_NO_ERROR ) {
			ri.Error( ERR_FATAL, "RE_BeginFrame() - glGetError() failed (0x%x)!\n", err );
		}
	}

	//
	// draw buffer stuff
	//
	cmd = static_cast<drawBufferCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_DRAW_BUFFER;

	if ( glConfig.stereoEnabled ) {
		if ( stereoFrame == STEREO_LEFT ) {
			cmd->buffer = (int)GL_BACK_LEFT;
		} else if ( stereoFrame == STEREO_RIGHT ) {
			cmd->buffer = (int)GL_BACK_RIGHT;
		} else {
			ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is enabled, but stereoFrame was %i", stereoFrame );
		}
	} else {
		if ( stereoFrame != STEREO_CENTER ) {
			ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is disabled, but stereoFrame was %i", stereoFrame );
		}
		if ( !Q_stricmp( r_drawBuffer->string, "GL_FRONT" ) ) {
			cmd->buffer = (int)GL_FRONT;
		} else {
			cmd->buffer = (int)GL_BACK;
		}
	}
}


/*
=============
RE_EndFrame

Returns the number of msec spent in the back end
=============
*/
void RE_EndFrame( int *frontEndMsec, int *backEndMsec ) {

#if !defined RTCW_ET
	swapBuffersCommand_t    *cmd;
#else
	renderCommandList_t *cmdList;
#endif // RTCW_XX

	if ( !tr.registered ) {
		return;
	}

#if !defined RTCW_ET
	cmd = static_cast<swapBuffersCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SWAP_BUFFERS;
#else
	// Needs to use reserved space, so no R_GetCommandBuffer.

#if 0
	cmdList = &backEndData[tr.smpFrame]->commands;
#endif // 0
	cmdList = &backEndData->commands;

	assert( cmdList );
	// add swap-buffers command
	*( int * )( cmdList->cmds + cmdList->used ) = RC_SWAP_BUFFERS;
	cmdList->used += sizeof( swapBuffersCommand_t );
#endif // RTCW_XX

	R_IssueRenderCommands( qtrue );

	// use the other buffers next frame, because another CPU
	// may still be rendering into the current ones
	R_ToggleSmpFrame();

	if ( frontEndMsec ) {
		*frontEndMsec = tr.frontEndMsec;
	}
	tr.frontEndMsec = 0;
	if ( backEndMsec ) {
		*backEndMsec = backEnd.pc.msec;
	}
	backEnd.pc.msec = 0;
}

#if defined RTCW_ET
//bani
/*
==================
RE_RenderToTexture
==================
*/
void RE_RenderToTexture( int textureid, int x, int y, int w, int h ) {
	renderToTextureCommand_t    *cmd;

//	ri.Printf( PRINT_ALL, "RE_RenderToTexture\n" );

	if ( textureid > tr.numImages || textureid < 0 ) {
		ri.Printf( PRINT_ALL, "Warning: trap_R_RenderToTexture textureid %d out of range.\n", textureid );
		return;
	}

	cmd = static_cast<renderToTextureCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_RENDERTOTEXTURE;
	cmd->image = tr.images[textureid];
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
}

//bani
/*
==================
RE_Finish
==================
*/
void RE_Finish( void ) {
	renderFinishCommand_t   *cmd;

	ri.Printf( PRINT_ALL, "RE_Finish\n" );

	cmd = static_cast<renderFinishCommand_t*> (R_GetCommandBuffer( sizeof( *cmd ) ));
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_FINISH;
}
#endif // RTCW_XX

