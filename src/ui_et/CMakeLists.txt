cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(rtcw_ui_et VERSION 1.0.0 LANGUAGES CXX)

find_package(SDL 2.0.4 REQUIRED)

option(RTCW_USE_MULTI_PROCESS_COMPILATION "Enables multi-process compilation if available." ON)

set(
	headers
	../cgame/tr_types.h
	../game/bg_public.h
	../game/q_shared.h
	../game/surfaceflags.h
	../ui/keycodes.h
	../ui/ui_public.h
	menudef.h
	ui_local.h
	ui_shared.h
)

set(
	sources
	../game/q_math.cpp
	../game/q_shared.cpp
	../game_et/bg_campaign.cpp
	../game_et/bg_classes.cpp
	../game_et/bg_misc.cpp
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
	list(
		APPEND
		sources
		ui_et.def
	)
endif ()

add_library(
	rtcw_ui_et
	SHARED
	${headers}
	${sources}
)

target_include_directories(
	rtcw_ui_et
	PRIVATE ${SDL_INCLUDE_DIR}
	PRIVATE .
	PRIVATE ../cgame
	PRIVATE ../game
	PRIVATE ../qcommon
	PRIVATE ../system
	PRIVATE ../ui
)

target_compile_definitions(
	rtcw_ui_et
	PRIVATE RTCW_ET
)

if (MSVC)
	target_compile_definitions(
		rtcw_ui_et
		PRIVATE _CRT_SECURE_NO_WARNINGS
	)
endif ()

if (RTCW_USE_MULTI_PROCESS_COMPILATION)
	if (MSVC)
		target_compile_options(
			rtcw_ui_et
			PRIVATE -MP
		)
	endif ()
endif ()

set_target_properties(
	rtcw_ui_et
	PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(
	rtcw_ui_et
	PROPERTIES
	PREFIX ""
	OUTPUT_NAME ui_mp_x86
)

install(
	TARGETS
	rtcw_ui_et
	RUNTIME DESTINATION et/Main
)
