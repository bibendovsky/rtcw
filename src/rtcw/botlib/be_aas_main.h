/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_aas_main.h
 *
 * desc:		AAS
 *
 *
 *****************************************************************************/

#ifdef AASINTERN

extern aas_t( *aasworld );

//AAS error message
void QDECL AAS_Error( const char *fmt, ... );
//set AAS initialized
void AAS_SetInitialized( void );
//setup AAS with the given number of entities and clients
int AAS_Setup( void );
//shutdown AAS
void AAS_Shutdown( void );
//start a new map
int AAS_LoadMap( const char *mapname );
//start a new time frame
int AAS_StartFrame( float time );
#endif //AASINTERN

//returns true if AAS is initialized
int AAS_Initialized( void );
//returns true if the AAS file is loaded
int AAS_Loaded( void );
//returns the model name from the given index
char *AAS_ModelFromIndex( int index );
//returns the index from the given model name
int AAS_IndexFromModel( const char *modelname );
//returns the current time
float AAS_Time( void );

// Ridah
void AAS_SetCurrentWorld( int index );
// done.
