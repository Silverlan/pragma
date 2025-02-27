include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

option(CONFIG_BUILD_WITH_VTUNE_SUPPORT "Build with VTune profiler support?" OFF)

set(PROJ_NAME shared)
pr_add_library(${PROJ_NAME} SHARED)
pr_precompile_headers(${PROJ_NAME} "stdafx_shared.h")

pr_add_dependency(${PROJ_NAME} panima TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} materialsystem TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} luasystem TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} util_pad TARGET)
pr_add_dependency(${PROJ_NAME} util_sound TARGET)
pr_add_dependency(${PROJ_NAME} wms_shared TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} util_zip TARGET)
pr_add_dependency(${PROJ_NAME} networkmanager TARGET)
pr_add_dependency(${PROJ_NAME} util_source_script TARGET)
pr_add_dependency(${PROJ_NAME} util_smdmodel TARGET)
pr_add_dependency(${PROJ_NAME} util_pragma_doc TARGET)
pr_add_dependency(${PROJ_NAME} util_unicode TARGET PUBLIC)

pr_add_dependency(${PROJ_NAME} Recast TARGET)
pr_add_dependency(${PROJ_NAME} Detour TARGET)
pr_add_dependency(${PROJ_NAME} spdlog TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} fmt TARGET)
pr_add_dependency(${PROJ_NAME} clip2tri-static TARGET)
pr_add_dependency(${PROJ_NAME} poly2tri-static TARGET)
pr_add_dependency(${PROJ_NAME} clipper-static TARGET)
pr_add_dependency(${PROJ_NAME} muparserx TARGET)

# We need these for the convar headers
target_include_directories(${PROJ_NAME} PRIVATE "../server/include/")
target_include_directories(${PROJ_NAME} PRIVATE "../client/include/")

pr_add_external_dependency(${PROJ_NAME} bvh HEADER_ONLY PUBLIC)
pr_add_external_dependency(${PROJ_NAME} eigen HEADER_ONLY)
pr_add_external_dependency(${PROJ_NAME} miniball HEADER_ONLY)

# Required by lnoise.cpp
pr_add_dependency(${PROJ_NAME} noise-static TARGET)
pr_add_dependency(${PROJ_NAME} noiseutils-static TARGET)
pr_add_compile_definitions(${PROJ_NAME} -DNOISE_STATIC)

# Required by sound_ogg.cpp
pr_add_dependency(${PROJ_NAME} vorbis TARGET)
pr_add_dependency(${PROJ_NAME} vorbisfile TARGET)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")
pr_init_module(${PROJ_NAME})

pr_add_compile_definitions(${PROJ_NAME} -DDLLNETWORK_EX -DENGINE_NETWORK)

if(${CONFIG_BUILD_WITH_VTUNE_SUPPORT})
	pr_add_external_dependency(${PROJ_NAME} vtune_profiler LIBRARY)
	pr_add_compile_definitions(${PROJ_NAME} -DPRAGMA_ENABLE_VTUNE_PROFILING PUBLIC)
endif()

pr_finalize(${PROJ_NAME})
