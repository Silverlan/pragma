include("../../CMakeShared.txt")

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/../pragma/appicon.rc")
add_include_dir(pragma_executable)
def_project(pragma_server exe ${APP_ICON_RESOURCE_WINDOWS})

if(UNIX)
	target_link_libraries(pragma_server "dl")
	target_link_libraries(pragma_server "pthread")
else()
	set_target_properties(pragma_server PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")

	# This is required so that the executable can be launched from within a Windows terminal
	set_target_properties(pragma_server PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()
