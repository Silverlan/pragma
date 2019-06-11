cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_video_recorder)

project(${PROJ_NAME} CXX)

link_external_library(util_video_recorder)
link_external_library(vulkan)
link_external_library(anvil)
link_external_library(prosper)

add_external_library(lua)
add_external_library(ffmpeg_avcodec)
add_external_library(ffmpeg_avutil)
add_external_library(ffmpeg_avformat)
add_external_library(ffmpeg_avdevice)
add_external_library(ffmpeg_avfilter)
add_external_library(ffmpeg_swscale)
add_external_library(avcpp)

add_include_dir(glm)
add_include_dir(anvil_build)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})
