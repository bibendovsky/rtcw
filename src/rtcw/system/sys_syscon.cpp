/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include <algorithm>
#include <string.h>
#include "client.h"
#include "sys_local.h"
#include "rtcw_syscon.h"
#include "rtcw_vector_trivial.h"

namespace {

const char* const syscon_title =
#if defined(RTCW_SP)
	"RTCW-SP Console"
#elif defined(RTCW_MP)
	#ifdef UPDATE_SERVER // DHM - Nerve
		"RTCW-MP Update Server"
	#else
		"RTCW-MP Console"
	#endif
#elif defined(RTCW_ET)
	"RTCW-ET Console"
#endif // RTCW_XX
	; // syscon_title

const int syscon_input_initial_capacity = 128 + 2;

typedef rtcw::VectorTrivial<char> SysconInput;

rtcw::Syscon syscon;
bool syscon_quit_on_close = false;
char* syscon_input_ptr = NULL;
SysconInput syscon_input;

void syscon_clear_input()
{
	syscon_input_ptr = NULL;

	syscon_input.clear();

	if (syscon_input.get_data() != NULL)
	{
		syscon_input.get_data()[0] = '\0';
	}
}

void syscon_input_callback(const char* text)
{
	Sys_Print(va("]%s\n", text));

	const int text_length = static_cast<int>(strlen(text));
	const int text_length_with_tail = text_length + 2;
	syscon_input.reserve(text_length_with_tail);
	std::copy(text, &text[text_length], syscon_input.get_data());
	syscon_input.resize_uninitialized(text_length_with_tail);
	syscon_input[text_length + 0] = '\n';
	syscon_input[text_length + 1] = '\0';
	syscon_input_ptr = syscon_input.get_data();
}

void syscon_show_mode_callback(rtcw::Syscon::ShowMode show_mode)
{
	if (com_viewlog != NULL && com_dedicated != NULL && com_dedicated->integer == 0)
	{
		if (com_viewlog->integer == 1)
		{
			if (show_mode == rtcw::Syscon::show_mode_minimized)
			{
				Cvar_Set("viewlog", "2");
			}
		}
		else if (com_viewlog->integer == 2)
		{
			if (show_mode != rtcw::Syscon::show_mode_minimized)
			{
				Cvar_Set("viewlog", "1");
			}
		}
	}
}

void syscon_quit_callback()
{
	if (com_dedicated != NULL && com_dedicated->integer != 0)
	{
		char* cmd_string = CopyString("quit");
		Sys_QueEvent(0, SE_CONSOLE, 0, 0, static_cast<int>(strlen(cmd_string)) + 1, cmd_string);
	}
	else if (syscon_quit_on_close)
	{}
	else
	{
		Sys_ShowConsole(0, qfalse);
		Cvar_Set("viewlog", "0");
	}
}

void syscon_callback(rtcw::Syscon::CallbackParam param)
{
	switch (param.type)
	{
		case rtcw::Syscon::callback_type_input_line:
			syscon_input_callback(param.input_line_text);
			break;
			
		case rtcw::Syscon::callback_type_show_mode:
			syscon_show_mode_callback(param.show_mode);
			break;

		case rtcw::Syscon::callback_type_quit:
			syscon_quit_callback();
			break;
	}
}

} // namespace

void Sys_CreateConsole()
{
	if (!syscon.initialize(syscon_callback))
	{
		Sys_Print(va("[syscon] %s\n", syscon.get_error_message()));
	}

	syscon.set_title(syscon_title);
	syscon_input.reserve(syscon_input_initial_capacity);
	syscon_clear_input();
}

void Sys_DestroyConsole()
{
	syscon.terminate();
}

void Sys_ShowConsole(int visibility_level, qboolean quit_on_close)
{
	syscon_quit_on_close = quit_on_close == qtrue;
	rtcw::Syscon::ShowMode show_mode;

	switch (visibility_level)
	{
		case 0:
			show_mode = rtcw::Syscon::show_mode_hidden;
			break;

		default:
		case 1:
			show_mode = rtcw::Syscon::show_mode_normal;
			break;

		case 2:
			show_mode = rtcw::Syscon::show_mode_minimized;
			break;
	}

	syscon.show(show_mode);
}

char* Sys_ConsoleInput()
{
	char* const old_syscon_input_ptr = syscon_input_ptr;
	syscon_input_ptr = NULL;
	return old_syscon_input_ptr;
}

void Conbuf_AppendText(const char* text)
{
	syscon.append_text(text);
}

void Sys_SetErrorText(const char* text)
{
	syscon.set_error_text(text);
}

void Sys_ClearViewlog_f()
{
	syscon.clear_text();
}

void sys_run_console()
{
	syscon.run();
}

void sys_handle_console_sdl_event(const SDL_Event& e)
{
	syscon.handle_event(e);
}

void sys_update_console()
{
	syscon.update();
}
