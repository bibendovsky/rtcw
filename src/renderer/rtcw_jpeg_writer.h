//
// A simple writer for JPEG codec.
//


#ifndef RTCW_JPEG_WRITER_H
#define RTCW_JPEG_WRITER_H


#include <string>
#include <vector>
#include <jpeglib.h>


namespace rtcw {


class JpegWriter {
public:
    JpegWriter();

    ~JpegWriter();

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

    typedef std::vector<JOCTET> Buffer;

    bool is_jcs_valid_;
    std::string error_message_;

    jpeg_error_mgr jem_;
    jpeg_destination_mgr jdm_;
    jpeg_compress_struct jcs_;
    Buffer line_buffer_;

    void clear_error_message();

    void set_error_message(
        const char* message);

    void j_error_exit(
        j_common_ptr jcp);

    void j_output_message(
        j_common_ptr jcp);

    static void j_error_exit_wrapper(
        j_common_ptr jcp);

    static void j_output_message_wrapper(
        j_common_ptr jcp);

    static void j_init_destination(
        j_compress_ptr jcp);

    static boolean j_empty_output_buffer(
        j_compress_ptr jcp);

    static void j_term_destination(
        j_compress_ptr jcp);

    static void rgba_to_rgb(
        const JOCTET* src_row,
        int src_width,
        JOCTET* dst_row);

    JpegWriter(
        const JpegWriter& that);

    JpegWriter& operator=(
        const JpegWriter& that);
}; // class JpegWriter


} // namespace rtcw


#endif // RTCW_JPEG_WRITER_H
