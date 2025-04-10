/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_sample.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

#ifdef AASINTERN
void AAS_InitAASLinkHeap( void );
void AAS_InitAASLinkedEntities( void );
void AAS_FreeAASLinkHeap( void );
void AAS_FreeAASLinkedEntities( void );
aas_face_t *AAS_AreaGroundFace( int areanum, vec3_t point );
aas_face_t *AAS_TraceEndFace( aas_trace_t *trace );
aas_plane_t *AAS_PlaneFromNum( int planenum );
aas_link_t *AAS_AASLinkEntity( vec3_t absmins, vec3_t absmaxs, int entnum );
aas_link_t *AAS_LinkEntityClientBBox( vec3_t absmins, vec3_t absmaxs, int entnum, int presencetype );
qboolean AAS_PointInsideFace( int facenum, vec3_t point, float epsilon );
qboolean AAS_InsideFace( aas_face_t *face, vec3_t pnormal, vec3_t point, float epsilon );
void AAS_UnlinkFromAreas( aas_link_t *areas );
#endif //AASINTERN

//returns the mins and maxs of the bounding box for the given presence type
void AAS_PresenceTypeBoundingBox( int presencetype, vec3_t mins, vec3_t maxs );
//returns the cluster the area is in (negative portal number if the area is a portal)
int AAS_AreaCluster( int areanum );
//returns the presence type(s) of the area
int AAS_AreaPresenceType( int areanum );
//returns the presence type(s) at the given point
int AAS_PointPresenceType( vec3_t point );
//returns the result of the trace of a client bbox
aas_trace_t AAS_TraceClientBBox( vec3_t start, vec3_t end, int presencetype, int passent );
//stores the areas the trace went through and returns the number of passed areas
int AAS_TraceAreas( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas );
//returns the area the point is in
int AAS_PointAreaNum( vec3_t point );
//returns the plane the given face is in
void AAS_FacePlane( int facenum, vec3_t normal, float *dist );

int AAS_BBoxAreas( vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas );

#if defined RTCW_ET
void AAS_AreaCenter( int areanum, vec3_t center );
qboolean AAS_AreaWaypoint( int areanum, vec3_t center );
#endif // RTCW_XX

