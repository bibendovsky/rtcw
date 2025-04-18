/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// vm.c -- virtual machine

/*


intermix code and data
symbol table

a dll has one imported function: VM_SystemCall
and one exported function: Perform


*/

// BBi
// Non-dll functionality successfully removed.
// BBi


#include "vm_local.h"


vm_t    *currentVM = NULL; // bk001212
vm_t    *lastVM    = NULL; // bk001212
int vm_debugLevel;

#define MAX_VM      3
vm_t vmTable[MAX_VM];


void VM_VmInfo_f( void );
void VM_VmProfile_f( void );


// BBi
//// converts a VM pointer to a C pointer and
//// checks to make sure that the range is acceptable
//void    *VM_VM2C( vmptr_t p, int length ) {
//	return (void *)p;
//}
// BBi

void VM_Debug( int level ) {
	vm_debugLevel = level;
}

/*
==============
VM_Init
==============
*/
void VM_Init( void ) {
	Cvar_Get( "vm_cgame", "0", CVAR_ARCHIVE );
	Cvar_Get( "vm_game",  "0", CVAR_ARCHIVE );
	Cvar_Get( "vm_ui",    "0", CVAR_ARCHIVE );

	Cmd_AddCommand( "vmprofile", VM_VmProfile_f );
	Cmd_AddCommand( "vminfo", VM_VmInfo_f );

	Com_Memset( vmTable, 0, sizeof( vmTable ) );
}


// BBi
///*
//===============
//VM_ValueToSymbol
//
//Assumes a program counter value
//===============
//*/
//const char *VM_ValueToSymbol( vm_t *vm, int value ) {
//	vmSymbol_t  *sym;
//	static char text[MAX_TOKEN_CHARS];
//
//	sym = vm->symbols;
//	if ( !sym ) {
//		return "NO SYMBOLS";
//	}
//
//	// find the symbol
//	while ( sym->next && sym->next->symValue <= value ) {
//		sym = sym->next;
//	}
//
//	if ( value == sym->symValue ) {
//		return sym->symName;
//	}
//
//	Com_sprintf( text, sizeof( text ), "%s+%i", sym->symName, value - sym->symValue );
//
//	return text;
//}
// BBi

// BBi
///*
//===============
//VM_ValueToFunctionSymbol
//
//For profiling, find the symbol behind this value
//===============
//*/
//vmSymbol_t *VM_ValueToFunctionSymbol( vm_t *vm, int value ) {
//	vmSymbol_t  *sym;
//	static vmSymbol_t nullSym;
//
//	sym = vm->symbols;
//	if ( !sym ) {
//		return &nullSym;
//	}
//
//	while ( sym->next && sym->next->symValue <= value ) {
//		sym = sym->next;
//	}
//
//	return sym;
//}
// BBi

// BBi
///*
//===============
//VM_SymbolToValue
//===============
//*/
//int VM_SymbolToValue( vm_t *vm, const char *symbol ) {
//	vmSymbol_t  *sym;
//
//	for ( sym = vm->symbols ; sym ; sym = sym->next ) {
//		if ( !strcmp( symbol, sym->symName ) ) {
//			return sym->symValue;
//		}
//	}
//	return 0;
//}
// BBi

// BBi
///*
//=====================
//VM_SymbolForCompiledPointer
//=====================
//*/
//const char *VM_SymbolForCompiledPointer( vm_t *vm, void *code ) {
//	int i;
//
//	if ( code < (void *)vm->codeBase ) {
//		return "Before code block";
//	}
//	if ( code >= ( void * )( vm->codeBase + vm->codeLength ) ) {
//		return "After code block";
//	}
//
//	// find which original instruction it is after
//	for ( i = 0 ; i < vm->codeLength ; i++ ) {
//		if ( (void *)vm->instructionPointers[i] > code ) {
//			break;
//		}
//	}
//	i--;
//
//	// now look up the bytecode instruction pointer
//	return VM_ValueToSymbol( vm, i );
//}
// BBi

// BBi
///*
//===============
//ParseHex
//===============
//*/
//int ParseHex( const char *text ) {
//	int value;
//	int c;
//
//	value = 0;
//	while ( ( c = *text++ ) != 0 ) {
//		if ( c >= '0' && c <= '9' ) {
//			value = value * 16 + c - '0';
//			continue;
//		}
//		if ( c >= 'a' && c <= 'f' ) {
//			value = value * 16 + 10 + c - 'a';
//			continue;
//		}
//		if ( c >= 'A' && c <= 'F' ) {
//			value = value * 16 + 10 + c - 'A';
//			continue;
//		}
//	}
//
//	return value;
//}
// BBi

// BBi
///*
//===============
//VM_LoadSymbols
//===============
//*/
//void VM_LoadSymbols( vm_t *vm ) {
//	int len;
//	char    *mapfile, *text_p, *token;
//	char name[MAX_QPATH];
//	char symbols[MAX_QPATH];
//	vmSymbol_t  **prev, *sym;
//	int count;
//	int value;
//	int chars;
//	int segment;
//	int numInstructions;
//
//	// don't load symbols if not developer
//	if ( !com_developer->integer ) {
//		return;
//	}
//
//#if defined RTCW_SP
//	COM_StripExtension( vm->name, name );
//#else
//	COM_StripExtension2( vm->name, name, sizeof( name ) );
//#endif // RTCW_XX
//
//	Com_sprintf( symbols, sizeof( symbols ), "vm/%s.map", name );
//	len = FS_ReadFile( symbols, (void **)&mapfile );
//	if ( !mapfile ) {
//		Com_Printf( "Couldn't load symbol file: %s\n", symbols );
//		return;
//	}
//
//	numInstructions = vm->instructionPointersLength >> 2;
//
//	// parse the symbols
//	text_p = mapfile;
//	prev = &vm->symbols;
//	count = 0;
//
//	while ( 1 ) {
//		token = COM_Parse( &text_p );
//		if ( !token[0] ) {
//			break;
//		}
//		segment = ParseHex( token );
//		if ( segment ) {
//			COM_Parse( &text_p );
//			COM_Parse( &text_p );
//			continue;       // only load code segment values
//		}
//
//		token = COM_Parse( &text_p );
//		if ( !token[0] ) {
//			Com_Printf( "WARNING: incomplete line at end of file\n" );
//			break;
//		}
//		value = ParseHex( token );
//
//		token = COM_Parse( &text_p );
//		if ( !token[0] ) {
//			Com_Printf( "WARNING: incomplete line at end of file\n" );
//			break;
//		}
//		chars = strlen( token );
//		sym = static_cast<vmSymbol_t*> (Hunk_Alloc( sizeof( *sym ) + chars, h_high ));
//		*prev = sym;
//		prev = &sym->next;
//		sym->next = NULL;
//
//		// convert value from an instruction number to a code offset
//		if ( value >= 0 && value < numInstructions ) {
//			value = vm->instructionPointers[value];
//		}
//
//		sym->symValue = value;
//		Q_strncpyz( sym->symName, token, chars + 1 );
//
//		count++;
//	}
//
//	vm->numSymbols = count;
//	Com_Printf( "%i symbols parsed from %s\n", count, symbols );
//	FS_FreeFile( mapfile );
//}
// BBi

/*
============
VM_DllSyscall

Dlls will call this directly

 rcg010206 The horror; the horror.

  The syscall mechanism relies on stack manipulation to get it's args.
   This is likely due to C's inability to pass "..." parameters to
   a function in one clean chunk. On PowerPC Linux, these parameters
   are not necessarily passed on the stack, so while (&arg[0] == arg)
   is true, (&arg[1] == 2nd function parameter) is not necessarily
   accurate, as arg's value might have been stored to the stack or
   other piece of scratch memory to give it a valid address, but the
   next parameter might still be sitting in a register.

  Quake's syscall system also assumes that the stack grows downward,
   and that any needed types can be squeezed, safely, into a signed int.

  This hack below copies all needed values for an argument to a
   array in memory, so that Quake can get the correct values. This can
   also be used on systems where the stack grows upwards, as the
   presumably standard and safe stdargs.h macros are used.

  As for having enough space in a signed int for your datatypes, well,
   it might be better to wait for DOOM 3 before you start porting.  :)

  The original code, while probably still inherently dangerous, seems
   to work well enough for the platforms it already works on. Rather
   than add the performance hit for those platforms, the original code
   is still in use there.

  For speed, we just grab 15 arguments, and don't worry about exactly
   how many the syscall actually needs; the extra is thrown away.

============
*/

// BBi
//int QDECL VM_DllSyscall( int arg, ... ) {
int32_t QDECL VM_DllSyscall (
	intptr_t arg,
	...)
{
// BBi

	intptr_t args[16];

	args[0] = arg;

	va_list ap;
	va_start( ap, arg );

	for (size_t i = 1; i < sizeof(args) / sizeof(args[i]); ++i)
	{
		args[i] = va_arg(ap, intptr_t);
	}

	va_end(ap);

	return currentVM->systemCall(args);
}

/*
=================
VM_Restart

Reload the data, but leave everything else in place
This allows a server to do a map_restart without changing memory allocation
=================
*/

// BBi
//vm_t *VM_Restart( vm_t *vm ) {
//	vmHeader_t  *header;
//	int length;
//	int dataLength;
//	int i;
//	char filename[MAX_QPATH];
//
//	// DLL's can't be restarted in place
//	if ( vm->dllHandle ) {
//		char name[MAX_QPATH];
//		int ( *systemCall )( int *parms );
//
//		systemCall = vm->systemCall;
//		Q_strncpyz( name, vm->name, sizeof( name ) );
//
//		VM_Free( vm );
//
//		vm = VM_Create( name, systemCall, VMI_NATIVE );
//		return vm;
//	}
//
//	// load the image
//
//#if !defined RTCW_ET
//	Com_Printf( "VM_Restart()\n", filename );
//#else
//	Com_Printf( "VM_Restart()\n" );
//#endif // RTCW_XX
//
//	Com_sprintf( filename, sizeof( filename ), "vm/%s.qvm", vm->name );
//	Com_Printf( "Loading vm file %s.\n", filename );
//	length = FS_ReadFile( filename, (void **)&header );
//	if ( !header ) {
//		Com_Error( ERR_DROP, "VM_Restart failed.\n" );
//	}
//
//	// byte swap the header
//	for ( i = 0 ; i < sizeof( *header ) / 4 ; i++ ) {
//		( (int *)header )[i] = LittleLong( ( (int *)header )[i] );
//	}
//
//	// validate
//	if ( header->vmMagic != VM_MAGIC
//		 || header->bssLength < 0
//		 || header->dataLength < 0
//		 || header->litLength < 0
//		 || header->codeLength <= 0 ) {
//		VM_Free( vm );
//		Com_Error( ERR_FATAL, "%s has bad header", filename );
//	}
//
//	// round up to next power of 2 so all data operations can
//	// be mask protected
//	dataLength = header->dataLength + header->litLength + header->bssLength;
//	for ( i = 0 ; dataLength > ( 1 << i ) ; i++ ) {
//	}
//	dataLength = 1 << i;
//
//	// clear the data
//	Com_Memset( vm->dataBase, 0, dataLength );
//
//	// copy the intialized data
//	Com_Memcpy( vm->dataBase, (byte *)header + header->dataOffset, header->dataLength + header->litLength );
//
//	// byte swap the longs
//	for ( i = 0 ; i < header->dataLength ; i += 4 ) {
//		*( int * )( vm->dataBase + i ) = LittleLong( *( int * )( vm->dataBase + i ) );
//	}
//
//	// free the original file
//	FS_FreeFile( header );
//
//	return vm;
//}

vm_t* VM_Restart (
	vm_t* vm)
{
	if (vm->dllHandle == 0)
		return vm;


	char name[MAX_QPATH];
	int32_t (*systemCall) (intptr_t* parms);

	systemCall = vm->systemCall;
	Q_strncpyz (name, vm->name, sizeof (name));

	VM_Free (vm);

	vm = VM_Create (name, systemCall);

	return vm;
}
// BBi

/*
================
VM_Create

If image ends in .qvm it will be interpreted, otherwise
it will attempt to load as a system dll
================
*/

// BBi
//#define STACK_SIZE  0x20000
//
//vm_t *VM_Create( const char *module, int ( *systemCalls )(int *),
//				 vmInterpret_t interpret ) {
//	vm_t        *vm;
//	vmHeader_t  *header;
//	int length;
//	int dataLength;
//	int i, remaining;
//	char filename[MAX_QPATH];
//
//	if ( !module || !module[0] || !systemCalls ) {
//		Com_Error( ERR_FATAL, "VM_Create: bad parms" );
//	}
//
//	remaining = Hunk_MemoryRemaining();
//
//	// see if we already have the VM
//	for ( i = 0 ; i < MAX_VM ; i++ ) {
//		if ( !Q_stricmp( vmTable[i].name, module ) ) {
//			vm = &vmTable[i];
//			return vm;
//		}
//	}
//
//	// find a free vm
//	for ( i = 0 ; i < MAX_VM ; i++ ) {
//		if ( !vmTable[i].name[0] ) {
//			break;
//		}
//	}
//
//	if ( i == MAX_VM ) {
//		Com_Error( ERR_FATAL, "VM_Create: no free vm_t" );
//	}
//
//	vm = &vmTable[i];
//
//	Q_strncpyz( vm->name, module, sizeof( vm->name ) );
//	vm->systemCall = systemCalls;
//
//#if defined RTCW_SP
//	// never allow dll loading with a demo
///*	if ( interpret == VMI_NATIVE ) {
//		if ( Cvar_VariableValue( "fs_restrict" ) ) {
//			interpret = VMI_COMPILED;
//		}
//	}
//*/
//	// Wolf.  /only/ allow dll loading with a demo
//	if ( interpret != VMI_NATIVE ) {
//		if ( Cvar_VariableValue( "fs_restrict" ) ) {
//			interpret = VMI_NATIVE;
//		}
//	}
//#endif // RTCW_XX
//
//
//	if ( interpret == VMI_NATIVE ) {
//		// try to load as a system dll
//
//#if defined RTCW_SP
//		vm->dllHandle = Sys_LoadDll( module, &vm->entryPoint, VM_DllSyscall );
//#else
//		vm->dllHandle = Sys_LoadDll( module, vm->fqpath, &vm->entryPoint, VM_DllSyscall );
//#endif // RTCW_XX
//
//		if ( vm->dllHandle ) {
//			return vm;
//		}
//
//#if !defined RTCW_ET
//		Com_Printf( "Failed to load dll, looking for qvm.\n" );
//		interpret = VMI_COMPILED;
//#else
//		return NULL;
//#endif // RTCW_XX
//
//	}
//
//	// load the image
//	Com_sprintf( filename, sizeof( filename ), "vm/%s.qvm", vm->name );
//	Com_Printf( "Loading vm file %s.\n", filename );
//	length = FS_ReadFile( filename, (void **)&header );
//	if ( !header ) {
//		Com_Printf( "Failed.\n" );
//		VM_Free( vm );
//		return NULL;
//	}
//
//	// byte swap the header
//	for ( i = 0 ; i < sizeof( *header ) / 4 ; i++ ) {
//		( (int *)header )[i] = LittleLong( ( (int *)header )[i] );
//	}
//
//	// validate
//	if ( header->vmMagic != VM_MAGIC
//		 || header->bssLength < 0
//		 || header->dataLength < 0
//		 || header->litLength < 0
//		 || header->codeLength <= 0 ) {
//		VM_Free( vm );
//		Com_Error( ERR_FATAL, "%s has bad header", filename );
//	}
//
//	// round up to next power of 2 so all data operations can
//	// be mask protected
//	dataLength = header->dataLength + header->litLength + header->bssLength;
//	for ( i = 0 ; dataLength > ( 1 << i ) ; i++ ) {
//	}
//	dataLength = 1 << i;
//
//	// allocate zero filled space for initialized and uninitialized data
//	vm->dataBase = static_cast<byte*> (Hunk_Alloc( dataLength, h_high ));
//	vm->dataMask = dataLength - 1;
//
//	// copy the intialized data
//	Com_Memcpy( vm->dataBase, (byte *)header + header->dataOffset, header->dataLength + header->litLength );
//
//	// byte swap the longs
//	for ( i = 0 ; i < header->dataLength ; i += 4 ) {
//		*( int * )( vm->dataBase + i ) = LittleLong( *( int * )( vm->dataBase + i ) );
//	}
//
//	// allocate space for the jump targets, which will be filled in by the compile/prep functions
//	vm->instructionPointersLength = header->instructionCount * 4;
//	vm->instructionPointers = static_cast<int*> (Hunk_Alloc( vm->instructionPointersLength, h_high ));
//
//	// copy or compile the instructions
//	vm->codeLength = header->codeLength;
//
//	if ( interpret >= VMI_COMPILED ) {
//		vm->compiled = qtrue;
//		VM_Compile( vm, header );
//	} else {
//		vm->compiled = qfalse;
//		VM_PrepareInterpreter( vm, header );
//	}
//
//	// free the original file
//	FS_FreeFile( header );
//
//	// load the map file
//	VM_LoadSymbols( vm );
//
//	// the stack is implicitly at the end of the image
//	vm->programStack = vm->dataMask + 1;
//	vm->stackBottom = vm->programStack - STACK_SIZE;
//
//	Com_Printf( "%s loaded in %d bytes on the hunk\n", module, remaining - Hunk_MemoryRemaining() );
//
//	return vm;
//}

vm_t* VM_Create (
	const char* module,
	int32_t (*systemCalls) (intptr_t*))
{
	if ((module == 0) || (module[0] == '\0') || (systemCalls == 0))
		Com_Error (ERR_FATAL, "VM_Create: bad parms");

	int remaining = Hunk_MemoryRemaining ();

	int i;
	vm_t* vm = 0;

	// see if we already have the VM
	for (i = 0; i < MAX_VM; ++i) {
		if (Q_stricmp (vmTable[i].name, module) == 0) {
			vm = &vmTable[i];
			return vm;
		}
	}

	// find a free vm
	for (i = 0; i < MAX_VM; ++i) {
		if (vmTable[i].name[0] == '\0')
			break;
	}

	if (i == MAX_VM) {
		Com_Error (ERR_FATAL, "VM_Create: no free vm_t");
	}

	vm = &vmTable[i];

	Q_strncpyz (vm->name, module, sizeof (vm->name));
	vm->systemCall = systemCalls;

	vm->dllHandle = Sys_LoadDll(module, vm->fqpath, &vm->entryPoint, VM_DllSyscall);

	if (vm->dllHandle != 0)
		return vm;

	return 0;
}
// BBi

/*
==============
VM_Free
==============
*/

// BBi
//void VM_Free( vm_t *vm ) {
//
//	if ( vm->dllHandle ) {
//		Sys_UnloadDll( vm->dllHandle );
//		Com_Memset( vm, 0, sizeof( *vm ) );
//	}
//#if 0   // now automatically freed by hunk
//	if ( vm->codeBase ) {
//		Z_Free( vm->codeBase );
//	}
//	if ( vm->dataBase ) {
//		Z_Free( vm->dataBase );
//	}
//	if ( vm->instructionPointers ) {
//		Z_Free( vm->instructionPointers );
//	}
//#endif
//	Com_Memset( vm, 0, sizeof( *vm ) );
//
//	currentVM = NULL;
//	lastVM = NULL;
//}

void VM_Free (
	vm_t* vm)
{
	if (vm->dllHandle != 0)
		Sys_UnloadDll (vm->dllHandle);

	Com_Memset (vm, 0, sizeof (*vm));

	currentVM = 0;
	lastVM = 0;
}
// BBi

// BBi
//void VM_Clear( void ) {
//	int i;
//	for ( i = 0; i < MAX_VM; i++ ) {
//		if ( vmTable[i].dllHandle ) {
//			Sys_UnloadDll( vmTable[i].dllHandle );
//		}
//		Com_Memset( &vmTable[i], 0, sizeof( vm_t ) );
//	}
//	currentVM = NULL;
//	lastVM = NULL;
//}

void VM_Clear ()
{
	for (int i = 0; i < MAX_VM; ++i) {
		if (vmTable[i].dllHandle != 0)
			Sys_UnloadDll (vmTable[i].dllHandle);

		Com_Memset (&vmTable[i], 0, sizeof (vm_t));
	}

	currentVM = 0;
	lastVM = 0;
}
// BBi

// BBi
//void *VM_ArgPtr( int intValue ) {
//	if ( !intValue ) {
//		return NULL;
//	}
//	// bk001220 - currentVM is missing on reconnect
//	if ( currentVM == NULL ) {
//		return NULL;
//	}
//
//	if ( currentVM->entryPoint ) {
//		return ( void * )( currentVM->dataBase + intValue );
//	} else {
//		return ( void * )( currentVM->dataBase + ( intValue & currentVM->dataMask ) );
//	}
//}

#if FIXME
void* VM_ArgPtr(
	intptr_t intValue)
{
	return VM_ExplicitArgPtr(currentVM, intValue);
}
#else
intptr_t VM_ArgPtr(
	intptr_t intValue)
{
	return VM_ExplicitArgPtr(currentVM, intValue);
}
#endif // FIXME
// BBi

// BBi
//void *VM_ExplicitArgPtr( vm_t *vm, int intValue ) {
//	if ( !intValue ) {
//		return NULL;
//	}
//
//	// bk010124 - currentVM is missing on reconnect here as well?
//	if ( currentVM == NULL ) {
//		return NULL;
//	}
//
//	//
//	if ( vm->entryPoint ) {
//		return ( void * )( vm->dataBase + intValue );
//	} else {
//		return ( void * )( vm->dataBase + ( intValue & vm->dataMask ) );
//	}
//}

#if FIXME
void* VM_ExplicitArgPtr(
	vm_t* vm,
	intptr_t intValue)
{
	if (intValue == 0)
	{
		return NULL;
	}

	// bk010124 - currentVM is missing on reconnect here as well?
	if (vm == NULL)
	{
		return NULL;
	}

	return reinterpret_cast<void*>(intValue);
}
#else
intptr_t VM_ExplicitArgPtr(
	vm_t* vm,
	intptr_t intValue)
{
	// bk010124 - currentVM is missing on reconnect here as well?
	return (vm != NULL) ? intValue : 0;
}
#endif // FIXME
// BBi

/*
==============
VM_Call


Upon a system call, the stack will look like:

sp+32	parm1
sp+28	parm0
sp+24	return value
sp+20	return address
sp+16	local1
sp+14	local0
sp+12	arg1
sp+8	arg0
sp+4	return stack
sp		return address

An interpreted function will immediately execute
an OP_ENTER instruction, which will subtract space for
locals from sp
==============
*/

// BBi
//#define MAX_STACK   256
//#define STACK_MASK  ( MAX_STACK - 1 )
//
//int QDECL VM_Call( vm_t *vm, int callnum, ... ) {
//	vm_t    *oldVM;
//	int r;
//	//rcg010207 see dissertation at top of VM_DllSyscall() in this file.
//#if ( ( defined __linux__ ) && ( defined __powerpc__ ) )
//	int i;
//	int args[16];
//	va_list ap;
//#endif
//
//#if defined RTCW_MP
//	// DHM - Nerve
//#ifdef UPDATE_SERVER
//	return 0;
//#endif
//#endif // RTCW_XX
//
//	if ( !vm ) {
//		Com_Error( ERR_FATAL, "VM_Call with NULL vm" );
//	}
//
//	oldVM = currentVM;
//	currentVM = vm;
//	lastVM = vm;
//
//	if ( vm_debugLevel ) {
//		Com_Printf( "VM_Call( %i )\n", callnum );
//	}
//
//	// if we have a dll loaded, call it directly
//	if ( vm->entryPoint ) {
//		//rcg010207 -  see dissertation at top of VM_DllSyscall() in this file.
//#if ( ( defined __linux__ ) && ( defined __powerpc__ ) ) || ( defined MACOS_X )
//		va_start( ap, callnum );
//		for ( i = 0; i < sizeof( args ) / sizeof( args[i] ); i++ )
//			args[i] = va_arg( ap, int );
//		va_end( ap );
//
//		r = vm->entryPoint( callnum,  args[0],  args[1],  args[2], args[3],
//							args[4],  args[5],  args[6], args[7],
//							args[8],  args[9], args[10], args[11],
//							args[12], args[13], args[14], args[15] );
//#else // PPC above, original id code below
//		r = vm->entryPoint( ( &callnum )[0], ( &callnum )[1], ( &callnum )[2], ( &callnum )[3],
//							( &callnum )[4], ( &callnum )[5], ( &callnum )[6], ( &callnum )[7],
//							( &callnum )[8],  ( &callnum )[9],  ( &callnum )[10],  ( &callnum )[11],  ( &callnum )[12] );
//#endif
//	} else if ( vm->compiled ) {
//		r = VM_CallCompiled( vm, &callnum );
//	} else {
//		r = VM_CallInterpreted( vm, &callnum );
//	}
//
//	if ( oldVM != NULL ) { // bk001220 - assert(currentVM!=NULL) for oldVM==NULL
//		currentVM = oldVM;
//	}
//	return r;
//}

int32_t QDECL VM_Call(
	vm_t* vm,
	intptr_t callnum,
	...)
{
#if FIXME
	int r = 0;
#else
	int32_t r = 0;
#endif // FIXME

	//rcg010207 see dissertation at top of VM_DllSyscall() in this file.

#if defined RTCW_MP && defined UPDATE_SERVER
	// DHM - Nerve
	return 0;
#endif // RTCW_XX

	if (vm == 0)
		Com_Error (ERR_FATAL, "VM_Call with NULL vm");

	vm_t* oldVM = currentVM;
	currentVM = vm;
	lastVM = vm;

	if (vm_debugLevel != 0)
		// FIXME
		Com_Printf("VM_Call( %i )\n", static_cast<int32_t>(callnum));

	// if we have a dll loaded, call it directly
	if ( vm->entryPoint != 0) {
		//rcg010207 -  see dissertation at top of VM_DllSyscall() in this file.
		intptr_t args[16];

		va_list ap;

		va_start(ap, callnum);

		for (int i = 0; i < 16; ++i)
		{
			args[i] = va_arg(ap, intptr_t);
		}

		va_end(ap);

		r = vm->entryPoint(
			callnum,
			args[0],
			args[1],
			args[2],
			args[3],
			args[4],
			args[5],
			args[6],
			args[7],
			args[8],
			args[9],
			args[10],
			args[11],
			args[12],
			args[13],
			args[14],
			args[15]
		);
	}

	if (oldVM != 0)
	{
		// bk001220 - assert(currentVM!=NULL) for oldVM==NULL
		currentVM = oldVM;
	}

	return r;
}
// BBi

//=================================================================

// BBi
//static int QDECL VM_ProfileSort( const void *a, const void *b ) {
//	vmSymbol_t  *sa, *sb;
//
//	sa = *(vmSymbol_t **)a;
//	sb = *(vmSymbol_t **)b;
//
//	if ( sa->profileCount < sb->profileCount ) {
//		return -1;
//	}
//	if ( sa->profileCount > sb->profileCount ) {
//		return 1;
//	}
//	return 0;
//}
// BBi

/*
==============
VM_VmProfile_f

==============
*/

// BBi
//void VM_VmProfile_f( void ) {
//	vm_t        *vm;
//	vmSymbol_t  **sorted, *sym;
//	int i;
//	double total;
//
//	if ( !lastVM ) {
//		return;
//	}
//
//	vm = lastVM;
//
//	if ( !vm->numSymbols ) {
//		return;
//	}
//
//	sorted = static_cast<vmSymbol_t**> (Z_Malloc( vm->numSymbols * sizeof( *sorted ) ));
//	sorted[0] = vm->symbols;
//	total = sorted[0]->profileCount;
//	for ( i = 1 ; i < vm->numSymbols ; i++ ) {
//		sorted[i] = sorted[i - 1]->next;
//		total += sorted[i]->profileCount;
//	}
//
//	qsort( sorted, vm->numSymbols, sizeof( *sorted ), VM_ProfileSort );
//
//	for ( i = 0 ; i < vm->numSymbols ; i++ ) {
//		int perc;
//
//		sym = sorted[i];
//
//		perc = 100 * (float) sym->profileCount / total;
//		Com_Printf( "%2i%% %9i %s\n", perc, sym->profileCount, sym->symName );
//		sym->profileCount = 0;
//	}
//
//	Com_Printf( "    %9.0f total\n", total );
//
//	Z_Free( sorted );
//}

void VM_VmProfile_f ()
{
}
// BBi

/*
==============
VM_VmInfo_f

==============
*/

// BBi
//void VM_VmInfo_f( void ) {
//	vm_t    *vm;
//	int i;
//
//	Com_Printf( "Registered virtual machines:\n" );
//	for ( i = 0 ; i < MAX_VM ; i++ ) {
//		vm = &vmTable[i];
//		if ( !vm->name[0] ) {
//			break;
//		}
//		Com_Printf( "%s : ", vm->name );
//		if ( vm->dllHandle ) {
//			Com_Printf( "native\n" );
//			continue;
//		}
//		if ( vm->compiled ) {
//			Com_Printf( "compiled on load\n" );
//		} else {
//			Com_Printf( "interpreted\n" );
//		}
//		Com_Printf( "    code length : %7i\n", vm->codeLength );
//		Com_Printf( "    table length: %7i\n", vm->instructionPointersLength );
//		Com_Printf( "    data length : %7i\n", vm->dataMask + 1 );
//	}
//}

void VM_VmInfo_f ()
{
	Com_Printf ("Registered virtual machines:\n");

	for (int i = 0; i < MAX_VM; ++i) {
		vm_t* vm = &vmTable[i];

		if (vm->name[0] == '\0')
			break;

		Com_Printf ("%s : ", vm->name);

		if (vm->dllHandle != 0) {
			Com_Printf ("native\n");
			continue;
		}
	}
}
// BBi

/*
===============
VM_LogSyscalls

Insert calls to this while debugging the vm compiler
===============
*/

// BBi
//void VM_LogSyscalls( int *args ) {
//	static int callnum;
//	static FILE    *f;
//
//	if ( !f ) {
//		f = fopen( "syscalls.log", "w" );
//	}
//	callnum++;
//	fprintf( f, "%i: %i (%i) = %i %i %i %i\n", callnum, args - (int *)currentVM->dataBase,
//			 args[0], args[1], args[2], args[3], args[4] );
//}

void VM_LogSyscalls (
	int* args)
{
	static int callnum;
	static FILE* f;

	if (f == 0)
		f = fopen ("syscalls.log", "w");

	++callnum;

	fprintf (f, "%i: (%i) = %i %i %i %i\n",
		callnum, args[0], args[1], args[2], args[3], args[4]);
}
// BBi

// BBi
//#ifdef DLL_ONLY // bk010215 - for DLL_ONLY dedicated servers/builds w/o VM
//int VM_CallCompiled( vm_t *vm, int *args ) {
//	return( 0 );
//}
//
//void VM_Compile( vm_t *vm, vmHeader_t *header ) {}
//#endif // DLL_ONLY
// BBi
