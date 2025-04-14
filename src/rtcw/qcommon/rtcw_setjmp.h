/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_SETJMP_INCLUDED
#define RTCW_SETJMP_INCLUDED

#if defined(__MINGW32__) && defined(__GNUC__) && !defined(__llvm__) && (defined(__x86_64__) || defined(_M_X64))
	#include <stdint.h>

	typedef intptr_t RTCW_JMP_BUF[5];
	#define RTCW_SETJMP(env) __builtin_setjmp(env)
	#define RTCW_LONGJMP(env,status) __builtin_longjmp(env,1)
#else
	#include <setjmp.h>

	typedef jmp_buf RTCW_JMP_BUF;
	#define RTCW_SETJMP(env) setjmp(env)
	#define RTCW_LONGJMP(env,status) longjmp(env,status)
#endif

#endif // RTCW_SETJMP_INCLUDED
