set(
    RTCW_TARGET
    rtcw_mp
)

add_definitions(-DBOTLIB)
add_definitions(-DRTCW_32)
add_definitions(-DRTCW_MP)

if (RTCW_STATIC_GLEW)
    add_definitions(-DGLEW_STATIC)
endif ()

include_directories(
    .
    ../botai
    ../cgame
    ../client
    ../game
    ../glm
    ../qcommon
    ../renderer
    ../system
    ../ui
    ../win32
    ${GLEW_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}/contrib/minizip
)

set(
    RTCW_HEADERS
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
    ../game/be_aas.h
    ../game/be_ai_char.h
    ../game/be_ai_chat.h
    ../game/be_ai_gen.h
    ../game/be_ai_goal.h
    ../game/be_ai_move.h
    ../game/be_ai_weap.h
    ../game/be_ea.h
    ../game/bg_public.h
    ../botai/botai.h
    ../game/botlib.h
    ../cgame/cg_public.h
    ../client/client.h
    ../qcommon/cm_local.h
    ../qcommon/cm_patch.h
    ../qcommon/cm_polylib.h
    ../qcommon/cm_public.h
    ../game/g_public.h
    ../renderer/anorms256.h
    ../renderer/rtcw_ogl_matrix_stack.h
    ../renderer/rtcw_ogl_program.h
    ../renderer/rtcw_ogl_tess_program.h
    ../renderer/rtcw_ogl_tess_state.h
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
    ../client/keys.h
    ../game/q_shared.h
    ../qcommon/qcommon.h
    ../qcommon/qfiles.h
    ../renderer/qgl.h
    ../server/server.h
    ../client/snd_local.h
    ../client/snd_public.h
    ../game/surfaceflags.h
    ../renderer/tr_local.h
    ../renderer/tr_public.h
    ../cgame/tr_types.h
    ../ui/ui_public.h
    ../qcommon/vm_local.h
    ../win32/glw_win.h
    ../win32/rtcw_mp_resource.h
    ../win32/win_local.h
)

if (WIN32)
set(
    RTCW_EXTRA_HEADERS
    ../win32/rtcw_mp_resource.h
)
endif ()

set(
    RTCW_SOURCES
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
    ../client/snd_dma_sdl.cpp
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
    ../game/q_math.cpp
    ../game/q_shared.cpp
    ../client/snd_adpcm.cpp
    ../client/snd_dma.cpp
    ../client/snd_mem.cpp
    ../client/snd_mix.cpp
    ../client/snd_wavelet.cpp
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
    ../renderer/tr_shader.cpp
    ../renderer/tr_shade_calc.cpp
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
    ../qcommon/vm.cpp
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
    ../system/sys_shared.cpp
    ../win32/win_main.cpp
    ../win32/win_net.cpp
    ../win32/win_syscon.cpp
    ../zlib/contrib/minizip/ioapi.c
    ../zlib/contrib/minizip/unzip.c
)

if (WIN32)
set(
    RTCW_EXTRA_SOURCES
    ../win32/rtcw_mp.ico
    ../win32/rtcw_mp.rc
)
endif ()

add_executable(
    ${RTCW_TARGET}
    ${RTCW_HEADERS}
    ${RTCW_EXTRA_HEADERS}
    ${RTCW_SOURCES}
    ${RTCW_EXTRA_SOURCES}
)

if (MINGW)
target_link_libraries(
    ${RTCW_TARGET}
    mingw32
)
endif ()

target_link_libraries(
    ${RTCW_TARGET}
    ${GLEW_LIBRARY}
    ${JPEG_LIBRARY}
    ${SDLMAIN_LIBRARY}
    ${SDL_LIBRARY}
    ${ZLIB_LIBRARY}
)

if (WIN32)
target_link_libraries(
    ${RTCW_TARGET}
    opengl32
    winmm
    ws2_32
)
endif ()

set_target_properties(
    ${RTCW_TARGET}
    PROPERTIES
    OUTPUT_NAME "rtcw_mp_x86"
    RUNTIME_OUTPUT_DIRECTORY "${RTCW_MP_BIN_DIR}"
)

if (MINGW)
set_target_properties(
    ${RTCW_TARGET}
    PROPERTIES
    LINK_FLAGS "-static -static-libgcc -static-libstdc++"
)
endif ()

if (MSVC)
set_target_properties(
    ${RTCW_TARGET}
    PROPERTIES
    LINK_FLAGS "/STACK:8388608"
)
endif ()

if (WIN32)
set_target_properties(
    ${RTCW_TARGET}
    PROPERTIES
    WIN32_EXECUTABLE TRUE
)
endif ()
