cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(rtcw_game_et VERSION 1.0.0 LANGUAGES CXX)

find_package(SDL 2.0.4 REQUIRED)

set(
	headers
	../botai/botai.h
	../botai_et/ai_cmd.h
	../botai_et/ai_distances.h
	../botai_et/ai_dmgoal_mp.h
	../botai_et/ai_dmnet_mp.h
	../botai_et/ai_dmq3.h
	../botai_et/ai_main.h
	../botai_et/ai_matrix.h
	../botai_et/ai_team.h
	../botai_et/chars.h
	../botai_et/inv.h
	../botai_et/match.h
	../botai_et/syn.h
	../game/be_aas.h
	../game/be_ai_char.h
	../game/be_ai_chat.h
	../game/be_ai_gen.h
	../game/be_ai_goal.h
	../game/be_ai_move.h
	../game/be_ai_weap.h
	../game/be_ea.h
	../game/bg_public.h
	../game/botlib.h
	../game/g_public.h
	../game/q_shared.h
	../game/surfaceflags.h
	bg_local.h
	g_local.h
	g_team.h
)

set(
	sources
	../botai_et/ai_cmd.cpp
	../botai_et/ai_dmgoal_mp.cpp
	../botai_et/ai_dmnet_mp.cpp
	../botai_et/ai_dmq3.cpp
	../botai_et/ai_main.cpp
	../botai_et/ai_script.cpp
	../botai_et/ai_script_actions.cpp
	../botai_et/ai_team.cpp
	../game/q_math.cpp
	../game/q_shared.cpp
	bg_animation.cpp
	bg_animgroup.cpp
	bg_campaign.cpp
	bg_character.cpp
	bg_classes.cpp
	bg_misc.cpp
	bg_pmove.cpp
	bg_slidemove.cpp
	bg_sscript.cpp
	bg_stats.cpp
	bg_tracemap.cpp
	g_active.cpp
	g_alarm.cpp
	g_antilag.cpp
	g_bot.cpp
	g_buddy_list.cpp
	g_character.cpp
	g_client.cpp
	g_cmds.cpp
	g_cmds_ext.cpp
	g_combat.cpp
	g_config.cpp
	g_fireteams.cpp
	g_items.cpp
	g_main.cpp
	g_match.cpp
	g_mem.cpp
	g_misc.cpp
	g_missile.cpp
	g_mover.cpp
	g_multiview.cpp
	g_props.cpp
	g_referee.cpp
	g_save.cpp
	g_script.cpp
	g_script_actions.cpp
	g_session.cpp
	g_spawn.cpp
	g_stats.cpp
	g_sv_entities.cpp
	g_svcmds.cpp
	g_syscalls.cpp
	g_systemmsg.cpp
	g_target.cpp
	g_team.cpp
	g_teammapdata.cpp
	g_trigger.cpp
	g_utils.cpp
	g_vote.cpp
	g_weapon.cpp
)

if (WIN32)
	list(
		APPEND
		sources
		game_et.def
	)
endif ()

add_library(
	rtcw_game_et
	SHARED
	${headers}
	${sources}
)

target_include_directories(
	rtcw_game_et
	PRIVATE ${SDL_INCLUDE_DIR}
	PRIVATE .
	PRIVATE ../botai
	PRIVATE ../botai_et
	PRIVATE ../game
	PRIVATE ../qcommon
	PRIVATE ../system
	PRIVATE ../ui_et
)

target_compile_definitions(
	rtcw_game_et
	PRIVATE GAMEDLL
	PRIVATE RTCW_ET
	PRIVATE BUILDING_REF_GL
)

if (MSVC)
	target_compile_definitions(
		rtcw_game_et
		PRIVATE _CRT_SECURE_NO_WARNINGS
	)
endif ()

set_target_properties(
	rtcw_game_et
	PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(
	rtcw_game_et
	PROPERTIES
	PREFIX ""
	OUTPUT_NAME qagame_mp_x86
)

install(
	TARGETS
	rtcw_game_et
	RUNTIME DESTINATION et/Main
)
