cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_pcl)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

link_external_library(pcl)
link_external_library(flann)
link_external_library(util_pcl)
link_external_library(ishared)

add_external_library(lua)

add_def(BOOST_ALL_NO_LIB)

add_include_dir(glm)
add_include_dir(eigen)
add_include_dir(pcl_io)
add_include_dir(pcl_kdtree)
add_include_dir(pcl_features)
add_include_dir(pcl_search)
add_include_dir(pcl_octree)
add_include_dir(pcl_surface)
add_include_dir(pcl_filters)
add_include_dir(pcl_sample_consensus)
add_include_dir(pcl_segmentation)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})

link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_THREAD_LIBRARY thread)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_SYSTEM_LIBRARY system)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_DATE_TIME_LIBRARY date_time)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_CHRONO_LIBRARY chrono)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_FILESYSTEM_LIBRARY filesystem)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_IOSTREAMS_LIBRARY iostreams)
