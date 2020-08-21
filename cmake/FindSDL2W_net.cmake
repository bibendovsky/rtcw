#[[

CMake wrapper for SDL2_net module.


Copyright (c) 2020 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.


Virtual components:
    - static - uses static version of SDL2_net.

Required variables:
    - SDL2W_SDL2_NET_DIR - the directory with CMake files or
                           the directory with MSVC / MinGW development build.
                           Leave empty to search automatically.

Targets:
    - SDL2W_net::SDL2W_net - SDL2_net.

]]


cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)

set (SDL2W_NET_VERSION "1.0.0")
message (STATUS "[SDL2W_net] Version: ${SDL2W_NET_VERSION}")

set (SDL2W_NET_TMP_TARGET "${CMAKE_FIND_PACKAGE_NAME}::${CMAKE_FIND_PACKAGE_NAME}")

set (SDL2W_SDL2_NET_DIR "" CACHE PATH "The directory with CMake files or the directory with MSVC / MinGW development builds. Leave empty to figure out the location of SDL2_net.")

# Parse components.
#
set (SDL2W_NET_TMP_USE_STATIC FALSE)

if (${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
	message (STATUS "[SDL2W_net] Selected components:")

	foreach (SDL2W_NET_TMP_COMP ${${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS})
		message (STATUS "[SDL2W_net]     \"${SDL2W_NET_TMP_COMP}\"")

		if (SDL2W_NET_TMP_COMP STREQUAL "static")
			set (SDL2W_NET_TMP_USE_STATIC TRUE)
		endif ()
	endforeach ()
else ()
	message (STATUS "[SDL2W_net] No components were selected.")
endif ()

unset (SDL2W_NET_TMP_FPHSA_NAME_MISMATCHED)

if (DEFINED FPHSA_NAME_MISMATCHED)
	set (SDL2W_NET_TMP_FPHSA_NAME_MISMATCHED ${FPHSA_NAME_MISMATCHED})
endif ()

set (FPHSA_NAME_MISMATCHED "TRUE")

include (FindPkgConfig)
pkg_search_module (SDL2_net QUIET SDL2_net>=2)

if (DEFINED SDL2W_NET_TMP_FPHSA_NAME_MISMATCHED)
	set (FPHSA_NAME_MISMATCHED ${SDL2W_NET_TMP_FPHSA_NAME_MISMATCHED})
else ()
	unset (FPHSA_NAME_MISMATCHED)
endif ()

if (SDL2W_SDL2_NET_DIR)
	message (STATUS "[SDL2W_net] Custom path: ${SDL2W_SDL2_NET_DIR}")
endif ()


set (SDL2W_NET_TMP_SDL2_INC_DIRS "")
set (SDL2W_NET_TMP_SDL2_LINK_LIBS "")


#
# Check for Visual C++ / MinGW development libraries.
#
set (SDL2W_NET_TMP_FOUND_DEV_LIBS FALSE)
set (SDL2W_NET_TMP_FOUND_MSVC_LIBS FALSE)
set (SDL2W_NET_TMP_FOUND_MINGW_LIBS FALSE)

if (WIN32 AND SDL2W_SDL2_NET_DIR)
	set (SDL2W_NET_TMP_MSVC_INCLUDE_DIR ${SDL2W_SDL2_NET_DIR}/include)

	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set (SDL2W_NET_TMP_MSVC_LIBRARIES_DIR ${SDL2W_SDL2_NET_DIR}/lib/x64)

		set (SDL2W_NET_TMP_MINGW_CPU_ARCH x86_64-w64-mingw32)
		set (SDL2W_NET_TMP_MINGW_INCLUDE_DIR ${SDL2W_SDL2_NET_DIR}/${SDL2W_NET_TMP_MINGW_CPU_ARCH}/include/SDL2)
		set (SDL2W_NET_TMP_MINGW_LIBRARIES_DIR ${SDL2W_SDL2_NET_DIR}/${SDL2W_NET_TMP_MINGW_CPU_ARCH}/lib)
	elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set (SDL2W_NET_TMP_MSVC_LIBRARIES_DIR ${SDL2W_SDL2_NET_DIR}/lib/x86)

		set (SDL2W_NET_TMP_MINGW_CPU_ARCH i686-w64-mingw32)
		set (SDL2W_NET_TMP_MINGW_INCLUDE_DIR ${SDL2W_SDL2_NET_DIR}/${SDL2W_NET_TMP_MINGW_CPU_ARCH}/include/SDL2)
		set (SDL2W_NET_TMP_MINGW_LIBRARIES_DIR ${SDL2W_SDL2_NET_DIR}/${SDL2W_NET_TMP_MINGW_CPU_ARCH}/lib)
	else ()
		message (FATAL_ERROR "[SDL2W_net] Unsupported CPU architecture.")
	endif ()

	set (SDL2W_NET_TMP_MSVC_SDL_NET_H ${SDL2W_NET_TMP_MSVC_INCLUDE_DIR}/SDL_net.h)
	set (SDL2W_NET_TMP_MSVC_SDL2_NET_LIB ${SDL2W_NET_TMP_MSVC_LIBRARIES_DIR}/SDL2_net.lib)

	set (SDL2W_NET_TMP_MINGW_SDL_H ${SDL2W_NET_TMP_MINGW_INCLUDE_DIR}/SDL_net.h)
	set (SDL2W_NET_TMP_MINGW_LIBSDL2_NET_A ${SDL2W_NET_TMP_MINGW_LIBRARIES_DIR}/libSDL2_net.a)
	set (SDL2W_NET_TMP_MINGW_LIBSDL2_NET_DLL_A ${SDL2W_NET_TMP_MINGW_LIBRARIES_DIR}/libSDL2_net.dll.a)

	if (EXISTS ${SDL2W_NET_TMP_MSVC_SDL_NET_H} AND
		EXISTS ${SDL2W_NET_TMP_MSVC_SDL2_NET_LIB}
		)
		message (STATUS "[SDL2W_net] Found Visual C++ development libraries.")

		set (SDL2W_NET_TMP_FOUND_DEV_LIBS TRUE)
		set (SDL2W_NET_TMP_FOUND_MSVC_LIBS TRUE)
	elseif (EXISTS ${SDL2W_NET_TMP_MINGW_SDL_H} AND
		EXISTS ${SDL2W_NET_TMP_MINGW_LIBSDL2_NET_A} AND
		EXISTS ${SDL2W_NET_TMP_MINGW_LIBSDL2_NET_DLL_A}
		)
		message (STATUS "[SDL2W_net] Found MinGW development libraries.")

		set (SDL2W_NET_TMP_FOUND_DEV_LIBS TRUE)
		set (SDL2W_NET_TMP_FOUND_MINGW_LIBS TRUE)
	endif ()
endif ()

set (SDL2W_NET_TMP_FOUND_CONFIG FALSE)

if (NOT SDL2W_NET_TMP_FOUND_DEV_LIBS)
	if (SDL2_net_FOUND)
		message (STATUS "[SDL2W_net] Found pkg config.")

		set (SDL2W_NET_TMP_FOUND_CONFIG TRUE)
	else ()
		message (FATAL_ERROR "[SDL2W_net] Pkg config not found.")
	endif ()
endif ()


set (SDL2W_NET_TMP_VERSION_STRING "")

# Get library locations.
#
if (SDL2W_NET_TMP_FOUND_CONFIG)
	if (SDL2_net_LIBRARY_DIRS)
		message (STATUS "[SDL2W_net] Library directories:")

		foreach (SDL2W_NET_TMP_LOCATION IN LISTS SDL2_net_LIBRARY_DIRS)
			message (STATUS "[SDL2W_net]     \"${SDL2W_NET_TMP_LOCATION}\"")
		endforeach ()
	else ()
		message (STATUS "[SDL2W_net] No library directories.")
	endif ()
endif ()

message (STATUS "[SDL2W_net] Libraries:")

if (SDL2W_NET_TMP_FOUND_CONFIG)
	if (MINGW)
		if (SDL2W_NET_TMP_USE_STATIC)
			set (SDL2W_NET_TMP_LOCATION ${SDL2_net_LIBRARY_DIRS}/libSDL2_net.a)
		else ()
			set (SDL2W_NET_TMP_LOCATION ${SDL2_net_LIBRARY_DIRS}/libSDL2_net.dll.a)
		endif ()

		message (STATUS "[SDL2W_net]     \"${SDL2W_NET_TMP_LOCATION}\"")
		list (APPEND SDL2W_NET_TMP_SDL2_LINK_LIBS ${SDL2W_NET_TMP_LOCATION})
	else ()
		foreach (SDL2W_NET_TMP_LOCATION IN LISTS SDL2_net_LIBRARIES)
			message (STATUS "[SDL2W_net]     \"${SDL2W_NET_TMP_LOCATION}\"")
		endforeach ()

		list (APPEND SDL2W_NET_TMP_SDL2_LINK_LIBS ${SDL2_net_LIBRARIES})
	endif ()
elseif (SDL2W_NET_TMP_FOUND_MSVC_LIBS)
	message (STATUS "[SDL2W_net]     \"${SDL2W_NET_TMP_MSVC_SDL2_NET_LIB}\"")
	list (APPEND SDL2W_NET_TMP_SDL2_LINK_LIBS ${SDL2W_NET_TMP_MSVC_SDL2_NET_LIB})
elseif (SDL2W_NET_TMP_FOUND_MINGW_LIBS)
	if (SDL2W_NET_TMP_USE_STATIC)
		message (STATUS "[SDL2W_net]     \"${SDL2W_NET_TMP_MINGW_LIBSDL2_NET_A}\"")
		list (APPEND SDL2W_NET_TMP_SDL2_LINK_LIBS ${SDL2W_NET_TMP_MINGW_LIBSDL2_NET_A})
	else ()
		message (STATUS "[SDL2W_net]     \"${SDL2W_NET_TMP_MINGW_LIBSDL2_NET_DLL_A}\"")
		list (APPEND SDL2W_NET_TMP_SDL2_LINK_LIBS ${SDL2W_NET_TMP_MINGW_LIBSDL2_NET_DLL_A})
	endif ()
else ()
	message (FATAL_ERROR "[SDL2W_net] Unsupported configuration.")
endif ()


# Get include directories.
#
if (SDL2W_NET_TMP_FOUND_CONFIG)
	set (SDL2W_NET_TMP_SDL2_INC_DIRS ${SDL2_net_INCLUDE_DIRS})
elseif (SDL2W_NET_TMP_FOUND_MSVC_LIBS)
	set (SDL2W_NET_TMP_SDL2_INC_DIRS ${SDL2W_NET_TMP_MSVC_INCLUDE_DIR})
elseif (SDL2W_NET_TMP_FOUND_MINGW_LIBS)
	set (SDL2W_NET_TMP_SDL2_INC_DIRS ${SDL2W_NET_TMP_MINGW_INCLUDE_DIR})
else ()
	message (FATAL_ERROR "[SDL2W_net] Include directories not found.")
endif ()


# Find header file with version.
#
if (NOT SDL2W_NET_TMP_SDL2_INC_DIRS)
	message (FATAL_ERROR "[SDL2W_net] Empty include dirs list.")
endif ()

message (STATUS "[SDL2W_net] Include directories:")

foreach (SDL2W_NET_TMP_INCLUDE_DIR IN LISTS SDL2W_NET_TMP_SDL2_INC_DIRS)
	message (STATUS "[SDL2W_net]    \"${SDL2W_NET_TMP_INCLUDE_DIR}\"")
endforeach ()

foreach (SDL2W_NET_TMP_INCLUDE_DIR IN LISTS SDL2W_NET_TMP_SDL2_INC_DIRS)
	set (SDL2W_NET_TMP_SDL_NET_H ${SDL2W_NET_TMP_INCLUDE_DIR}/SDL_net.h)

	# Convert POSIX path to Windows one (MinGW)
	#
	if (MINGW)
		execute_process (
			COMMAND "sh" "-c" "cmd //c echo ${SDL2W_NET_TMP_SDL_NET_H}"
			TIMEOUT 7
			OUTPUT_VARIABLE SDL2W_NET_TMP_SDL_NET_H
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	endif ()

	# Extract version.
	#
	if (EXISTS ${SDL2W_NET_TMP_SDL_NET_H})
		set (
			SDL2W_NET_TMP_MAJOR_REGEX
			"^#define[ \t]+SDL_NET_MAJOR_VERSION[ \t]+([0-9]+)$"
		)

		set (
			SDL2W_NET_TMP_MINOR_REGEX
			"^#define[ \t]+SDL_NET_MINOR_VERSION[ \t]+([0-9]+)$"
		)

		set (
			SDL2W_NET_TMP_PATCH_REGEX
			"^#define[ \t]+SDL_NET_PATCHLEVEL[ \t]+([0-9]+)$"
		)

		file (
			STRINGS
			${SDL2W_NET_TMP_SDL_NET_H}
			SDL2W_NET_TMP_MAJOR_VERSION_STRING
			REGEX ${SDL2W_NET_TMP_MAJOR_REGEX}
		)

		file (
			STRINGS
			${SDL2W_NET_TMP_SDL_NET_H}
			SDL2W_NET_TMP_MINOR_VERSION_STRING
			REGEX ${SDL2W_NET_TMP_MINOR_REGEX}
		)

		file (
			STRINGS
			${SDL2W_NET_TMP_SDL_NET_H}
			SDL2W_NET_TMP_PATCH_VERSION_STRING
			REGEX ${SDL2W_NET_TMP_PATCH_REGEX}
		)

		string (
			REGEX REPLACE
			${SDL2W_NET_TMP_MAJOR_REGEX}
			"\\1"
			SDL2W_NET_TMP_MAJOR_VERSION
			${SDL2W_NET_TMP_MAJOR_VERSION_STRING}
		)

		string (
			REGEX REPLACE
			${SDL2W_NET_TMP_MINOR_REGEX}
			"\\1"
			SDL2W_NET_TMP_MINOR_VERSION
			${SDL2W_NET_TMP_MINOR_VERSION_STRING}
		)

		string (
			REGEX REPLACE
			${SDL2W_NET_TMP_PATCH_REGEX}
			"\\1"
			SDL2W_NET_TMP_PATCH_VERSION
			${SDL2W_NET_TMP_PATCH_VERSION_STRING}
		)

		set (
			SDL2W_NET_TMP_DIGIT_REGEX
			"^[0-9]+$"
		)

		if (SDL2W_NET_TMP_MAJOR_VERSION MATCHES ${SDL2W_NET_TMP_DIGIT_REGEX} AND
			SDL2W_NET_TMP_MINOR_VERSION MATCHES ${SDL2W_NET_TMP_DIGIT_REGEX} AND
			SDL2W_NET_TMP_PATCH_VERSION MATCHES ${SDL2W_NET_TMP_DIGIT_REGEX}
			)
			if (NOT ${SDL2W_NET_TMP_MAJOR_VERSION} EQUAL 2)
				message (FATAL_ERROR "[SDL2W_net] Unsupported major version (got: ${SDL2W_NET_TMP_MAJOR_VERSION}; expected: 2).")
			endif ()

			set (
				SDL2W_NET_TMP_VERSION_STRING
				${SDL2W_NET_TMP_MAJOR_VERSION}.${SDL2W_NET_TMP_MINOR_VERSION}.${SDL2W_NET_TMP_PATCH_VERSION}
			)
		endif ()

		break ()
	endif ()
endforeach ()

message (STATUS "[SDL2W_net] Found SDL2_net version: ${SDL2W_NET_TMP_VERSION_STRING}")


# Default handler.
#
include (FindPackageHandleStandardArgs)

set (SDL2W_NET_TMP_REQUIRED_VARS "")

if (SDL2W_NET_TMP_FOUND_TARGETS)
	set (SDL2W_NET_TMP_REQUIRED_VARS SDL2_net_FOUND)
elseif (SDL2W_NET_TMP_FOUND_CONFIG)
	list (APPEND SDL2W_NET_TMP_REQUIRED_VARS SDL2_net_INCLUDE_DIRS)
	list (APPEND SDL2W_NET_TMP_REQUIRED_VARS SDL2_net_LIBRARIES)
elseif (SDL2W_NET_TMP_FOUND_MSVC_LIBS)
	list (APPEND SDL2W_NET_TMP_REQUIRED_VARS SDL2W_NET_TMP_MSVC_INCLUDE_DIR)
	list (APPEND SDL2W_NET_TMP_REQUIRED_VARS SDL2W_NET_TMP_MSVC_SDL2_NET_LIB)
elseif (SDL2W_NET_TMP_FOUND_MINGW_LIBS)
	list (APPEND SDL2W_NET_TMP_REQUIRED_VARS SDL2W_NET_TMP_MINGW_INCLUDE_DIR)
	list (APPEND SDL2W_NET_TMP_REQUIRED_VARS SDL2W_NET_TMP_MINGW_LIBSDL2_NET_A)
	list (APPEND SDL2W_NET_TMP_REQUIRED_VARS SDL2W_NET_TMP_MINGW_LIBSDL2_NET_DLL_A)
else ()
	message (FATAL_ERROR "[SDL2W_net] Invalid configuration.")
endif ()

find_package_handle_standard_args (
	${CMAKE_FIND_PACKAGE_NAME}
	REQUIRED_VARS
		${SDL2W_NET_TMP_REQUIRED_VARS}
	VERSION_VAR
		SDL2W_NET_TMP_VERSION_STRING
)

if (WIN32 AND SDL2W_NET_TMP_USE_STATIC)
	list (APPEND SDL2W_NET_TMP_SDL2_LINK_LIBS "iphlpapi")
	list (APPEND SDL2W_NET_TMP_SDL2_LINK_LIBS "ws2_32")
endif ()

# Add target.
#
if (NOT TARGET ${SDL2W_NET_TMP_TARGET})
	if (SDL2W_NET_TMP_FOUND_CONFIG AND SDL2_net_LIBRARY_DIRS AND NOT MINGW)
		link_directories (${SDL2_net_LIBRARY_DIRS})
	endif ()

	add_library (${SDL2W_NET_TMP_TARGET} INTERFACE IMPORTED)

	set_target_properties (
		${SDL2W_NET_TMP_TARGET}
		PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${SDL2W_NET_TMP_SDL2_INC_DIRS}"
	)

	set_target_properties (
		${SDL2W_NET_TMP_TARGET}
		PROPERTIES
			INTERFACE_LINK_LIBRARIES "${SDL2W_NET_TMP_SDL2_LINK_LIBS}"
	)
endif ()
