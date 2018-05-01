cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(rtcw_mp VERSION 1.0.0 LANGUAGES CXX C)

find_package(OpenGL REQUIRED)
find_package(SDL 2.0.4 REQUIRED)
find_package(SDL_net 2.0.1 REQUIRED)

set(
	headers
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
	../qcommon/vm_local.h
	../renderer/anorms256.h
	../renderer/jpgd.h
	../renderer/jpge.h
	../renderer/qgl.h
	../renderer/rtcw_jpeg_reader.h
	../renderer/rtcw_jpeg_writer.h
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
	../ui/keycodes.h
	../ui/ui_public.h
	../win32/glw_win.h
	../win32/rtcw_mp_resource.h
	../win32/win_local.h
	../zlib/contrib/minizip/ioapi.h
	../zlib/contrib/minizip/miniz.h
	../zlib/contrib/minizip/unzip.h
)

if (WIN32)
	list(
		APPEND
		headers
		../win32/rtcw_mp_resource.h
	)
endif ()

set(
	sources
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
	../client/cl_cgame.cpp
	../client/cl_cin.cpp
	../client/cl_console.cpp
	../client/cl_input.cpp
	../client/cl_keys.cpp
	../client/cl_main.cpp
	../client/cl_net_chan.cpp
	../client/cl_parse.cpp
	../client/cl_scrn.cpp
	../client/cl_ui.cpp
	../client/snd_adpcm.cpp
	../client/snd_dma.cpp
	../client/snd_dma_sdl.cpp
	../client/snd_mem.cpp
	../client/snd_mix.cpp
	../client/snd_wavelet.cpp
	../game/q_math.cpp
	../game/q_shared.cpp
	../qcommon/cm_load.cpp
	../qcommon/cm_patch.cpp
	../qcommon/cm_polylib.cpp
	../qcommon/cm_test.cpp
	../qcommon/cm_trace.cpp
	../qcommon/cmd.cpp
	../qcommon/common.cpp
	../qcommon/cvar.cpp
	../qcommon/files.cpp
	../qcommon/huffman.cpp
	../qcommon/md4.cpp
	../qcommon/msg.cpp
	../qcommon/net_chan.cpp
	../qcommon/vm.cpp
	../renderer/jpgd.cpp
	../renderer/jpge.cpp
	../renderer/rtcw_jpeg_reader.cpp
	../renderer/rtcw_jpeg_writer.cpp
	../renderer/rtcw_ogl_matrix_stack.cpp
	../renderer/rtcw_ogl_program.cpp
	../renderer/rtcw_ogl_tess_program.cpp
	../renderer/rtcw_ogl_tess_state.cpp
	../renderer/tr_animation.cpp
	../renderer/tr_backend.cpp
	../renderer/tr_bsp.cpp
	../renderer/tr_cmds.cpp
	../renderer/tr_cmesh.cpp
	../renderer/tr_curve.cpp
	../renderer/tr_flares.cpp
	../renderer/tr_font.cpp
	../renderer/tr_image.cpp
	../renderer/tr_init.cpp
	../renderer/tr_light.cpp
	../renderer/tr_main.cpp
	../renderer/tr_marks.cpp
	../renderer/tr_mesh.cpp
	../renderer/tr_model.cpp
	../renderer/tr_noise.cpp
	../renderer/tr_scene.cpp
	../renderer/tr_shade.cpp
	../renderer/tr_shade_calc.cpp
	../renderer/tr_shader.cpp
	../renderer/tr_shadows.cpp
	../renderer/tr_sky.cpp
	../renderer/tr_surface.cpp
	../renderer/tr_world.cpp
	../server/sv_bot.cpp
	../server/sv_ccmds.cpp
	../server/sv_client.cpp
	../server/sv_game.cpp
	../server/sv_init.cpp
	../server/sv_main.cpp
	../server/sv_net_chan.cpp
	../server/sv_snapshot.cpp
	../server/sv_world.cpp
	../splines/math_angles.cpp
	../splines/math_matrix.cpp
	../splines/math_quaternion.cpp
	../splines/math_vector.cpp
	../splines/q_parse.cpp
	../splines/splines.cpp
	../splines/util_str.cpp
	../system/rtcw_input_joystick.cpp
	../system/rtcw_input_keyboard.cpp
	../system/rtcw_input_mouse.cpp
	../system/sys_events.cpp
	../system/sys_glimp.cpp
	../system/sys_input.cpp
	../system/sys_net.cpp
	../system/sys_shared.cpp
	../win32/win_main.cpp
	../win32/win_syscon.cpp
	../zlib/contrib/minizip/ioapi.c
	../zlib/contrib/minizip/unzip.c
)

if (WIN32)
	list(
		APPEND
		sources
		../win32/rtcw_mp.ico
		../win32/rtcw_mp.rc
	)
endif ()

add_executable(
	rtcw_mp
	${headers}
	${sources}
)

target_include_directories(
	rtcw_mp
	PRIVATE ${SDL_INCLUDE_DIR}
	PRIVATE ${SDL_NET_INCLUDE_DIR}
	PRIVATE .
	PRIVATE ../botai
	PRIVATE ../cgame
	PRIVATE ../client
	PRIVATE ../game
	PRIVATE ../glm
	PRIVATE ../qcommon
	PRIVATE ../renderer
	PRIVATE ../system
	PRIVATE ../ui
	PRIVATE ../win32
	PRIVATE ../zlib/contrib/minizip
)

target_compile_definitions(
	rtcw_mp
	PRIVATE BOTLIB
	PRIVATE RTCW_32
	PRIVATE RTCW_MP
	PRIVATE MINIZ_NO_STDIO
	PRIVATE MINIZ_NO_TIME
)

if (MSVC)
	target_compile_definitions(
		rtcw_mp
		PRIVATE _CRT_SECURE_NO_WARNINGS
	)
endif ()

if (MINGW)
	target_link_libraries(
		rtcw_mp
		mingw32
	)
endif ()

target_link_libraries(
	rtcw_mp
	${SDLMAIN_LIBRARY}
	${SDL_LIBRARY}
	${SDL_NET_LIBRARY}
	OpenGL::GL
)

if (MINGW)
	target_link_libraries(
		rtcw_mp
		imm32
		iphlpapi
		version
		winmm
		ws2_32
	)
endif ()

set_target_properties(
	rtcw_mp
	PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)

set_target_properties(
	rtcw_mp
	PROPERTIES
	OUTPUT_NAME rtcw_mp_x86
)

if (WIN32)
	set_target_properties(
		rtcw_mp
		PROPERTIES
		WIN32_EXECUTABLE TRUE
	)

	if (MSVC)
		set_target_properties(
			rtcw_mp
			PROPERTIES
			LINK_FLAGS -STACK:8388608
		)
	endif ()
endif ()

install(
	TARGETS
	rtcw_mp
	RUNTIME DESTINATION mp
)
