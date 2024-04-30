cmake_minimum_required(VERSION 3.12)

set(INCLUDE_DIRS)
function(add_include_dir IDENTIFIER)
	set(PRETTYNAME ${IDENTIFIER})
	set(ARGV ${ARGN})
	list(LENGTH ARGV ARGC)
	if(${ARGC} GREATER 0)
		list(GET ARGV 0 PRETTYNAME)
	endif()
	string(TOUPPER ${IDENTIFIER} UIDENTIFIER)

	set(${DEFAULT_DIR} "")
	set(DEPENDENCY_${UIDENTIFIER}_INCLUDE ${DEFAULT_DIR} CACHE PATH "Path to ${PRETTYNAME} include directory.")
	set(INCLUDE_DIRS ${INCLUDE_DIRS} DEPENDENCY_${UIDENTIFIER}_INCLUDE PARENT_SCOPE)
endfunction(add_include_dir)

set(LIBRARIES)
function(add_external_library IDENTIFIER)
	set(PRETTYNAME ${IDENTIFIER})
	set(ARGV ${ARGN})
	list(LENGTH ARGV ARGC)
	if(${ARGC} GREATER 0)
		list(GET ARGV 0 PRETTYNAME)
	endif()
	string(TOUPPER ${IDENTIFIER} UIDENTIFIER)

	set(DEPENDENCY_${UIDENTIFIER}_LIBRARY "" CACHE FILEPATH "Path to ${PRETTYNAME} library.")
	set(LIBRARIES ${LIBRARIES} DEPENDENCY_${UIDENTIFIER}_LIBRARY PARENT_SCOPE)
endfunction(add_external_library)

function(link_external_library IDENTIFIER)
	set(PRETTYNAME ${IDENTIFIER})
	set(ARGV ${ARGN})
	list(LENGTH ARGV ARGC)
	if(${ARGC} GREATER 0)
		list(GET ARGV 0 PRETTYNAME)
	endif()
	string(TOUPPER ${IDENTIFIER} UIDENTIFIER)

	set(${DEFAULT_DIR} "")
	set(DEPENDENCY_${UIDENTIFIER}_INCLUDE ${DEFAULT_DIR} CACHE PATH "Path to ${PRETTYNAME} include directory.")
	set(INCLUDE_DIRS ${INCLUDE_DIRS} DEPENDENCY_${UIDENTIFIER}_INCLUDE PARENT_SCOPE)

	set(DEPENDENCY_${UIDENTIFIER}_LIBRARY "" CACHE FILEPATH "Path to ${PRETTYNAME} library.")
	set(LIBRARIES ${LIBRARIES} DEPENDENCY_${UIDENTIFIER}_LIBRARY PARENT_SCOPE)
endfunction(link_external_library)

set(DEFINITIONS)
function(add_def DEF)
	set(DEFINITIONS ${DEFINITIONS} ${DEF} PARENT_SCOPE)
endfunction(add_def)

##### CONFIGURATION #####

set(PROJ_NAME udm_convert)

project(${PROJ_NAME} CXX)

set(CMAKE_CXX_STANDARD 20)

set(DEPENDENCY_UTIL_UDM_INCLUDE "" CACHE PATH "Path to util_udm include directory.")
set(DEPENDENCY_SHAREDUTILS_LIBRARY "" CACHE FILEPATH "Path to sharedutils library.")
set(DEPENDENCY_MATHUTIL_LIBRARY "" CACHE FILEPATH "Path to mathutil library.")
set(DEPENDENCY_VFILESYSTEM_LIBRARY "" CACHE FILEPATH "Path to vfilesystem library.")
set(DEPENDENCY_UTIL_UDM_LIBRARY "" CACHE FILEPATH "Path to util_udm library.")
set(DEPENDENCY_LZ4_LIBRARY "" CACHE FILEPATH "Path to lz4 library.")
add_include_dir(sharedutils)
add_include_dir(mathutil)
add_include_dir(vfilesystem)
add_include_dir(util_udm)
add_include_dir(glm)

##### CONFIGURATION #####

foreach(def IN LISTS DEFINITIONS)
	add_definitions(-D${def})
endforeach(def)

function(def_vs_filters FILE_LIST)
	foreach(source IN LISTS FILE_LIST)
	    get_filename_component(source_path "${source}" PATH)
	    string(REPLACE "${CMAKE_CURRENT_LIST_DIR}" "" source_path_relative "${source_path}")
	    string(REPLACE "/" "\\" source_path_msvc "${source_path_relative}")
	    source_group("${source_path_msvc}" FILES "${source}")
	endforeach()
endfunction(def_vs_filters)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/*.hpp"
)
add_executable(${PROJ_NAME} ${SRC_FILES})
set_target_properties(${PROJ_NAME} PROPERTIES OUTPUT_NAME "udm_convert")

target_link_libraries(${PROJ_NAME} ${DEPENDENCY_SHAREDUTILS_LIBRARY})
target_link_libraries(${PROJ_NAME} ${DEPENDENCY_UTIL_UDM_LIBRARY})
target_link_libraries(${PROJ_NAME} ${DEPENDENCY_MATHUTIL_LIBRARY})
target_link_libraries(${PROJ_NAME} ${DEPENDENCY_VFILESYSTEM_LIBRARY})
target_link_libraries(${PROJ_NAME} ${DEPENDENCY_LZ4_LIBRARY})

if(WIN32)
	set_property(TARGET ${PROJ_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
	set_property(TARGET ${PROJ_NAME} PROPERTY VS_DEBUGGER_COMMAND "${CMAKE_INSTALL_PREFIX}/${PROJ_NAME}.exe")
	set_target_folder(${PROJ_NAME} core)

	target_compile_options(${PROJ_NAME} PRIVATE /wd4251)
	target_compile_options(${PROJ_NAME} PRIVATE /wd4996)
endif()
def_vs_filters("${SRC_FILES}")

foreach(LIB IN LISTS LIBRARIES)
	target_link_libraries(${PROJ_NAME} ${${LIB}})
endforeach(LIB)

target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

foreach(INCLUDE_PATH IN LISTS INCLUDE_DIRS)
	target_include_directories(${PROJ_NAME} PRIVATE ${${INCLUDE_PATH}})
endforeach(INCLUDE_PATH)

set(TARGET_PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${PROJ_NAME} PROPERTIES ${TARGET_PROPERTIES})

add_dependencies(${PROJ_NAME} sharedutils mathutil vfilesystem util_udm)
