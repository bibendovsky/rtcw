/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_UNIQUE_PTR_INCLUDED
#define RTCW_UNIQUE_PTR_INCLUDED

#include <assert.h>
#include <stddef.h>

namespace rtcw {

template<typename T>
class UniquePtr
{
public:
	UniquePtr();

	template<typename U>
	explicit UniquePtr(U* ptr)
		:
		ptr_(ptr)
	{}

	~UniquePtr();

	T* get() const;
	T* operator->() const;
	T* release();

	void reset();

	template<typename U>
	void reset(U* ptr)
	{
		assert(ptr == NULL || (ptr != NULL && ptr_ != ptr));
		delete ptr_;
		ptr_ = ptr;
	}

private:
	T* ptr_;

private:
	UniquePtr(UniquePtr& rhs);
	UniquePtr& operator=(UniquePtr& rhs);
};

// --------------------------------------------------------------------------

template<typename T>
UniquePtr<T>::UniquePtr()
	:
	ptr_()
{}

template<typename T>
UniquePtr<T>::~UniquePtr()
{
	delete ptr_;
}

template<typename T>
T* UniquePtr<T>::get() const
{
	return ptr_;
}

template<typename T>
T* UniquePtr<T>::operator->() const
{
	return ptr_;
}

template<typename T>
T* UniquePtr<T>::release()
{
	T* result = ptr_;
	ptr_ = NULL;
	return result;
}

template<typename T>
void UniquePtr<T>::reset()
{
	reset(static_cast<T*>(NULL));
}

} // namespace rtcw

#endif // RTCW_UNIQUE_PTR_INCLUDED
