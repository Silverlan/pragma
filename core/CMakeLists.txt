cmake_minimum_required(VERSION 3.12)

project(pragma_engine)

message("Processing core library 'shared'...")
add_subdirectory(shared)

message("Processing core library 'server'...")
add_subdirectory(server)

message("Processing core library 'client'...")
add_subdirectory(client)

message("Processing core library 'pragma'...")
add_subdirectory(pragma)

message("Processing core library 'pragma_server'...")
add_subdirectory(pragma_server)

message("Processing core library 'wms_shared'...")
add_subdirectory(wms_shared)

set_target_properties(shared PROPERTIES FOLDER core)
set_target_properties(server PROPERTIES FOLDER core)
set_target_properties(client PROPERTIES FOLDER core)
set_target_properties(pragma PROPERTIES FOLDER core)
set_target_properties(pragma_server PROPERTIES FOLDER core)
set_target_properties(wms_shared PROPERTIES FOLDER core)
if(WIN32)
    set_target_properties(pragma_console PROPERTIES FOLDER core)
endif()

set(CMAKE_CXX_STANDARD 20)
