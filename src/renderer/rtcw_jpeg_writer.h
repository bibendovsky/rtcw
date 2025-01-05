//
// A wrapper for JPEG writing.
//


#ifndef RTCW_JPEG_WRITER_INCLUDE
#define RTCW_JPEG_WRITER_INCLUDE


#include <string>
#include <vector>

#include "jpge.h"


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

	const std::string& get_error_message() const;

	static int estimate_dst_size(
		int width,
		int height);

private:
	class Exception;

	using Buffer = std::vector<unsigned char>;

	std::string error_message_;

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
