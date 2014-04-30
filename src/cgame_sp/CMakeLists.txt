set(
    RTCW_TARGET
    rtcw_cgame_sp
)

add_definitions(-DCGAMEDLL)
add_definitions(-DRTCW_SP)

include_directories(
    .
    ../cgame
    ../game
    ../game_sp
    ../qcommon
    ../system
    ../ui
    ../ui_sp
)

set(
    RTCW_HEADERS
    ../game_sp/bg_local.h
    ../game/bg_public.h
    ../cgame_sp/cg_local.h
    ../cgame/cg_public.h
    ../ui/keycodes.h
    ../ui_sp/menudef.h
    ../game/q_shared.h
    ../game/surfaceflags.h
    ../cgame/tr_types.h
    ../ui_sp/ui_shared.h
)

set(
    RTCW_SOURCES
    ../game/q_math.cpp
    ../game/q_shared.cpp
    ../game_sp/bg_animation.cpp
    ../game_sp/bg_misc.cpp
    ../game_sp/bg_pmove.cpp
    cg_consolecmds.cpp
    cg_draw.cpp
    cg_drawtools.cpp
    cg_effects.cpp
    cg_ents.cpp
    cg_event.cpp
    cg_flamethrower.cpp
    cg_info.cpp
    cg_localents.cpp
    cg_main.cpp
    cg_marks.cpp
    cg_newDraw.cpp
    cg_particles.cpp
    cg_players.cpp
    cg_playerstate.cpp
    cg_predict.cpp
    cg_scoreboard.cpp
    cg_servercmds.cpp
    cg_snapshot.cpp
    cg_sound.cpp
    cg_syscalls.cpp
    cg_trails.cpp
    cg_view.cpp
    cg_weapons.cpp
    ../game_sp/bg_slidemove.cpp
    ../ui_sp/ui_shared.cpp
)

if (MSVC)
set(
    RTCW_EXTRA_SOURCES
    cgame_sp.def
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
    OUTPUT_NAME cgamex86
    RUNTIME_OUTPUT_DIRECTORY ${RTCW_SP_BIN_DIR}
)
