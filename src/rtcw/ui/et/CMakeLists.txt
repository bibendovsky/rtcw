cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_ui_et VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_ui_et] Version: ${PROJECT_VERSION}")

add_library(rtcw_ui_et SHARED "")

set_target_properties(rtcw_ui_et PROPERTIES
	PREFIX ""
	OUTPUT_NAME "ui_mp_${RTCW_ARCH_STRING}"
)

target_include_directories(rtcw_ui_et
	PRIVATE
		.
		../../cgame
		../../game
		../../qcommon
		../../system
		../../ui
)

target_compile_definitions(rtcw_ui_et
	PRIVATE
		RTCW_ET
)

rtcw_configure_target(rtcw_ui_et)

target_sources(rtcw_ui_et
	PRIVATE
		../../cgame/tr_types.h
		../../game/bg_public.h
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../qcommon/rtcw_vm_args.h
		../../ui/keycodes.h
		../../ui/ui_public.h
		menudef.h
		ui_local.h
		ui_shared.h
)

target_sources(rtcw_ui_et
	PRIVATE
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/et/bg_campaign.cpp
		../../game/et/bg_classes.cpp
		../../game/et/bg_misc.cpp
		ui_atoms.cpp
		ui_gameinfo.cpp
		ui_loadpanel.cpp
		ui_main.cpp
		ui_players.cpp
		ui_shared.cpp
		ui_syscalls.cpp
		ui_util.cpp
)

if(WIN32)
	target_sources(rtcw_ui_et PRIVATE ui_et.def)
endif()

install(
	TARGETS rtcw_ui_et
	LIBRARY DESTINATION et/etmain
	RUNTIME DESTINATION et/etmain
)
