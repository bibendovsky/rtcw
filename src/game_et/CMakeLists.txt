set(
    RTCW_TARGET
    rtcw_game_et
)

add_definitions(-DBUILDING_REF_GL)
add_definitions(-DGAMEDLL)
add_definitions(-DRTCW_ET)

include_directories(
    .
    ../botai
    ../botai_et
    ../game
    ../qcommon
    ../system
    ../ui_et
)

set(
    RTCW_HEADERS
    ../botai_et/ai_cmd.h
    ../botai_et/ai_distances.h
    ../botai_et/ai_dmgoal_mp.h
    ../botai_et/ai_dmnet_mp.h
    ../botai_et/ai_dmq3.h
    ../botai_et/ai_main.h
    ../botai_et/ai_matrix.h
    ../botai_et/ai_team.h
    ../game/be_aas.h
    ../game/be_ai_char.h
    ../game/be_ai_chat.h
    ../game/be_ai_gen.h
    ../game/be_ai_goal.h
    ../game/be_ai_move.h
    ../game/be_ai_weap.h
    ../game/be_ea.h
    bg_local.h
    ../game/bg_public.h
    ../botai/botai.h
    ../game/botlib.h
    ../botai_et/chars.h
    g_local.h
    ../game/g_public.h
    g_team.h
    ../botai_et/inv.h
    ../botai_et/match.h
    ../game/q_shared.h
    ../game/surfaceflags.h
    ../botai_et/syn.h
)

set(
    RTCW_SOURCES
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

if (MSVC)
set(
    RTCW_EXTRA_SOURCES
    game_et.def
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
    RUNTIME_OUTPUT_DIRECTORY ${RTCW_ET_BIN_DIR}
)
