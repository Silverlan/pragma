cmake_minimum_required(VERSION 4.2.0)

project(tools)

function(register_tool TOOL_NAME)
	message("Processing tool '${TOOL_NAME}'...")
	add_subdirectory(${TOOL_NAME})
endfunction(register_tool)

file(GLOB TOOL_LIST
    "${CMAKE_CURRENT_LIST_DIR}/*"
)
foreach(TOOL_PATH ${TOOL_LIST})
	if(IS_DIRECTORY ${TOOL_PATH})
		get_filename_component(TOOL_NAME ${TOOL_PATH} NAME)
		register_tool(${TOOL_NAME})
		set_target_properties(${PROJ_NAME} PROPERTIES FOLDER tools)
	endif()
endforeach()

set(CMAKE_CXX_STANDARD 20)
