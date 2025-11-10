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

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")
pr_add_modules(${PROJ_NAME} "src/")

pr_add_compile_definitions(${PROJ_NAME} -DWMS_STATIC PUBLIC)
pr_add_compile_definitions(${PROJ_NAME} -DDLLCLIENT_EX)

pr_finalize(${PROJ_NAME})

if(MSVC)
  # produce per-compile JSON into build/scandeps
  set(SCAN_DIR "${CMAKE_BINARY_DIR}/scandeps")
  file(MAKE_DIRECTORY "${SCAN_DIR}")

  # Add the flag to your target (replace `client` with your actual target)
  target_compile_options(client
    PRIVATE
      "/sourceDependencies"
      "${SCAN_DIR}"
  )
endif()
