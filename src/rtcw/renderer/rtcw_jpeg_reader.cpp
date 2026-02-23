/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2014-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// A wrapper for JPEG reading

#include "rtcw_jpeg_reader.h"
#include "rtcw_memory.h"
#include <algorithm>

namespace rtcw {

void JpegReader::MemStreamDeleter::operator()(jpgd::jpeg_decoder_mem_stream* mem_stream) const
{
	mem::delete_object(mem_stream);
}

void JpegReader::DecoderDeleter::operator()(jpgd::jpeg_decoder* decoder) const
{
	mem::delete_object(decoder);
}

JpegReader::JpegReader()
	:
	width_(),
	height_(),
	is_grayscale_(),
	stream_(),
	decoder_(),
	error_message_()
{}

bool JpegReader::open(const void* src_data, int src_size, int& width, int& height)
{
	close();
	if (src_data == NULL)
	{
		error_message_ = "Null source data.";
		return false;
	}
	if (src_size <= 0)
	{
		error_message_ = "Zero or negative source size.";
		return false;
	}
	bool is_succeed = true;
	if (is_succeed)
	{
		stream_.reset(mem::new_object_2<jpgd::jpeg_decoder_mem_stream>(static_cast<const jpgd::uint8*>(src_data), src_size));
		//
		decoder_.reset(mem::new_object_1<jpgd::jpeg_decoder>(stream_.get()));
		if (decoder_->get_error_code() != jpgd::JPGD_SUCCESS)
		{
			error_message_ = "Failed to open an image.";
			is_succeed = false;
		}
	}
	if (is_succeed)
	{
		switch (decoder_->get_num_components())
		{
			case 1:
				is_grayscale_ = true;
				break;
			case 3:
				is_grayscale_ = false;
				break;
			default:
				is_succeed = false;
				error_message_ = "Unsupported color space.";
				break;
		}
	}
	if (is_succeed)
	{
		width_ = decoder_->get_width();
		height_ = decoder_->get_height();
		width = width_;
		height = height_;
	}
	if (!is_succeed)
	{
		close();
	}
	return is_succeed;
}

bool JpegReader::decode(void* dst_data)
{
	if (dst_data == NULL)
	{
		error_message_ = "Null target buffer.";
		return false;
	}
	if (decoder_.get() == NULL)
	{
		error_message_ = "Decoder not initialized.";
		return false;
	}
	bool is_succeed = true;
	if (is_succeed)
	{
		if (decoder_->begin_decoding() != jpgd::JPGD_SUCCESS)
		{
			is_succeed = false;
			error_message_ = "Failed to start decoding.";
		}
	}
	if (is_succeed)
	{
		const int dst_pitch = 4 * width_;
		unsigned char* dst_buffer = static_cast<unsigned char*>(dst_data);
		for (int y = 0; y < height_; ++y)
		{
			const void* raw_scanline = NULL;
			jpgd::uint scanline_length = 0;
			int decode_result = decoder_->decode(&raw_scanline, &scanline_length);
			if (decode_result == jpgd::JPGD_SUCCESS ||
				decode_result == jpgd::JPGD_DONE)
			{
				const unsigned char* const scanline = static_cast<const unsigned char*>(raw_scanline);
				if (is_grayscale_)
				{
					gray_to_rgba(scanline, width_, dst_buffer);
				}
				else
				{
					std::copy(scanline, &scanline[dst_pitch], dst_buffer);
				}
				dst_buffer += dst_pitch;
			}
			else
			{
				is_succeed = false;
				error_message_ = "Failed to decode a scanline.";
				break;
			}
		}
	}
	return is_succeed;
}

void JpegReader::close()
{
	width_ = 0;
	height_ = 0;
	is_grayscale_ = false;
	decoder_.reset();
	stream_.reset();
	error_message_.clear();
}

int JpegReader::get_width() const
{
	return width_;
}

int JpegReader::get_height() const
{
	return height_;
}

const String& JpegReader::get_error_message() const
{
	return error_message_;
}

void JpegReader::gray_to_rgba(const unsigned char* src_row, int src_width, unsigned char* dst_row)
{
	for (int i = 0; i < src_width; ++i)
	{
		dst_row[(4 * i) + 0] = src_row[i];
		dst_row[(4 * i) + 1] = src_row[i];
		dst_row[(4 * i) + 2] = src_row[i];
		dst_row[(4 * i) + 3] = 0xFF;
	}
}

} // namespace rtcw
