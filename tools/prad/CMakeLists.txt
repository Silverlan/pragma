include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME prad)
pr_add_executable(${PROJ_NAME} CONSOLE)

pr_add_dependency(${PROJ_NAME} vfilesystem TARGET PUBLIC)

pr_init_module(${PROJ_NAME})

pr_finalize(${PROJ_NAME} FOLDER "tools")
