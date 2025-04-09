cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw VERSION 1.0.0)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

message(STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")

option(RTCW_SP "Build single-player." ON)
message(STATUS "[${PROJECT_NAME}] Single-player: ${RTCW_SP}")

option(RTCW_SP_DEMO "Build single-player demo." ON)
message(STATUS "[${PROJECT_NAME}] Single-player: ${RTCW_SP_DEMO}")

option(RTCW_MP "Build multi-player." ON)
message(STATUS "[${PROJECT_NAME}] Multiplayer: ${RTCW_MP}")

option(RTCW_MP_DED "Build multi-player (dedicated)." ON)
message(STATUS "[${PROJECT_NAME}] Multiplayer (dedicated): ${RTCW_MP_DED}")

option(RTCW_ET "Build Enemy Territory." ON)
message(STATUS "[${PROJECT_NAME}] Enemy Territory: ${RTCW_ET}")

option(RTCW_ET_DED "Build Enemy Territory (dedicated)." ON)
message(STATUS "[${PROJECT_NAME}] Enemy Territory (dedicated): ${RTCW_ET_DED}")

if(NOT
	(RTCW_ET OR
		RTCW_ET_DED OR
		RTCW_MP OR
		RTCW_MP_DED OR
		RTCW_SP OR
		RTCW_SP_DEMO)
	)
	message(FATAL_ERROR "[${PROJECT_NAME}] Nothing to build!")
endif()

include(rtcw)

add_subdirectory(src/lib/miniz)

if(RTCW_SP)
	add_subdirectory(src/rtcw/cgame/sp)
	add_subdirectory(src/rtcw/game/sp)
	add_subdirectory(src/rtcw/ui/sp)
	add_subdirectory(src/rtcw/rtcw_sp)
endif()

if(RTCW_SP_DEMO)
	add_subdirectory(src/rtcw/cgame/sp_demo)
	add_subdirectory(src/rtcw/game/sp_demo)
	add_subdirectory(src/rtcw/ui/sp_demo)
	add_subdirectory(src/rtcw/rtcw_sp_demo)
endif()

if(RTCW_MP)
	add_subdirectory(src/rtcw/cgame/mp)
	add_subdirectory(src/rtcw/game/mp)
	add_subdirectory(src/rtcw/ui/mp)
	add_subdirectory(src/rtcw/rtcw_mp)
endif()

if(RTCW_MP_DED)
	add_subdirectory(src/rtcw/rtcw_mp_ded)
endif()

if(RTCW_ET)
	add_subdirectory(src/rtcw/cgame/et)
	add_subdirectory(src/rtcw/game/et)
	add_subdirectory(src/rtcw/ui/et)
	add_subdirectory(src/rtcw/rtcw_et)
endif()

if(RTCW_ET_DED)
	add_subdirectory(src/rtcw/rtcw_et_ded)
endif()
