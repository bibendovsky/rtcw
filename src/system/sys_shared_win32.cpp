/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include "sys_shared.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <new>
#include <windows.h>

// ==========================================================================

namespace {

template<typename T>
void ignore_result(const T&) {}

// ==========================================================================

class SysDirWin32
{
public:
	static SysDirWin32* create(const char* path);
	static void destroy(SysDirWin32* sys_dir);

	const SysDirEntry* read();

private:
	static const int max_path_size = 1024;

private:
	bool is_next_;
	HANDLE win32_handle_;
	WIN32_FIND_DATAW win32_find_data_;
	wchar_t u16_path_[max_path_size];
	char u8_name_[MAX_PATH];
	SysDirEntry entry_;

private:
	SysDirWin32(const SysDirWin32&);
	SysDirWin32& operator=(const SysDirWin32&);

private:
	SysDirWin32(const char* path);
	~SysDirWin32();

	bool is_open() const;
	void close();
	bool update_entry();
};

// --------------------------------------------------------------------------

SysDirWin32* SysDirWin32::create(const char* path)
{
	SysDirWin32* sys_dir = static_cast<SysDirWin32*>(::malloc(sizeof(SysDirWin32)));

	if (sys_dir == NULL)
	{
		return NULL;
	}

	::new (static_cast<void*>(sys_dir)) SysDirWin32(path);

	if (!sys_dir->is_open())
	{
		SysDirWin32::destroy(sys_dir);
		return NULL;
	}

	return sys_dir;
}

void SysDirWin32::destroy(SysDirWin32* sys_dir)
{
	sys_dir->~SysDirWin32();
	::free(sys_dir);
}

const SysDirEntry* SysDirWin32::read()
{
	assert(is_open());

	if (!is_next_)
	{
		is_next_ = true;
		return &entry_;
	}

	if (::FindNextFileW(win32_handle_, &win32_find_data_) != TRUE)
	{
		return NULL;
	}

	if (!update_entry())
	{
		return NULL;
	}

	return &entry_;
}

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
		//
		int u16_path_size = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, u16_path_, max_path_size);

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
		//
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
		//
		if (u16_path_size == max_path_size)
		{
			assert(false && "Path buffer too small.");
			return;
		}

		u16_path_[u16_path_size - 1] = L'*';
		u16_path_[u16_path_size] = L'\0';
	}

	// Make API call.
	//
	win32_handle_ = ::FindFirstFileW(u16_path_, &win32_find_data_);

	if (!is_open())
	{
		return;
	}

	// Initialize the entry.
	//
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

	const BOOL bool_result = ::FindClose(win32_handle_);
	assert(bool_result == TRUE);
	ignore_result(bool_result);
	win32_handle_ = INVALID_HANDLE_VALUE;
}

bool SysDirWin32::update_entry()
{
	// Convert file name to UTF-8.
	//
	const int u8_size = ::WideCharToMultiByte(
		CP_UTF8, 0, win32_find_data_.cFileName, -1, u8_name_, MAX_PATH, NULL, NULL);

	if (u8_size == 0)
	{
		return false;
	}

	// Update the entry.
	//
	entry_.is_dir = (win32_find_data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	entry_.name = u8_name_;
	return true;
}

} // namespace

// ==========================================================================

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
	//
	const int u16_path_size = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, u16_path, max_path_size);

	if (u16_path_size == 0)
	{
		assert(false && "MultiByteToWideChar");
		return;
	}

	const BOOL win32_result = ::CreateDirectoryW(u16_path, NULL);

#ifndef NDEBUG
	if (win32_result != TRUE)
	{
		const DWORD last_error = ::GetLastError();
		assert(last_error == ERROR_ALREADY_EXISTS);
	}
#endif // NDEBUG

	ignore_result(win32_result);
}

#endif // _WIN32
