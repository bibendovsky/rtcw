cmake_minimum_required(VERSION 3.8.2 FATAL_ERROR)


# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#
# CPU architecture.
#
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	set (RTCW_ARCH_STRING "x86")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set (RTCW_ARCH_STRING "x64")
else ()
	unset (RTCW_ARCH_STRING)
endif ()

if (RTCW_ARCH_STRING)
	message (STATUS "[${PROJECT_NAME}] Arch: ${RTCW_ARCH_STRING}")
else ()
	message (FATAL_ERROR "[${PROJECT_NAME}] Unsupported platform.")
endif ()


# Precompiled headers.
#
option (RTCW_USE_PCH "Enable precompiled headers if available." ON)
message (STATUS "[${PROJECT_NAME}] Enable precompiled headers: ${RTCW_USE_PCH}")


# Multi-process compilation.
#
option (RTCW_MULTI_PROCESS_COMPILATION "Enable multi-process compilation if available." ON)
message (STATUS "[${PROJECT_NAME}] Enable multi-process compilation: ${RTCW_MULTI_PROCESS_COMPILATION}")

option (RTCW_CURL "Enable cURL." ON)
message (STATUS "[${PROJECT_NAME}] Enable cURL: ${RTCW_CURL}")

option (RTCW_CURL_STATIC "Link cURL statically." ON)
message (STATUS "[${PROJECT_NAME}] Link cURL statically: ${RTCW_CURL_STATIC}")


# Static linking.
#
option (RTCW_ENABLE_STATIC_LINKING "Enable static linking." ON)
message (STATUS "[${PROJECT_NAME}] Enable static linking: ${RTCW_ENABLE_STATIC_LINKING}")

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

if (RTCW_ENABLE_STATIC_LINKING)
	set(RTCW_TMP_SDL2_COMPONENTS "static")
else ()
	set(RTCW_TMP_SDL2_COMPONENTS "")
endif ()

find_package(SDL2W 2.0.4 REQUIRED COMPONENTS ${RTCW_TMP_SDL2_COMPONENTS})

find_package (SDL2W_net 2.0.1 REQUIRED)

if (RTCW_CURL)
	find_package (CURL 7.0.0 REQUIRED)
endif ()

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

function (rtcw_configure_target)
	if (NOT (${ARGC} EQUAL 1))
		message (FATAL_ERROR "Usage: rtcw_configure_target <target_name>")
	endif ()

	set_target_properties (
		${ARGV0}
		PROPERTIES
			CXX_STANDARD 98
			CXX_STANDARD_REQUIRED ON
			CXX_EXTENSIONS OFF
	)

	get_target_property (RTCW_TMP_TARGET_TYPE ${ARGV0} TYPE)

	unset(RTCW_TMP_IS_EXECUTABLE)

	if (RTCW_TMP_TARGET_TYPE STREQUAL "EXECUTABLE")
		set(RTCW_TMP_IS_EXECUTABLE TRUE)

		if (WIN32)
			set_target_properties (
				${ARGV0}
				PROPERTIES
					WIN32_EXECUTABLE TRUE
			)

			if (MSVC)
				set_target_properties (
					${ARGV0}
					PROPERTIES
						LINK_FLAGS -STACK:8388608
				)
			endif ()
		endif ()
	endif ()

	target_compile_definitions (
		${ARGV0}
		PRIVATE
			__STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS # stdint.h
			__STDC_FORMAT_MACROS # inttypes.h
			"RTCW_ARCH_STRING=\"${RTCW_ARCH_STRING}\""
	)

	if (MSVC)
		target_compile_definitions (
			${ARGV0}
			PRIVATE
				_CRT_SECURE_NO_WARNINGS
		)
	endif ()

	if (RTCW_CURL AND RTCW_CURL_STATIC)
		target_compile_definitions (
			${ARGV0}
			PRIVATE
				CURL_STATICLIB
		)
	endif ()

	if (RTCW_MULTI_PROCESS_COMPILATION)
		if (MSVC)
			target_compile_options (
				${ARGV0}
				PRIVATE
					-MP
			)
		endif ()
	endif ()

	if (RTCW_ENABLE_STATIC_LINKING)
		if (MSVC)
			target_compile_options(
				${ARGV0}
				PRIVATE
					$<$<CONFIG:DEBUG>:-MTd>
					$<$<NOT:$<CONFIG:DEBUG>>:-MT>
			)
		endif ()
	endif ()

	if (RTCW_USE_PCH AND NOT (${CMAKE_VERSION} VERSION_LESS "3.16"))
		target_precompile_headers(
			${ARGV0}
			PRIVATE
				rtcw_pch.h
		)
	endif ()

	if (RTCW_CURL)
		if (WIN32)
			target_link_libraries (
				${ARGV0}
				PRIVATE
					crypt32
					wldap32
			)
		endif ()

		target_link_libraries (
			${ARGV0}
			PRIVATE
				CURL::libcurl
		)
	endif ()

	if (RTCW_ENABLE_STATIC_LINKING)
		if (WIN32)
			target_link_libraries (
				${ARGV0}
				PRIVATE
					ws2_32
					iphlpapi
			)
		endif ()
	endif ()

	target_link_libraries (
		${ARGV0}
		PRIVATE
			$<$<BOOL:${RTCW_TMP_IS_EXECUTABLE}>:SDL2W::SDL2Wmain>
			SDL2W::SDL2W
			SDL2W_net::SDL2W_net
			rtcw_lib_miniz
			$<$<BOOL:${WIN32}>:winmm>
	)
endfunction (rtcw_configure_target)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

function (rtcw_configure_3rd_party_target)
	if (NOT (${ARGC} EQUAL 1))
		message (FATAL_ERROR "Usage: rtcw_configure_3rd_party_target <target_name>")
	endif ()

	set_target_properties (
		${ARGV0}
		PROPERTIES
			CXX_STANDARD 98
			CXX_STANDARD_REQUIRED ON
			CXX_EXTENSIONS OFF
	)

	if (MSVC)
		target_compile_definitions (
			${ARGV0}
			PRIVATE
				_CRT_SECURE_NO_WARNINGS
		)
	endif ()

	if (RTCW_MULTI_PROCESS_COMPILATION)
		if (MSVC)
			target_compile_options (
				${ARGV0}
				PRIVATE
					-MP
			)
		endif ()
	endif ()

	if (RTCW_ENABLE_STATIC_LINKING)
		if (MSVC)
			target_compile_options(
				${ARGV0}
				PRIVATE
					$<$<CONFIG:DEBUG>:-MTd>
					$<$<NOT:$<CONFIG:DEBUG>>:-MT>
			)
		endif ()
	endif ()
endfunction (rtcw_configure_3rd_party_target)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>