cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#
# CPU architecture.
#
if((NOT DEFINED RTCW_ARCH_X86_32) OR (NOT DEFINED RTCW_ARCH_X86_64))
	message(STATUS "[${PROJECT_NAME}] Checking for x86.")
	set(RTCW_TRY_COMPILE_OUTPUT_RESULT "")

	try_compile(RTCW_ARCH_X86_32
		${CMAKE_CURRENT_BINARY_DIR}/cmake/try_compile
		${CMAKE_SOURCE_DIR}/cmake/try_compile/is_x86_32.cpp
		OUTPUT_VARIABLE RTCW_TRY_COMPILE_OUTPUT_RESULT
	)

	#[[
	if(RTCW_TRY_COMPILE_OUTPUT_RESULT)
		message(STATUS "<<<<<<<< TRY COMPILE RESULT <<<<<<<<")
		message(STATUS "${RTCW_TRY_COMPILE_OUTPUT_RESULT}")
		message(STATUS ">>>>>>>> TRY COMPILE RESULT >>>>>>>>")
	endif()
	]]

	if(RTCW_ARCH_X86_32)
		set(RTCW_ARCH_X86_32 TRUE CACHE INTERNAL "Architecture x86-32.")
	else()
		set(RTCW_ARCH_X86_32 FALSE CACHE INTERNAL "Architecture x86-32.")
	endif()

	message(STATUS "[${PROJECT_NAME}] Checking for x86_64.")
	set(RTCW_TRY_COMPILE_OUTPUT_RESULT "")

	try_compile(RTCW_ARCH_X86_64
		${CMAKE_CURRENT_BINARY_DIR}/cmake/try_compile
		${CMAKE_SOURCE_DIR}/cmake/try_compile/is_x86_64.cpp
		OUTPUT_VARIABLE RTCW_TRY_COMPILE_OUTPUT_RESULT
	)

	#[[
	if(RTCW_TRY_COMPILE_OUTPUT_RESULT)
		message(STATUS "<<<<<<<< TRY COMPILE RESULT <<<<<<<<")
		message(STATUS "${RTCW_TRY_COMPILE_OUTPUT_RESULT}")
		message(STATUS ">>>>>>>> TRY COMPILE RESULT >>>>>>>>")
	endif()
	]]

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

function(rtcw_configure_static_linking)
	if(NOT (${ARGC} EQUAL 1))
		message(FATAL_ERROR "Usage: rtcw_configure_static_linking <target_name>")
	endif()

	if(RTCW_ENABLE_STATIC_LINKING)
		target_compile_options(${ARGV0}
			PRIVATE
				$<$<AND:$<BOOL:${MSVC}>,$<CONFIG:DEBUG>>:/MTd>
				$<$<AND:$<BOOL:${MSVC}>,$<NOT:$<CONFIG:DEBUG>>>:/MT>
			PRIVATE
				$<$<AND:$<BOOL:${WIN32}>,$<C_COMPILER_ID:OpenWatcom>>:-bm>
				$<$<AND:$<BOOL:${WIN32}>,$<CXX_COMPILER_ID:OpenWatcom>>:-bm>
		)

		target_link_libraries(${ARGV0}
			PRIVATE
				$<$<AND:$<BOOL:${WIN32}>,$<C_COMPILER_ID:Clang>>:-static>
				$<$<AND:$<BOOL:${WIN32}>,$<CXX_COMPILER_ID:Clang>>:-static>
			PRIVATE
				$<$<AND:$<BOOL:${WIN32}>,$<C_COMPILER_ID:GNU>>:-static>
				$<$<AND:$<BOOL:${WIN32}>,$<CXX_COMPILER_ID:GNU>>:-static>
			PRIVATE
				$<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<C_COMPILER_ID:Clang>>:-static-libgcc>
				$<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<CXX_COMPILER_ID:Clang>>:-static-libstdc++>
				$<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<CXX_COMPILER_ID:Clang>>:-static-libgcc>
			PRIVATE
				$<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<C_COMPILER_ID:GNU>>:-static-libgcc>
				$<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<CXX_COMPILER_ID:GNU>>:-static-libstdc++>
				$<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<CXX_COMPILER_ID:GNU>>:-static-libgcc>
		)
	endif()
endfunction(rtcw_configure_static_linking)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

function(rtcw_configure_output_name)
	if(NOT (${ARGC} EQUAL 1))
		message(FATAL_ERROR "Usage: rtcw_configure_output_name <target_name>")
	endif()

	get_target_property(RTCW_TMP_TAGS ${ARGV0} RTCW_TAGS)

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
endfunction(rtcw_configure_output_name)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

function(rtcw_configure_stack)
	if(NOT (${ARGC} EQUAL 1))
		message(FATAL_ERROR "Usage: rtcw_configure_stack <target_name>")
	endif()

	if(NOT WIN32)
		return()
	endif()

	get_target_property(RTCW_TMP_TAGS ${ARGV0} RTCW_TAGS)

	if("exe" IN_LIST RTCW_TMP_TAGS)
		get_target_property(RTCW_TMP_LINK_FLAGS ${ARGV0} LINK_FLAGS)

		if(NOT RTCW_TMP_LINK_FLAGS)
			set(RTCW_TMP_LINK_FLAGS "")
		endif()

		set(RTCW_TMP_EXE_STACK_SIZE "0x800000")
		set(RTCW_TMP_EXE_STACK_SIZE_IS_SET FALSE)

		if(MSVC)
			set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} /STACK:${RTCW_TMP_EXE_STACK_SIZE}")
			set(RTCW_TMP_EXE_STACK_SIZE_IS_SET TRUE)
		elseif((CMAKE_C_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
			set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} -Wl,--stack,${RTCW_TMP_EXE_STACK_SIZE}")
			set(RTCW_TMP_EXE_STACK_SIZE_IS_SET TRUE)
		elseif((CMAKE_C_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
			set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} -Wl,--stack,${RTCW_TMP_EXE_STACK_SIZE}")
			set(RTCW_TMP_EXE_STACK_SIZE_IS_SET TRUE)
		elseif((CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom") OR (CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom"))
			set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} option stack=${RTCW_TMP_EXE_STACK_SIZE}")
			set(RTCW_TMP_EXE_STACK_SIZE_IS_SET TRUE)
		endif()

		if(RTCW_TMP_EXE_STACK_SIZE_IS_SET)
			message(STATUS "[${ARGV0}] Set stack size: ${RTCW_TMP_EXE_STACK_SIZE}")
			set_target_properties(${ARGV0} PROPERTIES LINK_FLAGS "${RTCW_TMP_LINK_FLAGS}")
		endif()
	endif()

	target_compile_options(${ARGV0}
		PRIVATE
			$<$<C_COMPILER_ID:OpenWatcom>:-sg> # Generate calls to grow the stack
			$<$<CXX_COMPILER_ID:OpenWatcom>:-sg> # Generate calls to grow the stack
	)
endfunction(rtcw_configure_stack)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

function(rtcw_configure_resources)
	if(NOT (${ARGC} EQUAL 1))
		message(FATAL_ERROR "Usage: rtcw_configure_resources <target_name>")
	endif()

	if(NOT WIN32)
		return()
	endif()

	message(STATUS "[${ARGV0}] Add resource files.")

	set(RTCW_TMP_IS_EXE FALSE)
	set(RTCW_TMP_IS_DLL FALSE)
	set(RTCW_TMP_RC_SOURCE_DIR "")
	get_target_property(RTCW_TMP_TAGS ${ARGV0} RTCW_TAGS)
	get_target_property(RTCW_TMP_SOURCE_DIR ${ARGV0} SOURCE_DIR)
	get_target_property(RTCW_TMP_BINARY_DIR ${ARGV0} BINARY_DIR)

	if("exe" IN_LIST RTCW_TMP_TAGS)
		set(RTCW_TMP_IS_EXE TRUE)
		set(RTCW_TMP_RC_SOURCE_DIR ${RTCW_TMP_SOURCE_DIR}/../win32)
	elseif("dll" IN_LIST RTCW_TMP_TAGS)
		set(RTCW_TMP_IS_DLL TRUE)
		set(RTCW_TMP_RC_SOURCE_DIR ${RTCW_TMP_SOURCE_DIR}/../../win32)
	else()
		return()
	endif()

	set(RTCW_TMP_RC_SOURCE_FILE_PATH "${RTCW_TMP_RC_SOURCE_DIR}/rtcw.rc")

	get_target_property(RTCW_TMP_OUTPUT_NAME ${ARGV0} OUTPUT_NAME)

	get_target_property(RTCW_TMP_OUTPUT_NAME_SUFFIX ${ARGV0} SUFFIX)

	if(NOT RTCW_TMP_OUTPUT_NAME_SUFFIX)
		set(RTCW_TMP_OUTPUT_NAME_SUFFIX ".exe")
	endif()

	set(RTCW_TMP_RC_ICON_FILE_NAME "")

	if("sp" IN_LIST RTCW_TMP_TAGS)
		set(RTCW_TMP_RC_ICON_FILE_NAME rtcw_sp.ico)
	elseif("mp" IN_LIST RTCW_TMP_TAGS)
		set(RTCW_TMP_RC_ICON_FILE_NAME rtcw_mp.ico)
	elseif("et" IN_LIST RTCW_TMP_TAGS)
		set(RTCW_TMP_RC_ICON_FILE_NAME rtcw_et.ico)
	else()
		message(FATAL_ERROR "[${ARGV0}] Unknown RC icon file name.")
	endif()

	get_target_property(RTCW_TMP_RC_VERSION ${ARGV0} RTCW_VERSION)
	get_target_property(RTCW_TMP_RC_VERSION_MAJOR ${ARGV0} RTCW_VERSION_MAJOR)
	get_target_property(RTCW_TMP_RC_VERSION_MINOR ${ARGV0} RTCW_VERSION_MINOR)
	get_target_property(RTCW_TMP_RC_VERSION_PATCH ${ARGV0} RTCW_VERSION_PATCH)

	if(RTCW_GIT_HASH)
		set(RTCW_TMP_RC_VERSION "${RTCW_TMP_RC_VERSION}+${RTCW_GIT_HASH}")
	endif()

	target_sources(${ARGV0} PRIVATE ${RTCW_TMP_RC_SOURCE_FILE_PATH})

	if(CMAKE_CXX_COMPILER_ID STREQUAL OpenWatcom)
		set(RTCW_TMP_RC_BINARY_FILE_PATH ${RTCW_TMP_BINARY_DIR}/rtcw.res)

		add_custom_command(
			TARGET ${ARGV0} POST_BUILD
			COMMAND wrc.exe
				/DRTCW_RC_HAS_MANIFEST
				/D$<$<CONFIG:DEBUG>:RTCW_RC_IS_DEBUG>
				/D$<$<BOOL:${RTCW_TMP_IS_EXE}>:RTCW_RC_IS_EXE>
				/D$<$<BOOL:${RTCW_TMP_IS_EXE}>:RTCW_RC_HAS_ICON>
				/D$<$<BOOL:${RTCW_TMP_IS_EXE}>:RTCW_RC_ICON_FILE_NAME=${RTCW_TMP_RC_ICON_FILE_NAME}>
				/DRTCW_RC_HAS_VERSION
				/DRTCW_RC_VERSION=${RTCW_TMP_RC_VERSION}
				/DRTCW_RC_VERSION_MAJOR=${RTCW_TMP_RC_VERSION_MAJOR}
				/DRTCW_RC_VERSION_MINOR=${RTCW_TMP_RC_VERSION_MINOR}
				/DRTCW_RC_VERSION_PATCH=${RTCW_TMP_RC_VERSION_PATCH}
				/DRTCW_RC_INTERNAL_NAME=${RTCW_TMP_OUTPUT_NAME}
				/DRTCW_RC_ORIGINAL_FILENAME=${RTCW_TMP_OUTPUT_NAME}${RTCW_TMP_OUTPUT_NAME_SUFFIX}
				/bt=nt
				/i=$<SHELL_PATH:${RTCW_TMP_RC_SOURCE_DIR}>
				/fo=$<SHELL_PATH:${RTCW_TMP_RC_BINARY_FILE_PATH}>
				$<SHELL_PATH:${RTCW_TMP_RC_SOURCE_FILE_PATH}>
				$<SHELL_PATH:$<TARGET_FILE:${ARGV0}>>
			COMMENT "[${ARGV0}][OpenWatcom] Embed resources."
			VERBATIM
		)
	else()
		target_compile_definitions(${ARGV0}
			PRIVATE
				RTCW_RC_HAS_MANIFEST
				$<$<CONFIG:DEBUG>:RTCW_RC_IS_DEBUG>
				$<$<BOOL:${RTCW_TMP_IS_EXE}>:RTCW_RC_IS_EXE>
				$<$<BOOL:${RTCW_TMP_IS_EXE}>:RTCW_RC_HAS_ICON>
				$<$<BOOL:${RTCW_TMP_IS_EXE}>:RTCW_RC_ICON_FILE_NAME=${RTCW_TMP_RC_ICON_FILE_NAME}>
				RTCW_RC_HAS_VERSION
				RTCW_RC_VERSION=${RTCW_TMP_RC_VERSION}
				RTCW_RC_VERSION_MAJOR=${RTCW_TMP_RC_VERSION_MAJOR}
				RTCW_RC_VERSION_MINOR=${RTCW_TMP_RC_VERSION_MINOR}
				RTCW_RC_VERSION_PATCH=${RTCW_TMP_RC_VERSION_PATCH}
				RTCW_RC_INTERNAL_NAME=${RTCW_TMP_OUTPUT_NAME}
				RTCW_RC_ORIGINAL_FILENAME=${RTCW_TMP_OUTPUT_NAME}${RTCW_TMP_OUTPUT_NAME_SUFFIX}
		)
	endif()
endfunction(rtcw_configure_resources)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

function(rtcw_configure_target)
	if(NOT (${ARGC} EQUAL 1))
		message(FATAL_ERROR "Usage: rtcw_configure_target <target_name>")
	endif()

	rtcw_configure_output_name(${ARGV0})
	rtcw_configure_stack(${ARGV0})
	rtcw_configure_resources(${ARGV0})
	rtcw_configure_static_linking(${ARGV0})

	get_target_property(RTCW_TMP_TAGS ${ARGV0} RTCW_TAGS)
	get_target_property(RTCW_TMP_VERSION ${ARGV0} RTCW_VERSION)

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
	endif()

	# ----------------------------
	# Windows application manifest
	if(MSVC AND (NOT (MSVC_VERSION LESS 1400)))
		set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} /MANIFEST:NO")
	endif()

	if("dll" IN_LIST RTCW_TMP_TAGS)
		if(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom"))
			message(STATUS "[${ARGV0}][OpenWatcom] Fix DLL exports.")
			set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} export dllEntry=_dllEntry,vmMain=_vmMain")
		endif()
	endif()

	if(CMAKE_STRIP AND (("exe" IN_LIST RTCW_TMP_TAGS) OR ("dll" IN_LIST RTCW_TMP_TAGS)))
		add_custom_command(
			TARGET ${ARGV0} POST_BUILD
			COMMAND $<$<CONFIG:RELEASE>:${CMAKE_STRIP}> $<$<CONFIG:RELEASE>:$<TARGET_FILE:${ARGV0}>>
			COMMENT "[${ARGV0}] Strip executable."
			VERBATIM
		)
	endif()

	set_target_properties(${ARGV0} PROPERTIES LINK_FLAGS "${RTCW_TMP_LINK_FLAGS}")

	target_compile_definitions(${ARGV0}
		PRIVATE
			__STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS # stdint.h
			__STDC_FORMAT_MACROS # inttypes.h
			"RTCW_ARCH_STRING=\"${RTCW_ARCH_STRING}\""
			RTCW_VERSION_NUMBER="${RTCW_TMP_VERSION}+${RTCW_GIT_HASH}"
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

	rtcw_configure_stack(${ARGV0})
	rtcw_configure_static_linking(${ARGV0})
endfunction(rtcw_configure_3rd_party_target)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>