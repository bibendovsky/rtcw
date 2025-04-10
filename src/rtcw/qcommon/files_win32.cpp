/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifdef _WIN32

#include <assert.h>
#include <windows.h>

// ==========================================================================

bool FS_Remove(const char* path)
{
	const int max_path_size = 1024;
	wchar_t u16_path[max_path_size];

	int u16_path_size = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, u16_path, max_path_size);

	if (u16_path_size == 0)
	{
		assert(false && "MultiByteToWideChar");
		return false;
	}

	const BOOL win32_result = ::DeleteFileW(u16_path);
	return win32_result == TRUE;
}

// ==========================================================================

bool sys_fs_rename(const char* old_path, const char* new_path)
{
	const int max_path_size = 1024;
	wchar_t u16_old_path[max_path_size];
	wchar_t u16_new_path[max_path_size];

	const int u16_old_path_size = ::MultiByteToWideChar(CP_UTF8, 0, old_path, -1, u16_old_path, max_path_size);

	if (u16_old_path_size == 0)
	{
		assert(false && "MultiByteToWideChar");
		return false;
	}

	const int u16_new_path_size = ::MultiByteToWideChar(CP_UTF8, 0, new_path, -1, u16_new_path, max_path_size);

	if (u16_new_path_size == 0)
	{
		assert(false && "MultiByteToWideChar");
		return false;
	}

	const BOOL win32_result = ::MoveFileW(u16_old_path, u16_new_path);
	return win32_result == TRUE;
}

// ==========================================================================

int FS_OSStatFile(const char* path)
{
	const int max_path_size = 1024;
	wchar_t u16_path[max_path_size];

	int u16_path_size = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, u16_path, max_path_size);

	if (u16_path_size == 0)
	{
		assert(false && "MultiByteToWideChar");
		return false;
	}

	const DWORD win32_result = ::GetFileAttributesW(u16_path);

	if (win32_result == INVALID_FILE_ATTRIBUTES)
	{
		return -1;
	}

	return (win32_result & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

#endif // _WIN32
