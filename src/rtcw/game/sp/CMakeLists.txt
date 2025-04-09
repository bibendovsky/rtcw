cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_game_sp VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_game_sp] Version: ${PROJECT_VERSION}")

add_library(rtcw_game_sp SHARED "")

set_target_properties(rtcw_game_sp PROPERTIES
	PREFIX ""
	OUTPUT_NAME "qagame${RTCW_ARCH_STRING}"
)

target_include_directories(rtcw_game_sp
	PRIVATE
		.
		../../botai
		../../botai/sp
		../../game
		../../qcommon
		../../system
		../../ui/sp
)

target_compile_definitions(rtcw_game_sp
	PRIVATE
		GAMEDLL
		RTCW_SP
		BUILDING_REF_GL
)

rtcw_configure_target(rtcw_game_sp)

target_sources(rtcw_game_sp
	PRIVATE
		../../botai/botai.h
		../../botai/sp/ai_chat.cpp
		../../botai/sp/ai_chat.h
		../../botai/sp/ai_cmd.cpp
		../../botai/sp/ai_cmd.h
		../../botai/sp/ai_dmnet.cpp
		../../botai/sp/ai_dmnet.h
		../../botai/sp/ai_dmq3.cpp
		../../botai/sp/ai_dmq3.h
		../../botai/sp/ai_main.cpp
		../../botai/sp/ai_main.h
		../../botai/sp/ai_team.cpp
		../../botai/sp/ai_team.h
		../../botai/sp/chars.h
		../../botai/sp/inv.h
		../../botai/sp/match.h
		../../botai/sp/syn.h
		../../game/be_aas.h
		../../game/be_ai_char.h
		../../game/be_ai_chat.h
		../../game/be_ai_gen.h
		../../game/be_ai_goal.h
		../../game/be_ai_move.h
		../../game/be_ai_weap.h
		../../game/be_ea.h
		../../game/bg_public.h
		../../game/botlib.h
		../../game/g_public.h
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../qcommon/rtcw_vm_args.h
		ai_cast.cpp
		ai_cast.h
		ai_cast_characters.cpp
		ai_cast_debug.cpp
		ai_cast_events.cpp
		ai_cast_fight.cpp
		ai_cast_fight.h
		ai_cast_func_attack.cpp
		ai_cast_func_boss1.cpp
		ai_cast_funcs.cpp
		ai_cast_global.h
		ai_cast_script.cpp
		ai_cast_script_actions.cpp
		ai_cast_script_ents.cpp
		ai_cast_sight.cpp
		ai_cast_think.cpp
		bg_animation.cpp
		bg_local.h
		bg_misc.cpp
		bg_pmove.cpp
		bg_slidemove.cpp
		g_active.cpp
		g_alarm.cpp
		g_bot.cpp
		g_client.cpp
		g_cmds.cpp
		g_combat.cpp
		g_func_decs.h
		g_funcs.h
		g_items.cpp
		g_local.h
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
		g_team.h
		g_tramcar.cpp
		g_trigger.cpp
		g_utils.cpp
		g_weapon.cpp
)

if(WIN32)
	target_sources(rtcw_game_sp PRIVATE game_sp.def)
endif()

install(
	TARGETS rtcw_game_sp
	LIBRARY DESTINATION sp
	RUNTIME DESTINATION sp
)
