/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "q_shared.h"
#include "qcommon.h"
#include "server.h"

#if (defined RTCW_SP && defined DO_NET_ENCODE) || !defined RTCW_SP
/*
==============
SV_Netchan_Encode

	// first four bytes of the data are always:
	long reliableAcknowledge;

==============
*/

#if !defined RTCW_ET
static void SV_Netchan_Encode( client_t *client, msg_t *msg ) {
#else
static void SV_Netchan_Encode( client_t *client, msg_t *msg, char *commandString ) {
#endif // RTCW_XX

	int32_t reliableAcknowledge, i, index;
	byte key, *string;
	int srdc, sbit, soob;

	if ( msg->cursize < SV_ENCODE_START ) {
		return;
	}

#if defined RTCW_ET
	// NOTE: saving pos, reading reliableAck, restoring, not using it .. useless?
#endif // RTCW_XX

	srdc = msg->readcount;
	sbit = msg->bit;
	soob = msg->oob;

	msg->bit = 0;
	msg->readcount = 0;
	msg->oob = 0;

	reliableAcknowledge = MSG_ReadLong( msg );

	msg->oob = soob;
	msg->bit = sbit;
	msg->readcount = srdc;

#if !defined RTCW_ET
	string = (byte *)client->lastClientCommandString;
#else
	string = (byte *)commandString;
#endif // RTCW_XX

	index = 0;
	// xor the client challenge with the netchan sequence number
	key = client->challenge ^ client->netchan.outgoingSequence;
	for ( i = SV_ENCODE_START; i < msg->cursize; i++ ) {
		// modify the key with the last received and with this message acknowledged client command
		if ( !string[index] ) {
			index = 0;
		}
		if ( string[index] > 127 || string[index] == '%' ) {
			key ^= '.' << ( i & 1 );
		} else {
			key ^= string[index] << ( i & 1 );
		}
		index++;
		// encode the data with this key
		*( msg->data + i ) = *( msg->data + i ) ^ key;
	}
}

/*
==============
SV_Netchan_Decode

	// first 12 bytes of the data are always:
	long serverId;
	long messageAcknowledge;
	long reliableAcknowledge;

==============
*/
static void SV_Netchan_Decode( client_t *client, msg_t *msg ) {
	int serverId, messageAcknowledge, reliableAcknowledge;
	int i, index, srdc, sbit, soob;
	byte key, *string;

	srdc = msg->readcount;
	sbit = msg->bit;
	soob = msg->oob;

	msg->oob = 0;

	serverId = MSG_ReadLong( msg );
	messageAcknowledge = MSG_ReadLong( msg );
	reliableAcknowledge = MSG_ReadLong( msg );

	msg->oob = soob;
	msg->bit = sbit;
	msg->readcount = srdc;

#if defined RTCW_SP
	string = (byte *)SV_GetReliableCommand( client, reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) );
#else
	string = (byte *)client->reliableCommands[ reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ];
#endif // RTCW_XX

	index = 0;
	//
	key = client->challenge ^ serverId ^ messageAcknowledge;
	for ( i = msg->readcount + SV_DECODE_START; i < msg->cursize; i++ ) {
		// modify the key with the last sent and acknowledged server command
		if ( !string[index] ) {
			index = 0;
		}
		if ( string[index] > 127 || string[index] == '%' ) {
			key ^= '.' << ( i & 1 );
		} else {
			key ^= string[index] << ( i & 1 );
		}
		index++;
		// decode the data with this key
		*( msg->data + i ) = *( msg->data + i ) ^ key;
	}
}
#endif // RTCW_XX

/*
=================
SV_Netchan_TransmitNextFragment
=================
*/

#if defined RTCW_SP
void SV_Netchan_TransmitNextFragment( netchan_t *chan ) {
	Netchan_TransmitNextFragment( chan );
}
#elif defined RTCW_MP
void SV_Netchan_TransmitNextFragment( client_t *client ) {
	Netchan_TransmitNextFragment( &client->netchan );
	if ( !client->netchan.unsentFragments ) {
		// make sure the netchan queue has been properly initialized (you never know)
		if ( !client->netchan_end_queue ) {
			Com_Error( ERR_DROP, "netchan queue is not properly initialized in SV_Netchan_TransmitNextFragment\n" );
		}
		// the last fragment was transmitted, check wether we have queued messages
		if ( client->netchan_start_queue ) {
			netchan_buffer_t *netbuf;
			//Com_DPrintf("Netchan_TransmitNextFragment: popping a queued message for transmit\n");
			netbuf = client->netchan_start_queue;

			SV_Netchan_Encode( client, &netbuf->msg );
			Netchan_Transmit( &client->netchan, netbuf->msg.cursize, netbuf->msg.data );

			// pop from queue
			client->netchan_start_queue = netbuf->next;
			if ( !client->netchan_start_queue ) {
				//Com_DPrintf("Netchan_TransmitNextFragment: emptied queue\n");
				client->netchan_end_queue = &client->netchan_start_queue;
			}
			/*
			else
				Com_DPrintf("Netchan_TransmitNextFragment: remaining queued message\n");
				*/
			Z_Free( netbuf );
		}
	}
}
#else
void SV_Netchan_TransmitNextFragment( client_t *client ) {
	Netchan_TransmitNextFragment( &client->netchan );
	while ( !client->netchan.unsentFragments && client->netchan_start_queue )
	{
		// make sure the netchan queue has been properly initialized (you never know)
		//%	if (!client->netchan_end_queue) {
		//%		Com_Error(ERR_DROP, "netchan queue is not properly initialized in SV_Netchan_TransmitNextFragment\n");
		//%	}
		// the last fragment was transmitted, check wether we have queued messages
		netchan_buffer_t* netbuf = client->netchan_start_queue;

		// pop from queue
		client->netchan_start_queue = netbuf->next;
		if ( !client->netchan_start_queue ) {
			client->netchan_end_queue = NULL;
		}

		if ( !SV_GameIsSinglePlayer() ) {
			SV_Netchan_Encode( client, &netbuf->msg, netbuf->lastClientCommandString );
		}
		Netchan_Transmit( &client->netchan, netbuf->msg.cursize, netbuf->msg.data );

		Z_Free( netbuf );
	}
}
#endif // RTCW_XX

#if defined RTCW_ET
/*
===============
SV_WriteBinaryMessage
===============
*/
static void SV_WriteBinaryMessage( msg_t *msg, client_t *cl ) {
	if ( !cl->binaryMessageLength ) {
		return;
	}

	MSG_Uncompressed( msg );

	if ( ( msg->cursize + cl->binaryMessageLength ) >= msg->maxsize ) {
		cl->binaryMessageOverflowed = qtrue;
		return;
	}

	MSG_WriteData( msg, cl->binaryMessage, cl->binaryMessageLength );
	cl->binaryMessageLength = 0;
	cl->binaryMessageOverflowed = qfalse;
}
#endif // RTCW_XX

#if defined RTCW_SP
/*
===============
SV_Netchan_Transmit
================
*/
#else
/*
===============
SV_Netchan_Transmit

TTimo
show_bug.cgi?id=462
if there are some unsent fragments (which may happen if the snapshots
and the gamestate are fragmenting, and collide on send for instance)
then buffer them and make sure they get sent in correct order
================
*/
#endif // RTCW_XX

#if defined RTCW_SP
//extern byte chksum[65536];
void SV_Netchan_Transmit( client_t *client, msg_t *msg ) {   //int length, const byte *data ) {
//	int i;
	MSG_WriteByte( msg, svc_EOF );
//	for(i=SV_ENCODE_START;i<msg->cursize;i++) {
//		chksum[i-SV_ENCODE_START] = msg->data[i];
//	}
//	Huff_Compress( msg, SV_ENCODE_START );
#if DO_NET_ENCODE
	SV_Netchan_Encode( client, msg );
#endif
	Netchan_Transmit( &client->netchan, msg->cursize, msg->data );
}
#elif defined RTCW_MP
void SV_Netchan_Transmit( client_t *client, msg_t *msg ) {   //int length, const byte *data ) {
	MSG_WriteByte( msg, svc_EOF );
	if ( client->netchan.unsentFragments ) {
		netchan_buffer_t *netbuf;
		//Com_DPrintf("SV_Netchan_Transmit: there are unsent fragments remaining\n");
		netbuf = (netchan_buffer_t *)Z_Malloc( sizeof( netchan_buffer_t ) );
		// store the msg, we can't store it encoded, as the encoding depends on stuff we still have to finish sending
		MSG_Copy( &netbuf->msg, netbuf->msgBuffer, sizeof( netbuf->msgBuffer ), msg );
		netbuf->next = NULL;
		// insert it in the queue, the message will be encoded and sent later
		*client->netchan_end_queue = netbuf;
		client->netchan_end_queue = &( *client->netchan_end_queue )->next;
		// emit the next fragment of the current message for now
		Netchan_TransmitNextFragment( &client->netchan );
	} else {
		SV_Netchan_Encode( client, msg );
		Netchan_Transmit( &client->netchan, msg->cursize, msg->data );
	}
}
#else
void SV_Netchan_Transmit( client_t *client, msg_t *msg ) {   //int length, const byte *data ) {
	MSG_WriteByte( msg, svc_EOF );
	SV_WriteBinaryMessage( msg, client );

	if ( client->netchan.unsentFragments ) {
		netchan_buffer_t *netbuf;
		//Com_DPrintf("SV_Netchan_Transmit: there are unsent fragments remaining\n");
		netbuf = (netchan_buffer_t *)Z_Malloc( sizeof( netchan_buffer_t ) );

		// store the msg, we can't store it encoded, as the encoding depends on stuff we still have to finish sending
		MSG_Copy( &netbuf->msg, netbuf->msgBuffer, sizeof( netbuf->msgBuffer ), msg );

		// copy the command, since the command number used for encryption is
		// already compressed in the buffer, and receiving a new command would
		// otherwise lose the proper encryption key
		strcpy( netbuf->lastClientCommandString, client->lastClientCommandString );

		// insert it in the queue, the message will be encoded and sent later
		//%	*client->netchan_end_queue = netbuf;
		//%	client->netchan_end_queue = &(*client->netchan_end_queue)->next;
		netbuf->next = NULL;
		if ( !client->netchan_start_queue ) {
			client->netchan_start_queue = netbuf;
		} else {
			client->netchan_end_queue->next = netbuf;
		}
		client->netchan_end_queue = netbuf;

		// emit the next fragment of the current message for now
		Netchan_TransmitNextFragment( &client->netchan );
	} else {
		if ( !SV_GameIsSinglePlayer() ) {
			SV_Netchan_Encode( client, msg, client->lastClientCommandString );
		}
		Netchan_Transmit( &client->netchan, msg->cursize, msg->data );
	}
}
#endif // RTCW_XX

/*
=================
Netchan_SV_Process
=================
*/
qboolean SV_Netchan_Process( client_t *client, msg_t *msg ) {
	int ret;

#if defined RTCW_SP
//	int i;
#endif // RTCW_XX

	ret = Netchan_Process( &client->netchan, msg );
	if ( !ret ) {
		return qfalse;
	}

#if defined RTCW_ET
	if ( !SV_GameIsSinglePlayer() ) {
#endif // RTCW_XX

#if (defined RTCW_SP && DO_NET_ENCODE) || !defined RTCW_SP
	SV_Netchan_Decode( client, msg );
#endif // RTCW_XX

#if defined RTCW_ET
	}
#endif // RTCW_XX

#if defined RTCW_SP
//	Huff_Decompress( msg, SV_DECODE_START );
//	for(i=SV_DECODE_START+msg->readcount;i<msg->cursize;i++) {
//		if (msg->data[i] != chksum[i-(SV_DECODE_START+msg->readcount)]) {
//			Com_Error(ERR_DROP,"bad\n");
//		}
//	}
#endif // RTCW_XX

	return qtrue;
}

