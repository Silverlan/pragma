include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME pragma_server)
pr_add_executable(${PROJ_NAME} CONSOLE APP_ICON_WIN "${CMAKE_CURRENT_SOURCE_DIR}/../pragma/appicon.rc" DEBUGGER_LAUNCH_ARGS "-console -luaext")

pr_add_include_dir(${PROJ_NAME} PRAGMA_EXECUTABLE)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

if(UNIX)
    target_link_libraries(${PROJ_NAME} PRIVATE "dl")
    target_link_libraries(${PROJ_NAME} PRIVATE "pthread")
endif()

pr_finalize(${PROJ_NAME})
