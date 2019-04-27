include("../../CMakeShared.txt")

set(PRECOMPILED_HEADER "stdafx_weave")
def_project(pragma exe)

if(UNIX)
	target_link_libraries(pragma "dl")
	target_link_libraries(pragma "pthread")
	# target_compile_options(pragma PRIVATE -Wl,-rpath,$ORIGIN/lib)
	# SET_TARGET_PROPERTIES(pragma PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
else()
	set_target_properties(pragma PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")
endif()
