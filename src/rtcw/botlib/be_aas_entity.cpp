/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_entity.c
 *
 * desc:		AAS entities
 *
 *
 *****************************************************************************/

#include "q_shared.h"
#include "l_memory.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "l_utils.h"
#include "l_log.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_interface.h"
#include "be_aas_def.h"

#define MASK_SOLID      CONTENTS_PLAYERCLIP

// Ridah, always use the default world for entities

#if !defined RTCW_MP
extern aas_t aasworlds[MAX_AAS_WORLDS];
#else
extern aas_t aasworlds[2];
#endif // RTCW_XX

aas_t *defaultaasworld = aasworlds;

//FIXME: these might change

#if !defined RTCW_ET
enum {
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER
};
#else
/*enum {
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER
};*/
#endif // RTCW_XX

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_UpdateEntity( int entnum, bot_entitystate_t *state ) {
	int relink;
	aas_entity_t *ent;
	vec3_t absmins, absmaxs;

	if ( !( *defaultaasworld ).loaded ) {
		botimport.Print( PRT_MESSAGE, "AAS_UpdateEntity: not loaded\n" );
		return BLERR_NOAASFILE;
	} //end if

	ent = &( *defaultaasworld ).entities[entnum];

	ent->i.update_time = AAS_Time() - ent->i.ltime;
	ent->i.type = state->type;
	ent->i.flags = state->flags;
	ent->i.ltime = AAS_Time();
	VectorCopy( ent->i.origin, ent->i.lastvisorigin );
	VectorCopy( state->old_origin, ent->i.old_origin );
	ent->i.solid = state->solid;
	ent->i.groundent = state->groundent;
	ent->i.modelindex = state->modelindex;
	ent->i.modelindex2 = state->modelindex2;
	ent->i.frame = state->frame;
	//ent->i.event = state->event;
	ent->i.eventParm = state->eventParm;
	ent->i.powerups = state->powerups;
	ent->i.weapon = state->weapon;
	ent->i.legsAnim = state->legsAnim;
	ent->i.torsoAnim = state->torsoAnim;

//	ent->i.weapAnim = state->weapAnim;	//----(SA)
//----(SA)	didn't want to comment in as I wasn't sure of any implications of changing the aas_entityinfo_t and bot_entitystate_t structures.

	//number of the entity
	ent->i.number = entnum;
	//updated so set valid flag
	ent->i.valid = qtrue;
	//link everything the first frame

	if ( ( *defaultaasworld ).numframes == 1 ) {
		relink = qtrue;

#if !defined RTCW_ET
	} else { relink = qfalse;}
#else
	} else {
		relink = qfalse;
	}
#endif // RTCW_XX

	//
	if ( ent->i.solid == SOLID_BSP ) {
		//if the angles of the model changed
		if ( !VectorCompare( state->angles, ent->i.angles ) ) {
			VectorCopy( state->angles, ent->i.angles );
			relink = qtrue;
		} //end if
		  //get the mins and maxs of the model
		  //FIXME: rotate mins and maxs

#if !defined RTCW_ET
		AAS_BSPModelMinsMaxsOrigin( ent->i.modelindex, ent->i.angles, ent->i.mins, ent->i.maxs, NULL );
#else
		// RF, this is broken, just use the state bounds
		//AAS_BSPModelMinsMaxsOrigin(ent->i.modelindex, ent->i.angles, ent->i.mins, ent->i.maxs, NULL);
		VectorCopy( state->mins, ent->i.mins );
		VectorCopy( state->maxs, ent->i.maxs );
#endif // RTCW_XX

	} //end if
	else if ( ent->i.solid == SOLID_BBOX ) {
		//if the bounding box size changed
		if ( !VectorCompare( state->mins, ent->i.mins ) ||
			 !VectorCompare( state->maxs, ent->i.maxs ) ) {
			VectorCopy( state->mins, ent->i.mins );
			VectorCopy( state->maxs, ent->i.maxs );
			relink = qtrue;
		} //end if
	} //end if
	  //if the origin changed
	if ( !VectorCompare( state->origin, ent->i.origin ) ) {
		VectorCopy( state->origin, ent->i.origin );
		relink = qtrue;
	} //end if
	  //if the entity should be relinked
	if ( relink ) {
		//don't link the world model
		if ( entnum != ENTITYNUM_WORLD ) {
			//absolute mins and maxs
			VectorAdd( ent->i.mins, ent->i.origin, absmins );
			VectorAdd( ent->i.maxs, ent->i.origin, absmaxs );

			//unlink the entity
			AAS_UnlinkFromAreas( ent->areas );
			//relink the entity to the AAS areas (use the larges bbox)
			ent->areas = AAS_LinkEntityClientBBox( absmins, absmaxs, entnum, PRESENCE_NORMAL );
			//unlink the entity from the BSP leaves
			AAS_UnlinkFromBSPLeaves( ent->leaves );
			//link the entity to the world BSP tree
			ent->leaves = AAS_BSPLinkEntity( absmins, absmaxs, entnum, 0 );
		} //end if
	} //end if
	return BLERR_NOERROR;
} //end of the function AAS_UpdateEntity
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_EntityInfo( int entnum, aas_entityinfo_t *info ) {
#if !defined RTCW_ET
	if ( !( *defaultaasworld ).initialized ) {
		botimport.Print( PRT_FATAL, "AAS_EntityInfo: (*defaultaasworld) not initialized\n" );
		memset( info, 0, sizeof( aas_entityinfo_t ) );
		return;
	} //end if

	if ( entnum < 0 || entnum >= ( *defaultaasworld ).maxentities ) {
		botimport.Print( PRT_FATAL, "AAS_EntityInfo: entnum %d out of range\n", entnum );
#else
	// Gordon: lets not spam this message making it impossible to see anything on the console
	static qboolean debug_msg_done = qfalse;

	if ( !( *defaultaasworld ).initialized ) {
		if ( !debug_msg_done ) {
			debug_msg_done = qtrue;
			botimport.Print( PRT_FATAL, "AAS_EntityInfo: (*defaultaasworld) not initialized\n" );
			memset( info, 0, sizeof( aas_entityinfo_t ) );
		}
		return;
	} //end if

	if ( entnum < 0 || entnum >= ( *defaultaasworld ).maxentities ) {
		// if it's not a bot game entity, then report it
		if ( !( entnum >= ( *defaultaasworld ).maxentities && entnum < ( *defaultaasworld ).maxentities + NUM_BOTGAMEENTITIES ) ) {
			botimport.Print( PRT_FATAL, "AAS_EntityInfo: entnum %d out of range\n", entnum );
		}
#endif // RTCW_XX

		memset( info, 0, sizeof( aas_entityinfo_t ) );
		return;
	} //end if

	memcpy( info, &( *defaultaasworld ).entities[entnum].i, sizeof( aas_entityinfo_t ) );
} //end of the function AAS_EntityInfo
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_EntityOrigin( int entnum, vec3_t origin ) {
	if ( entnum < 0 || entnum >= ( *defaultaasworld ).maxentities ) {
		botimport.Print( PRT_FATAL, "AAS_EntityOrigin: entnum %d out of range\n", entnum );
		VectorClear( origin );
		return;
	} //end if

	VectorCopy( ( *defaultaasworld ).entities[entnum].i.origin, origin );
} //end of the function AAS_EntityOrigin
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_EntityModelindex( int entnum ) {
	if ( entnum < 0 || entnum >= ( *defaultaasworld ).maxentities ) {
		botimport.Print( PRT_FATAL, "AAS_EntityModelindex: entnum %d out of range\n", entnum );
		return 0;
	} //end if
	return ( *defaultaasworld ).entities[entnum].i.modelindex;
} //end of the function AAS_EntityModelindex
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_EntityType( int entnum ) {
	if ( !( *defaultaasworld ).initialized ) {
		return 0;
	}

	if ( entnum < 0 || entnum >= ( *defaultaasworld ).maxentities ) {
		botimport.Print( PRT_FATAL, "AAS_EntityType: entnum %d out of range\n", entnum );
		return 0;
	} //end if
	return ( *defaultaasworld ).entities[entnum].i.type;
} //end of the AAS_EntityType
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_EntityModelNum( int entnum ) {
	if ( !( *defaultaasworld ).initialized ) {
		return 0;
	}

	if ( entnum < 0 || entnum >= ( *defaultaasworld ).maxentities ) {
		botimport.Print( PRT_FATAL, "AAS_EntityModelNum: entnum %d out of range\n", entnum );
		return 0;
	} //end if
	return ( *defaultaasworld ).entities[entnum].i.modelindex;
} //end of the function AAS_EntityModelNum
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_OriginOfEntityWithModelNum( int modelnum, vec3_t origin ) {
	int i;
	aas_entity_t *ent;

	for ( i = 0; i < ( *defaultaasworld ).maxentities; i++ )
	{
		ent = &( *defaultaasworld ).entities[i];
		if ( ent->i.type == ET_MOVER ) {
			if ( ent->i.modelindex == modelnum ) {
				VectorCopy( ent->i.origin, origin );
				return qtrue;
			} //end if
		}
	} //end for
	return qfalse;
} //end of the function AAS_OriginOfEntityWithModelNum
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_EntitySize( int entnum, vec3_t mins, vec3_t maxs ) {
	aas_entity_t *ent;

	if ( !( *defaultaasworld ).initialized ) {
		return;
	}

	if ( entnum < 0 || entnum >= ( *defaultaasworld ).maxentities ) {
		botimport.Print( PRT_FATAL, "AAS_EntitySize: entnum %d out of range\n", entnum );
		return;
	} //end if

	ent = &( *defaultaasworld ).entities[entnum];
	VectorCopy( ent->i.mins, mins );
	VectorCopy( ent->i.maxs, maxs );
} //end of the function AAS_EntitySize
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_EntityBSPData( int entnum, bsp_entdata_t *entdata ) {
	aas_entity_t *ent;

	ent = &( *defaultaasworld ).entities[entnum];
	VectorCopy( ent->i.origin, entdata->origin );
	VectorCopy( ent->i.angles, entdata->angles );
	VectorAdd( ent->i.origin, ent->i.mins, entdata->absmins );
	VectorAdd( ent->i.origin, ent->i.maxs, entdata->absmaxs );
	entdata->solid = ent->i.solid;
	entdata->modelnum = ent->i.modelindex - 1;
} //end of the function AAS_EntityBSPData
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ResetEntityLinks( void ) {
	int i;
	for ( i = 0; i < ( *defaultaasworld ).maxentities; i++ )
	{
		( *defaultaasworld ).entities[i].areas = NULL;
		( *defaultaasworld ).entities[i].leaves = NULL;
	} //end for
} //end of the function AAS_ResetEntityLinks
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_InvalidateEntities( void ) {
	int i;
	for ( i = 0; i < ( *defaultaasworld ).maxentities; i++ )
	{
		( *defaultaasworld ).entities[i].i.valid = qfalse;
		( *defaultaasworld ).entities[i].i.number = i;
	} //end for
} //end of the function AAS_InvalidateEntities
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_NearestEntity( vec3_t origin, int modelindex ) {
	int i, bestentnum;
	float dist, bestdist;
	aas_entity_t *ent;
	vec3_t dir;

	bestentnum = 0;
	bestdist = 99999;
	for ( i = 0; i < ( *defaultaasworld ).maxentities; i++ )
	{
		ent = &( *defaultaasworld ).entities[i];
		if ( ent->i.modelindex != modelindex ) {
			continue;
		}
		VectorSubtract( ent->i.origin, origin, dir );
		if ( c::abs( dir[0] ) < 40 ) {
			if ( c::abs( dir[1] ) < 40 ) {
				dist = VectorLength( dir );
				if ( dist < bestdist ) {
					bestdist = dist;
					bestentnum = i;
				} //end if
			} //end if
		} //end if
	} //end for
	return bestentnum;
} //end of the function AAS_NearestEntity
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_BestReachableEntityArea( int entnum ) {
	aas_entity_t *ent;

	ent = &( *defaultaasworld ).entities[entnum];
	return AAS_BestReachableLinkArea( ent->areas );
} //end of the function AAS_BestReachableEntityArea
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_NextEntity( int entnum ) {
	if ( !( *defaultaasworld ).loaded ) {
		return 0;
	}

	if ( entnum < 0 ) {
		entnum = -1;
	}
	while ( ++entnum < ( *defaultaasworld ).maxentities )
	{
		if ( ( *defaultaasworld ).entities[entnum].i.valid ) {
			return entnum;
		}
	} //end while
	return 0;
} //end of the function AAS_NextEntity

// Ridah, used to find out if there is an entity touching the given area, if so, try and avoid it
/*
============
AAS_EntityInArea
============
*/
int AAS_IsEntityInArea( int entnumIgnore, int entnumIgnore2, int areanum ) {
	aas_link_t *link;
	aas_entity_t *ent;

#if !defined RTCW_ET
//	int i;
#endif // RTCW_XX

#if defined RTCW_SP
	// RF, not functional (doesnt work with multiple areas)
	return qfalse;
#endif // RTCW_XX

	for ( link = ( *aasworld ).arealinkedentities[areanum]; link; link = link->next_ent )
	{
		//ignore the pass entity
		if ( link->entnum == entnumIgnore ) {
			continue;
		}
		if ( link->entnum == entnumIgnore2 ) {
			continue;
		}
		//
		ent = &( *defaultaasworld ).entities[link->entnum];
		if ( !ent->i.valid ) {
			continue;
		}
		if ( !ent->i.solid ) {
			continue;
		}
		return qtrue;
	}
/*
	ent = (*defaultaasworld).entities;
	for (i = 0; i < (*defaultaasworld).maxclients; i++, ent++)
	{
		if (!ent->i.valid)
			continue;
		if (!ent->i.solid)
			continue;
		if (i == entnumIgnore)
			continue;
		if (i == entnumIgnore2)
			continue;
		for (link = ent->areas; link; link = link->next_area)
		{
			if (link->areanum == areanum)
			{
				return qtrue;
			} //end if
		} //end for
	}
*/
	return qfalse;
}

/*
=============
AAS_SetAASBlockingEntity
=============
*/
int AAS_EnableRoutingArea( int areanum, int enable );

#if !defined RTCW_ET
void AAS_SetAASBlockingEntity( vec3_t absmin, vec3_t absmax, qboolean blocking ) {
	int areas[128];
#else
void AAS_SetAASBlockingEntity( vec3_t absmin, vec3_t absmax, int blocking ) {
	int areas[1024];
#endif // RTCW_XX

	int numareas, i, w;

#if defined RTCW_ET
	qboolean mover, changed = qfalse;
#endif // RTCW_XX
	//
	// check for resetting AAS blocking

#if !defined RTCW_MP
	if ( VectorCompare( absmin, absmax ) && blocking < 0 ) {
#else
	// TTimo WTF?? qboolean blocking
	// warning: comparison is always false due to limited range of data type
	//  if (VectorCompare( absmin, absmax ) && blocking < 0) {
	if ( VectorCompare( absmin, absmax ) && !blocking ) {
#endif // RTCW_XX

		for ( w = 0; w < MAX_AAS_WORLDS; w++ ) {
			AAS_SetCurrentWorld( w );
			//
			if ( !( *aasworld ).loaded ) {
				continue;
			}
			// now clear blocking status
			for ( i = 1; i < ( *aasworld ).numareas; i++ ) {
				AAS_EnableRoutingArea( i, qtrue );
			}
		}
		//
		return;
	}
	//

#if defined RTCW_ET
	if ( blocking & BLOCKINGFLAG_MOVER ) {
		mover = qtrue;
		blocking &= ~BLOCKINGFLAG_MOVER;
	} else {
		mover = qfalse;
	}
	//
areas_again:
	//
#endif // RTCW_XX

	for ( w = 0; w < MAX_AAS_WORLDS; w++ ) {
		AAS_SetCurrentWorld( w );
		//
		if ( !( *aasworld ).loaded ) {
			continue;
		}
		// grab the list of areas

#if !defined RTCW_ET
		numareas = AAS_BBoxAreas( absmin, absmax, areas, 128 );
#else
		numareas = AAS_BBoxAreas( absmin, absmax, areas, 1024 );
#endif // RTCW_XX

		// now set their blocking status
		for ( i = 0; i < numareas; i++ ) {

#if !defined RTCW_ET
			AAS_EnableRoutingArea( areas[i], !blocking );
		}
	}
}
#else
			if ( mover ) {
				if ( !( aasworld->areasettings[areas[i]].contents & AREACONTENTS_MOVER ) ) {
					continue;   // this isn't a mover area, so ignore it
				}
			}
			AAS_EnableRoutingArea( areas[i], ( blocking & ~0x1 ) | !( blocking & 1 ) );
			changed = qtrue;
		}
	}
	//
	if ( mover && !changed ) {    // map must not be compiled with MOVER flags enabled, so redo the old way
		mover = qfalse;
		goto areas_again;
	}
}
#endif // RTCW_X

