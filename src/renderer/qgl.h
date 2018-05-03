/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
** QGL.H
*/

#ifndef __QGL_H__
#define __QGL_H__


#include "sdl_ogl11_loader_gl.h"
#include "SDL_opengl_glext.h"


void APIENTRY glActiveTexture(
	GLenum texture);

void APIENTRY glAttachShader(
	GLuint program,
	GLuint shader);

void APIENTRY glBindBuffer(
	GLenum target,
	GLuint buffer);

void APIENTRY glBufferData(
	GLenum target,
	GLsizeiptr size,
	const void* data,
	GLenum usage);

void APIENTRY glBufferSubData(
	GLenum target,
	GLintptr offset,
	GLsizeiptr size,
	const void* data);

void APIENTRY glClientActiveTexture(
	GLenum texture);

void APIENTRY glCompileShader(
	GLuint shader);

GLuint APIENTRY glCreateProgram();

GLuint APIENTRY glCreateShader(
	GLenum type);

void APIENTRY glDeleteBuffers(
	GLsizei n,
	const GLuint* buffers);

void APIENTRY glDeleteProgram(
	GLuint program);

void APIENTRY glDeleteShader(
	GLuint shader);

void APIENTRY glDisableVertexAttribArray(
	GLuint index);

void APIENTRY glDrawElementsBaseVertex(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const void* indices,
	GLint basevertex);

void APIENTRY glEnableVertexAttribArray(
	GLuint index);

void APIENTRY glGenBuffers(
	GLsizei n,
	GLuint* buffers);

void APIENTRY glGenerateMipmap(
	GLenum target);

GLint APIENTRY glGetAttribLocation(
	GLuint program,
	const GLchar* name);

void APIENTRY glGetProgramInfoLog(
	GLuint program,
	GLsizei bufSize,
	GLsizei* length,
	GLchar* infoLog);

void APIENTRY glGetProgramiv(
	GLuint program,
	GLenum pname,
	GLint* params);

void APIENTRY glGetShaderInfoLog(
	GLuint shader,
	GLsizei bufSize,
	GLsizei* length,
	GLchar* infoLog);

void APIENTRY glGetShaderiv(
	GLuint shader,
	GLenum pname,
	GLint* params);

GLint APIENTRY glGetUniformLocation(
	GLuint program,
	const GLchar* name);

void APIENTRY glLinkProgram(
	GLuint program);

void APIENTRY glLockArraysEXT(
	GLint first,
	GLsizei count);

void APIENTRY glMultiTexCoord2f(
	GLenum target,
	GLfloat s,
	GLfloat t);

void APIENTRY glShaderSource(
	GLuint shader,
	GLsizei count,
	const GLchar* const* string,
	const GLint* length);

void APIENTRY glUniform1f(
	GLint location,
	GLfloat v0);

void APIENTRY glUniform1i(
	GLint location,
	GLint v0);

void APIENTRY glUniform4fv(
	GLint location,
	GLsizei count,
	const GLfloat* value);

void APIENTRY glUniformMatrix4fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat* value);

void APIENTRY glUnlockArraysEXT();

void APIENTRY glUseProgram(
	GLuint program);

void APIENTRY glVertexAttrib2f(
	GLuint index,
	GLfloat x,
	GLfloat y);

void APIENTRY glVertexAttrib4f(
	GLuint index,
	GLfloat x,
	GLfloat y,
	GLfloat z,
	GLfloat w);

void APIENTRY glVertexAttribPointer(
	GLuint index,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLsizei stride,
	const void* pointer);


#endif
