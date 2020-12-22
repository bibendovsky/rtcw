cmake_minimum_required(VERSION 3.8.2 FATAL_ERROR)
project(rtcw VERSION 1.0.0)

set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

message (STATUS "")
message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")

option (RTCW_SP "Build single-player." ON)
message (STATUS "[${PROJECT_NAME}] Single-player: ${RTCW_SP}")

option (RTCW_MP "Build multiplayer." ON)
message (STATUS "[${PROJECT_NAME}] Multiplayer: ${RTCW_MP}")

option (RTCW_MP_DED "Build multiplayer (dedicated)." ON)
message (STATUS "[${PROJECT_NAME}] Multiplayer (dedicated): ${RTCW_MP_DED}")

option (RTCW_ET "Build Enemy Territory." ON)
message (STATUS "[${PROJECT_NAME}] Enemy Territory: ${RTCW_ET}")

option (RTCW_ET_DED "Build Enemy Territory (dedicated)." ON)
message (STATUS "[${PROJECT_NAME}] Enemy Territory (dedicated): ${RTCW_ET_DED}")

if (NOT (RTCW_ET OR
	RTCW_ET_DED OR
	RTCW_MP OR
	RTCW_MP_DED OR
	RTCW_SP))
	message (FATAL_ERROR "[${PROJECT_NAME}] Nothing to build!")
endif ()

add_subdirectory (src/lib/dear_imgui)
add_subdirectory (src/lib/miniz)

if (RTCW_SP)
	add_subdirectory (src/cgame/sp)
	add_subdirectory (src/game/sp)
	add_subdirectory (src/ui/sp)
	add_subdirectory (src/rtcw_sp)
endif ()

if (RTCW_MP)
	add_subdirectory (src/cgame/mp)
	add_subdirectory (src/game/mp)
	add_subdirectory (src/ui/mp)
	add_subdirectory (src/rtcw_mp)
endif ()

if (RTCW_MP_DED)
	add_subdirectory (src/rtcw_mp_ded)
endif ()

if (RTCW_ET)
	add_subdirectory (src/cgame/et)
	add_subdirectory (src/game/et)
	add_subdirectory (src/ui/et)
	add_subdirectory (src/rtcw_et)
endif ()

if (RTCW_ET_DED)
	add_subdirectory (src/rtcw_et_ded)
endif ()
