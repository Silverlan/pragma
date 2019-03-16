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

link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_THREAD_LIBRARY boost_thread-vc141-mt-x64-1_69.lib)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_SYSTEM_LIBRARY boost_system-vc141-mt-x64-1_69.lib)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_DATE_TIME_LIBRARY boost_date_time-vc141-mt-x64-1_69.lib)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_CHRONO_LIBRARY boost_chrono-vc141-mt-x64-1_69.lib)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_FILESYSTEM_LIBRARY boost_filesystem-vc141-mt-x64-1_69.lib)
link_boost_library(${PROJ_NAME} DEPENDENCY_BOOST_IOSTREAMS_LIBRARY boost_iostreams-vc141-mt-x64-1_69.lib)
