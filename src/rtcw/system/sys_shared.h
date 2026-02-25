/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2026 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef SYS_SHARED_INCLUDED
#define SYS_SHARED_INCLUDED

struct SysDirHandle_ {};
typedef SysDirHandle_* SysDirHandle;

struct SysDirEntry
{
	bool is_dir;
	const char* name;
};

SysDirHandle sys_open_dir(const char* path);
const SysDirEntry* sys_read_dir(SysDirHandle handle);
void sys_close_dir(SysDirHandle& handle);

void Sys_Mkdir(const char* path);
char* Sys_Cwd();
int Sys_Milliseconds();

#endif // SYS_SHARED_INCLUDED
