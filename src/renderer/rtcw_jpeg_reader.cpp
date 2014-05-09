//
// A simple reader for JPEG codec.
//


#include "rtcw_jpeg_reader.h"
#include "jerror.h"
#include "jpeglib.h"


namespace rtcw {


namespace {


enum AddonMessageCode {
    k_amc_first_code = 1000,
    k_amc_unexpected_end_of_source_data,
    k_amc_last_code
}; // enum AddonMessageCode


const char* const k_addon_message_table[] = {
    NULL,
    "Unexpected end of source data."
};


} // namespace


// ===============================
// Class JpegReader::Exception
//

class JpegReader::Exception {
}; // class JpegReader::Exception


// ================
// Class JpegReader
//

JpegReader::JpegReader() :
    is_open_(),
    is_decoded_(),
    is_jds_valid_(),
    error_message_(),
    jem_(),
    jsm_(),
    jds_(),
    line_buffer_()
{
    jds_.client_data = this;

    jds_.err = jpeg_std_error(&jem_);
    jds_.err->first_addon_message = k_amc_first_code;
    jds_.err->last_addon_message = k_amc_last_code;
    jds_.err->addon_message_table = k_addon_message_table;
    jds_.err->error_exit = j_error_exit_wrapper;
    jds_.err->output_message = j_output_message_wrapper;

    jsm_.init_source = j_init_source;
    jsm_.fill_input_buffer = j_fill_input_buffer;
    jsm_.skip_input_data = j_skip_input_data;
    jsm_.resync_to_restart = jpeg_resync_to_restart;
    jsm_.term_source = j_term_source;
}

JpegReader::~JpegReader()
{
    close();

    if (is_jds_valid_)
        jpeg_destroy_decompress(&jds_);
}

bool JpegReader::open(
    const void* src_data,
    int src_size,
    int& width,
    int& height)
{
    close();

    if (src_data == NULL) {
        set_error_message("Null source data.");
        return false;
    }

    if (src_size <= 0) {
        set_error_message("Zero or negative source size.");
        return false;
    }


    bool is_succeed = true;

    if (is_succeed) {
        try {
            if (!is_jds_valid_) {
                jpeg_create_decompress(&jds_);
                is_jds_valid_ = true;
                jds_.src = &jsm_;
            }

            jsm_.next_input_byte = static_cast<const JOCTET*>(src_data);
            jsm_.bytes_in_buffer = src_size;

            static_cast<void>(jpeg_read_header(&jds_, TRUE));
        } catch (const Exception&) {
            is_succeed = false;
        }
    }

    if (is_succeed) {
        switch (jds_.out_color_space) {
        case JCS_GRAYSCALE:
        case JCS_RGB:
            break;

        default:
            is_succeed = false;
            set_error_message("Unsupported output color space.");
        }
    }

    if (is_succeed) {
        is_open_ = true;
        width = get_width();
        height = get_height();
    }

    return is_open();
}

bool JpegReader::decode(
    void* dst_data)
{
    if (!is_open()) {
        set_error_message("Reader not open.");
        return false;
    }

    if (is_decoded()) {
        set_error_message("Already decoded.");
        return false;
    }

    clear_error_message();

    if (get_width() == 0 || get_height() == 0) {
        is_decoded_ = true;
        return true;
    }

    bool is_succeed = true;

    try {
        jpeg_start_decompress(&jds_);

        line_buffer_.resize(get_width() * jds_.out_color_components);

        JOCTET* dst_data_ptr = static_cast<JOCTET*>(dst_data);

        JSAMPROW buffer[1] = { &line_buffer_[0] };
        const int dst_stride = 4 * get_width();

        for (int i = 0; i < get_height(); ++i) {
            static_cast<void>(jpeg_read_scanlines(&jds_, buffer, 1));

            switch (jds_.out_color_components) {
            case 1:
                gray_to_rgba(&line_buffer_[0], get_width(), dst_data_ptr);
                break;

            case 3:
                rgb_to_rgba(&line_buffer_[0], get_width(), dst_data_ptr);
                break;
            }

            dst_data_ptr += dst_stride;
        }

        jpeg_finish_decompress(&jds_);
    } catch(const Exception&) {
        is_succeed = false;
    }

    is_decoded_ = true;

    return is_succeed;
}

void JpegReader::close()
{
    if (!is_open())
        return;

    is_open_ = false;
    is_decoded_ = false;
}

bool JpegReader::is_open() const
{
    return is_open_;
}

bool JpegReader::is_decoded() const
{
    return is_decoded_;
}

int JpegReader::get_width() const
{
    return static_cast<int>(jds_.image_width);
}

int JpegReader::get_height() const
{
    return static_cast<int>(jds_.image_height);
}

const std::string& JpegReader::get_error_message() const
{
    return error_message_;
}

void JpegReader::clear_error_message()
{
    error_message_.clear();
}

void JpegReader::set_error_message(
    const char* message)
{
    error_message_.assign(message);
}

void JpegReader::j_error_exit(
    j_common_ptr jcp)
{
    jcp->err->output_message(jcp);
    is_jds_valid_ = false;
    jpeg_destroy(jcp);
    throw Exception();
}

void JpegReader::j_output_message(
    j_common_ptr jcp)
{
    char buffer[JMSG_LENGTH_MAX];
    jcp->err->format_message(jcp, buffer);
    set_error_message(buffer);
}

// (static)
void JpegReader::j_error_exit_wrapper(
    j_common_ptr jcp)
{
    static_cast<JpegReader*>(jcp->client_data)->j_error_exit(jcp);
}

// (static)
void JpegReader::j_output_message_wrapper(
    j_common_ptr jcp)
{
    static_cast<JpegReader*>(jcp->client_data)->j_output_message(jcp);
}

// (static)
void JpegReader::j_init_source(
    j_decompress_ptr jdp)
{
}

// (static)
boolean JpegReader::j_fill_input_buffer(
    j_decompress_ptr jdp)
{
    ERREXIT(jdp, k_amc_unexpected_end_of_source_data);
    throw Exception();
}

// (static)
void JpegReader::j_skip_input_data(
    j_decompress_ptr jdp,
    long num_bytes)
{
    jdp->src->next_input_byte += num_bytes;
    jdp->src->bytes_in_buffer -= num_bytes;
}

// (static)
void JpegReader::j_term_source(
    j_decompress_ptr jdp)
{
}

// (static)
void JpegReader::gray_to_rgba(
    const JOCTET* src_row,
    int src_width,
    JOCTET* dst_row)
{
    for (int i = 0; i < src_width; ++i) {
        dst_row[(4 * i) + 0] = src_row[i];
        dst_row[(4 * i) + 1] = src_row[i];
        dst_row[(4 * i) + 2] = src_row[i];
        dst_row[(4 * i) + 3] = static_cast<JOCTET>(0xFF);
    }
}

// (static)
void JpegReader::rgb_to_rgba(
    const JOCTET* src_row,
    int src_width,
    JOCTET* dst_row)
{
    for (int i = 0; i < src_width; ++i) {
        dst_row[(4 * i) + 0] = src_row[(3 * i) + 0];
        dst_row[(4 * i) + 1] = src_row[(3 * i) + 1];
        dst_row[(4 * i) + 2] = src_row[(3 * i) + 2];
        dst_row[(4 * i) + 3] = static_cast<JOCTET>(0xFF);
    }
}


} // namespace rtcw
