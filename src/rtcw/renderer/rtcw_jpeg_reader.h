/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

//
// A wrapper for JPEG reading.
//


#ifndef RTCW_JPEG_READER_INCLUDED
#define RTCW_JPEG_READER_INCLUDED


#include "jpgd.h"
#include "rtcw_string.h"
#include "rtcw_unique_ptr.h"


namespace rtcw
{


class JpegReader
{
public:
	JpegReader();

	bool open(
		const void* src_data,
		int src_size,
		int& width,
		int& height);

	bool decode(
		void* dst_data);

	void close();

	int get_width() const;

	int get_height() const;

	const String& get_error_message() const;


private:
	int width_;
	int height_;
	bool is_grayscale_;
	rtcw::UniquePtr<jpgd::jpeg_decoder_mem_stream> stream_;
	rtcw::UniquePtr<jpgd::jpeg_decoder> decoder_;
	String error_message_;

	JpegReader(const JpegReader&);
	JpegReader& operator=(const JpegReader&);

	static void gray_to_rgba(
		const unsigned char* src_row,
		int src_width,
		unsigned char* dst_row);
}; // JpegReader


} // rtcw


#endif // !RTCW_JPEG_READER_INCLUDED
