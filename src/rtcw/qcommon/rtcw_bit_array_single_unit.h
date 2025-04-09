#ifndef RTCW_BIT_ARRAY_SINGLE_UNIT_INCLUDED
#define RTCW_BIT_ARRAY_SINGLE_UNIT_INCLUDED

#include <assert.h>
#include <limits.h>
#include "rtcw_type_utility.h"

namespace rtcw {

template<int TCapacity>
class BitArraySingleUnit
{
public:
	BitArraySingleUnit();

	void clear();
	bool is_clear() const;
	bool is_set(int index) const;
	void toggle(int index);
	bool operator[](int index) const;

private:
	typedef
		typename Conditional<
			sizeof(unsigned char) * CHAR_BIT >= TCapacity,
			unsigned char,
			typename Conditional<
				sizeof(unsigned short) * CHAR_BIT >= TCapacity,
				unsigned short,
				typename Conditional<
					sizeof(unsigned int) * CHAR_BIT >= TCapacity,
					unsigned int,
					typename Conditional<
						sizeof(unsigned long) * CHAR_BIT >= TCapacity,
						unsigned long,
						typename Conditional<
							sizeof(unsigned long long) * CHAR_BIT >= TCapacity,
							unsigned long long,
							void
						>::Type
					>::Type
				>::Type
			>::Type
		>::Type
	Storage;

private:
	Storage storage_;
};

// --------------------------------------------------------------------------

template<int TCapacity>
BitArraySingleUnit<TCapacity>::BitArraySingleUnit()
	:
	storage_()
{}

template<int TCapacity>
void BitArraySingleUnit<TCapacity>::clear()
{
	storage_ = 0;
}

template<int TCapacity>
bool BitArraySingleUnit<TCapacity>::is_clear() const
{
	return storage_ == 0;
}

template<int TCapacity>
bool BitArraySingleUnit<TCapacity>::is_set(int index) const
{
	assert(index >= 0 && index < TCapacity);
	return ((storage_ >> index) & 1) != 0;
}

template<int TCapacity>
void BitArraySingleUnit<TCapacity>::toggle(int index)
{
	assert(index >= 0 && index < TCapacity);
	storage_ = static_cast<Storage>(storage_ ^ (Storage(1) << index));
}

template<int TCapacity>
bool BitArraySingleUnit<TCapacity>::operator[](int index) const
{
	return is_set(index);
}

} // namespace rtcw

#endif // RTCW_BIT_ARRAY_SINGLE_UNIT_INCLUDED
