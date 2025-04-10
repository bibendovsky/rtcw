/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_file.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

#ifdef AASINTERN
//loads the AAS file with the given name
int AAS_LoadAASFile( char *filename );
//writes an AAS file with the given name
qboolean AAS_WriteAASFile( char *filename );
//dumps the loaded AAS data
void AAS_DumpAASData( void );
//print AAS file information
void AAS_FileInfo( void );
#endif //AASINTERN

