include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME client)
pr_add_library(${PROJ_NAME} SHARED)

pr_add_dependency(${PROJ_NAME} shared TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} cmaterialsystem TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} clientmanager TARGET)
pr_add_dependency(${PROJ_NAME} alsoundsystem TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} util_pad TARGET)
pr_add_dependency(${PROJ_NAME} wgui TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} util_sound TARGET)
pr_add_dependency(${PROJ_NAME} util_source_script TARGET)
pr_add_dependency(${PROJ_NAME} util_zip TARGET)
pr_add_dependency(${PROJ_NAME} util_timeline_scene TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} util_bsp TARGET)

pr_add_external_dependency(${PROJ_NAME} tinygltf HEADER_ONLY)
pr_add_external_dependency(${PROJ_NAME} recast LIBRARY)
pr_add_external_dependency(${PROJ_NAME} detour LIBRARY)
pr_add_external_dependency(${PROJ_NAME} RectangleBinPack LIBRARY)
pr_add_external_dependency(${PROJ_NAME} rapidxml LIBRARY)

pr_add_external_dependency(${PROJ_NAME} libvorbis LIBRARY LINK_ONLY)
pr_add_external_dependency(${PROJ_NAME} libvorbisfile LIBRARY LINK_ONLY)

pr_add_external_dependency(${PROJ_NAME} libnoise LIBRARY)
pr_add_external_dependency(${PROJ_NAME} libnoiseutils LIBRARY)
pr_add_compile_definitions(${PROJ_NAME} -DNOISE_STATIC)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

pr_add_compile_definitions(${PROJ_NAME} -DWMS_STATIC PUBLIC)
pr_add_compile_definitions(${PROJ_NAME} -DDLLCLIENT_EX)

pr_finalize(${PROJ_NAME})
