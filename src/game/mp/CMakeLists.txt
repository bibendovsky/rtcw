cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (rtcw_game_mp VERSION 1.0.0 LANGUAGES CXX)


message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")


add_library (${PROJECT_NAME} SHARED "")


set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		PREFIX ""
		OUTPUT_NAME "qagame_mp_${RTCW_ARCH_STRING}"
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../../botai
		../../botai/mp
		../../game
		../../qcommon
		../../system
		../../ui/mp
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		GAMEDLL
		RTCW_MP
		BUILDING_REF_GL
)

rtcw_configure_target (${PROJECT_NAME})

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../botai/botai.h
		../../botai/mp/ai_chat.h
		../../botai/mp/ai_cmd.h
		../../botai/mp/ai_dmnet.h
		../../botai/mp/ai_dmq3.h
		../../botai/mp/ai_main.h
		../../botai/mp/ai_team.h
		../../botai/mp/chars.h
		../../botai/mp/inv.h
		../../botai/mp/match.h
		../../botai/mp/syn.h
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
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../game/et/bg_local.h
		../../qcommon/rtcw_vm_args.h
		ai_cast.h
		ai_cast_fight.h
		ai_cast_global.h
		g_local.h
		g_team.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../botai/mp/ai_chat.cpp
		../../botai/mp/ai_cmd.cpp
		../../botai/mp/ai_dmnet.cpp
		../../botai/mp/ai_dmq3.cpp
		../../botai/mp/ai_main.cpp
		../../botai/mp/ai_team.cpp
		../../game/q_math.cpp
		../../game/q_shared.cpp
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
		g_antilag.cpp
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
	target_sources (
		${PROJECT_NAME}
		PRIVATE
			game_mp.def
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
