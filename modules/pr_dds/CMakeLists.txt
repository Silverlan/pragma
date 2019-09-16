cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_dds)

project(${PROJ_NAME} CXX)

link_external_library(util_image)
link_external_library(nvidia_texture_tools)
link_external_library(lpng)
link_external_library(libzip)
link_external_library(zlib)
link_external_library(materialsystem)
link_external_library(cmaterialsystem)

link_external_library(client)
link_external_library(cengine)
link_external_library(prosper)
link_external_library(vulkan)
link_external_library(anvil)
link_external_library(glfw)
link_external_library(iglfw)
link_external_library(glslang)

add_include_dir(glm)
add_include_dir(gli)
add_include_dir(lpng_build)
add_include_dir(zlib_build)
add_include_dir(anvil_build)

add_external_library(luajit)
add_external_library(lua)
add_external_library(nvidia_texture_tools)
add_external_library(nvidia_texture_tools_image)
add_external_library(nvidia_texture_tools_core)
add_external_library(nvidia_texture_tools_math)
add_external_library(nvidia_texture_tools_bc7)
add_external_library(nvidia_texture_tools_bc6h)
add_external_library(nvidia_texture_tools_thread)
add_external_library(squish)

add_def(ANVIL_VULKAN_CPP)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
