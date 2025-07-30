/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifdef _WIN32

#ifndef COM_NO_WINDOWS_H
#define COM_NO_WINDOWS_H
#endif

#include "rtcw_hdr_mgr.h"

#include <cassert>
#include <cstddef>

// DXGI
#include "rpc.h"
#include "rpcndr.h"

// ======================================

namespace rtcw {

namespace {

// ======================================
// DXGI common stuff

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

// ======================================
// dxgicommon.h

#ifndef CONST_VTBL
#define CONST_VTBL const
#endif /* CONST_VTBL */

typedef struct DXGI_RATIONAL
{
	UINT Numerator;
	UINT Denominator;
} DXGI_RATIONAL;

/* The following values are used with DXGI_SAMPLE_DESC::Quality: */
#define DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN 0xFFFFFFFF
#define DXGI_CENTER_MULTISAMPLE_QUALITY_PATTERN   0xFFFFFFFE

typedef struct DXGI_SAMPLE_DESC
{
	UINT Count;
	UINT Quality;
} DXGI_SAMPLE_DESC;

typedef enum DXGI_COLOR_SPACE_TYPE
{
	DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709           = 0,
	DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709           = 1,
	DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709         = 2,
	DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020        = 3,
	DXGI_COLOR_SPACE_RESERVED                         = 4,
	DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601    = 5,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601       = 6,
	DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601         = 7,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709       = 8,
	DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709         = 9,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020      = 10,
	DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020        = 11,
	DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020        = 12,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020    = 13,
	DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020      = 14,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020   = 15,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020 = 16,
	DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020          = 17,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_GHLG_TOPLEFT_P2020  = 18,
	DXGI_COLOR_SPACE_YCBCR_FULL_GHLG_TOPLEFT_P2020    = 19,
	DXGI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P709         = 20,
	DXGI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P2020        = 21,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_LEFT_P709       = 22,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_LEFT_P2020      = 23,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_TOPLEFT_P2020   = 24,
	DXGI_COLOR_SPACE_CUSTOM                           = 0xFFFFFFFF
} DXGI_COLOR_SPACE_TYPE;

// ======================================
// dxgiformat.h

#define DXGI_FORMAT_DEFINED 1

typedef enum DXGI_FORMAT
{
	DXGI_FORMAT_UNKNOWN	                                = 0,
	DXGI_FORMAT_R32G32B32A32_TYPELESS                   = 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT                      = 2,
	DXGI_FORMAT_R32G32B32A32_UINT                       = 3,
	DXGI_FORMAT_R32G32B32A32_SINT                       = 4,
	DXGI_FORMAT_R32G32B32_TYPELESS                      = 5,
	DXGI_FORMAT_R32G32B32_FLOAT                         = 6,
	DXGI_FORMAT_R32G32B32_UINT                          = 7,
	DXGI_FORMAT_R32G32B32_SINT                          = 8,
	DXGI_FORMAT_R16G16B16A16_TYPELESS                   = 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT                      = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM                      = 11,
	DXGI_FORMAT_R16G16B16A16_UINT                       = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM                      = 13,
	DXGI_FORMAT_R16G16B16A16_SINT                       = 14,
	DXGI_FORMAT_R32G32_TYPELESS                         = 15,
	DXGI_FORMAT_R32G32_FLOAT                            = 16,
	DXGI_FORMAT_R32G32_UINT                             = 17,
	DXGI_FORMAT_R32G32_SINT                             = 18,
	DXGI_FORMAT_R32G8X24_TYPELESS                       = 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT                    = 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS                = 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT                 = 22,
	DXGI_FORMAT_R10G10B10A2_TYPELESS                    = 23,
	DXGI_FORMAT_R10G10B10A2_UNORM                       = 24,
	DXGI_FORMAT_R10G10B10A2_UINT                        = 25,
	DXGI_FORMAT_R11G11B10_FLOAT                         = 26,
	DXGI_FORMAT_R8G8B8A8_TYPELESS                       = 27,
	DXGI_FORMAT_R8G8B8A8_UNORM                          = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB                     = 29,
	DXGI_FORMAT_R8G8B8A8_UINT                           = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM                          = 31,
	DXGI_FORMAT_R8G8B8A8_SINT                           = 32,
	DXGI_FORMAT_R16G16_TYPELESS                         = 33,
	DXGI_FORMAT_R16G16_FLOAT                            = 34,
	DXGI_FORMAT_R16G16_UNORM                            = 35,
	DXGI_FORMAT_R16G16_UINT                             = 36,
	DXGI_FORMAT_R16G16_SNORM                            = 37,
	DXGI_FORMAT_R16G16_SINT                             = 38,
	DXGI_FORMAT_R32_TYPELESS                            = 39,
	DXGI_FORMAT_D32_FLOAT                               = 40,
	DXGI_FORMAT_R32_FLOAT                               = 41,
	DXGI_FORMAT_R32_UINT                                = 42,
	DXGI_FORMAT_R32_SINT                                = 43,
	DXGI_FORMAT_R24G8_TYPELESS                          = 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT                       = 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS                   = 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT                    = 47,
	DXGI_FORMAT_R8G8_TYPELESS                           = 48,
	DXGI_FORMAT_R8G8_UNORM                              = 49,
	DXGI_FORMAT_R8G8_UINT                               = 50,
	DXGI_FORMAT_R8G8_SNORM                              = 51,
	DXGI_FORMAT_R8G8_SINT                               = 52,
	DXGI_FORMAT_R16_TYPELESS                            = 53,
	DXGI_FORMAT_R16_FLOAT                               = 54,
	DXGI_FORMAT_D16_UNORM                               = 55,
	DXGI_FORMAT_R16_UNORM                               = 56,
	DXGI_FORMAT_R16_UINT                                = 57,
	DXGI_FORMAT_R16_SNORM                               = 58,
	DXGI_FORMAT_R16_SINT                                = 59,
	DXGI_FORMAT_R8_TYPELESS                             = 60,
	DXGI_FORMAT_R8_UNORM                                = 61,
	DXGI_FORMAT_R8_UINT                                 = 62,
	DXGI_FORMAT_R8_SNORM                                = 63,
	DXGI_FORMAT_R8_SINT                                 = 64,
	DXGI_FORMAT_A8_UNORM                                = 65,
	DXGI_FORMAT_R1_UNORM                                = 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP                      = 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM                         = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM                         = 69,
	DXGI_FORMAT_BC1_TYPELESS                            = 70,
	DXGI_FORMAT_BC1_UNORM                               = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB                          = 72,
	DXGI_FORMAT_BC2_TYPELESS                            = 73,
	DXGI_FORMAT_BC2_UNORM                               = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB                          = 75,
	DXGI_FORMAT_BC3_TYPELESS                            = 76,
	DXGI_FORMAT_BC3_UNORM                               = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB                          = 78,
	DXGI_FORMAT_BC4_TYPELESS                            = 79,
	DXGI_FORMAT_BC4_UNORM                               = 80,
	DXGI_FORMAT_BC4_SNORM                               = 81,
	DXGI_FORMAT_BC5_TYPELESS                            = 82,
	DXGI_FORMAT_BC5_UNORM                               = 83,
	DXGI_FORMAT_BC5_SNORM                               = 84,
	DXGI_FORMAT_B5G6R5_UNORM                            = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM                          = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM                          = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM                          = 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM              = 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS                       = 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB                     = 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS                       = 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB                     = 93,
	DXGI_FORMAT_BC6H_TYPELESS                           = 94,
	DXGI_FORMAT_BC6H_UF16                               = 95,
	DXGI_FORMAT_BC6H_SF16                               = 96,
	DXGI_FORMAT_BC7_TYPELESS                            = 97,
	DXGI_FORMAT_BC7_UNORM                               = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB                          = 99,
	DXGI_FORMAT_AYUV                                    = 100,
	DXGI_FORMAT_Y410                                    = 101,
	DXGI_FORMAT_Y416                                    = 102,
	DXGI_FORMAT_NV12                                    = 103,
	DXGI_FORMAT_P010                                    = 104,
	DXGI_FORMAT_P016                                    = 105,
	DXGI_FORMAT_420_OPAQUE                              = 106,
	DXGI_FORMAT_YUY2                                    = 107,
	DXGI_FORMAT_Y210                                    = 108,
	DXGI_FORMAT_Y216                                    = 109,
	DXGI_FORMAT_NV11                                    = 110,
	DXGI_FORMAT_AI44                                    = 111,
	DXGI_FORMAT_IA44                                    = 112,
	DXGI_FORMAT_P8                                      = 113,
	DXGI_FORMAT_A8P8                                    = 114,
	DXGI_FORMAT_B4G4R4A4_UNORM                          = 115,

	DXGI_FORMAT_P208                                    = 130,
	DXGI_FORMAT_V208                                    = 131,
	DXGI_FORMAT_V408                                    = 132,

	DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE         = 189,
	DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE = 190,

	DXGI_FORMAT_A4B4G4R4_UNORM                          = 191,

	DXGI_FORMAT_FORCE_UINT                              = 0xFFFFFFFF
} DXGI_FORMAT;

// ======================================
// dxgitype.h

#define _FACDXGI 0x87a

#define MAKE_DXGI_HRESULT(code) MAKE_HRESULT(1, _FACDXGI, code)
#define MAKE_DXGI_STATUS(code)  MAKE_HRESULT(0, _FACDXGI, code)

/* DXGI error messages have moved to winerror.h */

#define DXGI_CPU_ACCESS_NONE       (0)
#define DXGI_CPU_ACCESS_DYNAMIC    (1)
#define DXGI_CPU_ACCESS_READ_WRITE (2)
#define DXGI_CPU_ACCESS_SCRATCH    (3)
#define DXGI_CPU_ACCESS_FIELD      15

typedef struct DXGI_RGB
{
	float Red;
	float Green;
	float Blue;
} DXGI_RGB;

#ifndef D3DCOLORVALUE_DEFINED
typedef struct _D3DCOLORVALUE
{
	float r;
	float g;
	float b;
	float a;
} D3DCOLORVALUE;

#define D3DCOLORVALUE_DEFINED
#endif

typedef D3DCOLORVALUE DXGI_RGBA;

typedef struct DXGI_GAMMA_CONTROL
{
	DXGI_RGB Scale;
	DXGI_RGB Offset;
	DXGI_RGB GammaCurve[1025];
} DXGI_GAMMA_CONTROL;

typedef struct DXGI_GAMMA_CONTROL_CAPABILITIES
{
	BOOL  ScaleAndOffsetSupported;
	float MaxConvertedValue;
	float MinConvertedValue;
	UINT  NumGammaControlPoints;
	float ControlPointPositions[1025];
} DXGI_GAMMA_CONTROL_CAPABILITIES;

typedef enum DXGI_MODE_SCANLINE_ORDER
{
	DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED = 0,
	DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE = 1,
	DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST = 2,
	DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST = 3
} DXGI_MODE_SCANLINE_ORDER;

typedef enum DXGI_MODE_SCALING
{
	DXGI_MODE_SCALING_UNSPECIFIED = 0,
	DXGI_MODE_SCALING_CENTERED = 1,
	DXGI_MODE_SCALING_STRETCHED = 2
} DXGI_MODE_SCALING;

typedef enum DXGI_MODE_ROTATION
{
	DXGI_MODE_ROTATION_UNSPECIFIED = 0,
	DXGI_MODE_ROTATION_IDENTITY = 1,
	DXGI_MODE_ROTATION_ROTATE90 = 2,
	DXGI_MODE_ROTATION_ROTATE180 = 3,
	DXGI_MODE_ROTATION_ROTATE270 = 4
} DXGI_MODE_ROTATION;

typedef struct DXGI_MODE_DESC
{
	UINT                     Width;
	UINT                     Height;
	DXGI_RATIONAL            RefreshRate;
	DXGI_FORMAT              Format;
	DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;
	DXGI_MODE_SCALING        Scaling;
} DXGI_MODE_DESC;

typedef struct DXGI_JPEG_DC_HUFFMAN_TABLE
{
	BYTE CodeCounts[12];
	BYTE CodeValues[12];
} DXGI_JPEG_DC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_AC_HUFFMAN_TABLE
{
	BYTE CodeCounts[16];
	BYTE CodeValues[162];
} DXGI_JPEG_AC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_QUANTIZATION_TABLE
{
	BYTE Elements[64];
} DXGI_JPEG_QUANTIZATION_TABLE;

// ======================================
// dxgi.h

/* Forward Declarations */

typedef interface IDXGIObject          IDXGIObject;
typedef interface IDXGIDeviceSubObject IDXGIDeviceSubObject;
typedef interface IDXGIResource        IDXGIResource;
typedef interface IDXGIKeyedMutex      IDXGIKeyedMutex;
typedef interface IDXGISurface         IDXGISurface;
typedef interface IDXGISurface1        IDXGISurface1;
typedef interface IDXGIAdapter         IDXGIAdapter;
typedef interface IDXGIOutput          IDXGIOutput;
typedef interface IDXGISwapChain       IDXGISwapChain;
typedef interface IDXGIFactory         IDXGIFactory;
typedef interface IDXGIDevice          IDXGIDevice;
typedef interface IDXGIFactory1        IDXGIFactory1;
typedef interface IDXGIAdapter1        IDXGIAdapter1;
typedef interface IDXGIDevice1         IDXGIDevice1;

#define DXGI_CPU_ACCESS_FIELD           15

#define DXGI_USAGE_SHADER_INPUT         0x00000010UL
#define DXGI_USAGE_RENDER_TARGET_OUTPUT 0x00000020UL
#define DXGI_USAGE_BACK_BUFFER          0x00000040UL
#define DXGI_USAGE_SHARED               0x00000080UL
#define DXGI_USAGE_READ_ONLY            0x00000100UL
#define DXGI_USAGE_DISCARD_ON_PRESENT   0x00000200UL
#define DXGI_USAGE_UNORDERED_ACCESS     0x00000400UL

typedef UINT DXGI_USAGE;

typedef struct DXGI_FRAME_STATISTICS
{
	UINT          PresentCount;
	UINT          PresentRefreshCount;
	UINT          SyncRefreshCount;
	LARGE_INTEGER SyncQPCTime;
	LARGE_INTEGER SyncGPUTime;
} DXGI_FRAME_STATISTICS;

typedef struct DXGI_MAPPED_RECT
{
	INT   Pitch;
	BYTE* pBits;
} DXGI_MAPPED_RECT;

typedef struct DXGI_ADAPTER_DESC
{
	WCHAR  Description[128];
	UINT   VendorId;
	UINT   DeviceId;
	UINT   SubSysId;
	UINT   Revision;
	SIZE_T DedicatedVideoMemory;
	SIZE_T DedicatedSystemMemory;
	SIZE_T SharedSystemMemory;
	LUID   AdapterLuid;
} DXGI_ADAPTER_DESC;

#if !defined(HMONITOR_DECLARED) && !defined(HMONITOR) && (WINVER < 0x0500)
#define HMONITOR_DECLARED
#if 0
typedef HANDLE HMONITOR;
#endif
DECLARE_HANDLE(HMONITOR);
#endif

typedef struct DXGI_OUTPUT_DESC
{
	WCHAR              DeviceName[32];
	RECT               DesktopCoordinates;
	BOOL               AttachedToDesktop;
	DXGI_MODE_ROTATION Rotation;
	HMONITOR           Monitor;
} DXGI_OUTPUT_DESC;

typedef struct DXGI_SHARED_RESOURCE
{
	HANDLE Handle;
} DXGI_SHARED_RESOURCE;

#define	DXGI_RESOURCE_PRIORITY_MINIMUM (0x28000000)
#define	DXGI_RESOURCE_PRIORITY_LOW     (0x50000000)
#define	DXGI_RESOURCE_PRIORITY_NORMAL  (0x78000000)
#define	DXGI_RESOURCE_PRIORITY_HIGH    (0xA0000000)
#define	DXGI_RESOURCE_PRIORITY_MAXIMUM (0xC8000000)

typedef enum DXGI_RESIDENCY
{
	DXGI_RESIDENCY_FULLY_RESIDENT            = 1,
	DXGI_RESIDENCY_RESIDENT_IN_SHARED_MEMORY = 2,
	DXGI_RESIDENCY_EVICTED_TO_DISK           = 3
} DXGI_RESIDENCY;

typedef struct DXGI_SURFACE_DESC
{
	UINT             Width;
	UINT             Height;
	DXGI_FORMAT      Format;
	DXGI_SAMPLE_DESC SampleDesc;
} DXGI_SURFACE_DESC;

typedef enum DXGI_SWAP_EFFECT
{
	DXGI_SWAP_EFFECT_DISCARD         = 0,
	DXGI_SWAP_EFFECT_SEQUENTIAL      = 1,
	DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL = 3,
	DXGI_SWAP_EFFECT_FLIP_DISCARD    = 4
} DXGI_SWAP_EFFECT;

typedef enum DXGI_SWAP_CHAIN_FLAG
{
	DXGI_SWAP_CHAIN_FLAG_NONPREROTATED                          = 1,
	DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH                      = 2,
	DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE                         = 4,
	DXGI_SWAP_CHAIN_FLAG_RESTRICTED_CONTENT                     = 8,
	DXGI_SWAP_CHAIN_FLAG_RESTRICT_SHARED_RESOURCE_DRIVER        = 16,
	DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY                           = 32,
	DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT          = 64,
	DXGI_SWAP_CHAIN_FLAG_FOREGROUND_LAYER                       = 128,
	DXGI_SWAP_CHAIN_FLAG_FULLSCREEN_VIDEO                       = 256,
	DXGI_SWAP_CHAIN_FLAG_YUV_VIDEO                              = 512,
	DXGI_SWAP_CHAIN_FLAG_HW_PROTECTED                           = 1024,
	DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING                          = 2048,
	DXGI_SWAP_CHAIN_FLAG_RESTRICTED_TO_ALL_HOLOGRAPHIC_DISPLAYS = 4096
} DXGI_SWAP_CHAIN_FLAG;

typedef struct DXGI_SWAP_CHAIN_DESC
{
	DXGI_MODE_DESC   BufferDesc;
	DXGI_SAMPLE_DESC SampleDesc;
	DXGI_USAGE       BufferUsage;
	UINT             BufferCount;
	HWND             OutputWindow;
	BOOL             Windowed;
	DXGI_SWAP_EFFECT SwapEffect;
	UINT             Flags;
} DXGI_SWAP_CHAIN_DESC;

MIDL_INTERFACE("AEC22FB8-76F3-4639-9BE0-28EB43A67A2E")
IDXGIObject : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(
		REFGUID Name,
		UINT* pDataSize,
		void* pData) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetParent(
		REFIID riid,
		void** ppParent) = 0;
};

MIDL_INTERFACE("3D3E0379-F9DE-4D58-BB6C-18D62992F1A6")
IDXGIDeviceSubObject : public IDXGIObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) = 0;
};

MIDL_INTERFACE("035F3AB4-482E-4E50-B41F-8A7F8BD8960B")
IDXGIResource : public IDXGIDeviceSubObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetSharedHandle(
		HANDLE* pSharedHandle) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetUsage(
		DXGI_USAGE* pUsage) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetEvictionPriority(
		UINT EvictionPriority) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetEvictionPriority(
		UINT* pEvictionPriority) = 0;
};

MIDL_INTERFACE("9D8E1289-D7B3-465F-8126-250E349AF85D")
IDXGIKeyedMutex : public IDXGIDeviceSubObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE AcquireSync(
		UINT64 Key,
		DWORD dwMilliseconds) = 0;
	virtual HRESULT STDMETHODCALLTYPE ReleaseSync(
		UINT64 Key) = 0;
};

#define	DXGI_MAP_READ    (1UL)
#define	DXGI_MAP_WRITE   (2UL)
#define	DXGI_MAP_DISCARD (4UL)

MIDL_INTERFACE("CAFCB56C-6AC3-4889-BF47-9E23BBD260EC")
IDXGISurface : public IDXGIDeviceSubObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDesc(
		DXGI_SURFACE_DESC* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE Map(
		DXGI_MAPPED_RECT* pLockedRect,
		UINT MapFlags) = 0;
	virtual HRESULT STDMETHODCALLTYPE Unmap() = 0;
};

MIDL_INTERFACE("4AE63092-6327-4C1B-80AE-BFE12EA32B86")
IDXGISurface1 : public IDXGISurface
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDC(
		BOOL Discard,
		HDC* phdc) = 0;
	virtual HRESULT STDMETHODCALLTYPE ReleaseDC(
		RECT* pDirtyRect) = 0;
};

MIDL_INTERFACE("2411E7E1-12AC-4CCF-BD14-9798E8534DC0")
IDXGIAdapter : public IDXGIObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE EnumOutputs(
		UINT Output,
		IDXGIOutput** ppOutput) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDesc(
		DXGI_ADAPTER_DESC* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(
		REFGUID InterfaceName,
		LARGE_INTEGER* pUMDVersion) = 0;
};

#define	DXGI_ENUM_MODES_INTERLACED (1UL)
#define	DXGI_ENUM_MODES_SCALING    (2UL)

MIDL_INTERFACE("AE02EEDB-C735-4690-8D52-5A8DC20213AA")
IDXGIOutput : public IDXGIObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDesc(
		DXGI_OUTPUT_DESC* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDisplayModeList(
		DXGI_FORMAT EnumFormat,
		UINT Flags,
		UINT* pNumModes,
		DXGI_MODE_DESC* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE FindClosestMatchingMode(
		const DXGI_MODE_DESC* pModeToMatch,
		DXGI_MODE_DESC* pClosestMatch,
		IUnknown* pConcernedDevice) = 0;
	virtual HRESULT STDMETHODCALLTYPE WaitForVBlank() = 0;
	virtual HRESULT STDMETHODCALLTYPE TakeOwnership(
		IUnknown* pDevice,
		BOOL Exclusive) = 0;
	virtual void STDMETHODCALLTYPE ReleaseOwnership() = 0;
	virtual HRESULT STDMETHODCALLTYPE GetGammaControlCapabilities(
		DXGI_GAMMA_CONTROL_CAPABILITIES* pGammaCaps) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetGammaControl(
		const DXGI_GAMMA_CONTROL* pArray) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetGammaControl(
		DXGI_GAMMA_CONTROL* pArray) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDisplaySurface(
		IDXGISurface* pScanoutSurface) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDisplaySurfaceData(
		IDXGISurface* pDestination) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFrameStatistics(
		DXGI_FRAME_STATISTICS* pStats) = 0;
};

#define DXGI_MAX_SWAP_CHAIN_BUFFERS        (16)

#define DXGI_PRESENT_TEST                  0x00000001UL
#define DXGI_PRESENT_DO_NOT_SEQUENCE       0x00000002UL
#define DXGI_PRESENT_RESTART               0x00000004UL
#define DXGI_PRESENT_DO_NOT_WAIT           0x00000008UL
#define DXGI_PRESENT_STEREO_PREFER_RIGHT   0x00000010UL
#define DXGI_PRESENT_STEREO_TEMPORARY_MONO 0x00000020UL
#define DXGI_PRESENT_RESTRICT_TO_OUTPUT    0x00000040UL
#define DXGI_PRESENT_USE_DURATION          0x00000100UL
#define DXGI_PRESENT_ALLOW_TEARING         0x00000200UL

MIDL_INTERFACE("310D36A0-D2E7-4C0A-AA04-6A9D23B8886A")
IDXGISwapChain : public IDXGIDeviceSubObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE Present(
		UINT SyncInterval,
		UINT Flags) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetBuffer(
		UINT Buffer,
		REFIID riid,
		void** ppSurface) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetFullscreenState(
		BOOL Fullscreen,
		IDXGIOutput* pTarget) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFullscreenState(
		BOOL* pFullscreen,
		IDXGIOutput** ppTarget) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDesc(
		DXGI_SWAP_CHAIN_DESC* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE ResizeBuffers(
		UINT BufferCount,
		UINT Width,
		UINT Height,
		DXGI_FORMAT NewFormat,
		UINT SwapChainFlags) = 0;
	virtual HRESULT STDMETHODCALLTYPE ResizeTarget(
		const DXGI_MODE_DESC* pNewTargetParameters) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetContainingOutput(
		IDXGIOutput** ppOutput) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFrameStatistics(
		DXGI_FRAME_STATISTICS* pStats) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetLastPresentCount(
		UINT* pLastPresentCount) = 0;
};

#define DXGI_MWA_NO_WINDOW_CHANGES (1 << 0)
#define DXGI_MWA_NO_ALT_ENTER      (1 << 1)
#define DXGI_MWA_NO_PRINT_SCREEN   (1 << 2)
#define DXGI_MWA_VALID             (0x7)

MIDL_INTERFACE("7B7166EC-21C7-44AE-B21A-C9AE321AE369")
IDXGIFactory : public IDXGIObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE EnumAdapters(
		UINT Adapter,
		IDXGIAdapter** ppAdapter) = 0;
	virtual HRESULT STDMETHODCALLTYPE MakeWindowAssociation(
		HWND WindowHandle,
		UINT Flags) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetWindowAssociation(
		HWND* pWindowHandle) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateSwapChain(
		IUnknown* pDevice,
		DXGI_SWAP_CHAIN_DESC* pDesc,
		IDXGISwapChain** ppSwapChain) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(
		HMODULE Module,
		IDXGIAdapter** ppAdapter) = 0;
};

MIDL_INTERFACE("54EC77FA-1377-44E6-8C32-88FD5F44C84C")
IDXGIDevice : public IDXGIObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetAdapter(
		IDXGIAdapter** pAdapter) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateSurface(
		const DXGI_SURFACE_DESC* pDesc,
		UINT NumSurfaces,
		DXGI_USAGE Usage,
		const DXGI_SHARED_RESOURCE* pSharedResource,
		IDXGISurface** ppSurface) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryResourceResidency(
		IUnknown* const* ppResources,
		DXGI_RESIDENCY* pResidencyStatus,
		UINT NumResources) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetGPUThreadPriority(
		INT Priority) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetGPUThreadPriority(
		INT* pPriority) = 0;
};

typedef enum DXGI_ADAPTER_FLAG
{
	DXGI_ADAPTER_FLAG_NONE        = 0,
	DXGI_ADAPTER_FLAG_REMOTE      = 1,
	DXGI_ADAPTER_FLAG_SOFTWARE    = 2,
	DXGI_ADAPTER_FLAG_FORCE_DWORD = 0xFFFFFFFF
} DXGI_ADAPTER_FLAG;

typedef struct DXGI_ADAPTER_DESC1
{
	WCHAR  Description[128];
	UINT   VendorId;
	UINT   DeviceId;
	UINT   SubSysId;
	UINT   Revision;
	SIZE_T DedicatedVideoMemory;
	SIZE_T DedicatedSystemMemory;
	SIZE_T SharedSystemMemory;
	LUID   AdapterLuid;
	UINT   Flags;
} DXGI_ADAPTER_DESC1;

typedef struct DXGI_DISPLAY_COLOR_SPACE
{
	FLOAT PrimaryCoordinates[8][2];
	FLOAT WhitePoints[16][2];
} DXGI_DISPLAY_COLOR_SPACE;

MIDL_INTERFACE("770AAE78-F26F-4DBA-A829-253C83D1B387")
IDXGIFactory1 : public IDXGIFactory
{
public:
	virtual HRESULT STDMETHODCALLTYPE EnumAdapters1(
		UINT Adapter,
		IDXGIAdapter1** ppAdapter) = 0;
	virtual BOOL STDMETHODCALLTYPE IsCurrent() = 0;
};

MIDL_INTERFACE("29038F61-3839-4626-91FD-086879011A05")
IDXGIAdapter1 : public IDXGIAdapter
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDesc1(
		DXGI_ADAPTER_DESC1* pDesc) = 0;
};

MIDL_INTERFACE("77DB970F-6276-48BA-BA28-070143B4392C")
IDXGIDevice1 : public IDXGIDevice
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetMaximumFrameLatency(
		UINT MaxLatency) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetMaximumFrameLatency(
		UINT* pMaxLatency) = 0;
};

// ======================================
// dxgi1_2.h

/* Forward Declarations */

typedef interface IDXGIDisplayControl    IDXGIDisplayControl;
typedef interface IDXGIOutputDuplication IDXGIOutputDuplication;
typedef interface IDXGISurface2          IDXGISurface2;
typedef interface IDXGIResource1         IDXGIResource1;
typedef interface IDXGIDevice2           IDXGIDevice2;
typedef interface IDXGISwapChain1        IDXGISwapChain1;
typedef interface IDXGIFactory2          IDXGIFactory2;
typedef interface IDXGIAdapter2          IDXGIAdapter2;
typedef interface IDXGIOutput1           IDXGIOutput1;

MIDL_INTERFACE("EA9DBF1A-C88E-4486-854A-98AA0138F30C")
IDXGIDisplayControl : public IUnknown
{
public:
	virtual BOOL STDMETHODCALLTYPE IsStereoEnabled() = 0;
	virtual void STDMETHODCALLTYPE SetStereoEnabled(
		BOOL enabled) = 0;
};

typedef struct DXGI_OUTDUPL_MOVE_RECT
{
	POINT SourcePoint;
	RECT  DestinationRect;
} DXGI_OUTDUPL_MOVE_RECT;

typedef struct DXGI_OUTDUPL_DESC
{
	DXGI_MODE_DESC     ModeDesc;
	DXGI_MODE_ROTATION Rotation;
	BOOL               DesktopImageInSystemMemory;
} DXGI_OUTDUPL_DESC;

typedef struct DXGI_OUTDUPL_POINTER_POSITION
{
	POINT Position;
	BOOL  Visible;
} DXGI_OUTDUPL_POINTER_POSITION;

typedef enum DXGI_OUTDUPL_POINTER_SHAPE_TYPE
{
	DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME   = 0x1,
	DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR        = 0x2,
	DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR = 0x4
} DXGI_OUTDUPL_POINTER_SHAPE_TYPE;

typedef struct DXGI_OUTDUPL_POINTER_SHAPE_INFO
{
	UINT  Type;
	UINT  Width;
	UINT  Height;
	UINT  Pitch;
	POINT HotSpot;
} DXGI_OUTDUPL_POINTER_SHAPE_INFO;

typedef struct DXGI_OUTDUPL_FRAME_INFO
{
	LARGE_INTEGER                 LastPresentTime;
	LARGE_INTEGER                 LastMouseUpdateTime;
	UINT                          AccumulatedFrames;
	BOOL                          RectsCoalesced;
	BOOL                          ProtectedContentMaskedOut;
	DXGI_OUTDUPL_POINTER_POSITION PointerPosition;
	UINT                          TotalMetadataBufferSize;
	UINT                          PointerShapeBufferSize;
} DXGI_OUTDUPL_FRAME_INFO;

MIDL_INTERFACE("191CFAC3-A341-470D-B26E-A864F428319C")
IDXGIOutputDuplication : public IDXGIObject
{
public:
	virtual void STDMETHODCALLTYPE GetDesc(
		DXGI_OUTDUPL_DESC* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE AcquireNextFrame(
		UINT TimeoutInMilliseconds,
		DXGI_OUTDUPL_FRAME_INFO* pFrameInfo,
		IDXGIResource** ppDesktopResource) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFrameDirtyRects(
		UINT DirtyRectsBufferSize,
		RECT* pDirtyRectsBuffer,
		UINT* pDirtyRectsBufferSizeRequired) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFrameMoveRects(
		UINT MoveRectsBufferSize,
		DXGI_OUTDUPL_MOVE_RECT* pMoveRectBuffer,
		UINT* pMoveRectsBufferSizeRequired) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFramePointerShape(
		UINT PointerShapeBufferSize,
		void* pPointerShapeBuffer,
		UINT* pPointerShapeBufferSizeRequired,
		DXGI_OUTDUPL_POINTER_SHAPE_INFO* pPointerShapeInfo) = 0;
	virtual HRESULT STDMETHODCALLTYPE MapDesktopSurface(
		DXGI_MAPPED_RECT* pLockedRect) = 0;
	virtual HRESULT STDMETHODCALLTYPE UnMapDesktopSurface() = 0;
	virtual HRESULT STDMETHODCALLTYPE ReleaseFrame() = 0;
};

typedef enum DXGI_ALPHA_MODE
{
	DXGI_ALPHA_MODE_UNSPECIFIED   = 0,
	DXGI_ALPHA_MODE_PREMULTIPLIED = 1,
	DXGI_ALPHA_MODE_STRAIGHT      = 2,
	DXGI_ALPHA_MODE_IGNORE        = 3,
	DXGI_ALPHA_MODE_FORCE_DWORD   = 0xFFFFFFFF
} DXGI_ALPHA_MODE;

MIDL_INTERFACE("ABA496DD-B617-4CB8-A866-BC44D7EB1FA2")
IDXGISurface2 : public IDXGISurface1
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetResource(
		REFIID riid,
		void** ppParentResource,
		UINT* pSubresourceIndex) = 0;
};

MIDL_INTERFACE("30961379-4609-4A41-998E-54FE567EE0C1")
IDXGIResource1 : public IDXGIResource
{
public:
	virtual HRESULT STDMETHODCALLTYPE CreateSubresourceSurface(
		UINT index,
		IDXGISurface2** ppSurface) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateSharedHandle(
		const SECURITY_ATTRIBUTES* pAttributes,
		DWORD dwAccess,
		LPCWSTR lpName,
		HANDLE* pHandle) = 0;
};

typedef enum _DXGI_OFFER_RESOURCE_PRIORITY
{
	DXGI_OFFER_RESOURCE_PRIORITY_LOW    = 1,
	DXGI_OFFER_RESOURCE_PRIORITY_NORMAL = DXGI_OFFER_RESOURCE_PRIORITY_LOW + 1,
	DXGI_OFFER_RESOURCE_PRIORITY_HIGH   = DXGI_OFFER_RESOURCE_PRIORITY_NORMAL + 1
} DXGI_OFFER_RESOURCE_PRIORITY;

MIDL_INTERFACE("05008617-FBFD-4051-A790-144884B4F6A9")
IDXGIDevice2 : public IDXGIDevice1
{
public:
	virtual HRESULT STDMETHODCALLTYPE OfferResources(
		UINT NumResources,
		IDXGIResource* const* ppResources,
		DXGI_OFFER_RESOURCE_PRIORITY Priority) = 0;
	virtual HRESULT STDMETHODCALLTYPE ReclaimResources(
		UINT NumResources,
		IDXGIResource* const* ppResources,
		BOOL* pDiscarded) = 0;
	virtual HRESULT STDMETHODCALLTYPE EnqueueSetEvent(
		HANDLE hEvent) = 0;
};

#define	DXGI_ENUM_MODES_STEREO          (4UL)
#define	DXGI_ENUM_MODES_DISABLED_STEREO (8UL)
#define	DXGI_SHARED_RESOURCE_READ       (0x80000000L)
#define	DXGI_SHARED_RESOURCE_WRITE      (1)

typedef struct DXGI_MODE_DESC1
{
	UINT                     Width;
	UINT                     Height;
	DXGI_RATIONAL            RefreshRate;
	DXGI_FORMAT              Format;
	DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;
	DXGI_MODE_SCALING        Scaling;
	BOOL                     Stereo;
} DXGI_MODE_DESC1;

typedef enum DXGI_SCALING
{
	DXGI_SCALING_STRETCH              = 0,
	DXGI_SCALING_NONE                 = 1,
	DXGI_SCALING_ASPECT_RATIO_STRETCH = 2
} DXGI_SCALING;

typedef struct DXGI_SWAP_CHAIN_DESC1
{
	UINT             Width;
	UINT             Height;
	DXGI_FORMAT      Format;
	BOOL             Stereo;
	DXGI_SAMPLE_DESC SampleDesc;
	DXGI_USAGE       BufferUsage;
	UINT             BufferCount;
	DXGI_SCALING     Scaling;
	DXGI_SWAP_EFFECT SwapEffect;
	DXGI_ALPHA_MODE  AlphaMode;
	UINT             Flags;
} DXGI_SWAP_CHAIN_DESC1;

typedef struct DXGI_SWAP_CHAIN_FULLSCREEN_DESC
{
	DXGI_RATIONAL            RefreshRate;
	DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;
	DXGI_MODE_SCALING        Scaling;
	BOOL                     Windowed;
} DXGI_SWAP_CHAIN_FULLSCREEN_DESC;

typedef struct DXGI_PRESENT_PARAMETERS
{
	UINT   DirtyRectsCount;
	RECT*  pDirtyRects;
	RECT*  pScrollRect;
	POINT* pScrollOffset;
} DXGI_PRESENT_PARAMETERS;

MIDL_INTERFACE("790A45F7-0D42-4876-983A-0A55CFE6F4AA")
IDXGISwapChain1 : public IDXGISwapChain
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDesc1(
		DXGI_SWAP_CHAIN_DESC1* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFullscreenDesc(
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetHwnd(
		HWND* pHwnd) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCoreWindow(
		REFIID refiid,
		void** ppUnk) = 0;
	virtual HRESULT STDMETHODCALLTYPE Present1(
		UINT SyncInterval,
		UINT PresentFlags,
		const DXGI_PRESENT_PARAMETERS* pPresentParameters) = 0;
	virtual BOOL STDMETHODCALLTYPE IsTemporaryMonoSupported() = 0;
	virtual HRESULT STDMETHODCALLTYPE GetRestrictToOutput(
		IDXGIOutput** ppRestrictToOutput) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBackgroundColor(
		const DXGI_RGBA* pColor) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetBackgroundColor(
		DXGI_RGBA* pColor) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetRotation(
		DXGI_MODE_ROTATION Rotation) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetRotation(
		DXGI_MODE_ROTATION* pRotation) = 0;
};

MIDL_INTERFACE("50C83A1C-E072-4C48-87B0-3630FA36A6D0")
IDXGIFactory2 : public IDXGIFactory1
{
public:
	virtual BOOL STDMETHODCALLTYPE IsWindowedStereoEnabled() = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForHwnd(
		IUnknown* pDevice,
		HWND hWnd,
		const DXGI_SWAP_CHAIN_DESC1* pDesc,
		const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc,
		IDXGIOutput* pRestrictToOutput,
		IDXGISwapChain1** ppSwapChain) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForCoreWindow(
		IUnknown* pDevice,
		IUnknown* pWindow,
		const DXGI_SWAP_CHAIN_DESC1* pDesc,
		IDXGIOutput* pRestrictToOutput,
		IDXGISwapChain1** ppSwapChain) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetSharedResourceAdapterLuid(
		HANDLE hResource,
		LUID* pLuid) = 0;
	virtual HRESULT STDMETHODCALLTYPE RegisterStereoStatusWindow(
		HWND WindowHandle,
		UINT wMsg,
		DWORD* pdwCookie) = 0;
	virtual HRESULT STDMETHODCALLTYPE RegisterStereoStatusEvent(
		HANDLE hEvent,
		DWORD* pdwCookie) = 0;
	virtual void STDMETHODCALLTYPE UnregisterStereoStatus(
		DWORD dwCookie) = 0;
	virtual HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusWindow(
		HWND WindowHandle,
		UINT wMsg,
		DWORD* pdwCookie) = 0;
	virtual HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusEvent(
		HANDLE hEvent,
		DWORD* pdwCookie) = 0;
	virtual void STDMETHODCALLTYPE UnregisterOcclusionStatus(
		DWORD dwCookie) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForComposition(
		IUnknown* pDevice,
		const DXGI_SWAP_CHAIN_DESC1* pDesc,
		IDXGIOutput* pRestrictToOutput,
		IDXGISwapChain1** ppSwapChain) = 0;
};

typedef enum DXGI_GRAPHICS_PREEMPTION_GRANULARITY
{
	DXGI_GRAPHICS_PREEMPTION_DMA_BUFFER_BOUNDARY  = 0,
	DXGI_GRAPHICS_PREEMPTION_PRIMITIVE_BOUNDARY   = 1,
	DXGI_GRAPHICS_PREEMPTION_TRIANGLE_BOUNDARY    = 2,
	DXGI_GRAPHICS_PREEMPTION_PIXEL_BOUNDARY       = 3,
	DXGI_GRAPHICS_PREEMPTION_INSTRUCTION_BOUNDARY = 4
} DXGI_GRAPHICS_PREEMPTION_GRANULARITY;

typedef enum DXGI_COMPUTE_PREEMPTION_GRANULARITY
{
	DXGI_COMPUTE_PREEMPTION_DMA_BUFFER_BOUNDARY   = 0,
	DXGI_COMPUTE_PREEMPTION_DISPATCH_BOUNDARY     = 1,
	DXGI_COMPUTE_PREEMPTION_THREAD_GROUP_BOUNDARY = 2,
	DXGI_COMPUTE_PREEMPTION_THREAD_BOUNDARY       = 3,
	DXGI_COMPUTE_PREEMPTION_INSTRUCTION_BOUNDARY  = 4
} DXGI_COMPUTE_PREEMPTION_GRANULARITY;

typedef struct DXGI_ADAPTER_DESC2
{
	WCHAR                                Description[128];
	UINT                                 VendorId;
	UINT                                 DeviceId;
	UINT                                 SubSysId;
	UINT                                 Revision;
	SIZE_T                               DedicatedVideoMemory;
	SIZE_T                               DedicatedSystemMemory;
	SIZE_T                               SharedSystemMemory;
	LUID                                 AdapterLuid;
	UINT                                 Flags;
	DXGI_GRAPHICS_PREEMPTION_GRANULARITY GraphicsPreemptionGranularity;
	DXGI_COMPUTE_PREEMPTION_GRANULARITY  ComputePreemptionGranularity;
} DXGI_ADAPTER_DESC2;

MIDL_INTERFACE("0AA1AE0A-FA0E-4B84-8644-E05FF8E5ACB5")
IDXGIAdapter2 : public IDXGIAdapter1
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDesc2(
		DXGI_ADAPTER_DESC2* pDesc) = 0;
};

MIDL_INTERFACE("00CDDEA8-939B-4B83-A340-A685226666CC")
IDXGIOutput1 : public IDXGIOutput
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDisplayModeList1(
		DXGI_FORMAT EnumFormat,
		UINT Flags,
		UINT* pNumModes,
		DXGI_MODE_DESC1* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE FindClosestMatchingMode1(
		const DXGI_MODE_DESC1* pModeToMatch,
		DXGI_MODE_DESC1* pClosestMatch,
		IUnknown* pConcernedDevice) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDisplaySurfaceData1(
		IDXGIResource* pDestination) = 0;
	virtual HRESULT STDMETHODCALLTYPE DuplicateOutput(
		IUnknown* pDevice,
		IDXGIOutputDuplication** ppOutputDuplication) = 0;
};

// ======================================
// dxgi1_3.h

/* Forward Declarations */

typedef interface IDXGIDevice3         IDXGIDevice3;
typedef interface IDXGISwapChain2      IDXGISwapChain2;
typedef interface IDXGIOutput2         IDXGIOutput2;
typedef interface IDXGIFactory3        IDXGIFactory3;
typedef interface IDXGIDecodeSwapChain IDXGIDecodeSwapChain;
typedef interface IDXGIFactoryMedia    IDXGIFactoryMedia;
typedef interface IDXGISwapChainMedia  IDXGISwapChainMedia;
typedef interface IDXGIOutput3         IDXGIOutput3;

#define DXGI_CREATE_FACTORY_DEBUG 0x1

MIDL_INTERFACE("6007896C-3244-4AFD-BF18-A6D3BEDA5023")
IDXGIDevice3 : public IDXGIDevice2
{
public:
	virtual void STDMETHODCALLTYPE Trim() = 0;
};

typedef struct DXGI_MATRIX_3X2_F
{
	FLOAT _11;
	FLOAT _12;
	FLOAT _21;
	FLOAT _22;
	FLOAT _31;
	FLOAT _32;
} DXGI_MATRIX_3X2_F;

MIDL_INTERFACE("A8BE2AC4-199F-4946-B331-79599FB98DE7")
IDXGISwapChain2 : public IDXGISwapChain1
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetSourceSize(
		UINT Width,
		UINT Height) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetSourceSize(
		UINT* pWidth,
		UINT* pHeight) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMaximumFrameLatency(
		UINT MaxLatency) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetMaximumFrameLatency(
		UINT* pMaxLatency) = 0;
	virtual HANDLE STDMETHODCALLTYPE GetFrameLatencyWaitableObject() = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMatrixTransform(
		const DXGI_MATRIX_3X2_F* pMatrix) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetMatrixTransform(
		DXGI_MATRIX_3X2_F* pMatrix) = 0;
};

MIDL_INTERFACE("595E39D1-2724-4663-99B1-DA969DE28364")
IDXGIOutput2 : public IDXGIOutput1
{
public:
	virtual BOOL STDMETHODCALLTYPE SupportsOverlays() = 0;
};

MIDL_INTERFACE("25483823-CD46-4C7D-86CA-47AA95B837BD")
IDXGIFactory3 : public IDXGIFactory2
{
public:
	virtual UINT STDMETHODCALLTYPE GetCreationFlags() = 0;
};

typedef struct DXGI_DECODE_SWAP_CHAIN_DESC
{
	UINT Flags;
} DXGI_DECODE_SWAP_CHAIN_DESC;

typedef
	enum DXGI_MULTIPLANE_OVERLAY_YCbCr_FLAGS
{
	DXGI_MULTIPLANE_OVERLAY_YCbCr_FLAG_NOMINAL_RANGE = 0x1,
	DXGI_MULTIPLANE_OVERLAY_YCbCr_FLAG_BT709         = 0x2,
	DXGI_MULTIPLANE_OVERLAY_YCbCr_FLAG_xvYCC         = 0x4
} DXGI_MULTIPLANE_OVERLAY_YCbCr_FLAGS;

MIDL_INTERFACE("2633066B-4514-4C7A-8FD8-12EA98059D18")
IDXGIDecodeSwapChain : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE PresentBuffer(
		UINT BufferToPresent,
		UINT SyncInterval,
		UINT Flags) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetSourceRect(
		const RECT* pRect) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetTargetRect(
		const RECT* pRect) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDestSize(
		UINT Width,
		UINT Height) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetSourceRect(
		RECT* pRect) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetTargetRect(
		RECT* pRect) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDestSize(
		UINT* pWidth,
		UINT* pHeight) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetColorSpace(
		DXGI_MULTIPLANE_OVERLAY_YCbCr_FLAGS ColorSpace) = 0;
	virtual DXGI_MULTIPLANE_OVERLAY_YCbCr_FLAGS STDMETHODCALLTYPE GetColorSpace() = 0;
};

MIDL_INTERFACE("41E7D1F2-A591-4F7B-A2E5-FA9C843E1C12")
IDXGIFactoryMedia : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForCompositionSurfaceHandle(
		IUnknown* pDevice,
		HANDLE hSurface,
		const DXGI_SWAP_CHAIN_DESC1* pDesc,
		IDXGIOutput* pRestrictToOutput,
		IDXGISwapChain1** ppSwapChain) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateDecodeSwapChainForCompositionSurfaceHandle(
		IUnknown* pDevice,
		HANDLE hSurface,
		DXGI_DECODE_SWAP_CHAIN_DESC* pDesc,
		IDXGIResource* pYuvDecodeBuffers,
		IDXGIOutput* pRestrictToOutput,
		IDXGIDecodeSwapChain** ppSwapChain) = 0;
};

typedef enum DXGI_FRAME_PRESENTATION_MODE
{
	DXGI_FRAME_PRESENTATION_MODE_COMPOSED            = 0,
	DXGI_FRAME_PRESENTATION_MODE_OVERLAY             = 1,
	DXGI_FRAME_PRESENTATION_MODE_NONE                = 2,
	DXGI_FRAME_PRESENTATION_MODE_COMPOSITION_FAILURE = 3
} DXGI_FRAME_PRESENTATION_MODE;

typedef struct DXGI_FRAME_STATISTICS_MEDIA
{
	UINT                         PresentCount;
	UINT                         PresentRefreshCount;
	UINT                         SyncRefreshCount;
	LARGE_INTEGER                SyncQPCTime;
	LARGE_INTEGER                SyncGPUTime;
	DXGI_FRAME_PRESENTATION_MODE CompositionMode;
	UINT                         ApprovedPresentDuration;
} DXGI_FRAME_STATISTICS_MEDIA;

MIDL_INTERFACE("DD95B90B-F05F-4F6A-BD65-25BFB264BD84")
IDXGISwapChainMedia : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetFrameStatisticsMedia(
		DXGI_FRAME_STATISTICS_MEDIA* pStats) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetPresentDuration(
		UINT Duration) = 0;
	virtual HRESULT STDMETHODCALLTYPE CheckPresentDurationSupport(
		UINT DesiredPresentDuration,
		UINT* pClosestSmallerPresentDuration,
		UINT* pClosestLargerPresentDuration) = 0;
};

typedef enum DXGI_OVERLAY_SUPPORT_FLAG
{
	DXGI_OVERLAY_SUPPORT_FLAG_DIRECT  = 0x1,
	DXGI_OVERLAY_SUPPORT_FLAG_SCALING = 0x2
} DXGI_OVERLAY_SUPPORT_FLAG;

MIDL_INTERFACE("8A6BB301-7E7E-41F4-A8E0-5B32F7F99B18")
IDXGIOutput3 : public IDXGIOutput2
{
public:
	virtual HRESULT STDMETHODCALLTYPE CheckOverlaySupport(
		DXGI_FORMAT EnumFormat,
		IUnknown* pConcernedDevice,
		UINT* pFlags) = 0;
};

// ======================================
// dxgi1_4.h

/* Forward Declarations */

typedef interface IDXGISwapChain3 IDXGISwapChain3;
typedef interface IDXGIOutput4    IDXGIOutput4;
typedef interface IDXGIFactory4   IDXGIFactory4;
typedef interface IDXGIAdapter3   IDXGIAdapter3;

typedef enum DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG
{
	DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT         = 0x1,
	DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_OVERLAY_PRESENT = 0x2
} DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG;

MIDL_INTERFACE("94D99BDB-F1F8-4AB0-B236-7DA0170EDAB1")
IDXGISwapChain3 : public IDXGISwapChain2
{
public:
	virtual UINT STDMETHODCALLTYPE GetCurrentBackBufferIndex() = 0;
	virtual HRESULT STDMETHODCALLTYPE CheckColorSpaceSupport(
		DXGI_COLOR_SPACE_TYPE ColorSpace,
		UINT* pColorSpaceSupport) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetColorSpace1(
		DXGI_COLOR_SPACE_TYPE ColorSpace) = 0;
	virtual HRESULT STDMETHODCALLTYPE ResizeBuffers1(
		UINT BufferCount,
		UINT Width,
		UINT Height,
		DXGI_FORMAT Format,
		UINT SwapChainFlags,
		const UINT* pCreationNodeMask,
		IUnknown* const* ppPresentQueue) = 0;
};

typedef enum DXGI_OVERLAY_COLOR_SPACE_SUPPORT_FLAG
{
	DXGI_OVERLAY_COLOR_SPACE_SUPPORT_FLAG_PRESENT = 0x1
} DXGI_OVERLAY_COLOR_SPACE_SUPPORT_FLAG;

MIDL_INTERFACE("DC7DCA35-2196-414D-9F53-617884032A60")
IDXGIOutput4 : public IDXGIOutput3
{
public:
	virtual HRESULT STDMETHODCALLTYPE CheckOverlayColorSpaceSupport(
		DXGI_FORMAT Format,
		DXGI_COLOR_SPACE_TYPE ColorSpace,
		IUnknown* pConcernedDevice,
		UINT* pFlags) = 0;
};

MIDL_INTERFACE("1BC6EA02-EF36-464F-BF0C-21CA39E5168A")
IDXGIFactory4 : public IDXGIFactory3
{
public:
	virtual HRESULT STDMETHODCALLTYPE EnumAdapterByLuid(
		LUID AdapterLuid,
		REFIID riid,
		void** ppvAdapter) = 0;
	virtual HRESULT STDMETHODCALLTYPE EnumWarpAdapter(
		REFIID riid,
		void** ppvAdapter) = 0;
};

typedef enum DXGI_MEMORY_SEGMENT_GROUP
{
	DXGI_MEMORY_SEGMENT_GROUP_LOCAL     = 0,
	DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL = 1
} DXGI_MEMORY_SEGMENT_GROUP;

typedef struct DXGI_QUERY_VIDEO_MEMORY_INFO
{
	UINT64 Budget;
	UINT64 CurrentUsage;
	UINT64 AvailableForReservation;
	UINT64 CurrentReservation;
} DXGI_QUERY_VIDEO_MEMORY_INFO;

MIDL_INTERFACE("645967A4-1392-4310-A798-8053CE3E93FD")
IDXGIAdapter3 : public IDXGIAdapter2
{
public:
	virtual HRESULT STDMETHODCALLTYPE RegisterHardwareContentProtectionTeardownStatusEvent(
		HANDLE hEvent,
		DWORD* pdwCookie) = 0;
	virtual void STDMETHODCALLTYPE UnregisterHardwareContentProtectionTeardownStatus(
		DWORD dwCookie) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryVideoMemoryInfo(
		UINT NodeIndex,
		DXGI_MEMORY_SEGMENT_GROUP MemorySegmentGroup,
		DXGI_QUERY_VIDEO_MEMORY_INFO* pVideoMemoryInfo) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetVideoMemoryReservation(
		UINT NodeIndex,
		DXGI_MEMORY_SEGMENT_GROUP MemorySegmentGroup,
		UINT64 Reservation) = 0;
	virtual HRESULT STDMETHODCALLTYPE RegisterVideoMemoryBudgetChangeNotificationEvent(
		HANDLE hEvent,
		DWORD* pdwCookie) = 0;
	virtual void STDMETHODCALLTYPE UnregisterVideoMemoryBudgetChangeNotification(
		DWORD dwCookie) = 0;
};

// ======================================
// dxgi1_5.h

/* Forward Declarations */

typedef interface IDXGIOutput5    IDXGIOutput5;
typedef interface IDXGISwapChain4 IDXGISwapChain4;
typedef interface IDXGIDevice4    IDXGIDevice4;
typedef interface IDXGIFactory5   IDXGIFactory5;

typedef enum DXGI_OUTDUPL_FLAG
{
	DXGI_OUTDUPL_COMPOSITED_UI_CAPTURE_ONLY = 1
} DXGI_OUTDUPL_FLAG;

MIDL_INTERFACE("80A07424-AB52-42EB-833C-0C42FD282D98")
IDXGIOutput5 : public IDXGIOutput4
{
public:
	virtual HRESULT STDMETHODCALLTYPE DuplicateOutput1(
		IUnknown* pDevice,
		UINT Flags,
		UINT SupportedFormatsCount,
		const DXGI_FORMAT* pSupportedFormats,
		IDXGIOutputDuplication** ppOutputDuplication) = 0;
};

typedef enum DXGI_HDR_METADATA_TYPE
{
	DXGI_HDR_METADATA_TYPE_NONE      = 0,
	DXGI_HDR_METADATA_TYPE_HDR10     = 1,
	DXGI_HDR_METADATA_TYPE_HDR10PLUS = 2
} DXGI_HDR_METADATA_TYPE;

typedef struct DXGI_HDR_METADATA_HDR10
{
	UINT16 RedPrimary[2];
	UINT16 GreenPrimary[2];
	UINT16 BluePrimary[2];
	UINT16 WhitePoint[2];
	UINT   MaxMasteringLuminance;
	UINT   MinMasteringLuminance;
	UINT16 MaxContentLightLevel;
	UINT16 MaxFrameAverageLightLevel;
} DXGI_HDR_METADATA_HDR10;

typedef struct DXGI_HDR_METADATA_HDR10PLUS
{
	BYTE Data[72];
} DXGI_HDR_METADATA_HDR10PLUS;

MIDL_INTERFACE("3D585D5A-BD4A-489E-B1F4-3DBCB6452FFB")
IDXGISwapChain4 : public IDXGISwapChain3
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetHDRMetaData(
		DXGI_HDR_METADATA_TYPE Type,
		UINT Size,
		void* pMetaData) = 0;
};

typedef enum _DXGI_OFFER_RESOURCE_FLAGS
{
	DXGI_OFFER_RESOURCE_FLAG_ALLOW_DECOMMIT = 0x1
} DXGI_OFFER_RESOURCE_FLAGS;

typedef enum _DXGI_RECLAIM_RESOURCE_RESULTS
{
	DXGI_RECLAIM_RESOURCE_RESULT_OK            = 0,
	DXGI_RECLAIM_RESOURCE_RESULT_DISCARDED     = 1,
	DXGI_RECLAIM_RESOURCE_RESULT_NOT_COMMITTED = 2
} DXGI_RECLAIM_RESOURCE_RESULTS;

MIDL_INTERFACE("95B4F95F-D8DA-4CA4-9EE6-3B76D5968A10")
IDXGIDevice4 : public IDXGIDevice3
{
public:
	virtual HRESULT STDMETHODCALLTYPE OfferResources1(
		UINT NumResources,
		IDXGIResource* const* ppResources,
		DXGI_OFFER_RESOURCE_PRIORITY Priority,
		UINT Flags) = 0;
	virtual HRESULT STDMETHODCALLTYPE ReclaimResources1(
		UINT NumResources,
		IDXGIResource* const* ppResources,
		DXGI_RECLAIM_RESOURCE_RESULTS* pResults) = 0;
};

typedef enum DXGI_FEATURE
{
	DXGI_FEATURE_PRESENT_ALLOW_TEARING = 0
} DXGI_FEATURE;

MIDL_INTERFACE("7632E1F5-EE65-4DCA-87FD-84CD75F8838D")
IDXGIFactory5 : public IDXGIFactory4
{
public:
	virtual HRESULT STDMETHODCALLTYPE CheckFeatureSupport(
		DXGI_FEATURE Feature,
		void* pFeatureSupportData,
		UINT FeatureSupportDataSize) = 0;
};

// ======================================
// dxgi1_6.h

/* Forward Declarations */

typedef interface IDXGIAdapter4 IDXGIAdapter4;
typedef interface IDXGIOutput6  IDXGIOutput6;
typedef interface IDXGIFactory6 IDXGIFactory6;
typedef interface IDXGIFactory7 IDXGIFactory7;

typedef enum DXGI_ADAPTER_FLAG3
{
	DXGI_ADAPTER_FLAG3_NONE                         = 0,
	DXGI_ADAPTER_FLAG3_REMOTE                       = 1,
	DXGI_ADAPTER_FLAG3_SOFTWARE                     = 2,
	DXGI_ADAPTER_FLAG3_ACG_COMPATIBLE               = 4,
	DXGI_ADAPTER_FLAG3_SUPPORT_MONITORED_FENCES     = 8,
	DXGI_ADAPTER_FLAG3_SUPPORT_NON_MONITORED_FENCES = 0x10,
	DXGI_ADAPTER_FLAG3_KEYED_MUTEX_CONFORMANCE      = 0x20,
	DXGI_ADAPTER_FLAG3_FORCE_DWORD                  = 0xFFFFFFFF
} DXGI_ADAPTER_FLAG3;

typedef struct DXGI_ADAPTER_DESC3
{
	WCHAR                                Description[128];
	UINT                                 VendorId;
	UINT                                 DeviceId;
	UINT                                 SubSysId;
	UINT                                 Revision;
	SIZE_T                               DedicatedVideoMemory;
	SIZE_T                               DedicatedSystemMemory;
	SIZE_T                               SharedSystemMemory;
	LUID                                 AdapterLuid;
	DXGI_ADAPTER_FLAG3                   Flags;
	DXGI_GRAPHICS_PREEMPTION_GRANULARITY GraphicsPreemptionGranularity;
	DXGI_COMPUTE_PREEMPTION_GRANULARITY  ComputePreemptionGranularity;
} DXGI_ADAPTER_DESC3;

MIDL_INTERFACE("3C8D99D1-4FBF-4181-A82C-AF66BF7BD24E")
IDXGIAdapter4 : public IDXGIAdapter3
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDesc3(
		DXGI_ADAPTER_DESC3* pDesc) = 0;
};

typedef struct DXGI_OUTPUT_DESC1
{
	WCHAR                 DeviceName[32];
	RECT                  DesktopCoordinates;
	BOOL                  AttachedToDesktop;
	DXGI_MODE_ROTATION    Rotation;
	HMONITOR              Monitor;
	UINT                  BitsPerColor;
	DXGI_COLOR_SPACE_TYPE ColorSpace;
	FLOAT                 RedPrimary[2];
	FLOAT                 GreenPrimary[2];
	FLOAT                 BluePrimary[2];
	FLOAT                 WhitePoint[2];
	FLOAT                 MinLuminance;
	FLOAT                 MaxLuminance;
	FLOAT                 MaxFullFrameLuminance;
} DXGI_OUTPUT_DESC1;

typedef enum DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAGS
{
	DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN       = 1,
	DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED         = 2,
	DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_CURSOR_STRETCHED = 4
} DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAGS;

MIDL_INTERFACE("068346E8-AAEC-4B84-ADD7-137F513F77A1")
IDXGIOutput6 : public IDXGIOutput5
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetDesc1(
		DXGI_OUTPUT_DESC1* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE CheckHardwareCompositionSupport(
		UINT* pFlags) = 0;
};

typedef enum DXGI_GPU_PREFERENCE
{
	DXGI_GPU_PREFERENCE_UNSPECIFIED =      0,
	DXGI_GPU_PREFERENCE_MINIMUM_POWER =    DXGI_GPU_PREFERENCE_UNSPECIFIED + 1,
	DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE = DXGI_GPU_PREFERENCE_MINIMUM_POWER + 1
} DXGI_GPU_PREFERENCE;

MIDL_INTERFACE("C1B6694F-FF09-44A9-B03C-77900A0A1D17")
IDXGIFactory6 : public IDXGIFactory5
{
public:
	virtual HRESULT STDMETHODCALLTYPE EnumAdapterByGpuPreference(
		UINT Adapter,
		DXGI_GPU_PREFERENCE GpuPreference,
		REFIID riid,
		void** ppvAdapter) = 0;
};

MIDL_INTERFACE("A4966EED-76DB-44DA-84C1-EE9A7AFB20A8")
IDXGIFactory7 : public IDXGIFactory6
{
public:
	virtual HRESULT STDMETHODCALLTYPE RegisterAdaptersChangedEvent(
		HANDLE hEvent,
		DWORD* pdwCookie) = 0;
	virtual HRESULT STDMETHODCALLTYPE UnregisterAdaptersChangedEvent(
		DWORD dwCookie) = 0;
};

// ======================================
// DisplayConfig stuff

#define QDC_ALL_PATHS                  0x00000001
#define QDC_ONLY_ACTIVE_PATHS          0x00000002
#define QDC_DATABASE_CURRENT           0x00000004
#define QDC_VIRTUAL_MODE_AWARE         0x00000010
#define QDC_INCLUDE_HMD                0x00000020
#define QDC_VIRTUAL_REFRESH_RATE_AWARE 0x00000040

typedef enum
{
	DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME                = 1,
	DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME                = 2,
	DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE      = 3,
	DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME               = 4,
	DISPLAYCONFIG_DEVICE_INFO_SET_TARGET_PERSISTENCE         = 5,
	DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_BASE_TYPE           = 6,
	DISPLAYCONFIG_DEVICE_INFO_GET_SUPPORT_VIRTUAL_RESOLUTION = 7,
	DISPLAYCONFIG_DEVICE_INFO_SET_SUPPORT_VIRTUAL_RESOLUTION = 8,
	DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO        = 9,
	DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE       = 10,
	DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL            = 11,
	DISPLAYCONFIG_DEVICE_INFO_GET_MONITOR_SPECIALIZATION     = 12,
	DISPLAYCONFIG_DEVICE_INFO_SET_MONITOR_SPECIALIZATION     = 13,
	DISPLAYCONFIG_DEVICE_INFO_SET_RESERVED1                  = 14,
	DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO_2      = 15,
	DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE                  = 16,
	DISPLAYCONFIG_DEVICE_INFO_SET_WCG_STATE                  = 17,
	DISPLAYCONFIG_DEVICE_INFO_FORCE_UINT32                   = 0xFFFFFFFF
} DISPLAYCONFIG_DEVICE_INFO_TYPE;

typedef struct DISPLAYCONFIG_DEVICE_INFO_HEADER
{
	DISPLAYCONFIG_DEVICE_INFO_TYPE type;
	UINT32                         size;
	LUID                           adapterId;
	UINT32                         id;
} DISPLAYCONFIG_DEVICE_INFO_HEADER;

typedef struct _DISPLAYCONFIG_SDR_WHITE_LEVEL
{
	DISPLAYCONFIG_DEVICE_INFO_HEADER header;

	// SDRWhiteLevel represents a multiplier for standard SDR white
	// peak value i.e. 80 nits represented as fixed point.
	// To get value in nits use the following conversion
	// SDRWhiteLevel in nits = (SDRWhiteLevel / 1000 ) * 80
	ULONG                            SDRWhiteLevel;
} DISPLAYCONFIG_SDR_WHITE_LEVEL;

typedef struct DISPLAYCONFIG_PATH_SOURCE_INFO
{
	LUID           adapterId;
	UINT32         id;
	union
	{
		UINT32     modeInfoIdx;
		struct
		{
			UINT32 cloneGroupId      : 16;
			UINT32 sourceModeInfoIdx : 16;
#if 0 // FIXME
		} DUMMYSTRUCTNAME;
#else
		} s1;
#endif
#if 0 // FIXME
	} DUMMYUNIONNAME;
#else
	} u1;
#endif
	UINT32         statusFlags;
} DISPLAYCONFIG_PATH_SOURCE_INFO;

typedef enum
{
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER                  = -1,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HD15                   = 0,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SVIDEO                 = 1,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPOSITE_VIDEO        = 2,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPONENT_VIDEO        = 3,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI                    = 4,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI                   = 5,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_LVDS                   = 6,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_D_JPN                  = 8,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDI                    = 9,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL   = 10,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED   = 11,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EXTERNAL           = 12,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED           = 13,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDTVDONGLE             = 14,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_MIRACAST               = 15,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INDIRECT_WIRED         = 16,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INDIRECT_VIRTUAL       = 17,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_USB_TUNNEL = 18,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL               = 0x80000000,
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_FORCE_UINT32           = 0xFFFFFFFF
} DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY;

typedef enum
{
	DISPLAYCONFIG_ROTATION_IDENTITY     = 1,
	DISPLAYCONFIG_ROTATION_ROTATE90     = 2,
	DISPLAYCONFIG_ROTATION_ROTATE180    = 3,
	DISPLAYCONFIG_ROTATION_ROTATE270    = 4,
	DISPLAYCONFIG_ROTATION_FORCE_UINT32 = 0xFFFFFFFF
} DISPLAYCONFIG_ROTATION;

typedef enum
{
	DISPLAYCONFIG_SCALING_IDENTITY               = 1,
	DISPLAYCONFIG_SCALING_CENTERED               = 2,
	DISPLAYCONFIG_SCALING_STRETCHED              = 3,
	DISPLAYCONFIG_SCALING_ASPECTRATIOCENTEREDMAX = 4,
	DISPLAYCONFIG_SCALING_CUSTOM                 = 5,
	DISPLAYCONFIG_SCALING_PREFERRED              = 128,
	DISPLAYCONFIG_SCALING_FORCE_UINT32           = 0xFFFFFFFF
} DISPLAYCONFIG_SCALING;

typedef struct DISPLAYCONFIG_RATIONAL
{
	UINT32 Numerator;
	UINT32 Denominator;
} DISPLAYCONFIG_RATIONAL;

typedef enum
{
	DISPLAYCONFIG_SCANLINE_ORDERING_UNSPECIFIED                = 0,
	DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE                = 1,
	DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED                 = 2,
	DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED_UPPERFIELDFIRST = DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED,
	DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED_LOWERFIELDFIRST = 3,
	DISPLAYCONFIG_SCANLINE_ORDERING_FORCE_UINT32               = 0xFFFFFFFF
} DISPLAYCONFIG_SCANLINE_ORDERING;

typedef struct DISPLAYCONFIG_PATH_TARGET_INFO
{
	LUID                                  adapterId;
	UINT32                                id;
	union
	{
		UINT32                            modeInfoIdx;
		struct
		{
			UINT32                        desktopModeInfoIdx : 16;
			UINT32                        targetModeInfoIdx  : 16;
#if 0 // FIXME
		} DUMMYSTRUCTNAME;
#else
		} s1;
#endif
#if 0 // FIXME
	} DUMMYUNIONNAME;
#else
	} u1;
#endif
	DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY outputTechnology;
	DISPLAYCONFIG_ROTATION                rotation;
	DISPLAYCONFIG_SCALING                 scaling;
	DISPLAYCONFIG_RATIONAL                refreshRate;
	DISPLAYCONFIG_SCANLINE_ORDERING       scanLineOrdering;
	BOOL                                  targetAvailable;
	UINT32                                statusFlags;
} DISPLAYCONFIG_PATH_TARGET_INFO;

typedef struct DISPLAYCONFIG_PATH_INFO
{
	DISPLAYCONFIG_PATH_SOURCE_INFO sourceInfo;
	DISPLAYCONFIG_PATH_TARGET_INFO targetInfo;
	UINT32                         flags;
} DISPLAYCONFIG_PATH_INFO;

typedef enum
{
	DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE        = 1,
	DISPLAYCONFIG_MODE_INFO_TYPE_TARGET        = 2,
	DISPLAYCONFIG_MODE_INFO_TYPE_DESKTOP_IMAGE = 3,
	DISPLAYCONFIG_MODE_INFO_TYPE_FORCE_UINT32  = 0xFFFFFFFF
} DISPLAYCONFIG_MODE_INFO_TYPE;

typedef struct DISPLAYCONFIG_2DREGION
{
	UINT32 cx;
	UINT32 cy;
} DISPLAYCONFIG_2DREGION;

typedef struct DISPLAYCONFIG_VIDEO_SIGNAL_INFO
{
	UINT64                          pixelRate;
	DISPLAYCONFIG_RATIONAL          hSyncFreq;
	DISPLAYCONFIG_RATIONAL          vSyncFreq;
	DISPLAYCONFIG_2DREGION          activeSize;
	DISPLAYCONFIG_2DREGION          totalSize;

	union
	{
		struct
		{
			UINT32                  videoStandard    : 16;

			// Vertical refresh frequency divider
			UINT32                  vSyncFreqDivider : 6;

			UINT32                  reserved         : 10;
		} AdditionalSignalInfo;

		UINT32                      videoStandard;
#if 0 // FIXME
	} DUMMYUNIONNAME;
#else
	} u1;
#endif

	// Scan line ordering (e.g. progressive, interlaced).
	DISPLAYCONFIG_SCANLINE_ORDERING scanLineOrdering;
} DISPLAYCONFIG_VIDEO_SIGNAL_INFO;

typedef struct DISPLAYCONFIG_TARGET_MODE
{
	DISPLAYCONFIG_VIDEO_SIGNAL_INFO targetVideoSignalInfo;
} DISPLAYCONFIG_TARGET_MODE;

typedef enum
{
	DISPLAYCONFIG_PIXELFORMAT_8BPP         = 1,
	DISPLAYCONFIG_PIXELFORMAT_16BPP        = 2,
	DISPLAYCONFIG_PIXELFORMAT_24BPP        = 3,
	DISPLAYCONFIG_PIXELFORMAT_32BPP        = 4,
	DISPLAYCONFIG_PIXELFORMAT_NONGDI       = 5,
	DISPLAYCONFIG_PIXELFORMAT_FORCE_UINT32 = 0xFFFFFFFF
} DISPLAYCONFIG_PIXELFORMAT;

typedef struct DISPLAYCONFIG_SOURCE_MODE
{
	UINT32                    width;
	UINT32                    height;
	DISPLAYCONFIG_PIXELFORMAT pixelFormat;
	POINTL                    position;
} DISPLAYCONFIG_SOURCE_MODE;

typedef struct DISPLAYCONFIG_DESKTOP_IMAGE_INFO
{
	POINTL PathSourceSize;
	RECTL  DesktopImageRegion;
	RECTL  DesktopImageClip;
} DISPLAYCONFIG_DESKTOP_IMAGE_INFO;

typedef struct DISPLAYCONFIG_MODE_INFO
{
	DISPLAYCONFIG_MODE_INFO_TYPE         infoType;
	UINT32                               id;
	LUID                                 adapterId;
	union
	{
		DISPLAYCONFIG_TARGET_MODE        targetMode;
		DISPLAYCONFIG_SOURCE_MODE        sourceMode;
		DISPLAYCONFIG_DESKTOP_IMAGE_INFO desktopImageInfo;
	} DUMMYUNIONNAME;
} DISPLAYCONFIG_MODE_INFO;

typedef enum DISPLAYCONFIG_TOPOLOGY_ID
{
	DISPLAYCONFIG_TOPOLOGY_INTERNAL     = 0x00000001,
	DISPLAYCONFIG_TOPOLOGY_CLONE        = 0x00000002,
	DISPLAYCONFIG_TOPOLOGY_EXTEND       = 0x00000004,
	DISPLAYCONFIG_TOPOLOGY_EXTERNAL     = 0x00000008,
	DISPLAYCONFIG_TOPOLOGY_FORCE_UINT32 = 0xFFFFFFFF
} DISPLAYCONFIG_TOPOLOGY_ID;

// ======================================

template<typename T>
void maybe_unused(const T&)
{}

// ======================================

template<typename TDst, typename TSrc>
void bit_cast(const TSrc& src, TDst &dst)
{
	const std::size_t size = sizeof(TSrc);
	struct Sanity
	{
		unsigned char value[2 * (size == sizeof(TDst)) - 1];
	};
	maybe_unused(Sanity().value);

	const unsigned char* const src_bytes = reinterpret_cast<const unsigned char*>(&src);
	unsigned char* const dst_bytes = reinterpret_cast<unsigned char*>(&dst);

	for (std::size_t i = 0; i < size; ++i)
	{
		dst_bytes[i] = src_bytes[i];
	}
}

// ======================================

template<typename T>
class ComUPtr
{
public:
	explicit ComUPtr(T* pointer);
	~ComUPtr();

	bool is_empty() const;
	operator T*() const;
	T* operator->() const;

private:
	T* pointer_;

private:
	ComUPtr(const ComUPtr&);
	ComUPtr& operator=(const ComUPtr&);

private:
	static UINT release_iunknown(IUnknown* iunknown);
};

// --------------------------------------

template<typename T>
ComUPtr<T>::ComUPtr(T* pointer)
	:
	pointer_(pointer)
{}

template<typename T>
ComUPtr<T>::~ComUPtr()
{
	if (is_empty())
	{
		return;
	}

	while (release_iunknown(pointer_) != 0)
	{}
}

template<typename T>
bool ComUPtr<T>::is_empty() const
{
	return pointer_ == NULL;
}

template<typename T>
ComUPtr<T>::operator T*() const
{
	return pointer_;
}

template<typename T>
T* ComUPtr<T>::operator->() const
{
	return pointer_;
}

template<typename T>
UINT ComUPtr<T>::release_iunknown(IUnknown* iunknown)
{
	return iunknown->Release();
}

// ======================================

template<typename T>
class UninitializedVector
{
public:
	UninitializedVector();
	~UninitializedVector();

	int get_size() const;
	const T* get_data() const;
	T* get_data();
	const T& operator[](int index) const;

	void resize(int new_size);

private:
	T* elements_;
	int capacity_;
	int size_;

private:
	UninitializedVector(const UninitializedVector&);
	UninitializedVector& operator=(const UninitializedVector&);
};

// --------------------------------------

template<typename T>
UninitializedVector<T>::UninitializedVector()
	:
	elements_(),
	capacity_(),
	size_()
{}

template<typename T>
UninitializedVector<T>::~UninitializedVector()
{
	::operator delete(elements_);
}

template<typename T>
int UninitializedVector<T>::get_size() const
{
	return size_;
}

template<typename T>
const T* UninitializedVector<T>::get_data() const
{
	return elements_;
}

template<typename T>
T* UninitializedVector<T>::get_data()
{
	return elements_;
}

template<typename T>
const T& UninitializedVector<T>::operator[](int index) const
{
	assert(index >= 0 && index < size_);
	return elements_[index];
}

template<typename T>
void UninitializedVector<T>::resize(int new_size)
{
	if (new_size > capacity_)
	{
		::operator delete(elements_);
		elements_ = NULL;
		elements_ = static_cast<T*>(::operator new(new_size * sizeof(T)));
		capacity_ = new_size;
	}

	size_ = new_size;
}

// ======================================

class HdrMgrImpl : public HdrMgr
{
public:
	HdrMgrImpl();
	virtual ~HdrMgrImpl();

private:
	typedef HRESULT (WINAPI * IMPL_CreateDXGIFactory)(REFIID riid, void** ppFactory);

	typedef LONG (WINAPI * IMPL_DisplayConfigGetDeviceInfo)(DISPLAYCONFIG_DEVICE_INFO_HEADER *requestPacket);

	typedef LONG (WINAPI * IMPL_GetDisplayConfigBufferSizes)(
		UINT32  flags,
		UINT32* numPathArrayElements,
		UINT32* numModeInfoArrayElements);

	typedef LONG (WINAPI * IMPL_QueryDisplayConfig)(
		UINT32                     flags,
		UINT32*                    numPathArrayElements,
		DISPLAYCONFIG_PATH_INFO*   pathArray,
		UINT32*                    numModeInfoArrayElements,
		DISPLAYCONFIG_MODE_INFO*   modeInfoArray,
		DISPLAYCONFIG_TOPOLOGY_ID* currentTopologyId);

	typedef UninitializedVector<DISPLAYCONFIG_PATH_INFO> DisplayconfigPathInfoCache;
	typedef UninitializedVector<DISPLAYCONFIG_MODE_INFO> DisplayconfigModeInfoCache;

private:
	static const GUID impl_IID_IDXGIFactory_;
	static const GUID impl_IID_IDXGIOutput6_;

private:
	DisplayconfigPathInfoCache displayconfig_path_info_cache_;
	DisplayconfigModeInfoCache displayconfig_mode_info_cache_;

	HMODULE dxgi_module_;
	IMPL_CreateDXGIFactory impl_CreateDXGIFactory_;

	HMODULE user32_module_;
	IMPL_DisplayConfigGetDeviceInfo impl_DisplayConfigGetDeviceInfo_;
	IMPL_GetDisplayConfigBufferSizes impl_GetDisplayConfigBufferSizes_;
	IMPL_QueryDisplayConfig impl_QueryDisplayConfig_;

private:
	virtual bool do_is_hdr_enabled();
	virtual double do_get_sdr_white_level_double();

private:
	HdrMgrImpl(const HdrMgrImpl&);
	HdrMgrImpl& operator=(const HdrMgrImpl&);

private:
	void do_get_sdr_white_level_double_internal(double& level);
};

// --------------------------------------

const GUID HdrMgrImpl::impl_IID_IDXGIFactory_ = {0x7B7166EC,0x21C7,0x44AE,{0xB2,0x1A,0xC9,0xAE,0x32,0x1A,0xE3,0x69}};
const GUID HdrMgrImpl::impl_IID_IDXGIOutput6_ = {0x068346E8,0xAAEC,0x4B84,{0xAD,0xD7,0x13,0x7F,0x51,0x3F,0x77,0xA1}};

// --------------------------------------

HdrMgrImpl::HdrMgrImpl()
{
	dxgi_module_ = LoadLibraryW(L"dxgi.dll");

	if (dxgi_module_ != NULL)
	{
#define HDR_MGR_IMPL_GET_PROC(symbol) bit_cast(GetProcAddress(dxgi_module_, #symbol), impl_##symbol##_)

		HDR_MGR_IMPL_GET_PROC(CreateDXGIFactory);

#undef HDR_MGR_IMPL_GET_PROC
	}
	else
	{
		impl_CreateDXGIFactory_ = NULL;
	}

	user32_module_ = LoadLibraryW(L"user32.dll");

	if (user32_module_ != NULL)
	{
#define HDR_MGR_IMPL_GET_PROC(symbol) bit_cast(GetProcAddress(user32_module_, #symbol), impl_##symbol##_)

		HDR_MGR_IMPL_GET_PROC(DisplayConfigGetDeviceInfo);
		HDR_MGR_IMPL_GET_PROC(GetDisplayConfigBufferSizes);
		HDR_MGR_IMPL_GET_PROC(QueryDisplayConfig);

#undef HDR_MGR_IMPL_GET_PROC
	}
	else
	{
		impl_DisplayConfigGetDeviceInfo_ = NULL;
		impl_GetDisplayConfigBufferSizes_ = NULL;
		impl_QueryDisplayConfig_ = NULL;
	}
}

HdrMgrImpl::~HdrMgrImpl()
{
	if (user32_module_ != NULL)
	{
		FreeLibrary(user32_module_);
	}
}

bool HdrMgrImpl::do_is_hdr_enabled()
{
	typedef ComUPtr<IDXGIFactory> DxgiFactoryComUPtr;
	typedef ComUPtr<IDXGIAdapter> DxgiAdapterComUPtr;
	typedef ComUPtr<IDXGIOutput> DxgiOutputComUPtr;

	// Factory
	//
	if (impl_CreateDXGIFactory_ == NULL)
	{
		return false;
	}

	void* dxgi_factory_raw;

	if (FAILED(impl_CreateDXGIFactory_(impl_IID_IDXGIFactory_, &dxgi_factory_raw)))
	{
		return false;
	}

	DxgiFactoryComUPtr dxgi_factory(static_cast<IDXGIFactory*>(dxgi_factory_raw));

	// Adapter
	//
	IDXGIAdapter* dxgi_adapter_raw;

	if (FAILED(dxgi_factory->EnumAdapters(0, &dxgi_adapter_raw)))
	{
		return false;
	}

	DxgiAdapterComUPtr dxgi_adapter(dxgi_adapter_raw);

	// Output
	//
	IDXGIOutput* dxgi_output_raw;

	if (FAILED(dxgi_adapter->EnumOutputs(0, &dxgi_output_raw)))
	{
		return false;
	}

	DxgiOutputComUPtr dxgi_output(dxgi_output_raw);

	// Output6
	//
	void* dxgi_output6_raw;

	if (FAILED(dxgi_output->QueryInterface(impl_IID_IDXGIOutput6_, &dxgi_output6_raw)))
	{
		return false;
	}

	IDXGIOutput6* const dxgi_output6 = static_cast<IDXGIOutput6*>(dxgi_output6_raw);

	// Desc1
	//
	DXGI_OUTPUT_DESC1 dxgi_output_desc1;

	if (FAILED(dxgi_output6->GetDesc1(&dxgi_output_desc1)))
	{
		return false;
	}

	// Result
	//
	return dxgi_output_desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
}

double HdrMgrImpl::do_get_sdr_white_level_double()
{
	double level;
	do_get_sdr_white_level_double_internal(level);
	return level;
}

void HdrMgrImpl::do_get_sdr_white_level_double_internal(double& output_level)
{
	output_level = 1.0;

	if (impl_DisplayConfigGetDeviceInfo_ == NULL ||
		impl_GetDisplayConfigBufferSizes_ == NULL ||
		impl_QueryDisplayConfig_ == NULL)
	{
		return;
	}

	const UINT32 flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
	UINT32 path_count = 0;
	UINT32 mode_count = 0;

	const LONG get_display_config_buffer_sizes_result = impl_GetDisplayConfigBufferSizes_(
		/* flags */                    flags,
		/* numPathArrayElements */     &path_count,
		/* numModeInfoArrayElements */ &mode_count
	);

	if (get_display_config_buffer_sizes_result != ERROR_SUCCESS)
	{
		return;
	}

	if (path_count == 0 || mode_count == 0)
	{
		return;
	}

	displayconfig_path_info_cache_.resize(path_count);
	displayconfig_mode_info_cache_.resize(mode_count);

	const LONG query_display_config_result = impl_QueryDisplayConfig_(
		/* flags */                    flags,
		/* numPathArrayElements */     &path_count,
		/* pathArray */                displayconfig_path_info_cache_.get_data(),
		/* numModeInfoArrayElements */ &mode_count,
		/* modeInfoArray */            displayconfig_mode_info_cache_.get_data(),
		/* currentTopologyId */        NULL
	);

	if (query_display_config_result != ERROR_SUCCESS)
	{
		return;
	}

	const DISPLAYCONFIG_PATH_INFO& path_info = displayconfig_path_info_cache_[0];

	DISPLAYCONFIG_SDR_WHITE_LEVEL displayconfig_sdr_white_level;
	displayconfig_sdr_white_level.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;
	displayconfig_sdr_white_level.header.size = sizeof(DISPLAYCONFIG_SDR_WHITE_LEVEL);
	displayconfig_sdr_white_level.header.adapterId = path_info.targetInfo.adapterId;
	displayconfig_sdr_white_level.header.id = path_info.targetInfo.id;

	const LONG display_config_get_device_info_result =
		impl_DisplayConfigGetDeviceInfo_(&displayconfig_sdr_white_level.header);

	if (display_config_get_device_info_result != ERROR_SUCCESS)
	{
		return;
	}

	output_level = static_cast<double>(displayconfig_sdr_white_level.SDRWhiteLevel) / 1000.0;
}

} // namespace

// ======================================

HdrMgr* make_hdr_mgr()
{
	return new HdrMgrImpl();
}

} // namespace rtcw

#endif // _WIN32
