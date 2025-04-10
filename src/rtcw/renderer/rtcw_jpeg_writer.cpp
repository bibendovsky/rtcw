/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

//
// A wrapper for JPEG writing.
//


#include "rtcw_jpeg_writer.h"

#include <algorithm>
#include <memory>


namespace rtcw
{


namespace
{


class MemoryStream :
	public jpge::output_stream
{
public:
	MemoryStream(
		void* buffer,
		jpge::uint buffer_size)
		:
		buffer_(static_cast<jpge::uint8*>(buffer)),
		buffer_size_(buffer_size),
		buffer_offset_()
	{}

	virtual bool put_buf(
		const void* buffer,
		int size)
	{
		jpge::uint buf_remaining = buffer_size_ - buffer_offset_;

		if (static_cast<jpge::uint>(size) > buf_remaining)
		{
			return false;
		}

		std::copy(
			static_cast<const jpge::uint8*>(buffer),
			&static_cast<const jpge::uint8*>(buffer)[size],
			&buffer_[buffer_offset_]);

		buffer_offset_ += size;

		return true;
	}

	jpge::uint get_size() const
	{
		return buffer_offset_;
	}


private:
	jpge::uint8* buffer_;
	jpge::uint buffer_size_;
	jpge::uint buffer_offset_;

private:
	MemoryStream(const MemoryStream&);
	MemoryStream& operator=(const MemoryStream&);
}; // MemoryStream


} // namespace


JpegWriter::JpegWriter()
	:
	error_message_(),
	line_buffer_()
{}

bool JpegWriter::encode(
	int quality,
	const void* src_data,
	int width,
	int height,
	void* dst_data,
	int& dst_size)
{
	if (quality < 1 || quality > 100)
	{
		error_message_ = "Quality value out of range.";
		return false;
	}

	if (src_data == NULL)
	{
		error_message_ = "Null source data.";
		return false;
	}

	if (width < 0)
	{
		error_message_ = "Negative width.";
		return false;
	}

	if (height < 0)
	{
		error_message_ = "Negative height.";
		return false;
	}

	if (dst_data == NULL)
	{
		error_message_ = "Null destination data.";
		return false;
	}

	const jpge::uint dst_max_size = static_cast<jpge::uint>(estimate_dst_size(width, height));

	if (width == 0 || height == 0)
	{
		return true;
	}

	MemoryStream stream(dst_data, dst_max_size);

	jpge::params jpg_params;
	jpg_params.m_quality = quality;

	jpge::jpeg_encoder encoder;

	bool is_succeed = true;

	if (is_succeed)
	{
		bool init_result = encoder.init(
			&stream,
			width,
			height,
			3,
			jpg_params);

		if (!init_result)
		{
			is_succeed = false;
			error_message_ = "Failed to initialize an encoder.";
		}
	}

	if (is_succeed)
	{
		const int dst_pitch = 3 * width;

		line_buffer_.resize(dst_pitch);

		const unsigned char* src = static_cast<const unsigned char*>(src_data);

		jpge::uint pass_count = encoder.get_total_passes();

		for (jpge::uint p = 0; p < pass_count; ++p)
		{
			for (int h = height - 1; h >= 0; --h)
			{
				const unsigned char* src_scanline = src + (h * (width * 4));
				unsigned char* dst_scanline = &line_buffer_[0];

				rgba_to_rgb(src_scanline, width, dst_scanline);

				bool process_result = encoder.process_scanline(dst_scanline);

				if (!process_result)
				{
					is_succeed = false;
					error_message_ = "Failed to process a scanline.";
					break;
				}
			}

			if (!is_succeed)
			{
				break;
			}

			bool process_result = encoder.process_scanline(NULL);

			if (!process_result)
			{
				is_succeed = false;
				error_message_ = "Failed to finish a pass.";
				break;
			}
		}
	}

	if (is_succeed)
	{
		encoder.deinit();

		dst_size = static_cast<int>(stream.get_size());
	}

	return is_succeed;
}

const String& JpegWriter::get_error_message() const
{
	return error_message_;
}

int JpegWriter::estimate_dst_size(
	int width,
	int height)
{
	if (width <= 0 || height <= 0)
	{
		return 0;
	}

	const int min_size = 2048;

	int size = width * height;

	if (size < min_size)
	{
		size = min_size;
	}

	return size;
}

void JpegWriter::rgba_to_rgb(
	const unsigned char* src_row,
	int src_width,
	unsigned char* dst_row)
{
	for (int i = 0; i < src_width; ++i)
	{
		dst_row[(3 * i) + 0] = src_row[(4 * i) + 0];
		dst_row[(3 * i) + 1] = src_row[(4 * i) + 1];
		dst_row[(3 * i) + 2] = src_row[(4 * i) + 2];
	}
}


} // rtcw
