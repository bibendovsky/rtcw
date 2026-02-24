/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// Vector for trivial types

#ifndef RTCW_VECTOR_TRIVIAL_INCLUDED
#define RTCW_VECTOR_TRIVIAL_INCLUDED

#include "rtcw_memory.h"
#include <cassert>
#include <climits>
#include <algorithm>

namespace rtcw {

template<typename T>
class VectorTrivial
{
public:
	VectorTrivial();
	VectorTrivial(const VectorTrivial& that);
	VectorTrivial& operator=(const VectorTrivial&);
	~VectorTrivial();

	const T* get_data() const;
	T* get_data();
	int get_size() const;
	int get_capacity() const;
	bool is_empty() const;
	void reserve(int new_capacity);
	void resize(int new_size);
	void resize(int new_size, const T& value);
	void resize_uninitialized(int new_size);
	void clear();
	void insert_range(int index, const T* src_elements, int src_count);
	void erase(int index, int count);
	const T& operator[](int index) const;
	T& operator[](int index);
	void swap(VectorTrivial& that);

private:
	static const int element_size = sizeof(T);
	static const int max_capacity = (INT_MAX - 1) / element_size;

	static const T default_filler_;
	T* elements_;
	int size_;
	int capacity_;

	static void free_elements(void* elements);
	void resize_internal(int new_size, const T* value);
};

// -------------------------------------

template<typename T>
const T VectorTrivial<T>::default_filler_ = T();

// -------------------------------------

template<typename T>
VectorTrivial<T>::VectorTrivial()
	:
	elements_(),
	size_(),
	capacity_()
{}

template<typename T>
VectorTrivial<T>::VectorTrivial(const VectorTrivial& that)
	:
	elements_(),
	size_(),
	capacity_()
{
	if (that.is_empty())
	{
		return;
	}
	elements_ = static_cast<T*>(mem::allocate(that.size_ * sizeof(T)));
	size_ = that.size_;
	capacity_ = that.size_;
}

template<typename T>
VectorTrivial<T>& VectorTrivial<T>::operator=(const VectorTrivial& that)
{
	assert(&that != this);
	VectorTrivial that_copy(that);
	swap(that_copy);
	return *this;
}

template<typename T>
VectorTrivial<T>::~VectorTrivial()
{
	free_elements(elements_);
}

template<typename T>
const T* VectorTrivial<T>::get_data() const
{
	return elements_;
}

template<typename T>
T* VectorTrivial<T>::get_data()
{
	return elements_;
}

template<typename T>
int VectorTrivial<T>::get_size() const
{
	return size_;
}

template<typename T>
int VectorTrivial<T>::get_capacity() const
{
	return capacity_;
}

template<typename T>
bool VectorTrivial<T>::is_empty() const
{
	return size_ == 0;
}

template<typename T>
void VectorTrivial<T>::reserve(int new_capacity)
{
	assert(new_capacity >= 0);
	if (new_capacity <= capacity_)
	{
		return;
	}
	assert(new_capacity <= max_capacity);
	T* const new_elements = static_cast<T*>(mem::allocate(new_capacity * sizeof(T)));
	const int old_elements_size = size_ * element_size;
	std::copy(elements_, elements_ + old_elements_size, new_elements);
	free_elements(elements_);
	elements_ = new_elements;
	capacity_ = new_capacity;
}

template<typename T>
void VectorTrivial<T>::resize(int new_size)
{
	resize_internal(new_size, &default_filler_);
}

template<typename T>
void VectorTrivial<T>::resize(int new_size, const T& value)
{
	resize_internal(new_size, &value);
}

template<typename T>
void VectorTrivial<T>::resize_uninitialized(int new_size)
{
	resize_internal(new_size, NULL);
}

template<typename T>
void VectorTrivial<T>::clear()
{
	size_ = 0;
}

template<typename T>
void VectorTrivial<T>::insert_range(int index, const T* src_elements, int src_count)
{
	assert(index >= 0 && index <= size_);
	assert(src_elements != NULL);
	assert(src_count >= 0 && src_count <= max_capacity - size_);
	if (src_count == 0)
	{
		return;
	}
	resize_internal(size_ + src_count, NULL);
	const int new_size = size_ + src_count;
	const int move_count = size_ - index;
	for (int i = 0; i < move_count; ++i)
	{
		elements_[new_size - 1 - i] = elements_[index + move_count - 1 - i];
	}
	std::copy(src_elements, &src_elements[src_count], &elements_[index]);
}

template<typename T>
void VectorTrivial<T>::erase(int index, int count)
{
	assert(index >= 0 && index < size_);
	assert(count >= 0 && count <= size_);
	assert(index + count <= size_);
	const int move_count = size_ - index - count;
	for (int i = 0; i < move_count; ++i)
	{
		elements_[index + i] = elements_[index + count + i];
	}
	size_ -= count;
}

template<typename T>
const T& VectorTrivial<T>::operator[](int index) const
{
	assert(index >= 0 && index < size_);
	return elements_[index];
}

template<typename T>
T& VectorTrivial<T>::operator[](int index)
{
	assert(index >= 0 && index < size_);
	return elements_[index];
}

template<typename T>
void VectorTrivial<T>::swap(VectorTrivial& that)
{
	std::swap(elements_, that.elements_);
	std::swap(size_, that.size_);
	std::swap(capacity_, that.capacity_);
}

template<typename T>
void VectorTrivial<T>::free_elements(void* elements)
{
	mem::deallocate(elements);
}

template<typename T>
void VectorTrivial<T>::resize_internal(int new_size, const T* value)
{
	assert(new_size >= 0);
	if (new_size <= size_)
	{
		size_ = new_size;
		return;
	}
	reserve(new_size);
	if (value != NULL)
	{
		std::fill_n(elements_ + size_, new_size - size_, *value);
	}
	size_ = new_size;
}

} // namespace rtcw

#endif // RTCW_VECTOR_TRIVIAL_INCLUDED
