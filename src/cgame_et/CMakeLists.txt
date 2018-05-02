cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(rtcw_cgame_et VERSION 1.0.0 LANGUAGES CXX)

find_package(SDL 2.0.4 REQUIRED)

option(RTCW_USE_MULTI_PROCESS_COMPILATION "Enables multi-process compilation if available." ON)

set(
	headers
	../cgame/cg_public.h
	../cgame/tr_types.h
	../cgame_et/cg_local.h
	../game/bg_public.h
	../game/q_shared.h
	../game/surfaceflags.h
	../game_et/bg_local.h
	../ui/keycodes.h
	../ui_et/menudef.h
	../ui_et/ui_shared.h
)

set(
	sources
	../game/q_math.cpp
	../game/q_shared.cpp
	../game_et/bg_animation.cpp
	../game_et/bg_animgroup.cpp
	../game_et/bg_campaign.cpp
	../game_et/bg_character.cpp
	../game_et/bg_classes.cpp
	../game_et/bg_misc.cpp
	../game_et/bg_pmove.cpp
	../game_et/bg_slidemove.cpp
	../game_et/bg_sscript.cpp
	../game_et/bg_stats.cpp
	../game_et/bg_tracemap.cpp
	../ui_et/ui_shared.cpp
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
	list(
		APPEND
		sources
		cgame_et.def
	)
endif ()

add_library(
	rtcw_cgame_et
	SHARED
	${headers}
	${sources}
)

target_include_directories(
	rtcw_cgame_et
	PRIVATE ${SDL_INCLUDE_DIR}
	PRIVATE .
	PRIVATE ../cgame
	PRIVATE ../game
	PRIVATE ../game_et
	PRIVATE ../qcommon
	PRIVATE ../system
	PRIVATE ../ui
	PRIVATE ../ui_et
)

target_compile_definitions(
	rtcw_cgame_et
	PRIVATE CGAMEDLL
	PRIVATE RTCW_ET
)

if (MSVC)
	target_compile_definitions(
		rtcw_cgame_et
		PRIVATE _CRT_SECURE_NO_WARNINGS
	)
endif ()

if (RTCW_USE_MULTI_PROCESS_COMPILATION)
	if (MSVC)
		target_compile_options(
			rtcw_cgame_et
			PRIVATE -MP
		)
	endif ()
endif ()

set_target_properties(
	rtcw_cgame_et
	PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(
	rtcw_cgame_et
	PROPERTIES
	PREFIX ""
	OUTPUT_NAME cgame_mp_x86
)

install(
	TARGETS
	rtcw_cgame_et
	RUNTIME DESTINATION et/Main
)
