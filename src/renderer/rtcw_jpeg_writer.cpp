//
// A wrapper for JPEG writing.
//


#include "rtcw_jpeg_writer.h"

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
		buffer_{static_cast<jpge::uint8*>(buffer)},
		buffer_size_{buffer_size}
	{
	}

	MemoryStream(
		const MemoryStream& that) = delete;

	MemoryStream& operator=(
		const MemoryStream& that) = delete;


	bool put_buf(
		const void* buffer,
		int size) override
	{
		auto buf_remaining = buffer_size_ - buffer_offset_;

		if (static_cast<jpge::uint>(size) > buf_remaining)
		{
			return false;
		}

		std::uninitialized_copy_n(static_cast<const jpge::uint8*>(buffer), size, buffer_ + buffer_offset_);

		buffer_offset_ += size;

		return true;
	}

	jpge::uint get_size() const noexcept
	{
		return buffer_offset_;
	}


private:
	jpge::uint8* buffer_{};
	jpge::uint buffer_size_{};
	jpge::uint buffer_offset_{};
}; // MemoryStream


} // namespace


JpegWriter::JpegWriter() = default;

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

	if (src_data == nullptr)
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

	if (dst_data == nullptr)
	{
		error_message_ = "Null destination data.";
		return false;
	}

	const auto dst_max_size = static_cast<jpge::uint>(estimate_dst_size(width, height));

	if (width == 0 || height == 0)
	{
		return true;
	}

	MemoryStream stream{dst_data, dst_max_size};

	auto jpg_params = jpge::params{};
	jpg_params.m_quality = quality;

	jpge::jpeg_encoder encoder{};

	auto is_succeed = true;

	if (is_succeed)
	{
		auto init_result = encoder.init(
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
		const auto dst_pitch = 3 * width;

		line_buffer_.resize(dst_pitch);

		auto src = static_cast<const unsigned char*>(src_data);

		auto pass_count = encoder.get_total_passes();

		for (auto p = jpge::uint{}; p < pass_count; ++p)
		{
			for (int h = height - 1; h >= 0; --h)
			{
				auto src_scanline = src + (h * (width * 4));
				auto dst_scanline = line_buffer_.data();

				rgba_to_rgb(src_scanline, width, dst_scanline);

				auto process_result = encoder.process_scanline(dst_scanline);

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

			auto process_result = encoder.process_scanline(nullptr);

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

const std::string& JpegWriter::get_error_message() const noexcept
{
	return error_message_;
}

int JpegWriter::estimate_dst_size(
	int width,
	int height) noexcept
{
	if (width <= 0 || height <= 0)
	{
		return 0;
	}

	constexpr auto min_size = 2048;

	auto size = width * height;

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
