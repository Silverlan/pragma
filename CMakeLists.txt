cmake_minimum_required(VERSION 3.12)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(search_lib_recursive_full VAR_NAME FILE_PATH)
	set(${VAR_NAME} ${FILE_PATH} CACHE PATH "Path to library.")
endfunction(search_lib_recursive_full)

function(search_lib_recursive_ext VAR_NAME FILE_NAME)
	search_lib_recursive_full(${VAR_NAME} ${CMAKE_CURRENT_BINARY_DIR}/external_libs/${FILE_NAME}/${CMAKE_BUILD_TYPE}/${FILE_NAME}.lib)
	# file(GLOB_RECURSE FILE_PATHS ${FILE_NAME})
	# list(LENGTH FILE_PATHS NUM_FILE_PATHS)
	# if(${NUM_FILE_PATHS} GREATER 0)
	# 	list(GET FILE_PATHS 0 FILE_PATH)
	# 	string(REPLACE "/" "\\" FILE_PATH_N "${FILE_PATH}")
	# 	set(${VAR_NAME} ${FILE_PATH_N} CACHE PATH "Path to library.")
	# endif()
endfunction(search_lib_recursive_ext)

function(search_lib_recursive VAR_NAME FILE_PATH)
	search_lib_recursive_full(${VAR_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${FILE_PATH})
endfunction(search_lib_recursive)

set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)

set(DEPENDENCY_DATASYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/datasystem/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_DATASYSTEM_LIBRARY datasystem)

set(DEPENDENCY_ALSOUNDSYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/alsoundsystem/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_ALSOUNDSYSTEM_LIBRARY alsoundsystem)

set(DEPENDENCY_CMATERIALSYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/materialsystem/cmaterialsystem/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CMATERIALSYSTEM_LIBRARY external_libs/materialsystem/${CMAKE_BUILD_TYPE}/cmaterialsystem.lib)

set(DEPENDENCY_MATERIALSYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/materialsystem/materialsystem/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_MATERIALSYSTEM_LIBRARY external_libs/materialsystem/${CMAKE_BUILD_TYPE}/materialsystem.lib)

set(DEPENDENCY_VFILESYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/vfilesystem/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_VFILESYSTEM_LIBRARY vfilesystem)

set(DEPENDENCY_IGLFW_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/iglfw/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_IGLFW_LIBRARY iglfw)

set(DEPENDENCY_LUASYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/luasystem/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_LUASYSTEM_LIBRARY luasystem)

set(DEPENDENCY_MATHUTIL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/mathutil/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_MATHUTIL_LIBRARY mathutil)

set(DEPENDENCY_SHAREDUTILS_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/sharedutils/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_SHAREDUTILS_LIBRARY sharedutils)
search_lib_recursive(DEPENDENCY_SHAREDUTILS_LIBRARY_STATIC external_libs/sharedutils-static/${CMAKE_BUILD_TYPE}/sharedutils-static.lib)

set(DEPENDENCY_NETWORKMANAGER_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/networkmanager/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_NETWORKMANAGER_LIBRARY external_libs/networkmanager/${CMAKE_BUILD_TYPE}/networkmanager.lib)

set(DEPENDENCY_PROSPER_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/prosper/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_PROSPER_LIBRARY prosper)

set(DEPENDENCY_SHADERINFO_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/shaderinfo/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_SHADERINFO_LIBRARY shaderinfo)

set(DEPENDENCY_SMDMODEL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/smdmodel/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_SMDMODEL_LIBRARY smdmodel)

set(DEPENDENCY_UTIL_BSP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_bsp/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_BSP_LIBRARY util_bsp)

set(DEPENDENCY_UTIL_FGD_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_fgd/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_FGD_LIBRARY util_fgd)

set(DEPENDENCY_UTIL_MMD_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_mmd/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_MMD_LIBRARY util_mmd)

set(DEPENDENCY_UTIL_SMDMODEL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_smdmodel/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_SMDMODEL_LIBRARY util_smdmodel)

set(DEPENDENCY_UTIL_PAD_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_pad/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_PAD_LIBRARY util_pad)

set(CONFIG_ENABLE_EXECUTABLE_FLAG OFF CACHE BOOL "" FORCE)
set(DEPENDENCY_UTIL_PRAGMA_DOC_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_pragma_doc/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_PRAGMA_DOC_LIBRARY util_pragma_doc)

set(DEPENDENCY_UTIL_SOUND_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_sound/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_SOUND_LIBRARY util_sound)

set(DEPENDENCY_UTIL_SOURCE_SCRIPT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_source_script/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_SOURCE_SCRIPT_LIBRARY util_source_script)

set(DEPENDENCY_UTIL_TIMELINE_SCENE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_timeline_scene/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_TIMELINE_SCENE_LIBRARY util_timeline_scene)

set(DEPENDENCY_UTIL_VERSIONED_ARCHIVE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_versioned_archive/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_VERSIONED_ARCHIVE_LIBRARY util_versioned_archive)

set(DEPENDENCY_UTIL_VMF_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_vmf/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_VMF_LIBRARY util_vmf)

set(DEPENDENCY_UTIL_ZIP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_zip/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_UTIL_ZIP_LIBRARY third_party_libs/libzip/lib/${CMAKE_BUILD_TYPE}/zip.lib)

set(DEPENDENCY_UTIL_IMAGE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_image/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_IMAGE_LIBRARY util_image)

set(DEPENDENCY_WGUI_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/wgui/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_WGUI_LIBRARY wgui)

set(DEPENDENCY_SERVER_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/server/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_SERVER_LIBRARY core/server/${CMAKE_BUILD_TYPE}/server.lib)

set(DEPENDENCY_CENGINE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/cengine/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CENGINE_LIBRARY core/cengine/${CMAKE_BUILD_TYPE}/cengine.lib)

set(DEPENDENCY_ENGINE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/engine/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_ENGINE_LIBRARY core/engine/${CMAKE_BUILD_TYPE}/engine.lib)

set(DEPENDENCY_CLIENT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/client/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CLIENT_LIBRARY core/client/${CMAKE_BUILD_TYPE}/client.lib)

set(DEPENDENCY_SHARED_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/shared/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_SHARED_LIBRARY core/shared/${CMAKE_BUILD_TYPE}/shared.lib)

set(DEPENDENCY_WMS_SHARED_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/wms_shared/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_WMS_SHARED_LIBRARY core/wms_shared/${CMAKE_BUILD_TYPE}/wms_shared.lib)

set(DEPENDENCY_ICLIENT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/modules/interfaces/iclient/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_ICLIENT_LIBRARY modules/interfaces/iclient/${CMAKE_BUILD_TYPE}/iclient.lib)

set(DEPENDENCY_ISHARED_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/modules/interfaces/ishared/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_ISHARED_LIBRARY modules/interfaces/ishared/${CMAKE_BUILD_TYPE}/ishared.lib)

set(DEPENDENCY_PR_STEAMWORKS_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/modules/pr_steamworks/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_PR_STEAMWORKS_LIBRARY modules/pr_steamworks/${CMAKE_BUILD_TYPE}/pr_steamworks.lib)

set(DEPENDENCY_VULKAN_INCLUDE $ENV{VULKAN_SDK}/Include CACHE PATH "Path to include directory.")
search_lib_recursive_full(DEPENDENCY_VULKAN_LIBRARY $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)

# Third-party libraries
set(DEPENDENCY_GLM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/glm CACHE PATH "Path to include directory.")
set(DEPENDENCY_GLI_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/gli CACHE PATH "Path to include directory.")

set(DEPENDENCY_GLFW_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/glfw/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_GLFW_LIBRARY third_party_libs/glfw/src/${CMAKE_BUILD_TYPE}/glfw3dll.lib)

set(DEPENDENCY_ANVIL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/anvil/include CACHE PATH "Path to include directory.")

set(DEPENDENCY_CLIP2TRI_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/clip2tri/include CACHE PATH "Path to include directory.")
set(DEPENDENCY_CLIP2TRI_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/clip2tri/${CMAKE_BUILD_TYPE}/clip2tri-static.lib CACHE PATH "Path to library.")

set(DEPENDENCY_RECAST_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/recastnavigation/Recast/Include CACHE PATH "Path to include directory.")
set(DEPENDENCY_DETOUR_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/recastnavigation/Detour/Include CACHE PATH "Path to include directory.")
set(DEPENDENCY_EXPRTK_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/exprtk CACHE PATH "Path to include directory.")
set(DEPENDENCY_LIBZIP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libzip/lib CACHE PATH "Path to include directory.")
set(DEPENDENCY_LIBZIP_CONF_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libzip/xcode CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_LIBZIP_LIBRARY third_party_libs/libzip/lib/${CMAKE_BUILD_TYPE}/zip.lib)

set(DEPENDENCY_LPNG_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libpng CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_LPNG_LIBRARY third_party_libs/libpng/RelWithDebInfo/libpng16.lib)

set(DEPENDENCY_RECTANGLEBINPACK_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/rectangle_bin_pack CACHE PATH "Path to include directory.")
set(DEPENDENCY_MINIBALL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/miniball/cpp/main CACHE PATH "Path to include directory.")

set(DEPENDENCY_SQUISH_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/squish CACHE PATH "Path to include directory.")
set(DEPENDENCY_SQUISH_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/squish/lib64/vs9/squish.lib CACHE PATH "Path to library.")

set(DEPENDENCY_VTFLIB_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/vtflib CACHE PATH "Path to include directory.")
set(DEPENDENCY_VTFLIB_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/vtflib/sln/vs8/VTFLib/x64/Release/VTFLib.lib CACHE PATH "Path to library.")

set(DEPENDENCY_LUAJIT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/luajit/src CACHE PATH "Path to include directory.")
set(DEPENDENCY_LUAJIT_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/luajit/src/luajit.lib CACHE PATH "Path to library.")
set(DEPENDENCY_LUA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/luajit/src/lua51.lib CACHE PATH "Path to library.")

set(DEPENDENCY_ALURE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/alure/include CACHE PATH "Path to include directory.")

set(DEPENDENCY_LIBSAMPLERATE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libsamplerate/src CACHE PATH "Path to include directory.")
set(DEPENDENCY_LIBSAMPLERATE_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libsamplerate/RelWithDebInfo/libsamplerate-0.lib CACHE PATH "Path to library.")

set(DEPENDENCY_ASSIMP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/assimp/include CACHE PATH "Path to include directory.")
set(DEPENDENCY_ASSIMP_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/assimp/code/${CMAKE_BUILD_TYPE}/assimp-vc140-mt.lib CACHE PATH "Path to library.")

set(DEPENDENCY_EIGEN_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/eigen CACHE PATH "Path to include directory.")

set(DEPENDENCY_FREETYPE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/freetype/include CACHE PATH "Path to include directory.")
set(DEPENDENCY_FREETYPE_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/freetype/win64/freetype.lib CACHE PATH "Path to library.")

set(DEPENDENCY_OPENAL_SOFT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/openal-soft/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_OPENAL_SOFT_LIBRARY third_party_libs/openal-soft/${CMAKE_BUILD_TYPE}/OpenAL32.lib)

set(DEPENDENCY_BULLET_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/bullet/src CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_BULLET_LINEAR_MATH_LIBRARY third_party_libs/bullet/src/LinearMath/${CMAKE_BUILD_TYPE}/LinearMath.lib)
search_lib_recursive(DEPENDENCY_BULLET_DYNAMICS_LIBRARY third_party_libs/bullet/src/BulletDynamics/${CMAKE_BUILD_TYPE}/BulletDynamics.lib)
search_lib_recursive(DEPENDENCY_BULLET_COLLISION_LIBRARY third_party_libs/bullet/src/BulletCollision/${CMAKE_BUILD_TYPE}/BulletCollision.lib)


add_subdirectory(core)
add_subdirectory(external_libs)
add_subdirectory(modules)

# include generated headers
target_include_directories(shared PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/assimp/include)

set(LIBRARY_OUTPUT_PATH "" CACHE PATH "" FORCE)

# Openal-soft
add_subdirectory(third_party_libs/openal-soft)
set_target_properties(OpenAL PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(alrecord PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(altonegen PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(build_version PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(common PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(ex-common PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(native-tools PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(openal-info PROPERTIES FOLDER third_party_libs/openal-soft)
#

# Alure
# set(OPENAL_INCLUDE_DIR "${DEPENDENCY_OPENAL_SOFT_INCLUDE}" CACHE STRING "${DEPENDENCY_OPENAL_SOFT_INCLUDE}" FORCE)
# set(OPENAL_LIBRARY "${DEPENDENCY_OPENAL_SOFT_LIBRARY}" CACHE STRING "${DEPENDENCY_OPENAL_SOFT_LIBRARY}" FORCE)

# add_subdirectory(third_party_libs/alure)

# Assimp
add_subdirectory(third_party_libs/assimp)
set_target_properties(assimp PROPERTIES FOLDER third_party_libs/assimp)
set_target_properties(assimp_cmd PROPERTIES FOLDER third_party_libs/assimp)
set_target_properties(IrrXML PROPERTIES FOLDER third_party_libs/assimp)
set_target_properties(uninstall PROPERTIES FOLDER third_party_libs/assimp)
set_target_properties(unit PROPERTIES FOLDER third_party_libs/assimp)
set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER third_party_libs/assimp)
# set_target_properties(zlib PROPERTIES FOLDER third_party_libs/assimp)
# set_target_properties(zlibstatic PROPERTIES FOLDER third_party_libs/assimp)
#

# clip2tri
add_subdirectory(third_party_libs/clip2tri)
set_target_properties(clip2tri PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(clip2tri-static PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(clipper PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(clipper-static PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(poly2tri PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(poly2tri-static PROPERTIES FOLDER third_party_libs/clip2tri)
#

# eigen
# add_subdirectory(third_party_libs/eigen)
#

# glfw
add_subdirectory(third_party_libs/glfw)
set_target_properties(boing PROPERTIES FOLDER third_party_libs/glfw/Examples)
set_target_properties(gears PROPERTIES FOLDER third_party_libs/glfw/Examples)
set_target_properties(heightmap PROPERTIES FOLDER third_party_libs/glfw/Examples)
set_target_properties(offscreen PROPERTIES FOLDER third_party_libs/glfw/Examples)
set_target_properties(particles PROPERTIES FOLDER third_party_libs/glfw/Examples)
set_target_properties(sharing PROPERTIES FOLDER third_party_libs/glfw/Examples)
set_target_properties(simple PROPERTIES FOLDER third_party_libs/glfw/Examples)
set_target_properties(splitview PROPERTIES FOLDER third_party_libs/glfw/Examples)
set_target_properties(wave PROPERTIES FOLDER third_party_libs/glfw/Examples)

set_target_properties(clipboard PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(cursor PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(empty PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(events PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(gamma PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(glfwinfo PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(icon PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(iconify PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(inputlag PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(joysticks PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(monitors PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(msaa PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(opacity PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(reopen PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(tearing PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(threads PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(timeout PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(title PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(vulkan PROPERTIES FOLDER third_party_libs/glfw/Tests)
set_target_properties(windows PROPERTIES FOLDER third_party_libs/glfw/Tests)

set_target_properties(glfw PROPERTIES FOLDER third_party_libs/glfw)
#

# libzip
add_subdirectory(third_party_libs/libzip)
set_target_properties(add_from_filep PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(can_clone_file PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(check PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(cleanup PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(dist PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(distcheck PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(fopen_unchanged PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(fread PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(fseek PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(hole PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(in-memory PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(man PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(nonrandomopen PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(nonrandomopentest PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(testinput PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(tryopen PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(update_zip_err_str PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(update_zip_errors PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(update-html PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(update-man PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(zip PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(zipcmp PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(zipmerge PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(ziptool PROPERTIES FOLDER third_party_libs/libzip)
set_target_properties(ziptool_regress PROPERTIES FOLDER third_party_libs/libzip)
#

# libpng
set(ZLIB_INCLUDE_DIR ${DEPENDENCY_ZLIB_INCLUDE} CACHE STRING "" FORCE)
set(ZLIB_LIBRARY ${DEPENDENCY_ZLIB_LIBRARY} CACHE STRING "" FORCE)
add_subdirectory(third_party_libs/libpng)
set_target_properties(genfiles PROPERTIES FOLDER third_party_libs/libpng)
set_target_properties(png PROPERTIES FOLDER third_party_libs/libpng)
set_target_properties(png_static PROPERTIES FOLDER third_party_libs/libpng)
set_target_properties(pngfix PROPERTIES FOLDER third_party_libs/libpng)
set_target_properties(png-fix-itxt PROPERTIES FOLDER third_party_libs/libpng)
set_target_properties(pngimage PROPERTIES FOLDER third_party_libs/libpng)
set_target_properties(pngstest PROPERTIES FOLDER third_party_libs/libpng)
set_target_properties(pngtest PROPERTIES FOLDER third_party_libs/libpng)
set_target_properties(pngunknown PROPERTIES FOLDER third_party_libs/libpng)
set_target_properties(pngvalid PROPERTIES FOLDER third_party_libs/libpng)
#

# libsamplerate (required for alsoundsystem with fmod enabled)
add_subdirectory(third_party_libs/libsamplerate)
set_target_properties(callback_hang_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(callback_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(clone_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(downsample_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(float_short_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(misc_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(multi_channel_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(multichan_throughput_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(nullptr_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(reset_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(samplerate PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(simple_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(snr_bw_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(streaming_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(termination_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(throughput_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(timewarp-file PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(varispeed_test PROPERTIES FOLDER third_party_libs/libsamplerate)
set_target_properties(varispeed-play PROPERTIES FOLDER third_party_libs/libsamplerate)
#

# anvil
set(ENABLE_HLSL ON CACHE BOOL ON FORCE)
set(ANVIL_LINK_STATICALLY_WITH_VULKAN_LIB OFF CACHE BOOL OFF FORCE)
set(ANVIL_USE_BUILT_IN_VULKAN_HEADERS OFF CACHE BOOL OFF FORCE)
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL ON FORCE)
add_subdirectory(third_party_libs/anvil)
set_target_properties(Anvil PROPERTIES FOLDER third_party_libs/anvil)
set_target_properties(glslangValidator PROPERTIES FOLDER third_party_libs/anvil/tools)
set_target_properties(spirv-remap PROPERTIES FOLDER third_party_libs/anvil/tools)
set_target_properties(glslang PROPERTIES FOLDER third_party_libs/anvil/glslang)
set_target_properties(glslang-default-resource-limits PROPERTIES FOLDER third_party_libs/anvil/glslang)
set_target_properties(OGLCompiler PROPERTIES FOLDER third_party_libs/anvil/glslang)
set_target_properties(OSDependent PROPERTIES FOLDER third_party_libs/anvil/glslang)
set_target_properties(SPIRV PROPERTIES FOLDER third_party_libs/anvil/glslang)
set_target_properties(SPVRemapper PROPERTIES FOLDER third_party_libs/anvil/glslang)
#

# bullet
set(USE_DOUBLE_PRECISION ON CACHE BOOL ON FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL OFF FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL OFF FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL OFF FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL OFF FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL OFF FORCE)
set(CMAKE_RELWITHDEBINFO_POSTFIX "" CACHE STRING "" FORCE)
add_subdirectory(third_party_libs/bullet)
set_target_properties(App_obj2sdf PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(Bullet2FileLoader PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(Bullet3Collision PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(Bullet3Common PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(Bullet3Dynamics PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(Bullet3Geometry PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(Bullet3OpenCL_clew PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(BulletCollision PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(BulletDynamics PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(BulletFileLoader PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(BulletInverseDynamics PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(BulletInverseDynamicsUtils PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(BulletRobotics PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(BulletSoftBody PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(BulletWorldImporter PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(BulletXmlWorldImporter PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(ConvexDecomposition PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(GIMPACTUtils PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(HACD PROPERTIES FOLDER third_party_libs/bullet)
set_target_properties(LinearMath PROPERTIES FOLDER third_party_libs/bullet)
set(BUILD_SHARED_LIBS ON CACHE BOOL ON FORCE)
#

# recastnavigation
set(RECASTNAVIGATION_DEMO OFF CACHE BOOL OFF FORCE)
set(RECASTNAVIGATION_EXAMPLES OFF CACHE BOOL OFF FORCE)
set(RECASTNAVIGATION_STATIC OFF CACHE BOOL OFF FORCE)
set(RECASTNAVIGATION_TESTS OFF CACHE BOOL OFF FORCE)
add_subdirectory(third_party_libs/recastnavigation)
set_target_properties(DebugUtils PROPERTIES FOLDER third_party_libs/recast)
set_target_properties(Detour PROPERTIES FOLDER third_party_libs/recast)
set_target_properties(DetourCrowd PROPERTIES FOLDER third_party_libs/recast)
set_target_properties(DetourTileCache PROPERTIES FOLDER third_party_libs/recast)
set_target_properties(Recast PROPERTIES FOLDER third_party_libs/recast)
#

# vtflib
include_external_msproject(vtflib ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/vtflib/sln/vs8/VTFLib/VTFLib.vcproj)
set_target_properties(vtflib PROPERTIES FOLDER third_party_libs)
#

# squish
include_external_msproject(squish ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/squish/vs9/squish/squish.vcproj)
set_target_properties(squish PROPERTIES FOLDER third_party_libs)
#

# rectangle_bin_pack
include_external_msproject(rectangle_bin_pack ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/rectangle_bin_pack/RectangleBinPack/RectangleBinPack.vcxproj)
set_target_properties(rectangle_bin_pack PROPERTIES FOLDER third_party_libs)
#

# dependencies
add_dependencies(Anvil glslang)
add_dependencies(BulletCollision LinearMath)
add_dependencies(BulletDynamics BulletCollision)
add_dependencies(util_pad zip)

add_dependencies(mathutil sharedutils-static)
add_dependencies(sharedutils mathutil)
add_dependencies(vfilesystem sharedutils)
add_dependencies(datasystem vfilesystem)
add_dependencies(materialsystem datasystem util_image png)
add_dependencies(iglfw glfw)
add_dependencies(prosper vfilesystem iglfw Anvil)
add_dependencies(wgui prosper)

add_dependencies(wgui prosper)
add_dependencies(cmaterialsystem materialsystem prosper)

if(${CONFIG_UNDERLYING_LIBRARY} MATCHES Alure)
	add_dependencies(alsoundsystem OpenAL)
endif()
add_dependencies(alsoundsystem vfilesystem util_sound samplerate)

add_dependencies(servermanager networkmanager)
add_dependencies(clientmanager networkmanager)
add_dependencies(util_pad util_versioned_archive)

add_dependencies(engine materialsystem luasystem networkmanager util_zip util_pragma_doc util_pad wms_shared LinearMath BulletDynamics BulletCollision)
add_dependencies(shared engine networkmanager BulletSoftBody)
add_dependencies(server shared servermanager util_bsp)
add_dependencies(client shared clientmanager)
add_dependencies(cengine client)

add_dependencies(pragma_server server)
add_dependencies(pragma client server)