cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(rtcw_game_sp VERSION 1.0.0 LANGUAGES CXX)

find_package(SDL 2.0.4 REQUIRED)

option(RTCW_USE_MULTI_PROCESS_COMPILATION "Enables multi-process compilation if available." ON)

set(
	headers
	../botai/botai.h
	../botai_sp/ai_chat.h
	../botai_sp/ai_cmd.h
	../botai_sp/ai_dmnet.h
	../botai_sp/ai_dmq3.h
	../botai_sp/ai_main.h
	../botai_sp/ai_team.h
	../botai_sp/chars.h
	../botai_sp/inv.h
	../botai_sp/match.h
	../botai_sp/syn.h
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
	ai_cast.h
	ai_cast_fight.h
	ai_cast_global.h
	bg_local.h
	g_func_decs.h
	g_funcs.h
	g_local.h
	g_team.h
)

set(
	sources
	../botai_sp/ai_chat.cpp
	../botai_sp/ai_cmd.cpp
	../botai_sp/ai_dmnet.cpp
	../botai_sp/ai_dmq3.cpp
	../botai_sp/ai_main.cpp
	../botai_sp/ai_team.cpp
	../game/q_math.cpp
	../game/q_shared.cpp
	ai_cast.cpp
	ai_cast_characters.cpp
	ai_cast_debug.cpp
	ai_cast_events.cpp
	ai_cast_fight.cpp
	ai_cast_func_attack.cpp
	ai_cast_func_boss1.cpp
	ai_cast_funcs.cpp
	ai_cast_script.cpp
	ai_cast_script_actions.cpp
	ai_cast_script_ents.cpp
	ai_cast_sight.cpp
	ai_cast_think.cpp
	bg_animation.cpp
	bg_misc.cpp
	bg_pmove.cpp
	bg_slidemove.cpp
	g_active.cpp
	g_alarm.cpp
	g_bot.cpp
	g_client.cpp
	g_cmds.cpp
	g_combat.cpp
	g_items.cpp
	g_main.cpp
	g_mem.cpp
	g_misc.cpp
	g_missile.cpp
	g_mover.cpp
	g_props.cpp
	g_save.cpp
	g_script.cpp
	g_script_actions.cpp
	g_session.cpp
	g_spawn.cpp
	g_svcmds.cpp
	g_syscalls.cpp
	g_target.cpp
	g_team.cpp
	g_tramcar.cpp
	g_trigger.cpp
	g_utils.cpp
	g_weapon.cpp
)

if (WIN32)
	list(
		APPEND
		sources
		game_sp.def
	)
endif ()

add_library(
	rtcw_game_sp
	SHARED
	${headers}
	${sources}
)

target_include_directories(
	rtcw_game_sp
	PRIVATE ${SDL_INCLUDE_DIR}
	PRIVATE .
	PRIVATE ../botai
	PRIVATE ../botai_sp
	PRIVATE ../game
	PRIVATE ../qcommon
	PRIVATE ../system
	PRIVATE ../ui_sp
)

target_compile_definitions(
	rtcw_game_sp
	PRIVATE GAMEDLL
	PRIVATE RTCW_SP
	PRIVATE BUILDING_REF_GL
)

if (MSVC)
	target_compile_definitions(
		rtcw_game_sp
		PRIVATE _CRT_SECURE_NO_WARNINGS
	)
endif ()

if (RTCW_USE_MULTI_PROCESS_COMPILATION)
	if (MSVC)
		target_compile_options(
			rtcw_game_sp
			PRIVATE -MP
		)
	endif ()
endif ()

set_target_properties(
	rtcw_game_sp
	PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(
	rtcw_game_sp
	PROPERTIES
	PREFIX ""
	OUTPUT_NAME qagamex86
)

install(
	TARGETS
	rtcw_game_sp
	RUNTIME DESTINATION sp
)
