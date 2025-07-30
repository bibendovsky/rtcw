/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef _WIN32

#include "rtcw_hdr_mgr.h"

// ======================================

namespace rtcw {

namespace {

class HdrMgrNull : public HdrMgr
{
public:
	HdrMgrNull() {}
	virtual ~HdrMgrNull() {}

private:
	virtual bool do_is_hdr_enabled();
	virtual double do_get_sdr_white_level_double();
};

// --------------------------------------

bool HdrMgrNull::do_is_hdr_enabled()
{
	return false;
}

double HdrMgrNull::do_get_sdr_white_level_double()
{
	return 1.0;
}

} // namespace

// ======================================

HdrMgr* make_hdr_mgr()
{
	return new HdrMgrNull();
}

} // namespace rtcw

#endif // _WIN32
