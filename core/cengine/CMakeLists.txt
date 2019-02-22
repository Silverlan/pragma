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

add_include_dir(server)

add_def(DLLCENGINE_EX)

set(PRECOMPILED_HEADER "stdafx_cengine")

def_project(cengine lib)
