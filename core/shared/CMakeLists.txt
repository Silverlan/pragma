include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME shared)
pr_add_library(${PROJ_NAME} SHARED)

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

# We need these for the convar headers
pr_add_dependency(${PROJ_NAME} server HEADER_ONLY)
pr_add_dependency(${PROJ_NAME} client HEADER_ONLY)

pr_add_external_dependency(${PROJ_NAME} recast LIBRARY)
pr_add_external_dependency(${PROJ_NAME} detour LIBRARY)
pr_add_external_dependency(${PROJ_NAME} spdlog LIBRARY PUBLIC)
pr_add_external_dependency(${PROJ_NAME} fmt LIBRARY)
pr_add_external_dependency(${PROJ_NAME} clip2tri LIBRARY)
pr_add_external_dependency(${PROJ_NAME} poly2tri LIBRARY LINK_ONLY)
pr_add_external_dependency(${PROJ_NAME} clipper LIBRARY LINK_ONLY)
pr_add_external_dependency(${PROJ_NAME} muparserx LIBRARY)
pr_add_external_dependency(${PROJ_NAME} bvh HEADER_ONLY PUBLIC)
pr_add_external_dependency(${PROJ_NAME} eigen HEADER_ONLY)
pr_add_external_dependency(${PROJ_NAME} miniball HEADER_ONLY)

# Required by lnoise.cpp
pr_add_external_dependency(${PROJ_NAME} libnoise LIBRARY)
pr_add_external_dependency(${PROJ_NAME} libnoiseutils LIBRARY)
pr_add_compile_definitions(${PROJ_NAME} -DNOISE_STATIC)

# Required by sound_ogg.cpp
pr_add_external_dependency(${PROJ_NAME} libvorbis LIBRARY LINK_ONLY)
pr_add_external_dependency(${PROJ_NAME} libvorbisfile LIBRARY LINK_ONLY)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

pr_add_compile_definitions(${PROJ_NAME} -DDLLNETWORK_EX -DENGINE_NETWORK)

if(${CONFIG_BUILD_WITH_VTUNE_SUPPORT})
	pr_add_external_dependency(${PROJ_NAME} vtune_profiler LIBRARY)
endif()

pr_finalize(${PROJ_NAME})
