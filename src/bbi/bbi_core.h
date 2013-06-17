//
// Custom library.
// Copyright (C) 2012-2013 Boris I. Bendovsky
//
// Core checks, defines and basic types.
//


#ifndef BBI_CORE_H
#define BBI_CORE_H


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// Defines
//

#ifndef __cplusplus
    #error C++ compiler required.
#endif


#undef BBI_LE
#define BBI_LE (1L)

#undef BBI_BE
#define BBI_BE (2L)

#if (!defined BBI_ENDIANNESS) || (BBI_ENDIANNESS == 0L)
    #if (defined _M_AMD64) || (defined _M_IX86) || (defined _M_X64)
        #define BBI_ENDIANNESS BBI_LE
    #endif
#endif

#if (!defined BBI_ENDIANNESS) || (BBI_ENDIANNESS == 0L)
    #error Undefined platform's endianness (BBI_ENDIANNESS).
#endif


#undef BBI_MSC
#define BBI_MSC (1L)


#undef BBI_X86
#define BBI_X86 (1L)

#undef BBI_X64
#define BBI_X64 (2L)


#undef BBI_WIN32
#define BBI_WIN32 (1L)


#ifdef _WIN32
    #define BBI_PLATFORM BBI_WIN32

    #ifdef _WIN64
        #define BBI_PLATFORM_ARCH BBI_X64
    #else
        #define BBI_PLATFORM_ARCH BBI_X86
    #endif

    #ifdef _MSC_VER
        #if _MSC_VER < 1700L
            #error Visual C++ 2012 or higher required.
        #endif

        #define BBI_PLATFORM_BUILD_TOOL BBI_MSC
        #define BBI_PLATFORM_BUILD_TOOL_VERSION _MSC_VER
    #endif

    #ifndef BBI_SYS_API
        #define BBI_SYS_API __stdcall
    #endif
#endif // _WIN32


#ifndef BBI_PLATFORM
    #error Unknown platform (BBI_PLATFORM).
#endif

#ifndef BBI_PLATFORM_BUILD_TOOL
    #error Unknown platform build tool (BBI_PLATFORM_BUILD_TOOL).
#endif

#ifndef BBI_PLATFORM_ARCH
    #error Unknown platform arhitecture (BBI_PLATFORM_ARCH).
#endif


#ifndef BBI_SYS_API
    #define BBI_SYS_API
#endif


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// Includes
//

#include <cstddef>
#include <cstdint>

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// Basic types
//

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// Constants
//

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#endif // BBI_CORE_H
