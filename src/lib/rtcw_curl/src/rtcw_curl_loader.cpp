/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2025-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#include "rtcw_curl_loader.h"
#include <cstdarg>
#include <cstddef>
#include "curl/curl.h"

namespace rtcw {

namespace {

typedef char* (*IMPL_curl_version)();
IMPL_curl_version impl_curl_version = NULL;

typedef CURLcode (*IMPL_curl_global_init)(long flags);
IMPL_curl_global_init impl_curl_global_init = NULL;

typedef void (*IMPL_curl_global_cleanup)();
IMPL_curl_global_cleanup impl_curl_global_cleanup = NULL;

typedef CURL* (*IMPL_curl_easy_init)();
IMPL_curl_easy_init impl_curl_easy_init = NULL;

typedef void (*IMPL_curl_easy_cleanup)(CURL* curl);
IMPL_curl_easy_cleanup impl_curl_easy_cleanup = NULL;

typedef CURLcode (*IMPL_curl_easy_setopt)(CURL* curl, CURLoption option, ...);
IMPL_curl_easy_setopt impl_curl_easy_setopt = NULL;

typedef const char* (*IMPL_curl_easy_strerror)(CURLcode error);
IMPL_curl_easy_strerror impl_curl_easy_strerror = NULL;

typedef CURLM* (*IMPL_curl_multi_init)();
IMPL_curl_multi_init impl_curl_multi_init = NULL;

typedef CURLMcode (*IMPL_curl_multi_cleanup)(CURLM* multi_handle);
IMPL_curl_multi_cleanup impl_curl_multi_cleanup = NULL;

typedef CURLMcode (*IMPL_curl_multi_add_handle)(CURLM* multi_handle, CURL* curl_handle);
IMPL_curl_multi_add_handle impl_curl_multi_add_handle = NULL;

typedef CURLMcode (*IMPL_curl_multi_remove_handle)(CURLM* multi_handle, CURL* curl_handle);
IMPL_curl_multi_remove_handle impl_curl_multi_remove_handle = NULL;

typedef CURLMcode (*IMPL_curl_multi_perform)(CURLM* multi_handle, int* running_handles);
IMPL_curl_multi_perform impl_curl_multi_perform = NULL;

typedef CURLMsg* (*IMPL_curl_multi_info_read)(CURLM* multi_handle, int* msgs_in_queue);
IMPL_curl_multi_info_read impl_curl_multi_info_read = NULL;

// -------------------------------------

std::size_t global_curl_counter = 0;
char curl_null_version[] = "[RTCW] curl_version == NULL";

// -------------------------------------

template<typename TSymbol>
void curl_resolve_symbol(const char* symbol_name, TSymbol& symbol)
{
	void* const proc = global_curl_loader->get_proc_address(symbol_name);
	symbol = reinterpret_cast<TSymbol>(proc);
}

void curl_clear_symbols()
{
	impl_curl_version = NULL;
	impl_curl_global_init = NULL;
	impl_curl_global_cleanup = NULL;
	impl_curl_easy_init = NULL;
	impl_curl_easy_cleanup = NULL;
	impl_curl_easy_setopt = NULL;
	impl_curl_easy_strerror = NULL;
	impl_curl_multi_init = NULL;
	impl_curl_multi_cleanup = NULL;
	impl_curl_multi_add_handle = NULL;
	impl_curl_multi_remove_handle = NULL;
	impl_curl_multi_perform = NULL;
	impl_curl_multi_info_read = NULL;
}

void curl_resolve_symbols()
{
#define RTCW_MACRO(symbol) curl_resolve_symbol(#symbol, impl_##symbol)
	RTCW_MACRO(curl_version);
	RTCW_MACRO(curl_global_init);
	RTCW_MACRO(curl_global_cleanup);
	RTCW_MACRO(curl_easy_init);
	RTCW_MACRO(curl_easy_cleanup);
	RTCW_MACRO(curl_easy_setopt);
	RTCW_MACRO(curl_easy_strerror);
	RTCW_MACRO(curl_multi_init);
	RTCW_MACRO(curl_multi_cleanup);
	RTCW_MACRO(curl_multi_add_handle);
	RTCW_MACRO(curl_multi_remove_handle);
	RTCW_MACRO(curl_multi_perform);
	RTCW_MACRO(curl_multi_info_read);
#undef RTCW_MACRO
}

void curl_terminate()
{
	if (--global_curl_counter > 0)
	{
		return;
	}
	if (global_curl_loader != NULL)
	{
		global_curl_loader->unload_library();
	}
	curl_clear_symbols();
}

void curl_initialize()
{
	if (global_curl_counter++ > 0)
	{
		return;
	}
	if (global_curl_loader == NULL)
	{
		return;
	}
	if (!global_curl_loader->load_library())
	{
		return;
	}
	curl_resolve_symbols();
}

} // namespace

// =====================================

CurlLoader* global_curl_loader = NULL;

} // namespace rtcw

// ======================================

char* curl_version()
{
	if (rtcw::impl_curl_version == NULL)
	{
		return rtcw::curl_null_version;
	}
	return rtcw::impl_curl_version();
}

CURLcode curl_global_init(long flags)
{
	rtcw::curl_initialize();
	if (rtcw::impl_curl_global_init == NULL)
	{
		return CURLE_FAILED_INIT;
	}
	return rtcw::impl_curl_global_init(flags);
}

void curl_global_cleanup()
{
	if (rtcw::impl_curl_global_cleanup != NULL)
	{
		rtcw::impl_curl_global_cleanup();
	}
	rtcw::curl_terminate();
}

CURL* curl_easy_init()
{
	if (rtcw::impl_curl_easy_init == NULL)
	{
		return NULL;
	}
	return rtcw::impl_curl_easy_init();
}

void curl_easy_cleanup(CURL* curl)
{
	if (rtcw::impl_curl_easy_cleanup == NULL)
	{
		return;
	}
	rtcw::impl_curl_easy_cleanup(curl);
}

CURLcode curl_easy_setopt(CURL* curl, CURLoption option, ...)
{
	if (rtcw::impl_curl_easy_setopt == NULL)
	{
		return CURLE_FAILED_INIT;
	}

#define RTCW_MACRO(type)                   \
	std::va_list args;                     \
	va_start(args, option);                \
	type const value = va_arg(args, type); \
	va_end(args)

	switch (option)
	{
		case CURLOPT_FAILONERROR:
		case CURLOPT_NOPROGRESS:
			{
				RTCW_MACRO(long);
				return rtcw::impl_curl_easy_setopt(curl, option, value);
			}
		case CURLOPT_REFERER:
		case CURLOPT_URL:
		case CURLOPT_USERAGENT:
			{
				RTCW_MACRO(char*);
				return rtcw::impl_curl_easy_setopt(curl, option, value);
			}
		case CURLOPT_PROGRESSFUNCTION:
			{
				RTCW_MACRO(curl_progress_callback);
				return rtcw::impl_curl_easy_setopt(curl, option, value);
			}
		case CURLOPT_WRITEDATA:
			{
				RTCW_MACRO(void*);
				return rtcw::impl_curl_easy_setopt(curl, option, value);
			}
		case CURLOPT_WRITEFUNCTION:
			{
				RTCW_MACRO(curl_write_callback);
				return rtcw::impl_curl_easy_setopt(curl, option, value);
			}
		default:
			return CURLE_UNKNOWN_OPTION;
	}

#undef RTCW_MACRO
}

const char* curl_easy_strerror(CURLcode error)
{
	if (rtcw::impl_curl_easy_strerror == NULL)
	{
		return "[RTCW] curl_easy_strerror == NULL";
	}
	return rtcw::impl_curl_easy_strerror(error);
}

CURLM* curl_multi_init()
{
	if (rtcw::impl_curl_multi_init == NULL)
	{
		return NULL;
	}
	return rtcw::impl_curl_multi_init();
}

CURLMcode curl_multi_cleanup(CURLM* multi_handle)
{
	if (rtcw::impl_curl_multi_cleanup == NULL)
	{
		return CURLM_INTERNAL_ERROR;
	}
	return rtcw::impl_curl_multi_cleanup(multi_handle);
}

CURLMcode curl_multi_add_handle(CURLM* multi_handle, CURL* curl_handle)
{
	if (rtcw::impl_curl_multi_add_handle == NULL)
	{
		return CURLM_INTERNAL_ERROR;
	}
	return rtcw::impl_curl_multi_add_handle(multi_handle, curl_handle);
}

CURLMcode curl_multi_remove_handle(CURLM* multi_handle, CURL* curl_handle)
{
	if (rtcw::impl_curl_multi_remove_handle == NULL)
	{
		return CURLM_INTERNAL_ERROR;
	}
	return rtcw::impl_curl_multi_remove_handle(multi_handle, curl_handle);
}

CURLMcode curl_multi_perform(CURLM* multi_handle, int* running_handles)
{
	if (rtcw::impl_curl_multi_perform == NULL)
	{
		return CURLM_INTERNAL_ERROR;
	}
	return rtcw::impl_curl_multi_perform(multi_handle, running_handles);
}

CURLMsg* curl_multi_info_read(CURLM* multi_handle, int* msgs_in_queue)
{
	if (rtcw::impl_curl_multi_info_read == NULL)
	{
		return NULL;
	}
	return rtcw::impl_curl_multi_info_read(multi_handle, msgs_in_queue);
}
