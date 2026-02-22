/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_CURL_LOADER_INCLUDED
#define RTCW_CURL_LOADER_INCLUDED

namespace rtcw {

class CurlLoader
{
public:
	virtual bool load_library() = 0;
	virtual void unload_library() = 0;
	virtual void* get_proc_address(const char* symbol_name) = 0;
};

// =====================================

extern CurlLoader* global_curl_loader;

} // namespace rtcw

#endif // RTCW_CURL_LOADER_INCLUDED
