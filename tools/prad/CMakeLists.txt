include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME prad)
pr_add_executable(${PROJ_NAME} CONSOLE)

pr_add_dependency(${PROJ_NAME} util_udm TARGET PUBLIC)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

pr_finalize(${PROJ_NAME} FOLDER "tools")
