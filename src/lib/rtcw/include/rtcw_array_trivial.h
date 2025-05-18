/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_ARRAY_TRIVIAL_INCLUDED
#define RTCW_ARRAY_TRIVIAL_INCLUDED

// Fixed-size array for trivial types.

#include <assert.h>
#include <algorithm>

// ==========================================================================

namespace rtcw {

template<typename T, int TSize>
class ArrayTrivial
{
public:
	ArrayTrivial();
	ArrayTrivial(const ArrayTrivial& that);
	ArrayTrivial& operator=(const ArrayTrivial&);
	~ArrayTrivial() {}

	const T* get_data() const;
	T* get_data();
	int get_size() const;
	void fill(const T& value);
	const T& operator[](int index) const;
	T& operator[](int index);
	void swap(ArrayTrivial& that);

	const T* cbegin() const;
	const T* begin() const;
	T* begin();

	const T* cend() const;
	const T* end() const;
	T* end();

private:
	typedef T Elements[TSize];

private:
	Elements elements_;
};

// --------------------------------------------------------------------------

template<typename T, int TSize>
ArrayTrivial<T, TSize>::ArrayTrivial()
{
	fill(T());
}

template<typename T, int TSize>
ArrayTrivial<T, TSize>::ArrayTrivial(const ArrayTrivial& that)
{
	std::copy(that.cbegin(), that.cend(), begin());
}

template<typename T, int TSize>
ArrayTrivial<T, TSize>& ArrayTrivial<T, TSize>::operator=(const ArrayTrivial& that)
{
	assert(&that != this);
	std::copy(that.cbegin(), that.cend(), begin());
	return *this;
}

template<typename T, int TSize>
const T* ArrayTrivial<T, TSize>::get_data() const
{
	return elements_;
}

template<typename T, int TSize>
T* ArrayTrivial<T, TSize>::get_data()
{
	return elements_;
}

template<typename T, int TSize>
int ArrayTrivial<T, TSize>::get_size() const
{
	return TSize;
}

template<typename T, int TSize>
void ArrayTrivial<T, TSize>::fill(const T& value)
{
	std::fill_n(elements_, TSize, value);
}

template<typename T, int TSize>
const T& ArrayTrivial<T, TSize>::operator[](int index) const
{
	assert(index >= 0 && index < TSize);
	return elements_[index];
}

template<typename T, int TSize>
T& ArrayTrivial<T, TSize>::operator[](int index)
{
	assert(index >= 0 && index < TSize);
	return elements_[index];
}

template<typename T, int TSize>
void ArrayTrivial<T, TSize>::swap(ArrayTrivial& that)
{
	std::swap_ranges(elements_, &elements_[TSize], that.elements_);
}

template<typename T, int TSize>
const T* ArrayTrivial<T, TSize>::cbegin() const
{
	return begin();
}

template<typename T, int TSize>
const T* ArrayTrivial<T, TSize>::begin() const
{
	return elements_;
}

template<typename T, int TSize>
T* ArrayTrivial<T, TSize>::begin()
{
	return elements_;
}

template<typename T, int TSize>
const T* ArrayTrivial<T, TSize>::cend() const
{
	return end();
}

template<typename T, int TSize>
const T* ArrayTrivial<T, TSize>::end() const
{
	return &elements_[TSize];
}

template<typename T, int TSize>
T* ArrayTrivial<T, TSize>::end()
{
	return &elements_[TSize];
}

} // namespace rtcw

#endif // RTCW_ARRAY_TRIVIAL_INCLUDED
