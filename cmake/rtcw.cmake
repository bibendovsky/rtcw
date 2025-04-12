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

	get_target_property(RTCW_TMP_TAGS ${ARGV0} RTCW_TAGS)

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

	set(RTCW_TMP_IS_EXECUTABLE FALSE)

	get_target_property(RTCW_TMP_LINK_FLAGS ${ARGV0} LINK_FLAGS)

	if(NOT RTCW_TMP_LINK_FLAGS)
		set(RTCW_TMP_LINK_FLAGS "")
	endif()

	if("exe" IN_LIST RTCW_TMP_TAGS)
		set(RTCW_TMP_IS_EXECUTABLE TRUE)

		if(WIN32)
			set_target_properties(${ARGV0} PROPERTIES
				WIN32_EXECUTABLE TRUE
			)

			message(STATUS "[${ARGV0}] Set Windows subsystem.")
		endif()

		# ----------
		# Stack size
		if(WIN32)
			set(RTCW_TMP_EXE_STACK_SIZE "0x800000")
			set(RTCW_TMP_EXE_STACK_SIZE_IS_SET FALSE)

			if(MSVC)
				set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} /STACK:${RTCW_TMP_EXE_STACK_SIZE}")
				set(RTCW_TMP_EXE_STACK_SIZE_IS_SET TRUE)
			elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
				set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} -Wl,--stack,${RTCW_TMP_EXE_STACK_SIZE}")
				set(RTCW_TMP_EXE_STACK_SIZE_IS_SET TRUE)
			elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
				set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} -Wl,--stack,${RTCW_TMP_EXE_STACK_SIZE}")
				set(RTCW_TMP_EXE_STACK_SIZE_IS_SET TRUE)
			elseif(CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom")
				set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} option stack=${RTCW_TMP_EXE_STACK_SIZE}")
				set(RTCW_TMP_EXE_STACK_SIZE_IS_SET TRUE)
			endif()

			if(RTCW_TMP_EXE_STACK_SIZE_IS_SET)
				message(STATUS "[${ARGV0}] Set stack size: ${RTCW_TMP_EXE_STACK_SIZE}")
			endif()
		endif()

		# ----------------------------
		# Windows application manifest
		if(MSVC AND (NOT (MSVC_VERSION LESS 1400)))
			set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} /MANIFEST:NO")
		endif()

		add_custom_command(
			TARGET ${ARGV0} POST_BUILD
			COMMAND $<$<CONFIG:RELEASE>:${CMAKE_STRIP}> $<$<CONFIG:RELEASE>:$<TARGET_FILE:${ARGV0}>>
			COMMENT $<$<CONFIG:RELEASE>:"[${ARGV0}] Strip executable.">
			VERBATIM
		)

		# -------------
		# Add resources
		if(WIN32)
			target_sources(${ARGV0} PRIVATE ../win32/rtcw.rc)
			message(STATUS "[${ARGV0}] Add resource files.")

			if(CMAKE_CXX_COMPILER_ID STREQUAL OpenWatcom)
				set(RTCW_TMP_DEFINES "")

				if("sp" IN_LIST RTCW_TMP_TAGS)
					set(RTCW_TMP_DEFINES "RTCW_SP")
				elseif("mp" IN_LIST RTCW_TMP_TAGS)
					set(RTCW_TMP_DEFINES "RTCW_MP")
				elseif("et" IN_LIST RTCW_TMP_TAGS)
					set(RTCW_TMP_DEFINES "RTCW_ET")
				endif()

				if(RTCW_TMP_DEFINES)
					add_custom_command(
						TARGET ${ARGV0} POST_BUILD
						COMMAND wrc.exe
							"/D${RTCW_TMP_DEFINES}"
							/bt=nt
							/i=$<SHELL_PATH:${PROJECT_SOURCE_DIR}/../win32>
							/fo=$<SHELL_PATH:${PROJECT_BINARY_DIR}/rtcw.res>
							$<SHELL_PATH:${PROJECT_SOURCE_DIR}/../win32/rtcw.rc>
							$<SHELL_PATH:$<TARGET_FILE:${ARGV0}>>
						COMMENT "[${ARGV0}][OpenWatcom] Embed resources."
						VERBATIM
					)
				endif()
			endif()
		endif()
	endif()

	if("dll" IN_LIST RTCW_TMP_TAGS)
		if(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom"))
			message(STATUS "[${ARGV0}][OpenWatcom] Fix DLL exports.")
			set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} export dllEntry=_dllEntry,vmMain=_vmMain")
		endif()
	endif()

	set_target_properties(${ARGV0} PROPERTIES LINK_FLAGS "${RTCW_TMP_LINK_FLAGS}")

	# -----------------------------------------
	# Output name
	set(RTCW_TMP_OUTPUT_NAME "")

	if("dll" IN_LIST RTCW_TMP_TAGS)
		if("cgame" IN_LIST RTCW_TMP_TAGS)
			set(RTCW_TMP_OUTPUT_NAME "cgame")
		elseif("game" IN_LIST RTCW_TMP_TAGS)
			set(RTCW_TMP_OUTPUT_NAME "qagame")
		elseif("ui" IN_LIST RTCW_TMP_TAGS)
			set(RTCW_TMP_OUTPUT_NAME "ui")
		endif()

		if(RTCW_TMP_OUTPUT_NAME AND ("sp" IN_LIST RTCW_TMP_TAGS))
		elseif(RTCW_TMP_OUTPUT_NAME AND (("mp" IN_LIST RTCW_TMP_TAGS) OR ("et" IN_LIST RTCW_TMP_TAGS)))
			set(RTCW_TMP_OUTPUT_NAME "${RTCW_TMP_OUTPUT_NAME}_mp")
		endif()
	elseif("exe" IN_LIST RTCW_TMP_TAGS)
		if("sp" IN_LIST RTCW_TMP_TAGS)
			set(RTCW_TMP_OUTPUT_NAME "sp")
		elseif("mp" IN_LIST RTCW_TMP_TAGS)
			set(RTCW_TMP_OUTPUT_NAME "mp")
		elseif("et" IN_LIST RTCW_TMP_TAGS)
			set(RTCW_TMP_OUTPUT_NAME "et")
		endif()

		if(RTCW_TMP_OUTPUT_NAME AND ("dedicated" IN_LIST RTCW_TMP_TAGS))
			set(RTCW_TMP_OUTPUT_NAME "${RTCW_TMP_OUTPUT_NAME}_ded")
		endif()
		
		if(RTCW_TMP_OUTPUT_NAME AND ("demo" IN_LIST RTCW_TMP_TAGS))
			set(RTCW_TMP_OUTPUT_NAME "${RTCW_TMP_OUTPUT_NAME}_demo")
		endif()

		if(RTCW_TMP_OUTPUT_NAME)
			set(RTCW_TMP_OUTPUT_NAME "rtcw_${RTCW_TMP_OUTPUT_NAME}")
		endif()
	endif()

	if (RTCW_TMP_OUTPUT_NAME AND RTCW_ARCH_STRING)
		if("exe" IN_LIST RTCW_TMP_TAGS OR ("dll" IN_LIST RTCW_TMP_TAGS AND (("mp" IN_LIST RTCW_TMP_TAGS) OR ("et" IN_LIST RTCW_TMP_TAGS))))
			set(RTCW_TMP_OUTPUT_NAME "${RTCW_TMP_OUTPUT_NAME}_")
		endif()

		set(RTCW_TMP_OUTPUT_NAME "${RTCW_TMP_OUTPUT_NAME}${RTCW_ARCH_STRING}")
	endif()

	if(RTCW_TMP_OUTPUT_NAME AND ("dll" IN_LIST RTCW_TMP_TAGS) AND ("demo" IN_LIST RTCW_TMP_TAGS))
		set(RTCW_TMP_OUTPUT_NAME "${RTCW_TMP_OUTPUT_NAME}_d")
	endif()

	if(RTCW_TMP_OUTPUT_NAME)
		set_target_properties(${ARGV0} PROPERTIES
			PREFIX ""
			OUTPUT_NAME "${RTCW_TMP_OUTPUT_NAME}"
		)

		message(STATUS "[${ARGV0}] Output name: ${RTCW_TMP_OUTPUT_NAME}")
	endif()

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

	if(RTCW_BUILD_SDL2)
		target_link_libraries(${ARGV0}
			PRIVATE
				$<$<BOOL:${RTCW_TMP_IS_EXECUTABLE}>:SDL2::SDL2-static>
				$<$<BOOL:${RTCW_TMP_IS_EXECUTABLE}>:rtcw::sdl2_net>
		)
	else()
		target_link_libraries(${ARGV0}
			PRIVATE
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