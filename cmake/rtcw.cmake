cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#
# CPU architecture.
#
if(WIN32 AND NOT DEFINED RTCW_ARCH_X86_32 AND NOT DEFINED RTCW_ARCH_X86_64)
	try_compile(RTCW_ARCH_X86_32
		${CMAKE_CURRENT_BINARY_DIR}/cmake/try_compile
		${CMAKE_SOURCE_DIR}/cmake/try_compile/is_x86_32.cpp
		OUTPUT_VARIABLE RTCW_TRY_COMPILE_OUTPUT_RESULT
	)

	if(RTCW_ARCH_X86_32)
		set(RTCW_ARCH_X86_32 TRUE CACHE INTERNAL "Architecture x86-32.")
	else()
		set(RTCW_ARCH_X86_32 FALSE CACHE INTERNAL "Architecture x86-32.")
	endif()

	try_compile(RTCW_ARCH_X86_64
		${CMAKE_CURRENT_BINARY_DIR}/cmake/try_compile
		${CMAKE_SOURCE_DIR}/cmake/try_compile/is_x86_64.cpp
		OUTPUT_VARIABLE RTCW_TRY_COMPILE_OUTPUT_RESULT
	)

	if(RTCW_ARCH_X86_64)
		set(RTCW_ARCH_X86_64 TRUE CACHE INTERNAL "Architecture x86-64.")
	else()
		set(RTCW_ARCH_X86_64 FALSE CACHE INTERNAL "Architecture x86-64.")
	endif()
endif()

set(RTCW_ARCH_STRING "")

if(RTCW_ARCH_X86_32)
	set(RTCW_ARCH_STRING "x86")
elseif(RTCW_ARCH_X86_64)
	set(RTCW_ARCH_STRING "x64")
endif()

message(STATUS "[${PROJECT_NAME}] Architecture: ${RTCW_ARCH_STRING}")

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

if(NOT RTCW_BUILD_SDL2)
	if(RTCW_ENABLE_STATIC_LINKING)
		set(RTCW_TMP_SDL2_COMPONENTS "static")
		set(RTCW_TMP_SDL2_NET_COMPONENTS "static")
	else()
		set(RTCW_TMP_SDL2_COMPONENTS "")
		set(RTCW_TMP_SDL2_NET_COMPONENTS "")
	endif()

	find_package(SDL2W 2.0.4 REQUIRED COMPONENTS ${RTCW_TMP_SDL2_COMPONENTS})
	find_package(SDL2W_net 2.0.1 REQUIRED COMPONENTS ${RTCW_TMP_SDL2_NET_COMPONENTS})
endif()

if(RTCW_CURL)
	find_package(CURL 7.0.0 REQUIRED)
endif()

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

function(rtcw_configure_target)
	if(NOT (${ARGC} EQUAL 1))
		message(FATAL_ERROR "Usage: rtcw_configure_target <target_name>")
	endif()

	set_target_properties(${ARGV0} PROPERTIES
		CXX_STANDARD 98
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
	)

	if(NOT ((MSVC AND (MSVC_VERSION LESS 1600)) OR (CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom")))
		set_target_properties(${ARGV0} PROPERTIES
			COMPILE_FEATURES cxx_long_long_type
		)
	endif()

	get_target_property(RTCW_TMP_TARGET_TYPE ${ARGV0} TYPE)

	unset(RTCW_TMP_IS_EXECUTABLE)

	get_target_property(RTCW_TMP_LINK_FLAGS ${ARGV0} LINK_FLAGS)

	if(NOT RTCW_TMP_LINK_FLAGS)
		set(RTCW_TMP_LINK_FLAGS "")
	endif()

	if(RTCW_TMP_TARGET_TYPE STREQUAL "EXECUTABLE")
		set(RTCW_TMP_IS_EXECUTABLE TRUE)

		if(WIN32)
			set_target_properties(${ARGV0} PROPERTIES
				WIN32_EXECUTABLE TRUE
			)
		endif()

		############
		# Stack size
		if(WIN32)
			if(MSVC)
				set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} /STACK:0x800000")
			elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
				set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} -Wl,--stack,0x800000")
			elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
				set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} -Wl,--stack,0x800000")
			elseif(CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom")
				set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} option stack=0x800000")
			endif()
		endif()

		##############################
		# Windows application manifest
		if(MSVC AND (NOT (MSVC_VERSION LESS 1400)))
			set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} /MANIFEST:NO")
		endif()
	endif()

	set_target_properties(${ARGV0} PROPERTIES LINK_FLAGS "${RTCW_TMP_LINK_FLAGS}")

	target_compile_definitions(${ARGV0}
		PRIVATE
			__STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS # stdint.h
			__STDC_FORMAT_MACROS # inttypes.h
			"RTCW_ARCH_STRING=\"${RTCW_ARCH_STRING}\""
	)

	if(RTCW_CURL AND RTCW_CURL_STATIC)
		target_compile_definitions(${ARGV0}
			PRIVATE
				CURL_STATICLIB
		)
	endif()

	if(RTCW_MULTI_PROCESS_COMPILATION)
		if(MSVC)
			target_compile_options(${ARGV0}
				PRIVATE
					-MP
			)
		endif()
	endif()

	if(RTCW_ENABLE_STATIC_LINKING)
		if(MSVC)
			target_compile_options(${ARGV0}
				PRIVATE
					$<$<CONFIG:DEBUG>:-MTd>
					$<$<NOT:$<CONFIG:DEBUG>>:-MT>
			)
		endif()
	endif()

	if(RTCW_USE_PCH AND NOT (${CMAKE_VERSION} VERSION_LESS "3.16"))
		target_precompile_headers(${ARGV0} PRIVATE rtcw_pch.h)
	endif()

	if(RTCW_CURL)
		if(WIN32)
			target_link_libraries(${ARGV0}
				PRIVATE
					crypt32
					wldap32
			)
		endif()

		target_link_libraries(${ARGV0}
			PRIVATE
				CURL::libcurl
		)
	endif()

	if(NOT RTCW_BUILD_SDL2)
		target_link_libraries(${ARGV0}
			PRIVATE
				$<$<BOOL:${RTCW_TMP_IS_EXECUTABLE}>:SDL2W::SDL2Wmain>
				$<$<BOOL:${RTCW_TMP_IS_EXECUTABLE}>:SDL2W::SDL2W>
				$<$<AND:$<BOOL:${RTCW_TMP_IS_EXECUTABLE}>,$<BOOL:${WIN32}>>:winmm>
			PRIVATE
				$<$<BOOL:${RTCW_TMP_IS_EXECUTABLE}>:SDL2W_net::SDL2W_net>
				$<$<AND:$<BOOL:${RTCW_TMP_IS_EXECUTABLE}>,$<BOOL:${WIN32}>>:iphlpapi>
				$<$<AND:$<BOOL:${RTCW_TMP_IS_EXECUTABLE}>,$<BOOL:${WIN32}>>:ws2_32>
		)
	endif()

	target_link_libraries(${ARGV0}
		PRIVATE
			rtcw::miniz
	)
endfunction(rtcw_configure_target)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

function(rtcw_configure_3rd_party_target)
	if(NOT (${ARGC} EQUAL 1))
		message(FATAL_ERROR "Usage: rtcw_configure_3rd_party_target <target_name>")
	endif()

	set_target_properties(${ARGV0} PROPERTIES
		CXX_STANDARD 98
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
	)

	if(RTCW_MULTI_PROCESS_COMPILATION)
		if(MSVC)
			target_compile_options(${ARGV0}
				PRIVATE
					-MP
			)
		endif()
	endif()

	if(RTCW_ENABLE_STATIC_LINKING)
		if(MSVC)
			target_compile_options(${ARGV0}
				PRIVATE
					$<$<CONFIG:DEBUG>:-MTd>
					$<$<NOT:$<CONFIG:DEBUG>>:-MT>
			)
		endif()
	endif()
endfunction(rtcw_configure_3rd_party_target)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>