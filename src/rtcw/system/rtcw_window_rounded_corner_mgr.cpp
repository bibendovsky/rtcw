/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_window_rounded_corner_mgr.h"
#ifdef _WIN32
#include <stddef.h>
#include <windows.h>
#include "SDL_syswm.h"
#include "rtcw_unique_ptr.h"
#endif // _WIN32

namespace rtcw {

#ifdef _WIN32

class WindowRoundedCornerMgr::Impl
{
public:
	Impl();
	~Impl();

	static void disable(SDL_Window* sdl_window);

private:
	static const DWORD WIN32_DWMWCP_DONOTROUND = 1;
	static const DWORD WIN32_DWMWA_WINDOW_CORNER_PREFERENCE = 33;

private:
	typedef UniquePtr<WindowRoundedCornerMgr::Impl> Instance;

	typedef HRESULT (WINAPI * PFNDWMSETWINDOWATTRIBUTEPROC)(
		HWND hwnd,
		DWORD dwAttribute,
		LPCVOID pvAttribute,
		DWORD cbAttribute);

private:
	static Instance instance_;

	HMODULE dwmapi_module_;
	PFNDWMSETWINDOWATTRIBUTEPROC dwm_set_window_attribute_;

private:
	Impl(const Impl&);
	Impl& operator=(const Impl&);

	bool initialize();
	void terminate();
	void disable(HWND win32_window) const;
};

// --------------------------------------------------------------------------

WindowRoundedCornerMgr::Impl::Instance WindowRoundedCornerMgr::Impl::instance_;

// --------------------------------------------------------------------------

WindowRoundedCornerMgr::Impl::Impl()
{
	if (!initialize())
	{
		terminate();
	}
}

WindowRoundedCornerMgr::Impl::~Impl()
{
	terminate();
}

void WindowRoundedCornerMgr::Impl::disable(SDL_Window* sdl_window)
{
	SDL_SysWMinfo sdl_sys_wm_info = SDL_SysWMinfo();
	SDL_VERSION(&sdl_sys_wm_info.version);

	if (SDL_GetWindowWMInfo(sdl_window, &sdl_sys_wm_info) != SDL_TRUE ||
		sdl_sys_wm_info.info.win.window == NULL)
	{
		return;
	}

	if (instance_.get() == NULL)
	{
		instance_.reset(new Impl());
	}

	instance_->disable(sdl_sys_wm_info.info.win.window);
}

bool WindowRoundedCornerMgr::Impl::initialize()
{
	dwmapi_module_ = LoadLibraryW(L"dwmapi.dll");

	if (dwmapi_module_ == NULL)
	{
		return false;
	}

	union Cast
	{
		FARPROC from;
		PFNDWMSETWINDOWATTRIBUTEPROC to;
	};

	const Cast cast = {GetProcAddress(dwmapi_module_, "DwmSetWindowAttribute")};
	dwm_set_window_attribute_ = cast.to;

	if (dwm_set_window_attribute_ == NULL)
	{
		return false;
	}

	return true;
}

void WindowRoundedCornerMgr::Impl::terminate()
{
	dwm_set_window_attribute_ = NULL;

	if (dwmapi_module_ != NULL)
	{
		FreeLibrary(dwmapi_module_);
		dwmapi_module_ = NULL;
	}
}

void WindowRoundedCornerMgr::Impl::disable(HWND win32_window) const
{
	if (dwm_set_window_attribute_ == NULL)
	{
		return;
	}

	const DWORD win32_rounded_corner_type = WIN32_DWMWCP_DONOTROUND;

	dwm_set_window_attribute_(
		win32_window,
		WIN32_DWMWA_WINDOW_CORNER_PREFERENCE,
		&win32_rounded_corner_type,
		static_cast<DWORD>(sizeof(win32_rounded_corner_type)));
}

#endif // _WIN32

// ==========================================================================

#ifdef _WIN32

void WindowRoundedCornerMgr::disable(SDL_Window* sdl_window)
{
	Impl::disable(sdl_window);
}

#else

void WindowRoundedCornerMgr::disable(SDL_Window*)
{}

#endif // _WIN32

} // namespace rtcw
