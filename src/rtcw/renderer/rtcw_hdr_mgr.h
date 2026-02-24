/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_HDR_MGR_INCLUDED
#define RTCW_HDR_MGR_INCLUDED

namespace rtcw {

class HdrMgr
{
public:
	// Destroys the object.
	virtual void destroy() = 0;

	// Returns "true" if HDR enabled on primary display or "false" otherwise.
	virtual bool is_hdr_enabled() = 0;

	// Returns a relative SDR white level to the reference one (80 nits) as double.
	// Formula: sdr_white_factor = sdr_white_level_in_nits / 80.0nits
	virtual double get_sdr_white_level_double() = 0;

	// Returns a relative SDR white level to the reference one (80 nits) as float.
	// Formula: sdr_white_factor = sdr_white_level_in_nits / 80.0nits
	float get_sdr_white_level_float();
};

// =====================================

struct HdrMgrDeleter
{
	void operator()(HdrMgr* hdr_mgr) const;
};

HdrMgr* make_hdr_mgr();

} // namespace rtcw

#endif // RTCW_HDR_MGR_INCLUDED
