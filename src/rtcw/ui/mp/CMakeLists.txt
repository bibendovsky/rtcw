cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_ui_mp VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_ui_mp] Version: ${PROJECT_VERSION}")

add_library(rtcw_ui_mp SHARED "")

set_target_properties(rtcw_ui_mp PROPERTIES
	PREFIX ""
	OUTPUT_NAME "ui_mp_${RTCW_ARCH_STRING}"
)

if(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom"))
	get_target_property(RTCW_TMP_LINK_FLAGS rtcw_ui_mp LINK_FLAGS)

	if(NOT RTCW_TMP_LINK_FLAGS)
		set(RTCW_TMP_LINK_FLAGS "")
	endif()

	set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} export dllEntry=_dllEntry,vmMain=_vmMain")

	set_target_properties(rtcw_ui_mp PROPERTIES LINK_FLAGS "${RTCW_TMP_LINK_FLAGS}")
endif()

target_include_directories(rtcw_ui_mp
	PRIVATE
		.
		../../cgame
		../../game
		../../qcommon
		../../system
		../../ui
)

target_compile_definitions(rtcw_ui_mp
	PRIVATE
		RTCW_MP
)

rtcw_configure_target(rtcw_ui_mp)

target_sources(rtcw_ui_mp
	PRIVATE
		../../cgame/tr_types.h
		../../game/bg_public.h
		../../game/mp/bg_misc.cpp
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../qcommon/rtcw_vm_args.h
		../../ui/keycodes.h
		../../ui/ui_public.h
		menudef.h
		ui_atoms.cpp
		ui_gameinfo.cpp
		ui_local.h
		ui_main.cpp
		ui_players.cpp
		ui_shared.cpp
		ui_shared.h
		ui_syscalls.cpp
		ui_util.cpp
)

install(
	TARGETS rtcw_ui_mp
	LIBRARY DESTINATION mp/main
	RUNTIME DESTINATION mp/main
)
