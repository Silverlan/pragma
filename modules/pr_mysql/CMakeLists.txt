cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_mysql)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

add_include_dir(sqlite)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
