/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

//===========================================================================
//
// Name:				l_crc.h
// Function:		for CRC checks
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1997-12-31
// Tab Size:		3
//===========================================================================

typedef unsigned short crc_t;

void CRC_Init( unsigned short *crcvalue );
void CRC_ProcessByte( unsigned short *crcvalue, byte data );
unsigned short CRC_Value( unsigned short crcvalue );
unsigned short CRC_ProcessString( unsigned char *data, int length );
void CRC_ContinueProcessString( unsigned short *crc, char *data, int length );
