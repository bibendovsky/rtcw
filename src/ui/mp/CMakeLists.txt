cmake_minimum_required (VERSION 3.8.2 FATAL_ERROR)
project (rtcw_ui_mp VERSION 1.0.0 LANGUAGES CXX)


message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")


add_library (${PROJECT_NAME} SHARED "")


set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		PREFIX ""
		OUTPUT_NAME "ui_mp_${RTCW_ARCH_STRING}"
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../../cgame
		../../game
		../../qcommon
		../../system
		../../ui
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		RTCW_MP
)

rtcw_configure_target (${PROJECT_NAME})

target_sources (
	${PROJECT_NAME}
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

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/mp/bg_misc.cpp
		ui_atoms.cpp
		ui_gameinfo.cpp
		ui_main.cpp
		ui_players.cpp
		ui_shared.cpp
		ui_syscalls.cpp
		ui_util.cpp
)

if (WIN32)
	target_sources (
		${PROJECT_NAME}
		PRIVATE
			ui_mp.def
	)
endif ()

install (
	TARGETS
		${PROJECT_NAME}
	LIBRARY
		DESTINATION sp-mp/main
	RUNTIME
		DESTINATION sp-mp/main
)
