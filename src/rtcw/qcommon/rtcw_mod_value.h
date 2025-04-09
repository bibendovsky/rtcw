#ifndef RTCW_MOD_VALUE_INCLUDED
#define RTCW_MOD_VALUE_INCLUDED


namespace rtcw
{


template<
	typename T
>
class ModValue
{
public:
	ModValue()
		:
		value_(),
		is_modified_()
	{}

	bool is_modified() const
	{
		return is_modified_;
	}

	const T& get() const
	{
		return value_;
	}

	void set(
		const T& value)
	{
		is_modified_ |= (value != value_);
		value_ = value;
	}

	void set_modified(
		bool value)
	{
		is_modified_ = value;
	}


private:
	T value_;
	bool is_modified_;


	ModValue(
		const ModValue& that);
}; // ModValue


} // rtcw


#endif // !RTCW_MOD_VALUE_INCLUDED
