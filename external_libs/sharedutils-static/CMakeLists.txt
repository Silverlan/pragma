cmake_minimum_required(VERSION 3.12)

set(PROJ_NAME sharedutils-static)
project(${PROJ_NAME} CXX)

set(CMAKE_CXX_STANDARD 20)

set(DEPENDENCY_GLM_INCLUDE "" CACHE PATH "Path to GLM include directory.")
set(DEPENDENCY_MATHUTIL_INCLUDE "" CACHE PATH "Path to sharedutils include directory.")
set(DEPENDENCY_MATHUTIL_LIBRARY "" CACHE FILEPATH "Path to mathutil library. Required if this is compiled as a shared library.")

set(LIB_TYPE STATIC)

add_definitions(-DSHUTIL_DLL)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-URI_STATIC_BUILD)

function(def_vs_filters FILE_LIST)
	foreach(source IN LISTS FILE_LIST)
	    get_filename_component(source_path_real "${source}" REALPATH)
	    get_filename_component(source_path "${source_path_real}" PATH)
	    get_filename_component(current_list_path "${CMAKE_CURRENT_LIST_DIR}/../sharedutils" REALPATH)

	    string(REPLACE "${current_list_path}/" "" source_path_relative "${source_path}")
	    string(REPLACE "/" "\\" source_path_msvc "${source_path_relative}")
	    source_group("${source_path_msvc}" FILES "${source}")
	endforeach()
endfunction(def_vs_filters)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_LIST_DIR}/../sharedutils/src/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/../sharedutils/src/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../sharedutils/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../sharedutils/src/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/../sharedutils/include/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/../sharedutils/include/*.hpp"
)
def_vs_filters("${SRC_FILES}")
add_library(${PROJ_NAME} ${LIB_TYPE} ${SRC_FILES})
if(WIN32)
	target_compile_options(${PROJ_NAME} PRIVATE /wd4251)
	target_compile_options(${PROJ_NAME} PRIVATE /wd4996)
endif()

target_link_libraries(${PROJ_NAME} ${DEPENDENCY_MATHUTIL_LIBRARY})

set_target_properties(${PROJ_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../sharedutils/include)
target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../sharedutils/src)
target_include_directories(${PROJ_NAME} PRIVATE ${DEPENDENCY_GLM_INCLUDE})
target_include_directories(${PROJ_NAME} PRIVATE ${DEPENDENCY_MATHUTIL_INCLUDE})
