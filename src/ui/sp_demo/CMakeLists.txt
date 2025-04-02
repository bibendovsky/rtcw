cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (rtcw_ui_sp_demo VERSION 1.0.0 LANGUAGES CXX)


message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")


add_library (${PROJECT_NAME} SHARED "")


set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		PREFIX ""
		OUTPUT_NAME "ui${RTCW_ARCH_STRING}_d"
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../sp
		../../cgame
		../../game
		../../qcommon
		../../system
		../../ui
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		WOLF_SP_DEMO
		RTCW_SP
		RTCW_SP_DEMO
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
		../sp/menudef.h
		../sp/ui_local.h
		../sp/ui_shared.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/sp/bg_misc.cpp
		../sp/ui_atoms.cpp
		../sp/ui_gameinfo.cpp
		../sp/ui_main.cpp
		../sp/ui_players.cpp
		../sp/ui_shared.cpp
		../sp/ui_syscalls.cpp
		../sp/ui_util.cpp
)

if (WIN32)
	target_sources (
		${PROJECT_NAME}
		PRIVATE
			../sp/ui_sp.def
	)
endif ()

install (
	TARGETS
		${PROJECT_NAME}
	LIBRARY
		DESTINATION sp_demo
	RUNTIME
		DESTINATION sp_demo
)
