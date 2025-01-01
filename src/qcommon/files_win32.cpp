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
