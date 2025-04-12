cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_cgame_mp VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_cgame_mp] Version: ${PROJECT_VERSION}")

add_library(rtcw_cgame_mp SHARED "")

set_target_properties(rtcw_cgame_mp PROPERTIES
	PREFIX ""
	OUTPUT_NAME "cgame_mp_${RTCW_ARCH_STRING}"
)

if(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom"))
	get_target_property(RTCW_TMP_LINK_FLAGS rtcw_cgame_mp LINK_FLAGS)

	if(NOT RTCW_TMP_LINK_FLAGS)
		set(RTCW_TMP_LINK_FLAGS "")
	endif()

	set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} export dllEntry=_dllEntry,vmMain=_vmMain")

	set_target_properties(rtcw_cgame_mp PROPERTIES LINK_FLAGS "${RTCW_TMP_LINK_FLAGS}")
endif()

target_include_directories(rtcw_cgame_mp
	PRIVATE
		.
		../../cgame
		../../game
		../../game/mp
		../../qcommon
		../../system
		../../ui
		../../ui/mp
)

target_compile_definitions(rtcw_cgame_mp
	PRIVATE
		CGAMEDLL
		RTCW_MP
)

rtcw_configure_target(rtcw_cgame_mp)

target_sources(rtcw_cgame_mp
	PRIVATE
		../../cgame/cg_public.h
		../../cgame/tr_types.h
		../../game/bg_public.h
		../../game/mp/bg_animation.cpp
		../../game/mp/bg_local.h
		../../game/mp/bg_misc.cpp
		../../game/mp/bg_pmove.cpp
		../../game/mp/bg_slidemove.cpp
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../qcommon/rtcw_vm_args.h
		../../ui/keycodes.h
		../../ui/mp/menudef.h
		../../ui/mp/ui_shared.cpp
		../../ui/mp/ui_shared.h
		cg_consolecmds.cpp
		cg_draw.cpp
		cg_drawtools.cpp
		cg_effects.cpp
		cg_ents.cpp
		cg_event.cpp
		cg_flamethrower.cpp
		cg_info.cpp
		cg_local.h
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

install(
	TARGETS rtcw_cgame_mp
	LIBRARY DESTINATION mp/main
	RUNTIME DESTINATION mp/main
)
