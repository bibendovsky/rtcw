/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_hdr_mgr.h"
#include "rtcw_memory.h"

namespace rtcw {

float HdrMgr::get_sdr_white_level_float()
{
	return static_cast<float>(get_sdr_white_level_double());
}

void HdrMgrDeleter::operator()(HdrMgr* hdr_mgr) const
{
	mem::destroy_object(hdr_mgr);
}

} // namespace rtcw
