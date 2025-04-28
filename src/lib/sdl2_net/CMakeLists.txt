cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_sdl2_net VERSION 1.0.0 LANGUAGES C)

message(STATUS "[rtcw_sdl2_net] v${PROJECT_VERSION}")

add_library(rtcw_sdl2_net STATIC "")
add_library(rtcw::sdl2_net ALIAS rtcw_sdl2_net)

rtcw_configure_stack(rtcw_sdl2_net)
rtcw_configure_static_linking(rtcw_sdl2_net)

target_link_libraries(rtcw_sdl2_net
	PRIVATE
		SDL2::SDL2-static
		$<$<BOOL:${WIN32}>:iphlpapi>
		$<$<BOOL:${WIN32}>:ws2_32>
)

target_include_directories(rtcw_sdl2_net PUBLIC include)

target_sources(rtcw_sdl2_net
	PRIVATE
		include/SDL_net.h
	PRIVATE
		src/SDLnet.c
		src/SDLnetselect.c
		src/SDLnetsys.h
		src/SDLnetTCP.c
		src/SDLnetUDP.c
)
