cmake_minimum_required(VERSION 3.12)

project(external_libs)

function(register_external_lib SUB_PROJ_NAME)
	message("Processing external library '${SUB_PROJ_NAME}'...")
	add_subdirectory(${SUB_PROJ_NAME})

	if(SUB_PROJ_NAME MATCHES "util_*")
		set_target_properties(${SUB_PROJ_NAME} PROPERTIES FOLDER external_libs/util)
	else()
		set_target_properties(${SUB_PROJ_NAME} PROPERTIES FOLDER external_libs)
	endif()
endfunction(register_external_lib)

file(GLOB LIB_LIST
    "${CMAKE_CURRENT_LIST_DIR}/*"
)
foreach(LIB_PATH ${LIB_LIST})
	if(IS_DIRECTORY ${LIB_PATH})
		get_filename_component(LIB_NAME ${LIB_PATH} NAME)
		register_external_lib(${LIB_NAME})
	endif()
endforeach()

set(CMAKE_CXX_STANDARD 17)

set_target_properties(networkmanager PROPERTIES FOLDER external_libs/networkmanager)
set_target_properties(clientmanager PROPERTIES FOLDER external_libs/networkmanager)
set_target_properties(servermanager PROPERTIES FOLDER external_libs/networkmanager)

set_target_properties(materialsystem PROPERTIES FOLDER external_libs/materialsystem)
set_target_properties(cmaterialsystem PROPERTIES FOLDER external_libs/materialsystem)

set(CMAKE_CXX_STANDARD 17)
