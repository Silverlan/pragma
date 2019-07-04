include("../../CMakeShared.txt")

set(CMAKE_INSTALL_RPATH "$ORIGIN")

link_external_library(shared)
link_external_library(server)
add_external_library(sharedutils)
add_external_library(mathutil)
link_external_library(util_zip)
link_external_library(util_pragma_doc)
link_external_library(util_pad)
link_external_library(util_versioned_archive)
link_external_library(wms_shared)
link_external_library(bzip2)

add_external_library(luajit)
add_external_library(libzip)
if(WIN32)
	add_external_library(lua)
endif()

add_def(ENGINE_EXPORTS)
add_def(DLLENGINE_EX)

set(PRECOMPILED_HEADER "stdafx_engine")

def_project(engine lib)

# link_boost_library(engine DEPENDENCY_BOOST_DATE_TIME_LIBRARY date_time)
# link_boost_library(engine DEPENDENCY_BOOST_REGEX_LIBRARY regex)
# link_boost_library(engine DEPENDENCY_BOOST_THREAD_LIBRARY thread)
# link_boost_library(engine DEPENDENCY_BOOST_CHRONO_LIBRARY chrono)

if(UNIX)
	target_link_libraries(engine "pthread")
endif()
