cmake_minimum_required(VERSION 2.8)

option(RTCW_BUILD_ET "Build Enemy Territory" ON)
option(RTCW_BUILD_ET_DED "Build Enemy Territory (dedicated)" ON)
option(RTCW_BUILD_MP "Build multiplayer" ON)
option(RTCW_BUILD_MP_DED "Build multiplayer (dedicated)" ON)
option(RTCW_BUILD_SP "Build single player" ON)

if ((NOT RTCW_BUILD_ET) AND
    (NOT RTCW_BUILD_ET_DED) AND
    (NOT RTCW_BUILD_MP) AND
    (NOT RTCW_BUILD_MP_DED) AND
    (NOT RTCW_BUILD_SP)
)
    message(FATAL_ERROR "Nothing to build!")
endif ()

find_package(SDL 2 REQUIRED)
find_package(ZLIB 1.2.8 REQUIRED)

if (RTCW_BUILD_ET)
    find_package(CURL 7.36 REQUIRED)

    if (WIN32)
        option(RTCW_STATIC_CURL "Link cURL library statically" ON)
    endif ()
endif ()

if (RTCW_BUILD_ET OR RTCW_BUILD_MP OR RTCW_BUILD_SP)
    find_package(GLEW 1.10 REQUIRED)
    find_package(JPEG REQUIRED)

    if (WIN32)
        option(RTCW_STATIC_GLEW "Link GLEW library statically" ON)
    endif ()
endif ()

project(rtcw)

set(
    RTCW_BIN_DIR
    ${CMAKE_BINARY_DIR}/bin
)

set(
    RTCW_SP_BIN_DIR
    ${RTCW_BIN_DIR}/sp
)

set(
    RTCW_MP_BIN_DIR
    ${RTCW_BIN_DIR}/mp
)

set(
    RTCW_ET_BIN_DIR
    ${RTCW_BIN_DIR}/et
)

include_directories(
    ${SDL_INCLUDE_DIR}
)

if (RTCW_BUILD_ET)
add_subdirectory(src/cgame_et)
add_subdirectory(src/game_et)
add_subdirectory(src/ui_et)
add_subdirectory(src/rtcw_et)
endif ()

if (RTCW_BUILD_ET_DED)
    add_subdirectory(src/rtcw_et_ded)
endif ()

if (RTCW_BUILD_MP)
add_subdirectory(src/cgame_mp)
add_subdirectory(src/game_mp)
add_subdirectory(src/ui_mp)
add_subdirectory(src/rtcw_mp)
endif ()

if (RTCW_BUILD_MP_DED)
    add_subdirectory(src/rtcw_mp_ded)
endif ()

if (RTCW_BUILD_SP)
add_subdirectory(src/cgame_sp)
add_subdirectory(src/game_sp)
add_subdirectory(src/ui_sp)
add_subdirectory(src/rtcw_sp)
endif ()
