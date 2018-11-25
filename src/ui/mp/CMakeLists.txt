cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(rtcw_ui_mp VERSION 1.0.0 LANGUAGES CXX)

find_package(SDL 2.0.4 REQUIRED)

option(RTCW_USE_MULTI_PROCESS_COMPILATION "Enables multi-process compilation if available." ON)

add_library(${PROJECT_NAME} SHARED "")

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
		PREFIX ""
		OUTPUT_NAME ui_mp_x86
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${SDL_INCLUDE_DIR}
		.
		../../cgame
		../../game
		../../qcommon
		../../system
		../../ui
)

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
		RTCW_MP
)

if (MSVC)
	target_compile_definitions(
		${PROJECT_NAME}
		PRIVATE
			_CRT_SECURE_NO_WARNINGS
	)
endif ()

if (RTCW_USE_MULTI_PROCESS_COMPILATION)
	if (MSVC)
		target_compile_options(
			${PROJECT_NAME}
			PRIVATE
				-MP
		)
	endif ()
endif ()

target_sources(
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

target_sources(
	${PROJECT_NAME}
	PRIVATE
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/mp/bg_misc.cpp
		ui_atoms.cpp
		ui_gameinfo.cpp
		ui_main.cpp
		ui_players.cpp
		ui_shared.cpp
		ui_syscalls.cpp
		ui_util.cpp
)

if (WIN32)
	target_sources(
		${PROJECT_NAME}
		PRIVATE
			ui_mp.def
	)
endif ()

install(
	TARGETS
	${PROJECT_NAME}
	RUNTIME DESTINATION mp/Main
)
