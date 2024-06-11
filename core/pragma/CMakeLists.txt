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

if(WIN32)
	def_project(pragma_console exe ${APP_ICON_RESOURCE_WINDOWS})

	set_target_properties(pragma_console PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")

	# This is required so that the executable can be launched from within a Windows terminal
	set_target_properties(pragma_console PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")

	set_target_properties(pragma_console PROPERTIES OUTPUT_NAME "pragma" SUFFIX ".com")
	target_sources(pragma_console PRIVATE ${APP_ICON_RESOURCE_WINDOWS})
endif()
