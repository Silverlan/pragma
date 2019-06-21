cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_steam_networking)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})

set(DEPENDENCY_VALVE_GAMENETWORKINGSOCKETS_INCLUDE "" CACHE PATH "Path to include directory.")
set(DEPENDENCY_VALVE_GAMENETWORKINGSOCKETS_LIBRARY "" CACHE PATH "Path to library directory.")

set(DEPENDENCY_OPENSSL_INCLUDE "" CACHE PATH "Path to include directory.")
set(DEPENDENCY_OPENSSL_LIBRARY "" CACHE PATH "Path to library directory.")

set(DEPENDENCY_PROTOBUF_INCLUDE "" CACHE PATH "Path to include directory.")
set(DEPENDENCY_PROTOBUF_LIBRARY "" CACHE PATH "Path to library directory.")
