/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_OGL_MATRIX_STACK_INCLUDED
#define RTCW_OGL_MATRIX_STACK_INCLUDED

#include "rtcw_cgm_mat.h"

namespace rtcw {

class OglMatrixStack
{
public:
	static const int model_view_max_depth = 32;
	static const int projection_max_depth = 2;

public:
	typedef cgm::Mat4 Matrix;

public:
	explicit OglMatrixStack(int capacity);

	// Pops a top matrix from the stack and replaces by it a current one.
	void pop();

	// Pops a top matrix from the stack, replaces by it a current one and
	// returns it.
	Matrix& pop_and_get();

	// Pushes a current matrix into the stack.
	void push();

	// Pushes a current matrix into the stack and returns it.
	Matrix& push_and_get();

	// Pushes a current matrix into the stack and replaces the current matrix
	// by a specified one.
	void push_and_set(const Matrix& value);

	// Pushes a current matrix into the stack and replaces the current matrix
	// with the identity one.
	void push_and_set_identity();

	// Returns a current matrix.
	Matrix& get_current();

	// Returns elements of the current matrix.
	const Matrix& get_current() const;

	// Sets a current matrix.
	void set_current(const Matrix& value);

	// Sets a current matrix.
	// Note: Items must be specified in column-major order.
	void set_current(const float items[16]);

private:
	class Impl;

private:
	Matrix* stack_;
	int stack_capacity_;
	int stack_size_;

	Matrix current_;

private:
	OglMatrixStack(const OglMatrixStack& that);
	OglMatrixStack& operator=(const OglMatrixStack& that);
};

} // namespace rtcw

#endif // RTCW_OGL_MATRIX_STACK_INCLUDED
