cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_source)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

link_external_library(fbxsdk)
link_external_library(util_smdmodel)

add_include_dir(glm)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
