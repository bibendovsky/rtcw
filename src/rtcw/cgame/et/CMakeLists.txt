cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_cgame_et VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_cgame_et] Version: ${PROJECT_VERSION}")

add_library(rtcw_cgame_et SHARED "")

set_target_properties(rtcw_cgame_et PROPERTIES
	RTCW_TAGS "et;cgame;dll"
)

target_include_directories(rtcw_cgame_et
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

target_compile_definitions(rtcw_cgame_et
	PRIVATE
		CGAMEDLL
		RTCW_ET
)

rtcw_configure_target(rtcw_cgame_et)

target_sources(rtcw_cgame_et
	PRIVATE
		../../cgame/cg_public.h
		../../cgame/et/cg_local.h
		../../cgame/tr_types.h
		../../game/bg_public.h
		../../game/et/bg_animation.cpp
		../../game/et/bg_animgroup.cpp
		../../game/et/bg_campaign.cpp
		../../game/et/bg_character.cpp
		../../game/et/bg_classes.cpp
		../../game/et/bg_local.h
		../../game/et/bg_misc.cpp
		../../game/et/bg_pmove.cpp
		../../game/et/bg_slidemove.cpp
		../../game/et/bg_sscript.cpp
		../../game/et/bg_stats.cpp
		../../game/et/bg_tracemap.cpp
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../qcommon/rtcw_vm_args.h
		../../ui/et/menudef.h
		../../ui/et/ui_shared.cpp
		../../ui/et/ui_shared.h
		../../ui/keycodes.h
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

install(
	TARGETS rtcw_cgame_et
	LIBRARY DESTINATION et/etmain
	RUNTIME DESTINATION et/etmain
)
