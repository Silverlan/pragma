include("../../CMakeShared.txt")

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
add_include_dir(pragma_executable)
def_project(pragma exe ${APP_ICON_RESOURCE_WINDOWS})

if(UNIX)
	target_link_libraries(pragma "dl")
	target_link_libraries(pragma "pthread")
else()
	set_target_properties(pragma PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")
endif()


