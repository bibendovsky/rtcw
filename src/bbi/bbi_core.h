//
// Custom library.
// Copyright (C) 2012 Boris I. Bendovsky
//
// Core checks, defines and basic types.
//


#ifndef BBI_CORE_H
#define BBI_CORE_H


#include <cstddef>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// Defines
//

#ifndef __cplusplus
    #error C++ required.
#endif // BBI_CXX_STANDARD


#define BBI_LE 1L
#define BBI_BE 2L

#if (!defined BBI_ENDIANNESS) || (BBI_ENDIANNESS == 0L)

    #if (defined _M_AMD64) || (defined _M_IX86) || (defined _M_X64)
        #define BBI_ENDIANNESS BBI_LE
    #endif

#endif

#if (!defined BBI_ENDIANNESS) || (BBI_ENDIANNESS == 0L)
    #error Undefined platform's endianness (BBI_ENDIANNESS)
#endif


#ifndef BBI_MSC
    #define BBI_MSC 1L
#endif // BBI_MSC


#ifndef BBI_X86
    #define BBI_X86 1L
#endif // BBI_X86

#ifndef BBI_X64
    #define BBI_X64 2L
#endif // BBI_X64


#ifndef BBI_WIN32
    #define BBI_WIN32 1L
#endif // BBI_WIN32


#ifdef _WIN32
    #define BBI_PLATFORM BBI_WIN32

    #ifdef _WIN64
        #define BBI_PLATFORM_ARCH BBI_X64
    #else
        #define BBI_PLATFORM_ARCH BBI_X86
    #endif // _WIN64

    #ifdef _MSC_VER
        #if _MSC_VER < 1700L
            #error Visual C++ 2012 required.
        #endif

        #define BBI_PLATFORM_BUILD_TOOL BBI_MSC
        #define BBI_PLATFORM_BUILD_TOOL_VERSION _MSC_VER
    #endif // _MSC_VER

    #ifndef BBI_SYS_API
        #define BBI_SYS_API __stdcall
    #endif // BBI_SYS_API
#endif // _WIN32


#ifndef BBI_PLATFORM
    #error Unknown platform (BBI_PLATFORM).
#endif // BBI_PLATFORM

#ifndef BBI_PLATFORM_BUILD_TOOL
    #error Unknown platform build tool (BBI_PLATFORM_BUILD_TOOL).
#endif // BBI_PLATFORM_BUILD_TOOL

#ifndef BBI_PLATFORM_ARCH
    #error Unknown platform arhitecture (BBI_PLATFORM_ARCH).
#endif // BBI_PLATFORM_ARCH


#ifndef BBI_SYS_API
    #define BBI_SYS_API
#endif // BBI_SYS_API


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// Basic types
//

namespace bbi {


#if BBI_PLATFORM_BUILD_TOOL == BBI_MSC
    typedef signed __int8 Int8;
    typedef unsigned __int8 UInt8;

    typedef signed __int16 Int16;
    typedef unsigned __int16 UInt16;

    typedef signed __int32 Int32;
    typedef unsigned __int32 UInt32;

    typedef signed __int64 Int64;
    typedef unsigned __int64 UInt64;
#endif // BBI_PLATFORM_BUILD_TOOL

typedef signed char SChar;
typedef unsigned char UChar;
typedef unsigned short int UShort;
typedef unsigned int UInt;
typedef unsigned long int ULong;
typedef unsigned long long int ULLong;

typedef ptrdiff_t IntPtr;
typedef size_t UIntPtr;


} // namespace bbi

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// Constants
//

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#endif // BBI_CORE_H
