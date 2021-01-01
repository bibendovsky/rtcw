cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (rtcw_lib_dear_imgui VERSION 1.0.0 LANGUAGES CXX)

message (STATUS "")
message (STATUS "[${PROJECT_NAME}] Version: ${PROJECT_VERSION}")

add_library (${PROJECT_NAME} STATIC "")

target_include_directories (
	${PROJECT_NAME}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions (
	${PROJECT_NAME}
	PUBLIC
		IMGUI_DISABLE_OBSOLETE_FUNCTIONS
)

rtcw_configure_3rd_party_target (${PROJECT_NAME})

target_sources (
	${PROJECT_NAME}
	PRIVATE
		imconfig.h
		imgui.h
		imgui_internal.h
		imstb_rectpack.h
		imstb_textedit.h
		imstb_truetype.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		imgui.cpp
		imgui_draw.cpp
		imgui_widgets.cpp
)
