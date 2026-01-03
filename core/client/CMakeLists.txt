include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME client)
pr_add_library(${PROJ_NAME} SHARED)
pr_add_export_macro(${PROJ_NAME} DLLCLIENT)

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
pr_add_dependency(${PROJ_NAME} util_shadergraph TARGET)

pr_add_dependency(${PROJ_NAME} rectangle_bin_pack TARGET)
pr_add_dependency(${PROJ_NAME} Recast TARGET)
pr_add_dependency(${PROJ_NAME} Detour TARGET)
pr_add_dependency(${PROJ_NAME} vorbis TARGET)
pr_add_dependency(${PROJ_NAME} vorbisfile TARGET)
pr_add_dependency(${PROJ_NAME} noise-static TARGET)
pr_add_dependency(${PROJ_NAME} noiseutils-static TARGET)
pr_add_dependency(${PROJ_NAME} muparserx TARGET)
pr_add_dependency(${PROJ_NAME} OpenFBX TARGET)

pr_add_dependency(${PROJ_NAME} tinygltf TARGET)
pr_add_third_party_dependency(${PROJ_NAME} rapidxml)

pr_add_compile_definitions(${PROJ_NAME} -DNOISE_STATIC)
pr_add_compile_definitions(
    ${PROJ_NAME}
        -DMOTION_BLUR_DEBUG_ELEMENT_ENABLED=0
        -DENABLE_BLOB_DEPTH_TEST=0
        -DCHC_DEBUGGING_ENABLED=0
        -DENABLE_OCCLUSION_DEBUG_MODE=0
)
pr_add_compile_definitions(
    ${PROJ_NAME}
        -DENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
        -DENTITY_RENDER_BUFFER_USE_STORAGE_BUFFER=1
        -DDEBUG_RENDER_PERFORMANCE_TEST_ENABLED=0
        -DUSE_LIGHT_SOURCE_UNIFORM_BUFFER=1

        PUBLIC
)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")
pr_add_modules(${PROJ_NAME} "src/")

pr_add_compile_definitions(${PROJ_NAME} -DWMS_STATIC PUBLIC)
pr_add_compile_definitions(${PROJ_NAME} -DDLLCLIENT_EX)

pr_finalize(${PROJ_NAME})
