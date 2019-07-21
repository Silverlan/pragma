cmake_minimum_required(VERSION 3.12)

include("../../CMakeShared.txt")

set(PROJ_NAME pr_steam_networking_server)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

add_include_dir(networkmanager)
link_external_library(valve_gamenetworkingsockets)
link_external_library(openssl)
link_external_library(protobuf)
link_external_library(server)

set(CONFIG_ENABLE_STEAM_NETWORKING_STEAM_SERVER_SUPPORT ON CACHE BOOL "Enable steam server support for pr_steam_networking module?")
if(CONFIG_ENABLE_STEAM_NETWORKING_STEAM_SERVER_SUPPORT MATCHES ON)
	link_external_library(steamworks)
	add_def(ENABLE_STEAM_SERVER_SUPPORT)
endif()

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})

target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../shared/include)
