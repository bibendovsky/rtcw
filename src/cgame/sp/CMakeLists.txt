cmake_minimum_required (VERSION 3.8.2 FATAL_ERROR)
project (rtcw_cgame_sp VERSION 1.0.0 LANGUAGES CXX)

message (STATUS "")
message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")

option (RTCW_MULTI_PROCESS_COMPILATION "Enable multi-process compilation if available." ON)
message (STATUS "[${PROJECT_NAME}] Multi-process compilation: ${RTCW_MULTI_PROCESS_COMPILATION}")

# Precompiled headers.
#
option (RTCW_USE_PCH "Use precompiled headers if available." ON)
message (STATUS "[${PROJECT_NAME}] Precompiled headers: ${RTCW_USE_PCH}")


find_package (SDL2W 2.0.4 REQUIRED)

add_library (${PROJECT_NAME} SHARED "")

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		PREFIX ""
		OUTPUT_NAME cgamex86
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../../cgame
		../../game
		../../game/sp
		../../qcommon
		../../system
		../../ui
		../../ui/sp
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		CGAMEDLL
		RTCW_SP
)

if (MSVC)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			_CRT_SECURE_NO_WARNINGS
	)
endif ()

if (RTCW_MULTI_PROCESS_COMPILATION)
	if (MSVC)
		target_compile_options (
			${PROJECT_NAME}
			PRIVATE
				-MP
		)
	endif ()
endif ()


#
# Precompile headers.
#
if (RTCW_USE_PCH AND NOT (${CMAKE_VERSION} VERSION_LESS "3.16"))
	target_precompile_headers(
		${PROJECT_NAME}
		PRIVATE
			rtcw_pch.h
	)
endif ()


target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		SDL2W::SDL2W
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../cgame/cg_public.h
		../../cgame/tr_types.h
		../../cgame/sp/cg_local.h
		../../game/bg_public.h
		../../game/q_shared.h
		../../game/surfaceflags.h
		../../game/sp/bg_local.h
		../../ui/keycodes.h
		../../ui/sp/menudef.h
		../../ui/sp/ui_shared.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../game/q_math.cpp
		../../game/q_shared.cpp
		../../game/sp/bg_animation.cpp
		../../game/sp/bg_misc.cpp
		../../game/sp/bg_pmove.cpp
		../../game/sp/bg_slidemove.cpp
		../../ui/sp/ui_shared.cpp
		cg_consolecmds.cpp
		cg_draw.cpp
		cg_drawtools.cpp
		cg_effects.cpp
		cg_ents.cpp
		cg_event.cpp
		cg_flamethrower.cpp
		cg_info.cpp
		cg_localents.cpp
		cg_main.cpp
		cg_marks.cpp
		cg_newDraw.cpp
		cg_particles.cpp
		cg_players.cpp
		cg_playerstate.cpp
		cg_predict.cpp
		cg_scoreboard.cpp
		cg_servercmds.cpp
		cg_snapshot.cpp
		cg_sound.cpp
		cg_syscalls.cpp
		cg_trails.cpp
		cg_view.cpp
		cg_weapons.cpp
)

if (WIN32)
	target_sources (
		${PROJECT_NAME}
		PRIVATE
			cgame_sp.def
	)
endif ()

install (
	TARGETS
	${PROJECT_NAME}
	RUNTIME DESTINATION sp
)
