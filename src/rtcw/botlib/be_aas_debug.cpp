/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_debug.c
 *
 * desc:		AAS debug code
 *
 *
 *****************************************************************************/

#include "q_shared.h"
#include "l_memory.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "l_libvar.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_interface.h"
#include "be_aas_funcs.h"
#include "be_aas_def.h"

#define MAX_DEBUGLINES              1024

#if !defined RTCW_ET
#define MAX_DEBUGPOLYGONS           128
#endif // RTCW_XX

int debuglines[MAX_DEBUGLINES];
int debuglinevisible[MAX_DEBUGLINES];
int numdebuglines;

#if !defined RTCW_ET
static int debugpolygons[MAX_DEBUGPOLYGONS];
#else
static bot_debugpoly_t* debugpolygons[MAX_DEBUGPOLYS];
#endif // RTCW_XX

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ClearShownPolygons( void ) {
	int i;

#if !defined RTCW_ET
//*
	for ( i = 0; i < MAX_DEBUGPOLYGONS; i++ )
	{
		if ( debugpolygons[i] ) {
			botimport.DebugPolygonDelete( debugpolygons[i] );
		}
		debugpolygons[i] = 0;
	} //end for
//*/
/*
	for (i = 0; i < MAX_DEBUGPOLYGONS; i++)
	{
		botimport.DebugPolygonDelete(i);
		debugpolygons[i] = 0;
	} //end for
*/
#else
	for ( i = 0; i < MAX_DEBUGPOLYS; i++ ) {
		if ( debugpolygons[i] ) {
			botimport.DebugPolygonDeletePointer( debugpolygons[i] );
		}
		debugpolygons[i] = NULL;
	}
#endif // RTCW_XX

} //end of the function AAS_ClearShownPolygons
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================

#if !defined RTCW_ET
void AAS_ShowPolygon( int color, int numpoints, vec3_t *points ) {
	int i;

	for ( i = 0; i < MAX_DEBUGPOLYGONS; i++ )
	{
		if ( !debugpolygons[i] ) {
			debugpolygons[i] = botimport.DebugPolygonCreate( color, numpoints, points );
			break;
		} //end if
	} //end for
} //end of the function AAS_ShowPolygon
#endif // RTCW_XX

#if defined RTCW_ET
bot_debugpoly_t* AAS_GetDebugPolygon( void ) {
	int i;

	for ( i = 0; i < MAX_DEBUGPOLYS; i++ ) {
		if ( !debugpolygons[i] ) {
			debugpolygons[i] = botimport.DebugPolygonGetFree();

			return debugpolygons[i];
		}
	}

	return NULL;
} //end of the function AAS_GetDebugPolygon
#endif // RTCW_XX

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ClearShownDebugLines( void ) {
	int i;

	//make all lines invisible
	for ( i = 0; i < MAX_DEBUGLINES; i++ )
	{
		if ( debuglines[i] ) {
			//botimport.DebugLineShow(debuglines[i], NULL, NULL, LINECOLOR_NONE);
			botimport.DebugLineDelete( debuglines[i] );
			debuglines[i] = 0;
			debuglinevisible[i] = qfalse;
		} //end if
	} //end for
} //end of the function AAS_ClearShownDebugLines
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DebugLine( vec3_t start, vec3_t end, int color ) {
	int line;

	for ( line = 0; line < MAX_DEBUGLINES; line++ )
	{
		if ( !debuglines[line] ) {
			debuglines[line] = botimport.DebugLineCreate();
			debuglinevisible[line] = qfalse;
			numdebuglines++;
		} //end if
		if ( !debuglinevisible[line] ) {
			botimport.DebugLineShow( debuglines[line], start, end, color );
			debuglinevisible[line] = qtrue;
			return;
		} //end else
	} //end for
} //end of the function AAS_DebugLine
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_PermanentLine( vec3_t start, vec3_t end, int color ) {
	int line;

	line = botimport.DebugLineCreate();
	botimport.DebugLineShow( line, start, end, color );
} //end of the function AAS_PermenentLine
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_DrawPermanentCross( vec3_t origin, float size, int color ) {
	int i, debugline;
	vec3_t start, end;

	for ( i = 0; i < 3; i++ )
	{
		VectorCopy( origin, start );
		start[i] += size;
		VectorCopy( origin, end );
		end[i] -= size;
		AAS_DebugLine( start, end, color );
		debugline = botimport.DebugLineCreate();
		botimport.DebugLineShow( debugline, start, end, color );
	} //end for
} //end of the function AAS_DrawPermanentCross
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DrawPlaneCross( vec3_t point, vec3_t normal, float dist, int type, int color ) {
	int n0, n1, n2, j, line, lines[2];
	vec3_t start1, end1, start2, end2;

	//make a cross in the hit plane at the hit point
	VectorCopy( point, start1 );
	VectorCopy( point, end1 );
	VectorCopy( point, start2 );
	VectorCopy( point, end2 );

	n0 = type % 3;
	n1 = ( type + 1 ) % 3;
	n2 = ( type + 2 ) % 3;
	start1[n1] -= 6;
	start1[n2] -= 6;
	end1[n1] += 6;
	end1[n2] += 6;
	start2[n1] += 6;
	start2[n2] -= 6;
	end2[n1] -= 6;
	end2[n2] += 6;

	start1[n0] = ( dist - ( start1[n1] * normal[n1] +
							start1[n2] * normal[n2] ) ) / normal[n0];
	end1[n0] = ( dist - ( end1[n1] * normal[n1] +
						  end1[n2] * normal[n2] ) ) / normal[n0];
	start2[n0] = ( dist - ( start2[n1] * normal[n1] +
							start2[n2] * normal[n2] ) ) / normal[n0];
	end2[n0] = ( dist - ( end2[n1] * normal[n1] +
						  end2[n2] * normal[n2] ) ) / normal[n0];

	for ( j = 0, line = 0; j < 2 && line < MAX_DEBUGLINES; line++ )
	{
		if ( !debuglines[line] ) {
			debuglines[line] = botimport.DebugLineCreate();
			lines[j++] = debuglines[line];
			debuglinevisible[line] = qtrue;
			numdebuglines++;
		} //end if
		else if ( !debuglinevisible[line] ) {
			lines[j++] = debuglines[line];
			debuglinevisible[line] = qtrue;
		} //end else
	} //end for
	botimport.DebugLineShow( lines[0], start1, end1, color );
	botimport.DebugLineShow( lines[1], start2, end2, color );
} //end of the function AAS_DrawPlaneCross
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowBoundingBox( vec3_t origin, vec3_t mins, vec3_t maxs ) {
	vec3_t bboxcorners[8];
	int lines[3];
	int i, j, line;

	//upper corners
	bboxcorners[0][0] = origin[0] + maxs[0];
	bboxcorners[0][1] = origin[1] + maxs[1];
	bboxcorners[0][2] = origin[2] + maxs[2];
	//
	bboxcorners[1][0] = origin[0] + mins[0];
	bboxcorners[1][1] = origin[1] + maxs[1];
	bboxcorners[1][2] = origin[2] + maxs[2];
	//
	bboxcorners[2][0] = origin[0] + mins[0];
	bboxcorners[2][1] = origin[1] + mins[1];
	bboxcorners[2][2] = origin[2] + maxs[2];
	//
	bboxcorners[3][0] = origin[0] + maxs[0];
	bboxcorners[3][1] = origin[1] + mins[1];
	bboxcorners[3][2] = origin[2] + maxs[2];
	//lower corners
	memcpy( bboxcorners[4], bboxcorners[0], sizeof( vec3_t ) * 4 );
	for ( i = 0; i < 4; i++ ) bboxcorners[4 + i][2] = origin[2] + mins[2];
	//draw bounding box
	for ( i = 0; i < 4; i++ )
	{
		for ( j = 0, line = 0; j < 3 && line < MAX_DEBUGLINES; line++ )
		{
			if ( !debuglines[line] ) {
				debuglines[line] = botimport.DebugLineCreate();
				lines[j++] = debuglines[line];
				debuglinevisible[line] = qtrue;
				numdebuglines++;
			} //end if
			else if ( !debuglinevisible[line] ) {
				lines[j++] = debuglines[line];
				debuglinevisible[line] = qtrue;
			} //end else
		} //end for
		  //top plane
		botimport.DebugLineShow( lines[0], bboxcorners[i],
								 bboxcorners[( i + 1 ) & 3], LINECOLOR_RED );
		//bottom plane
		botimport.DebugLineShow( lines[1], bboxcorners[4 + i],
								 bboxcorners[4 + ( ( i + 1 ) & 3 )], LINECOLOR_RED );
		//vertical lines
		botimport.DebugLineShow( lines[2], bboxcorners[i],
								 bboxcorners[4 + i], LINECOLOR_RED );
	} //end for
} //end of the function AAS_ShowBoundingBox
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowFace( int facenum ) {
	int i, color, edgenum;
	aas_edge_t *edge;
	aas_face_t *face;
	aas_plane_t *plane;
	vec3_t start, end;

	color = LINECOLOR_YELLOW;
	//check if face number is in range
	if ( facenum >= ( *aasworld ).numfaces ) {
		botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
	} //end if
	face = &( *aasworld ).faces[facenum];
	//walk through the edges of the face
	for ( i = 0; i < face->numedges; i++ )
	{
		//edge number
		edgenum = c::abs( ( *aasworld ).edgeindex[face->firstedge + i] );
		//check if edge number is in range
		if ( edgenum >= ( *aasworld ).numedges ) {
			botimport.Print( PRT_ERROR, "edgenum %d out of range\n", edgenum );
		} //end if
		edge = &( *aasworld ).edges[edgenum];
		if ( color == LINECOLOR_RED ) {
			color = LINECOLOR_GREEN;
		} else if ( color == LINECOLOR_GREEN ) {
			color = LINECOLOR_BLUE;
		} else if ( color == LINECOLOR_BLUE )                                                            {
			color = LINECOLOR_YELLOW;
		} else { color = LINECOLOR_RED;}
		AAS_DebugLine( ( *aasworld ).vertexes[edge->v[0]],
					   ( *aasworld ).vertexes[edge->v[1]],
					   color );
	} //end for
	plane = &( *aasworld ).planes[face->planenum];
	edgenum = c::abs( ( *aasworld ).edgeindex[face->firstedge] );
	edge = &( *aasworld ).edges[edgenum];
	VectorCopy( ( *aasworld ).vertexes[edge->v[0]], start );
	VectorMA( start, 20, plane->normal, end );
	AAS_DebugLine( start, end, LINECOLOR_RED );
} //end of the function AAS_ShowFace
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowFacePolygon( int facenum, int color, int flip ) {

#if !defined RTCW_ET
	int i, edgenum, numpoints;
	vec3_t points[128];
	aas_edge_t *edge;
	aas_face_t *face;

	//check if face number is in range
	if ( facenum >= ( *aasworld ).numfaces ) {
		botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
	} //end if
	face = &( *aasworld ).faces[facenum];
	//walk through the edges of the face
	numpoints = 0;
	if ( flip ) {
		for ( i = face->numedges - 1; i >= 0; i-- )
		{
			//edge number
			edgenum = ( *aasworld ).edgeindex[face->firstedge + i];
			edge = &( *aasworld ).edges[c::abs( edgenum )];
			VectorCopy( ( *aasworld ).vertexes[edge->v[edgenum < 0]], points[numpoints] );
			numpoints++;
		} //end for
	} //end if
	else
	{
		for ( i = 0; i < face->numedges; i++ )
		{
			//edge number
			edgenum = ( *aasworld ).edgeindex[face->firstedge + i];
			edge = &( *aasworld ).edges[c::abs( edgenum )];
			VectorCopy( ( *aasworld ).vertexes[edge->v[edgenum < 0]], points[numpoints] );
			numpoints++;
		} //end for
	} //end else
	AAS_ShowPolygon( color, numpoints, points );
#else
	int i, edgenum;
	aas_edge_t *edge;
	aas_face_t *face;

	vec3_t points[128];
	int numpoints = 0;

	//check if face number is in range
	if ( facenum >= aasworld->numfaces ) {
		botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
	}

	//walk through the edges of the face
	face = &( aasworld->faces[facenum] );

	if ( flip ) {
		for ( i = face->numedges - 1; i >= 0; i-- ) {
			edgenum = aasworld->edgeindex[face->firstedge + i];
			edge = &( aasworld->edges[c::abs( edgenum )] );

			VectorCopy( aasworld->vertexes[edge->v[edgenum < 0]], points[numpoints] );

			numpoints++;
		} //end for
	} else {
		for ( i = 0; i < face->numedges; i++ ) {
			edgenum = aasworld->edgeindex[face->firstedge + i];
			edge = &( aasworld->edges[c::abs( edgenum )] );

			VectorCopy( aasworld->vertexes[edge->v[edgenum < 0]], points[numpoints] );

			numpoints++;
		}
	}

	botimport.BotDrawPolygon( color, numpoints, (float*) points );
#endif // RTCW_XX

} //end of the function AAS_ShowFacePolygon
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowArea( int areanum, int groundfacesonly ) {
	int areaedges[MAX_DEBUGLINES];
	int numareaedges, i, j, n, color = 0, line;
	int facenum, edgenum;
	aas_area_t *area;
	aas_face_t *face;
	aas_edge_t *edge;

	//
	numareaedges = 0;
	//
	if ( areanum < 0 || areanum >= ( *aasworld ).numareas ) {
		botimport.Print( PRT_ERROR, "area %d out of range [0, %d]\n",
						 areanum, ( *aasworld ).numareas );
		return;
	} //end if
	  //pointer to the convex area
	area = &( *aasworld ).areas[areanum];
	//walk through the faces of the area
	for ( i = 0; i < area->numfaces; i++ )
	{
		facenum = c::abs( ( *aasworld ).faceindex[area->firstface + i] );
		//check if face number is in range
		if ( facenum >= ( *aasworld ).numfaces ) {
			botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
		} //end if
		face = &( *aasworld ).faces[facenum];
		//ground faces only
		if ( groundfacesonly ) {
			if ( !( face->faceflags & ( FACE_GROUND | FACE_LADDER ) ) ) {
				continue;
			}
		} //end if
		  //walk through the edges of the face
		for ( j = 0; j < face->numedges; j++ )
		{
			//edge number
			edgenum = c::abs( ( *aasworld ).edgeindex[face->firstedge + j] );
			//check if edge number is in range
			if ( edgenum >= ( *aasworld ).numedges ) {
				botimport.Print( PRT_ERROR, "edgenum %d out of range\n", edgenum );
			} //end if
			  //check if the edge is stored already
			for ( n = 0; n < numareaedges; n++ )
			{
				if ( areaedges[n] == edgenum ) {
					break;
				}
			} //end for
			if ( n == numareaedges && numareaedges < MAX_DEBUGLINES ) {
				areaedges[numareaedges++] = edgenum;
			} //end if
		} //end for
		  //AAS_ShowFace(facenum);
	} //end for
	  //draw all the edges
	for ( n = 0; n < numareaedges; n++ )
	{
		for ( line = 0; line < MAX_DEBUGLINES; line++ )
		{
			if ( !debuglines[line] ) {
				debuglines[line] = botimport.DebugLineCreate();
				debuglinevisible[line] = qfalse;
				numdebuglines++;
			} //end if
			if ( !debuglinevisible[line] ) {
				break;
			} //end else
		} //end for
		if ( line >= MAX_DEBUGLINES ) {
			return;
		}
		edge = &( *aasworld ).edges[areaedges[n]];
		if ( color == LINECOLOR_RED ) {
			color = LINECOLOR_BLUE;
		} else if ( color == LINECOLOR_BLUE ) {
			color = LINECOLOR_GREEN;
		} else if ( color == LINECOLOR_GREEN )                                                            {
			color = LINECOLOR_YELLOW;
		} else { color = LINECOLOR_RED;}
		botimport.DebugLineShow( debuglines[line],
								 ( *aasworld ).vertexes[edge->v[0]],
								 ( *aasworld ).vertexes[edge->v[1]],
								 color );
		debuglinevisible[line] = qtrue;
	} //end for*/
} //end of the function AAS_ShowArea
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowAreaPolygons( int areanum, int color, int groundfacesonly ) {
	int i, facenum;
	aas_area_t *area;
	aas_face_t *face;

#if !defined RTCW_ET
	//
	if ( areanum < 0 || areanum >= ( *aasworld ).numareas ) {
		botimport.Print( PRT_ERROR, "area %d out of range [0, %d]\n",
						 areanum, ( *aasworld ).numareas );
		return;
	} //end if
	  //pointer to the convex area
	area = &( *aasworld ).areas[areanum];
	//walk through the faces of the area
	for ( i = 0; i < area->numfaces; i++ )
	{
		facenum = c::abs( ( *aasworld ).faceindex[area->firstface + i] );
		//check if face number is in range
		if ( facenum >= ( *aasworld ).numfaces ) {
			botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
		} //end if
		face = &( *aasworld ).faces[facenum];
		//ground faces only
		if ( groundfacesonly ) {
			if ( !( face->faceflags & ( FACE_GROUND | FACE_LADDER ) ) ) {
				continue;
			}
		} //end if
		AAS_ShowFacePolygon( facenum, color, face->frontarea != areanum );
	} //end for
#else
	if ( areanum < 0 || areanum >= aasworld->numareas ) {
		botimport.Print( PRT_ERROR, "area %d out of range [0, %d]\n", areanum, aasworld->numareas );
		return;
	}

	//pointer to the convex area
	area = &( aasworld->areas[areanum] );

	//walk through the faces of the area
	for ( i = 0; i < area->numfaces; i++ ) {
		facenum = c::abs( aasworld->faceindex[area->firstface + i] );

		//check if face number is in range
		if ( facenum >= aasworld->numfaces ) {
			botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
		}

		face = &( aasworld->faces[facenum] );

		//ground faces only
		if ( groundfacesonly ) {
			if ( !( face->faceflags & ( FACE_GROUND | FACE_LADDER ) ) ) {
				continue;
			}
		}

		AAS_ShowFacePolygon( facenum, color, face->frontarea != areanum );
	}
#endif // RTCW_XX

} //end of the function AAS_ShowAreaPolygons
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DrawCross( vec3_t origin, float size, int color ) {
	int i;
	vec3_t start, end;

	for ( i = 0; i < 3; i++ )
	{
		VectorCopy( origin, start );
		start[i] += size;
		VectorCopy( origin, end );
		end[i] -= size;
		AAS_DebugLine( start, end, color );
	} //end for
} //end of the function AAS_DrawCross
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_PrintTravelType( int traveltype ) {
#ifdef DEBUG
	char *str;
	//
	switch ( traveltype )
	{
	case TRAVEL_INVALID: str = "TRAVEL_INVALID"; break;
	case TRAVEL_WALK: str = "TRAVEL_WALK"; break;
	case TRAVEL_CROUCH: str = "TRAVEL_CROUCH"; break;
	case TRAVEL_BARRIERJUMP: str = "TRAVEL_BARRIERJUMP"; break;
	case TRAVEL_JUMP: str = "TRAVEL_JUMP"; break;
	case TRAVEL_LADDER: str = "TRAVEL_LADDER"; break;
	case TRAVEL_WALKOFFLEDGE: str = "TRAVEL_WALKOFFLEDGE"; break;
	case TRAVEL_SWIM: str = "TRAVEL_SWIM"; break;
	case TRAVEL_WATERJUMP: str = "TRAVEL_WATERJUMP"; break;
	case TRAVEL_TELEPORT: str = "TRAVEL_TELEPORT"; break;
	case TRAVEL_ELEVATOR: str = "TRAVEL_ELEVATOR"; break;
	case TRAVEL_ROCKETJUMP: str = "TRAVEL_ROCKETJUMP"; break;
	case TRAVEL_BFGJUMP: str = "TRAVEL_BFGJUMP"; break;
	case TRAVEL_GRAPPLEHOOK: str = "TRAVEL_GRAPPLEHOOK"; break;
	case TRAVEL_JUMPPAD: str = "TRAVEL_JUMPPAD"; break;
	case TRAVEL_FUNCBOB: str = "TRAVEL_FUNCBOB"; break;
	default: str = "UNKNOWN TRAVEL TYPE"; break;
	} //end switch
	botimport.Print( PRT_MESSAGE, "%s", str );
#endif //DEBUG
} //end of the function AAS_PrintTravelType
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DrawArrow( vec3_t start, vec3_t end, int linecolor, int arrowcolor ) {
	vec3_t dir, cross, p1, p2, up = {0, 0, 1};
	float dot;

	VectorSubtract( end, start, dir );
	VectorNormalize( dir );
	dot = DotProduct( dir, up );
	if ( dot > 0.99 || dot < -0.99 ) {
		VectorSet( cross, 1, 0, 0 );
	} else { CrossProduct( dir, up, cross );}

	VectorMA( end, -6, dir, p1 );
	VectorCopy( p1, p2 );
	VectorMA( p1, 6, cross, p1 );
	VectorMA( p2, -6, cross, p2 );

	AAS_DebugLine( start, end, linecolor );
	AAS_DebugLine( p1, end, arrowcolor );
	AAS_DebugLine( p2, end, arrowcolor );
} //end of the function AAS_DrawArrow
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowReachability( aas_reachability_t *reach ) {
	vec3_t dir, cmdmove, velocity;
	float speed, zvel;
	aas_clientmove_t move;

	AAS_ShowAreaPolygons( reach->areanum, 5, qtrue );
	//AAS_ShowArea(reach->areanum, qtrue);
	AAS_DrawArrow( reach->start, reach->end, LINECOLOR_BLUE, LINECOLOR_YELLOW );
	//
	if ( reach->traveltype == TRAVEL_JUMP || reach->traveltype == TRAVEL_WALKOFFLEDGE ) {
		AAS_HorizontalVelocityForJump( aassettings.sv_jumpvel, reach->start, reach->end, &speed );
		//
		VectorSubtract( reach->end, reach->start, dir );
		dir[2] = 0;
		VectorNormalize( dir );
		//set the velocity
		VectorScale( dir, speed, velocity );
		//set the command movement
		VectorClear( cmdmove );
		cmdmove[2] = aassettings.sv_jumpvel;
		//
		AAS_PredictClientMovement( &move, -1, reach->start, PRESENCE_NORMAL, qtrue,
								   velocity, cmdmove, 3, 30, 0.1,
								   SE_HITGROUND | SE_ENTERWATER | SE_ENTERSLIME |
								   SE_ENTERLAVA | SE_HITGROUNDDAMAGE, 0, qtrue );
		//
		if ( reach->traveltype == TRAVEL_JUMP ) {
			AAS_JumpReachRunStart( reach, dir );
			AAS_DrawCross( dir, 4, LINECOLOR_BLUE );
		} //end if
	} //end if
	else if ( reach->traveltype == TRAVEL_ROCKETJUMP ) {
		zvel = AAS_RocketJumpZVelocity( reach->start );
		AAS_HorizontalVelocityForJump( zvel, reach->start, reach->end, &speed );
		//
		VectorSubtract( reach->end, reach->start, dir );
		dir[2] = 0;
		VectorNormalize( dir );
		//get command movement
		VectorScale( dir, speed, cmdmove );
		VectorSet( velocity, 0, 0, zvel );
		//
		AAS_PredictClientMovement( &move, -1, reach->start, PRESENCE_NORMAL, qtrue,
								   velocity, cmdmove, 30, 30, 0.1,
								   SE_ENTERWATER | SE_ENTERSLIME |
								   SE_ENTERLAVA | SE_HITGROUNDDAMAGE |
								   SE_TOUCHJUMPPAD | SE_HITGROUNDAREA, reach->areanum, qtrue );
	} //end else if
	else if ( reach->traveltype == TRAVEL_JUMPPAD ) {
		VectorSet( cmdmove, 0, 0, 0 );
		//
		VectorSubtract( reach->end, reach->start, dir );
		dir[2] = 0;
		VectorNormalize( dir );
		//set the velocity
		//NOTE: the edgenum is the horizontal velocity
		VectorScale( dir, reach->edgenum, velocity );
		//NOTE: the facenum is the Z velocity
		velocity[2] = reach->facenum;
		//
		AAS_PredictClientMovement( &move, -1, reach->start, PRESENCE_NORMAL, qtrue,
								   velocity, cmdmove, 30, 30, 0.1,
								   SE_ENTERWATER | SE_ENTERSLIME |
								   SE_ENTERLAVA | SE_HITGROUNDDAMAGE |
								   SE_TOUCHJUMPPAD | SE_HITGROUNDAREA, reach->areanum, qtrue );
	} //end else if
} //end of the function AAS_ShowReachability
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowReachableAreas( int areanum ) {
	aas_areasettings_t *settings;
	static aas_reachability_t reach;
	static int index, lastareanum;
	static float lasttime;

	if ( areanum != lastareanum ) {
		index = 0;
		lastareanum = areanum;
	} //end if
	settings = &( *aasworld ).areasettings[areanum];
	//
	if ( !settings->numreachableareas ) {
		return;
	}
	//
	if ( index >= settings->numreachableareas ) {
		index = 0;
	}
	//
	if ( AAS_Time() - lasttime > 1.5 ) {
		memcpy( &reach, &( *aasworld ).reachability[settings->firstreachablearea + index], sizeof( aas_reachability_t ) );
		index++;
		lasttime = AAS_Time();
		AAS_PrintTravelType( reach.traveltype );
		botimport.Print( PRT_MESSAGE, "(traveltime: %i)\n", reach.traveltime );
	} //end if
	AAS_ShowReachability( &reach );
} //end of the function ShowReachableAreas
