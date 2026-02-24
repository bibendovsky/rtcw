/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef _WIN32

#include "rtcw_hdr_mgr.h"
#include "rtcw_memory.h"

// ======================================

namespace rtcw {

namespace {

class HdrMgrNull : public HdrMgr
{
public:
	virtual void destroy();
	virtual bool is_hdr_enabled();
	virtual double get_sdr_white_level_double();
};

// --------------------------------------

void HdrMgrNull::destroy()
{
	mem::delete_object_unchecked(this);
}

bool HdrMgrNull::is_hdr_enabled()
{
	return false;
}

double HdrMgrNull::get_sdr_white_level_double()
{
	return 1.0;
}

} // namespace

// ======================================

HdrMgr* make_hdr_mgr()
{
	return mem::new_object<HdrMgrNull>();
}

} // namespace rtcw

#endif // _WIN32
