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


// BBi
// Former win_shared.c
// BBi


#include <cassert>
#include <cmath>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string_view>

#include "SDL.h"

#include "q_shared.h"
#include "qcommon.h"
#include "sys_events.h"
#include "sys_local.h"


int Sys_Milliseconds()
{
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;

	static auto time_base = TimePoint{};
	static auto is_initialized = false;

	if (is_initialized)
	{
		const auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - time_base);
		const auto time_delta_ms = time_delta.count();

		return static_cast<int>(time_delta_ms);
	}
	else
	{
		is_initialized = true;
		time_base = Clock::now();

		return 0;
	}
}

void Sys_SnapVector(
	float* v)
{
	v[0] = std::trunc(v[0]);
	v[1] = std::trunc(v[1]);
	v[2] = std::trunc(v[2]);
}

int Sys_GetProcessorId()
{
	return CPUID_GENERIC;
}

const char* Sys_GetCurrentUser()
{
	return "player";
}

qboolean Sys_LowPhysicalMemory()
{
	return qfalse;
}

void Sys_StartProcess(
	const char *exeName,
	qboolean doexit)
{
}

void Sys_OpenURL(
	const char* url,
	qboolean doexit)
{
}

void Sys_BeginProfiling()
{
	// this is just used on the mac build
}

// Show the early console as an error dialog
void QDECL Sys_Error(
	const char* error,
	...)
{
	va_list argptr;
	char text[4096];

	va_start(argptr, error);

#if !defined RTCW_ET
	vsprintf(text, error, argptr);
#else
	Q_vsnprintf(text, sizeof(text), error, argptr);
#endif // RTCW_XX

	va_end(argptr);

	Conbuf_AppendText(text);
	Conbuf_AppendText("\n");

	Sys_SetErrorText(text);
	Sys_ShowConsole(1, qtrue);

	IN_Shutdown();

	sys_run_console();
	Com_Quit_f();
	Sys_DestroyConsole();

	exit(1);
}

void Sys_Quit()
{
	IN_Shutdown();
	Sys_DestroyConsole();

	exit(0);
}

void Sys_Print(
	const char* msg)
{
	Conbuf_AppendText(msg);
}

void Sys_Mkdir(
	const char* path)
try
{
	const auto path_u8 = std::filesystem::u8path(path);
	std::filesystem::create_directory(path_u8);
}
catch (...)
{
}

char* Sys_Cwd()
{
	static auto current_path = std::string{};

	try
	{
		const auto path = std::filesystem::current_path();
		current_path = path.u8string();
	}
	catch (...)
	{
		current_path.clear();
	}

	return current_path.data();
}

const char* Sys_DefaultCDPath()
{
	return "";
}

const char* Sys_DefaultBasePath()
{
	return Sys_Cwd();
}


struct SysListFilesIteratorContext
{
	bool is_recursive;

	std::filesystem::directory_iterator iterator;
	std::filesystem::directory_iterator end_iterator;

	std::filesystem::recursive_directory_iterator r_iterator;
	std::filesystem::recursive_directory_iterator end_r_iterator;
}; // SysListFilesIteratorContext


class SysListFilesIteratorProxy
{
public:
	SysListFilesIteratorProxy() noexcept = default;

	SysListFilesIteratorProxy(
		SysListFilesIteratorContext* context)
		:
		context_{context}
	{
	}


	bool operator==(
		const SysListFilesIteratorProxy& rhs) const noexcept
	{
		return context_ == rhs.context_;
	}

	bool operator!=(
		const SysListFilesIteratorProxy& rhs) const noexcept
	{
		return !operator==(rhs);
	}


	const std::filesystem::directory_entry& operator*() const
	{
		assert(context_ != nullptr);

		if (context_->is_recursive)
		{
			return context_->r_iterator.operator*();
		}
		else
		{
			return context_->iterator.operator*();
		}
	}

	const std::filesystem::directory_entry* operator->() const
	{
		assert(context_ != nullptr);

		if (context_->is_recursive)
		{
			return context_->r_iterator.operator->();
		}
		else
		{
			return context_->iterator.operator->();
		}
	}

	void operator++()
	{
		assert(context_ != nullptr);

		if (context_->is_recursive)
		{
			++context_->r_iterator;

			if (context_->r_iterator == std::filesystem::recursive_directory_iterator{})
			{
				context_ = nullptr;
			}
		}
		else
		{
			++context_->iterator;

			if (context_->iterator == std::filesystem::directory_iterator{})
			{
				context_ = nullptr;
			}
		}
	}


private:
	SysListFilesIteratorContext* context_{};
}; // SysListFilesIteratorProxy


class SysListFilesIterator
{
public:
	SysListFilesIterator(
		const std::filesystem::path& path,
		bool is_recursive)
	{
		context_.is_recursive = is_recursive;

		const auto flags =
			std::filesystem::directory_options::follow_directory_symlink |
			std::filesystem::directory_options::skip_permission_denied;

		if (context_.is_recursive)
		{
			context_.r_iterator = std::filesystem::recursive_directory_iterator{path, flags};
		}
		else
		{
			context_.iterator = std::filesystem::directory_iterator{path, flags};
		}
	}

	SysListFilesIteratorProxy begin()
	{
		return SysListFilesIteratorProxy{&context_};
	}

	SysListFilesIteratorProxy end()
	{
		return SysListFilesIteratorProxy{};
	}


private:
	SysListFilesIteratorContext context_{};
}; // SysListFilesIterator

class SysListFilesEntryFilter
{
public:
	SysListFilesEntryFilter(
		const char* extension,
		const char* filter)
		:
		extension_{extension != nullptr ? extension : ""},
		filter_{filter}
	{
		if (filter_ != nullptr)
		{
			is_match_method_ = &SysListFilesEntryFilter::is_match_filter_internal;
		}
		else if (extension_[0] != '\0')
		{
			if (extension_[0] == '/' && extension_[1] == '\0')
			{
				is_match_method_ = &SysListFilesEntryFilter::is_match_all_dirs_internal;
			}
			else
			{
				extension_string_view_ = extension_;
				is_match_method_ = &SysListFilesEntryFilter::is_match_extension_internal;
			}
		}
		else
		{
			is_match_method_ = &SysListFilesEntryFilter::is_match_all_files_internal;
		}
	}

	bool is_match(
		const std::filesystem::directory_entry& dir_entry) const
	{
		return (this->*is_match_method_)(dir_entry);
	}


private:
	using Method = bool (SysListFilesEntryFilter::*)(
		const std::filesystem::directory_entry& dir_entry) const;


	const char* extension_{};
	std::string_view extension_string_view_{};
	const char* filter_{};
	Method is_match_method_{};


	bool is_match_all_files_internal(
		const std::filesystem::directory_entry& dir_entry) const
	{
		return dir_entry.is_regular_file();
	}

	bool is_match_filter_internal(
		const std::filesystem::directory_entry& dir_entry) const
	{
		if (!dir_entry.is_regular_file())
		{
			return false;
		}

		const auto entry_path_u8 = dir_entry.path().u8string();

		return Com_FilterPath(filter_, entry_path_u8.c_str(), false) != 0;
	}

	bool is_match_all_dirs_internal(
		const std::filesystem::directory_entry& dir_entry) const
	{
		return dir_entry.is_directory();
	}

	bool is_match_extension_internal(
		const std::filesystem::directory_entry& dir_entry) const
	{
		if (!dir_entry.is_regular_file())
		{
			return false;
		}

		const auto entry_extension_u6 = dir_entry.path().extension().u8string();

		return entry_extension_u6 == extension_string_view_;
	}
}; // SysListFilesEntryFilter


bool sys_list_files_sort_predicate(
	const char* s0,
	const char* s1)
{
	auto l0 = strlen(s0);
	const auto l1 = strlen(s1);

	if (l1 < l0)
	{
		l0 = l1;
	}

	for (auto i = 0; i < l0; ++i)
	{
		if (s1[i] > s0[i])
		{
			return true;
		}

		if (s1[i] < s0[i])
		{
			return false;
		}
	}

	return false;
}

char** Sys_ListFiles(
	const char* directory,
	const char* extension,
	const char* filter,
	int* numfiles,
	qboolean wantsubs)
{
	if (numfiles == nullptr)
	{
		return nullptr;
	}

	auto& file_count = *numfiles;

	file_count = 0;

	if (directory == nullptr)
	{
		directory = "";
	}

	if (extension == nullptr)
	{
		extension = "";
	}

	constexpr auto MAX_FOUND_FILES = 0x1000;
	char* list[MAX_FOUND_FILES];

	auto directory_length = 0;

	const auto directory_string_view = std::string_view{directory};

	if (!directory_string_view.empty())
	{
		directory_length = static_cast<int>(directory_string_view.size());

		const auto last_char = directory_string_view.back();

		if (last_char != '\\' && last_char != '/')
		{
			directory_length += 1;
		}
	}

	try
	{
		const auto directory_u8 = std::filesystem::u8path(directory);

		const auto entry_filter = SysListFilesEntryFilter{extension, filter};

		for (const auto& entry : SysListFilesIterator{directory_u8, wantsubs != 0})
		{
			if (!entry_filter.is_match(entry))
			{
				continue;
			}

			if (file_count == (MAX_FOUND_FILES - 1))
			{
				break;
			}

			const auto entry_path_u8 = entry.path().u8string();
			const auto entry_name_u8 = entry_path_u8.c_str() + directory_length;

			list[file_count] = CopyString(entry_name_u8);
			file_count += 1;
		}
	}
	catch (...)
	{
	}

	if (file_count == 0)
	{
		return nullptr;
	}

	list[file_count] = nullptr;

	std::sort(list, list + file_count, sys_list_files_sort_predicate);

	auto list_copy = static_cast<char**>(Z_Malloc((file_count + 1) * static_cast<int>(sizeof(char*))));

	for (auto i = 0; i <= file_count; ++i)
	{
		list_copy[i] = list[i];
	}

	return list_copy;
}

void Sys_FreeFileList(
	char** list)
{
	if (list == nullptr)
	{
		return;
	}

	for (auto i = 0; list[i]; ++i)
	{
		Z_Free(list[i]);
	}

	Z_Free(list);
}

qboolean Sys_ScanForCD()
{
	return qfalse;
}

qboolean Sys_CheckCD()
{
	return qtrue;
}

char* Sys_GetClipboardData()
{
	if (SDL_HasClipboardText())
	{
		return SDL_GetClipboardText();
	}

	return nullptr;
}

void Sys_FreeClipboardData(
	char* clipboard_data)
{
	SDL_free(clipboard_data);
}


void Sys_UnloadDll(
	void* dllHandle)
{
	SDL_UnloadObject(dllHandle);
}


extern char* FS_BuildOSPath(
	const char* base,
	const char* game,
	const char* qpath);

#if defined RTCW_MP
#ifdef UPDATE_SERVER
int cl_connectedToPureServer;
#else
extern int cl_connectedToPureServer;
#endif
#elif defined RTCW_ET
extern int cl_connectedToPureServer;
#endif // RTCW_XX

const char* Sys_GetDLLName(
	const char* name)
{
	static const auto bits = std::string{
#if defined RTCW_32
		"x86"
#elif defined RTCW_64
		"x64"
#else
#error Unknown CPU architecture
#endif
	};

	static const auto game = std::string{
#ifdef RTCW_SP
		""
#else
		"_mp_"
#endif
	};

	static const auto demo = std::string{
#ifndef WOLF_SP_DEMO
		""
#else
		"_d"
#endif
	};

	static const auto ext = std::string{
#ifdef __WIN32__
		".dll"
#else
		".so"
#endif
	};

	static const auto buffer = name + game + bits + demo + ext;

	return buffer.c_str();
}

void* QDECL Sys_LoadDll(
	const char* name,
	char* fqpath,
	DllEntryPoint* entryPoint,
	DllEntryPoint systemcalls)
{
	using DllEntry = void (QDECL *)(DllEntryPoint);

	*fqpath = '\0'; // added 2/15/02 by T.Ray

	const auto basepath = Cvar_VariableString("fs_basepath");
	const auto cdpath = Cvar_VariableString("fs_cdpath");
	const auto gamedir = Cvar_VariableString("fs_game");

	const auto filename = std::string{Sys_GetDLLName(name)};

	// try gamepath first
	auto fn = FS_BuildOSPath(basepath, gamedir, filename.c_str());

#if !defined RTCW_SP
	// TTimo - this is only relevant for full client
	// if a full client runs a dedicated server, it's not affected by this
#if !defined DEDICATED
	// NERVE - SMF - extract dlls from pak file for security
	// we have to handle the game dll a little differently
	// TTimo - passing the exact path to check against
	//   (compatibility with other OSes loading procedure)
	if (cl_connectedToPureServer && Q_strncmp(name, "qagame", 6) != 0)
	{
		if (!FS_CL_ExtractFromPakFile(fn, gamedir, filename.c_str(), nullptr))
		{
			Com_Error(
				ERR_DROP,
				"Game code(%s) failed Pure Server check",
				filename.c_str());
		}
	}
#endif
#endif // RTCW_XX

	auto libHandle = SDL_LoadObject(fn);

	if (libHandle == nullptr)
	{
		if (cdpath[0] != '\0')
		{
			fn = FS_BuildOSPath(cdpath, gamedir, filename.c_str());
			libHandle = SDL_LoadObject(fn);
		}
	}

	if (libHandle == nullptr)
	{
		fn = FS_BuildOSPath(basepath, BASEGAME, filename.c_str());
		libHandle = SDL_LoadObject(fn);
	}

	if (libHandle == nullptr)
	{
		strcpy(fn, filename.c_str());
		libHandle = SDL_LoadObject(fn);
	}

	if (libHandle == nullptr)
	{
		return nullptr;
	}


	Q_strncpyz(fqpath, fn, MAX_QPATH); // added 2/15/02 by T.Ray

	const auto dllEntry = reinterpret_cast<DllEntry>(SDL_LoadFunction(libHandle, "dllEntry"));
	*entryPoint = reinterpret_cast<DllEntryPoint>(SDL_LoadFunction(libHandle, "vmMain"));

	if ((*entryPoint) == nullptr || dllEntry == nullptr)
	{
		SDL_UnloadObject(libHandle);
		return nullptr;
	}

	dllEntry(systemcalls);

	return libHandle;
}

constexpr auto MAX_QUED_EVENTS = 256;
constexpr auto MASK_QUED_EVENTS = MAX_QUED_EVENTS - 1;

sysEvent_t eventQue[MAX_QUED_EVENTS];
int eventHead;
int eventTail;
byte sys_packetReceived[MAX_MSGLEN];

//
// A time of 0 will get the current time
// Ptr should either be null, or point to a block of data that can
// be freed by the game later.
//
void Sys_QueEvent(
	int time,
	sysEventType_t type,
	int value,
	int value2,
	int ptrLength,
	void* ptr)
{
	sysEvent_t* ev;

	ev = &eventQue[eventHead & MASK_QUED_EVENTS];

	if ((eventHead - eventTail) >= MAX_QUED_EVENTS)
	{
		Com_Printf("Sys_QueEvent: overflow\n");

		// we are discarding an event, but don't leak memory
		if (ev->evPtr)
		{
			Z_Free(ev->evPtr);
		}

		eventTail += 1;
	}

	eventHead += 1;

	if (time == 0)
	{
		time = Sys_Milliseconds();
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

sysEvent_t Sys_GetEvent()
{
	sysEvent_t ev;
	msg_t netmsg;
	netadr_t adr;

	// return if we have data
	if (eventHead > eventTail)
	{
		eventTail += 1;
		return eventQue[(eventTail - 1) & MASK_QUED_EVENTS];
	}

	// pump the message loop
	sys_poll_events();

	// check for console commands
	const auto s = Sys_ConsoleInput();

	if (s != nullptr)
	{
		const auto len = static_cast<int>(strlen(s)) + 1;
		const auto b = static_cast<char*> (Z_Malloc(len));

		Q_strncpyz(b, s, len - 1);
		Sys_QueEvent(0, SE_CONSOLE, 0, 0, len, b);
	}

	// check for network packets
	MSG_Init(&netmsg, sys_packetReceived, sizeof(sys_packetReceived));

	if (Sys_GetPacket(&adr, &netmsg))
	{
		// copy out to a seperate buffer for qeueing
		// the readcount stepahead is for SOCKS support
		const auto len = static_cast<int>(sizeof(netadr_t)) + netmsg.cursize - netmsg.readcount;
		const auto buf = static_cast<netadr_t*>(Z_Malloc(len));

		*buf = adr;
		memcpy(buf + 1, &netmsg.data[netmsg.readcount], netmsg.cursize - netmsg.readcount);

		Sys_QueEvent(0, SE_PACKET, 0, 0, len, buf);
	}

	// return if we have data
	if (eventHead > eventTail)
	{
		eventTail += 1;
		return eventQue[(eventTail - 1) & MASK_QUED_EVENTS];
	}

	// create an empty event to return

	ev = sysEvent_t{};
	ev.evTime = Sys_Milliseconds();

	return ev;
}

// Restart the input subsystem
void Sys_In_Restart_f()
{
	IN_Shutdown();
	IN_Init();
}


// Restart the network subsystem
void Sys_Net_Restart_f()
{
	NET_Restart();
}


extern void Sys_ClearViewlog_f(); // fretn

// Called after the common systems (cvars, files, etc)
// are initialized
void Sys_Init()
{
	Cmd_AddCommand("in_restart", Sys_In_Restart_f);
	Cmd_AddCommand("net_restart", Sys_Net_Restart_f);
	Cmd_AddCommand("clearviewlog", Sys_ClearViewlog_f);

	// FIXME
	Cvar_Set("arch", SDL_GetPlatform());

	constexpr auto cpuid = CPUID_GENERIC;
	Cvar_SetValue("sys_cpuid", cpuid);

	Cvar_Get("sys_cpustring", "detect", 0);

	auto cpu_string = Cvar_VariableString("sys_cpustring");

	if (Q_stricmp(cpu_string, "detect") == 0)
	{
		Cvar_Set("sys_cpustring", "generic");
	}

	cpu_string = Cvar_VariableString("sys_cpustring");

	Cvar_Set("username", Sys_GetCurrentUser());

#if !defined RTCW_ET
	IN_Init(); // FIXME: not in dedicated?
#endif // RTCW_XX
}

#if defined RTCW_ET
bool Sys_IsNumLockDown()
{
	const auto state = SDL_GetModState();
	return (state & KMOD_NUM) != 0;
}
#endif // RTCW_XX

const char* Sys_DefaultHomePath()
{
	return nullptr;
}

const char* Sys_DefaultInstallPath()
{
	return Sys_Cwd();
}
