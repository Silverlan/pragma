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
link_external_library(client)
link_external_library(cengine)
link_external_library(vulkan)
link_external_library(glslang)
add_external_library(luajit)
if(WIN32)
	add_external_library(lua)
endif()

add_include_dir(glm)
add_include_dir(anvil_build)

add_def(ANVIL_VULKAN_CPP)
add_def(USE_VULKAN)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
