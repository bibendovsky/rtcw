cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (rtcw_ui_et VERSION 1.0.0 LANGUAGES CXX)

message (STATUS "")
message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")

option (RTCW_MULTI_PROCESS_COMPILATION "Enable multi-process compilation if available." ON)
message (STATUS "[${PROJECT_NAME}] Multi-process compilation: ${RTCW_MULTI_PROCESS_COMPILATION}")

# Precompiled headers.
#
option (RTCW_USE_PCH "Use precompiled headers if available." ON)
message (STATUS "[${PROJECT_NAME}] Precompiled headers: ${RTCW_USE_PCH}")


find_package (SDL2W 2.0.4 REQUIRED)

add_library (${PROJECT_NAME} SHARED "")

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		PREFIX ""
		OUTPUT_NAME ui_mp_x86
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../../cgame
		../../game
		../../qcommon
		../../system
		../../ui
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		RTCW_ET
)

if (MSVC)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			_CRT_SECURE_NO_WARNINGS
	)
endif ()

if (RTCW_MULTI_PROCESS_COMPILATION)
	if (MSVC)
		target_compile_options (
			${PROJECT_NAME}
			PRIVATE
				-MP
		)
	endif ()
endif ()


#
# Precompile headers.
#
if (RTCW_USE_PCH AND NOT (${CMAKE_VERSION} VERSION_LESS "3.16"))
	target_precompile_headers(
		${PROJECT_NAME}
		PRIVATE
			rtcw_pch.h
	)
endif ()


target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		SDL2W::SDL2W
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../cgame/tr_types.h
		../../game/bg_public.h
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../ui/keycodes.h
		../../ui/ui_public.h
		menudef.h
		ui_local.h
		ui_shared.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/et/bg_campaign.cpp
		../../game/et/bg_classes.cpp
		../../game/et/bg_misc.cpp
		ui_atoms.cpp
		ui_gameinfo.cpp
		ui_loadpanel.cpp
		ui_main.cpp
		ui_players.cpp
		ui_shared.cpp
		ui_syscalls.cpp
		ui_util.cpp
)

if (WIN32)
	target_sources (
		${PROJECT_NAME}
		PRIVATE
			ui_et.def
	)
endif ()

install (
	TARGETS
	${PROJECT_NAME}
	RUNTIME DESTINATION et/Main
)
