cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_cgame_sp_demo VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")

add_library(${PROJECT_NAME} SHARED "")

set_target_properties(${PROJECT_NAME} PROPERTIES
	PREFIX ""
	OUTPUT_NAME "cgame${RTCW_ARCH_STRING}_d"
)

target_include_directories(${PROJECT_NAME}
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

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		CGAMEDLL
		WOLF_SP_DEMO
		RTCW_SP
		RTCW_SP_DEMO
)

rtcw_configure_target(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
	PRIVATE
		../../cgame/cg_public.h
		../../cgame/tr_types.h
		../../cgame/sp/cg_local.h
		../../game/bg_public.h
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../game/sp/bg_local.h
		../../qcommon/rtcw_vm_args.h
		../../ui/keycodes.h
		../../ui/sp/menudef.h
		../../ui/sp/ui_shared.h
)

target_sources(${PROJECT_NAME}
	PRIVATE
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/sp/bg_animation.cpp
		../../game/sp/bg_misc.cpp
		../../game/sp/bg_pmove.cpp
		../../game/sp/bg_slidemove.cpp
		../../ui/sp/ui_shared.cpp
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
)

if(WIN32)
	target_sources(${PROJECT_NAME} PRIVATE ../sp/cgame_sp.def)
endif()

install(
	TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION sp_demo
	RUNTIME DESTINATION sp_demo
)
