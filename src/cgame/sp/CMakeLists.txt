cmake_minimum_required (VERSION 3.8.2 FATAL_ERROR)
project (rtcw_cgame_sp VERSION 1.0.0 LANGUAGES CXX)


message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")


add_library (${PROJECT_NAME} SHARED "")


set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		PREFIX ""
		OUTPUT_NAME "cgame${RTCW_ARCH_STRING}"
)

target_include_directories (
	${PROJECT_NAME}
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

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		CGAMEDLL
		RTCW_SP
)

rtcw_configure_target (${PROJECT_NAME})

target_sources (
	${PROJECT_NAME}
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

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/sp/bg_animation.cpp
		../../game/sp/bg_misc.cpp
		../../game/sp/bg_pmove.cpp
		../../game/sp/bg_slidemove.cpp
		../../ui/sp/ui_shared.cpp
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
	target_sources (
		${PROJECT_NAME}
		PRIVATE
			cgame_sp.def
	)
endif ()

install (
	TARGETS
		${PROJECT_NAME}
	LIBRARY
		DESTINATION sp-mp/main
	RUNTIME
		DESTINATION sp-mp/main
)
