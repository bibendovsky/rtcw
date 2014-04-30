set(
    RTCW_TARGET
    rtcw_game_mp
)

add_definitions(-DBUILDING_REF_GL)
add_definitions(-DGAMEDLL)
add_definitions(-DRTCW_MP)

include_directories(
    .
    ../botai
    ../botai_mp
    ../game
    ../qcommon
    ../system
    ../ui_mp
)

set(
    RTCW_HEADERS
    ai_cast.h
    ai_cast_fight.h
    ai_cast_global.h
    ../botai_mp/ai_chat.h
    ../botai_mp/ai_cmd.h
    ../botai_mp/ai_dmnet.h
    ../botai_mp/ai_dmq3.h
    ../botai_mp/ai_main.h
    ../botai_mp/ai_team.h
    ../game/be_aas.h
    ../game/be_ai_char.h
    ../game/be_ai_chat.h
    ../game/be_ai_gen.h
    ../game/be_ai_goal.h
    ../game/be_ai_move.h
    ../game/be_ai_weap.h
    ../game/be_ea.h
    ../game_et/bg_local.h
    ../game/bg_public.h
    ../botai/botai.h
    ../game/botlib.h
    ../botai_mp/chars.h
    g_local.h
    ../game/g_public.h
    g_team.h
    ../botai_mp/inv.h
    ../botai_mp/match.h
    ../game/q_shared.h
    ../game/surfaceflags.h
    ../botai_mp/syn.h
)

set(
    RTCW_SOURCES
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
    ../botai_mp/ai_chat.cpp
    ../botai_mp/ai_cmd.cpp
    ../botai_mp/ai_dmnet.cpp
    ../botai_mp/ai_dmq3.cpp
    ../botai_mp/ai_main.cpp
    ../botai_mp/ai_team.cpp
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

if (MSVC)
set(
    RTCW_EXTRA_SOURCES
    game_mp.def
)
endif ()

add_library(
    ${RTCW_TARGET} SHARED
    ${RTCW_HEADERS}
    ${RTCW_SOURCES}
    ${RTCW_EXTRA_SOURCES}
)

set_target_properties(
    ${RTCW_TARGET}
    PROPERTIES
    PREFIX ""
    OUTPUT_NAME qagame_mp_x86
    RUNTIME_OUTPUT_DIRECTORY ${RTCW_MP_BIN_DIR}
)
