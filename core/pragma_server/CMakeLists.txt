include("../../CMakeShared.txt")

# set(PRECOMPILED_HEADER "stdafx_pragma_server")
def_project(pragma_server exe)

if(UNIX)
	target_link_libraries(pragma_server "dl")
	target_link_libraries(pragma_server "pthread")
endif()
