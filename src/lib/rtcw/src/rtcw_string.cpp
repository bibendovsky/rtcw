/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_string.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <algorithm>

namespace rtcw {

int String::traits_type::length(const char* string)
{
	return static_cast<int>(strlen(string));
}

String::String()
	:
	capacity_(internal_storage_max_length),
	length_(),
	external_storage_()
{
	internal_storage_[0] = '\0';
}

String::String(const char* string)
{
	ctor_assign(string);
}

String::String(const String& that)
{
	ctor_assign(that.c_str(), that.length());
}

String& String::operator=(const String& that)
{
	assert(&that != this);

	String copy(that);
	swap(copy);
	return *this;
}

String::~String()
{
	::operator delete(external_storage_);
}

int String::length() const
{
	return length_;
}

bool String::empty() const
{
	return length() == 0;
}

const char* String::c_str() const
{
	return data();
}

const char* String::data() const
{
	return external_storage_ != NULL ? external_storage_ : internal_storage_;
}

char* String::data()
{
	return const_cast<char*>(const_cast<const String*>(this)->data());
}

const char& String::operator[](int index) const
{
	assert(index >= 0 && index < length());

	return data()[index];
}

char& String::operator[](int index)
{
	return const_cast<char&>(const_cast<const String*>(this)->operator[](index));
}

void String::clear()
{
	length_ = 0;
	data()[0] = '\0';
}

void String::reserve(int new_capacity)
{
	assert(new_capacity >= 0);

	if (new_capacity <= capacity_)
	{
		return;
	}

	char* const new_storage = static_cast<char*>(::operator new(static_cast<size_t>(new_capacity + 1)));
	const char* const src_data = data();
	std::copy(src_data, &src_data[length() + 1], new_storage);
	capacity_ = new_capacity;
	external_storage_ = new_storage;
}

void String::resize(int new_length)
{
	assert(new_length >= 0);

	reserve(new_length);
	const int old_length = length();
	char* const dst_data = data();
	const int fill_count = std::max(new_length - old_length + 1, 1);
	std::fill_n(&dst_data[old_length], fill_count, '\0');
	length_ = new_length;
}

String& String::append(char ch)
{
	return append(&ch, 1);
}

String& String::append(const char* string)
{
	const int string_length = traits_type::length(string);
	return append(string, string_length);
}

String& String::append(const char* string, int string_length)
{
	const int old_length = length();
	const int new_length = old_length + string_length;
	reserve(new_length);
	char* const dst_data = data();
	std::copy(string, &string[string_length], &dst_data[old_length]);
	dst_data[new_length] = '\0';
	length_ = new_length;
	return *this;
}

String& String::append(const String& string)
{
	return append(string.data(), string.length());
}

void String::swap(String& that)
{
	std::swap(capacity_, that.capacity_);
	std::swap(length_, that.length_);
	std::swap(external_storage_, that.external_storage_);
	std::swap_ranges(internal_storage_, &internal_storage_[internal_storage_capacity], that.internal_storage_);
}

char* String::ctor_initialize(int string_length)
{
	length_ = string_length;

	if (string_length <= internal_storage_max_length)
	{
		capacity_ = internal_storage_max_length;
		external_storage_ = NULL;
		return internal_storage_;
	}
	else
	{
		capacity_ = string_length;
		external_storage_ = static_cast<char*>(::operator new(static_cast<size_t>(string_length + 1)));
		return external_storage_;
	}
}

void String::ctor_assign(const char* string)
{
	const int string_length = traits_type::length(string);
	ctor_assign(string, string_length);
}

void String::ctor_assign(const char* string, int string_length)
{
	char* const storage = ctor_initialize(string_length);
	std::copy(string, &string[string_length + 1], storage);
}

String::String(ConcatenateTag, const char* a_string, int a_length, const char* b_string, int b_length)
{
	const int string_length = a_length + b_length;
	char* const storage = ctor_initialize(string_length);
	std::copy(a_string, &a_string[a_length], storage);
	std::copy(b_string, &b_string[b_length], &storage[a_length]);
	storage[string_length] = '\0';
}

// ==========================================================================

String& operator+=(String& a, char b)
{
	a.append(b);
	return a;
}

String& operator+=(String& a, const char* b)
{
	a.append(b);
	return a;
}

String& operator+=(String& a, const String& b)
{
	a.append(b);
	return a;
}

// --------------------------------------------------------------------------

String operator+(const String& a, const char* b)
{
	const int b_length = String::traits_type::length(b);
	return String(String::ConcatenateTag(), a.c_str(), a.length(), b, b_length);
}

String operator+(const char* a, const String& b)
{
	const int a_length = String::traits_type::length(a);
	return String(String::ConcatenateTag(), a, a_length, b.c_str(), b.length());
}

String operator+(const String& a, const String& b)
{
	return String(String::ConcatenateTag(), a.c_str(), a.length(), b.c_str(), b.length());
}

} // namespace rtcw
