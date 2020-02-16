cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_source)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

link_external_library(util_archive)
link_external_library(fbxsdk)
link_external_library(util_smdmodel)
link_external_library(niftools)
link_external_library(util_dmx)
link_external_library(util_source2)

add_external_library(fbxsdk)
add_external_library(lua)
add_external_library(lz4) # Required for source2 library

add_include_dir(niftools)
add_include_dir(glm)
add_include_dir(bullet)

add_def(SMDMDL_STATIC)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})

set_target_properties(pr_source PROPERTIES OUTPUT_NAME "pr_mount_external")
