cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_mdleditor)

project(${PROJ_NAME} CXX)

set(CMAKE_CXX_STANDARD 17)
set(ENABLE_STATIC_LIBRARY_FLAG 0)
set(PRECOMPILED_HEADER "stdafx_mdleditor")

link_external_library(bullet)
link_external_library(util_smdmodel)

add_external_library(util_archive)
add_external_library(niftools)
add_external_library(lua)
add_external_library(pr_mount_external)

add_def(SMDMDL_STATIC)

add_include_dir(glm)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
