cmake_minimum_required(VERSION 3.12)

project(pragma_engine)

add_subdirectory(engine)
add_subdirectory(cengine)
add_subdirectory(shared)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(pragma)
add_subdirectory(pragma_server)

set_target_properties(engine PROPERTIES FOLDER core)
set_target_properties(cengine PROPERTIES FOLDER core)
set_target_properties(shared PROPERTIES FOLDER core)
set_target_properties(server PROPERTIES FOLDER core)
set_target_properties(client PROPERTIES FOLDER core)
set_target_properties(pragma PROPERTIES FOLDER core)
set_target_properties(pragma_server PROPERTIES FOLDER core)

set(CMAKE_CXX_STANDARD 17)
