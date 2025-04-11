/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// Executable's entry point.

#ifndef RTCW_MAIN_INCLUDED
#define RTCW_MAIN_INCLUDED

#ifndef RTCW_NO_MAIN
	#ifdef _WIN32
		#ifdef main
			#undef main
		#endif

		int rtcw_main(int argc, char* argv[]);
		extern "C" int __cdecl rtcw_main_impl();

		#ifndef RTCW_MAIN_IMPL
			#ifdef _MSC_VER
				#if defined(UNICODE) && UNICODE
					int wmain(int, wchar_t*[], wchar_t*)
					{
						return rtcw_main_impl();
					}
				#else // UNICODE
					int main(int, char*[])
					{
						return rtcw_main_impl();
					}
				#endif // UNICODE
			#endif // _MSC_VER

			#ifndef WINAPI
				#define WINAPI __stdcall
			#endif

			#ifndef DECLARE_HANDLE
				typedef void* HANDLE;

				#ifdef STRICT
					#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__* name
				#else // STRICT
					#define DECLARE_HANDLE(name) typedef HANDLE name
				#endif // STRICT
			#endif // DECLARE_HANDLE

			DECLARE_HANDLE(HINSTANCE);
			typedef char* LPSTR;
			typedef wchar_t* PWSTR;

			#if defined(UNICODE) && UNICODE
			int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
			#else // UNICODE
			int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
			#endif // UNICODE
			{
				return rtcw_main_impl();
			}

			#define main rtcw_main
		#endif // RTCW_MAIN_IMPL
	#endif // _WIN32
#endif // RTCW_NO_MAIN

#endif // RTCW_MAIN_INCLUDED
