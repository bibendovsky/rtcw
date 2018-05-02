cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(rtcw_cgame_mp VERSION 1.0.0 LANGUAGES CXX)

find_package(SDL 2.0.4 REQUIRED)

option(RTCW_USE_MULTI_PROCESS_COMPILATION "Enables multi-process compilation if available." ON)

set(
	headers
	../../cgame/cg_public.h
	../../cgame/tr_types.h
	../../game/bg_public.h
	../../game/q_shared.h
	../../game/surfaceflags.h
	../../game/mp/bg_local.h
	../../ui/keycodes.h
	../../ui/mp/menudef.h
	../../ui/mp/ui_shared.h
	cg_local.h
)

set(
	sources
	../../game/q_math.cpp
	../../game/q_shared.cpp
	../../game/mp/bg_animation.cpp
	../../game/mp/bg_misc.cpp
	../../game/mp/bg_pmove.cpp
	../../game/mp/bg_slidemove.cpp
	../../ui/mp/ui_shared.cpp
	cg_consolecmds.cpp
	cg_draw.cpp
	cg_drawtools.cpp
	cg_effects.cpp
	cg_ents.cpp
	cg_event.cpp
	cg_flamethrower.cpp
	cg_info.cpp
	cg_localents.cpp
	cg_main.cpp
	cg_marks.cpp
	cg_newDraw.cpp
	cg_particles.cpp
	cg_players.cpp
	cg_playerstate.cpp
	cg_predict.cpp
	cg_scoreboard.cpp
	cg_servercmds.cpp
	cg_snapshot.cpp
	cg_sound.cpp
	cg_spawn.cpp
	cg_syscalls.cpp
	cg_trails.cpp
	cg_view.cpp
	cg_weapons.cpp
)

if (WIN32)
	list(
		APPEND
		sources
		cgame_mp.def
	)
endif ()

add_library(
	rtcw_cgame_mp
	SHARED
	${headers}
	${sources}
)

target_include_directories(
	rtcw_cgame_mp
	PRIVATE ${SDL_INCLUDE_DIR}
	PRIVATE .
	PRIVATE ../../cgame
	PRIVATE ../../game
	PRIVATE ../../game/mp
	PRIVATE ../../qcommon
	PRIVATE ../../system
	PRIVATE ../../ui
	PRIVATE ../../ui/mp
)

target_compile_definitions(
	rtcw_cgame_mp
	PRIVATE CGAMEDLL
	PRIVATE RTCW_MP
)

if (MSVC)
	target_compile_definitions(
		rtcw_cgame_mp
		PRIVATE _CRT_SECURE_NO_WARNINGS
	)
endif ()

if (RTCW_USE_MULTI_PROCESS_COMPILATION)
	if (MSVC)
		target_compile_options(
			rtcw_cgame_mp
			PRIVATE -MP
		)
	endif ()
endif ()

set_target_properties(
	rtcw_cgame_mp
	PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(
	rtcw_cgame_mp
	PROPERTIES
	PREFIX ""
	OUTPUT_NAME cgame_mp_x86
)

install(
	TARGETS
	rtcw_cgame_mp
	RUNTIME DESTINATION mp/Main
)
