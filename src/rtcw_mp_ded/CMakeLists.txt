cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (rtcw_mp_ded VERSION 1.0.0 LANGUAGES CXX)


message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")


add_executable (${PROJECT_NAME} "")

add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND $<$<CONFIG:RELEASE>:${CMAKE_STRIP}> $<$<CONFIG:RELEASE>:$<TARGET_FILE:${PROJECT_NAME}>>
	COMMENT "Strip executable."
	VERBATIM
)

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME "rtcw_mp_ded_${RTCW_ARCH_STRING}"
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../botai
		../cgame
		../client
		../game
		../qcommon
		../renderer
		../system
		../ui
		../win32
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		DEDICATED
		DO_LIGHT_DEDICATED
		BOTLIB
		RTCW_MP
)

rtcw_configure_target (${PROJECT_NAME})

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../botai/botai.h
		../botlib/be_aas_bsp.h
		../botlib/be_aas_cluster.h
		../botlib/be_aas_debug.h
		../botlib/be_aas_def.h
		../botlib/be_aas_entity.h
		../botlib/be_aas_file.h
		../botlib/be_aas_funcs.h
		../botlib/be_aas_main.h
		../botlib/be_aas_move.h
		../botlib/be_aas_optimize.h
		../botlib/be_aas_reach.h
		../botlib/be_aas_route.h
		../botlib/be_aas_routealt.h
		../botlib/be_aas_routetable.h
		../botlib/be_aas_sample.h
		../botlib/be_ai_weight.h
		../botlib/be_interface.h
		../botlib/l_crc.h
		../botlib/l_libvar.h
		../botlib/l_log.h
		../botlib/l_memory.h
		../botlib/l_precomp.h
		../botlib/l_script.h
		../botlib/l_struct.h
		../botlib/l_utils.h
		../cgame/cg_public.h
		../cgame/tr_types.h
		../client/client.h
		../client/keys.h
		../client/snd_local.h
		../client/snd_public.h
		../game/be_aas.h
		../game/be_ai_char.h
		../game/be_ai_chat.h
		../game/be_ai_gen.h
		../game/be_ai_goal.h
		../game/be_ai_move.h
		../game/be_ai_weap.h
		../game/be_ea.h
		../game/bg_public.h
		../game/botlib.h
		../game/g_public.h
		../game/q_shared.h
		../game/surfaceflags.h
		../qcommon/cm_local.h
		../qcommon/cm_patch.h
		../qcommon/cm_polylib.h
		../qcommon/cm_public.h
		../qcommon/qcommon.h
		../qcommon/qfiles.h
		../qcommon/rtcw_bit_array_single_unit.cpp
		../qcommon/rtcw_bit_array_single_unit.h
		../qcommon/rtcw_string.cpp
		../qcommon/rtcw_string.h
		../qcommon/vm_local.h
		../renderer/anorms256.h
		../renderer/qgl.h
		../renderer/rtcw_cgm_clip_space.cpp
		../renderer/rtcw_cgm_clip_space.h
		../renderer/rtcw_cgm_mat.cpp
		../renderer/rtcw_cgm_mat.h
		../renderer/rtcw_cgm_transform.cpp
		../renderer/rtcw_cgm_transform.h
		../renderer/rtcw_cgm_vec.cpp
		../renderer/rtcw_cgm_vec.h
		../renderer/rtcw_ogl_matrix_stack.h
		../renderer/rtcw_ogl_program.h
		../renderer/rtcw_ogl_tess_program.h
		../renderer/rtcw_ogl_tess_state.h
		../renderer/tr_local.h
		../renderer/tr_public.h
		../server/server.h
		../splines/math_angles.h
		../splines/math_matrix.h
		../splines/math_quaternion.h
		../splines/math_vector.h
		../splines/splines.h
		../splines/util_list.h
		../splines/util_str.h
		../system/rtcw_endian.h
		../system/rtcw_input_joystick.h
		../system/rtcw_input_keyboard.h
		../system/rtcw_input_mouse.h
		../system/sys_events.h
		../system/sys_input.h
		../system/sys_local.h
		../system/sys_shared.h
		../ui/keycodes.h
		../ui/ui_public.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../botlib/be_aas_bspq3.cpp
		../botlib/be_aas_cluster.cpp
		../botlib/be_aas_debug.cpp
		../botlib/be_aas_entity.cpp
		../botlib/be_aas_file.cpp
		../botlib/be_aas_main.cpp
		../botlib/be_aas_move.cpp
		../botlib/be_aas_optimize.cpp
		../botlib/be_aas_reach.cpp
		../botlib/be_aas_route.cpp
		../botlib/be_aas_routealt.cpp
		../botlib/be_aas_routetable.cpp
		../botlib/be_aas_sample.cpp
		../botlib/be_ai_char.cpp
		../botlib/be_ai_chat.cpp
		../botlib/be_ai_gen.cpp
		../botlib/be_ai_goal.cpp
		../botlib/be_ai_move.cpp
		../botlib/be_ai_weap.cpp
		../botlib/be_ai_weight.cpp
		../botlib/be_ea.cpp
		../botlib/be_interface.cpp
		../botlib/l_crc.cpp
		../botlib/l_libvar.cpp
		../botlib/l_log.cpp
		../botlib/l_memory.cpp
		../botlib/l_precomp.cpp
		../botlib/l_script.cpp
		../botlib/l_struct.cpp
		../game/q_math.cpp
		../game/q_shared.cpp
		../null/null_client.cpp
		../null/null_input.cpp
		../null/null_snddma.cpp
		../qcommon/cm_load.cpp
		../qcommon/cm_patch.cpp
		../qcommon/cm_polylib.cpp
		../qcommon/cm_test.cpp
		../qcommon/cm_trace.cpp
		../qcommon/cmd.cpp
		../qcommon/common.cpp
		../qcommon/cvar.cpp
		../qcommon/files.cpp
		../qcommon/files_posix.cpp
		../qcommon/files_win32.cpp
		../qcommon/huffman.cpp
		../qcommon/md4.cpp
		../qcommon/msg.cpp
		../qcommon/net_chan.cpp
		../qcommon/vm.cpp
		../server/sv_bot.cpp
		../server/sv_ccmds.cpp
		../server/sv_client.cpp
		../server/sv_game.cpp
		../server/sv_init.cpp
		../server/sv_main.cpp
		../server/sv_net_chan.cpp
		../server/sv_snapshot.cpp
		../server/sv_world.cpp
		../system/sys_events.cpp
		../system/sys_main.cpp
		../system/sys_net.cpp
		../system/sys_shared.cpp
		../system/sys_shared_posix.cpp
		../system/sys_shared_win32.cpp
		../system/sys_syscon.cpp
		../system/rtcw_array_trivial.cpp
		../system/rtcw_array_trivial.h
		../system/rtcw_sdl_utility.cpp
		../system/rtcw_sdl_utility.h
		../system/rtcw_syscon.cpp
		../system/rtcw_syscon.h
		../system/rtcw_syscon_font_16x8.cpp
		../system/rtcw_syscon_font_16x8.h
		../system/rtcw_type_utility.cpp
		../system/rtcw_type_utility.h
		../system/rtcw_vector_trivial.cpp
		../system/rtcw_vector_trivial.h
		../system/rtcw_window_rounded_corner_mgr.cpp
		../system/rtcw_window_rounded_corner_mgr.h
)

if (WIN32)
	target_sources(${PROJECT_NAME} PRIVATE ../win32/rtcw.rc)

	if (CMAKE_CXX_COMPILER_ID STREQUAL OpenWatcom)
		add_custom_command(
			TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND wrc.exe
				/DRTCW_MP
				/bt=nt
				/i=$<SHELL_PATH:${PROJECT_SOURCE_DIR}/../win32>
				/fo=$<SHELL_PATH:${PROJECT_BINARY_DIR}/rtcw.res>
				$<SHELL_PATH:${PROJECT_SOURCE_DIR}/../win32/rtcw.rc>
				$<SHELL_PATH:$<TARGET_FILE:${PROJECT_NAME}>>
			COMMENT "[${PROJECT_NAME}] Embed resources."
			VERBATIM
		)
	endif ()
endif ()

install (
	TARGETS
		${PROJECT_NAME}
	RUNTIME
		DESTINATION mp
)
