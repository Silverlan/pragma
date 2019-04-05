include("../../CMakeShared.txt")

set(PRECOMPILED_HEADER "stdafx_weave")
def_project(pragma exe)

if(WIN32)
	set_target_properties(pragma PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")
endif()
