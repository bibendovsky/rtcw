set(
    RTCW_TARGET
    rtcw_cgame_et
)

add_definitions(-DCGAMEDLL)
add_definitions(-DRTCW_ET)

include_directories(
    .
    ../cgame
    ../game
    ../game_et
    ../qcommon
    ../system
    ../ui
    ../ui_et
)

set(
    RTCW_HEADERS
    ../game_et/bg_local.h
    ../game/bg_public.h
    ../cgame_et/cg_local.h
    ../cgame/cg_public.h
    ../ui/keycodes.h
    ../ui_et/menudef.h
    ../game/q_shared.h
    ../game/surfaceflags.h
    ../cgame/tr_types.h
    ../ui_et/ui_shared.h
)

set(
    RTCW_SOURCES
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
    ../game_et/bg_tracemap.cpp
    ../ui_et/ui_shared.cpp
)

if (MSVC)
set(
    RTCW_EXTRA_SOURCES
    cgame_et.def
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
    OUTPUT_NAME cgame_mp_x86
    RUNTIME_OUTPUT_DIRECTORY ${RTCW_ET_BIN_DIR}
)
