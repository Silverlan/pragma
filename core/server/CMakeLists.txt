include("../../CMakeShared.txt")

add_external_library(luajit)
add_external_library(lua)
link_external_library(shared)
link_external_library(engine)
link_external_library(wms_shared)
link_external_library(util_sound)
link_external_library(util_bsp)
link_external_library(bzip2)
link_external_library(recast)
link_external_library(detour)

add_include_dir(bzip2)

add_external_library(servermanager)
add_external_library(clientmanager)
add_external_library(libvorbisfile)

add_def(DLLSERVER_EX)
add_def(WMS_STATIC)
add_def(ENGINE_SERVER)

set(PRECOMPILED_HEADER "stdafx_server")

def_project(server lib)

link_boost_library(server DEPENDENCY_BOOST_DATE_TIME_LIBRARY boost_date_time-vc141-mt-x64-1_69.lib)
link_boost_library(server DEPENDENCY_BOOST_REGEX_LIBRARY boost_regex-vc141-mt-x64-1_69.lib)
link_boost_library(server DEPENDENCY_BOOST_THREAD_LIBRARY boost_thread-vc141-mt-x64-1_69.lib)
link_boost_library(server DEPENDENCY_BOOST_CHRONO_LIBRARY boost_chrono-vc141-mt-x64-1_69.lib)
