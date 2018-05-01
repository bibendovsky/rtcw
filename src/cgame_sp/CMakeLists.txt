cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(rtcw_cgame_sp VERSION 1.0.0 LANGUAGES CXX)

find_package(SDL 2.0.4 REQUIRED)

set(
	headers
	../cgame/cg_public.h
	../cgame/tr_types.h
	../cgame_sp/cg_local.h
	../game/bg_public.h
	../game/q_shared.h
	../game/surfaceflags.h
	../game_sp/bg_local.h
	../ui/keycodes.h
	../ui_sp/menudef.h
	../ui_sp/ui_shared.h
)

set(
	sources
	../game/q_math.cpp
	../game/q_shared.cpp
	../game_sp/bg_animation.cpp
	../game_sp/bg_misc.cpp
	../game_sp/bg_pmove.cpp
	../game_sp/bg_slidemove.cpp
	../ui_sp/ui_shared.cpp
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
	cg_syscalls.cpp
	cg_trails.cpp
	cg_view.cpp
	cg_weapons.cpp
)

if (WIN32)
	list(
		APPEND
		sources
		cgame_sp.def
	)
endif ()

add_library(
	rtcw_cgame_sp
	SHARED
	${headers}
	${sources}
)

target_include_directories(
	rtcw_cgame_sp
	PRIVATE ${SDL_INCLUDE_DIR}
	PRIVATE .
	PRIVATE ../cgame
	PRIVATE ../game
	PRIVATE ../game_sp
	PRIVATE ../qcommon
	PRIVATE ../system
	PRIVATE ../ui
	PRIVATE ../ui_sp
)

target_compile_definitions(
	rtcw_cgame_sp
	PRIVATE CGAMEDLL
	PRIVATE RTCW_SP
)

if (MSVC)
	target_compile_definitions(
		rtcw_cgame_sp
		PRIVATE _CRT_SECURE_NO_WARNINGS
	)
endif ()

set_target_properties(
	rtcw_cgame_sp
	PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(
	rtcw_cgame_sp
	PROPERTIES
	PREFIX ""
	OUTPUT_NAME cgamex86
)

install(
	TARGETS
	rtcw_cgame_sp
	RUNTIME DESTINATION sp
)
