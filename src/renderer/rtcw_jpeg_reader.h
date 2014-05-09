//
// A simple reader for JPEG codec.
//


#ifndef RTCW_JPEG_READER_H
#define RTCW_JPEG_READER_H


#include <string>
#include <vector>
#include <jpeglib.h>


namespace rtcw {


class JpegReader {
public:
    JpegReader();

    ~JpegReader();

    bool open(
        const void* src_data,
        int src_size,
        int& width,
        int& height);

    bool decode(
        void* dst_data);

    void close();

    bool is_open() const;

    bool is_decoded() const;

    int get_width() const;

    int get_height() const;

    const std::string& get_error_message() const;

private:
    class Exception;

    typedef std::vector<JOCTET> Buffer;

    bool is_open_;
    bool is_decoded_;
    bool is_jds_valid_;
    std::string error_message_;

    jpeg_error_mgr jem_;
    jpeg_source_mgr jsm_;
    jpeg_decompress_struct jds_;
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

    static void j_init_source(
        j_decompress_ptr jdp);

    static boolean j_fill_input_buffer(
        j_decompress_ptr jdp);

    static void j_skip_input_data(
        j_decompress_ptr jdp,
        long num_bytes);

    static void j_term_source(
        j_decompress_ptr jdp);

    static void gray_to_rgba(
        const JOCTET* src_row,
        int src_width,
        JOCTET* dst_row);

    static void rgb_to_rgba(
        const JOCTET* src_row,
        int src_width,
        JOCTET* dst_row);

    JpegReader(
        const JpegReader& that);

    JpegReader& operator=(
        const JpegReader& that);
}; // class JpegReader


} // namespace rtcw


#endif // RTCW_JPEG_READER_H
