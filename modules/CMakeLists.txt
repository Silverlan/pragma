cmake_minimum_required(VERSION 3.12)

project(modules)

function(register_module MODULE_NAME)
	message("Processing module '${MODULE_NAME}'...")
	add_subdirectory(${MODULE_NAME})
endfunction(register_module)

file(GLOB MODULE_LIST
    "${CMAKE_CURRENT_LIST_DIR}/*"
)
foreach(MODULE_PATH ${MODULE_LIST})
	if(IS_DIRECTORY ${MODULE_PATH})
		get_filename_component(MODULE_NAME ${MODULE_PATH} NAME)

		set_property(GLOBAL PROPERTY PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER 0)

		register_module(${MODULE_NAME})

		get_property(PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER GLOBAL PROPERTY PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER)

		if(${PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER} EQUAL 0)
			if(NOT "${MODULE_NAME}" STREQUAL "interfaces")
				set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules)
			endif()
		endif()
	endif()
endforeach()

set(CMAKE_CXX_STANDARD 20)
