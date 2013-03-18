#include "bbi_ogl_version.h"

#include <sstream>


namespace bbi {


OglVersion::OglVersion () :
    major_number_ (0),
    minor_number_ (0),
    release_number_ (),
    vendor_number_ (),
    version_string_ ()
{
}

OglVersion::OglVersion (const char* version_string)
{
    assign (version_string);
}

OglVersion::OglVersion (const OglVersion& that) :
    major_number_ (that.major_number_),
    minor_number_ (that.minor_number_),
    release_number_ (that.release_number_),
    vendor_number_ (that.vendor_number_),
    version_string_ (that.version_string_)
{
}

OglVersion::~OglVersion ()
{
}

OglVersion& OglVersion::operator = (const OglVersion& that)
{
    if (&that != this) {
        major_number_ = that.major_number_;
        minor_number_ = that.minor_number_;
        release_number_ = that.release_number_;
        vendor_number_ = that.vendor_number_;
        version_string_ = that.version_string_;
    }

    return *this;
}

void OglVersion::assign (const char* version_string)
{
    major_number_ = 0;
    minor_number_ = 0;
    release_number_ = 0;
    vendor_number_.clear ();
    version_string_.clear ();

    if (version_string == nullptr)
        return;

    if (version_string[0] == '\0')
        return;

    std::istringstream iss;

    std::string version = version_string;

    auto dot1_offset = version.find ('.', 0);
    auto has_dot1 = (dot1_offset > 0) && (dot1_offset < version.size ());

    if (!has_dot1)
        return;

    int major = 0;
    int minor = 0;
    int release = 0;
    std::string vendor;

    // major
    iss.str (version.substr (0, dot1_offset));
    iss.clear ();
    iss >> major;

    if (iss.fail ())
        return;

    auto dot2_offset = version.find ('.', dot1_offset + 1);
    auto has_dot2 = (dot2_offset != std::string::npos);

    auto space_offset = version.find (' ', dot1_offset + 1);
    auto has_space = (space_offset != std::string::npos);

    // minor
    if (has_dot2) {
        iss.str (version.substr (dot1_offset + 1,
            dot2_offset - dot1_offset - 1));
    } else {
        if (has_space) {
            iss.str (version.substr (dot1_offset + 1,
                space_offset - dot1_offset - 1));
        } else
            iss.str (version.substr (dot1_offset + 1));
    }

    iss.clear ();
    iss >> minor;

    if (iss.fail ())
        return;

    // release
    if (has_dot2) {
        if (has_space) {
            iss.str (version.substr (dot2_offset + 1,
                space_offset - dot2_offset - 1));
        } else
            iss.str (version.substr (dot2_offset + 1));

        iss.clear ();
        iss >> release;

        if (iss.fail ())
            return;
    }

    // vendor
    if (has_space)
        vendor = version.substr (space_offset + 1);

    major_number_ = major;
    minor_number_ = minor;
    release_number_ = release;
    vendor_number_ = vendor;
    version_string_ = version_string;
}

int OglVersion::get_major () const
{
    return major_number_;
}

int OglVersion::get_minor () const
{
    return minor_number_;
}

int OglVersion::get_release () const
{
    return release_number_;
}

const std::string& OglVersion::get_vendor () const
{
    return vendor_number_;
}

const std::string& OglVersion::get_version () const
{
    return version_string_;
}

bool OglVersion::is_minimum_version (int major, int minor, int release)
{
    if (major > major_number_)
        return false;

    if (major == major_number_) {
        if (minor > minor_number_)
            return false;

        if (release > release_number_)
            return false;
    }

    return true;
}


} // namespace bbi
