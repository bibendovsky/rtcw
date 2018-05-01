cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(rtcw VERSION 1.0.0)

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
	RTCW_BUILD_SP))
	message(FATAL_ERROR "Nothing to build!")
endif ()

if (RTCW_BUILD_SP)
	add_subdirectory(src/cgame_sp)
	add_subdirectory(src/game_sp)
	add_subdirectory(src/ui_sp)
	add_subdirectory(src/rtcw_sp)
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

if (RTCW_BUILD_ET)
	add_subdirectory(src/cgame_et)
	add_subdirectory(src/game_et)
	add_subdirectory(src/ui_et)
	add_subdirectory(src/rtcw_et)
endif ()

if (RTCW_BUILD_ET_DED)
	add_subdirectory(src/rtcw_et_ded)
endif ()
