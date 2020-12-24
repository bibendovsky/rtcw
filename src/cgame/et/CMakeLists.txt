cmake_minimum_required (VERSION 3.8.2 FATAL_ERROR)
project (rtcw_cgame_et VERSION 1.0.0 LANGUAGES CXX)

message (STATUS "")
message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")

option (RTCW_MULTI_PROCESS_COMPILATION "Enable multi-process compilation if available." ON)
message (STATUS "[${PROJECT_NAME}] Multi-process compilation: ${RTCW_MULTI_PROCESS_COMPILATION}")

# Precompiled headers.
#
option (RTCW_USE_PCH "Use precompiled headers if available." ON)
message (STATUS "[${PROJECT_NAME}] Precompiled headers: ${RTCW_USE_PCH}")


#
# CPU architecture.
#
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	set (RTCW_ARCH_STRING "x86")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set (RTCW_ARCH_STRING "x64")
else ()
	unset (RTCW_ARCH_STRING)
endif ()

if (RTCW_ARCH_STRING)
	message (STATUS "[${PROJECT_NAME}] Arch: ${RTCW_ARCH_STRING}")
else ()
	message (FATAL_ERROR "[${PROJECT_NAME}] Unsupported platform.")
endif ()


find_package (SDL2W 2.0.4 REQUIRED)

add_library (${PROJECT_NAME} SHARED "")


set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		PREFIX ""
		OUTPUT_NAME "cgame_mp_${RTCW_ARCH_STRING}"
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../../cgame
		../../game
		../../game/et
		../../qcommon
		../../system
		../../ui
		../../ui/et
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		CGAMEDLL
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
		../../cgame/cg_public.h
		../../cgame/tr_types.h
		../../cgame/et/cg_local.h
		../../game/bg_public.h
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../game/et/bg_local.h
		../../qcommon/rtcw_vm_args.h
		../../ui/keycodes.h
		../../ui/et/menudef.h
		../../ui/et/ui_shared.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/et/bg_animation.cpp
		../../game/et/bg_animgroup.cpp
		../../game/et/bg_campaign.cpp
		../../game/et/bg_character.cpp
		../../game/et/bg_classes.cpp
		../../game/et/bg_misc.cpp
		../../game/et/bg_pmove.cpp
		../../game/et/bg_slidemove.cpp
		../../game/et/bg_sscript.cpp
		../../game/et/bg_stats.cpp
		../../game/et/bg_tracemap.cpp
		../../ui/et/ui_shared.cpp
		cg_atmospheric.cpp
		cg_character.cpp
		cg_commandmap.cpp
		cg_consolecmds.cpp
		cg_debriefing.cpp
		cg_draw.cpp
		cg_drawtools.cpp
		cg_effects.cpp
		cg_ents.cpp
		cg_event.cpp
		cg_fireteamoverlay.cpp
		cg_fireteams.cpp
		cg_flamethrower.cpp
		cg_info.cpp
		cg_limbopanel.cpp
		cg_loadpanel.cpp
		cg_localents.cpp
		cg_main.cpp
		cg_marks.cpp
		cg_missionbriefing.cpp
		cg_multiview.cpp
		cg_newDraw.cpp
		cg_particles.cpp
		cg_players.cpp
		cg_playerstate.cpp
		cg_polybus.cpp
		cg_popupmessages.cpp
		cg_predict.cpp
		cg_scoreboard.cpp
		cg_servercmds.cpp
		cg_snapshot.cpp
		cg_sound.cpp
		cg_spawn.cpp
		cg_statsranksmedals.cpp
		cg_syscalls.cpp
		cg_trails.cpp
		cg_view.cpp
		cg_weapons.cpp
		cg_window.cpp
)

if (WIN32)
	target_sources (
		${PROJECT_NAME}
		PRIVATE
			cgame_et.def
	)
endif ()

install (
	TARGETS
		${PROJECT_NAME}
	LIBRARY
		DESTINATION et/main
	RUNTIME
		DESTINATION et/main
)
