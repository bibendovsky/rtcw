/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifdef _WIN32

#undef WINVER
#define WINVER 0x0501

#define CINTERFACE
#define COBJMACROS
 
#include "rtcw_hdr_mgr.h"
#include <cassert>
#include <cstddef>
#include <windows.h>

namespace rtcw {

namespace {

// ======================================
// DXGI
// --------------------------------------

typedef INT32 DXGI_MODE_ROTATION;

// --------------------------------------

enum DXGI_COLOR_SPACE_TYPE
{
	DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 = 12,
	DXGI_COLOR_SPACE_CUSTOM                     = 0xFFFFFFFF
};

// --------------------------------------

struct DXGI_OUTPUT_DESC1
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
};

struct DXGI_COLOR_SPACE_TYPE_Validator
{
	static const bool is_64_bit = sizeof(void*) == 8;
	static const std::size_t struct_size = 144 + is_64_bit * 8;
	static const std::size_t color_space_offset = 96 + is_64_bit * 4;
	bool validate_struct_size[2 * (sizeof(DXGI_OUTPUT_DESC1) == struct_size) - 1];
	bool validate_color_space_offset[2 * (offsetof(DXGI_OUTPUT_DESC1, ColorSpace) == color_space_offset) - 1];
};

// --------------------------------------

struct IDXGIOutput;

struct IDXGIOutputVtbl
{
	HRESULT (STDMETHODCALLTYPE* QueryInterface)(IDXGIOutput* This, REFIID riid, void** ppvObject);
	ULONG (STDMETHODCALLTYPE* AddRef)(IDXGIOutput* This);
	ULONG (STDMETHODCALLTYPE* Release)(IDXGIOutput* This);
};

struct IDXGIOutput
{
	const IDXGIOutputVtbl* lpVtbl;
};

// --------------------------------------

struct IDXGIAdapter;

struct IDXGIAdapterVtbl
{
	HRESULT (STDMETHODCALLTYPE* QueryInterface)(IDXGIAdapter* This, REFIID riid, void** ppvObject);
	ULONG (STDMETHODCALLTYPE* AddRef)(IDXGIAdapter* This);
	ULONG (STDMETHODCALLTYPE* Release)(IDXGIAdapter* This);

	FARPROC SetPrivateData;
	FARPROC SetPrivateDataInterface;
	FARPROC GetPrivateData;
	FARPROC GetParent;
	HRESULT (STDMETHODCALLTYPE* EnumOutputs)(IDXGIAdapter* This, UINT Output, IDXGIOutput** ppOutput);
	FARPROC GetDesc;
	FARPROC CheckInterfaceSupport;
};

struct IDXGIAdapter
{
	const IDXGIAdapterVtbl* lpVtbl;
};

#define IDXGIAdapter_EnumOutputs(This,Output,ppOutput) \
	((This)->lpVtbl->EnumOutputs(This,Output,ppOutput))

// --------------------------------------

struct IDXGIFactory;

struct IDXGIFactoryVtbl
{
	HRESULT (STDMETHODCALLTYPE* QueryInterface)(IDXGIFactory* This, REFIID riid, void** ppvObject);
	ULONG (STDMETHODCALLTYPE* AddRef)(IDXGIFactory* This);
	ULONG (STDMETHODCALLTYPE* Release)(IDXGIFactory* This);

	FARPROC SetPrivateData;
	FARPROC SetPrivateDataInterface;
	FARPROC GetPrivateData;
	FARPROC GetParent;
	HRESULT (STDMETHODCALLTYPE* EnumAdapters)(IDXGIFactory* This, UINT Adapter, IDXGIAdapter** ppAdapter);
	FARPROC MakeWindowAssociation;
	FARPROC GetWindowAssociation;
	FARPROC CreateSwapChain;
	FARPROC CreateSoftwareAdapter;
};

struct IDXGIFactory
{
	const IDXGIFactoryVtbl* lpVtbl;
};

#define IDXGIFactory_EnumAdapters(This,Adapter,ppAdapter) \
	((This)->lpVtbl->EnumAdapters(This,Adapter,ppAdapter))

// --------------------------------------

struct IDXGIOutput6;

struct IDXGIOutput6Vtbl
{
	HRESULT (STDMETHODCALLTYPE* QueryInterface)(IDXGIOutput6* This, REFIID riid, void** ppvObject);
	ULONG (STDMETHODCALLTYPE* AddRef)(IDXGIOutput6* This);
	ULONG (STDMETHODCALLTYPE* Release)(IDXGIOutput6* This);

	FARPROC SetPrivateData;
	FARPROC SetPrivateDataInterface;
	FARPROC GetPrivateData;
	FARPROC GetParent;
	FARPROC GetDesc;
	FARPROC GetDisplayModeList;
	FARPROC FindClosestMatchingMode;
	FARPROC WaitForVBlank;
	FARPROC TakeOwnership;
	FARPROC ReleaseOwnership;
	FARPROC GetGammaControlCapabilities;
	FARPROC SetGammaControl;
	FARPROC GetGammaControl;
	FARPROC SetDisplaySurface;
	FARPROC GetDisplaySurfaceData;
	FARPROC GetFrameStatistics;

	FARPROC GetDisplayModeList1;
	FARPROC FindClosestMatchingMode1;
	FARPROC GetDisplaySurfaceData1;
	FARPROC DuplicateOutput;

	FARPROC SupportsOverlays;

	FARPROC CheckOverlaySupport;

	FARPROC CheckOverlayColorSpaceSupport;

	FARPROC DuplicateOutput1;
	HRESULT (STDMETHODCALLTYPE* GetDesc1)(IDXGIOutput6* This, DXGI_OUTPUT_DESC1* pDesc);
	FARPROC CheckHardwareCompositionSupport;
};

struct IDXGIOutput6
{
	const IDXGIOutput6Vtbl* lpVtbl;
};

#define IDXGIOutput6_QueryInterface(This,riid,ppvObject) \
	((This)->lpVtbl->QueryInterface(This,riid,ppvObject))

#define IDXGIOutput6_GetDesc1(This,pDesc) \
	((This)->lpVtbl->GetDesc1(This,pDesc))

// ======================================
// Display Config
// --------------------------------------

#define QDC_ONLY_ACTIVE_PATHS  0x00000002
#define QDC_VIRTUAL_MODE_AWARE 0x00000010

// --------------------------------------

typedef void DISPLAYCONFIG_TOPOLOGY_ID;

// --------------------------------------

enum DISPLAYCONFIG_DEVICE_INFO_TYPE
{
	DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL = 11,
	DISPLAYCONFIG_DEVICE_INFO_FORCE_UINT32        = 0xFFFFFFFF
};

enum DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY
{
	DISPLAYCONFIG_OUTPUT_TECHNOLOGY_FORCE_UINT32 = 0xFFFFFFFF
};

enum DISPLAYCONFIG_ROTATION
{
	DISPLAYCONFIG_ROTATION_FORCE_UINT32 = 0xFFFFFFFF
};

enum DISPLAYCONFIG_SCALING
{
	DISPLAYCONFIG_SCALING_FORCE_UINT32 = 0xFFFFFFFF
};

enum DISPLAYCONFIG_SCANLINE_ORDERING
{
	DISPLAYCONFIG_SCANLINE_ORDERING_FORCE_UINT32 = 0xFFFFFFFF
};

// --------------------------------------

struct DISPLAYCONFIG_RATIONAL
{
	UINT32 Numerator;
	UINT32 Denominator;
};

// --------------------------------------

struct DISPLAYCONFIG_PATH_SOURCE_INFO
{
	LUID   adapterId;
	UINT32 id;
	UINT32 modeInfoIdx;
	UINT32 statusFlags;
};

// --------------------------------------

struct DISPLAYCONFIG_PATH_TARGET_INFO
{
	LUID                                  adapterId;
	UINT32                                id;
	UINT32                                modeInfoIdx;
	DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY outputTechnology;
	DISPLAYCONFIG_ROTATION                rotation;
	DISPLAYCONFIG_SCALING                 scaling;
	DISPLAYCONFIG_RATIONAL                refreshRate;
	DISPLAYCONFIG_SCANLINE_ORDERING       scanLineOrdering;
	BOOL                                  targetAvailable;
	UINT32                                statusFlags;
};

struct DISPLAYCONFIG_PATH_TARGET_INFO_Validator
{
	static const std::size_t struct_size = 48;
	static const std::size_t adapter_id_offset = 0;
	static const std::size_t id_offset = 8;
	bool validate_struct_size[2 * (sizeof(DISPLAYCONFIG_PATH_TARGET_INFO) == struct_size) - 1];
	bool validate_adapter_id_offset[2 * (offsetof(DISPLAYCONFIG_PATH_TARGET_INFO, adapterId) == adapter_id_offset) - 1];
	bool validate_id_offset[2 * (offsetof(DISPLAYCONFIG_PATH_TARGET_INFO, id) == id_offset) - 1];
};

// --------------------------------------

struct DISPLAYCONFIG_PATH_INFO
{
	DISPLAYCONFIG_PATH_SOURCE_INFO sourceInfo;
	DISPLAYCONFIG_PATH_TARGET_INFO targetInfo;
	UINT32                         flags;
};

struct DISPLAYCONFIG_PATH_INFO_Validator
{
	static const std::size_t struct_size = 72;
	bool validate_struct_size[2 * (sizeof(DISPLAYCONFIG_PATH_INFO) == struct_size) - 1];
};

// --------------------------------------

class DISPLAYCONFIG_MODE_INFO
{
public:
	DISPLAYCONFIG_MODE_INFO() {}
	~DISPLAYCONFIG_MODE_INFO() {}

private:
	unsigned char storage_[64];

	DISPLAYCONFIG_MODE_INFO(const DISPLAYCONFIG_MODE_INFO&);
	DISPLAYCONFIG_MODE_INFO& operator=(const DISPLAYCONFIG_MODE_INFO&);
};

// --------------------------------------

struct DISPLAYCONFIG_DEVICE_INFO_HEADER
{
	DISPLAYCONFIG_DEVICE_INFO_TYPE type;
	UINT32                         size;
	LUID                           adapterId;
	UINT32                         id;
};

struct DISPLAYCONFIG_DEVICE_INFO_HEADER_Validator
{
	static const std::size_t struct_size = 20;
	static const std::size_t type_offset = 0;
	static const std::size_t size_offset = 4;
	static const std::size_t adapter_id_offset = 8;
	static const std::size_t id_offset = 16;
	bool validate_struct_size[2 * (sizeof(DISPLAYCONFIG_DEVICE_INFO_HEADER) == struct_size) - 1];
	bool validate_type_offset[2 * (offsetof(DISPLAYCONFIG_DEVICE_INFO_HEADER, type) == type_offset) - 1];
	bool validate_size_offset[2 * (offsetof(DISPLAYCONFIG_DEVICE_INFO_HEADER, size) == size_offset) - 1];
	bool validate_adapter_id_offset[2 * (offsetof(DISPLAYCONFIG_DEVICE_INFO_HEADER, adapterId) == adapter_id_offset) - 1];
	bool validate_id_offset[2 * (offsetof(DISPLAYCONFIG_DEVICE_INFO_HEADER, id) == id_offset) - 1];
};

// --------------------------------------

struct DISPLAYCONFIG_SDR_WHITE_LEVEL
{
	DISPLAYCONFIG_DEVICE_INFO_HEADER header;
	ULONG                            SDRWhiteLevel;
};

// ======================================

template<typename T>
void maybe_unused(const T&)
{}

// ======================================

template<typename TDst, typename TSrc>
void bit_cast(const TSrc& src, TDst &dst)
{
	const std::size_t size = sizeof(TSrc);
	struct Validator
	{
		bool validate_struct_size[2 * (size == sizeof(TDst)) - 1];
	};
	maybe_unused(Validator().validate_struct_size);
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
	ComUPtr();
	explicit ComUPtr(T* pointer);
	~ComUPtr();

	bool is_empty() const;
	operator T*() const;
	operator T**();
	operator void**();
	T* operator->() const;

private:
	T* pointer_;

	ComUPtr(const ComUPtr&);
	ComUPtr& operator=(const ComUPtr&);
};

// --------------------------------------

template<typename T>
ComUPtr<T>::ComUPtr()
	:
	pointer_()
{}

template<typename T>
ComUPtr<T>::ComUPtr(T* pointer)
	:
	pointer_(pointer)
{}

template<typename T>
ComUPtr<T>::~ComUPtr()
{
	if (!is_empty())
	{
		while (IUnknown_Release(reinterpret_cast<IUnknown*>(pointer_)) != 0)
		{}
	}
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
ComUPtr<T>::operator T**()
{
	return &pointer_;
}

template<typename T>
ComUPtr<T>::operator void**()
{
	return reinterpret_cast<void**>(&pointer_);
}

template<typename T>
T* ComUPtr<T>::operator->() const
{
	return pointer_;
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
	typedef HRESULT (WINAPI* PFN_CreateDXGIFactory)(REFIID riid, void** ppFactory);
	typedef LONG (WINAPI* PFN_DisplayConfigGetDeviceInfo)(DISPLAYCONFIG_DEVICE_INFO_HEADER *requestPacket);
	typedef LONG (WINAPI* PFN_GetDisplayConfigBufferSizes)(
		UINT32  flags,
		UINT32* numPathArrayElements,
		UINT32* numModeInfoArrayElements);
	typedef LONG (WINAPI* PFN_QueryDisplayConfig)(
		UINT32                     flags,
		UINT32*                    numPathArrayElements,
		DISPLAYCONFIG_PATH_INFO*   pathArray,
		UINT32*                    numModeInfoArrayElements,
		DISPLAYCONFIG_MODE_INFO*   modeInfoArray,
		DISPLAYCONFIG_TOPOLOGY_ID* currentTopologyId);

	typedef UninitializedVector<DISPLAYCONFIG_PATH_INFO> DisplayconfigPathInfoCache;
	typedef UninitializedVector<DISPLAYCONFIG_MODE_INFO> DisplayconfigModeInfoCache;

	static const GUID IID_IDXGIFactory_;
	static const GUID IID_IDXGIOutput6_;

	DisplayconfigPathInfoCache displayconfig_path_info_cache_;
	DisplayconfigModeInfoCache displayconfig_mode_info_cache_;

	HMODULE dxgi_module_;
	PFN_CreateDXGIFactory CreateDXGIFactory_;

	HMODULE user32_module_;
	PFN_DisplayConfigGetDeviceInfo DisplayConfigGetDeviceInfo_;
	PFN_GetDisplayConfigBufferSizes GetDisplayConfigBufferSizes_;
	PFN_QueryDisplayConfig QueryDisplayConfig_;

	virtual bool do_is_hdr_enabled();
	virtual double do_get_sdr_white_level_double();

	void do_get_sdr_white_level_double_internal(double& level);

	HdrMgrImpl(const HdrMgrImpl&);
	HdrMgrImpl& operator=(const HdrMgrImpl&);

	FARPROC get_dxgi_dll_symbol(const char* symbol_name) const;
	FARPROC get_user32_dll_symbol(const char* symbol_name) const;
};

// --------------------------------------

const GUID HdrMgrImpl::IID_IDXGIFactory_ = {0x7B7166EC,0x21C7,0x44AE,{0xB2,0x1A,0xC9,0xAE,0x32,0x1A,0xE3,0x69}};
const GUID HdrMgrImpl::IID_IDXGIOutput6_ = {0x068346E8,0xAAEC,0x4B84,{0xAD,0xD7,0x13,0x7F,0x51,0x3F,0x77,0xA1}};

// --------------------------------------

HdrMgrImpl::HdrMgrImpl()
{
	// dxgi.dll
	//
	dxgi_module_ = LoadLibraryW(L"dxgi.dll");
	if (dxgi_module_ != NULL)
	{
#define RTCW_MACRO(symbol) bit_cast(get_dxgi_dll_symbol(#symbol), symbol##_)
		RTCW_MACRO(CreateDXGIFactory);
#undef RTCW_MACRO
	}
	else
	{
		CreateDXGIFactory_ = NULL;
	}
	// user32.dll
	//
	user32_module_ = LoadLibraryW(L"user32.dll");
	if (user32_module_ != NULL)
	{
#define RTCW_MACRO(symbol) bit_cast(get_user32_dll_symbol(#symbol), symbol##_)
		RTCW_MACRO(DisplayConfigGetDeviceInfo);
		RTCW_MACRO(GetDisplayConfigBufferSizes);
		RTCW_MACRO(QueryDisplayConfig);
#undef RTCW_MACRO
	}
	else
	{
		DisplayConfigGetDeviceInfo_ = NULL;
		GetDisplayConfigBufferSizes_ = NULL;
		QueryDisplayConfig_ = NULL;
	}
}

HdrMgrImpl::~HdrMgrImpl()
{
	if (dxgi_module_ != NULL)
	{
		FreeLibrary(dxgi_module_);
	}
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
	// IDXGIFactory
	//
	if (CreateDXGIFactory_ == NULL)
	{
		return false;
	}
	DxgiFactoryComUPtr dxgi_factory;
	if (FAILED(CreateDXGIFactory_(IID_IDXGIFactory_, dxgi_factory)))
	{
		return false;
	}
	// IDXGIAdapter
	//
	DxgiAdapterComUPtr dxgi_adapter;
	if (FAILED(IDXGIFactory_EnumAdapters(dxgi_factory, 0, dxgi_adapter)))
	{
		return false;
	}
	// IDXGIOutput
	//
	DxgiOutputComUPtr dxgi_output;
	if (FAILED(IDXGIAdapter_EnumOutputs(dxgi_adapter, 0, dxgi_output)))
	{
		return false;
	}
	// IDXGIOutput6
	//
	IDXGIOutput6* dxgi_output6 = NULL;
	if (FAILED(IDXGIOutput6_QueryInterface(dxgi_output, IID_IDXGIOutput6_, reinterpret_cast<void**>(&dxgi_output6))))
	{
		return false;
	}
	// DXGI_OUTPUT_DESC1
	//
	DXGI_OUTPUT_DESC1 dxgi_output_desc1;
	if (FAILED(IDXGIOutput6_GetDesc1(dxgi_output6, &dxgi_output_desc1)))
	{
		return false;
	}
	//
	const DXGI_COLOR_SPACE_TYPE& color_space = dxgi_output_desc1.ColorSpace;
	return color_space == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
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
	if (DisplayConfigGetDeviceInfo_ == NULL ||
		GetDisplayConfigBufferSizes_ == NULL ||
		QueryDisplayConfig_ == NULL)
	{
		return;
	}
	// GetDisplayConfigBufferSizes
	//
	const UINT32 flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
	UINT32 path_count = 0;
	UINT32 mode_count = 0;
	const LONG get_display_config_buffer_sizes_result = GetDisplayConfigBufferSizes_(
		/* flags */                    flags,
		/* numPathArrayElements */     &path_count,
		/* numModeInfoArrayElements */ &mode_count);
	if (get_display_config_buffer_sizes_result != ERROR_SUCCESS ||
		path_count == 0 ||
		mode_count == 0)
	{
		return;
	}
	displayconfig_path_info_cache_.resize(path_count);
	displayconfig_mode_info_cache_.resize(mode_count);
	// QueryDisplayConfig
	//
	const LONG query_display_config_result = QueryDisplayConfig_(
		/* flags */                    flags,
		/* numPathArrayElements */     &path_count,
		/* pathArray */                displayconfig_path_info_cache_.get_data(),
		/* numModeInfoArrayElements */ &mode_count,
		/* modeInfoArray */            displayconfig_mode_info_cache_.get_data(),
		/* currentTopologyId */        NULL);
	if (query_display_config_result != ERROR_SUCCESS)
	{
		return;
	}
	// DisplayConfigGetDeviceInfo
	//
	const DISPLAYCONFIG_PATH_INFO& path_info = displayconfig_path_info_cache_[0];
	DISPLAYCONFIG_SDR_WHITE_LEVEL displayconfig_sdr_white_level = DISPLAYCONFIG_SDR_WHITE_LEVEL();
	displayconfig_sdr_white_level.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;
	displayconfig_sdr_white_level.header.size = sizeof(DISPLAYCONFIG_SDR_WHITE_LEVEL);
	displayconfig_sdr_white_level.header.adapterId = path_info.targetInfo.adapterId;
	displayconfig_sdr_white_level.header.id = path_info.targetInfo.id;
	const LONG display_config_get_device_info_result = DisplayConfigGetDeviceInfo_(&displayconfig_sdr_white_level.header);
	if (display_config_get_device_info_result == ERROR_SUCCESS)
	{
		output_level = static_cast<double>(displayconfig_sdr_white_level.SDRWhiteLevel) / 1000.0;
	}
}

FARPROC HdrMgrImpl::get_dxgi_dll_symbol(const char* symbol_name) const
{
	return GetProcAddress(dxgi_module_, symbol_name);
}

FARPROC HdrMgrImpl::get_user32_dll_symbol(const char* symbol_name) const
{
	return GetProcAddress(user32_module_, symbol_name);
}

} // namespace

// ======================================

HdrMgr* make_hdr_mgr()
{
	return new HdrMgrImpl();
}

} // namespace rtcw

#endif // _WIN32
