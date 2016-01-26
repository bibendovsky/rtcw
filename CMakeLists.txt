project("rtcw")
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include("rtcw")

option(RTCW_BUILD_SP "Build single player" ON)
option(RTCW_BUILD_MP "Build multiplayer" ON)
option(RTCW_BUILD_MP_DED "Build multiplayer (dedicated)" ON)
option(RTCW_BUILD_ET "Build Enemy Territory" ON)
option(RTCW_BUILD_ET_DED "Build Enemy Territory (dedicated)" ON)
option(RTCW_USE_CURL "Use cURL (Enemy Territory)" ON)

if (NOT (RTCW_BUILD_ET OR
    RTCW_BUILD_ET_DED OR
    RTCW_BUILD_MP OR
    RTCW_BUILD_MP_DED OR
    RTCW_BUILD_SP)
)
    message(FATAL_ERROR "Nothing to build!")
endif ()

find_package(SDL 2 REQUIRED)

if (RTCW_BUILD_ET AND RTCW_USE_CURL)
    find_package(CURL 7 REQUIRED)

    if (WIN32)
        option(RTCW_STATIC_CURL "Link cURL library statically" ON)
    endif ()
endif ()

set(RTCW_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(RTCW_SP_BIN_DIR "${RTCW_BIN_DIR}/sp")
set(RTCW_MP_BIN_DIR "${RTCW_BIN_DIR}/mp")
set(RTCW_ET_BIN_DIR "${RTCW_BIN_DIR}/et")

rtcw_set_common_options()

include_directories(
    "${SDL_INCLUDE_DIR}"
)

if (RTCW_BUILD_SP)
    add_subdirectory("src/cgame_sp")
    add_subdirectory("src/game_sp")
    add_subdirectory("src/ui_sp")
    add_subdirectory("src/rtcw_sp")
endif ()

if (RTCW_BUILD_MP)
    add_subdirectory("src/cgame_mp")
    add_subdirectory("src/game_mp")
    add_subdirectory("src/ui_mp")
    add_subdirectory("src/rtcw_mp")
endif ()

if (RTCW_BUILD_MP_DED)
    add_subdirectory("src/rtcw_mp_ded")
endif ()

if (RTCW_BUILD_ET)
    add_subdirectory("src/cgame_et")
    add_subdirectory("src/game_et")
    add_subdirectory("src/ui_et")
    add_subdirectory("src/rtcw_et")
endif ()

if (RTCW_BUILD_ET_DED)
    add_subdirectory("src/rtcw_et_ded")
endif ()
