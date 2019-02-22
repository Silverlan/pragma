cmake_minimum_required(VERSION 3.12)

project(modules)

add_subdirectory(interfaces)
add_subdirectory(pr_steamworks)

set_target_properties(pr_steamworks PROPERTIES FOLDER modules)

set(CMAKE_CXX_STANDARD 17)
