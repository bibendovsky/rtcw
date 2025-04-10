/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		l_memory.h
 *
 * desc:		memory management
 *
 *
 *****************************************************************************/

#ifdef _DEBUG

#if defined RTCW_SP
#ifndef BSPC
	#define MEMDEBUG
#endif
#elif defined RTCW_MP
	#define MEMDEBUG
#else
//	#define MEMDEBUG
#endif // RTCW_XX

#endif

#ifdef MEMDEBUG
#define GetMemory( size )             GetMemoryDebug( size, # size, __FILE__, __LINE__ )
#define GetClearedMemory( size )      GetClearedMemoryDebug( size, # size, __FILE__, __LINE__ )
//allocate a memory block of the given size
void *GetMemoryDebug( uint32_t size, char *label, char *file, int line );
//allocate a memory block of the given size and clear it
void *GetClearedMemoryDebug( uint32_t size, char *label, char *file, int line );
//
#define GetHunkMemory( size )         GetHunkMemoryDebug( size, # size, __FILE__, __LINE__ )
#define GetClearedHunkMemory( size )  GetClearedHunkMemoryDebug( size, # size, __FILE__, __LINE__ )
//allocate a memory block of the given size
void *GetHunkMemoryDebug( uint32_t size, char *label, char *file, int line );
//allocate a memory block of the given size and clear it
void *GetClearedHunkMemoryDebug( uint32_t size, char *label, char *file, int line );
#else
//allocate a memory block of the given size
void *GetMemory( uint32_t size );
//allocate a memory block of the given size and clear it
void *GetClearedMemory( uint32_t size );
//
#ifdef BSPC
#define GetHunkMemory GetMemory
#define GetClearedHunkMemory GetClearedMemory
#else
//allocate a memory block of the given size
void *GetHunkMemory( uint32_t size );
//allocate a memory block of the given size and clear it
void *GetClearedHunkMemory( uint32_t size );
#endif
#endif

//free the given memory block
void FreeMemory( void *ptr );
//prints the total used memory size
void PrintUsedMemorySize( void );
//print all memory blocks with label
void PrintMemoryLabels( void );
//returns the size of the memory block in bytes
int MemoryByteSize( void *ptr );
//free all allocated memory
void DumpMemory( void );
