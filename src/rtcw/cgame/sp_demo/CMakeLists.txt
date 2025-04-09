cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_cgame_sp_demo VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_cgame_sp_demo] Version: ${PROJECT_VERSION}")

add_library(rtcw_cgame_sp_demo SHARED "")

set_target_properties(rtcw_cgame_sp_demo PROPERTIES
	PREFIX ""
	OUTPUT_NAME "cgame${RTCW_ARCH_STRING}_d"
)

target_include_directories(rtcw_cgame_sp_demo
	PRIVATE
		.
		../sp
		../../cgame
		../../game
		../../game/sp
		../../qcommon
		../../system
		../../ui
		../../ui/sp
)

target_compile_definitions(rtcw_cgame_sp_demo
	PRIVATE
		CGAMEDLL
		WOLF_SP_DEMO
		RTCW_SP
		RTCW_SP_DEMO
)

rtcw_configure_target(rtcw_cgame_sp_demo)

target_sources(rtcw_cgame_sp_demo
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
		../sp/cg_consolecmds.cpp
		../sp/cg_draw.cpp
		../sp/cg_drawtools.cpp
		../sp/cg_effects.cpp
		../sp/cg_ents.cpp
		../sp/cg_event.cpp
		../sp/cg_flamethrower.cpp
		../sp/cg_info.cpp
		../sp/cg_localents.cpp
		../sp/cg_main.cpp
		../sp/cg_marks.cpp
		../sp/cg_newDraw.cpp
		../sp/cg_particles.cpp
		../sp/cg_players.cpp
		../sp/cg_playerstate.cpp
		../sp/cg_predict.cpp
		../sp/cg_scoreboard.cpp
		../sp/cg_servercmds.cpp
		../sp/cg_snapshot.cpp
		../sp/cg_sound.cpp
		../sp/cg_syscalls.cpp
		../sp/cg_trails.cpp
		../sp/cg_view.cpp
		../sp/cg_weapons.cpp
	PRIVATE
		$<$<BOOL:${WIN32}>:../sp/cgame_sp.def>
)

install(
	TARGETS rtcw_cgame_sp_demo
	LIBRARY DESTINATION sp_demo
	RUNTIME DESTINATION sp_demo
)
