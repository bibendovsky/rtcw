set(
    RTCW_TARGET
    rtcw_ui_mp
)

add_definitions(-DRTCW_MP)

include_directories(
    .
    ../cgame
    ../game
    ../qcommon
    ../system
    ../ui
)

set(
    RTCW_HEADERS
    ../game/bg_public.h
    ../ui/keycodes.h
    menudef.h
    ../game/q_shared.h
    ../game/surfaceflags.h
    ../cgame/tr_types.h
    ui_local.h
    ../ui/ui_public.h
    ui_shared.h
)

set(
    RTCW_SOURCES
    ../game/q_math.cpp
    ../game/q_shared.cpp
    ../game_mp/bg_misc.cpp
    ui_atoms.cpp
    ui_gameinfo.cpp
    ui_main.cpp
    ui_players.cpp
    ui_shared.cpp
    ui_syscalls.cpp
    ui_util.cpp
)

if (MSVC)
set(
    RTCW_EXTRA_SOURCES
    ui_mp.def
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
    OUTPUT_NAME ui_mp_x86
    RUNTIME_OUTPUT_DIRECTORY ${RTCW_MP_BIN_DIR}
)
