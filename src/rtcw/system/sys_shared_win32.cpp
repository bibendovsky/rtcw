/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2026 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifdef _WIN32

#include "sys_shared.h"
#include "rtcw_memory.h"
#include <cassert>
#include <cstddef>
#include <windows.h>

namespace {

template<typename T>
void maybe_unused(const T&)
{}

// =====================================

class SysDirWin32
{
public:
	SysDirWin32(const char* path);
	~SysDirWin32();

	static SysDirWin32* create(const char* path);
	static void destroy(SysDirWin32* sys_dir);

	const SysDirEntry* read();

private:
	static const int max_path_size = 1024;

	bool is_next_;
	HANDLE win32_handle_;
	WIN32_FIND_DATAW win32_find_data_;
	wchar_t u16_path_[max_path_size];
	char u8_name_[MAX_PATH];
	SysDirEntry entry_;

	SysDirWin32(const SysDirWin32&);
	SysDirWin32& operator=(const SysDirWin32&);

	bool is_open() const;
	void close();
	bool update_entry();
};

// -------------------------------------

SysDirWin32::SysDirWin32(const char* path)
	:
	is_next_(),
	win32_handle_(INVALID_HANDLE_VALUE)
{
	if (path[0] == '\0' || (path[0] == '.' && path[1] == '\0'))
	{
		u16_path_[0] = L'.';
		u16_path_[1] = L'\0';
	}
	else
	{
		// Convert the path to UTF-16.
		int u16_path_size = MultiByteToWideChar(CP_UTF8, 0, path, -1, u16_path_, max_path_size);
		if (u16_path_size == 0)
		{
			assert(false && "MultiByteToWideChar");
			return;
		}
		if (u16_path_size < 2)
		{
			assert(false && "Source path too short.");
			return;
		}
		// Append separator.
		if (u16_path_[u16_path_size - 2] != L'\\')
		{
			if (u16_path_size == max_path_size)
			{
				assert(false && "Path buffer too small.");
				return;
			}
			u16_path_[u16_path_size - 1] = L'\\';
			u16_path_[u16_path_size] = L'\0';
			++u16_path_size;
		}
		// Append pattern.
		if (u16_path_size == max_path_size)
		{
			assert(false && "Path buffer too small.");
			return;
		}
		u16_path_[u16_path_size - 1] = L'*';
		u16_path_[u16_path_size] = L'\0';
	}
	// Make API call.
	win32_handle_ = FindFirstFileW(u16_path_, &win32_find_data_);
	if (!is_open())
	{
		return;
	}
	// Initialize the entry.
	if (!update_entry())
	{
		close();
		return;
	}
}

SysDirWin32::~SysDirWin32()
{
	close();
}

SysDirWin32* SysDirWin32::create(const char* path)
{
	SysDirWin32* const sys_dir = rtcw::mem::new_object_1<SysDirWin32>(path);
	if (sys_dir == NULL || !sys_dir->is_open())
	{
		SysDirWin32::destroy(sys_dir);
		return NULL;
	}
	return sys_dir;
}

void SysDirWin32::destroy(SysDirWin32* sys_dir)
{
	rtcw::mem::delete_object(sys_dir);
}

const SysDirEntry* SysDirWin32::read()
{
	assert(is_open());
	if (!is_next_)
	{
		is_next_ = true;
		return &entry_;
	}
	if (FindNextFileW(win32_handle_, &win32_find_data_) != TRUE)
	{
		return NULL;
	}
	if (!update_entry())
	{
		return NULL;
	}
	return &entry_;
}

bool SysDirWin32::is_open() const
{
	return win32_handle_ != INVALID_HANDLE_VALUE;
}

void SysDirWin32::close()
{
	if (!is_open())
	{
		return;
	}
	const BOOL bool_result = FindClose(win32_handle_);
	assert(bool_result == TRUE);
	maybe_unused(bool_result);
	win32_handle_ = INVALID_HANDLE_VALUE;
}

bool SysDirWin32::update_entry()
{
	// Convert file name to UTF-8.
	const int u8_size = WideCharToMultiByte(CP_UTF8, 0, win32_find_data_.cFileName, -1, u8_name_, MAX_PATH, NULL, NULL);
	if (u8_size == 0)
	{
		return false;
	}
	// Update the entry.
	entry_.is_dir = (win32_find_data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	entry_.name = u8_name_;
	return true;
}

} // namespace

// =====================================

SysDirHandle sys_open_dir(const char* path)
{
	return reinterpret_cast<SysDirHandle>(SysDirWin32::create(path));
}

const SysDirEntry* sys_read_dir(SysDirHandle handle)
{
	return reinterpret_cast<SysDirWin32*>(handle)->read();
}

void sys_close_dir(SysDirHandle& handle)
{
	SysDirWin32::destroy(reinterpret_cast<SysDirWin32*>(handle));
}

// ==========================================================================

void Sys_Mkdir(const char* path)
{
	const int max_path_size = 1024;
	wchar_t u16_path[max_path_size];
	// Convert the path to UTF-16.
	const int u16_path_size = MultiByteToWideChar(CP_UTF8, 0, path, -1, u16_path, max_path_size);
	if (u16_path_size == 0)
	{
		assert(false && "MultiByteToWideChar");
		return;
	}
	const BOOL win32_result = CreateDirectoryW(u16_path, NULL);
#ifndef NDEBUG
	if (win32_result != TRUE)
	{
		const DWORD last_error = GetLastError();
		assert(last_error == ERROR_ALREADY_EXISTS);
	}
#endif // NDEBUG
	maybe_unused(win32_result);
}

// ==========================================================================

char* Sys_Cwd()
{
	const DWORD max_path_size = 1024;
	static char u8_path[max_path_size];
	wchar_t u16_path[max_path_size];
	const DWORD u16_path_length = GetCurrentDirectoryW(max_path_size - 1, u16_path);
	if (u16_path_length > max_path_size - 1)
	{
		assert(false && "Path buffer too small.");
		u8_path[0] = '\0';
		return u8_path;
	}
	const int u16_size = static_cast<int>(u16_path_length + 1);
	const int u8_size = WideCharToMultiByte(CP_UTF8, 0, u16_path, u16_size, u8_path, static_cast<int>(max_path_size), NULL, NULL);
	if (u8_size == 0)
	{
		assert(false && "Path buffer too small.");
		u8_path[0] = '\0';
		return u8_path;
	}
	return u8_path;
}

// =====================================

int Sys_Milliseconds()
{
	static bool is_initialized = false;
	static DWORD time_base_ms = 0;
	const DWORD current_time_ms = timeGetTime();
	if (!is_initialized)
	{
		is_initialized = true;
		time_base_ms = current_time_ms;
		return 0;
	}
	const DWORD time_diff = current_time_ms - time_base_ms;
	return static_cast<int>(time_diff);
}

#endif // _WIN32
