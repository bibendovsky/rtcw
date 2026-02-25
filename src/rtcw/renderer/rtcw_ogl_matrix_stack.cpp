/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_ogl_matrix_stack.h"
#include "rtcw_array_trivial.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace rtcw {

class OglMatrixStack::Impl
{
public:
	static Matrix* allocate(int count);

private:
	static const int storage_capacity = model_view_max_depth + projection_max_depth;

	typedef ArrayTrivial<Matrix, storage_capacity> Storage;

	static Storage storage_;
	static int storage_size_;
};

// -------------------------------------

OglMatrixStack::Impl::Storage OglMatrixStack::Impl::storage_;
int OglMatrixStack::Impl::storage_size_ = 0;

// -------------------------------------

OglMatrixStack::Matrix* OglMatrixStack::Impl::allocate(int count)
{
	assert(count >= 0);
	if (storage_capacity - storage_size_ < count)
	{
		std::fputs("\n[rtcw::OglMatrixStack] Out of memory.\n", stderr);
		assert(false && "Out of memory.");
		std::abort();
	}
	Matrix* const result = &storage_[storage_size_];
	storage_size_ += count;
	return result;
}

// =====================================

// FIXME
OglMatrixStack::OglMatrixStack(int capacity)
	:
	stack_(Impl::allocate(capacity)),
	stack_capacity_(capacity),
	stack_size_(),
	current_(Matrix::identity)
{}

void OglMatrixStack::pop()
{
	assert(stack_size_ > 0);
	current_ = stack_[stack_size_ - 1];
	--stack_size_;
}

OglMatrixStack::Matrix& OglMatrixStack::pop_and_get()
{
	pop();
	return get_current();
}

void OglMatrixStack::push()
{
	assert(stack_size_ < stack_capacity_);
	stack_[stack_size_] = current_;
	++stack_size_;
}

OglMatrixStack::Matrix& OglMatrixStack::push_and_get()
{
	push();
	return get_current();
}

void OglMatrixStack::push_and_set(const Matrix& value)
{
	push();
	set_current(value);
}

void OglMatrixStack::push_and_set_identity()
{
	push();
	set_current(Matrix::identity);
}

OglMatrixStack::Matrix& OglMatrixStack::get_current()
{
	return current_;
}

const OglMatrixStack::Matrix& OglMatrixStack::get_current() const
{
	return current_;
}

void OglMatrixStack::set_current(const Matrix& value)
{
	current_ = value;
}

void OglMatrixStack::set_current(const float items[16])
{
	current_ = Matrix(
		items[0], items[1], items[2], items[3],
		items[4], items[5], items[6], items[7],
		items[8], items[9], items[10], items[11],
		items[12], items[13], items[14], items[15]);
}

} // namespace rtcw
