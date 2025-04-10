/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

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
