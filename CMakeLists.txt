cmake_minimum_required(VERSION 3.8)

if(WIN32)
	set(BUILD_TYPE RelWithDebInfo)
else()
	# set(BUILD_TYPE Release)
	set(BUILD_TYPE RelWithDebInfo)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if(UNIX)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set(CONFIG_ENABLE_VTF_SUPPORT OFF CACHE BOOL OFF FORCE) # VTFLib is Windows-only
endif()

set(CMAKE_BUILD_TYPE ${BUILD_TYPE} CACHE STRING "" FORCE)
if(WIN32)
	set(BINARY_PRAGMA_DIR ${BUILD_TYPE} CACHE INTERNAL "" FORCE)
else()
	set(BINARY_PRAGMA_DIR "" CACHE INTERNAL "" FORCE)
endif()

function(search_lib_recursive_full VAR_NAME FILE_PATH)
	if(WIN32)
		set(FILE_PATH_WITH_EXT "${FILE_PATH}.lib")
	else()
		set(FILE_EXTENSION ".so")
		set(extra_macro_args ${ARGN})

		list(LENGTH extra_macro_args num_extra_args)
		if(${num_extra_args} GREATER 0)
			list(GET extra_macro_args 0 optional_arg)
			if(${optional_arg} STREQUAL "STATIC")
				set(FILE_EXTENSION ".a")
			endif()
		endif()

		set(FILE_PATH_WITH_EXT "${FILE_PATH}${FILE_EXTENSION}")
	endif()
	set(${VAR_NAME} ${FILE_PATH_WITH_EXT} CACHE PATH "Path to library." FORCE)
endfunction(search_lib_recursive_full)

function(search_lib_recursive_ext VAR_NAME FILE_NAME)
	set(ARG_STATIC_LIBRARY FALSE)
	set(extra_macro_args ${ARGN})

	list(LENGTH extra_macro_args num_extra_args)
	if(${num_extra_args} GREATER 0)
		list(GET extra_macro_args 0 optional_arg)
		if(${optional_arg} STREQUAL "STATIC")
			set(ARG_STATIC_LIBRARY "STATIC")
		endif()
	endif()

	set(LIB_NAME ${FILE_NAME})
	if(UNIX)
		set(LIB_NAME lib${LIB_NAME})
	endif()
	search_lib_recursive_full(${VAR_NAME} ${CMAKE_CURRENT_BINARY_DIR}/external_libs/${FILE_NAME}/${BINARY_PRAGMA_DIR}/${LIB_NAME} ${ARG_STATIC_LIBRARY})
	# file(GLOB_RECURSE FILE_PATHS ${FILE_NAME})
	# list(LENGTH FILE_PATHS NUM_FILE_PATHS)
	# if(${NUM_FILE_PATHS} GREATER 0)
	# 	list(GET FILE_PATHS 0 FILE_PATH)
	# 	string(REPLACE "/" "\\" FILE_PATH_N "${FILE_PATH}")
	# 	set(${VAR_NAME} ${FILE_PATH_N} CACHE PATH "Path to library.")
	# endif()
endfunction(search_lib_recursive_ext)

function(search_lib_recursive VAR_NAME FILE_PATH LIB_NAME)
	set(ARG_STATIC_LIBRARY FALSE)
	set(ROOT_DIR ${CMAKE_CURRENT_BINARY_DIR})
	set(extra_macro_args ${ARGN})

	list(LENGTH extra_macro_args num_extra_args)
	if(${num_extra_args} GREATER 0)
		list(GET extra_macro_args 0 optional_arg)
		if(${optional_arg} STREQUAL "STATIC")
			set(ARG_STATIC_LIBRARY "STATIC")
		endif()
		if(${num_extra_args} GREATER 1)
			list(GET extra_macro_args 1 optional_arg)
			set(ROOT_DIR ${optional_arg})
		endif()
	endif()

	if(UNIX)
		set(LIB_NAME lib${LIB_NAME})
	endif()
	search_lib_recursive_full(${VAR_NAME} ${ROOT_DIR}/${FILE_PATH}/${LIB_NAME} ${ARG_STATIC_LIBRARY})
endfunction(search_lib_recursive)

set(DEPENDENCY_DATASYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/datasystem/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_DATASYSTEM_LIBRARY datasystem)

set(DEPENDENCY_ALSOUNDSYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/alsoundsystem/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_ALSOUNDSYSTEM_LIBRARY alsoundsystem)

set(DEPENDENCY_CMATERIALSYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/materialsystem/cmaterialsystem/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CMATERIALSYSTEM_LIBRARY external_libs/materialsystem/${BINARY_PRAGMA_DIR} cmaterialsystem)

set(DEPENDENCY_MATERIALSYSTEM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/materialsystem/materialsystem/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_MATERIALSYSTEM_LIBRARY external_libs/materialsystem/${BINARY_PRAGMA_DIR} materialsystem)

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
search_lib_recursive(DEPENDENCY_SHAREDUTILS_LIBRARY_STATIC external_libs/sharedutils-static/${BINARY_PRAGMA_DIR} sharedutils-static STATIC)

set(DEPENDENCY_NETWORKMANAGER_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/networkmanager/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_NETWORKMANAGER_LIBRARY external_libs/networkmanager/${BINARY_PRAGMA_DIR} networkmanager STATIC)
search_lib_recursive(DEPENDENCY_SERVERMANAGER_LIBRARY external_libs/networkmanager/${BINARY_PRAGMA_DIR} servermanager STATIC)
search_lib_recursive(DEPENDENCY_CLIENTMANAGER_LIBRARY external_libs/networkmanager/${BINARY_PRAGMA_DIR} clientmanager STATIC)

set(DEPENDENCY_PROSPER_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/prosper/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_PROSPER_LIBRARY prosper)

set(DEPENDENCY_SHADERINFO_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/shaderinfo/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_SHADERINFO_LIBRARY shaderinfo)

set(DEPENDENCY_SMDMODEL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/smdmodel/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_SMDMODEL_LIBRARY smdmodel)

set(DEPENDENCY_UTIL_BSP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_bsp/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_BSP_LIBRARY util_bsp STATIC)

set(DEPENDENCY_UTIL_ARCHIVE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_archive/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_ARCHIVE_LIBRARY util_archive)

set(DEPENDENCY_UTIL_PCL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_pcl/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_PCL_LIBRARY util_pcl)

set(DEPENDENCY_UTIL_FGD_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_fgd/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_FGD_LIBRARY util_fgd STATIC)

set(DEPENDENCY_UTIL_MMD_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_mmd/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_MMD_LIBRARY util_mmd STATIC)

set(DEPENDENCY_UTIL_DMX_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_dmx/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_DMX_LIBRARY util_dmx)

set(DEPENDENCY_UTIL_SMDMODEL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_smdmodel/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_SMDMODEL_LIBRARY util_smdmodel STATIC)

set(DEPENDENCY_UTIL_PAD_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_pad/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_PAD_LIBRARY util_pad STATIC)

set(CONFIG_ENABLE_EXECUTABLE_FLAG OFF CACHE BOOL "" FORCE)
set(DEPENDENCY_UTIL_PRAGMA_DOC_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_pragma_doc/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_PRAGMA_DOC_LIBRARY util_pragma_doc STATIC)

set(DEPENDENCY_UTIL_SOUND_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_sound/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_SOUND_LIBRARY util_sound STATIC)

set(DEPENDENCY_UTIL_SOURCE_SCRIPT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_source_script/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_SOURCE_SCRIPT_LIBRARY util_source_script STATIC)

set(DEPENDENCY_UTIL_TIMELINE_SCENE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_timeline_scene/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_TIMELINE_SCENE_LIBRARY util_timeline_scene STATIC)

set(DEPENDENCY_UTIL_VERSIONED_ARCHIVE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_versioned_archive/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_VERSIONED_ARCHIVE_LIBRARY util_versioned_archive STATIC)

set(DEPENDENCY_UTIL_VMF_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_vmf/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_VMF_LIBRARY util_vmf STATIC)

set(DEPENDENCY_UTIL_ZIP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_zip/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_UTIL_ZIP_LIBRARY external_libs/util_zip/${BINARY_PRAGMA_DIR} util_zip STATIC)
if(WIN32)
	set(ZLIB_LIB_NAME "zlib" CACHE INTERNAL "" FORCE)
else()
	set(ZLIB_LIB_NAME "z" CACHE INTERNAL "" FORCE)
endif()
search_lib_recursive(DEPENDENCY_ZLIB_LIBRARY third_party_libs/zlib/${BINARY_PRAGMA_DIR} ${ZLIB_LIB_NAME})

set(DEPENDENCY_UTIL_IMAGE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/util_image/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_UTIL_IMAGE_LIBRARY util_image STATIC)

set(DEPENDENCY_WGUI_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/external_libs/wgui/include CACHE PATH "Path to include directory.")
search_lib_recursive_ext(DEPENDENCY_WGUI_LIBRARY wgui)

set(DEPENDENCY_SERVER_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/server/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_SERVER_LIBRARY core/server/${BINARY_PRAGMA_DIR} server)

set(DEPENDENCY_CENGINE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/cengine/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CENGINE_LIBRARY core/cengine/${BINARY_PRAGMA_DIR} cengine)

set(DEPENDENCY_ENGINE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/engine/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_ENGINE_LIBRARY core/engine/${BINARY_PRAGMA_DIR} engine)

set(DEPENDENCY_CLIENT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/client/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CLIENT_LIBRARY core/client/${BINARY_PRAGMA_DIR} client)

set(DEPENDENCY_SHARED_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/shared/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_SHARED_LIBRARY core/shared/${BINARY_PRAGMA_DIR} shared)

set(DEPENDENCY_WMS_SHARED_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/core/wms_shared/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_WMS_SHARED_LIBRARY core/wms_shared/${BINARY_PRAGMA_DIR} wms_shared STATIC)

set(DEPENDENCY_ICLIENT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/modules/interfaces/iclient/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_ICLIENT_LIBRARY modules/interfaces/iclient/${BINARY_PRAGMA_DIR} iclient)

set(DEPENDENCY_ISHARED_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/modules/interfaces/ishared/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_ISHARED_LIBRARY modules/interfaces/ishared/${BINARY_PRAGMA_DIR} ishared)

set(DEPENDENCY_PR_STEAMWORKS_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/modules/pr_steamworks/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_PR_STEAMWORKS_LIBRARY modules/pr_steamworks/${BINARY_PRAGMA_DIR} pr_steamworks)

set(DEPENDENCY_LIBVORBIS_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/vorbis/include CACHE STRING "" FORCE)
search_lib_recursive(DEPENDENCY_LIBVORBIS_LIBRARY third_party_libs/vorbis/lib/${BINARY_PRAGMA_DIR} vorbis)
search_lib_recursive(DEPENDENCY_LIBVORBISFILE_LIBRARY third_party_libs/vorbis/lib/${BINARY_PRAGMA_DIR} vorbisfile)

search_lib_recursive(DEPENDENCY_LIBOGG_LIBRARY third_party_libs/ogg/${BINARY_PRAGMA_DIR} ogg)
set(DEPENDENCY_LIBOGG_BUILD_INCLUDE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/ogg/include CACHE PATH "Path to include directory.")
set(OGG_LIBRARIES ${DEPENDENCY_LIBOGG_LIBRARY} CACHE STRING "" FORCE)

set(DEPENDENCY_BZIP2_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/bzip2 CACHE STRING "" FORCE)
search_lib_recursive(DEPENDENCY_BZIP2_LIBRARY third_party_libs/bzip2/${BINARY_PRAGMA_DIR} bz2)

set(DEPENDENCY_LUABIND_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/luabind CACHE STRING "" FORCE)
search_lib_recursive(DEPENDENCY_LUABIND_LIBRARY lib/${BINARY_PRAGMA_DIR} luabind)

set(DEPENDENCY_FREETYPE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/freetype2/include CACHE STRING "" FORCE)
search_lib_recursive(DEPENDENCY_FREETYPE_LIBRARY third_party_libs/freetype2/${BINARY_PRAGMA_DIR} freetype)

set(DEPENDENCY_RAPIDXML_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/rapidxml CACHE STRING "" FORCE)
set(DEPENDENCY_GLSLANG_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/anvil/deps/glslang CACHE STRING "" FORCE)
search_lib_recursive(DEPENDENCY_GLSLANG_LIBRARY third_party_libs/anvil/deps/glslang/glslang glslang STATIC)

set(DEPENDENCY_LUA_INCLUDE ${DEPENDENCY_LUAJIT_INCLUDE} CACHE STRING "" FORCE)

if(UNIX)
	set(VULKAN_SDK "" CACHE PATH "Path to Vulkan SDK.")
	search_lib_recursive(DEPENDENCY_GLSLANG_OS_DEPENDENT_LIBRARY third_party_libs/anvil/deps/glslang/glslang/OSDependent/Unix OSDependent STATIC)
else()
	set(VULKAN_SDK $ENV{VULKAN_SDK} INTERNAL "Path to Vulkan SDK.")
endif()

set(DEPENDENCY_VULKAN_INCLUDE ${VULKAN_SDK}/Include CACHE PATH "Path to include directory.")
if(WIN32)
	search_lib_recursive(DEPENDENCY_SPIRV_TOOLS_LIBRARY third_party_libs/vulkan/spirv-tools/source/${BINARY_PRAGMA_DIR} SPIRV-Tools)
	search_lib_recursive(DEPENDENCY_SPIRV_TOOLS_OPT_LIBRARY third_party_libs/vulkan/spirv-tools/source/opt/${BINARY_PRAGMA_DIR} SPIRV-Tools-opt)
	search_lib_recursive(DEPENDENCY_GLSLANG_LIBRARY third_party_libs/anvil/deps/glslang/glslang/${BINARY_PRAGMA_DIR} glslang)
	search_lib_recursive(DEPENDENCY_GLSLANG_OGL_COMPILER_LIBRARY third_party_libs/anvil/deps/glslang/OGLCompilersDLL/${BINARY_PRAGMA_DIR} OGLCompiler)
	search_lib_recursive(DEPENDENCY_SPIRV_LIBRARY third_party_libs/anvil/deps/glslang/SPIRV/${BINARY_PRAGMA_DIR} SPIRV)
	search_lib_recursive(DEPENDENCY_GLSLANG_OS_DEPENDENT_LIBRARY third_party_libs/anvil/deps/glslang/glslang/OSDependent/Windows/${BINARY_PRAGMA_DIR} OSDependent)
	search_lib_recursive(DEPENDENCY_SPV_REMAPPER_LIBRARY third_party_libs/anvil/deps/glslang/SPIRV/${BINARY_PRAGMA_DIR} SPVRemapper)
	# search_lib_recursive(DEPENDENCY_HLSL_LIBRARY ${VULKAN_SDK}/glslang/${BINARY_PRAGMA_DIR}/hlsl/${BINARY_PRAGMA_DIR}/HLSL)

	search_lib_recursive_full(DEPENDENCY_VULKAN_LIBRARY ${VULKAN_SDK}/Lib/vulkan-1)
else()
	search_lib_recursive_full(DEPENDENCY_VULKAN_LIBRARY /usr/lib/x86_64-linux-gnu/libvulkan)
	# search_lib_recursive_full(DEPENDENCY_GLSLANG_LIBRARY )
	# search_lib_recursive_full(DEPENDENCY_GLSLANG_OGL_COMPILER_LIBRARY )
	# search_lib_recursive_full(DEPENDENCY_GLSLANG_OS_DEPENDENT_LIBRARY )
	# search_lib_recursive_full(DEPENDENCY_HLSL_LIBRARY )
	# search_lib_recursive_full(DEPENDENCY_SPIRV_LIBRARY /usr/lib/x86_64-linux-gnu/libSPIRV)
	search_lib_recursive_full(DEPENDENCY_SPIRV_TOOLS_LIBRARY /usr/lib/x86_64-linux-gnu/libSPIRV-Tools STATIC)
	search_lib_recursive_full(DEPENDENCY_SPIRV_TOOLS_OPT_LIBRARY /usr/lib/x86_64-linux-gnu/libSPIRV-Tools-opt STATIC)
	# search_lib_recursive_full(DEPENDENCY_SPV_REMAPPER_LIBRARY /usr/lib/x86_64-linux-gnu/) == TODO
endif()

# Third-party libraries
set(DEPENDENCY_GLM_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/glm CACHE PATH "Path to include directory.")
set(DEPENDENCY_GLI_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/gli CACHE PATH "Path to include directory.")

set(DEPENDENCY_GLFW_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/glfw/include CACHE PATH "Path to include directory.")

set(DEPENDENCY_GLFW_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/glfw/include CACHE PATH "Path to include directory.")
if(WIN32)
	set(GLFW_LIB_NAME "glfw3dll" CACHE INTERNAL "" FORCE)
else()
	set(GLFW_LIB_NAME "glfw" CACHE INTERNAL "" FORCE)
endif()
search_lib_recursive(DEPENDENCY_GLFW_LIBRARY third_party_libs/glfw/src/${BINARY_PRAGMA_DIR} ${GLFW_LIB_NAME})

set(DEPENDENCY_ANVIL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/anvil/include CACHE PATH "Path to include directory.")
set(DEPENDENCY_ANVIL_BUILD_INCLUDE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/anvil/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_ANVIL_LIBRARY third_party_libs/anvil/${BINARY_PRAGMA_DIR} Anvil STATIC)

set(DEPENDENCY_CLIP2TRI_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/clip2tri CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_CLIP2TRI_LIBRARY third_party_libs/clip2tri/${BINARY_PRAGMA_DIR} clip2tri-static STATIC)
search_lib_recursive(DEPENDENCY_CLIPPER_LIBRARY third_party_libs/clip2tri/clipper/${BINARY_PRAGMA_DIR} clipper-static STATIC)
search_lib_recursive(DEPENDENCY_POLY2TRI_LIBRARY third_party_libs/clip2tri/poly2tri/${BINARY_PRAGMA_DIR} poly2tri-static STATIC)

set(DEPENDENCY_RECAST_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/recastnavigation/Recast/Include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_RECAST_LIBRARY third_party_libs/recastnavigation/Recast/${BINARY_PRAGMA_DIR} Recast STATIC)

set(DEPENDENCY_DETOUR_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/recastnavigation/Detour/Include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_DETOUR_LIBRARY third_party_libs/recastnavigation/Detour/${BINARY_PRAGMA_DIR} Detour STATIC)

set(DEPENDENCY_EXPRTK_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/exprtk CACHE PATH "Path to include directory.")
set(DEPENDENCY_LIBZIP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libzip/lib CACHE PATH "Path to include directory.")
set(DEPENDENCY_LIBZIP_CONF_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libzip/xcode CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_LIBZIP_LIBRARY third_party_libs/libzip/lib/${BINARY_PRAGMA_DIR} zip)

set(DEPENDENCY_LPNG_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libpng CACHE PATH "Path to include directory.")
if(WIN32)
	set(LIBPNG_LIB_NAME "libpng16" CACHE INTERNAL "" FORCE)
else()
	set(LIBPNG_LIB_NAME "png16" CACHE INTERNAL "" FORCE)
endif()
search_lib_recursive(DEPENDENCY_LPNG_LIBRARY third_party_libs/libpng/${BINARY_PRAGMA_DIR} ${LIBPNG_LIB_NAME})

set(DEPENDENCY_RECTANGLEBINPACK_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/rectangle_bin_pack CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_RECTANGLEBINPACK_LIBRARY third_party_libs/rectangle_bin_pack/${BINARY_PRAGMA_DIR} rectangle_bin_pack STATIC)

set(DEPENDENCY_MINIBALL_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/miniball/cpp/main CACHE PATH "Path to include directory.")

set(DEPENDENCY_SQUISH_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/squish CACHE PATH "Path to include directory.")
set(DEPENDENCY_SQUISH_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/squish/lib64/vs9/squish CACHE PATH "Path to library.")

set(DEPENDENCY_VTFLIB_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/vtflib CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_VTFLIB_LIBRARY third_party_libs/vtflib/sln/vs8/VTFLib/x64/Release VTFLib DYNAMIC ${CMAKE_CURRENT_LIST_DIR})

set(DEPENDENCY_LUAJIT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/luajit/src CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_LUAJIT_LIBRARY third_party_libs/luajit/src luajit STATIC ${CMAKE_CURRENT_LIST_DIR})
if(WIN32)
	search_lib_recursive(DEPENDENCY_LUA_LIBRARY third_party_libs/luajit/src lua51 STATIC ${CMAKE_CURRENT_LIST_DIR})
else()
	search_lib_recursive(DEPENDENCY_LUA_LIBRARY third_party_libs/luajit/src luajit STATIC ${CMAKE_CURRENT_LIST_DIR})
endif()

set(DEPENDENCY_ALURE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/alure/include CACHE PATH "Path to include directory.")

set(DEPENDENCY_LIBSAMPLERATE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libsamplerate/src CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_LIBSAMPLERATE_LIBRARY third_party_libs/libsamplerate/${BINARY_PRAGMA_DIR} samplerate STATIC)

set(DEPENDENCY_UTF8_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/utf8/source CACHE PATH "Path to include directory.")

set(DEPENDENCY_LIBNOISE_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libnoise/src CACHE PATH "Path to include directory.")
set(DEPENDENCY_LIBNOISEUTILS_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libnoise/noiseutils CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_LIBNOISE_LIBRARY third_party_libs/libnoise/src/${BINARY_PRAGMA_DIR} noise STATIC)
search_lib_recursive(DEPENDENCY_LIBNOISEUTILS_LIBRARY third_party_libs/libnoise/noiseutils/${BINARY_PRAGMA_DIR} noiseutils-static STATIC)

set(DEPENDENCY_HLLIB_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/hllib/HLLib CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_HLLIB_LIBRARY third_party_libs/hllib/sln/vs17/HLLib/x64/${BINARY_PRAGMA_DIR} HLLib)

if(WIN32)
	set(ASSIMP_LIB_NAME assimp-vc140-mt)
else()
	set(ASSIMP_LIB_NAME assimp)
endif()
set(DEPENDENCY_ASSIMP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/assimp/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_ASSIMP_LIBRARY third_party_libs/assimp/code/${BINARY_PRAGMA_DIR} ${ASSIMP_LIB_NAME})

set(DEPENDENCY_EIGEN_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/eigen CACHE PATH "Path to include directory.")

set(DEPENDENCY_OPENAL_SOFT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/openal-soft/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_OPENAL_SOFT_LIBRARY third_party_libs/openal-soft/${BINARY_PRAGMA_DIR} OpenAL32)

set(DEPENDENCY_BULLET_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/bullet/src CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_BULLET_LINEAR_MATH_LIBRARY third_party_libs/bullet/src/LinearMath/${BINARY_PRAGMA_DIR} LinearMath STATIC)
search_lib_recursive(DEPENDENCY_BULLET_DYNAMICS_LIBRARY third_party_libs/bullet/src/BulletDynamics/${BINARY_PRAGMA_DIR} BulletDynamics STATIC)
search_lib_recursive(DEPENDENCY_BULLET_COLLISION_LIBRARY third_party_libs/bullet/src/BulletCollision/${BINARY_PRAGMA_DIR} BulletCollision STATIC)
search_lib_recursive(DEPENDENCY_BULLET_SOFTBODY_LIBRARY third_party_libs/bullet/src/BulletSoftBody/${BINARY_PRAGMA_DIR} BulletSoftBody STATIC)

set(DEPENDENCY_NIFTOOLS_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/niflib/include CACHE PATH "Path to include directory.")
search_lib_recursive(DEPENDENCY_NIFTOOLS_LIBRARY third_party_libs/niflib/${BINARY_PRAGMA_DIR} niflib_static)

message("Processing core libraries...")
add_subdirectory(core)

message("Processing external libraries...")
add_subdirectory(external_libs)

set_target_properties(wgui PROPERTIES FOLDER external_libs/wgui)
set_target_properties(sample_application PROPERTIES FOLDER external_libs/wgui)
target_include_directories(util_image PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
target_include_directories(cmaterialsystem PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)

message("Processing modules...")
add_subdirectory(modules)

message("Processing third-party libaries...")
# include generated headers
target_include_directories(shared PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/assimp/include)

set(LIBRARY_OUTPUT_PATH "" CACHE PATH "" FORCE)

function(register_third_party_library LIB_NAME)
	message("Processing third-party library '${LIB_NAME}'...")
	set(extra_macro_args ${ARGN})

	list(LENGTH extra_macro_args num_extra_args)
	if(${num_extra_args} GREATER 0)
		list(GET extra_macro_args 0 optional_arg)
		add_subdirectory(third_party_libs/${LIB_NAME} third_party_libs/${optional_arg})
		return()
	endif()
	add_subdirectory(third_party_libs/${LIB_NAME})
endfunction(register_third_party_library)

# Vulkan
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL ON FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL ".." FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL OFF FORCE)
add_subdirectory(${VULKAN_SDK}/spirv-tools third_party_libs/vulkan/spirv-tools)
set(BUILD_SHARED_LIBS ON CACHE BOOL ".." FORCE)
set_target_properties(core_tables PROPERTIES FOLDER third_party_libs/anvil/tools/spirv)
set_target_properties(enum_string_mapping PROPERTIES FOLDER third_party_libs/anvil/tools/spirv)
set_target_properties(extinst_tables PROPERTIES FOLDER third_party_libs/anvil/tools/spirv)
set_target_properties(spirv-tools-pkg-config PROPERTIES FOLDER third_party_libs/anvil/tools/spirv)
set_target_properties(spirv-tools-shared-pkg-config PROPERTIES FOLDER third_party_libs/anvil/tools/spirv)

set_target_properties(spirv-tools-build-version PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/build)
set_target_properties(spirv-tools-debuginfo PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/build)
set_target_properties(spirv-tools-header-DebugInfo PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/build)
set_target_properties(spirv-tools-spv-amd-gcn-shader PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/build)
set_target_properties(spirv-tools-spv-amd-shader-ballot PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/build)
set_target_properties(spirv-tools-spv-amd-shader-explicit-vertex-parameter PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/build)
set_target_properties(spirv-tools-spv-amd-shader-trinary-minmax PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/build)

set_target_properties(SPIRV-Tools PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/libraries)
set_target_properties(SPIRV-Tools-link PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/libraries)
set_target_properties(SPIRV-Tools-opt PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/libraries)
set_target_properties(SPIRV-Tools-reduce PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/libraries)
set_target_properties(SPIRV-Tools-shared PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/libraries)

set_target_properties(spirv-tools-vimsyntax PROPERTIES FOLDER third_party_libs/anvil/tools/spirv/utilities)
#


# Openal-soft
set(ALSOFT_EXAMPLES OFF CACHE BOOL OFF FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL OFF FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL OFF FORCE)
register_third_party_library(openal-soft)
set_target_properties(OpenAL PROPERTIES FOLDER third_party_libs/openal-soft)
#set_target_properties(alrecord PROPERTIES FOLDER third_party_libs/openal-soft)
#set_target_properties(altonegen PROPERTIES FOLDER third_party_libs/openal-soft)
#set_target_properties(build_version PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(common PROPERTIES FOLDER third_party_libs/openal-soft)
#set_target_properties(ex-common PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(native-tools PROPERTIES FOLDER third_party_libs/openal-soft)
set_target_properties(openal-info PROPERTIES FOLDER third_party_libs/openal-soft)
#

# Alure
# set(OPENAL_INCLUDE_DIR "${DEPENDENCY_OPENAL_SOFT_INCLUDE}" CACHE STRING "${DEPENDENCY_OPENAL_SOFT_INCLUDE}" FORCE)
# set(OPENAL_LIBRARY "${DEPENDENCY_OPENAL_SOFT_LIBRARY}" CACHE STRING "${DEPENDENCY_OPENAL_SOFT_LIBRARY}" FORCE)

# register_third_party_library(alure)

# Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL ".." FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL ".." FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT ON CACHE BOOL ".." FORCE)
register_third_party_library(assimp)
set_target_properties(assimp PROPERTIES FOLDER third_party_libs/assimp)
set_target_properties(IrrXML PROPERTIES FOLDER third_party_libs/assimp)
set_target_properties(uninstall PROPERTIES FOLDER third_party_libs/assimp)
if(WIN32)
	set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER third_party_libs/assimp)
endif()
target_include_directories(assimp PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
#

# LuaJIT
# register_third_party_library(luajit)
#

# Luabind
set(LUA_INCLUDE_DIR ${DEPENDENCY_LUAJIT_INCLUDE} CACHE STRING "" FORCE)
set(LUA_LIBRARY ${DEPENDENCY_LUA_LIBRARY} CACHE STRING "" FORCE)
set(LUA_FOUND ON CACHE BOOL ON FORCE)
set(LUA_LIBRARIES ${LUA_LIBRARY} CACHE STRING "" FORCE)
register_third_party_library(luabind)
set_target_properties(luabind PROPERTIES FOLDER third_party_libs)
target_include_directories(luabind PUBLIC ${LUA_INCLUDE_DIR})
#

# clip2tri
register_third_party_library(clip2tri)
set_target_properties(clip2tri PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(clip2tri-static PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(clipper PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(clipper-static PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(poly2tri PROPERTIES FOLDER third_party_libs/clip2tri)
set_target_properties(poly2tri-static PROPERTIES FOLDER third_party_libs/clip2tri)
#

# eigen
# register_third_party_library(eigen)
#

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL OFF FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL OFF FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL OFF FORCE)
set(GLFW_INSTALL OFF CACHE BOOL OFF FORCE)
set(VULKAN_INCLUDE_DIR ${DEPENDENCY_VULKAN_INCLUDE} CACHE STRING "" FORCE)
set(VULKAN_LIBRARY ${DEPENDENCY_VULKAN_LIBRARY} CACHE STRING "" FORCE)

register_third_party_library(glfw)
set_target_properties(glfw PROPERTIES FOLDER third_party_libs/glfw)
#

# libogg
set(OGG_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/ogg/include CACHE STRING "" FORCE)
set(DEPENDENCY_LIBOGG_INCLUDE ${OGG_INCLUDE_DIRS} CACHE STRING "" FORCE)
register_third_party_library(ogg)
set_target_properties(ogg PROPERTIES FOLDER third_party_libs)
#

# libvorbis
register_third_party_library(vorbis)
set_target_properties(vorbis PROPERTIES FOLDER third_party_libs/vorbis)
set_target_properties(vorbisenc PROPERTIES FOLDER third_party_libs/vorbis)
set_target_properties(vorbisfile PROPERTIES FOLDER third_party_libs/vorbis)
target_include_directories(vorbis PUBLIC "${DEPENDENCY_LIBOGG_BUILD_INCLUDE}")
#

# zlib
register_third_party_library(zlib)
set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/zlib CACHE STRING "" FORCE)
set(DEPENDENCY_ZLIB_INCLUDE ${ZLIB_INCLUDE_DIR} CACHE STRING "" FORCE)
target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
target_include_directories(zlibstatic PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
set_target_properties(example PROPERTIES FOLDER third_party_libs/zlib)
set_target_properties(minigzip PROPERTIES FOLDER third_party_libs/zlib)
set_target_properties(zlib PROPERTIES FOLDER third_party_libs/zlib)
set_target_properties(zlibstatic PROPERTIES FOLDER third_party_libs/zlib)

target_include_directories(materialsystem PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
#

# libzip
set(ZLIB_VERSION_STRING "1.2.0" CACHE STRING "" FORCE) # Arbitrary version number larger then 1.1.2 to disable an error message from libzip, because the zlib-library does not exist at this point in time yet
set(ZLIB_INCLUDE_DIR ${DEPENDENCY_ZLIB_INCLUDE} CACHE STRING "" FORCE)
set(ZLIB_LIBRARY ${DEPENDENCY_ZLIB_LIBRARY} CACHE STRING "" FORCE)
register_third_party_library(libzip)
target_include_directories(zip PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
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

# bzip2
register_third_party_library(bzip2)
set_target_properties(bzip2 PROPERTIES FOLDER third_party_libs/bzip2)
set_target_properties(bzip2recover PROPERTIES FOLDER third_party_libs/bzip2)
set_target_properties(libz2 PROPERTIES FOLDER third_party_libs/bzip2)
#

# libnoise
set(BUILD_LIBNOISE_EXAMPLES OFF CACHE BOOL ".." FORCE)
set(BUILD_LIBNOISE_UTILS ON CACHE BOOL ".." FORCE)
unset(BUILD_SHARED_LIBS CACHE)
register_third_party_library(libnoise)
set(BUILD_SHARED_LIBS ON CACHE BOOL ".." FORCE)
set_target_properties(noise PROPERTIES FOLDER third_party_libs/libnoise)
set_target_properties(noise-static PROPERTIES FOLDER third_party_libs/libnoise)
set_target_properties(noiseutils PROPERTIES FOLDER third_party_libs/libnoise)
set_target_properties(noiseutils-static PROPERTIES FOLDER third_party_libs/libnoise)

target_include_directories(noiseutils-static PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libnoise/src)
#

# libpng
register_third_party_library(libpng)
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

target_include_directories(png PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
#

# libsamplerate (required for alsoundsystem with fmod enabled)
set(BUILD_SHARED_LIBS OFF CACHE BOOL OFF FORCE)
register_third_party_library(libsamplerate)
set(BUILD_SHARED_LIBS ON CACHE BOOL ON FORCE)
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
# set(ANVIL_INCLUDE_XCB_WINDOW_SYSTEM_SUPPORT OFF CACHE BOOL OFF FORCE)
set(ANVIL_LINK_STATICALLY_WITH_VULKAN_LIB ON CACHE BOOL ON FORCE)
set(ANVIL_USE_BUILT_IN_VULKAN_HEADERS OFF CACHE BOOL OFF FORCE)
set(ENABLE_HLSL ON CACHE BOOL ".." FORCE)
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL ON FORCE)
set(VULKAN_LIBRARY ${DEPENDENCY_VULKAN_LIBRARY} CACHE PATH "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL OFF FORCE)
register_third_party_library(anvil)
set(BUILD_SHARED_LIBS ON CACHE BOOL ON FORCE)
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
register_third_party_library(bullet)
set(BUILD_SHARED_LIBS ON CACHE BOOL ON FORCE)
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

if(CONFIG_ENABLE_BULLET_PROFILING)
	target_compile_definitions(Bullet2FileLoader PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(Bullet3Collision PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(Bullet3Common PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(Bullet3Dynamics PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(Bullet3Geometry PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(Bullet3OpenCL_clew PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(BulletCollision PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(BulletDynamics PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(BulletFileLoader PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(BulletInverseDynamics PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(BulletInverseDynamicsUtils PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(BulletRobotics PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(BulletSoftBody PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(BulletWorldImporter PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(BulletXmlWorldImporter PUBLIC BT_ENABLE_PROFILE)
	target_compile_definitions(LinearMath PUBLIC BT_ENABLE_PROFILE)
endif()
#

# recastnavigation
set(RECASTNAVIGATION_DEMO OFF CACHE BOOL OFF FORCE)
set(RECASTNAVIGATION_EXAMPLES OFF CACHE BOOL OFF FORCE)
set(RECASTNAVIGATION_STATIC ON CACHE BOOL ON FORCE)
set(RECASTNAVIGATION_TESTS OFF CACHE BOOL OFF FORCE)
register_third_party_library(recastnavigation)
set_target_properties(DebugUtils PROPERTIES FOLDER third_party_libs/recast)
set_target_properties(Detour PROPERTIES FOLDER third_party_libs/recast)
set_target_properties(DetourCrowd PROPERTIES FOLDER third_party_libs/recast)
set_target_properties(DetourTileCache PROPERTIES FOLDER third_party_libs/recast)
set_target_properties(Recast PROPERTIES FOLDER third_party_libs/recast)
#

# vtflib
if(WIN32)
	include_external_msproject(vtflib ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/vtflib/sln/vs8/VTFLib/VTFLib.vcproj)
	set_target_properties(vtflib PROPERTIES FOLDER third_party_libs)
endif()
#

# squish
if(WIN32)
	include_external_msproject(squish ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/squish/vs9/squish/squish.vcproj)
	set_target_properties(squish PROPERTIES FOLDER third_party_libs)
endif()
#

# rectangle_bin_pack
# if(WIN32)
# 	include_external_msproject(rectangle_bin_pack ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/rectangle_bin_pack/RectangleBinPack/RectangleBinPack.vcxproj)
# 	set_target_properties(rectangle_bin_pack PROPERTIES FOLDER third_party_libs)
# else()
	register_third_party_library(rectangle_bin_pack_cmake rectangle_bin_pack)
	set_target_properties(rectangle_bin_pack PROPERTIES FOLDER third_party_libs)
# endif()
#

# freetype2
register_third_party_library(freetype2)
set_target_properties(freetype PROPERTIES FOLDER third_party_libs)
target_include_directories(freetype PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
#

# hllib
if(WIN32)
	include_external_msproject(hllib ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/hllib/sln/vs17/HLLib/HLLib.vcxproj)
	set_target_properties(hllib PROPERTIES FOLDER third_party_libs)
endif()
#

# niflib
set(BOOST_INCLUDEDIR ${DEPENDENCY_BOOST_INCLUDE} CACHE PATH "Path to include directory.")
register_third_party_library(niflib)
set_target_properties(bslightingshaderproperty_test PROPERTIES FOLDER third_party_libs/niflib)
set_target_properties(missing_link_stack_test PROPERTIES FOLDER third_party_libs/niflib)
set_target_properties(niflib PROPERTIES FOLDER third_party_libs/niflib)
set_target_properties(niflib_static PROPERTIES FOLDER third_party_libs/niflib)
set_target_properties(ninode_test PROPERTIES FOLDER third_party_libs/niflib)
set_target_properties(numuvsets_test PROPERTIES FOLDER third_party_libs/niflib)
set_target_properties(skin_test PROPERTIES FOLDER third_party_libs/niflib)
set_target_properties(skinpart_test PROPERTIES FOLDER third_party_libs/niflib)
set_target_properties(trishape_test PROPERTIES FOLDER third_party_libs/niflib)
set_target_properties(write_test PROPERTIES FOLDER third_party_libs/niflib)
target_include_directories(niflib_static PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/niflib)
#

# qhull
register_third_party_library(qhull)
set_target_properties(libqhull PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qconvex PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qdelaunay PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qhalf PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qhull PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qhull_p PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qhull_r PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qhullcpp PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qhullp PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qhullstatic PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qhullstatic_r PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(qvoronoi PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(rbox PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(testqset PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(testqset_r PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(user_eg PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(user_eg2 PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(user_eg3 PROPERTIES FOLDER third_party_libs/qhull)
set_target_properties(user_egp PROPERTIES FOLDER third_party_libs/qhull)
#

# dependencies
# add_dependencies(Anvil glslang)
add_dependencies(png zlib)
add_dependencies(vorbis ogg)
add_dependencies(BulletCollision LinearMath)
add_dependencies(BulletDynamics BulletCollision)
add_dependencies(util_pad zip)
add_dependencies(pr_pcl util_pcl)
add_dependencies(niflib_static qhullstatic)
add_dependencies(pr_source util_archive util_dmx niflib_static)
add_dependencies(pr_dmx util_dmx)
add_dependencies(pr_mdleditor util_archive ishared pr_source)

if(WIN32)
	add_dependencies(pr_mdleditor hllib)
endif()

add_dependencies(mathutil sharedutils-static)
add_dependencies(sharedutils mathutil)
add_dependencies(vfilesystem sharedutils)
add_dependencies(datasystem vfilesystem)
add_dependencies(luasystem vfilesystem luabind)
add_dependencies(materialsystem datasystem util_image png)
add_dependencies(iglfw glfw)
add_dependencies(iglfw vfilesystem)
add_dependencies(prosper vfilesystem iglfw Anvil util_image SPIRV SPIRV-Tools OSDependent OGLCompiler SPIRV-Tools-opt SPVRemapper glslang)

add_dependencies(cmaterialsystem materialsystem prosper)
add_dependencies(wgui cmaterialsystem freetype glslang)

if(${CONFIG_UNDERLYING_LIBRARY} MATCHES Alure)
	add_dependencies(alsoundsystem OpenAL)
endif()
add_dependencies(alsoundsystem vfilesystem util_sound samplerate)

add_dependencies(servermanager networkmanager)
add_dependencies(clientmanager networkmanager)
add_dependencies(util_pad util_versioned_archive)

add_dependencies(engine materialsystem luasystem networkmanager util_zip util_pragma_doc util_pad wms_shared LinearMath BulletDynamics BulletCollision BulletSoftBody libz2 bzip2)
add_dependencies(shared engine networkmanager BulletSoftBody clip2tri-static clipper-static poly2tri-static util_vmf util_fgd util_mmd util_bsp util_smdmodel util_source_script rectangle_bin_pack assimp Recast Detour util_pad noise-static noiseutils-static ogg vorbis vorbisfile)
add_dependencies(server shared servermanager clientmanager util_bsp util_sound)
add_dependencies(client shared clientmanager freetype wgui alsoundsystem util_timeline_scene)
add_dependencies(cengine client util_timeline_scene)

add_dependencies(pragma_server server)
add_dependencies(pragma client server)

set_property(TARGET pragma PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output")
set_property(TARGET pragma_server PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output")

set_property(TARGET pragma PROPERTY VS_DEBUGGER_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/output/pragma.exe")
set_property(TARGET pragma_server PROPERTY VS_DEBUGGER_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/output/pragma_server.exe")

set_property(TARGET pragma PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")
set_property(TARGET pragma_server PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")

if(WIN32)
	set(SHARED_LIB_EXT ".dll")
else()
	set(SHARED_LIB_EXT ".so")
endif()
macro(get_full_file_name OUT_NAME FILE_NAME FILE_TYPE)
	set(TMP_FILE_NAME ${FILE_NAME})
	if(WIN32)
		set(FILE_EXT ${SHARED_LIB_EXT})
	else()
		if(NOT ${FILE_TYPE} STREQUAL "EXECUTABLE")
			set(TMP_FILE_NAME lib${TMP_FILE_NAME})
		endif()
		set(FILE_EXT ${SHARED_LIB_EXT})
	endif()
	if(${FILE_TYPE} STREQUAL "EXECUTABLE")
		if(WIN32)
			set(FILE_EXT ".exe")
		else()
			set(FILE_EXT "")
		endif()
	elseif(${FILE_TYPE} STREQUAL "DYNAMIC")

	else()
		set(FILE_EXT "")
	endif()

	set(${OUT_NAME} ${TMP_FILE_NAME}${FILE_EXT})
endmacro()

function(copy_output_file_abs SRC_PATH DST_PATH FILE_NAME)
	set(optional_arg "DYNAMIC")
	set(extra_macro_args ${ARGN})
	list(LENGTH extra_macro_args num_extra_args)
	if(${num_extra_args} GREATER 0)
		list(GET extra_macro_args 0 optional_arg)
	endif()
	get_full_file_name(FILE_NAME "${FILE_NAME}" ${optional_arg})

	get_filename_component(SRC_PATH "${SRC_PATH}" DIRECTORY)
	set(SRC_PATH ${SRC_PATH}/${FILE_NAME})
	if(EXISTS "${SRC_PATH}")
		configure_file("${SRC_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/output/${DST_PATH}/${FILE_NAME}" COPYONLY)
	endif()
endfunction(copy_output_file_abs)

function(copy_output_file SRC_PATH DST_PATH FILE_NAME)
	set(optional_arg "DYNAMIC")
	set(extra_macro_args ${ARGN})
	list(LENGTH extra_macro_args num_extra_args)
	if(${num_extra_args} GREATER 0)
		list(GET extra_macro_args 0 optional_arg)
	endif()
	copy_output_file_abs("${CMAKE_CURRENT_BINARY_DIR}/${SRC_PATH}/${FILE_NAME}" "${DST_PATH}" "${FILE_NAME}" "${optional_arg}")
endfunction(copy_output_file)

function(copy_output_file_ext SRC_PATH DST_PATH)
	get_filename_component(SRC_FILE_NAME "${SRC_PATH}" NAME_WE)
	get_filename_component(SRC_FILE_PATH "${SRC_PATH}" DIRECTORY)

	set(LIB_NAME ${SRC_FILE_NAME})
	set(optional_arg "DYNAMIC")
	set(extra_macro_args ${ARGN})
	list(LENGTH extra_macro_args num_extra_args)
	if(${num_extra_args} GREATER 0)
		list(GET extra_macro_args 0 optional_arg)
	endif()
	get_full_file_name("LIB_NAME" "${LIB_NAME}" "${optional_arg}")
	if(EXISTS "${SRC_FILE_PATH}/${LIB_NAME}" AND NOT "${SRC_FILE_PATH}/${LIB_NAME}" STREQUAL "/")
		configure_file("${SRC_FILE_PATH}/${LIB_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/output/${DST_PATH}/${LIB_NAME}" COPYONLY)
	endif()
endfunction(copy_output_file_ext)

function(copy_output_file_extc SRC_PATH DST_PATH DST_FILE_NAME)
	get_filename_component(SRC_FILE_NAME "${SRC_PATH}" NAME_WE)
	get_filename_component(SRC_FILE_PATH "${SRC_PATH}" DIRECTORY)

	set(optional_arg "DYNAMIC")
	set(extra_macro_args ${ARGN})
	list(LENGTH extra_macro_args num_extra_args)
	if(${num_extra_args} GREATER 0)
		list(GET extra_macro_args 0 optional_arg)
	endif()
	get_full_file_name(DST_FILE_NAME ${DST_FILE_NAME} ${optional_arg})
	if(EXISTS "${SRC_FILE_PATH}/${DST_FILE_NAME}" AND NOT "${SRC_FILE_PATH}/${DST_FILE_NAME}" STREQUAL "/")
		configure_file("${SRC_FILE_PATH}/${DST_FILE_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/output/${DST_PATH}/${DST_FILE_NAME}" COPYONLY)
	endif()
endfunction(copy_output_file_extc)

if(WIN32)
	set(BINARY_OUTPUT_DIR "bin")
else()
	set(BINARY_OUTPUT_DIR "lib")
endif()
copy_output_file(core/pragma/${BINARY_PRAGMA_DIR} "" pragma EXECUTABLE)
copy_output_file(core/pragma_server/${BINARY_PRAGMA_DIR} "" pragma_server EXECUTABLE)
if(UNIX)
	# copy_output_file_ext("${DEPENDENCY_LUAJIT_LIBRARY}" ${BINARY_OUTPUT_DIR})
endif()

copy_output_file(core/client/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} client)
copy_output_file(core/server/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} server)
copy_output_file(core/engine/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} engine)
copy_output_file(core/cengine/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} cengine)
copy_output_file(core/shared/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} shared)
copy_output_file(external_libs/alsoundsystem/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} alsoundsystem)
copy_output_file(external_libs/datasystem/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} datasystem)
copy_output_file(external_libs/iglfw/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} iglfw)
copy_output_file(external_libs/luasystem/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} luasystem)
copy_output_file(external_libs/materialsystem/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} materialsystem)
copy_output_file(external_libs/materialsystem/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} cmaterialsystem)
copy_output_file(external_libs/mathutil/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} mathutil)
copy_output_file(external_libs/networkmanager/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} clientmanager)
copy_output_file(external_libs/networkmanager/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} servermanager)
copy_output_file(external_libs/prosper/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} prosper)
copy_output_file(external_libs/sharedutils/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} sharedutils)
copy_output_file(external_libs/vfilesystem/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} vfilesystem)
copy_output_file(external_libs/wgui/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} wgui)
copy_output_file(modules/interfaces/ishared/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} ishared)
copy_output_file(modules/interfaces/iserver/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} iserver)
copy_output_file(modules/interfaces/iclient/${BINARY_PRAGMA_DIR} ${BINARY_OUTPUT_DIR} iclient)

copy_output_file(external_libs/util_pcl/${BINARY_PRAGMA_DIR} "modules/pcl" util_pcl)
copy_output_file(external_libs/util_archive/${BINARY_PRAGMA_DIR} "modules/mount_external" util_archive)

copy_output_file(modules/pr_source/${BINARY_PRAGMA_DIR} "modules/mount_external" pr_mount_external)
copy_output_file(modules/pr_dmx/${BINARY_PRAGMA_DIR} "modules" pr_dmx)
copy_output_file(modules/pr_dds/${BINARY_PRAGMA_DIR} "modules" pr_dds)
copy_output_file(modules/pr_steamworks/${BINARY_PRAGMA_DIR} "modules/steamworks" pr_steamworks)
copy_output_file(modules/pr_pcl/${BINARY_PRAGMA_DIR} "modules/pcl" pr_pcl)

copy_output_file_ext("${DEPENDENCY_BOOST_SYSTEM_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_BOOST_DATE_TIME_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_LUA_LIBRARY}" ${BINARY_OUTPUT_DIR})

search_lib_recursive(DEPENDENCY_LUABIND_BINARY bin/${BINARY_PRAGMA_DIR} luabind)
copy_output_file_ext("${DEPENDENCY_LUABIND_BINARY}" ${BINARY_OUTPUT_DIR})

copy_output_file_ext("${DEPENDENCY_LIBVORBIS_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_LIBOGG_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_LIBVORBISFILE_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_LPNG_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_LIBZIP_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_LIBNOISE_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_BZIP2_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_ASSIMP_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext("${DEPENDENCY_FREETYPE_LIBRARY}" ${BINARY_OUTPUT_DIR})
copy_output_file_ext(${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib/${BINARY_PRAGMA_DIR}/zlib1${SHARED_LIB_EXT} ${BINARY_OUTPUT_DIR})

copy_output_file_extc("${DEPENDENCY_FMOD_LIBRARY}" ${BINARY_OUTPUT_DIR} "fmod64")
copy_output_file_extc("${DEPENDENCY_FMOD_STUDIO_LIBRARY}" ${BINARY_OUTPUT_DIR} "fmodstudio64")
copy_output_file_extc("${DEPENDENCY_GLFW_LIBRARY}" ${BINARY_OUTPUT_DIR} "glfw3")

get_filename_component(STEAM_AUDIO_SRC_PATH "${DEPENDENCY_STEAM_AUDIO_LIBRARY}" DIRECTORY)
copy_output_file_abs("${STEAM_AUDIO_SRC_PATH}/../../../bin/Windows/x64/phonon" ${BINARY_OUTPUT_DIR} "phonon")

get_filename_component(STEAMWORKS_DLL_DIRECTORY "${DEPENDENCY_STEAMWORKS_LIBRARY}" DIRECTORY)
get_filename_component(STEAMWORKS_DLL_NAME "${DEPENDENCY_STEAMWORKS_LIBRARY}" NAME_WE)
copy_output_file_abs("${STEAMWORKS_DLL_DIRECTORY}/${STEAMWORKS_DLL_NAME}" "modules/steamworks" "${STEAMWORKS_DLL_NAME}")

set(PCL_BINARY_DIRECTORY "" CACHE PATH "Path to PCL binary files.")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_common_release" "modules/pcl" "pcl_common_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_features_release" "modules/pcl" "pcl_features_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_filters_release" "modules/pcl" "pcl_filters_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_io_ply_release" "modules/pcl" "pcl_io_ply_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_io_release" "modules/pcl" "pcl_io_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_kdtree_release" "modules/pcl" "pcl_kdtree_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_octree_release" "modules/pcl" "pcl_octree_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_sample_consensus_release" "modules/pcl" "pcl_sample_consensus_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_search_release" "modules/pcl" "pcl_search_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_segmentation_release" "modules/pcl" "pcl_segmentation_release")
copy_output_file_abs("${PCL_BINARY_DIRECTORY}/pcl_surface_release" "modules/pcl" "pcl_surface_release")

# zip.dll
# libsamplerate-0.dll
