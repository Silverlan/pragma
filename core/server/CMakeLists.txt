include("../../CMakeShared.txt")

link_external_library(shared)
link_external_library(engine)
link_external_library(wms_shared)
link_external_library(util_sound)
link_external_library(util_bsp)
link_external_library(shaderinfo)

add_include_dir(bzip2)

add_def(DLLSERVER_EX)

set(PRECOMPILED_HEADER "stdafx_server")

def_project(server lib)
