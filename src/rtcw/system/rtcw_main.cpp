/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// Executable's entry point.

#ifdef _WIN32

#define RTCW_MAIN_IMPL

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include "rtcw_main.h"

// ==========================================================================

namespace {

template<typename T>
void rtcw_ignore_result(const T&) {}

// ==========================================================================

template<typename T, typename TDeleter>
class RtcwUniquePtr
{
public:
	explicit RtcwUniquePtr(T* pointer, const TDeleter& deleter);
	~RtcwUniquePtr();

	T* get() const;
	void reset();

private:
	RtcwUniquePtr(const RtcwUniquePtr&);
	RtcwUniquePtr& operator=(const RtcwUniquePtr&);

private:
	T* pointer_;
	TDeleter deleter_;
};

// --------------------------------------------------------------------------

template<typename T, typename TDeleter>
RtcwUniquePtr<T, TDeleter>::RtcwUniquePtr(T* pointer, const TDeleter& deleter)
	:
	pointer_(pointer),
	deleter_(deleter)
{}

template<typename T, typename TDeleter>
RtcwUniquePtr<T, TDeleter>::~RtcwUniquePtr()
{
	reset();
}

template<typename T, typename TDeleter>
T* RtcwUniquePtr<T, TDeleter>::get() const
{
	return pointer_;
}

template<typename T, typename TDeleter>
void RtcwUniquePtr<T, TDeleter>::reset()
{
	if (pointer_ == NULL)
	{
		return;
	}

	deleter_(pointer_);
	pointer_ = NULL;
}

// ==========================================================================

class RtcwWArgvUPtrDeleter
{
public:
	void operator()(wchar_t** pointer) const;
};

// --------------------------------------------------------------------------

void RtcwWArgvUPtrDeleter::operator()(wchar_t** pointer) const
{
	if (pointer == NULL)
	{
		return;
	}

	const HLOCAL result = LocalFree(pointer);
	assert(result == NULL);
	rtcw_ignore_result(result);
}

// ==========================================================================

typedef RtcwUniquePtr<wchar_t*, RtcwWArgvUPtrDeleter> RtcwWArgvUPtr;

// ==========================================================================

class RtcwHeapUPtrDeleter
{
public:
	RtcwHeapUPtrDeleter(HANDLE heap_handle);

	void operator()(void* pointer) const;

private:
	HANDLE heap_handle_;
};

// --------------------------------------------------------------------------

RtcwHeapUPtrDeleter::RtcwHeapUPtrDeleter(HANDLE heap_handle)
	:
	heap_handle_(heap_handle)
{}

void RtcwHeapUPtrDeleter::operator()(void* pointer) const
{
	if (pointer == NULL)
	{
		return;
	}

	assert(heap_handle_ != NULL);
	const BOOL win32_result = HeapFree(heap_handle_, 0, pointer);
	assert(win32_result == TRUE);
	rtcw_ignore_result(win32_result);
}

// ==========================================================================

typedef RtcwUniquePtr<void, RtcwHeapUPtrDeleter> RtcwHeapUPtr;

// ==========================================================================

class RtcwEntryPoint
{
public:
	int invoke();

private:
	HANDLE stderr_handle_;

private:
	int get_utf16_to_utf8_size_with_null(LPCWSTR u16_string) const;
	void log_and_show_error_message(LPCWSTR message) const;
	void log_and_show_last_error_message() const;
	int invoke(LPWSTR lpCmdLine);
};

// --------------------------------------------------------------------------

int RtcwEntryPoint::invoke()
{
	return invoke(GetCommandLineW());
}

int RtcwEntryPoint::get_utf16_to_utf8_size_with_null(LPCWSTR u16_string) const
{
	return WideCharToMultiByte(CP_UTF8, 0, u16_string, -1, NULL, 0, NULL, NULL);
}

void RtcwEntryPoint::log_and_show_error_message(LPCWSTR message) const
{
	const wchar_t* title_message = L"[RTCW ENTRY POINT]";

	if (stderr_handle_ != INVALID_HANDLE_VALUE)
	{
		DWORD written_size;
		BOOL bool_result;

		const DWORD title_length = static_cast<DWORD>(lstrlenW(title_message));
		const DWORD title_size = title_length * 2;

		const DWORD message_length = static_cast<DWORD>(lstrlenW(message));
		const DWORD message_size = message_length * 2;

		// Prefix.
		//
		bool_result = WriteFile(stderr_handle_, title_message, title_size, &written_size, NULL);
		assert(bool_result == TRUE);
		assert(written_size == title_size);

		// Space.
		//
		bool_result = WriteFile(stderr_handle_, L" ", 2, &written_size, NULL);
		assert(bool_result == TRUE);
		assert(written_size == 2);

		// Message.
		//
		bool_result = WriteFile(stderr_handle_, message, message_size, &written_size, NULL);
		assert(bool_result == TRUE);
		assert(written_size == message_size);

		// Flush the buffers.
		//
		bool_result = FlushFileBuffers(stderr_handle_);
		assert(bool_result == TRUE);

		//
		rtcw_ignore_result(bool_result);
	}

	const int int_result = MessageBoxW(NULL, message, title_message, MB_OK | MB_ICONERROR);
	assert(int_result != 0);
	rtcw_ignore_result(int_result);
}

void RtcwEntryPoint::log_and_show_last_error_message() const
{
	const DWORD error_code = GetLastError();

	const DWORD max_message_chars = 2048;
	WCHAR message[max_message_chars];

	const DWORD format_message_w_result = FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		message,
		max_message_chars,
		NULL);

	if (format_message_w_result == 0)
	{
		lstrcpyW(message, L"Generic failure.");
	}

	log_and_show_error_message(message);
}

int RtcwEntryPoint::invoke(LPWSTR lpCmdLine)
{
	stderr_handle_ = GetStdHandle(STD_ERROR_HANDLE);

	// Convert command-line string into UTF-16 arguments.
	//
	int wargc = 0;
	RtcwWArgvUPtr wargv(CommandLineToArgvW(lpCmdLine, &wargc), RtcwWArgvUPtrDeleter());

	if (wargv.get() == NULL)
	{
		log_and_show_last_error_message();
		return EXIT_FAILURE;
	}

	// Calculate the size of UTF-8 arguments block.
	//
	const size_t args_ptrs_size = (static_cast<size_t>(wargc) + 1) * sizeof(void*); // argv[argc] == NULL
	size_t args_block_size = args_ptrs_size;

	for (int i = 0; i < wargc; ++i)
	{
		const int u8_size_with_null = get_utf16_to_utf8_size_with_null(wargv.get()[i]);

		if (u8_size_with_null == 0)
		{
			log_and_show_last_error_message();
			return EXIT_FAILURE;
		}

		args_block_size += static_cast<size_t>(u8_size_with_null);
	}

	// Allocate the UTF-8 arguments block.
	//
	HANDLE process_heap = GetProcessHeap();
	const RtcwHeapUPtr argv_block(HeapAlloc(process_heap, 0, args_block_size), RtcwHeapUPtrDeleter(process_heap));

	if (argv_block.get() == NULL)
	{
		log_and_show_last_error_message();
		return EXIT_FAILURE;
	}

	// Initialize the UTF-8 block.
	//
	char** const argv_ptrs_begin = static_cast<char**>(argv_block.get());
	char* const argv_args_begin = static_cast<char*>(argv_block.get()) + args_ptrs_size;

	char** argv_ptrs = argv_ptrs_begin;
	char* argv_args = argv_args_begin;

	for (int i = 0; i < wargc; ++i)
	{
		const int u8_size_with_null = get_utf16_to_utf8_size_with_null(wargv.get()[i]);

		if (u8_size_with_null == 0)
		{
			log_and_show_last_error_message();
			return EXIT_FAILURE;
		}

		// UTF-16 -> UTF-8
		const int u8_used_size = WideCharToMultiByte(
			CP_UTF8,
			0,
			wargv.get()[i],
			-1,
			argv_args,
			u8_size_with_null,
			NULL,
			NULL);

		if (u8_used_size != u8_size_with_null)
		{
			log_and_show_error_message(L"UTF-16 to UTF-8 conversion mismatch size.");
			return EXIT_FAILURE;
		}

		(*argv_ptrs++) = argv_args;
		argv_args += u8_size_with_null;
	}

	*argv_ptrs = NULL; // argv[argc] = null

	// Release a memory allocated by the CommandLineToArgvW.
	//
	wargv.reset();

	// Call the user's entry point.
	//
	return rtcw_main(wargc, argv_ptrs_begin);
}

} // namespace

// ==========================================================================

extern "C" int __cdecl rtcw_main_impl()
{
	return RtcwEntryPoint().invoke();
}

#endif // _WIN32
