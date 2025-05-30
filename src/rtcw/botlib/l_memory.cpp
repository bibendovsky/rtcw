/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		l_memory.c
 *
 * desc:		memory allocation
 *
 *
 *****************************************************************************/

#ifdef RTCW_VANILLA

#include "q_shared.h"
#include "botlib.h"
#include "l_log.h"
#include "be_interface.h"

#ifdef _DEBUG

#if !defined RTCW_ET
	#define MEMDEBUG
#else
//	#define MEMDEBUG
#endif // RTCW_XX

	#define MEMORYMANEGER
#endif

#define MEM_ID      0x12345678l
#define HUNK_ID     0x87654321l

int allocatedmemory;
int totalmemorysize;
int numblocks;

#ifdef MEMORYMANEGER

typedef struct memoryblock_s
{
	uint32_t id;
	void *ptr;
	int size;
#ifdef MEMDEBUG
	char *label;
	char *file;
	int line;
#endif //MEMDEBUG
	struct memoryblock_s *prev, *next;
} memoryblock_t;

memoryblock_t *memory;

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void LinkMemoryBlock( memoryblock_t *block ) {
	block->prev = NULL;
	block->next = memory;
	if ( memory ) {
		memory->prev = block;
	}
	memory = block;
} //end of the function LinkMemoryBlock
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void UnlinkMemoryBlock( memoryblock_t *block ) {
	if ( block->prev ) {
		block->prev->next = block->next;
	} else { memory = block->next;}
	if ( block->next ) {
		block->next->prev = block->prev;
	}
} //end of the function UnlinkMemoryBlock
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef MEMDEBUG
void *GetMemoryDebug( uint32_t size, char *label, char *file, int line )
#else
void *GetMemory( uint32_t size )
#endif //MEMDEBUG
{
	void *ptr;
	memoryblock_t *block;

	ptr = botimport.GetMemory( size + sizeof( memoryblock_t ) );
	block = (memoryblock_t *) ptr;
	block->id = MEM_ID;
	block->ptr = (char *) ptr + sizeof( memoryblock_t );
	block->size = size + sizeof( memoryblock_t );
#ifdef MEMDEBUG
	block->label = label;
	block->file = file;
	block->line = line;
#endif //MEMDEBUG
	LinkMemoryBlock( block );
	allocatedmemory += block->size;
	totalmemorysize += block->size + sizeof( memoryblock_t );
	numblocks++;
	return block->ptr;
} //end of the function GetMemoryDebug
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef MEMDEBUG
void *GetClearedMemoryDebug( uint32_t size, char *label, char *file, int line )
#else
void *GetClearedMemory( uint32_t size )
#endif //MEMDEBUG
{
	void *ptr;
#ifdef MEMDEBUG
	ptr = GetMemoryDebug( size, label, file, line );
#else
	ptr = GetMemory( size );
#endif //MEMDEBUG
	memset( ptr, 0, size );
	return ptr;
} //end of the function GetClearedMemory
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef MEMDEBUG
void *GetHunkMemoryDebug( uint32_t size, char *label, char *file, int line )
#else
void *GetHunkMemory( uint32_t size )
#endif //MEMDEBUG
{
	void *ptr;
	memoryblock_t *block;

	ptr = botimport.HunkAlloc( size + sizeof( memoryblock_t ) );
	block = (memoryblock_t *) ptr;
	block->id = HUNK_ID;
	block->ptr = (char *) ptr + sizeof( memoryblock_t );
	block->size = size + sizeof( memoryblock_t );
#ifdef MEMDEBUG
	block->label = label;
	block->file = file;
	block->line = line;
#endif //MEMDEBUG
	LinkMemoryBlock( block );
	allocatedmemory += block->size;
	totalmemorysize += block->size + sizeof( memoryblock_t );
	numblocks++;
	return block->ptr;
} //end of the function GetHunkMemoryDebug
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef MEMDEBUG
void *GetClearedHunkMemoryDebug( uint32_t size, char *label, char *file, int line )
#else
void *GetClearedHunkMemory( uint32_t size )
#endif //MEMDEBUG
{
	void *ptr;
#ifdef MEMDEBUG
	ptr = GetHunkMemoryDebug( size, label, file, line );
#else
	ptr = GetHunkMemory( size );
#endif //MEMDEBUG
	memset( ptr, 0, size );
	return ptr;
} //end of the function GetClearedHunkMemory
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
memoryblock_t *BlockFromPointer( void *ptr, char *str ) {
	memoryblock_t *block;

	if ( !ptr ) {
#ifdef MEMDEBUG
		//char *crash = (char *) NULL;
		//crash[0] = 1;
		botimport.Print( PRT_FATAL, "%s: NULL pointer\n", str );
#endif //MEMDEBUG
		return NULL;
	} //end if
	block = ( memoryblock_t * )( (char *) ptr - sizeof( memoryblock_t ) );
	if ( block->id != MEM_ID && block->id != HUNK_ID ) {
		botimport.Print( PRT_FATAL, "%s: invalid memory block\n", str );
		return NULL;
	} //end if
	if ( block->ptr != ptr ) {
		botimport.Print( PRT_FATAL, "%s: memory block pointer invalid\n", str );
		return NULL;
	} //end if
	return block;
} //end of the function BlockFromPointer
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void FreeMemory( void *ptr ) {
	memoryblock_t *block;

	block = BlockFromPointer( ptr, "FreeMemory" );
	if ( !block ) {
		return;
	}
	UnlinkMemoryBlock( block );
	allocatedmemory -= block->size;
	totalmemorysize -= block->size + sizeof( memoryblock_t );
	numblocks--;
	//
	if ( block->id == MEM_ID ) {
		botimport.FreeMemory( block );
	} //end if
} //end of the function FreeMemory
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int MemoryByteSize( void *ptr ) {
	memoryblock_t *block;

	block = BlockFromPointer( ptr, "MemoryByteSize" );
	if ( !block ) {
		return 0;
	}
	return block->size;
} //end of the function MemoryByteSize
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void PrintUsedMemorySize( void ) {
	botimport.Print( PRT_MESSAGE, "total allocated memory: %d KB\n", allocatedmemory >> 10 );
	botimport.Print( PRT_MESSAGE, "total botlib memory: %d KB\n", totalmemorysize >> 10 );
	botimport.Print( PRT_MESSAGE, "total memory blocks: %d\n", numblocks );
} //end of the function PrintUsedMemorySize
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void PrintMemoryLabels( void ) {
	memoryblock_t *block;
	int i;

	PrintUsedMemorySize();
	i = 0;
	Log_Write( "\r\n" );
	for ( block = memory; block; block = block->next )
	{
#ifdef MEMDEBUG
		if ( block->id == HUNK_ID ) {
			Log_Write( "%6d, hunk %p, %8d: %24s line %6d: %s\r\n", i, block->ptr, block->size, block->file, block->line, block->label );
		} //end if
		else
		{
			Log_Write( "%6d,      %p, %8d: %24s line %6d: %s\r\n", i, block->ptr, block->size, block->file, block->line, block->label );
		} //end else
#endif //MEMDEBUG
		i++;
	} //end for
} //end of the function PrintMemoryLabels
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void DumpMemory( void ) {
	memoryblock_t *block;

	for ( block = memory; block; block = memory )
	{
		FreeMemory( block->ptr );
	} //end for
	totalmemorysize = 0;
	allocatedmemory = 0;
} //end of the function DumpMemory

#else

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef MEMDEBUG
void *GetMemoryDebug( uint32_t size, char *label, char *file, int line )
#else
void *GetMemory( uint32_t size )
#endif //MEMDEBUG
{
	void *ptr;
	uint32_t *memid;

	ptr = botimport.GetMemory( size + sizeof( uint32_t ) );
	if ( !ptr ) {
		return NULL;
	}
	memid = (uint32_t *) ptr;
	*memid = MEM_ID;
	return (uint32_t *) ( (char *) ptr + sizeof( uint32_t ) );
} //end of the function GetMemory
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef MEMDEBUG
void *GetClearedMemoryDebug( uint32_t size, char *label, char *file, int line )
#else
void *GetClearedMemory( uint32_t size )
#endif //MEMDEBUG
{
	void *ptr;
#ifdef MEMDEBUG
	ptr = GetMemoryDebug( size, label, file, line );
#else
ptr = GetMemory( size );
#endif //MEMDEBUG
memset( ptr, 0, size );
return ptr;
} //end of the function GetClearedMemory
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef MEMDEBUG
void *GetHunkMemoryDebug( uint32_t size, char *label, char *file, int line )
#else
void *GetHunkMemory( uint32_t size )
#endif //MEMDEBUG
{
	void *ptr;
	uint32_t *memid;

	ptr = botimport.HunkAlloc( size + sizeof( uint32_t ) );
	if ( !ptr ) {
		return NULL;
	}
	memid = (uint32_t *) ptr;
	*memid = HUNK_ID;
	return (uint32_t *) ( (char *) ptr + sizeof( uint32_t ) );
} //end of the function GetHunkMemory
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
#ifdef MEMDEBUG
void *GetClearedHunkMemoryDebug( uint32_t size, char *label, char *file, int line )
#else
void *GetClearedHunkMemory( uint32_t size )
#endif //MEMDEBUG
{
	void *ptr;
#ifdef MEMDEBUG
	ptr = GetHunkMemoryDebug( size, label, file, line );
#else
ptr = GetHunkMemory( size );
#endif //MEMDEBUG
memset( ptr, 0, size );
return ptr;
} //end of the function GetClearedHunkMemory
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void FreeMemory( void *ptr ) {
	uint32_t *memid;

	memid = (uint32_t *) ( (char *) ptr - sizeof( uint32_t ) );

	if ( *memid == MEM_ID ) {
		botimport.FreeMemory( memid );
	} //end if
} //end of the function FreeMemory
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void PrintUsedMemorySize( void ) {
} //end of the function PrintUsedMemorySize
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void PrintMemoryLabels( void ) {
} //end of the function PrintMemoryLabels

#endif

#else // RTCW_VANILLA

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "q_shared.h"
#include "botlib.h"
#include "be_interface.h"
#include "l_memory.h"

namespace {

const unsigned int MEM_ID = 0x12345678U;
const unsigned int HUNK_ID = 0x87654321U;

union Header
{
	unsigned int id;
	unsigned char alignment_[16];
};

const int header_size = sizeof(Header);

typedef void* (*AllocateFunc)(int);

template<typename T>
inline void maybe_unused(T&)
{}

void* allocate(int size, unsigned int id, AllocateFunc allocate_func)
{
	if (size < 0 || INT_MAX - size < header_size)
	{
		assert(false && "Negative size or size too big.");
		return NULL;
	}

	void* const raw = allocate_func(header_size + size);

	if (raw == NULL)
	{
		return NULL;
	}

	Header* header = static_cast<Header*>(raw);
	header->id = id;
	return &static_cast<unsigned char*>(raw)[header_size];
}

} // namespace

void* GetMemory(int size)
{
	return allocate(size, MEM_ID, botimport.GetMemory);
}

void* GetClearedMemory(int size)
{
	void* const ptr = GetMemory(size);

	if (ptr != NULL)
	{
		memset(ptr, 0, static_cast<size_t>(size));
	}

	return ptr;
}

void* GetHunkMemory(int size)
{
	return allocate(size, HUNK_ID, botimport.HunkAlloc);
}

void* GetClearedHunkMemory(int size)
{
	void* const ptr = GetHunkMemory(size);

	if (ptr != NULL)
	{
		memset(ptr, 0, static_cast<size_t>(size));
	}

	return ptr;
}

void FreeMemory(void* ptr)
{
	if (ptr == NULL)
	{
		return;
	}

	Header* const header = &static_cast<Header*>(ptr)[-1];

	switch (header->id)
	{
		case MEM_ID:
			botimport.FreeMemory(header);
			break;

		case HUNK_ID:
			break;

		default:
			assert(false && "Invalid block id.");
			break;
	}
}

void PrintUsedMemorySize()
{}

void PrintMemoryLabels()
{}

int MemoryByteSize(void* ptr)
{
	maybe_unused(ptr);
	return -1;
}

void DumpMemory()
{}

#endif // RTCW_VANILLA
