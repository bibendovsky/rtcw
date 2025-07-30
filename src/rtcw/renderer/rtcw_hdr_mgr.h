/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_HDR_MGR_INCLUDED
#define RTCW_HDR_MGR_INCLUDED

namespace rtcw {

class HdrMgr
{
public:
	HdrMgr() {}
	virtual ~HdrMgr() {}

	// Returns "true" if HDR enabled on primary display or "false" otherwise.
	bool is_hdr_enabled();

	// Returns a relative SDR white level to the reference one (80 nits).
	// Formula: sdr_white_factor = sdr_white_level_in_nits / 80.0nits
	float get_sdr_white_level_float();

private:
	virtual bool do_is_hdr_enabled() = 0;
	virtual double do_get_sdr_white_level_double() = 0;
};

// ======================================

HdrMgr* make_hdr_mgr();

} // namespace rtcw

#endif // RTCW_HDR_MGR_INCLUDED
