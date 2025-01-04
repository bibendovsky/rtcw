#include "rtcw_ogl_matrix_stack.h"
#include <cassert>
#include "glm/gtc/type_ptr.hpp"


namespace rtcw
{


OglMatrixStack::OglMatrixStack()
	:
	current_{1}
{
}

void OglMatrixStack::pop()
{
	assert(!stack_.empty());

	current_ = stack_.top();
	stack_.pop();
}

OglMatrixStack::Matrix& OglMatrixStack::pop_and_get()
{
	pop();
	return get_current();
}

float* OglMatrixStack::pop_and_get_items()
{
	pop();
	return get_current_items();
}

void OglMatrixStack::push()
{
	assert(static_cast<int>(stack_.size()) < get_max_depth());

	if (stack_.size() == get_max_depth())
	{
		return;
	}

	stack_.push(current_);
}

OglMatrixStack::Matrix& OglMatrixStack::push_and_get()
{
	push();
	return get_current();
}

void OglMatrixStack::push_and_set(
	const Matrix& value)
{
	push();
	set_current(value);
}

void OglMatrixStack::push_and_set(
	const float items[16])
{
	push();
	set_current(items);
}

void OglMatrixStack::push_and_set_identity()
{
	push();
	set_current(Matrix{1});
}

OglMatrixStack::Matrix& OglMatrixStack::get_current()
{
	return current_;
}

float* OglMatrixStack::get_current_items()
{
	return glm::value_ptr(current_);
}

const float* OglMatrixStack::get_current_items() const
{
	return glm::value_ptr(current_);
}

const OglMatrixStack::Matrix& OglMatrixStack::get_current() const
{
	return current_;
}

void OglMatrixStack::set_current(
	const Matrix& value)
{
	current_ = value;
}

void OglMatrixStack::set_current(
	const float items[16])
{
	current_ = Matrix{
		items[0], items[1], items[2], items[3],
		items[4], items[5], items[6], items[7],
		items[8], items[9], items[10], items[11],
		items[12], items[13], items[14], items[15]
	};
}

int OglMatrixStack::get_max_depth()
{
	return 1;
}


} // rtcw
