/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef _DL_PUBLIC_H_
#define _DL_PUBLIC_H_

typedef enum {
	DL_CONTINUE,
	DL_DONE,
	DL_FAILED
} dlStatus_t;

int DL_BeginDownload( const char *localName, const char *remoteName, int debug );
dlStatus_t DL_DownloadLoop();

void DL_Shutdown();

// bitmask
typedef enum {
	DL_FLAG_DISCON = 0,
	DL_FLAG_URL
} dlFlags_t;

int FS_CreatePath( const char *OSPath );

#endif
