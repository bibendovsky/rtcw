/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "q_shared.h"
#include "qcommon.h"

// BBi
//typedef enum {
//	OP_UNDEF,
//
//	OP_IGNORE,
//
//	OP_BREAK,
//
//	OP_ENTER,
//	OP_LEAVE,
//	OP_CALL,
//	OP_PUSH,
//	OP_POP,
//
//	OP_CONST,
//	OP_LOCAL,
//
//	OP_JUMP,
//
//	//-------------------
//
//	OP_EQ,
//	OP_NE,
//
//	OP_LTI,
//	OP_LEI,
//	OP_GTI,
//	OP_GEI,
//
//	OP_LTU,
//	OP_LEU,
//	OP_GTU,
//	OP_GEU,
//
//	OP_EQF,
//	OP_NEF,
//
//	OP_LTF,
//	OP_LEF,
//	OP_GTF,
//	OP_GEF,
//
//	//-------------------
//
//	OP_LOAD1,
//	OP_LOAD2,
//	OP_LOAD4,
//	OP_STORE1,
//	OP_STORE2,
//	OP_STORE4,              // *(stack[top-1]) = stack[top]
//	OP_ARG,
//
//	OP_BLOCK_COPY,
//
//	//-------------------
//
//	OP_SEX8,
//	OP_SEX16,
//
//	OP_NEGI,
//	OP_ADD,
//	OP_SUB,
//	OP_DIVI,
//	OP_DIVU,
//	OP_MODI,
//	OP_MODU,
//	OP_MULI,
//	OP_MULU,
//
//	OP_BAND,
//	OP_BOR,
//	OP_BXOR,
//	OP_BCOM,
//
//	OP_LSH,
//	OP_RSHI,
//	OP_RSHU,
//
//	OP_NEGF,
//	OP_ADDF,
//	OP_SUBF,
//	OP_DIVF,
//	OP_MULF,
//
//	OP_CVIF,
//	OP_CVFI
//} opcode_t;
//
//
//typedef int vmptr_t;
//
//typedef struct vmSymbol_s {
//	struct vmSymbol_s   *next;
//	int symValue;
//	int profileCount;
//	char symName[1];        // variable sized
//} vmSymbol_t;
//
//#define VM_OFFSET_PROGRAM_STACK     0
//#define VM_OFFSET_SYSTEM_CALL       4
// BBi

struct vm_s {
	// DO NOT MOVE OR CHANGE THESE WITHOUT CHANGING THE VM_OFFSET_* DEFINES
	// USED BY THE ASM CODE

	// BBi
	//int programStack;               // the vm may be recursively entered
	// BBi

	// BBi
	//int (*systemCall) (int* parms);
	int32_t (*systemCall)(intptr_t* parms);
	// BBi

	//------------------------------------

	char name[MAX_QPATH];

// fqpath member added 2/15/02 by T.Ray
	char fqpath[MAX_QPATH + 1];

	// for dynamic linked modules
	void        *dllHandle;

	// BBi
	//int (QDECL* entryPoint) (int callNum, ...);
	int32_t (QDECL* entryPoint) (intptr_t callNum, ...);
	// BBi

	// BBi
	//// for interpreted modules
	//qboolean currentlyInterpreting;

	//qboolean compiled;
	//byte        *codeBase;
	//int codeLength;

	//int         *instructionPointers;
	//int instructionPointersLength;

	//byte        *dataBase;
	//int dataMask;

	//int stackBottom;                // if programStack < stackBottom, error

	//int numSymbols;
	//struct vmSymbol_s   *symbols;

	//int callLevel;                  // for debug indenting
	//int breakFunction;              // increment breakCount on function entry to this
	//int breakCount;
	// BBi
};


extern vm_t    *currentVM;
extern int vm_debugLevel;

// BBi
//void VM_Compile( vm_t *vm, vmHeader_t *header );
//int VM_CallCompiled( vm_t *vm, int *args );
//
//void VM_PrepareInterpreter( vm_t *vm, vmHeader_t *header );
//int VM_CallInterpreted( vm_t *vm, int *args );
//
//vmSymbol_t *VM_ValueToFunctionSymbol( vm_t *vm, int value );
//int VM_SymbolToValue( vm_t *vm, const char *symbol );
//const char *VM_ValueToSymbol( vm_t *vm, int value );
// BBi

void VM_LogSyscalls( int *args );
