#ifndef RTCW_MOD_VALUE_H
#define RTCW_MOD_VALUE_H


namespace rtcw {


template<class T>
class ModValue {
public:
    ModValue () :
        value_ (),
        is_modified_ (false)
    {
    }

    ~ModValue ()
    {
    }

    bool is_modified () const
    {
        return is_modified_;
    }

    const T& get () const
    {
        return value_;
    }

    void set (const T& value)
    {
        is_modified_ |= (value != value_);
        value_ = value;
    }

    void set_modified (bool value)
    {
        is_modified_ = value;
    }


private:
    T value_;
    bool is_modified_;

    ModValue (const ModValue& that);
}; // class ModValue


} // namespace rtcw


#endif // RTCW_MOD_VALUE_H
