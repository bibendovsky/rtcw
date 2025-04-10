/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "q_shared.h"
#include "qcommon.h"
#include "client.h"
#include "rtcw_endian.h"

#if (!defined RTCW_SP) || ((defined RTCW_SP) && (defined DO_NET_ENCODE))
/*
==============
CL_Netchan_Encode

	// first 12 bytes of the data are always:
	long serverId;
	long messageAcknowledge;
	long reliableAcknowledge;

==============
*/
static void CL_Netchan_Encode( msg_t *msg ) {
	int serverId, messageAcknowledge, reliableAcknowledge;
	int i, index, srdc, sbit, soob;
	byte key, *string;

	if ( msg->cursize <= CL_ENCODE_START ) {
		return;
	}

	srdc = msg->readcount;
	sbit = msg->bit;
	soob = msg->oob;

	msg->bit = 0;
	msg->readcount = 0;
	msg->oob = 0;

	serverId = MSG_ReadLong( msg );
	messageAcknowledge = MSG_ReadLong( msg );
	reliableAcknowledge = MSG_ReadLong( msg );

	msg->oob = soob;
	msg->bit = sbit;
	msg->readcount = srdc;

	string = (byte *)clc.serverCommands[ reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ];
	index = 0;
	//
	key = clc.challenge ^ serverId ^ messageAcknowledge;
	for ( i = CL_ENCODE_START; i < msg->cursize; i++ ) {
		// modify the key with the last received now acknowledged server command
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
		*( msg->data + i ) = ( *( msg->data + i ) ) ^ key;
	}
}

/*
==============
CL_Netchan_Decode

	// first four bytes of the data are always:
	long reliableAcknowledge;

==============
*/
static void CL_Netchan_Decode( msg_t *msg ) {
	int32_t reliableAcknowledge, i, index;
	byte key, *string;
	int srdc, sbit, soob;

	srdc = msg->readcount;
	sbit = msg->bit;
	soob = msg->oob;

	msg->oob = 0;

	reliableAcknowledge = MSG_ReadLong( msg );

	msg->oob = soob;
	msg->bit = sbit;
	msg->readcount = srdc;

	string = reinterpret_cast<byte*> (clc.reliableCommands[ reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ]);
	index = 0;
	// xor the client challenge with the netchan sequence number (need something that changes every message)
	key = clc.challenge ^ rtcw::Endian::le( *(unsigned *)msg->data );
	for ( i = msg->readcount + CL_DECODE_START; i < msg->cursize; i++ ) {
		// modify the key with the last sent and with this message acknowledged client command
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
CL_Netchan_TransmitNextFragment
=================
*/
void CL_Netchan_TransmitNextFragment( netchan_t *chan ) {
	Netchan_TransmitNextFragment( chan );
}

#if defined RTCW_SP
//byte chksum[65536];
#endif // RTCW_XX

#if defined RTCW_ET
extern qboolean SV_GameIsSinglePlayer( void );

/*
================
CL_WriteBinaryMessage
================
*/
static void CL_WriteBinaryMessage( msg_t *msg ) {
	if ( !clc.binaryMessageLength ) {
		return;
	}

	MSG_Uncompressed( msg );

	if ( ( msg->cursize + clc.binaryMessageLength ) >= msg->maxsize ) {
		clc.binaryMessageOverflowed = qtrue;
		return;
	}

	MSG_WriteData( msg, clc.binaryMessage, clc.binaryMessageLength );
	clc.binaryMessageLength = 0;
	clc.binaryMessageOverflowed = qfalse;
}
#endif // RTCW_XX


/*
================
CL_Netchan_Transmit
================
*/
void CL_Netchan_Transmit( netchan_t *chan, msg_t* msg ) {

#if defined RTCW_SP
//	int i;
	MSG_WriteByte( msg, clc_EOF );
//	for(i=CL_ENCODE_START;i<msg->cursize;i++) {
//		chksum[i-CL_ENCODE_START] = msg->data[i];
//	}

//	Huff_Compress( msg, CL_ENCODE_START );
#if DO_NET_ENCODE
	CL_Netchan_Encode( msg );
#endif
	Netchan_Transmit( chan, msg->cursize, msg->data );
#elif defined RTCW_MP
	MSG_WriteByte( msg, clc_EOF );
	CL_Netchan_Encode( msg );
	Netchan_Transmit( chan, msg->cursize, msg->data );
#else
	MSG_WriteByte( msg, clc_EOF );
	CL_WriteBinaryMessage( msg );

	if ( !SV_GameIsSinglePlayer() ) {
		CL_Netchan_Encode( msg );
	}
	Netchan_Transmit( chan, msg->cursize, msg->data );
#endif // RTCW_XX

}

extern int oldsize;
int newsize = 0;

/*
=================
CL_Netchan_Process
=================
*/
qboolean CL_Netchan_Process( netchan_t *chan, msg_t *msg ) {
	int ret;

#if defined RTCW_SP
//	int i;
//	static		int newsize = 0;
#endif // RTCW_XX

	ret = Netchan_Process( chan, msg );
	if ( !ret ) {
		return qfalse;
	}

#if (defined RTCW_SP && DO_NET_ENCODE) || defined RTCW_MP
	CL_Netchan_Decode( msg );
#elif defined RTCW_ET
	if ( !SV_GameIsSinglePlayer() ) {
		CL_Netchan_Decode( msg );
	}
#endif

#if defined RTCW_SP
//	Huff_Decompress( msg, CL_DECODE_START );
//	for(i=CL_DECODE_START+msg->readcount;i<msg->cursize;i++) {
//		if (msg->data[i] != chksum[i-(CL_DECODE_START+msg->readcount)]) {
//			Com_Error(ERR_DROP,"bad %d v %d\n", msg->data[i], chksum[i-(CL_DECODE_START+msg->readcount)]);
//		}
//	}
#endif // RTCW_XX

	newsize += msg->cursize;

#if defined RTCW_SP
//	Com_Printf("saved %d to %d (%d%%)\n", (oldsize>>3), newsize, 100-(newsize*100/(oldsize>>3)));
#endif // RTCW_XX

	return qtrue;
}
