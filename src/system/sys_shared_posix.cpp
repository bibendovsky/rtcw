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

#ifndef _WIN32

#include "sys_shared.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <dirent.h>
#include <sys/stat.h>

// ==========================================================================

namespace {

class SysDirPosix
{
public:
	static SysDirPosix* create(const char* path);
	static void destroy(SysDirPosix* sys_dir);

	const SysDirEntry* read();

private:
	static const size_t max_path_size = 1024;

private:
	size_t name_index_;
	char path_[max_path_size];
	DIR* posix_dir_;
	struct dirent* posix_dirent_;
	SysDirEntry entry_;

private:
	SysDirPosix(const SysDirPosix&);
	SysDirPosix& operator=(const SysDirPosix&);

private:
	SysDirPosix(const char* path);
	~SysDirPosix();

	template<typename T>
	static void ignore_result(const T&) {}

	bool is_open() const;
	void close();
	bool update_entry();
};

// --------------------------------------------------------------------------

SysDirPosix* SysDirPosix::create(const char* path)
{
	SysDirPosix* sys_dir = static_cast<SysDirPosix*>(::malloc(sizeof(SysDirPosix)));

	if (sys_dir == NULL)
	{
		return NULL;
	}

	::new (static_cast<void*>(sys_dir)) SysDirPosix(path);

	if (!sys_dir->is_open())
	{
		SysDirPosix::destroy(sys_dir);
		return NULL;
	}

	return sys_dir;
}

void SysDirPosix::destroy(SysDirPosix* sys_dir)
{
	sys_dir->~SysDirPosix();
	::free(sys_dir);
}

const SysDirEntry* SysDirPosix::read()
{
	assert(is_open());
	posix_dirent_ = ::readdir(posix_dir_);

	if (posix_dirent_ == NULL)
	{
		return NULL;
	}

	if (!update_entry())
	{
		return NULL;
	}

	return &entry_;
}

SysDirPosix::SysDirPosix(const char* path)
	:
	posix_dir_()
{
	if (path[0] == '\0' || (path[0] == '.' && path[1] == '\0'))
	{
		name_index_ = 0;
		path_[0] = '.';
		path_[1] = '\0';
	}
	else
	{
		// Copy whole path.
		//
		size_t path_size = strlen(path) + 1;

		if (path_size >= max_path_size)
		{
			assert(false && "Path buffer too small.");
			return;
		}

		memcpy(path_, path, path_size);

		// Append slash.
		//
		if (path_[path_size - 2] != '/')
		{
			if (path_size == max_path_size)
			{
				assert(false && "Path buffer too small.");
				return;
			}

			path_[path_size - 1] = '/';
			path_[path_size] = '\0';
			++path_size;
		}

		// Append the dot.
		//
		if (path_size == max_path_size)
		{
			assert(false && "Path buffer too small.");
			return;
		}

		path_[path_size - 1] = '.';
		path_[path_size] = L'\0';
		name_index_ = path_size - 1;
	}

	// Make API call.
	//
	posix_dir_ = ::opendir(path_);
}

SysDirPosix::~SysDirPosix()
{
	close();
}

bool SysDirPosix::is_open() const
{
	return posix_dir_ != NULL;
}

void SysDirPosix::close()
{
	if (!is_open())
	{
		return;
	}

	const int int_result = ::closedir(posix_dir_);
	assert(int_result == 0);
	ignore_result(int_result);
	posix_dir_ = NULL;
}

bool SysDirPosix::update_entry()
{
	// Append the name to the path.
	//
	const size_t name_size = strlen(posix_dirent_->d_name) + 1;

	if (name_index_ + name_size > max_path_size)
	{
		assert(false && "Path buffer too small.");
		return false;
	}

	memcpy(&path_[name_index_], posix_dirent_->d_name, name_size);

	// Get file info.
	//
	struct stat posix_stat;
	const int int_result = ::stat(path_, &posix_stat);

	if (int_result != 0)
	{
		return false;
	}

	
	// Update the entry.
	//
	entry_.is_dir = S_ISDIR(posix_stat.st_mode) != 0;
	entry_.name = &path_[name_index_];
	return true;
}

} // namespace

// ==========================================================================

SysDirHandle sys_open_dir(const char* path)
{
	return reinterpret_cast<SysDirHandle>(SysDirPosix::create(path));
}

const SysDirEntry* sys_read_dir(SysDirHandle handle)
{
	return reinterpret_cast<SysDirPosix*>(handle)->read();
}

void sys_close_dir(SysDirHandle& handle)
{
	SysDirPosix::destroy(reinterpret_cast<SysDirPosix*>(handle));
}

#endif // _WIN32
