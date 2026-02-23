/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// HDR GLSL program

#ifndef RTCW_OGL_HDR_PROGRAM_INCLUDED
#define RTCW_OGL_HDR_PROGRAM_INCLUDED

#include "rtcw_string.h"
#include "rtcw_ogl_program.h"

namespace rtcw {

class OglHdrProgram : public OglProgram
{
public:
	int a_pos_vec2;
	int a_tc_vec2;
	int u_tex_2d;
	int u_cctf_id;
	int u_cctf_gamma;
	int u_sdr_white_level;

	OglHdrProgram(const String& glsl_dir, const String& base_name);
	OglHdrProgram(const char* vertex_shader_source, const char* fragment_shader_source);
	~OglHdrProgram();

	virtual void destroy();
	virtual bool reload();
	virtual void unload();

protected:
	virtual OglProgram* create_new(const String& glsl_dir, const String& base_name);
	virtual OglProgram* create_new(const char* vertex_shader_source, const char* fragment_shader_source);

private:
	static const char* const impl_attribute_names_[max_vertex_attributes];

	OglHdrProgram(const OglHdrProgram&);
	OglHdrProgram& operator=(const OglHdrProgram&);

	void unload_internal();
	bool reload_internal();
};

} // namespace rtcw

#endif // RTCW_OGL_HDR_PROGRAM_INCLUDED
