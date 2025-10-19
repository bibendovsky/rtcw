cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_curl VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_curl] Version: ${PROJECT_VERSION}")
add_library(rtcw_curl STATIC "")
add_library(rtcw::curl ALIAS rtcw_curl)
rtcw_configure_static_linking(rtcw_curl)
target_include_directories(rtcw_curl PUBLIC include)
target_compile_definitions(rtcw_curl PUBLIC CURL_STATICLIB)

target_sources(rtcw_curl PRIVATE
	include/rtcw_curl_loader.h
	#
	include/curl/curl.h
	include/curl/curlver.h
	include/curl/easy.h
	include/curl/header.h
	include/curl/mprintf.h
	include/curl/multi.h
	include/curl/options.h
	include/curl/system.h
	include/curl/typecheck-gcc.h
	include/curl/urlapi.h
	include/curl/websockets.h
	#
	src/rtcw_curl_loader.cpp
)
