cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_game_sp_demo VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_game_sp_demo] Version: ${PROJECT_VERSION}")

add_library(rtcw_game_sp_demo SHARED "")

set_target_properties(rtcw_game_sp_demo PROPERTIES
	PREFIX ""
	OUTPUT_NAME "qagame${RTCW_ARCH_STRING}_d"
)

target_include_directories(rtcw_game_sp_demo
	PRIVATE
		.
		../sp
		../../botai
		../../botai/sp
		../../game
		../../qcommon
		../../system
		../../ui/sp
)

target_compile_definitions(rtcw_game_sp_demo
	PRIVATE
		GAMEDLL
		WOLF_SP_DEMO
		RTCW_SP
		RTCW_SP_DEMO
		BUILDING_REF_GL
)

rtcw_configure_target(rtcw_game_sp_demo)

target_sources(rtcw_game_sp_demo
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
		../sp/ai_cast.cpp
		../sp/ai_cast.h
		../sp/ai_cast_characters.cpp
		../sp/ai_cast_debug.cpp
		../sp/ai_cast_events.cpp
		../sp/ai_cast_fight.cpp
		../sp/ai_cast_fight.h
		../sp/ai_cast_func_attack.cpp
		../sp/ai_cast_func_boss1.cpp
		../sp/ai_cast_funcs.cpp
		../sp/ai_cast_global.h
		../sp/ai_cast_script.cpp
		../sp/ai_cast_script_actions.cpp
		../sp/ai_cast_script_ents.cpp
		../sp/ai_cast_sight.cpp
		../sp/ai_cast_think.cpp
		../sp/bg_animation.cpp
		../sp/bg_local.h
		../sp/bg_misc.cpp
		../sp/bg_pmove.cpp
		../sp/bg_slidemove.cpp
		../sp/g_active.cpp
		../sp/g_alarm.cpp
		../sp/g_bot.cpp
		../sp/g_client.cpp
		../sp/g_cmds.cpp
		../sp/g_combat.cpp
		../sp/g_func_decs.h
		../sp/g_funcs.h
		../sp/g_items.cpp
		../sp/g_local.h
		../sp/g_main.cpp
		../sp/g_mem.cpp
		../sp/g_misc.cpp
		../sp/g_missile.cpp
		../sp/g_mover.cpp
		../sp/g_props.cpp
		../sp/g_save.cpp
		../sp/g_script.cpp
		../sp/g_script_actions.cpp
		../sp/g_session.cpp
		../sp/g_spawn.cpp
		../sp/g_svcmds.cpp
		../sp/g_syscalls.cpp
		../sp/g_target.cpp
		../sp/g_team.cpp
		../sp/g_team.h
		../sp/g_tramcar.cpp
		../sp/g_trigger.cpp
		../sp/g_utils.cpp
		../sp/g_weapon.cpp
	PRIVATE
		$<$<BOOL:${WIN32}>:../sp/game_sp.def>
)

install(
	TARGETS rtcw_game_sp_demo
	LIBRARY DESTINATION sp_demo
	RUNTIME DESTINATION sp_demo
)
