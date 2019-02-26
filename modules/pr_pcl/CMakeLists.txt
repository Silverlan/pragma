cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_pcl)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

link_external_library(pcl)

add_include_dir(eigen)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
