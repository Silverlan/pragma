include("../../CMakeShared.txt")

# set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# set(PRECOMPILED_HEADER "stdafx_pragma_server")
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/../pragma/appicon.rc")
def_project(pragma_server exe ${APP_ICON_RESOURCE_WINDOWS})

if(UNIX)
	target_link_libraries(pragma_server "dl")
	target_link_libraries(pragma_server "pthread")
else()
	set_target_properties(pragma PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")
endif()
