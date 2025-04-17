/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


// BBi
// Former win_shared.c
// BBi


#include <assert.h>
#include <math.h>

#include <algorithm>

#include "SDL.h"

#include "q_shared.h"
#include "qcommon.h"
#include "sys_events.h"
#include "sys_local.h"
#include "sys_shared.h"
#include "rtcw_string.h"

extern int Sys_Milliseconds();

namespace {

float rtcw_trunc(float x)
{
	double result;
	static_cast<void>(modf(x, &result));
	return static_cast<float>(result);
}

} // namespace

void Sys_SnapVector(
	float* v)
{
	v[0] = rtcw_trunc(v[0]);
	v[1] = rtcw_trunc(v[1]);
	v[2] = rtcw_trunc(v[2]);
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

extern void Sys_Mkdir(const char* path);
extern char* Sys_Cwd();

const char* Sys_DefaultCDPath()
{
	return "";
}

const char* Sys_DefaultBasePath()
{
	return Sys_Cwd();
}

namespace {

const int MAX_FOUND_FILES = 0x1000;

void Sys_ListFilteredFiles(const char* basedir, const char* subdirs, const char* filter, char** list, int* numfiles)
{
	if (numfiles == NULL || *numfiles >= MAX_FOUND_FILES - 1)
	{
		return;
	}

	char search[MAX_OSPATH];

	if (subdirs[0] != '\0')
	{
		Com_sprintf(search, sizeof(search), "%s%c%s", basedir, PATH_SEP, subdirs);
	}
	else
	{
		Com_sprintf(search, sizeof(search), "%s", basedir);
	}

	SysDirHandle fdir;

	if ((fdir = sys_open_dir(search)) == NULL)
	{
		return;
	}

	const SysDirEntry* d;
	char newsubdirs[MAX_OSPATH];
	char filename[MAX_OSPATH];

	while ((d = sys_read_dir(fdir)) != NULL)
	{
		Com_sprintf(filename, sizeof(filename), "%s%c%s", search, PATH_SEP, d->name);

		if (d->is_dir)
		{
			if (Q_stricmp(d->name, ".") != 0 && Q_stricmp(d->name, "..") != 0)
			{
				if (subdirs[0] != '\0')
				{
					Com_sprintf(newsubdirs, sizeof(newsubdirs), "%s%c%s", subdirs, PATH_SEP, d->name);
				}
				else
				{
					Com_sprintf(newsubdirs, sizeof(newsubdirs), "%s", d->name);
				}

				Sys_ListFilteredFiles(basedir, newsubdirs, filter, list, numfiles);
			}
		}

		if (*numfiles >= MAX_FOUND_FILES - 1)
		{
			break;
		}

		Com_sprintf(filename, sizeof(filename), "%s%c%s", subdirs, PATH_SEP, d->name);

		if (!Com_FilterPath(filter, filename, qfalse))
		{
			continue;
		}

		list[*numfiles] = CopyString(filename);
		(*numfiles)++;
	}

	sys_close_dir(fdir);
}

bool strgtr(const char* s0, const char* s1)
{
	size_t l0 = strlen(s0);
	const size_t l1 = strlen(s1);

	if (l1 < l0)
	{
		l0 = l1;
	}

	for (size_t i = 0; i < l0; ++i)
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

} // namespace

char** Sys_ListFiles(
	const char* directory,
	const char* extension,
	const char* filter,
	int* numfiles,
	qboolean wantsubs)
{
	char* list[MAX_FOUND_FILES];

	if (filter != NULL)
	{
		int nfiles = 0;
		Sys_ListFilteredFiles(directory, "", filter, list, &nfiles);
		list[nfiles] = NULL;
		*numfiles = nfiles;

		if (nfiles == 0)
		{
			return NULL;
		}

		char** list_copy = static_cast<char**>(Z_Malloc((nfiles + 1) * sizeof(*list_copy)));
		std::copy(list, &list[nfiles], list_copy);
		list_copy[nfiles] = NULL;
		return list_copy;
	}

	qboolean dironly = wantsubs;

	if (extension == NULL)
	{
		extension = "";
	}

	if (extension[0] == '/' && extension[1] == '\0')
	{
		extension = "";
		dironly = qtrue;
	}

	int nfiles = 0;
	SysDirHandle fdir;

	if ((fdir = sys_open_dir(directory)) == NULL)
	{
		*numfiles = 0;
		return NULL;
	}

	const SysDirEntry* d;
	char search[MAX_OSPATH];

	while ((d = sys_read_dir(fdir)) != NULL)
	{
		Com_sprintf(search, sizeof(search), "%s%c%s", directory, PATH_SEP, d->name);

		if ((dironly && !d->is_dir) || (!dironly && d->is_dir))
		{
			continue;
		}

		if (extension[0] != '\0')
		{
			if (strlen(d->name) < strlen(extension) ||
				Q_stricmp(d->name + strlen(d->name) - strlen(extension), extension))
			{
				continue; // didn't match
			}
		}

		if (nfiles == MAX_FOUND_FILES - 1)
		{
			break;
		}

		list[nfiles] = CopyString(d->name);
		++nfiles;
	}

	list[nfiles] = NULL;
	sys_close_dir(fdir);

	// return a copy of the list
	*numfiles = nfiles;

	if (nfiles == 0)
	{
		return NULL;
	}

	char** list_copy = static_cast<char**>(Z_Malloc((nfiles + 1) * sizeof(*list_copy)));
	std::copy(list, &list[nfiles], list_copy);
	list_copy[nfiles] = NULL;

	bool flag;

	do
	{
		flag = false;

		for (int i = 1; i < nfiles; ++i)
		{
			if (strgtr(list_copy[i - 1], list_copy[i]))
			{
				char* temp = list_copy[i];
				list_copy[i] = list_copy[i - 1];
				list_copy[i - 1] = temp;
				flag = true;
			}
		}
	}
	while (flag);

	return list_copy;
}

void Sys_FreeFileList(
	char** list)
{
	if (list == NULL)
	{
		return;
	}

	for (int i = 0; list[i]; ++i)
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

	return NULL;
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
	static const rtcw::String bits(RTCW_ARCH_STRING);

	static const rtcw::String game(
#ifdef RTCW_SP
		""
#else
		"_mp_"
#endif
	);

	static const rtcw::String demo(
#ifndef WOLF_SP_DEMO
		""
#else
		"_d"
#endif
	);

	static const rtcw::String ext(
#ifdef __WIN32__
		".dll"
#else
		".so"
#endif
	);

	static rtcw::String buffer;

	buffer = name + game + bits + demo + ext;

	return buffer.c_str();
}

void* QDECL Sys_LoadDll(
	const char* name,
	char* fqpath,
	DllEntryPoint* entryPoint,
	DllEntryPoint systemcalls)
{
	typedef void (QDECL * DllEntry)(DllEntryPoint);

	*fqpath = '\0'; // added 2/15/02 by T.Ray

	const char* basepath = Cvar_VariableString("fs_basepath");
	const char* cdpath = Cvar_VariableString("fs_cdpath");
	const char* gamedir = Cvar_VariableString("fs_game");

	const rtcw::String filename(Sys_GetDLLName(name));

	// try gamepath first
	char* fn = FS_BuildOSPath(basepath, gamedir, filename.c_str());

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
		if (!FS_CL_ExtractFromPakFile(fn, gamedir, filename.c_str(), NULL))
		{
			Com_Error(
				ERR_DROP,
				"Game code(%s) failed Pure Server check",
				filename.c_str());
		}
	}
#endif
#endif // RTCW_XX

	void* libHandle = SDL_LoadObject(fn);

	if (libHandle == NULL)
	{
		if (cdpath[0] != '\0')
		{
			fn = FS_BuildOSPath(cdpath, gamedir, filename.c_str());
			libHandle = SDL_LoadObject(fn);
		}
	}

	if (libHandle == NULL)
	{
		fn = FS_BuildOSPath(basepath, BASEGAME, filename.c_str());
		libHandle = SDL_LoadObject(fn);
	}

#if 0
	if (libHandle == NULL)
	{
		strcpy(fn, filename.c_str());
		libHandle = SDL_LoadObject(fn);
	}
#else
	if (libHandle == NULL)
	{
		fn = FS_BuildOSPath(basepath, ".", filename.c_str());
		libHandle = SDL_LoadObject(fn);
	}

	if (libHandle == NULL)
	{
		if (cdpath[0] != '\0')
		{
			fn = FS_BuildOSPath(cdpath, ".", filename.c_str());
			libHandle = SDL_LoadObject(fn);
		}
	}
#endif

	if (libHandle == NULL)
	{
		return NULL;
	}


	Q_strncpyz(fqpath, fn, MAX_QPATH); // added 2/15/02 by T.Ray

	const DllEntry dllEntry = reinterpret_cast<DllEntry>(SDL_LoadFunction(libHandle, "dllEntry"));
	*entryPoint = reinterpret_cast<DllEntryPoint>(SDL_LoadFunction(libHandle, "vmMain"));

	if ((*entryPoint) == NULL || dllEntry == NULL)
	{
		SDL_UnloadObject(libHandle);
		return NULL;
	}

	dllEntry(systemcalls);

	return libHandle;
}

const int MAX_QUED_EVENTS = 256;
const int MASK_QUED_EVENTS = MAX_QUED_EVENTS - 1;

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
	const char* s = Sys_ConsoleInput();

	if (s != NULL)
	{
		const int len = static_cast<int>(strlen(s)) + 1;
		char* b = static_cast<char*> (Z_Malloc(len));

		Q_strncpyz(b, s, len - 1);
		Sys_QueEvent(0, SE_CONSOLE, 0, 0, len, b);
	}

	// check for network packets
	MSG_Init(&netmsg, sys_packetReceived, sizeof(sys_packetReceived));

	if (Sys_GetPacket(&adr, &netmsg))
	{
		// copy out to a seperate buffer for qeueing
		// the readcount stepahead is for SOCKS support
		const int len = static_cast<int>(sizeof(netadr_t)) + netmsg.cursize - netmsg.readcount;
		netadr_t* buf = static_cast<netadr_t*>(Z_Malloc(len));

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

	memset(&ev, 0, sizeof(sysEvent_t));
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

	const int cpuid = CPUID_GENERIC;
	Cvar_SetValue("sys_cpuid", cpuid);

	Cvar_Get("sys_cpustring", "detect", 0);

	const char* cpu_string = Cvar_VariableString("sys_cpustring");

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
	const SDL_Keymod state = SDL_GetModState();
	return (state & KMOD_NUM) != 0;
}
#endif // RTCW_XX

const char* Sys_DefaultHomePath()
{
	return NULL;
}

const char* Sys_DefaultInstallPath()
{
	return Sys_Cwd();
}
