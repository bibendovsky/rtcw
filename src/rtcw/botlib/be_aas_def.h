/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_def.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

//debugging on
#define AAS_DEBUG

#if !defined RTCW_ET
//#define MAX_CLIENTS			128
//#define	MAX_MODELS			256		// these are sent over the net as 8 bits
//#define	MAX_SOUNDS			256		// so they cannot be blindly increased
//#define	MAX_CONFIGSTRINGS	1024
#define MAX_CONFIGSTRINGS   2048    //----(SA)	upped

//#define	CS_SCORES			32
//#define	CS_MODELS			(CS_SCORES+MAX_CLIENTS)
//#define	CS_SOUNDS			(CS_MODELS+MAX_MODELS)
#endif // RTCW_XX

#define DF_AASENTNUMBER( x )      ( x - ( *aasworlds ).entities )
#define DF_NUMBERAASENT( x )      ( &( *aasworlds ).entities[x] )
#define DF_AASENTCLIENT( x )      ( x - ( *aasworlds ).entities - 1 )
#define DF_CLIENTAASENT( x )      ( &( *aasworlds ).entities[x + 1] )

#ifndef MAX_PATH
	#define MAX_PATH                MAX_QPATH
#endif

//string index (for model, sound and image index)
typedef struct aas_stringindex_s
{
	int numindexes;
	char **index;
} aas_stringindex_t;

//structure to link entities to areas and areas to entities
typedef struct aas_link_s
{
	int entnum;
	int areanum;
	struct aas_link_s *next_ent, *prev_ent;
	struct aas_link_s *next_area, *prev_area;
} aas_link_t;

//structure to link entities to leaves and leaves to entities
typedef struct bsp_link_s
{
	int entnum;
	int leafnum;
	struct bsp_link_s *next_ent, *prev_ent;
	struct bsp_link_s *next_leaf, *prev_leaf;
} bsp_link_t;

typedef struct bsp_entdata_s
{
	vec3_t origin;
	vec3_t angles;
	vec3_t absmins;
	vec3_t absmaxs;
	int solid;
	int modelnum;
} bsp_entdata_t;

//entity
typedef struct aas_entity_s
{
	//entity info
	aas_entityinfo_t i;
	//links into the AAS areas
	aas_link_t *areas;
	//links into the BSP leaves
	bsp_link_t *leaves;
} aas_entity_t;

typedef struct aas_settings_s
{
	float sv_friction;
	float sv_stopspeed;
	float sv_gravity;
	float sv_waterfriction;
	float sv_watergravity;
	float sv_maxvelocity;
	float sv_maxwalkvelocity;
	float sv_maxcrouchvelocity;
	float sv_maxswimvelocity;
	float sv_walkaccelerate;
	float sv_airaccelerate;
	float sv_swimaccelerate;
	float sv_maxstep;
	float sv_maxsteepness;
	float sv_maxwaterjump;
	float sv_maxbarrier;
	float sv_jumpvel;

#if !defined RTCW_MP
	qboolean sv_allowladders;
#endif // RTCW_XX

} aas_settings_t;

//routing cache

// BBi Reference structure for x64 code.
// BBi All pointers replaced with 4-byte integers.
// BBi Of course this structure must be identical to original one (aas_routingcache_t).
struct AasRoutingCache32 {
	int32_t size;
	float time;
	int32_t cluster;
	int32_t areanum;
	vec3_t origin;
	float starttraveltime;
	int32_t travelflags;
	int32_t prev; // pointer
	int32_t next; // pointer
	int32_t reachabilities; // pointer
	uint16_t traveltimes[1];
}; // struct AasRoutingCache32
// BBi

typedef struct aas_routingcache_s
{
	int size;                                   //size of the routing cache
	float time;                                 //last time accessed or updated
	int cluster;                                //cluster the cache is for
	int areanum;                                //area the cache is created for
	vec3_t origin;                              //origin within the area
	float starttraveltime;                      //travel time to start with
	int travelflags;                            //combinations of the travel flags
	struct aas_routingcache_s *prev, *next;
	unsigned char *reachabilities;              //reachabilities used for routing
	unsigned short int traveltimes[1];          //travel time for every area (variable sized)


	// BBi
	typedef aas_routingcache_s Struct;
	typedef AasRoutingCache32 Struct32;

	static Struct* convertFrom32 (
		const Struct32* struct32);

	void convertTo32 (
		Struct32* struct32) const;
	// BBi
} aas_routingcache_t;

//fields for the routing algorithm
typedef struct aas_routingupdate_s
{
	int cluster;
	int areanum;                                //area number of the update
	vec3_t start;                               //start point the area was entered
	unsigned short int tmptraveltime;           //temporary travel time
	unsigned short int *areatraveltimes;        //travel times within the area
	qboolean inlist;                            //true if the update is in the list
	struct aas_routingupdate_s *next;
	struct aas_routingupdate_s *prev;
} aas_routingupdate_t;

//reversed reachability link
typedef struct aas_reversedlink_s
{
	int linknum;                                //the aas_areareachability_t
	int areanum;                                //reachable from this area
	struct aas_reversedlink_s *next;            //next link
} aas_reversedlink_t;

//reversed area reachability
typedef struct aas_reversedreachability_s
{
	int numlinks;
	aas_reversedlink_t *first;
} aas_reversedreachability_t;

// Ridah, route-tables
#include "be_aas_routetable.h"
// done.

typedef struct aas_s
{
	int loaded;                                 //true when an AAS file is loaded
	int initialized;                            //true when AAS has been initialized
	int savefile;                               //set true when file should be saved
	int bspchecksum;
	//current time
	float time;
	int numframes;
	//name of the aas file
	char filename[MAX_PATH];
	char mapname[MAX_PATH];
	//bounding boxes
	int numbboxes;
	aas_bbox_t *bboxes;
	//vertexes
	int numvertexes;
	aas_vertex_t *vertexes;
	//planes
	int numplanes;
	aas_plane_t *planes;
	//edges
	int numedges;
	aas_edge_t *edges;
	//edge index
	int edgeindexsize;
	aas_edgeindex_t *edgeindex;
	//faces
	int numfaces;
	aas_face_t *faces;
	//face index
	int faceindexsize;
	aas_faceindex_t *faceindex;
	//convex areas
	int numareas;
	aas_area_t *areas;
	//convex area settings
	int numareasettings;
	aas_areasettings_t *areasettings;
	//reachablity list
	int reachabilitysize;
	aas_reachability_t *reachability;
	//nodes of the bsp tree
	int numnodes;
	aas_node_t *nodes;
	//cluster portals
	int numportals;
	aas_portal_t *portals;
	//cluster portal index
	int portalindexsize;
	aas_portalindex_t *portalindex;
	//clusters
	int numclusters;
	aas_cluster_t *clusters;
	//
	int reachabilityareas;
	float reachabilitytime;
	//enities linked in the areas
	aas_link_t *linkheap;                       //heap with link structures
	int linkheapsize;                           //size of the link heap
	aas_link_t *freelinks;                      //first free link
	aas_link_t **arealinkedentities;            //entities linked into areas
	//entities
	int maxentities;
	int maxclients;
	aas_entity_t *entities;
	//string indexes
	char *configstrings[MAX_CONFIGSTRINGS];
	int indexessetup;
	//index to retrieve travel flag for a travel type
	int travelflagfortype[MAX_TRAVELTYPES];
	//routing update
	aas_routingupdate_t *areaupdate;
	aas_routingupdate_t *portalupdate;
	//number of routing updates during a frame (reset every frame)
	int frameroutingupdates;
	//reversed reachability links
	aas_reversedreachability_t *reversedreachability;
	//travel times within the areas
	unsigned short ***areatraveltimes;
	//array of size numclusters with cluster cache
	aas_routingcache_t ***clusterareacache;
	aas_routingcache_t **portalcache;
	//maximum travel time through portals
	int *portalmaxtraveltimes;
	// Ridah, pointer to Route-Table information
	aas_rt_t    *routetable;
	//hide travel times
	unsigned short int *hidetraveltimes;

#if defined RTCW_ET
	// Distance from Dangerous areas
	unsigned short int *distanceFromDanger;

	// Priority Queue Implementation
	unsigned short int *PQ_accumulator;

	// How many items are in the PQ
	int PQ_size;
#endif // RTCW_XX

	//vis data
	byte *decompressedvis;
	int decompressedvisarea;
	byte **areavisibility;
	// done.
	// Ridah, store the area's waypoint for hidepos calculations (center traced downwards)
	vec3_t *areawaypoints;
	// Ridah, so we can cache the areas that have already been tested for visibility/attackability
	byte *visCache;

#if defined RTCW_ET
	// RF, cluster team flags (-1 means not calculated)
	int *clusterTeamTravelFlags;
	// RF, last time a death influenced this area. Seperate lists for axis/allies
	int *teamDeathTime;
	// RF, number of deaths accumulated before the time expired
	byte    *teamDeathCount;
	// RF, areas that are influenced by a death count
	byte    *teamDeathAvoid;
#endif // RTCW_XX

} aas_t;

#define AASINTERN

#ifndef BSPCINCLUDE

#include "be_aas_main.h"
#include "be_aas_entity.h"
#include "be_aas_sample.h"
#include "be_aas_cluster.h"
#include "be_aas_reach.h"
#include "be_aas_route.h"
#include "be_aas_routealt.h"
#include "be_aas_debug.h"
#include "be_aas_file.h"
#include "be_aas_optimize.h"
#include "be_aas_bsp.h"
#include "be_aas_move.h"

// Ridah, route-tables
#include "be_aas_routetable.h"

#endif //BSPCINCLUDE
