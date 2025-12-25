cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_ui_et VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_ui_et] Version: ${PROJECT_VERSION}")

add_library(rtcw_ui_et SHARED "")

set_target_properties(rtcw_ui_et PROPERTIES
	RTCW_TAGS "et;ui;dll"
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
		../../game/et/bg_campaign.cpp
		../../game/et/bg_classes.cpp
		../../game/et/bg_misc.cpp
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
		ui_loadpanel.cpp
		ui_local.h
		ui_main.cpp
		ui_players.cpp
		ui_shared.cpp
		ui_shared.h
		ui_syscalls.cpp
		ui_util.cpp
)

install(
	TARGETS rtcw_ui_et
	LIBRARY DESTINATION etmain
	RUNTIME DESTINATION etmain
)
