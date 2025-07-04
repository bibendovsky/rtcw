cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

message(STATUS "[rtcw]")

include(GetGitHeadHash)
get_git_hash(${CMAKE_CURRENT_SOURCE_DIR} RTCW_GIT_HASH LENGTH 8)
message(STATUS "[rtcw] Git hash: ${RTCW_GIT_HASH}")

set(RTCW_PACKAGE_VERSION_MAJOR 1)
set(RTCW_PACKAGE_VERSION_MINOR 0)
set(RTCW_PACKAGE_VERSION_PATCH 0)
set(RTCW_PACKAGE_VERSION_TWEAK 0)
set(RTCW_PACKAGE_VERSION "${RTCW_PACKAGE_VERSION_MAJOR}.${RTCW_PACKAGE_VERSION_MINOR}.${RTCW_PACKAGE_VERSION_PATCH}")

if(RTCW_GIT_HASH)
	set(RTCW_PACKAGE_VERSION "${RTCW_PACKAGE_VERSION}+${RTCW_GIT_HASH}")
endif()

option(RTCW_SP "Build single-player." ON)
message(STATUS "[rtcw] Single-player: ${RTCW_SP}")

option(RTCW_SP_DEMO "Build single-player demo." ON)
message(STATUS "[rtcw] Single-player: ${RTCW_SP_DEMO}")

option(RTCW_MP "Build multi-player." ON)
message(STATUS "[rtcw] Multiplayer: ${RTCW_MP}")

option(RTCW_MP_DED "Build multi-player (dedicated)." ON)
message(STATUS "[rtcw] Multiplayer (dedicated): ${RTCW_MP_DED}")

option(RTCW_ET "Build Enemy Territory." ON)
message(STATUS "[rtcw] Enemy Territory: ${RTCW_ET}")

option(RTCW_ET_DED "Build Enemy Territory (dedicated)." ON)
message(STATUS "[rtcw] Enemy Territory (dedicated): ${RTCW_ET_DED}")

option(RTCW_BUILD_SDL2 "Build SDL2 libraries from bundled source code." OFF)
message(STATUS "[rtcw] Build SDL2 libraries: ${RTCW_BUILD_SDL2}")

# Precompiled headers.
#
option(RTCW_USE_PCH "Enable precompiled headers if available." OFF)
message(STATUS "[${PROJECT_NAME}] Enable precompiled headers: ${RTCW_USE_PCH}")

# Multi-process compilation.
#
option(RTCW_MULTI_PROCESS_COMPILATION "Enable multi-process compilation if available." ON)
message(STATUS "[${PROJECT_NAME}] Enable multi-process compilation: ${RTCW_MULTI_PROCESS_COMPILATION}")

option(RTCW_CURL "Enable cURL." ON)
message(STATUS "[${PROJECT_NAME}] Enable cURL: ${RTCW_CURL}")

option(RTCW_CURL_STATIC "Link cURL statically." ON)
message(STATUS "[${PROJECT_NAME}] Link cURL statically: ${RTCW_CURL_STATIC}")

# Static linking.
#
option(RTCW_ENABLE_STATIC_LINKING "Enable static linking." ON)
message(STATUS "[${PROJECT_NAME}] Enable static linking: ${RTCW_ENABLE_STATIC_LINKING}")

if(NOT
	(RTCW_ET OR
		RTCW_ET_DED OR
		RTCW_MP OR
		RTCW_MP_DED OR
		RTCW_SP OR
		RTCW_SP_DEMO)
	)
	message(FATAL_ERROR "[rtcw] Nothing to build!")
endif()

define_property(GLOBAL PROPERTY RTCW_TAGS
	BRIEF_DOCS "RTCW tag list."
	FULL_DOCS "RTCW tag list."
)

include(rtcw)

add_subdirectory(src/lib/miniz)

if(RTCW_BUILD_SDL2)
	add_subdirectory(src/lib/sdl2)
	add_subdirectory(src/lib/sdl2_net)
endif()

add_subdirectory(src/lib/rtcw)

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

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/README.md
	DESTINATION .
	RENAME rtcw_readme.txt
)

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt
	DESTINATION .
	RENAME rtcw_license.txt
)

set(CPACK_PACKAGE_NAME "rtcw")
set(CPACK_PACKAGE_VENDOR "Boris I. Bendovsky")
set(CPACK_PACKAGE_VERSION_MAJOR ${RTCW_PACKAGE_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${RTCW_PACKAGE_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${RTCW_PACKAGE_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${RTCW_PACKAGE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory")
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR "ZIP")
include(CPack)
