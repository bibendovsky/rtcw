//
// A wrapper for JPEG writing.
//


#ifndef RTCW_JPEG_WRITER_INCLUDE
#define RTCW_JPEG_WRITER_INCLUDE


#include <string>
#include <vector>
#include "jpge.h"


namespace rtcw {


class JpegWriter {
public:
    JpegWriter();

    ~JpegWriter();

    JpegWriter(
        const JpegWriter& that) = delete;

    JpegWriter& operator=(
        const JpegWriter& that) = delete;


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

    typedef std::vector<unsigned char> Buffer;

    std::string error_message_;

    Buffer line_buffer_;

    static void rgba_to_rgb(
        const unsigned char* src_row,
        int src_width,
        unsigned char* dst_row);
}; // JpegWriter


} // rtcw


#endif // RTCW_JPEG_WRITER_INCLUDE
