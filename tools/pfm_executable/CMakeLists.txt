include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME pfm)
pr_add_executable(${PROJ_NAME} APP_ICON_WIN "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc" DEBUGGER_LAUNCH_ARGS "-console -luaext")

pr_add_include_dir(${PROJ_NAME} PRAGMA_EXECUTABLE)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

if(UNIX)
    target_link_libraries(${PROJ_NAME} PRIVATE "dl")
    target_link_libraries(${PROJ_NAME} PRIVATE "pthread")
else()
    set_target_properties(${PROJ_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
    set_target_properties(${PROJ_NAME} PROPERTIES VS_DEBUGGER_COMMAND "${CMAKE_INSTALL_PREFIX}/${PROJ_NAME}.exe")
endif()

pr_finalize(${PROJ_NAME} FOLDER core)
