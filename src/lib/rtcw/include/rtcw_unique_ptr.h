/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_UNIQUE_PTR_INCLUDED
#define RTCW_UNIQUE_PTR_INCLUDED

#include <cassert>
#include <cstddef>

namespace rtcw {

template<typename T>
struct UniquePtrDefaultDeleter
{
	void operator()(T* pointer) const
	{
		delete pointer;
	}
};

// =====================================

template<typename T, typename TDeleter>
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
		TDeleter()(ptr_);
		ptr_ = ptr;
	}

private:
	T* ptr_;

private:
	UniquePtr(UniquePtr& rhs);
	UniquePtr& operator=(UniquePtr& rhs);
};

// -------------------------------------

template<typename T, typename TDeleter>
UniquePtr<T, TDeleter>::UniquePtr()
	:
	ptr_()
{}

template<typename T, typename TDeleter>
UniquePtr<T, TDeleter>::~UniquePtr()
{
	TDeleter()(ptr_);
}

template<typename T, typename TDeleter>
T* UniquePtr<T, TDeleter>::get() const
{
	return ptr_;
}

template<typename T, typename TDeleter>
T* UniquePtr<T, TDeleter>::operator->() const
{
	return ptr_;
}

template<typename T, typename TDeleter>
T* UniquePtr<T, TDeleter>::release()
{
	T* result = ptr_;
	ptr_ = NULL;
	return result;
}

template<typename T, typename TDeleter>
void UniquePtr<T, TDeleter>::reset()
{
	reset(static_cast<T*>(NULL));
}

} // namespace rtcw

#endif // RTCW_UNIQUE_PTR_INCLUDED
