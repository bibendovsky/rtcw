/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_hdr_mgr.h"

// ======================================

namespace rtcw {

bool HdrMgr::is_hdr_enabled()
{
	return do_is_hdr_enabled();
}

float HdrMgr::get_sdr_white_level_float()
{
	return static_cast<float>(do_get_sdr_white_level_double());
}

} // namespace rtcw
