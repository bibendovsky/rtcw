/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

//
// A wrapper for JPEG writing.
//


#ifndef RTCW_JPEG_WRITER_INCLUDE
#define RTCW_JPEG_WRITER_INCLUDE


#include "jpge.h"
#include "rtcw_string.h"
#include "rtcw_vector_trivial.h"


namespace rtcw
{


class JpegWriter
{
public:
	JpegWriter();

	bool encode(
		int quality,
		const void* src_data,
		int width,
		int height,
		void* dst_data,
		int& dst_size);

	const String& get_error_message() const;

	static int estimate_dst_size(
		int width,
		int height);

private:
	class Exception;

	typedef VectorTrivial<unsigned char> Buffer;

	String error_message_;

	Buffer line_buffer_;

	JpegWriter(const JpegWriter&);
	JpegWriter& operator=(const JpegWriter&);

	static void rgba_to_rgb(
		const unsigned char* src_row,
		int src_width,
		unsigned char* dst_row);
}; // JpegWriter


} // rtcw


#endif // !RTCW_JPEG_WRITER_INCLUDE
