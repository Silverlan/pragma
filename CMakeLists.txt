cmake_minimum_required(VERSION 3.12)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(search_lib_recursive_full VAR_NAME FILE_PATH)
	set(${VAR_NAME} ${FILE_PATH} CACHE PATH "Path to library.")
endfunction(search_lib_recursive_full)

function(search_lib_recursive_ext VAR_NAME FILE_NAME)
	search_lib_recursive_full(${VAR_NAME} ${CMAKE_CURRENT_BINARY_DIR}/external_libs/${FILE_NAME}/${CONFIG_BUILD_CONFIGURATION}/${FILE_NAME}.lib)
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

set(CONFIG_BUILD_CONFIGURATION RelWithDebInfo CACHE STRING "Build configuration, used to identify library locations.")

set(DEPENDENCY_DATASYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/datasystem/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_DATASYSTEM_LIBRARY datasystem)

set(DEPENDENCY_ALSOUNDSYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/alsoundsystem/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_ALSOUNDSYSTEM_LIBRARY alsoundsystem)

set(DEPENDENCY_CMATERIALSYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/materialsystem/cmaterialsystem/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CMATERIALSYSTEM_LIBRARY external_libs/materialsystem/cmaterialsystem/cmaterialsystem.lib)

set(DEPENDENCY_MATERIALSYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/materialsystem/materialsystem/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_MATERIALSYSTEM_LIBRARY external_libs/materialsystem/materialsystem/materialsystem.lib)

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

set(DEPENDENCY_NETWORKMANAGER_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/networkmanager/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_NETWORKMANAGER_LIBRARY external_libs/networkmanager/networkmanager.lib)

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

set(DEPENDENCY_UTIL_PAD_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_pad/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_PAD_LIBRARY util_pad)

set(DEPENDENCY_UTIL_PRAGMA_DOC_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_pragma_doc/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_PRAGMA_DOC_LIBRARY util_pragma_doc)

set(DEPENDENCY_UTIL_SOUND_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_sound/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_SOUND_LIBRARY util_sound)

set(DEPENDENCY_UTIL_SOURCE_SCRIPT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_source_script/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_SOURCE_SCRIPT_LIBRARY util_source_script)

set(DEPENDENCY_UTIL_TIMELINE_SCENE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_timeline_scene/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_TIMELINE_SCENE_LIBRARY util_timeline_scene)

set(DEPENDENCY_UTIL_VERSIONED_ARCHIVE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_versioned_archive_scene/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_VERSIONED_ARCHIVE_LIBRARY util_versioned_archive_scene)

set(DEPENDENCY_UTIL_VMF_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_vmf/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_VMF_LIBRARY util_vmf)

set(DEPENDENCY_UTIL_ZIP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_zip/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_ZIP_LIBRARY util_zip)

set(DEPENDENCY_WGUI_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/wgui/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_WGUI_LIBRARY wgui)

set(DEPENDENCY_SERVER_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/server/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_SERVER_LIBRARY server/server.lib)

set(DEPENDENCY_CENGINE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/cengine/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CENGINE_LIBRARY cengine/cengine.lib)

set(DEPENDENCY_ENGINE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/engine/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_ENGINE_LIBRARY engine/cengine.lib)

set(DEPENDENCY_CLIENT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/client/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CLIENT_LIBRARY client/client.lib)

set(DEPENDENCY_SHARED_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/shared/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_SHARED_LIBRARY shared/shared.lib)

set(DEPENDENCY_WMS_SHARED_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/wms_shared/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_WMS_SHARED_LIBRARY shared/wms_shared/wms_shared.lib)

set(DEPENDENCY_PR_STEAMWORKS_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/modules/pr_steamworks/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_PR_STEAMWORKS_LIBRARY modules/pr_steamworks/pr_steamworks.lib)

set(DEPENDENCY_VULKAN_INCLUDE $ENV{VULKAN_SDK}/Include CACHE PATH "Path to include directory.")
search_lib_recursive_full(DEPENDENCY_VULKAN_LIBRARY $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)

add_subdirectory(core)
add_subdirectory(external_libs)
add_subdirectory(modules)
