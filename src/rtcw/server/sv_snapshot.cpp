/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "server.h"

#include "rtcw_vm_args.h"


/*
=============================================================================

Delta encode a client frame onto the network channel

A normal server packet will look like:

4	sequence number (high bit set if an oversize fragment)
<optional reliable commands>
1	svc_snapshot
4	last client reliable command
4	serverTime
1	lastframe for delta compression
1	snapFlags
1	areaBytes
<areabytes>
<playerstate>
<packetentities>

=============================================================================
*/

/*
=============
SV_EmitPacketEntities

Writes a delta update of an entityState_t list to the message.
=============
*/
static void SV_EmitPacketEntities( clientSnapshot_t *from, clientSnapshot_t *to, msg_t *msg ) {
	entityState_t   *oldent, *newent;
	int oldindex, newindex;
	int oldnum, newnum;
	int from_num_entities;

	// generate the delta update
	if ( !from ) {
		from_num_entities = 0;
	} else {
		from_num_entities = from->num_entities;
	}

	newent = NULL;
	oldent = NULL;
	newindex = 0;
	oldindex = 0;
	while ( newindex < to->num_entities || oldindex < from_num_entities ) {
		if ( newindex >= to->num_entities ) {
			newnum = 9999;
		} else {
			newent = &svs.snapshotEntities[( to->first_entity + newindex ) % svs.numSnapshotEntities];
			newnum = newent->number;
		}

		if ( oldindex >= from_num_entities ) {
			oldnum = 9999;
		} else {
			oldent = &svs.snapshotEntities[( from->first_entity + oldindex ) % svs.numSnapshotEntities];
			oldnum = oldent->number;
		}

		if ( newnum == oldnum ) {
			// delta update from old position
			// because the force parm is qfalse, this will not result
			// in any bytes being emited if the entity has not changed at all
			MSG_WriteDeltaEntity( msg, oldent, newent, qfalse );
			oldindex++;
			newindex++;
			continue;
		}

		if ( newnum < oldnum ) {
			// this is a new entity, send it from the baseline
			MSG_WriteDeltaEntity( msg, &sv.svEntities[newnum].baseline, newent, qtrue );
			newindex++;
			continue;
		}

		if ( newnum > oldnum ) {
			// the old entity isn't present in the new message
			MSG_WriteDeltaEntity( msg, oldent, NULL, qtrue );
			oldindex++;
			continue;
		}
	}

	MSG_WriteBits( msg, ( MAX_GENTITIES - 1 ), GENTITYNUM_BITS );   // end of packetentities
}



/*
==================
SV_WriteSnapshotToClient
==================
*/
static void SV_WriteSnapshotToClient( client_t *client, msg_t *msg ) {
	clientSnapshot_t    *frame, *oldframe;
	int lastframe;
	int i;
	int snapFlags;

	// this is the snapshot we are creating
	frame = &client->frames[ client->netchan.outgoingSequence & PACKET_MASK ];

	// try to use a previous frame as the source for delta compressing the snapshot
	if ( client->deltaMessage <= 0 || client->state != CS_ACTIVE ) {
		// client is asking for a retransmit
		oldframe = NULL;
		lastframe = 0;
	} else if ( client->netchan.outgoingSequence - client->deltaMessage
				>= ( PACKET_BACKUP - 3 ) ) {
		// client hasn't gotten a good message through in a long time
		Com_DPrintf( "%s: Delta request from out of date packet.\n", client->name );
		oldframe = NULL;
		lastframe = 0;
	} else {
		// we have a valid snapshot to delta from
		oldframe = &client->frames[ client->deltaMessage & PACKET_MASK ];
		lastframe = client->netchan.outgoingSequence - client->deltaMessage;

		// the snapshot's entities may still have rolled off the buffer, though
		if ( oldframe->first_entity <= svs.nextSnapshotEntities - svs.numSnapshotEntities ) {
			Com_DPrintf( "%s: Delta request from out of date entities.\n", client->name );
			oldframe = NULL;
			lastframe = 0;
		}
	}

	MSG_WriteByte( msg, svc_snapshot );

	// NOTE, MRE: now sent at the start of every message from server to client
	// let the client know which reliable clientCommands we have received
	//MSG_WriteLong( msg, client->lastClientCommand );

	// send over the current server time so the client can drift
	// its view of time to try to match
	MSG_WriteLong( msg, svs.time );

	// what we are delta'ing from
	MSG_WriteByte( msg, lastframe );

	snapFlags = svs.snapFlagServerBit;
	if ( client->rateDelayed ) {
		snapFlags |= SNAPFLAG_RATE_DELAYED;
	}
	if ( client->state != CS_ACTIVE ) {
		snapFlags |= SNAPFLAG_NOT_ACTIVE;
	}

	MSG_WriteByte( msg, snapFlags );

	// send over the areabits
	MSG_WriteByte( msg, frame->areabytes );
	MSG_WriteData( msg, frame->areabits, frame->areabytes );

#if defined RTCW_ET
	{
//		int sz = msg->cursize;
//		int usz = msg->uncompsize;
#endif // RTCW_XX

	// delta encode the playerstate
	if ( oldframe ) {
		MSG_WriteDeltaPlayerstate( msg, &oldframe->ps, &frame->ps );
	} else {
		MSG_WriteDeltaPlayerstate( msg, NULL, &frame->ps );
	}

#if defined RTCW_ET
//		Com_Printf( "Playerstate delta size: %f\n", ((msg->cursize - sz) * sv_fps->integer) / 8.f );
	}
#endif // RTCW_XX

	// delta encode the entities
	SV_EmitPacketEntities( oldframe, frame, msg );

	// padding for rate debugging
	if ( sv_padPackets->integer ) {
		for ( i = 0 ; i < sv_padPackets->integer ; i++ ) {
			MSG_WriteByte( msg, svc_nop );
		}
	}
}


/*
==================
SV_UpdateServerCommandsToClient

(re)send all server commands the client hasn't acknowledged yet
==================
*/
void SV_UpdateServerCommandsToClient( client_t *client, msg_t *msg ) {
	int i;

	// write any unacknowledged serverCommands
	for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {
		MSG_WriteByte( msg, svc_serverCommand );
		MSG_WriteLong( msg, i );

#if defined RTCW_SP
		//MSG_WriteString( msg, client->reliableCommands[ i & (MAX_RELIABLE_COMMANDS-1) ] );
		MSG_WriteString( msg, SV_GetReliableCommand( client, i & ( MAX_RELIABLE_COMMANDS - 1 ) ) );
#else
		MSG_WriteString( msg, client->reliableCommands[ i & ( MAX_RELIABLE_COMMANDS - 1 ) ] );
#endif // RTCW_XX

	}
	client->reliableSent = client->reliableSequence;
}

/*
=============================================================================

Build a client snapshot structure

=============================================================================
*/

//#define	MAX_SNAPSHOT_ENTITIES	1024
#define MAX_SNAPSHOT_ENTITIES   2048

typedef struct {
	int numSnapshotEntities;
	int snapshotEntities[MAX_SNAPSHOT_ENTITIES];
} snapshotEntityNumbers_t;

/*
=======================
SV_QsortEntityNumbers
=======================
*/
#ifdef RTCW_VANILLA
static int QDECL SV_QsortEntityNumbers( const void *a, const void *b ) {
	int *ea, *eb;

	ea = (int *)a;
	eb = (int *)b;

	if ( *ea == *eb ) {
		Com_Error( ERR_DROP, "SV_QsortEntityStates: duplicated entity" );
	}

	if ( *ea < *eb ) {
		return -1;
	}

	return 1;
}
#else // RTCW_VANILLA
namespace {

// The qsort implementation may compare for same values.
// Check for duplicates after the sorting.
int SV_QsortEntityNumbers(const void* a, const void* b)
{
	const int ea = *static_cast<const int*>(a);
	const int eb = *static_cast<const int*>(b);
	return ea - eb;
}

} // namespace
#endif // RTCW_VANILLA


/*
===============
SV_AddEntToSnapshot
===============
*/

#if !defined RTCW_ET
static void SV_AddEntToSnapshot( svEntity_t *svEnt, sharedEntity_t *gEnt, snapshotEntityNumbers_t *eNums ) {
#else
static void SV_AddEntToSnapshot( sharedEntity_t* clientEnt, svEntity_t *svEnt, sharedEntity_t *gEnt, snapshotEntityNumbers_t *eNums ) {
#endif // RTCW_XX

	// if we have already added this entity to this snapshot, don't add again
	if ( svEnt->snapshotCounter == sv.snapshotCounter ) {
		return;
	}
	svEnt->snapshotCounter = sv.snapshotCounter;

	// if we are full, silently discard entities
	if ( eNums->numSnapshotEntities == MAX_SNAPSHOT_ENTITIES ) {
		return;
	}

#if defined RTCW_ET
	if ( gEnt->r.snapshotCallback ) {
		if (!VM_Call(
			gvm,
			GAME_SNAPSHOT_CALLBACK,
			rtcw::to_vm_arg(gEnt->s.number),
			rtcw::to_vm_arg(clientEnt->s.number)))
		{
			return;
		}
	}
#endif // RTCW_XX

	eNums->snapshotEntities[ eNums->numSnapshotEntities ] = gEnt->s.number;
	eNums->numSnapshotEntities++;
}

/*
===============
SV_AddEntitiesVisibleFromPoint
===============
*/

#if !defined RTCW_ET
static void SV_AddEntitiesVisibleFromPoint( vec3_t origin, clientSnapshot_t *frame,
//									snapshotEntityNumbers_t *eNums, qboolean portal, clientSnapshot_t *oldframe, qboolean localClient ) {
//									snapshotEntityNumbers_t *eNums, qboolean portal ) {
											snapshotEntityNumbers_t *eNums, qboolean portal, qboolean localClient  ) {
#else
static void SV_AddEntitiesVisibleFromPoint( vec3_t origin, clientSnapshot_t *frame,
//									snapshotEntityNumbers_t *eNums, qboolean portal, clientSnapshot_t *oldframe, qboolean localClient ) {
//									snapshotEntityNumbers_t *eNums, qboolean portal ) {
											snapshotEntityNumbers_t *eNums /*, qboolean portal, qboolean localClient*/  ) {
#endif // RTCW_XX

	int e, i;
	sharedEntity_t *ent, *playerEnt;
	svEntity_t  *svEnt;
	int l;
	int clientarea, clientcluster;
	int leafnum;
	int c_fullsend;
	byte    *clientpvs;
	byte    *bitvector;

	// during an error shutdown message we may need to transmit
	// the shutdown message after the server has shutdown, so
	// specfically check for it
	if ( !sv.state ) {
		return;
	}

	leafnum = CM_PointLeafnum( origin );
	clientarea = CM_LeafArea( leafnum );
	clientcluster = CM_LeafCluster( leafnum );

	// calculate the visible areas
	frame->areabytes = CM_WriteAreaBits( frame->areabits, clientarea );

	clientpvs = CM_ClusterPVS( clientcluster );

	c_fullsend = 0;

	playerEnt = SV_GentityNum( frame->ps.clientNum );

#if defined RTCW_ET
	if ( playerEnt->r.svFlags & SVF_SELF_PORTAL ) {
		SV_AddEntitiesVisibleFromPoint( playerEnt->s.origin2, frame, eNums );
	}
#endif // RTCW_XX

	for ( e = 0 ; e < sv.num_entities ; e++ ) {
		ent = SV_GentityNum( e );

		// never send entities that aren't linked in
		if ( !ent->r.linked ) {
			continue;
		}

		if ( ent->s.number != e ) {
			Com_DPrintf( "FIXING ENT->S.NUMBER!!!\n" );
			ent->s.number = e;
		}

		// entities can be flagged to explicitly not be sent to the client
		if ( ent->r.svFlags & SVF_NOCLIENT ) {
			continue;
		}

		// entities can be flagged to be sent to only one client
		if ( ent->r.svFlags & SVF_SINGLECLIENT ) {
			if ( ent->r.singleClient != frame->ps.clientNum ) {
				continue;
			}
		}
		// entities can be flagged to be sent to everyone but one client
		if ( ent->r.svFlags & SVF_NOTSINGLECLIENT ) {
			if ( ent->r.singleClient == frame->ps.clientNum ) {
				continue;
			}
		}

		svEnt = SV_SvEntityForGentity( ent );

		// don't double add an entity through portals
		if ( svEnt->snapshotCounter == sv.snapshotCounter ) {
			continue;
		}

#if !defined RTCW_ET
		// if this client is viewing from a camera, only add ents visible from portal ents
		if ( ( playerEnt->s.eFlags & EF_VIEWING_CAMERA ) && !portal ) {
			if ( ent->r.svFlags & SVF_PORTAL ) {
				SV_AddEntToSnapshot( svEnt, ent, eNums );
//				SV_AddEntitiesVisibleFromPoint( ent->s.origin2, frame, eNums, qtrue, oldframe, localClient );
				SV_AddEntitiesVisibleFromPoint( ent->s.origin2, frame, eNums, qtrue, localClient );
			}
			continue;
		}
#endif // RTCW_XX

		// broadcast entities are always sent
		if ( ent->r.svFlags & SVF_BROADCAST ) {

#if !defined RTCW_ET
			SV_AddEntToSnapshot( svEnt, ent, eNums );
#else
			SV_AddEntToSnapshot( playerEnt, svEnt, ent, eNums );
			continue;
		}

		bitvector = clientpvs;

		// Gordon: just check origin for being in pvs, ignore bmodel extents
		if ( ent->r.svFlags & SVF_IGNOREBMODELEXTENTS ) {
			if ( bitvector[svEnt->originCluster >> 3] & ( 1 << ( svEnt->originCluster & 7 ) ) ) {
				SV_AddEntToSnapshot( playerEnt, svEnt, ent, eNums );
			}
#endif // RTCW_XX

			continue;
		}

		// ignore if not touching a PV leaf
		// check area
		if ( !CM_AreasConnected( clientarea, svEnt->areanum ) ) {
			// doors can legally straddle two areas, so
			// we may need to check another one
			if ( !CM_AreasConnected( clientarea, svEnt->areanum2 ) ) {

#if !defined RTCW_ET
				goto notVisible;    // blocked by a door
#else
				continue;
#endif // RTCW_XX

			}
		}

#if !defined RTCW_ET
		bitvector = clientpvs;
#endif // RTCW_XX

		// check individual leafs
		if ( !svEnt->numClusters ) {

#if !defined RTCW_ET
			goto notVisible;
#else
			continue;
#endif // RTCW_XX

		}
		l = 0;
		for ( i = 0 ; i < svEnt->numClusters ; i++ ) {
			l = svEnt->clusternums[i];
			if ( bitvector[l >> 3] & ( 1 << ( l & 7 ) ) ) {
				break;
			}
		}

		// if we haven't found it to be visible,
		// check overflow clusters that coudln't be stored
		if ( i == svEnt->numClusters ) {
			if ( svEnt->lastCluster ) {
				for ( ; l <= svEnt->lastCluster ; l++ ) {
					if ( bitvector[l >> 3] & ( 1 << ( l & 7 ) ) ) {
						break;
					}
				}
				if ( l == svEnt->lastCluster ) {

#if !defined RTCW_ET
					goto notVisible;    // not visible
#else
					continue;
#endif // RTCW_XX

				}
			} else {

#if !defined RTCW_ET
				goto notVisible;
#else
				continue;
#endif // RTCW_XX

			}
		}

		//----(SA) added "visibility dummies"
		if ( ent->r.svFlags & SVF_VISDUMMY ) {
			sharedEntity_t *ment = 0;

			//find master;
			ment = SV_GentityNum( ent->s.otherEntityNum );

			if ( ment ) {
				svEntity_t *master = 0;
				master = SV_SvEntityForGentity( ment );

				if ( master->snapshotCounter == sv.snapshotCounter || !ment->r.linked ) {

#if !defined RTCW_ET
					goto notVisible;
					//continue;
#else
					continue;
#endif // RTCW_XX

				}

#if !defined RTCW_ET
				SV_AddEntToSnapshot( master, ment, eNums );
#else
				SV_AddEntToSnapshot( playerEnt, master, ment, eNums );
#endif // RTCW_XX

			}

#if !defined RTCW_ET
			goto notVisible;
			//continue;	// master needs to be added, but not this dummy ent
#else
			continue;   // master needs to be added, but not this dummy ent
#endif // RTCW_XX

		}
		//----(SA) end
		else if ( ent->r.svFlags & SVF_VISDUMMY_MULTIPLE ) {
			{
				int h;
				sharedEntity_t *ment = 0;
				svEntity_t *master = 0;

				for ( h = 0; h < sv.num_entities; h++ )
				{
					ment = SV_GentityNum( h );

					if ( ment == ent ) {
						continue;
					}

					if ( ment ) {
						master = SV_SvEntityForGentity( ment );
					} else {
						continue;
					}

					if ( !( ment->r.linked ) ) {
						continue;
					}

					if ( ment->s.number != h ) {
						Com_DPrintf( "FIXING vis dummy multiple ment->S.NUMBER!!!\n" );
						ment->s.number = h;
					}

					if ( ment->r.svFlags & SVF_NOCLIENT ) {
						continue;
					}

					if ( master->snapshotCounter == sv.snapshotCounter ) {
						continue;
					}

					if ( ment->s.otherEntityNum == ent->s.number ) {

#if !defined RTCW_ET
						SV_AddEntToSnapshot( master, ment, eNums );
#else
						SV_AddEntToSnapshot( playerEnt, master, ment, eNums );
#endif // RTCW_XX

					}
				}

#if !defined RTCW_ET
				goto notVisible;
#else
				continue;
#endif // RTCW_XX

			}
		}

		// add it

#if !defined RTCW_ET
		SV_AddEntToSnapshot( svEnt, ent, eNums );
#else
		SV_AddEntToSnapshot( playerEnt, svEnt, ent, eNums );
#endif // RTCW_XX

		// if its a portal entity, add everything visible from its camera position
		if ( ent->r.svFlags & SVF_PORTAL ) {
//			SV_AddEntitiesVisibleFromPoint( ent->s.origin2, frame, eNums, qtrue, oldframe, localClient );

#if !defined RTCW_ET
			SV_AddEntitiesVisibleFromPoint( ent->s.origin2, frame, eNums, qtrue, localClient );
#else
			SV_AddEntitiesVisibleFromPoint( ent->s.origin2, frame, eNums /*, qtrue, localClient*/ );
#endif // RTCW_XX

		}

		continue;

#if !defined RTCW_ET
notVisible:

		// Ridah, if this entity has changed events, then send it regardless of whether we can see it or not
		// DHM - Nerve :: not in multiplayer please
		if ( sv_gametype->integer == GT_SINGLE_PLAYER && localClient ) {
			if ( ent->r.eventTime == svs.time ) {
				ent->s.eFlags |= EF_NODRAW;     // don't draw, just process event
				SV_AddEntToSnapshot( svEnt, ent, eNums );
			} else if ( ent->s.eType == ET_PLAYER ) {
				// keep players around if they are alive and active (so sounds dont get messed up)
				if ( !( ent->s.eFlags & EF_DEAD ) ) {
					ent->s.eFlags |= EF_NODRAW;     // don't draw, just process events and sounds
					SV_AddEntToSnapshot( svEnt, ent, eNums );
				}
			}
		}
#endif // RTCW_XX

	}
}

/*
=============
SV_BuildClientSnapshot

Decides which entities are going to be visible to the client, and
copies off the playerstate and areabits.

This properly handles multiple recursive portals, but the render
currently doesn't.

For viewing through other player's eyes, clent can be something other than client->gentity
=============
*/
static void SV_BuildClientSnapshot( client_t *client ) {
	vec3_t org;
//	clientSnapshot_t			*frame, *oldframe;
	clientSnapshot_t            *frame;
	snapshotEntityNumbers_t entityNumbers;
	int i;
	sharedEntity_t              *ent;
	entityState_t               *state;
	svEntity_t                  *svEnt;
	sharedEntity_t              *clent;
	int clientNum;
	playerState_t               *ps;

	// bump the counter used to prevent double adding
	sv.snapshotCounter++;

	// this is the frame we are creating
	frame = &client->frames[ client->netchan.outgoingSequence & PACKET_MASK ];

#if defined RTCW_SP
//	// try to use a previous frame as the source for delta compressing the snapshot
//	if ( client->deltaMessage <= 0 || client->state != CS_ACTIVE ) {
//		// client is asking for a retransmit
//		oldframe = NULL;
//	} else if ( client->netchan.outgoingSequence - client->deltaMessage
//		>= (PACKET_BACKUP - 3) ) {
//		// client hasn't gotten a good message through in a long time
//		Com_DPrintf ("%s: Delta request from out of date packet.\n", client->name);
//		oldframe = NULL;
//	} else {
//		// we have a valid snapshot to delta from
//		oldframe = &client->frames[ client->deltaMessage & PACKET_MASK ];
//
//		// the snapshot's entities may still have rolled off the buffer, though
//		if ( oldframe->first_entity <= svs.nextSnapshotEntities - svs.numSnapshotEntities ) {
//			Com_DPrintf ("%s: Delta request from out of date entities.\n", client->name);
//			oldframe = NULL;
//		}
//	}
#endif // RTCW_XX

	// clear everything in this snapshot
	entityNumbers.numSnapshotEntities = 0;
	memset( frame->areabits, 0, sizeof( frame->areabits ) );

#if !defined RTCW_SP
	// show_bug.cgi?id=62
	frame->num_entities = 0;
#endif // RTCW_XX

	clent = client->gentity;
	if ( !clent || client->state == CS_ZOMBIE ) {
		return;
	}

	// grab the current playerState_t
	ps = SV_GameClientNum( client - svs.clients );
	frame->ps = *ps;

	// never send client's own entity, because it can
	// be regenerated from the playerstate
	clientNum = frame->ps.clientNum;
	if ( clientNum < 0 || clientNum >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	svEnt = &sv.svEntities[ clientNum ];

	svEnt->snapshotCounter = sv.snapshotCounter;

#if !defined RTCW_ET
	// find the client's viewpoint
	VectorCopy( ps->origin, org );
#else
	if ( clent->r.svFlags & SVF_SELF_PORTAL_EXCLUSIVE ) {
		// find the client's viewpoint
		VectorCopy( clent->s.origin2, org );
	} else {
		VectorCopy( ps->origin, org );
	}
#endif // RTCW_XX

	org[2] += ps->viewheight;

//----(SA)	added for 'lean'
	// need to account for lean, so areaportal doors draw properly
	if ( frame->ps.leanf != 0 ) {
		vec3_t right, v3ViewAngles;
		VectorCopy( ps->viewangles, v3ViewAngles );
		v3ViewAngles[2] += frame->ps.leanf / 2.0f;
		AngleVectors( v3ViewAngles, NULL, right, NULL );
		VectorMA( org, frame->ps.leanf, right, org );
	}
//----(SA)	end

	// add all the entities directly visible to the eye, which
	// may include portal entities that merge other viewpoints

#if !defined RTCW_ET
	SV_AddEntitiesVisibleFromPoint( org, frame, &entityNumbers, qfalse, client->netchan.remoteAddress.type == NA_LOOPBACK );
#else
	SV_AddEntitiesVisibleFromPoint( org, frame, &entityNumbers /*, qfalse, client->netchan.remoteAddress.type == NA_LOOPBACK*/ );
#endif // RTCW_XX

#if defined RTCW_SP
//	SV_AddEntitiesVisibleFromPoint( org, frame, &entityNumbers, qfalse, oldframe, client->netchan.remoteAddress.type == NA_LOOPBACK );
#endif // RTCW_XX

	// if there were portals visible, there may be out of order entities
	// in the list which will need to be resorted for the delta compression
	// to work correctly.  This also catches the error condition
	// of an entity being included twice.
	qsort( entityNumbers.snapshotEntities, entityNumbers.numSnapshotEntities,
		   sizeof( entityNumbers.snapshotEntities[0] ), SV_QsortEntityNumbers );

// { RTCW
// The qsort implementation may compare for same values.
// Check for duplicates after the sorting.
	for (i = 1; i < entityNumbers.numSnapshotEntities; ++i)
	{
		if (entityNumbers.snapshotEntities[i] == entityNumbers.snapshotEntities[i - 1])
		{
			Com_Error(ERR_DROP, "SV_QsortEntityStates: duplicated entity");
		}
	}
// } RTCW

	// now that all viewpoint's areabits have been OR'd together, invert
	// all of them to make it a mask vector, which is what the renderer wants
	for ( i = 0 ; i < MAX_MAP_AREA_BYTES / 4 ; i++ ) {
		( (int *)frame->areabits )[i] = ( (int *)frame->areabits )[i] ^ -1;
	}

	// copy the entity states out
	frame->num_entities = 0;
	frame->first_entity = svs.nextSnapshotEntities;
	for ( i = 0 ; i < entityNumbers.numSnapshotEntities ; i++ ) {
		ent = SV_GentityNum( entityNumbers.snapshotEntities[i] );
		state = &svs.snapshotEntities[svs.nextSnapshotEntities % svs.numSnapshotEntities];
		*state = ent->s;
		svs.nextSnapshotEntities++;
		// this should never hit, map should always be restarted first in SV_Frame
		if ( svs.nextSnapshotEntities >= 0x7FFFFFFE ) {
			Com_Error( ERR_FATAL, "svs.nextSnapshotEntities wrapped" );
		}
		frame->num_entities++;
	}
}


#if defined RTCW_SP
/*
====================
SV_RateMsec

Return the number of msec a given size message is supposed
to take to clear, based on the current rate
====================
*/
#else
/*
====================
SV_RateMsec

Return the number of msec a given size message is supposed
to take to clear, based on the current rate
TTimo - use sv_maxRate or sv_dl_maxRate depending on regular or downloading client
====================
*/
#endif // RTCW_XX

#define HEADER_RATE_BYTES   48      // include our header, IP header, and some overhead
static int SV_RateMsec( client_t *client, int messageSize ) {
	int rate;
	int rateMsec;

#if !defined RTCW_SP
	int maxRate;
#endif // RTCW_XX

	// individual messages will never be larger than fragment size
	if ( messageSize > 1500 ) {
		messageSize = 1500;
	}

#if !defined RTCW_SP
	// low watermark for sv_maxRate, never 0 < sv_maxRate < 1000 (0 is no limitation)
	if ( sv_maxRate->integer && sv_maxRate->integer < 1000 ) {
		Cvar_Set( "sv_MaxRate", "1000" );
	}
#endif // RTCW_XX

	rate = client->rate;

#if defined RTCW_SP
	if ( sv_maxRate->integer ) {
		if ( sv_maxRate->integer < 1000 ) {
			Cvar_Set( "sv_MaxRate", "1000" );
		}
		if ( sv_maxRate->integer < rate ) {
			rate = sv_maxRate->integer;
#else
	// work on the appropriate max rate (client or download)
	if ( !*client->downloadName ) {
		maxRate = sv_maxRate->integer;
	} else
	{
		maxRate = sv_dl_maxRate->integer;
	}
	if ( maxRate ) {
		if ( maxRate < rate ) {
			rate = maxRate;
#endif // RTCW_XX

		}
	}
	rateMsec = ( messageSize + HEADER_RATE_BYTES ) * 1000 / rate;

	return rateMsec;
}

/*
=======================
SV_SendMessageToClient

Called by SV_SendClientSnapshot and SV_SendClientGameState
=======================
*/
void SV_SendMessageToClient( msg_t *msg, client_t *client ) {
	int rateMsec;

	// record information about the message
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSize = msg->cursize;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSent = svs.time;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageAcked = -1;

	// send the datagram

#if defined RTCW_SP
	SV_Netchan_Transmit( client, msg ); //msg->cursize, msg->data );
#else
	SV_Netchan_Transmit( client, msg );
#endif // RTCW_XX

	// set nextSnapshotTime based on rate and requested number of updates

	// local clients get snapshots every frame

#if defined RTCW_SP
	if ( client->netchan.remoteAddress.type == NA_LOOPBACK || Sys_IsLANAddress( client->netchan.remoteAddress ) ) {
#else
	// TTimo - show_bug.cgi?id=491
	// added sv_lanForceRate check
	if ( client->netchan.remoteAddress.type == NA_LOOPBACK || ( sv_lanForceRate->integer && Sys_IsLANAddress( client->netchan.remoteAddress ) ) ) {
#endif // RTCW_XX

		client->nextSnapshotTime = svs.time - 1;
		return;
	}

	// normal rate / snapshotMsec calculation
	rateMsec = SV_RateMsec( client, msg->cursize );

#if defined RTCW_SP
	if ( rateMsec < client->snapshotMsec ) {
#else
	// TTimo - during a download, ignore the snapshotMsec
	// the update server on steroids, with this disabled and sv_fps 60, the download can reach 30 kb/s
	// on a regular server, we will still top at 20 kb/s because of sv_fps 20
	if ( !*client->downloadName && rateMsec < client->snapshotMsec ) {
#endif // RTCW_XX

		// never send more packets than this, no matter what the rate is at
		rateMsec = client->snapshotMsec;
		client->rateDelayed = qfalse;
	} else {
		client->rateDelayed = qtrue;
	}

	client->nextSnapshotTime = svs.time + rateMsec;

	// don't pile up empty snapshots while connecting
	if ( client->state != CS_ACTIVE ) {
		// a gigantic connection message may have already put the nextSnapshotTime
		// more than a second away, so don't shorten it
		// do shorten if client is downloading
		if ( !*client->downloadName && client->nextSnapshotTime < svs.time + 1000 ) {
			client->nextSnapshotTime = svs.time + 1000;
		}
	}
}

#if defined RTCW_ET
//bani
/*
=======================
SV_SendClientIdle

There is no need to send full snapshots to clients who are loading a map.
So we send them "idle" packets with the bare minimum required to keep them on the server.

=======================
*/
void SV_SendClientIdle( client_t *client ) {
	byte msg_buf[MAX_MSGLEN];
	msg_t msg;

	MSG_Init( &msg, msg_buf, sizeof( msg_buf ) );
	msg.allowoverflow = qtrue;

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand );

	// (re)send any reliable server commands
	SV_UpdateServerCommandsToClient( client, &msg );

	// send over all the relevant entityState_t
	// and the playerState_t
//	SV_WriteSnapshotToClient( client, &msg );

	// Add any download data if the client is downloading
	SV_WriteDownloadToClient( client, &msg );

	// check for overflow
	if ( msg.overflowed ) {
		Com_Printf( "WARNING: msg overflowed for %s\n", client->name );
		MSG_Clear( &msg );

		SV_DropClient( client, "Msg overflowed" );
		return;
	}

	SV_SendMessageToClient( &msg, client );

	sv.bpsTotalBytes += msg.cursize;            // NERVE - SMF - net debugging
	sv.ubpsTotalBytes += msg.uncompsize / 8;    // NERVE - SMF - net debugging
}
#endif // RTCW_XX

#if !defined RTCW_ET
/*
=======================
SV_SendClientSnapshot

Also called by SV_FinalMessage

=======================
*/
#else
/*
=======================
SV_SendClientSnapshot

Also called by SV_FinalCommand

=======================
*/
#endif // RTCW_XX

void SV_SendClientSnapshot( client_t *client ) {
	byte msg_buf[MAX_MSGLEN];
	msg_t msg;

#if defined RTCW_SP
	//RF, AI don't need snapshots built
	if ( client->gentity && client->gentity->r.svFlags & SVF_CASTAI ) {
		return;
	}
#endif // RTCW_XX

#if defined RTCW_ET
	//bani
	if ( client->state < CS_ACTIVE ) {
		// bani - #760 - zombie clients need full snaps so they can still process reliable commands
		// (eg so they can pick up the disconnect reason)
		if ( client->state != CS_ZOMBIE ) {
			SV_SendClientIdle( client );
			return;
		}
	}
#endif // RTCW_XX

	// build the snapshot
	SV_BuildClientSnapshot( client );

	// bots need to have their snapshots build, but
	// the query them directly without needing to be sent
	if ( client->gentity && client->gentity->r.svFlags & SVF_BOT ) {
		return;
	}

	MSG_Init( &msg, msg_buf, sizeof( msg_buf ) );
	msg.allowoverflow = qtrue;

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand );

	// (re)send any reliable server commands
	SV_UpdateServerCommandsToClient( client, &msg );

	// send over all the relevant entityState_t
	// and the playerState_t
	SV_WriteSnapshotToClient( client, &msg );

	// Add any download data if the client is downloading
	SV_WriteDownloadToClient( client, &msg );

	// check for overflow
	if ( msg.overflowed ) {
		Com_Printf( "WARNING: msg overflowed for %s\n", client->name );
		MSG_Clear( &msg );

#if defined RTCW_ET
		SV_DropClient( client, "Msg overflowed" );
		return;
#endif // RTCW_XX

	}

	SV_SendMessageToClient( &msg, client );

#if !defined RTCW_SP
	sv.bpsTotalBytes += msg.cursize;            // NERVE - SMF - net debugging
	sv.ubpsTotalBytes += msg.uncompsize / 8;    // NERVE - SMF - net debugging
#endif // RTCW_XX

}


/*
=======================
SV_SendClientMessages
=======================
*/

void SV_SendClientMessages( void ) {
	int i;
	client_t    *c;

#if !defined RTCW_SP
	int numclients = 0;         // NERVE - SMF - net debugging

	sv.bpsTotalBytes = 0;       // NERVE - SMF - net debugging
	sv.ubpsTotalBytes = 0;      // NERVE - SMF - net debugging
#endif // RTCW_XX

#if defined RTCW_ET
	// Gordon: update any changed configstrings from this frame
	SV_UpdateConfigStrings();
#endif // RTCW_XX

	// send a message to each connected client

#if !defined RTCW_ET
	for ( i = 0, c = svs.clients ; i < sv_maxclients->integer ; i++, c++ ) {
#else
	for ( i = 0; i < sv_maxclients->integer; i++ ) {
#endif // RTCW_XX

#if defined RTCW_ET
		c = &svs.clients[i];
#endif // RTCW_XX

#if !defined RTCW_ET
		if ( !c->state ) {
#else
		// rain - changed <= CS_ZOMBIE to < CS_ZOMBIE so that the
		// disconnect reason is properly sent in the network stream
		if ( c->state < CS_ZOMBIE ) {
#endif // RTCW_XX

			continue;       // not connected
		}

#if defined RTCW_ET
		// RF, needed to insert this otherwise bots would cause error drops in sv_net_chan.c:
		// --> "netchan queue is not properly initialized in SV_Netchan_TransmitNextFragment\n"
		if ( c->gentity && c->gentity->r.svFlags & SVF_BOT ) {
			continue;
		}
#endif // RTCW_XX

		if ( svs.time < c->nextSnapshotTime ) {
			continue;       // not time yet
		}

#if !defined RTCW_SP
		numclients++;       // NERVE - SMF - net debugging
#endif // RTCW_XX

		// send additional message fragments if the last message
		// was too large to send at once
		if ( c->netchan.unsentFragments ) {
			c->nextSnapshotTime = svs.time +
								  SV_RateMsec( c, c->netchan.unsentLength - c->netchan.unsentFragmentStart );
#if defined RTCW_SP
			SV_Netchan_TransmitNextFragment( &c->netchan );
#else
			SV_Netchan_TransmitNextFragment( c );
#endif // RTCW_XX

			continue;
		}

		// generate and send a new message
		SV_SendClientSnapshot( c );
	}

#if !defined RTCW_SP
	// NERVE - SMF - net debugging
	if ( sv_showAverageBPS->integer && numclients > 0 ) {
		float ave = 0, uave = 0;

		for ( i = 0; i < MAX_BPS_WINDOW - 1; i++ ) {
			sv.bpsWindow[i] = sv.bpsWindow[i + 1];
			ave += sv.bpsWindow[i];

			sv.ubpsWindow[i] = sv.ubpsWindow[i + 1];
			uave += sv.ubpsWindow[i];
		}

		sv.bpsWindow[MAX_BPS_WINDOW - 1] = sv.bpsTotalBytes;
		ave += sv.bpsTotalBytes;

		sv.ubpsWindow[MAX_BPS_WINDOW - 1] = sv.ubpsTotalBytes;
		uave += sv.ubpsTotalBytes;

		if ( sv.bpsTotalBytes >= sv.bpsMaxBytes ) {
			sv.bpsMaxBytes = sv.bpsTotalBytes;
		}

		if ( sv.ubpsTotalBytes >= sv.ubpsMaxBytes ) {
			sv.ubpsMaxBytes = sv.ubpsTotalBytes;
		}

		sv.bpsWindowSteps++;

		if ( sv.bpsWindowSteps >= MAX_BPS_WINDOW ) {
			float comp_ratio;

			sv.bpsWindowSteps = 0;

			ave = ( ave / (float)MAX_BPS_WINDOW );
			uave = ( uave / (float)MAX_BPS_WINDOW );

			comp_ratio = ( 1 - ave / uave ) * 100.f;
			sv.ucompAve += comp_ratio;
			sv.ucompNum++;

			Com_DPrintf( "bpspc(%2.0f) bps(%2.0f) pk(%i) ubps(%2.0f) upk(%i) cr(%2.2f) acr(%2.2f)\n",
						 ave / (float)numclients, ave, sv.bpsMaxBytes, uave, sv.ubpsMaxBytes, comp_ratio, sv.ucompAve / sv.ucompNum );
		}
	}
	// -NERVE - SMF
#endif // RTCW_XX

}
