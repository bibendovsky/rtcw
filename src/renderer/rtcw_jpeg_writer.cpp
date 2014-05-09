//
// A simple writer for JPEG codec.
//


#include "rtcw_jpeg_writer.h"
#include "jerror.h"
#include "jpeglib.h"


namespace rtcw {


namespace {


enum AddonMessageCode {
    k_amc_first_code = 1000,
    k_amc_dst_buffer_too_small,
    k_amc_last_code
}; // enum AddonMessageCode


const char* const k_addon_message_table[] = {
    NULL,
    "Destination buffer too small."
};


} // namespace


// ===============================
// Class JpegWriter::Exception
//

class JpegWriter::Exception {
}; // class JpegWriter::Exception


// ================
// Class JpegWriter
//

JpegWriter::JpegWriter() :
    is_jcs_valid_(),
    error_message_(),
    jem_(),
    jdm_(),
    jcs_(),
    line_buffer_()
{
    jcs_.client_data = this;

    jcs_.err = jpeg_std_error(&jem_);
    jcs_.err->first_addon_message = k_amc_first_code;
    jcs_.err->last_addon_message = k_amc_last_code;
    jcs_.err->addon_message_table = k_addon_message_table;
    jcs_.err->error_exit = j_error_exit_wrapper;
    jcs_.err->output_message = j_output_message_wrapper;

    jdm_.init_destination = j_init_destination;
    jdm_.empty_output_buffer = j_empty_output_buffer;
    jdm_.term_destination = j_term_destination;
}

JpegWriter::~JpegWriter()
{
    if (is_jcs_valid_)
        jpeg_destroy_compress(&jcs_);
}

bool JpegWriter::encode(
    int quality,
    const void* src_data,
    int width,
    int height,
    void* dst_data,
    int& dst_size)
{
    if (quality < 0 || quality > 100) {
        set_error_message("Quality value out of range.");
        return false;
    }

    if (src_data == NULL) {
        set_error_message("Null source data.");
        return false;
    }

    if (width < 0) {
        set_error_message("Negative width.");
        return false;
    }

    if (height < 0) {
        set_error_message("Negative height.");
        return false;
    }

    if (dst_data == NULL) {
        set_error_message("Null destination data.");
        return false;
    }

    int dst_max_size = estimate_dst_size(width, height);

    clear_error_message();

    if (width == 0 || height == 0)
        return true;

    bool is_succeed = true;

    try {
        if (!is_jcs_valid_) {
            jpeg_create_compress(&jcs_);
            jcs_.dest = &jdm_;
            is_jcs_valid_ = true;
        }

        jdm_.next_output_byte = static_cast<JOCTET*>(dst_data);
        jdm_.free_in_buffer = dst_max_size;

        jcs_.image_width = width;
        jcs_.image_height = height;
        jcs_.input_components = 3;
        jcs_.in_color_space = JCS_RGB;

        jpeg_set_defaults(&jcs_);
        jpeg_set_quality(&jcs_, quality, TRUE);
        jpeg_start_compress(&jcs_, TRUE);

        const JOCTET* src_ptr =
            static_cast<const JOCTET*>(src_data) + (4 * width * height);

        line_buffer_.resize(3 * width);
        JSAMPROW scanline[1] = { &line_buffer_[0] };
        const int src_stride = 4 * width;

        for (int i = 0; i < height; ++i) {
            src_ptr -= src_stride;
            rgba_to_rgb(src_ptr, width, &line_buffer_[0]);
            static_cast<void>(jpeg_write_scanlines(&jcs_, scanline, 1));
        }

        jpeg_finish_compress(&jcs_);

        dst_size =
            dst_max_size - static_cast<int>(jcs_.dest->free_in_buffer);
    } catch(const Exception&) {
        is_succeed = false;
    }

    return is_succeed;
}

const std::string& JpegWriter::get_error_message() const
{
    return error_message_;
}

// (static)
int JpegWriter::estimate_dst_size(
    int width,
    int height)
{
    if (width <= 0 || height <= 0)
        return 0;

    const int k_min_size = 2048;

    int size = width * height;

    if (size < k_min_size)
        size = k_min_size;

    return size;
}

void JpegWriter::clear_error_message()
{
    error_message_.clear();
}

void JpegWriter::set_error_message(
    const char* message)
{
    error_message_.assign(message);
}

void JpegWriter::j_error_exit(
    j_common_ptr jcp)
{
    jcp->err->output_message(jcp);
    is_jcs_valid_ = false;
    jpeg_destroy(jcp);
    throw Exception();
}

void JpegWriter::j_output_message(
    j_common_ptr jcp)
{
    char buffer[JMSG_LENGTH_MAX];
    jcp->err->format_message(jcp, buffer);
    set_error_message(buffer);
}

// (static)
void JpegWriter::j_error_exit_wrapper(
    j_common_ptr jcp)
{
    static_cast<JpegWriter*>(jcp->client_data)->j_error_exit(jcp);
}

// (static)
void JpegWriter::j_output_message_wrapper(
    j_common_ptr jcp)
{
    static_cast<JpegWriter*>(jcp->client_data)->j_output_message(jcp);
}

// (static)
void JpegWriter::j_init_destination(
    j_compress_ptr jcp)
{
}

// (static)
boolean JpegWriter::j_empty_output_buffer(
    j_compress_ptr jcp)
{
    ERREXIT(jcp, k_amc_dst_buffer_too_small);
    throw Exception();
}

// (static)
void JpegWriter::j_term_destination(
    j_compress_ptr jcp)
{
}

// (static)
void JpegWriter::rgba_to_rgb(
    const JOCTET* src_row,
    int src_width,
    JOCTET* dst_row)
{
    for (int i = 0; i < src_width; ++i) {
        dst_row[(3 * i) + 0] = src_row[(4 * i) + 0];
        dst_row[(3 * i) + 1] = src_row[(4 * i) + 1];
        dst_row[(3 * i) + 2] = src_row[(4 * i) + 2];
    }
}


} // namespace rtcw
