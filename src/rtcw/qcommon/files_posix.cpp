/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef _WIN32

#include <stdio.h>
#include <sys/stat.h>

// ==========================================================================

bool FS_Remove(const char* path)
{
	const int posix_result = ::remove(path);
	return posix_result == 0;
}

// ==========================================================================

bool sys_fs_rename(const char* old_path, const char* new_path)
{
	const int posix_result = ::rename(old_path, new_path);
	return posix_result == 0;
}

// ==========================================================================

int FS_OSStatFile(const char* path)
{
	struct stat posix_stat;

	if (::stat(path, &posix_stat) != 0)
	{
		return -1;
	}

	return S_ISDIR(posix_stat.st_mode) != 0;
}

#endif // _WIN32
