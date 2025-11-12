include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME pragma_updater)
pr_add_executable(${PROJ_NAME} CONSOLE)

if(WIN32)
    set_target_properties(${PROJ_NAME} PROPERTIES OUTPUT_NAME "updater" SUFFIX ".exe")
else()
    set_target_properties(${PROJ_NAME} PROPERTIES OUTPUT_NAME "updater")
endif()

pr_add_dependency(${PROJ_NAME} util_string TARGET)

pr_init_module(${PROJ_NAME})

pr_finalize(${PROJ_NAME} FOLDER "tools")
