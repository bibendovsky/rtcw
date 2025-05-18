/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_STRING_INCLUDED
#define RTCW_STRING_INCLUDED

#include <stddef.h>

namespace rtcw {

class String
{
public:
	struct traits_type
	{
		static int length(const char* string);
	};

public:
	String();
	String(const char* string);
	String(const String& that);
	String& operator=(const String& that);
	~String();

	int length() const;
	bool empty() const;
	const char* c_str() const;
	const char* data() const;
	char* data();
	const char& operator[](int index) const;
	char& operator[](int index);

	void clear();
	void reserve(int new_capacity);
	void resize(int new_length);
	void append(char ch);
	void append(const char* string);
	void append(const char* string, int string_length);
	void swap(String& that);

private:
	friend String operator+(const String& a, const char* b);
	friend String operator+(const char* a, const String& b);
	friend String operator+(const String& a, const String& b);

private:
	struct ConcatenateTag {};

private:
	static const int internal_storage_max_length = 15;
	static const int internal_storage_capacity = internal_storage_max_length + 1;

private:
	int capacity_; // Without NUL.
	int length_; // Without NUL.
	char* external_storage_;
	char internal_storage_[internal_storage_capacity];

private:
	char* ctor_initialize(int string_length);
	void ctor_assign(const char* string);
	void ctor_assign(const char* string, int string_length);
	String(ConcatenateTag, const char* a_string, int a_length, const char* b_string, int b_length);
};

// ==========================================================================

String& operator+=(String& a, char b);
String& operator+=(String& a, const char* b);

// --------------------------------------------------------------------------

String operator+(const String& a, const char* b);
String operator+(const char* a, const String& b);
String operator+(const String& a, const String& b);

} // namespace rtcw

#endif // RTCW_STRING_INCLUDED
