cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (rtcw_lib_miniz VERSION 1.0.0 LANGUAGES CXX)

message (STATUS "")
message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")

add_library (${PROJECT_NAME} STATIC "")

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)

target_include_directories (
	${PROJECT_NAME}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions (
	${PROJECT_NAME}
	PUBLIC
		MINIZ_NO_STDIO
		MINIZ_NO_TIME
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		miniz.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		rtcw_miniz.cpp
)
