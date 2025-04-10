/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_SYSCON_FONT_16X8_INCLUDED
#define RTCW_SYSCON_FONT_16X8_INCLUDED

namespace rtcw {

class SysconFont16x8
{
public:
	static const int glyph_width = 8;
	static const int glyph_height = 16;
	static const int glyph_count = 190;
	static const int bytes_per_glyph = (glyph_width / 8) * glyph_height;
	static const int total_bitmap_size = glyph_count * bytes_per_glyph;

public:
	// U+0020-U+007E, U+00A0-U+00AC, U+00AE-U+00FF
	static const unsigned char bitmap[total_bitmap_size];

	static bool has_code_point(int code_point);
};

} // namespace rtcw

#endif // RTCW_SYSCON_FONT_16X8_INCLUDED
