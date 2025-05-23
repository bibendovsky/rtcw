/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "q_shared.h"
#include "qcommon.h"
#include "rtcw_endian.h"

/*

packet header
-------------
4	outgoing sequence.  high bit will be set if this is a fragmented message
[2	qport (only for client to server)]
[2	fragment start byte]
[2	fragment length. if < FRAGMENT_SIZE, this is the last fragment]

if the sequence number is -1, the packet should be handled as an out-of-band
message instead of as part of a netcon.

All fragments will have the same sequence numbers.

The qport field is a workaround for bad address translating routers that
sometimes remap the client's source port on a packet during gameplay.

If the base part of the net address matches and the qport matches, then the
channel matches even if the IP port differs.  The IP port should be updated
to the new value before sending out any replies.

*/


#define MAX_PACKETLEN           1400        // max size of a network packet

#define FRAGMENT_SIZE           ( MAX_PACKETLEN - 100 )
#define PACKET_HEADER           10          // two ints and a short

#define FRAGMENT_BIT    ( 1 << 31 )

cvar_t      *showpackets;
cvar_t      *showdrop;
cvar_t      *qport;

static const char *netsrcString[2] = {
	"client",
	"server"
};

/*
===============
Netchan_Init

===============
*/
void Netchan_Init( int port ) {
	port &= 0xffff;
	showpackets = Cvar_Get( "showpackets", "0", CVAR_TEMP );
	showdrop = Cvar_Get( "showdrop", "0", CVAR_TEMP );
	qport = Cvar_Get( "net_qport", va( "%i", port ), CVAR_INIT );
}

/*
==============
Netchan_Setup

called to open a channel to a remote system
==============
*/
void Netchan_Setup( netsrc_t sock, netchan_t *chan, netadr_t adr, int qport ) {

#if !defined RTCW_ET
	memset( chan, 0, sizeof( *chan ) );
#else
	Com_Memset( chan, 0, sizeof( *chan ) );
#endif // RTCW_XX

	chan->sock = sock;
	chan->remoteAddress = adr;
	chan->qport = qport;
	chan->incomingSequence = 0;
	chan->outgoingSequence = 1;
}

#if defined RTCW_SP && DO_NET_ENCODE
/*
==============
Netchan_ScramblePacket

A probably futile attempt to make proxy hacking somewhat
more difficult.
==============
*/
#define SCRAMBLE_START  6
static void Netchan_ScramblePacket( msg_t *buf ) {
	unsigned seed;
	int i, j, c, mask, temp;
	int seq[MAX_PACKETLEN];

	seed = ( bbi::Endian::le ( *(unsigned *)buf->data ) * 3 ) ^ ( buf->cursize * 123 );
	c = buf->cursize;
	if ( c <= SCRAMBLE_START ) {
		return;
	}
	if ( c > MAX_PACKETLEN ) {
		Com_Error( ERR_DROP, "MAX_PACKETLEN" );
	}

	// generate a sequence of "random" numbers
	for ( i = 0 ; i < c ; i++ ) {
		seed = ( 119 * seed + 1 );
		seq[i] = seed;
	}

	// transpose each character
	for ( mask = 1 ; mask < c - SCRAMBLE_START ; mask = ( mask << 1 ) + 1 ) {
	}
	mask >>= 1;
	for ( i = SCRAMBLE_START ; i < c ; i++ ) {
		j = SCRAMBLE_START + ( seq[i] & mask );
		temp = buf->data[j];
		buf->data[j] = buf->data[i];
		buf->data[i] = temp;
	}

	// byte xor the data after the header
	for ( i = SCRAMBLE_START ; i < c ; i++ ) {
		buf->data[i] ^= seq[i];
	}
}

static void Netchan_UnScramblePacket( msg_t *buf ) {
	unsigned seed;
	int i, j, c, mask, temp;
	int seq[MAX_PACKETLEN];

	seed = ( bbi::Endian::le ( *(unsigned *)buf->data ) * 3 ) ^ ( buf->cursize * 123 );
	c = buf->cursize;
	if ( c <= SCRAMBLE_START ) {
		return;
	}
	if ( c > MAX_PACKETLEN ) {
		Com_Error( ERR_DROP, "MAX_PACKETLEN" );
	}

	// generate a sequence of "random" numbers
	for ( i = 0 ; i < c ; i++ ) {
		seed = ( 119 * seed + 1 );
		seq[i] = seed;
	}

	// byte xor the data after the header
	for ( i = SCRAMBLE_START ; i < c ; i++ ) {
		buf->data[i] ^= seq[i];
	}

	// transpose each character in reverse order
	for ( mask = 1 ; mask < c - SCRAMBLE_START ; mask = ( mask << 1 ) + 1 ) {
	}
	mask >>= 1;
	for ( i = c - 1 ; i >= SCRAMBLE_START ; i-- ) {
		j = SCRAMBLE_START + ( seq[i] & mask );
		temp = buf->data[j];
		buf->data[j] = buf->data[i];
		buf->data[i] = temp;
	}
}
#endif // RTCW_XX

/*
=================
Netchan_TransmitNextFragment

Send one fragment of the current message
=================
*/
void Netchan_TransmitNextFragment( netchan_t *chan ) {
	msg_t send;
	byte send_buf[MAX_PACKETLEN];
	int fragmentLength;

	// write the packet header
	MSG_InitOOB( &send, send_buf, sizeof( send_buf ) );                // <-- only do the oob here

	MSG_WriteLong( &send, chan->outgoingSequence | FRAGMENT_BIT );

	// send the qport if we are a client
	if ( chan->sock == NS_CLIENT ) {
		MSG_WriteShort( &send, qport->integer );
	}

	// copy the reliable message to the packet first
	fragmentLength = FRAGMENT_SIZE;
	if ( chan->unsentFragmentStart  + fragmentLength > chan->unsentLength ) {
		fragmentLength = chan->unsentLength - chan->unsentFragmentStart;
	}

	MSG_WriteShort( &send, chan->unsentFragmentStart );
	MSG_WriteShort( &send, fragmentLength );
	MSG_WriteData( &send, chan->unsentBuffer + chan->unsentFragmentStart, fragmentLength );

#if defined RTCW_SP
	// XOR scramble all data in the packet after the header
//	Netchan_ScramblePacket( &send );
#endif // RTCW_XX

	// send the datagram
	NET_SendPacket( chan->sock, send.cursize, send.data, chan->remoteAddress );

	if ( showpackets->integer ) {
		Com_Printf( "%s send %4i : s=%i fragment=%i,%i\n"
					, netsrcString[ chan->sock ]
					, send.cursize

#if defined RTCW_SP
					, chan->outgoingSequence - 1
#else
					, chan->outgoingSequence
#endif // RTCW_XX

					, chan->unsentFragmentStart, fragmentLength );
	}

	chan->unsentFragmentStart += fragmentLength;

	// this exit condition is a little tricky, because a packet
	// that is exactly the fragment length still needs to send
	// a second packet of zero length so that the other side
	// can tell there aren't more to follow
	if ( chan->unsentFragmentStart == chan->unsentLength && fragmentLength != FRAGMENT_SIZE ) {
		chan->outgoingSequence++;
		chan->unsentFragments = qfalse;
	}
}


/*
===============
Netchan_Transmit

Sends a message to a connection, fragmenting if necessary
A 0 length will still generate a packet.
================
*/
void Netchan_Transmit( netchan_t *chan, int length, const byte *data ) {
	msg_t send;
	byte send_buf[MAX_PACKETLEN];

	if ( length > MAX_MSGLEN ) {
		Com_Error( ERR_DROP, "Netchan_Transmit: length = %i", length );
	}
	chan->unsentFragmentStart = 0;

	// fragment large reliable messages
	if ( length >= FRAGMENT_SIZE ) {
		chan->unsentFragments = qtrue;
		chan->unsentLength = length;
		Com_Memcpy( chan->unsentBuffer, data, length );

		// only send the first fragment now
		Netchan_TransmitNextFragment( chan );

		return;
	}

	// write the packet header
	MSG_InitOOB( &send, send_buf, sizeof( send_buf ) );

	MSG_WriteLong( &send, chan->outgoingSequence );
	chan->outgoingSequence++;

	// send the qport if we are a client
	if ( chan->sock == NS_CLIENT ) {
		MSG_WriteShort( &send, qport->integer );
	}

	MSG_WriteData( &send, data, length );

#if defined RTCW_SP
	// XOR scramble all data in the packet after the header
//	Netchan_ScramblePacket( &send );
#endif // RTCW_XX

	// send the datagram
	NET_SendPacket( chan->sock, send.cursize, send.data, chan->remoteAddress );

	if ( showpackets->integer ) {
		Com_Printf( "%s send %4i : s=%i ack=%i\n"
					, netsrcString[ chan->sock ]
					, send.cursize
					, chan->outgoingSequence - 1
					, chan->incomingSequence );
	}
}

/*
=================
Netchan_Process

Returns qfalse if the message should not be processed due to being
out of order or a fragment.

Msg must be large enough to hold MAX_MSGLEN, because if this is the
final fragment of a multi-part message, the entire thing will be
copied out.
=================
*/
qboolean Netchan_Process( netchan_t *chan, msg_t *msg ) {
	int sequence;
	int qport;
	int fragmentStart, fragmentLength;
	qboolean fragmented;

#if defined RTCW_SP
	// XOR unscramble all data in the packet after the header
//	Netchan_UnScramblePacket( msg );
#endif // RTCW_XX

	// get sequence numbers
	MSG_BeginReadingOOB( msg );
	sequence = MSG_ReadLong( msg );

	// check for fragment information
	if ( sequence & FRAGMENT_BIT ) {
		sequence &= ~FRAGMENT_BIT;
		fragmented = qtrue;
	} else {
		fragmented = qfalse;
	}

	// read the qport if we are a server
	if ( chan->sock == NS_SERVER ) {
		qport = MSG_ReadShort( msg );
	}

	// read the fragment information
	if ( fragmented ) {
		fragmentStart = MSG_ReadShort( msg );
		fragmentLength = MSG_ReadShort( msg );
	} else {
		fragmentStart = 0;      // stop warning message
		fragmentLength = 0;
	}

	if ( showpackets->integer ) {
		if ( fragmented ) {
			Com_Printf( "%s recv %4i : s=%i fragment=%i,%i\n"
						, netsrcString[ chan->sock ]
						, msg->cursize
						, sequence
						, fragmentStart, fragmentLength );
		} else {
			Com_Printf( "%s recv %4i : s=%i\n"
						, netsrcString[ chan->sock ]
						, msg->cursize
						, sequence );
		}
	}

	//
	// discard out of order or duplicated packets
	//
	if ( sequence <= chan->incomingSequence ) {
		if ( showdrop->integer || showpackets->integer ) {
			Com_Printf( "%s:Out of order packet %i at %i\n"
						, NET_AdrToString( chan->remoteAddress )
						,  sequence
						, chan->incomingSequence );
		}
		return qfalse;
	}

	//
	// dropped packets don't keep the message from being used
	//
	chan->dropped = sequence - ( chan->incomingSequence + 1 );
	if ( chan->dropped > 0 ) {
		if ( showdrop->integer || showpackets->integer ) {
			Com_Printf( "%s:Dropped %i packets at %i\n"
						, NET_AdrToString( chan->remoteAddress )
						, chan->dropped
						, sequence );
		}
	}


	//
	// if this is the final framgent of a reliable message,
	// bump incoming_reliable_sequence
	//
	if ( fragmented ) {
		// TTimo
		// make sure we add the fragments in correct order
		// either a packet was dropped, or we received this one too soon
		// we don't reconstruct the fragments. we will wait till this fragment gets to us again
		// (NOTE: we could probably try to rebuild by out of order chunks if needed)
		if ( sequence != chan->fragmentSequence ) {
			chan->fragmentSequence = sequence;
			chan->fragmentLength = 0;
		}

		// if we missed a fragment, dump the message
		if ( fragmentStart != chan->fragmentLength ) {
			if ( showdrop->integer || showpackets->integer ) {

#if !defined RTCW_ET
				Com_Printf( "%s:Dropped a message fragment\n"
#else
				Com_Printf( "%s:Dropped a message fragment, sequence %d\n"
#endif // RTCW_XX

							, NET_AdrToString( chan->remoteAddress )
							, sequence );
			}
			// we can still keep the part that we have so far,
			// so we don't need to clear chan->fragmentLength
			return qfalse;
		}

		// copy the fragment to the fragment buffer
		if ( fragmentLength < 0 || msg->readcount + fragmentLength > msg->cursize ||
			 chan->fragmentLength + fragmentLength > sizeof( chan->fragmentBuffer ) ) {
			if ( showdrop->integer || showpackets->integer ) {
				Com_Printf( "%s:illegal fragment length\n"
							, NET_AdrToString( chan->remoteAddress ) );
			}
			return qfalse;
		}

#if !defined RTCW_ET
		memcpy( chan->fragmentBuffer + chan->fragmentLength,
				msg->data + msg->readcount, fragmentLength );
#else
		Com_Memcpy( chan->fragmentBuffer + chan->fragmentLength,
					msg->data + msg->readcount, fragmentLength );
#endif // RTCW_XX

		chan->fragmentLength += fragmentLength;

		// if this wasn't the last fragment, don't process anything
		if ( fragmentLength == FRAGMENT_SIZE ) {
			return qfalse;
		}

		if ( chan->fragmentLength > msg->maxsize ) {
			Com_Printf( "%s:fragmentLength %i > msg->maxsize\n"
						, NET_AdrToString( chan->remoteAddress ),
						chan->fragmentLength );
			return qfalse;
		}

		// copy the full message over the partial fragment

		// make sure the sequence number is still there
		*(int *)msg->data = rtcw::Endian::le( sequence );

#if !defined RTCW_ET
		memcpy( msg->data + 4, chan->fragmentBuffer, chan->fragmentLength );
#else
		Com_Memcpy( msg->data + 4, chan->fragmentBuffer, chan->fragmentLength );
#endif // RTCW_XX

		msg->cursize = chan->fragmentLength + 4;
		chan->fragmentLength = 0;
		msg->readcount = 4; // past the sequence number
		msg->bit = 32;  // past the sequence number

#if !defined RTCW_SP
		// TTimo
		// clients were not acking fragmented messages
		chan->incomingSequence = sequence;
#endif // RTCW_XX

		return qtrue;
	}

	//
	// the message can now be read from the current message pointer
	//
	chan->incomingSequence = sequence;

	return qtrue;
}


//==============================================================================

/*
===================
NET_CompareBaseAdr

Compares without the port
===================
*/
qboolean    NET_CompareBaseAdr( netadr_t a, netadr_t b ) {
	if ( a.type != b.type ) {
		return qfalse;
	}

	if ( a.type == NA_LOOPBACK ) {
		return qtrue;
	}

	if ( a.type == NA_IP ) {
		if ( a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] ) {
			return qtrue;
		}
		return qfalse;
	}

	if ( a.type == NA_IPX ) {
		if ( ( memcmp( a.ipx, b.ipx, 10 ) == 0 ) ) {
			return qtrue;
		}
		return qfalse;
	}


	Com_Printf( "NET_CompareBaseAdr: bad address type\n" );
	return qfalse;
}

const char  *NET_AdrToString( netadr_t a ) {
	static char s[64];

	if ( a.type == NA_LOOPBACK ) {
		Com_sprintf( s, sizeof( s ), "loopback" );
	} else if ( a.type == NA_BOT ) {
		Com_sprintf( s, sizeof( s ), "bot" );
	} else if ( a.type == NA_IP ) {

#if defined RTCW_SP
		Com_sprintf( s, sizeof( s ), "%i.%i.%i.%i:%i",
#else
		Com_sprintf( s, sizeof( s ), "%i.%i.%i.%i:%hu",
#endif // RTCW_XX

					 a.ip[0], a.ip[1], a.ip[2], a.ip[3], rtcw::Endian::be( a.port ) );
	} else {

#if defined RTCW_SP
		Com_sprintf( s, sizeof( s ), "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%i",
#else
		Com_sprintf( s, sizeof( s ), "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%hu",
#endif // RTCW_XX

					 a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9],
					 rtcw::Endian::be( a.port ) );
	}

	return s;
}


qboolean    NET_CompareAdr( netadr_t a, netadr_t b ) {
	if ( a.type != b.type ) {
		return qfalse;
	}

	if ( a.type == NA_LOOPBACK ) {
		return qtrue;
	}

	if ( a.type == NA_IP ) {
		if ( a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port ) {
			return qtrue;
		}
		return qfalse;
	}

	if ( a.type == NA_IPX ) {
		if ( ( memcmp( a.ipx, b.ipx, 10 ) == 0 ) && a.port == b.port ) {
			return qtrue;
		}
		return qfalse;
	}

	Com_Printf( "NET_CompareAdr: bad address type\n" );
	return qfalse;
}


qboolean    NET_IsLocalAddress( netadr_t adr ) {
	return adr.type == NA_LOOPBACK;
}


/*
=============================================================================

LOOPBACK BUFFERS FOR LOCAL PLAYER

=============================================================================
*/

// there needs to be enough loopback messages to hold a complete
// gamestate of maximum size
#define MAX_LOOPBACK    16

typedef struct {
	byte data[MAX_PACKETLEN];
	int datalen;
} loopmsg_t;

typedef struct {
	loopmsg_t msgs[MAX_LOOPBACK];
	int get, send;
} loopback_t;

loopback_t loopbacks[2];


qboolean    NET_GetLoopPacket( netsrc_t sock, netadr_t *net_from, msg_t *net_message ) {
	int i;
	loopback_t  *loop;

	loop = &loopbacks[sock];

	if ( loop->send - loop->get > MAX_LOOPBACK ) {
		loop->get = loop->send - MAX_LOOPBACK;
	}

	if ( loop->get >= loop->send ) {
		return qfalse;
	}

	i = loop->get & ( MAX_LOOPBACK - 1 );
	loop->get++;

#if !defined RTCW_ET
	memcpy( net_message->data, loop->msgs[i].data, loop->msgs[i].datalen );
#else
	Com_Memcpy( net_message->data, loop->msgs[i].data, loop->msgs[i].datalen );
#endif // RTCW_XX

	net_message->cursize = loop->msgs[i].datalen;

#if !defined RTCW_ET
	memset( net_from, 0, sizeof( *net_from ) );
#else
	Com_Memset( net_from, 0, sizeof( *net_from ) );
#endif // RTCW_XX

	net_from->type = NA_LOOPBACK;
	return qtrue;

}


void NET_SendLoopPacket( netsrc_t sock, int length, const void *data, netadr_t to ) {
	int i;
	loopback_t  *loop;

	loop = &loopbacks[sock ^ 1];

	i = loop->send & ( MAX_LOOPBACK - 1 );
	loop->send++;

#if !defined RTCW_ET
	memcpy( loop->msgs[i].data, data, length );
#else
	Com_Memcpy( loop->msgs[i].data, data, length );
#endif // RTCW_XX

	loop->msgs[i].datalen = length;
}

//=============================================================================

#if defined RTCW_ET
//bani
extern cvar_t *sv_maxclients;
extern cvar_t *sv_packetloss;
extern cvar_t *sv_packetdelay;
#ifndef DEDICATED
extern cvar_t *cl_packetloss;
extern cvar_t *cl_packetdelay;
#endif

typedef struct delaybuf delaybuf_t;
struct delaybuf {
	netsrc_t sock;
	int length;
	char data[MAX_PACKETLEN];
	netadr_t to;
	int time;
	delaybuf_t *next;
};

static delaybuf_t *sv_delaybuf_head = NULL;
static delaybuf_t *sv_delaybuf_tail = NULL;
#ifndef DEDICATED
static delaybuf_t *cl_delaybuf_head = NULL;
static delaybuf_t *cl_delaybuf_tail = NULL;
#endif
#endif // RTCW_XX

void NET_SendPacket( netsrc_t sock, int length, const void *data, netadr_t to ) {

#if defined RTCW_ET
//bani
	int packetloss, packetdelay;
	delaybuf_t **delaybuf_head, **delaybuf_tail;

	switch ( sock ) {
#ifndef DEDICATED
	case NS_CLIENT:
		packetloss = cl_packetloss->integer;
		packetdelay = cl_packetdelay->integer;
		delaybuf_head = &cl_delaybuf_head;
		delaybuf_tail = &cl_delaybuf_tail;
		break;
#endif
	case NS_SERVER:
		packetloss = sv_packetloss->integer;
		packetdelay = sv_packetdelay->integer;
		delaybuf_head = &sv_delaybuf_head;
		delaybuf_tail = &sv_delaybuf_tail;
		break;

	default:
		// shut up compiler for dedicated
		packetloss = 0;
		packetdelay = 0;
		delaybuf_head = NULL;
		delaybuf_tail = NULL;
		break;
	}

	if ( packetloss > 0 ) {
		if ( ( (float)rand() / RAND_MAX ) * 100 <= packetloss ) {
			if ( showpackets->integer ) {
				Com_Printf( "drop packet %4i\n", length );
			}
			return;
		}
	}

//bani
	if ( packetdelay ) {
		int curtime;
		delaybuf_t *buf, *nextbuf;

		curtime = Sys_Milliseconds();

		//send any scheduled packets, starting from oldest
		for ( buf = *delaybuf_head; buf; buf = nextbuf ) {

			if ( ( buf->time + packetdelay ) > curtime ) {
				break;
			}

			if ( showpackets->integer ) {
				Com_Printf( "delayed packet(%dms) %4i\n", buf->time - curtime, buf->length );
			}

			switch ( buf->to.type ) {
			case NA_BOT:
			case NA_BAD:
				break;
			case NA_LOOPBACK:
				NET_SendLoopPacket( buf->sock, buf->length, buf->data, buf->to );
				break;
			default:
				Sys_SendPacket( buf->length, buf->data, buf->to );
				break;
			}

			// remove from queue
			nextbuf = buf->next;
			*delaybuf_head = nextbuf;
			if ( !*delaybuf_head ) {
				*delaybuf_tail = NULL;
			}
			Z_Free( buf );
		}

		// create buffer and add it to the queue
		buf = (delaybuf_t *)Z_Malloc( sizeof( *buf ) );
		if ( !buf ) {
			Com_Error( ERR_FATAL, "Couldn't allocate packet delay buffer\n" );
		}

		buf->sock = sock;
		buf->length = length;
		memcpy( buf->data, data, length );
		buf->to = to;
		buf->time = curtime;
		buf->next = NULL;

		if ( *delaybuf_head ) {
			( *delaybuf_tail )->next = buf;
		} else {
			*delaybuf_head = buf;
		}
		*delaybuf_tail = buf;

		return;
	}
#endif // RTCW_XX

	// sequenced packets are shown in netchan, so just show oob
	if ( showpackets->integer && *(int *)data == -1 ) {
		Com_Printf( "send packet %4i\n", length );
	}

	if ( to.type == NA_LOOPBACK ) {
		NET_SendLoopPacket( sock, length, data, to );
		return;
	}
	if ( to.type == NA_BOT ) {
		return;
	}
	if ( to.type == NA_BAD ) {
		return;
	}

	Sys_SendPacket( length, data, to );
}

/*
===============
NET_OutOfBandPrint

Sends a text message in an out-of-band datagram
================
*/
void QDECL NET_OutOfBandPrint( netsrc_t sock, netadr_t adr, const char *format, ... ) {
	va_list argptr;
	char string[MAX_MSGLEN];

	// set the header
	string[0] = -1;
	string[1] = -1;
	string[2] = -1;
	string[3] = -1;

	va_start( argptr, format );

#if defined RTCW_SP
	vsprintf( string + 4, format, argptr );
#else
	Q_vsnprintf( string + 4, sizeof( string ) - 4, format, argptr );
#endif // RTCW_XX

	va_end( argptr );

	// send the datagram
	NET_SendPacket( sock, strlen( string ), string, adr );
}


#if !defined RTCW_SP
/*
===============
NET_OutOfBandPrint

Sends a data message in an out-of-band datagram (only used for "connect")
================
*/
void QDECL NET_OutOfBandData( netsrc_t sock, netadr_t adr, byte *format, int len ) {
	byte string[MAX_MSGLEN * 2];
	int i;
	msg_t mbuf;

#if defined RTCW_ET
	MSG_InitOOB( &mbuf, string, sizeof( string ) );
#endif // RTCW_XX

	// set the header
	string[0] = 0xff;
	string[1] = 0xff;
	string[2] = 0xff;
	string[3] = 0xff;

	for ( i = 0; i < len; i++ ) {
		string[i + 4] = format[i];
	}

#if !defined RTCW_ET
	mbuf.data = string;
#endif // RTCW_XX

	mbuf.cursize = len + 4;
	Huff_Compress( &mbuf, 12 );
	// send the datagram
	NET_SendPacket( sock, mbuf.cursize, mbuf.data, adr );
}
#endif // RTCW_XX


/*
=============
NET_StringToAdr

Traps "localhost" for loopback, passes everything else to system
=============
*/
qboolean    NET_StringToAdr( const char *s, netadr_t *a ) {
	bool r;
	char base[MAX_STRING_CHARS];
	char    *port;

	if ( !strcmp( s, "localhost" ) ) {

#if !defined RTCW_ET
		memset( a, 0, sizeof( *a ) );
#else
		Com_Memset( a, 0, sizeof( *a ) );
#endif // RTCW_XX

		a->type = NA_LOOPBACK;
		return qtrue;
	}

	// look for a port number
	Q_strncpyz( base, s, sizeof( base ) );
	port = strstr( base, ":" );
	if ( port ) {
		*port = 0;
		port++;
	}

	r = Sys_StringToAdr( base, a );

	if ( !r ) {
		a->type = NA_BAD;
		return qfalse;
	}

	// inet_addr returns this if out of range
	if ( a->ip[0] == 255 && a->ip[1] == 255 && a->ip[2] == 255 && a->ip[3] == 255 ) {
		a->type = NA_BAD;
		return qfalse;
	}

	if ( port ) {
		a->port = rtcw::Endian::be( (short)atoi( port ) );
	} else {
		a->port = rtcw::Endian::be( PORT_SERVER );
	}

	return qtrue;
}

