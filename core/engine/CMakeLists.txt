include("../../CMakeShared.txt")

link_external_library(shared)
link_external_library(server)
link_external_library(util_zip)
link_external_library(util_pragma_doc)
link_external_library(util_pad)
link_external_library(wms_shared)
link_external_library(bzip2)

add_external_library(luajit)
add_external_library(lua)
add_external_library(util_versioned_archive)
add_external_library(libzip)

add_def(ENGINE_EXPORTS)
add_def(DLLENGINE_EX)

set(PRECOMPILED_HEADER "stdafx_engine")

def_project(engine lib)

link_boost_library(engine DEPENDENCY_BOOST_DATE_TIME_LIBRARY boost_date_time-vc141-mt-x64-1_69.lib)
link_boost_library(engine DEPENDENCY_BOOST_REGEX_LIBRARY boost_regex-vc141-mt-x64-1_69.lib)
link_boost_library(engine DEPENDENCY_BOOST_THREAD_LIBRARY boost_thread-vc141-mt-x64-1_69.lib)
link_boost_library(engine DEPENDENCY_BOOST_CHRONO_LIBRARY boost_chrono-vc141-mt-x64-1_69.lib)
