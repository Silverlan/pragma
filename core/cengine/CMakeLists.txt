include("../../CMakeShared.txt")

link_external_library(engine)
link_external_library(shared)
link_external_library(client)
link_external_library(wgui)
link_external_library(prosper)
link_external_library(vulkan)
link_external_library(anvil)
link_external_library(glfw)
link_external_library(iglfw)
link_external_library(util_zip)
link_external_library(alsoundsystem)
link_external_library(cmaterialsystem)
link_external_library(wms_shared)

add_external_library(luajit)
add_external_library(lua)
add_external_library(libzip)
add_external_library(util_timeline_scene)

add_include_dir(server)

add_def(CENGINE_EXPORTS)
add_def(DLLCENGINE_EX)
add_def(ALSYS_LIBRARY_TYPE)

set(PRECOMPILED_HEADER "stdafx_cengine")

def_project(cengine lib)

link_boost_library(cengine DEPENDENCY_BOOST_DATE_TIME_LIBRARY boost_date_time-vc141-mt-x64-1_69.lib)
link_boost_library(cengine DEPENDENCY_BOOST_REGEX_LIBRARY boost_regex-vc141-mt-x64-1_69.lib)
link_boost_library(cengine DEPENDENCY_BOOST_THREAD_LIBRARY boost_thread-vc141-mt-x64-1_69.lib)
link_boost_library(cengine DEPENDENCY_BOOST_CHRONO_LIBRARY boost_chrono-vc141-mt-x64-1_69.lib)
