cmake_minimum_required(VERSION 4.2.0)

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

set_target_properties(shared PROPERTIES FOLDER core/engine)
set_target_properties(server PROPERTIES FOLDER core/engine)
set_target_properties(client PROPERTIES FOLDER core/engine)
set_target_properties(pragma_executable PROPERTIES FOLDER core)
set_target_properties(pragma_server PROPERTIES FOLDER core)
set_target_properties(wms_shared PROPERTIES FOLDER external_libs)
if(WIN32)
    set_target_properties(pragma_console PROPERTIES FOLDER core)
endif()
