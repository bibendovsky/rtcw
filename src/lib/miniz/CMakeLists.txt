cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_lib_miniz VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_lib_miniz] Version: ${PROJECT_VERSION}")

add_library(rtcw_lib_miniz STATIC "")

target_include_directories(rtcw_lib_miniz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(rtcw_lib_miniz
	PUBLIC
		MINIZ_NO_STDIO
		MINIZ_NO_TIME
)

rtcw_configure_3rd_party_target(rtcw_lib_miniz)

target_sources(rtcw_lib_miniz
	PRIVATE
		miniz.h
)

target_sources(rtcw_lib_miniz
	PRIVATE
		rtcw_miniz.cpp
)
