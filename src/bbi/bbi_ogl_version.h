#ifndef BBI_OGL_VERSION_H
#define BBI_OGL_VERSION_H


#include <string>


namespace bbi {


class OglVersion {
public:
    OglVersion ();
    OglVersion (const char* version_string);
    OglVersion (const OglVersion& that);
    ~OglVersion ();
    OglVersion& operator = (const OglVersion& that);

    void assign (const char* version_string);

    int get_major () const;
    int get_minor () const;
    int get_release () const;
    const std::string& get_vendor () const;
    const std::string& get_version () const;

    bool is_minimum_version (int major, int minor = 0, int release = 0);


private:
    int major_number_;
    int minor_number_;
    int release_number_;
    std::string vendor_number_;
    std::string version_string_;
}; // class OglVersion


} // namespace bbi


#endif // BBI_OGL_VERSION_H
