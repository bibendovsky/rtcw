cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_cgame_sp VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_cgame_sp] Version: ${PROJECT_VERSION}")

add_library(rtcw_cgame_sp SHARED "")

set_target_properties(rtcw_cgame_sp PROPERTIES
	RTCW_TAGS "sp;cgame;dll"
)

target_include_directories(rtcw_cgame_sp
	PRIVATE
		.
		../../cgame
		../../game
		../../game/sp
		../../qcommon
		../../system
		../../ui
		../../ui/sp
)

target_compile_definitions(rtcw_cgame_sp
	PRIVATE
		CGAMEDLL
		RTCW_SP
)

rtcw_configure_target(rtcw_cgame_sp)

target_sources(rtcw_cgame_sp
	PRIVATE
		../../cgame/cg_public.h
		../../cgame/sp/cg_local.h
		../../cgame/tr_types.h
		../../game/bg_public.h
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/q_shared.h
		../../game/sp/bg_animation.cpp
		../../game/sp/bg_local.h
		../../game/sp/bg_misc.cpp
		../../game/sp/bg_pmove.cpp
		../../game/sp/bg_slidemove.cpp
		../../game/surfaceflags.h
		../../qcommon/rtcw_vm_args.h
		../../ui/keycodes.h
		../../ui/sp/menudef.h
		../../ui/sp/ui_shared.cpp
		../../ui/sp/ui_shared.h
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

install(
	TARGETS rtcw_cgame_sp
	LIBRARY DESTINATION sp
	RUNTIME DESTINATION sp
)
