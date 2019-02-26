cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_dds)

project(${PROJ_NAME} CXX)

link_external_library(util_image)
link_external_library(nvidia_texture_tools)
link_external_library(lpng)
link_external_library(libzip)
link_external_library(zlib)

add_include_dir(glm)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
