/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_file.c
 *
 * desc:		AAS file loading/writing
 *
 *
 *****************************************************************************/

#include "q_shared.h"
#include "l_memory.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "l_libvar.h"
#include "l_utils.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_interface.h"
#include "be_aas_def.h"
#include "rtcw_endian.h"

//#define AASFILEDEBUG

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_SwapAASData( void ) {
	// Ridah, no need to do anything if this OS doesn't need byte swapping
	if (rtcw::Endian::is_little ())
		return;
	// done.

	//bounding boxes
	for (int i = 0; i < aasworld->numbboxes; ++i) {
		rtcw::Endian::lei(aasworld->bboxes[i].presencetype);
		rtcw::Endian::lei(aasworld->bboxes[i].flags);

		rtcw::Endian::lei(aasworld->bboxes[i].mins);
		rtcw::Endian::lei(aasworld->bboxes[i].maxs);
	}

	//vertexes
	rtcw::Endian::lei(aasworld->vertexes, aasworld->numvertexes);

	//planes
	for (int i = 0; i < aasworld->numplanes; ++i) {
		rtcw::Endian::lei(aasworld->planes[i].normal);
		rtcw::Endian::lei(aasworld->planes[i].dist);
		rtcw::Endian::lei(aasworld->planes[i].type);
	}

	//edges
	for (int i = 0; i < aasworld->numedges; ++i)
		rtcw::Endian::lei(aasworld->edges[i].v);

	//edgeindex
	rtcw::Endian::lei(aasworld->edgeindex, aasworld->edgeindexsize);

	//faces
	for (int i = 0; i < aasworld->numfaces; ++i) {
		rtcw::Endian::lei(aasworld->faces[i].planenum);
		rtcw::Endian::lei(aasworld->faces[i].faceflags);
		rtcw::Endian::lei(aasworld->faces[i].numedges);
		rtcw::Endian::lei(aasworld->faces[i].firstedge);
		rtcw::Endian::lei(aasworld->faces[i].frontarea);
		rtcw::Endian::lei(aasworld->faces[i].backarea);
	}

	//face index
	rtcw::Endian::lei(aasworld->faceindex, aasworld->faceindexsize);

	//convex areas
	for (int i = 0; i < aasworld->numareas; ++i) {
		rtcw::Endian::lei(aasworld->areas[i].areanum);
		rtcw::Endian::lei(aasworld->areas[i].numfaces);
		rtcw::Endian::lei(aasworld->areas[i].firstface);
		rtcw::Endian::lei(aasworld->areas[i].mins);
		rtcw::Endian::lei(aasworld->areas[i].maxs);
		rtcw::Endian::lei(aasworld->areas[i].center);
	}

	//area settings
	for (int i = 0; i < aasworld->numareasettings; ++i) {
		rtcw::Endian::lei(aasworld->areasettings[i].contents);
		rtcw::Endian::lei(aasworld->areasettings[i].areaflags);
		rtcw::Endian::lei(aasworld->areasettings[i].presencetype);
		rtcw::Endian::lei(aasworld->areasettings[i].cluster);
		rtcw::Endian::lei(aasworld->areasettings[i].clusterareanum);
		rtcw::Endian::lei(aasworld->areasettings[i].numreachableareas);
		rtcw::Endian::lei(aasworld->areasettings[i].firstreachablearea);
		rtcw::Endian::lei(aasworld->areasettings[i].groundsteepness);
	}

	//area reachability
	for (int i = 0; i < aasworld->reachabilitysize; ++i) {
		rtcw::Endian::lei(aasworld->reachability[i].areanum);
		rtcw::Endian::lei(aasworld->reachability[i].facenum);
		rtcw::Endian::lei(aasworld->reachability[i].edgenum);
		rtcw::Endian::lei(aasworld->reachability[i].start);
		rtcw::Endian::lei(aasworld->reachability[i].end);
		rtcw::Endian::lei(aasworld->reachability[i].traveltype);
		rtcw::Endian::lei(aasworld->reachability[i].traveltime);
	}

	//nodes
	for (int i = 0; i < aasworld->numnodes; ++i) {
		rtcw::Endian::lei(aasworld->nodes[i].planenum);
		rtcw::Endian::lei(aasworld->nodes[i].children);
	}

	//cluster portals
	for (int i = 0; i < aasworld->numportals; ++i) {
		rtcw::Endian::lei(aasworld->portals[i].areanum);
		rtcw::Endian::lei(aasworld->portals[i].frontcluster);
		rtcw::Endian::lei(aasworld->portals[i].backcluster);
		rtcw::Endian::lei(aasworld->portals[i].clusterareanum);
	}

	//cluster portal index
	rtcw::Endian::lei(aasworld->portalindex, aasworld->portalindexsize);

	//cluster
	for (int i = 0; i < aasworld->numclusters; ++i) {
		rtcw::Endian::lei(aasworld->clusters[i].numareas);
		rtcw::Endian::lei(aasworld->clusters[i].numreachabilityareas);
		rtcw::Endian::lei(aasworld->clusters[i].numportals);
		rtcw::Endian::lei(aasworld->clusters[i].firstportal);
	}
} //end of the function AAS_SwapAASData
//===========================================================================
// dump the current loaded aas file
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DumpAASData( void ) {
	( *aasworld ).numbboxes = 0;
	if ( ( *aasworld ).bboxes ) {
		FreeMemory( ( *aasworld ).bboxes );
	}
	( *aasworld ).bboxes = NULL;
	( *aasworld ).numvertexes = 0;
	if ( ( *aasworld ).vertexes ) {
		FreeMemory( ( *aasworld ).vertexes );
	}
	( *aasworld ).vertexes = NULL;
	( *aasworld ).numplanes = 0;
	if ( ( *aasworld ).planes ) {
		FreeMemory( ( *aasworld ).planes );
	}
	( *aasworld ).planes = NULL;
	( *aasworld ).numedges = 0;
	if ( ( *aasworld ).edges ) {
		FreeMemory( ( *aasworld ).edges );
	}
	( *aasworld ).edges = NULL;
	( *aasworld ).edgeindexsize = 0;
	if ( ( *aasworld ).edgeindex ) {
		FreeMemory( ( *aasworld ).edgeindex );
	}
	( *aasworld ).edgeindex = NULL;
	( *aasworld ).numfaces = 0;
	if ( ( *aasworld ).faces ) {
		FreeMemory( ( *aasworld ).faces );
	}
	( *aasworld ).faces = NULL;
	( *aasworld ).faceindexsize = 0;
	if ( ( *aasworld ).faceindex ) {
		FreeMemory( ( *aasworld ).faceindex );
	}
	( *aasworld ).faceindex = NULL;
	( *aasworld ).numareas = 0;
	if ( ( *aasworld ).areas ) {
		FreeMemory( ( *aasworld ).areas );
	}
	( *aasworld ).areas = NULL;
	( *aasworld ).numareasettings = 0;
	if ( ( *aasworld ).areasettings ) {
		FreeMemory( ( *aasworld ).areasettings );
	}
	( *aasworld ).areasettings = NULL;
	( *aasworld ).reachabilitysize = 0;
	if ( ( *aasworld ).reachability ) {
		FreeMemory( ( *aasworld ).reachability );
	}
	( *aasworld ).reachability = NULL;
	( *aasworld ).numnodes = 0;
	if ( ( *aasworld ).nodes ) {
		FreeMemory( ( *aasworld ).nodes );
	}
	( *aasworld ).nodes = NULL;
	( *aasworld ).numportals = 0;
	if ( ( *aasworld ).portals ) {
		FreeMemory( ( *aasworld ).portals );
	}
	( *aasworld ).portals = NULL;
	( *aasworld ).numportals = 0;
	if ( ( *aasworld ).portalindex ) {
		FreeMemory( ( *aasworld ).portalindex );
	}
	( *aasworld ).portalindex = NULL;
	( *aasworld ).portalindexsize = 0;
	if ( ( *aasworld ).clusters ) {
		FreeMemory( ( *aasworld ).clusters );
	}
	( *aasworld ).clusters = NULL;
	( *aasworld ).numclusters = 0;
	//
	( *aasworld ).loaded = qfalse;
	( *aasworld ).initialized = qfalse;
	( *aasworld ).savefile = qfalse;
} //end of the function AAS_DumpAASData
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
#ifdef AASFILEDEBUG
void AAS_FileInfo( void ) {
	int i, n, optimized;

	botimport.Print( PRT_MESSAGE, "version = %d\n", AASVERSION );
	botimport.Print( PRT_MESSAGE, "numvertexes = %d\n", ( *aasworld ).numvertexes );
	botimport.Print( PRT_MESSAGE, "numplanes = %d\n", ( *aasworld ).numplanes );
	botimport.Print( PRT_MESSAGE, "numedges = %d\n", ( *aasworld ).numedges );
	botimport.Print( PRT_MESSAGE, "edgeindexsize = %d\n", ( *aasworld ).edgeindexsize );
	botimport.Print( PRT_MESSAGE, "numfaces = %d\n", ( *aasworld ).numfaces );
	botimport.Print( PRT_MESSAGE, "faceindexsize = %d\n", ( *aasworld ).faceindexsize );
	botimport.Print( PRT_MESSAGE, "numareas = %d\n", ( *aasworld ).numareas );
	botimport.Print( PRT_MESSAGE, "numareasettings = %d\n", ( *aasworld ).numareasettings );
	botimport.Print( PRT_MESSAGE, "reachabilitysize = %d\n", ( *aasworld ).reachabilitysize );
	botimport.Print( PRT_MESSAGE, "numnodes = %d\n", ( *aasworld ).numnodes );
	botimport.Print( PRT_MESSAGE, "numportals = %d\n", ( *aasworld ).numportals );
	botimport.Print( PRT_MESSAGE, "portalindexsize = %d\n", ( *aasworld ).portalindexsize );
	botimport.Print( PRT_MESSAGE, "numclusters = %d\n", ( *aasworld ).numclusters );
	//
	for ( n = 0, i = 0; i < ( *aasworld ).numareasettings; i++ )
	{
		if ( ( *aasworld ).areasettings[i].areaflags & AREA_GROUNDED ) {
			n++;
		}
	} //end for
	botimport.Print( PRT_MESSAGE, "num grounded areas = %d\n", n );
	//
	botimport.Print( PRT_MESSAGE, "planes size %d bytes\n", ( *aasworld ).numplanes * sizeof( aas_plane_t ) );
	botimport.Print( PRT_MESSAGE, "areas size %d bytes\n", ( *aasworld ).numareas * sizeof( aas_area_t ) );
	botimport.Print( PRT_MESSAGE, "areasettings size %d bytes\n", ( *aasworld ).numareasettings * sizeof( aas_areasettings_t ) );
	botimport.Print( PRT_MESSAGE, "nodes size %d bytes\n", ( *aasworld ).numnodes * sizeof( aas_node_t ) );
	botimport.Print( PRT_MESSAGE, "reachability size %d bytes\n", ( *aasworld ).reachabilitysize * sizeof( aas_reachability_t ) );
	botimport.Print( PRT_MESSAGE, "portals size %d bytes\n", ( *aasworld ).numportals * sizeof( aas_portal_t ) );
	botimport.Print( PRT_MESSAGE, "clusters size %d bytes\n", ( *aasworld ).numclusters * sizeof( aas_cluster_t ) );

	optimized = ( *aasworld ).numplanes * sizeof( aas_plane_t ) +
				( *aasworld ).numareas * sizeof( aas_area_t ) +
				( *aasworld ).numareasettings * sizeof( aas_areasettings_t ) +
				( *aasworld ).numnodes * sizeof( aas_node_t ) +
				( *aasworld ).reachabilitysize * sizeof( aas_reachability_t ) +
				( *aasworld ).numportals * sizeof( aas_portal_t ) +
				( *aasworld ).numclusters * sizeof( aas_cluster_t );
	botimport.Print( PRT_MESSAGE, "optimzed size %d KB\n", optimized >> 10 );
} //end of the function AAS_FileInfo
#endif //AASFILEDEBUG
//===========================================================================
// allocate memory and read a lump of a AAS file
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *AAS_LoadAASLump( fileHandle_t fp, int offset, int length, int *lastoffset ) {
	char *buf;
	//
	if ( !length ) {
		return NULL;
	}
	//seek to the data
	if ( offset != *lastoffset ) {
		botimport.Print( PRT_WARNING, "AAS file not sequentially read\n" );
		if ( botimport.FS_Seek( fp, offset, FS_SEEK_SET ) ) {
			AAS_Error( "can't seek to aas lump\n" );
			AAS_DumpAASData();
			botimport.FS_FCloseFile( fp );
			return 0;
		} //end if
	} //end if
	  //allocate memory
	buf = (char *) GetClearedHunkMemory( length + 1 );
	//read the data
	if ( length ) {
		botimport.FS_Read( buf, length, fp );
		*lastoffset += length;
	} //end if
	return buf;
} //end of the function AAS_LoadAASLump
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_DData( unsigned char *data, int size ) {
	int i;

	for ( i = 0; i < size; i++ )
	{
		data[i] ^= (unsigned char) i * 119;
	} //end for
} //end of the function AAS_DData
//===========================================================================
// load an aas file
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_LoadAASFile( char *filename ) {
	fileHandle_t fp;
	aas_header_t header;
	int offset, length, lastoffset;

#if defined RTCW_ET
	int nocrc;
#endif // RTCW_XX

	botimport.Print( PRT_MESSAGE, "trying to load %s\n", filename );
	//dump current loaded aas file
	AAS_DumpAASData();
	//open the file
	botimport.FS_FOpenFile( filename, &fp, FS_READ );
	if ( !fp ) {
		AAS_Error( "can't open %s\n", filename );
		return BLERR_CANNOTOPENAASFILE;
	} //end if
	  //read the header
	botimport.FS_Read( &header, sizeof( aas_header_t ), fp );
	lastoffset = sizeof( aas_header_t );
	//check header identification
	rtcw::Endian::lei(header.ident);
	if ( header.ident != AASID ) {
		AAS_Error( "%s is not an AAS file\n", filename );
		botimport.FS_FCloseFile( fp );
		return BLERR_WRONGAASFILEID;
	} //end if
	  //check the version
	rtcw::Endian::lei(header.version);
	//
	if ( header.version != AASVERSION ) {
		AAS_Error( "aas file %s is version %i, not %i\n", filename, header.version, AASVERSION );
		botimport.FS_FCloseFile( fp );
		return BLERR_WRONGAASFILEVERSION;
	} //end if
	  //

#if defined RTCW_ET
	  //RF, checksum of -1 always passes, hack to fix commercial maps without having to distribute new bsps
	nocrc = 0;
	if (rtcw::Endian::le(header.bspchecksum) == -1)
		nocrc = 1;
	//
#endif // RTCW_XX

	if ( header.version == AASVERSION ) {
		AAS_DData( (unsigned char *) &header + 8, sizeof( aas_header_t ) - 8 );
	} //end if
	  //
	( *aasworld ).bspchecksum = atoi( LibVarGetString( "sv_mapChecksum" ) );

#if !defined RTCW_ET
	if ( rtcw::Endian::le( header.bspchecksum ) != ( *aasworld ).bspchecksum ) {
#else
	if ( !nocrc && rtcw::Endian::le( header.bspchecksum ) != ( *aasworld ).bspchecksum ) {
#endif // RTCW_XX

		AAS_Error( "aas file %s is out of date\n", filename );
		botimport.FS_FCloseFile( fp );
		return BLERR_WRONGAASFILEVERSION;
	} //end if
	  //load the lumps:
	  //bounding boxes
	offset = rtcw::Endian::le( header.lumps[AASLUMP_BBOXES].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_BBOXES].filelen );
	( *aasworld ).bboxes = (aas_bbox_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numbboxes = length / sizeof( aas_bbox_t );
	if ( ( *aasworld ).numbboxes && !( *aasworld ).bboxes ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//vertexes
	offset = rtcw::Endian::le( header.lumps[AASLUMP_VERTEXES].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_VERTEXES].filelen );
	( *aasworld ).vertexes = (aas_vertex_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numvertexes = length / sizeof( aas_vertex_t );
	if ( ( *aasworld ).numvertexes && !( *aasworld ).vertexes ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//planes
	offset = rtcw::Endian::le( header.lumps[AASLUMP_PLANES].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_PLANES].filelen );
	( *aasworld ).planes = (aas_plane_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numplanes = length / sizeof( aas_plane_t );
	if ( ( *aasworld ).numplanes && !( *aasworld ).planes ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//edges
	offset = rtcw::Endian::le( header.lumps[AASLUMP_EDGES].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_EDGES].filelen );
	( *aasworld ).edges = (aas_edge_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numedges = length / sizeof( aas_edge_t );
	if ( ( *aasworld ).numedges && !( *aasworld ).edges ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//edgeindex
	offset = rtcw::Endian::le( header.lumps[AASLUMP_EDGEINDEX].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_EDGEINDEX].filelen );
	( *aasworld ).edgeindex = (aas_edgeindex_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).edgeindexsize = length / sizeof( aas_edgeindex_t );
	if ( ( *aasworld ).edgeindexsize && !( *aasworld ).edgeindex ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//faces
	offset = rtcw::Endian::le( header.lumps[AASLUMP_FACES].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_FACES].filelen );
	( *aasworld ).faces = (aas_face_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numfaces = length / sizeof( aas_face_t );
	if ( ( *aasworld ).numfaces && !( *aasworld ).faces ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//faceindex
	offset = rtcw::Endian::le( header.lumps[AASLUMP_FACEINDEX].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_FACEINDEX].filelen );
	( *aasworld ).faceindex = (aas_faceindex_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).faceindexsize = length / sizeof( int );
	if ( ( *aasworld ).faceindexsize && !( *aasworld ).faceindex ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//convex areas
	offset = rtcw::Endian::le( header.lumps[AASLUMP_AREAS].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_AREAS].filelen );
	( *aasworld ).areas = (aas_area_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numareas = length / sizeof( aas_area_t );
	if ( ( *aasworld ).numareas && !( *aasworld ).areas ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//area settings
	offset = rtcw::Endian::le( header.lumps[AASLUMP_AREASETTINGS].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_AREASETTINGS].filelen );
	( *aasworld ).areasettings = (aas_areasettings_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numareasettings = length / sizeof( aas_areasettings_t );
	if ( ( *aasworld ).numareasettings && !( *aasworld ).areasettings ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//reachability list
	offset = rtcw::Endian::le( header.lumps[AASLUMP_REACHABILITY].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_REACHABILITY].filelen );
	( *aasworld ).reachability = (aas_reachability_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).reachabilitysize = length / sizeof( aas_reachability_t );
	if ( ( *aasworld ).reachabilitysize && !( *aasworld ).reachability ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//nodes
	offset = rtcw::Endian::le( header.lumps[AASLUMP_NODES].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_NODES].filelen );
	( *aasworld ).nodes = (aas_node_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numnodes = length / sizeof( aas_node_t );
	if ( ( *aasworld ).numnodes && !( *aasworld ).nodes ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//cluster portals
	offset = rtcw::Endian::le( header.lumps[AASLUMP_PORTALS].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_PORTALS].filelen );
	( *aasworld ).portals = (aas_portal_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numportals = length / sizeof( aas_portal_t );
	if ( ( *aasworld ).numportals && !( *aasworld ).portals ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//cluster portal index
	offset = rtcw::Endian::le( header.lumps[AASLUMP_PORTALINDEX].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_PORTALINDEX].filelen );
	( *aasworld ).portalindex = (aas_portalindex_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).portalindexsize = length / sizeof( aas_portalindex_t );
	if ( ( *aasworld ).portalindexsize && !( *aasworld ).portalindex ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//clusters
	offset = rtcw::Endian::le( header.lumps[AASLUMP_CLUSTERS].fileofs );
	length = rtcw::Endian::le( header.lumps[AASLUMP_CLUSTERS].filelen );
	( *aasworld ).clusters = (aas_cluster_t *) AAS_LoadAASLump( fp, offset, length, &lastoffset );
	( *aasworld ).numclusters = length / sizeof( aas_cluster_t );
	if ( ( *aasworld ).numclusters && !( *aasworld ).clusters ) {
		return BLERR_CANNOTREADAASLUMP;
	}
	//swap everything
	AAS_SwapAASData();
	//aas file is loaded
	( *aasworld ).loaded = qtrue;
	//close the file
	botimport.FS_FCloseFile( fp );
	//
#ifdef AASFILEDEBUG
	AAS_FileInfo();
#endif //AASFILEDEBUG
	   //

#if defined RTCW_ET
	{
		int j = 0;
		int i;
		for ( i = 1; i < aasworld->numareas; i++ ) {
			j += aasworld->areasettings[i].numreachableareas;
		}
		if ( j > aasworld->reachabilitysize ) {
			Com_Error( ERR_DROP, "aas reachabilitysize incorrect\n" );
		}
	}
#endif // RTCW_XX

	return BLERR_NOERROR;
} //end of the function AAS_LoadAASFile
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
static int AAS_WriteAASLump_offset;

int AAS_WriteAASLump( fileHandle_t fp, aas_header_t *h, int lumpnum, void *data, int length ) {
	aas_lump_t *lump;

	lump = &h->lumps[lumpnum];

	lump->fileofs = rtcw::Endian::le( AAS_WriteAASLump_offset );    //LittleLong(ftell(fp));
	lump->filelen = rtcw::Endian::le( length );

	if ( length > 0 ) {
		botimport.FS_Write( data, length, fp );
	} //end if

	AAS_WriteAASLump_offset += length;

	return qtrue;
} //end of the function AAS_WriteAASLump
//===========================================================================
// aas data is useless after writing to file because it is byte swapped
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean AAS_WriteAASFile( char *filename ) {
	aas_header_t header;
	fileHandle_t fp;

	botimport.Print( PRT_MESSAGE, "writing %s\n", filename );
	//swap the aas data
	AAS_SwapAASData();
	//initialize the file header
	memset( &header, 0, sizeof( aas_header_t ) );
	header.ident = rtcw::Endian::le( AASID );
	header.version = rtcw::Endian::le( AASVERSION );
	header.bspchecksum = rtcw::Endian::le( ( *aasworld ).bspchecksum );
	//open a new file
	botimport.FS_FOpenFile( filename, &fp, FS_WRITE );
	if ( !fp ) {
		botimport.Print( PRT_ERROR, "error opening %s\n", filename );
		return qfalse;
	} //end if
	  //write the header
	botimport.FS_Write( &header, sizeof( aas_header_t ), fp );
	AAS_WriteAASLump_offset = sizeof( aas_header_t );
	//add the data lumps to the file
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_BBOXES, ( *aasworld ).bboxes,
							( *aasworld ).numbboxes * sizeof( aas_bbox_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_VERTEXES, ( *aasworld ).vertexes,
							( *aasworld ).numvertexes * sizeof( aas_vertex_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_PLANES, ( *aasworld ).planes,
							( *aasworld ).numplanes * sizeof( aas_plane_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_EDGES, ( *aasworld ).edges,
							( *aasworld ).numedges * sizeof( aas_edge_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_EDGEINDEX, ( *aasworld ).edgeindex,
							( *aasworld ).edgeindexsize * sizeof( aas_edgeindex_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_FACES, ( *aasworld ).faces,
							( *aasworld ).numfaces * sizeof( aas_face_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_FACEINDEX, ( *aasworld ).faceindex,
							( *aasworld ).faceindexsize * sizeof( aas_faceindex_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_AREAS, ( *aasworld ).areas,
							( *aasworld ).numareas * sizeof( aas_area_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_AREASETTINGS, ( *aasworld ).areasettings,
							( *aasworld ).numareasettings * sizeof( aas_areasettings_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_REACHABILITY, ( *aasworld ).reachability,
							( *aasworld ).reachabilitysize * sizeof( aas_reachability_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_NODES, ( *aasworld ).nodes,
							( *aasworld ).numnodes * sizeof( aas_node_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_PORTALS, ( *aasworld ).portals,
							( *aasworld ).numportals * sizeof( aas_portal_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_PORTALINDEX, ( *aasworld ).portalindex,
							( *aasworld ).portalindexsize * sizeof( aas_portalindex_t ) ) ) {
		return qfalse;
	}
	if ( !AAS_WriteAASLump( fp, &header, AASLUMP_CLUSTERS, ( *aasworld ).clusters,
							( *aasworld ).numclusters * sizeof( aas_cluster_t ) ) ) {
		return qfalse;
	}
	//rewrite the header with the added lumps
	botimport.FS_Seek( fp, 0, FS_SEEK_SET );
	botimport.FS_Write( &header, sizeof( aas_header_t ), fp );
	//close the file
	botimport.FS_FCloseFile( fp );
	return qtrue;
} //end of the function AAS_WriteAASFile

