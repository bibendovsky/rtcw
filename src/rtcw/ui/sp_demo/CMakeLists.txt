cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_ui_sp_demo VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_ui_sp_demo] Version: ${PROJECT_VERSION}")

add_library(rtcw_ui_sp_demo SHARED "")

set_target_properties(rtcw_ui_sp_demo PROPERTIES
	PREFIX ""
	OUTPUT_NAME "ui${RTCW_ARCH_STRING}_d"
)

if(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "OpenWatcom"))
	get_target_property(RTCW_TMP_LINK_FLAGS rtcw_ui_sp_demo LINK_FLAGS)

	if(NOT RTCW_TMP_LINK_FLAGS)
		set(RTCW_TMP_LINK_FLAGS "")
	endif()

	set(RTCW_TMP_LINK_FLAGS "${RTCW_TMP_LINK_FLAGS} export dllEntry=_dllEntry,vmMain=_vmMain")

	set_target_properties(rtcw_ui_sp_demo PROPERTIES LINK_FLAGS "${RTCW_TMP_LINK_FLAGS}")
endif()

target_include_directories(rtcw_ui_sp_demo
	PRIVATE
		.
		../sp
		../../cgame
		../../game
		../../qcommon
		../../system
		../../ui
)

target_compile_definitions(rtcw_ui_sp_demo
	PRIVATE
		WOLF_SP_DEMO
		RTCW_SP
		RTCW_SP_DEMO
)

rtcw_configure_target(rtcw_ui_sp_demo)

target_sources(rtcw_ui_sp_demo
	PRIVATE
		../../cgame/tr_types.h
		../../game/bg_public.h
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/q_shared.h
		../../game/sp/bg_misc.cpp
		../../game/surfaceflags.h
		../../qcommon/rtcw_vm_args.h
		../../ui/keycodes.h
		../../ui/ui_public.h
		../sp/menudef.h
		../sp/ui_atoms.cpp
		../sp/ui_gameinfo.cpp
		../sp/ui_local.h
		../sp/ui_main.cpp
		../sp/ui_players.cpp
		../sp/ui_shared.cpp
		../sp/ui_shared.h
		../sp/ui_syscalls.cpp
		../sp/ui_util.cpp
)

install(
	TARGETS rtcw_ui_sp_demo
	LIBRARY DESTINATION sp_demo
	RUNTIME DESTINATION sp_demo
)
