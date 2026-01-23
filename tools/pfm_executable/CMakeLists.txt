include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME pfm_executable)
pr_add_executable(${PROJ_NAME} APP_ICON_WIN "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc" DEBUGGER_LAUNCH_ARGS "-console -luaext")
set_target_properties(${PROJ_NAME} PROPERTIES RUNTIME_OUTPUT_NAME "pfm")
pr_set_output_directory_to_install(${PROJ_NAME})

pr_add_third_party_dependency(${PROJ_NAME} pragma_executable PUBLIC)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

set_target_properties(${PROJ_NAME} PROPERTIES DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
if(UNIX)
    target_link_libraries(${PROJ_NAME} PRIVATE "dl")
    target_link_libraries(${PROJ_NAME} PRIVATE "pthread")
else()
    set_target_properties(${PROJ_NAME} PROPERTIES VS_DEBUGGER_COMMAND "${CMAKE_INSTALL_PREFIX}/${PROJ_NAME}.exe")
endif()

pr_finalize(${PROJ_NAME} FOLDER core)
pr_add_executable_install_target(pfm ${PROJ_NAME})
