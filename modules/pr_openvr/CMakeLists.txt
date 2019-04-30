cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_openvr)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

link_external_library(iclient)
link_external_library(openvr)
link_external_library(glfw)
link_external_library(iglfw)
link_external_library(prosper)
link_external_library(anvil)

add_include_dir(glm)
add_include_dir(anvil_build)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
