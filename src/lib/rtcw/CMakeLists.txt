cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(rtcw_rtcw VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[rtcw_rtcw] Version: ${PROJECT_VERSION}")

add_library(rtcw_rtcw STATIC "")
add_library(rtcw::rtcw ALIAS rtcw_rtcw)

target_include_directories(rtcw_rtcw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

rtcw_configure_3rd_party_target(rtcw_rtcw)

target_sources(rtcw_rtcw
	PRIVATE
		include/rtcw_array_trivial.h
		include/rtcw_bit_array_single_unit.h
		include/rtcw_c.h
		include/rtcw_cgm_clip_space.h
		include/rtcw_cgm_mat.h
		include/rtcw_cgm_transform.h
		include/rtcw_cgm_vec.h
		include/rtcw_endian.h
		include/rtcw_memory.h
		include/rtcw_mod_value.h
		include/rtcw_string.h
		include/rtcw_type_utility.h
		include/rtcw_unique_ptr.h
		include/rtcw_vector_trivial.h
	PRIVATE
		src/rtcw_array_trivial.cpp
		src/rtcw_bit_array_single_unit.cpp
		src/rtcw_cgm_clip_space.cpp
		src/rtcw_cgm_mat.cpp
		src/rtcw_cgm_transform.cpp
		src/rtcw_cgm_vec.cpp
		src/rtcw_memory.cpp
		src/rtcw_string.cpp
		src/rtcw_type_utility.cpp
		src/rtcw_vector_trivial.cpp
)
