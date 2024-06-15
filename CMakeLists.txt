cmake_minimum_required(VERSION 3.28)

include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/pr_legacy.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/pr_install.cmake)

project(pragma)

# Global settings
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4715")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we4715")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=return-type")
    set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -Og -g")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(WIN32)
    set(BINARY_OUTPUT_DIR "bin")
    set(EXECUTABLE_OUTPUT_DIR "bin")

    set(OS_LIB_BIN_EXT ".dll")
    set(OS_LIB_BIN_PRE "")
    set(OS_LIB_SHARED_EXT ".lib")
    set(OS_EXE_EXT ".exe")
else()
    set(BINARY_OUTPUT_DIR "lib")
    set(EXECUTABLE_OUTPUT_DIR "lib")

    set(OS_LIB_BIN_EXT ".so")
    set(OS_LIB_BIN_PRE "lib")
    set(OS_LIB_SHARED_EXT ".so")
    set(OS_EXE_EXT "")
endif()

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
    set(CONFIG_BUILD_TYPE
        "$<CONFIG>"
        CACHE STRING "Build Type" FORCE)

    set(CMAKE_BUILD_TYPE "") # ignored in multi-config scenarios but not in  anvil's ChooseMSVCCRT
else()
    set(CONFIG_BUILD_TYPE
        "RelWithDebInfo"
        CACHE STRING "Build Type")
    set(CMAKE_BUILD_TYPE "${CONFIG_BUILD_TYPE}")
endif()

set(CMAKE_CONFIGURATION_TYPES
    "RelWithDebInfo;Release;Debug;"
    CACHE STRING "Supported configuration options" FORCE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(is_multi_config)
    set(BINARY_PRAGMA_DIR
        ${CONFIG_BUILD_TYPE}
        CACHE INTERNAL "" FORCE)
else()
    set(BINARY_PRAGMA_DIR
        "."
        CACHE INTERNAL "" FORCE)
endif()

set(DEPENDENCY_PRAGMA_EXECUTABLE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/core/pragma/include
    CACHE PATH "Path to include directory.")

unset(CONFIG_ENABLE_EXECUTABLE_FLAG CACHE)
set(CONFIG_ENABLE_EXECUTABLE_FLAG
    OFF
    CACHE BOOL "")

if(WIN32)
    set(ZLIB_LIB_NAME
        "zlib"
        CACHE INTERNAL "" FORCE)
else()
    set(ZLIB_LIB_NAME
        "z"
        CACHE INTERNAL "" FORCE)
endif()
search_lib_recursive(DEPENDENCY_ZLIB_LIBRARY "third_party_libs/zlib/${BINARY_PRAGMA_DIR}" ${ZLIB_LIB_NAME})

pr_set_include_path(zlip_build "${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib")

search_lib_recursive(DEPENDENCY_NVIDIA_TEXTURE_TOOLS_BC6H_LIBRARY external_libs/util_image/third_party_libs/nvtt/src/bc6h/${BINARY_PRAGMA_DIR} bc6h STATIC)
search_lib_recursive(DEPENDENCY_NVIDIA_TEXTURE_TOOLS_BC7_LIBRARY external_libs/util_image/third_party_libs/nvtt/src/bc7/${BINARY_PRAGMA_DIR} bc7 STATIC)
search_lib_recursive(DEPENDENCY_NVIDIA_TEXTURE_TOOLS_CORE_LIBRARY external_libs/util_image/third_party_libs/nvtt/src/nvcore/${BINARY_PRAGMA_DIR} nvcore STATIC)
search_lib_recursive(DEPENDENCY_NVIDIA_TEXTURE_TOOLS_IMAGE_LIBRARY external_libs/util_image/third_party_libs/nvtt/src/nvimage/${BINARY_PRAGMA_DIR} nvimage
                     STATIC)
search_lib_recursive(DEPENDENCY_NVIDIA_TEXTURE_TOOLS_LIBRARY external_libs/util_image/third_party_libs/nvtt/src/nvtt/${BINARY_PRAGMA_DIR} nvtt STATIC)
search_lib_recursive(DEPENDENCY_NVIDIA_TEXTURE_TOOLS_MATH_LIBRARY external_libs/util_image/third_party_libs/nvtt/src/nvmath/${BINARY_PRAGMA_DIR} nvmath STATIC)
search_lib_recursive(DEPENDENCY_NVIDIA_TEXTURE_TOOLS_THREAD_LIBRARY external_libs/util_image/third_party_libs/nvtt/src/nvthread/${BINARY_PRAGMA_DIR} nvthread
                     STATIC)
search_lib_recursive(DEPENDENCY_NVIDIA_TEXTURE_TOOLS_SQUISH_LIBRARY external_libs/util_image/third_party_libs/nvtt/src/nvtt/squish/${BINARY_PRAGMA_DIR}
                     nvsquish STATIC)
search_lib_recursive(DEPENDENCY_SQUISH_LIBRARY external_libs/util_image/third_party_libs/nvtt/extern/libsquish-1.15/${BINARY_PRAGMA_DIR} squish STATIC)

unset(WGUI_INCLUDE_DEPENDENCIES CACHE)
set(WGUI_INCLUDE_DEPENDENCIES
    OFF
    CACHE BOOL "Include dependencies required to build the wgui library?")

pr_set_include_path(libvorbis "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/vorbis/include")
search_lib_recursive(DEPENDENCY_LIBVORBIS_LIBRARY third_party_libs/vorbis/lib/${BINARY_PRAGMA_DIR} vorbis)
search_lib_recursive(DEPENDENCY_LIBVORBISFILE_LIBRARY third_party_libs/vorbis/lib/${BINARY_PRAGMA_DIR} vorbisfile)

search_lib_recursive(DEPENDENCY_LIBOGG_LIBRARY third_party_libs/ogg/${BINARY_PRAGMA_DIR} ogg)
pr_set_include_path(libogg_build "${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/ogg/include")
set(OGG_LIBRARIES
    ${DEPENDENCY_LIBOGG_LIBRARY}
    CACHE STRING "" FORCE)

pr_set_include_path(lz4 "${CMAKE_CURRENT_LIST_DIR}/external_libs/util_source2/third_party_libs/lz4/lib" FORCE)
search_lib_recursive(DEPENDENCY_LZ4_LIBRARY external_libs/util_source2/third_party_libs/lz4/${BINARY_PRAGMA_DIR} lz4 STATIC)

pr_set_include_path(liblzma "${CMAKE_CURRENT_LIST_DIR}/external_libs/util_bsp/third_party_libs/liblzma/include" FORCE)
search_lib_recursive(DEPENDENCY_LIBLZMA_LIBRARY external_libs/util_bsp/third_party_libs/liblzma/${BINARY_PRAGMA_DIR} lzma STATIC)

if(UNIX)
    find_package(Freetype REQUIRED)
    pr_set_include_path(freetype "${FREETYPE_INCLUDE_DIR_freetype2}" FORCE)
    set(DEPENDENCY_FREETYPE_LIBRARY
        ${FREETYPE_LIBRARIES}
        CACHE STRING "" FORCE)
endif()

pr_set_include_path(luajit "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/luajit/src")
pr_set_include_path(lua "${DEPENDENCY_LUAJIT_INCLUDE}")

# Third-party libraries
pr_set_include_path(gli "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/gli")

pr_set_include_path(clip2tri "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/clip2tri")
search_lib_recursive(DEPENDENCY_CLIP2TRI_LIBRARY third_party_libs/clip2tri/${BINARY_PRAGMA_DIR} clip2tri-static STATIC)
search_lib_recursive(DEPENDENCY_CLIPPER_LIBRARY third_party_libs/clip2tri/clipper/${BINARY_PRAGMA_DIR} clipper-static STATIC)
search_lib_recursive(DEPENDENCY_POLY2TRI_LIBRARY third_party_libs/clip2tri/poly2tri/${BINARY_PRAGMA_DIR} poly2tri-static STATIC)

pr_set_include_path(rapidxml "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/rapidxml")
pr_set_include_path(ba2 "${CMAKE_CURRENT_LIST_DIR}/modules/pr_source/third_party/ba2lib/BA2Lib")

pr_set_include_path(exprtk "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/exprtk")

pr_set_include_path(lpng "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libpng")
pr_set_include_path(lpng_build "${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/libpng")
if(WIN32)
    set(LIBPNG_LIB_NAME
        "libpng16$<$<CONFIG:Debug>:d>"
        CACHE INTERNAL "" FORCE)
else()
    set(LIBPNG_LIB_NAME
        "png16$<$<CONFIG:Debug>:d>"
        CACHE INTERNAL "" FORCE)
endif()
search_lib_recursive(DEPENDENCY_LPNG_LIBRARY third_party_libs/libpng/${BINARY_PRAGMA_DIR} ${LIBPNG_LIB_NAME})

pr_set_include_path(miniball "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/miniball/cpp/main")
pr_set_include_path(bvh "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/bvh/src")

if(WIN32)
    if("${DEPENDENCY_LUAJIT_LIBRARY}" STREQUAL "")
        search_lib_recursive(DEPENDENCY_LUAJIT_LIBRARY third_party_libs/luajit/src luajit STATIC ${CMAKE_CURRENT_LIST_DIR})
    endif()
    if("${DEPENDENCY_LUA_LIBRARY}" STREQUAL "")
        search_lib_recursive(DEPENDENCY_LUA_LIBRARY third_party_libs/luajit/src luajit STATIC ${CMAKE_CURRENT_LIST_DIR})
    endif()
else()
    if("${DEPENDENCY_LUAJIT_LIBRARY}" STREQUAL "")
        search_lib_recursive(DEPENDENCY_LUAJIT_LIBRARY third_party_libs/luajit/src luajit-p DYNAMIC ${CMAKE_CURRENT_LIST_DIR})
    endif()
    if("${DEPENDENCY_LUA_LIBRARY}" STREQUAL "")
        search_lib_recursive(DEPENDENCY_LUA_LIBRARY third_party_libs/luajit/src luajit-p DYNAMIC ${CMAKE_CURRENT_LIST_DIR})
    endif()
endif()

pr_set_include_path(libnoise "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libnoise/src")
pr_set_include_path(libnoiseutils "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libnoise/noiseutils")
search_lib_recursive(DEPENDENCY_LIBNOISE_LIBRARY third_party_libs/libnoise/src/${BINARY_PRAGMA_DIR} noise STATIC)
search_lib_recursive(DEPENDENCY_LIBNOISEUTILS_LIBRARY third_party_libs/libnoise/noiseutils/${BINARY_PRAGMA_DIR} noiseutils-static STATIC)

pr_set_include_path(eigen "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/eigen")
pr_set_include_path(tinygltf "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/tinygltf")

if(UNIX)
    pr_set_library_path(boost_chrono "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_chrono.a" FORCE)
    pr_set_library_path(boost_date_time "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_date_time.a" FORCE)
    pr_set_library_path(boost_filesystem "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_filesystem.a" FORCE)
    pr_set_library_path(boost_iostreams "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_iostreams.a" FORCE)
    pr_set_library_path(boost_regex "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_regex.a" FORCE)
    pr_set_library_path(boost_system "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_system.a" FORCE)
    pr_set_library_path(boost_thread "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_thread.a" FORCE)

    # Resolve links
    resolve_links(
        bzip2
        clip2tri
        clipper
        curl
        detour
        fbxsdk
        flann
        glfw
        glslang
        glslang_ogl_compiler
        glslang_os_dependent
        harfbuzz
        libnoiseutils
        libnoise
        libogg
        libsamplerate
        libvorbisfile
        libvorbis
        libzip
        lpng
        luabind_binary
        luabind
        luajit
        lua
        niftools
        openvr
        ovr
        poly2tri
        recast
        rectanglebinpack
        sharedutils_static
        steamworks
        steam_audio
        vtflib
        zlib)
    if(win32)
        resolve_links(freetype)
    endif()
endif()
set(BOOST_INCLUDEDIR
    ${DEPENDENCY_BOOST_INCLUDE}
    CACHE PATH "Path to include directory." FORCE)
set(BOOST_ROOT
    ${DEPENDENCY_BOOST_INCLUDE}
    CACHE PATH "Path to include directory." FORCE)

set(OGG_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/ogg/include
    CACHE STRING "" FORCE)
set(DEPENDENCY_LIBOGG_INCLUDE
    ${OGG_INCLUDE_DIRS}
    CACHE STRING "" FORCE)

set(ZLIB_INCLUDE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/zlib
    CACHE STRING "" FORCE)
pr_set_include_path(zlib "${ZLIB_INCLUDE_DIR}" FORCE)

if(is_multi_config)
    set(PR_MODULE_INSTALL_SUBDIR "${CONFIG_BUILD_TYPE}")
else()
    set(PR_MODULE_INSTALL_SUBDIR ".")
endif()

message("Processing external libraries...")
add_subdirectory(external_libs)
set_target_folder(wgui external_libs/wgui)
set_target_folder(sample_application external_libs/wgui)
target_include_directories(util_image PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
target_include_directories(cmaterialsystem PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)

message("Processing core libraries...")
add_subdirectory(core)

message("Processing modules...")
add_subdirectory(modules)

message("Processing tools...")
add_subdirectory(tools)

message("Processing third-party libaries...")
target_include_directories(libcurl_shared PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)

set(LIBRARY_OUTPUT_PATH
    ""
    CACHE PATH "" FORCE)

set(PRAGMA_INSTALL_COMPONENT "pragma")
set(PRAGMA_INSTALL_COMPONENT_FULL "pragma-full")
add_custom_target(pragma-install COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=${PRAGMA_INSTALL_COMPONENT} -DBUILD_TYPE=${CONFIG_BUILD_TYPE} -P
                                         "${CMAKE_BINARY_DIR}/cmake_install.cmake")
add_custom_target(pragma-install-full COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=${PRAGMA_INSTALL_COMPONENT_FULL} -DBUILD_TYPE=${CONFIG_BUILD_TYPE}
                                              -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
# Luabind
set(LUA_INCLUDE_DIR ${DEPENDENCY_LUAJIT_INCLUDE} CACHE STRING "" FORCE)
set(LUA_LIBRARY ${DEPENDENCY_LUA_LIBRARY} CACHE STRING "" FORCE)
set(LUA_FOUND ON CACHE BOOL ON FORCE)
set(LUA_LIBRARIES ${LUA_LIBRARY} CACHE STRING "" FORCE)
pr_include_third_party_library(luabind INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/luabind" LIB "lib/${BINARY_PRAGMA_DIR}")
set_target_folder(luabind third_party_libs)
target_include_directories(luabind PUBLIC ${LUA_INCLUDE_DIR})
add_dependencies(luasystem luabind)
#

# mpaheader
pr_include_third_party_library(mpaheader)
set_target_folder(mpaheader third_party_libs)
add_dependencies(util_sound mpaheader)
#

# VKVParser
pr_include_third_party_library(
    vkv_parser
    TARGET
    KVParser
    DIR
    vkvparser
    INC
    "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/vkvparser/include"
    LIB
    "third_party_libs/vkvparser/${BINARY_PRAGMA_DIR}")
add_dependencies(materialsystem KVParser)
#

# spdlog
set(SPDLOG_BUILD_SHARED
    ON
    CACHE BOOL ON FORCE)
set(SPDLOG_BUILD_PIC
    ON
    CACHE BOOL ON FORCE)
if(WIN32)
    set(SPDLOG_USE_STD_FORMAT
        ON
        CACHE BOOL ON FORCE)
else()
    set(SPDLOG_USE_STD_FORMAT
        OFF
        CACHE BOOL OFF FORCE)
endif()
set(SPDLOG_CLOCK_COARSE
    ON
    CACHE BOOL ON FORCE)
set(CMAKE_CXX_STANDARD 20) # bump to c++23 after we change scope ops in luabind.
pr_include_third_party_library(spdlog LIB_NAME spdlog$<$<CONFIG:Debug>:d>)
set_target_folder(spdlog third_party_libs)
unset(CMAKE_CXX_STANDARD)
add_dependencies(shared spdlog)
#

# fmt
unset(BUILD_SHARED_LIBS CACHE)
set(FMT_TEST
    OFF
    CACHE BOOL OFF FORCE)
set(FMT_DOC
    OFF
    CACHE BOOL OFF FORCE)
pr_include_third_party_library(fmt LIB_NAME fmt$<$<CONFIG:Debug>:d> STATIC)
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ".." FORCE)
#

# 7zip cpp
if(WIN32)
    set(BUILD_SHARED_LIBS
        OFF
        CACHE BOOL OFF FORCE)
    pr_include_third_party_library(7zpp)
    set(BUILD_SHARED_LIBS
        ON
        CACHE BOOL ".." FORCE)
endif()

# muparserx
set(BUILD_EXAMPLES
    OFF
    CACHE BOOL OFF FORCE)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL OFF FORCE)
pr_include_third_party_library(muparserx INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/muparserx/parser" STATIC)
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ".." FORCE)
set_target_folder(muparserx third_party_libs/muparserx)
#

# clip2tri
if(WIN32)
    set(M_LIBRARY "" FORCE)
else()
    # BUG: CMakeLists parser strips quotes making this effectively set(M_LIBRARY FORCE)
    unset(M_LIBRARY)
endif()
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL OFF FORCE)
set(BUILD_STATIC_LIBS
    ON
    CACHE BOOL ON FORCE)
register_third_party_library(clip2tri)
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ".." FORCE)
set_target_folder(clip2tri third_party_libs/clip2tri)
set_target_folder(clip2tri-static third_party_libs/clip2tri)
set_target_folder(clipper third_party_libs/clip2tri)
set_target_folder(clipper-static third_party_libs/clip2tri)
set_target_folder(poly2tri third_party_libs/clip2tri)
set_target_folder(poly2tri-static third_party_libs/clip2tri)
#
# BUG: M_LIBRARY is not restored to previous value breaking anything using it.

# eigen register_third_party_library(eigen)
#

# glfw
set(GLFW_BUILD_DOCS
    OFF
    CACHE BOOL OFF FORCE)
set(GLFW_BUILD_EXAMPLES
    OFF
    CACHE BOOL OFF FORCE)
set(GLFW_BUILD_TESTS
    OFF
    CACHE BOOL OFF FORCE)
set(GLFW_INSTALL
    OFF
    CACHE BOOL OFF FORCE)
if(WIN32)
    set(GLFW_LIB_NAME
        "glfw3dll"
        CACHE INTERNAL "" FORCE)
else()
    set(GLFW_LIB_NAME
        "glfw"
        CACHE INTERNAL "" FORCE)
endif()
pr_include_third_party_library(glfw LIB "third_party_libs/glfw/src/${BINARY_PRAGMA_DIR}" LIB_NAME "${GLFW_LIB_NAME}")
set_target_folder(glfw third_party_libs/glfw)
add_dependencies(iglfw glfw)
#

# libogg
register_third_party_library(ogg)
set_target_folder(ogg third_party_libs)
#

# libvorbis
register_third_party_library(vorbis)
set_target_folder(vorbis third_party_libs/vorbis)
set_target_folder(vorbisenc third_party_libs/vorbis)
set_target_folder(vorbisfile third_party_libs/vorbis)
target_include_directories(vorbis PUBLIC "${DEPENDENCY_LIBOGG_BUILD_INCLUDE}")
#

# zlib
register_third_party_library(zlib)
target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
target_include_directories(zlibstatic PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
set_target_folder(example third_party_libs/zlib)
set_target_folder(minigzip third_party_libs/zlib)
set_target_folder(zlib third_party_libs/zlib)
set_target_folder(zlibstatic third_party_libs/zlib)

target_include_directories(materialsystem PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)

# if(UNIX) intervene here beacuse libzip uses pkgconfig
set(ZLIB_INCLUDE_DIR
    ${DEPENDENCY_ZLIB_INCLUDE}
    CACHE STRING "" FORCE)
set(ZLIB_LIBRARY
    "$<TARGET_LINKER_FILE:zlib>"
    CACHE STRING "" FORCE)
add_library(ZLIB::ZLIB ALIAS zlib) # for libzip just ahead.
set(ZLIB_LINK_LIBRARY_NAME "ZLIB::ZLIB")
# endif()
#

# libzip
set(LIBZIP_DO_INSTALL
    OFF
    CACHE BOOL ".." FORCE)
set(ENABLE_BZIP2
    OFF
    CACHE BOOL ".." FORCE)
set(ENABLE_LZMA
    OFF
    CACHE BOOL ".." FORCE)
pr_include_third_party_library(libzip TARGET zip INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libzip/lib" LIB "third_party_libs/libzip/lib/${BINARY_PRAGMA_DIR}")
pr_set_include_path(libzip_conf "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libzip/xcode")
target_include_directories(zip PUBLIC ${DEPENDENCY_ZLIB_INCLUDE}) # ew.
set_target_folder(add_from_filep third_party_libs/libzip)
set_target_folder(can_clone_file third_party_libs/libzip)
set_target_folder(check third_party_libs/libzip)
set_target_folder(cleanup third_party_libs/libzip)
set_target_folder(dist third_party_libs/libzip)
set_target_folder(distcheck third_party_libs/libzip)
set_target_folder(fopen_unchanged third_party_libs/libzip)
set_target_folder(fread third_party_libs/libzip)
set_target_folder(fseek third_party_libs/libzip)
set_target_folder(hole third_party_libs/libzip)
# set_target_folder(in-memory third_party_libs/libzip)
set_target_folder(man third_party_libs/libzip)
set_target_folder(nonrandomopen third_party_libs/libzip)
set_target_folder(nonrandomopentest third_party_libs/libzip)
set_target_folder(testinput third_party_libs/libzip)
set_target_folder(tryopen third_party_libs/libzip)
# set_target_folder(update_zip_err_str third_party_libs/libzip)
set_target_folder(update_zip_errors third_party_libs/libzip)
set_target_folder(update-html third_party_libs/libzip)
set_target_folder(update-man third_party_libs/libzip)
set_target_folder(zip third_party_libs/libzip)
set_target_folder(zipcmp third_party_libs/libzip)
set_target_folder(zipmerge third_party_libs/libzip)
set_target_folder(ziptool third_party_libs/libzip)
set_target_folder(ziptool_regress third_party_libs/libzip)
#

# bzip2
pr_include_third_party_library(bzip2 INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/bzip2" LIB_NAME "bz2$<$<CONFIG:Debug>:d>")
set_target_folder(bzip2 third_party_libs/bzip2)
set_target_folder(bzip2recover third_party_libs/bzip2)
set_target_folder(libz2 third_party_libs/bzip2)
#

# libnoise
set(BUILD_LIBNOISE_EXAMPLES
    OFF
    CACHE BOOL ".." FORCE)
set(BUILD_LIBNOISE_UTILS
    ON
    CACHE BOOL ".." FORCE)
unset(BUILD_SHARED_LIBS CACHE)
register_third_party_library(libnoise)
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ".." FORCE)
set_target_folder(noise third_party_libs/libnoise)
set_target_folder(noise-static third_party_libs/libnoise)
set_target_folder(noiseutils third_party_libs/libnoise)
set_target_folder(noiseutils-static third_party_libs/libnoise)

target_include_directories(noiseutils-static PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libnoise/src)
#

# libpng
register_third_party_library(libpng)
set_target_folder(genfiles third_party_libs/libpng)
set_target_folder(png third_party_libs/libpng)
set_target_folder(png_static third_party_libs/libpng)
set_target_folder(pngfix third_party_libs/libpng)
set_target_folder(png-fix-itxt third_party_libs/libpng)
set_target_folder(pngimage third_party_libs/libpng)
set_target_folder(pngstest third_party_libs/libpng)
set_target_folder(pngtest third_party_libs/libpng)
set_target_folder(pngunknown third_party_libs/libpng)
set_target_folder(pngvalid third_party_libs/libpng)

target_include_directories(png PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
add_dependencies(util_image png)
#

# libsamplerate (required for alsoundsystem with fmod enabled)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL OFF FORCE)
pr_include_third_party_library(libsamplerate INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/libsamplerate/src" LIB_NAME "samplerate" STATIC)
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ON FORCE)
set_target_folder(callback_hang_test third_party_libs/libsamplerate)
set_target_folder(callback_test third_party_libs/libsamplerate)
set_target_folder(clone_test third_party_libs/libsamplerate)
set_target_folder(downsample_test third_party_libs/libsamplerate)
set_target_folder(float_short_test third_party_libs/libsamplerate)
set_target_folder(misc_test third_party_libs/libsamplerate)
set_target_folder(multi_channel_test third_party_libs/libsamplerate)
set_target_folder(multichan_throughput_test third_party_libs/libsamplerate)
set_target_folder(nullptr_test third_party_libs/libsamplerate)
set_target_folder(reset_test third_party_libs/libsamplerate)
set_target_folder(samplerate third_party_libs/libsamplerate)
set_target_folder(simple_test third_party_libs/libsamplerate)
set_target_folder(snr_bw_test third_party_libs/libsamplerate)
set_target_folder(streaming_test third_party_libs/libsamplerate)
set_target_folder(termination_test third_party_libs/libsamplerate)
set_target_folder(throughput_test third_party_libs/libsamplerate)
set_target_folder(timewarp-file third_party_libs/libsamplerate)
set_target_folder(varispeed_test third_party_libs/libsamplerate)
set_target_folder(varispeed-play third_party_libs/libsamplerate)
#

# vtflib
set(USE_LIBTXC_DXTN
    OFF
    CACHE BOOL OFF FORCE)
pr_set_include_path(vtflib_build "${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/vtflib/src")
pr_include_third_party_library(vtflib INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/vtflib/src" LIB "third_party_libs/vtflib/src/${BINARY_PRAGMA_DIR}" LIB_NAME "VTFLib13")
set_target_folder(VTFLib13 third_party_libs)
target_include_directories(VTFLib13 PUBLIC ${DEPENDENCY_LIBZIP_CONF_INCLUDE})
add_dependencies(materialsystem VTFLib13)
#

# recastnavigation
set(RECASTNAVIGATION_DEMO
    OFF
    CACHE BOOL OFF FORCE)
set(RECASTNAVIGATION_EXAMPLES
    OFF
    CACHE BOOL OFF FORCE)
set(RECASTNAVIGATION_STATIC
    ON
    CACHE BOOL ON FORCE)
set(RECASTNAVIGATION_TESTS
    OFF
    CACHE BOOL OFF FORCE)
pr_include_third_party_library(recastnavigation TARGET Recast INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/recastnavigation/Recast/Include" LIB "third_party_libs/recastnavigation/Recast/${BINARY_PRAGMA_DIR}" STATIC)
pr_include_third_party_library(TARGET Detour INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/recastnavigation/Detour/Include" LIB "third_party_libs/recastnavigation/Detour/${BINARY_PRAGMA_DIR}" STATIC)
set_target_folder(DebugUtils third_party_libs/recast)
set_target_folder(Detour third_party_libs/recast)
set_target_folder(DetourCrowd third_party_libs/recast)
set_target_folder(DetourTileCache third_party_libs/recast)
set_target_folder(Recast third_party_libs/recast)
#

# rectangle_bin_pack
pr_include_third_party_library(rectangle_bin_pack_cmake TARGET rectangle_bin_pack INC "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/rectangle_bin_pack" STATIC)
set_target_folder(rectangle_bin_pack third_party_libs)

if(WIN32)
    add_library(freetype SHARED IMPORTED)
    set_property(TARGET freetype PROPERTY IMPORTED_IMPLIB "${DEPENDENCY_FREETYPE_LIBRARY}") # pragma_install_lib should pick up the dll file to install, since I
                                                                                            # did not install a target here

    target_include_directories(freetype INTERFACE ${DEPENDENCY_FREETYPE_INCLUDE})
endif()
# in linux the check is done in wgui

if(WIN32)
    set_target_properties(freetype PROPERTIES FOLDER third_party_libs)
endif()
# target_include_directories(freetype PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
#

# dependencies

add_dependencies(png zlib)
add_dependencies(zip zlib)
add_dependencies(vorbis ogg)
add_dependencies(util_pad zip)

if(WIN32)
    add_dependencies(util_zip 7zpp)
endif()

add_dependencies(
    shared
    muparserx
    libz2
    bzip2
    clip2tri-static
    clipper-static
    poly2tri-static
    rectangle_bin_pack
    Recast
    Detour
    noise-static
    noiseutils-static
    ogg
    vorbis
    vorbisfile
    fmt)

if(WIN32)
    set_target_properties(pragma PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
    set_target_properties(pragma_server PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")

    set_target_properties(pragma PROPERTIES VS_DEBUGGER_COMMAND "${CMAKE_INSTALL_PREFIX}/pragma.exe")
    set_target_properties(pragma_server PROPERTIES VS_DEBUGGER_COMMAND "${CMAKE_INSTALL_PREFIX}/pragma_server.exe")

    set_target_properties(pragma PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext -graphics_api vulkan")
    set_target_properties(pragma_server PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT pragma)

# Installation

pragma_install(pragma ".")
pragma_install(pragma_server ".")
if(WIN32)
    pragma_install(pragma_console ".")
endif()
pragma_install(alsoundsystem "${BINARY_OUTPUT_DIR}")

pragma_install(client "${BINARY_OUTPUT_DIR}")
pragma_install(cmaterialsystem "${BINARY_OUTPUT_DIR}")
pragma_install(datasystem "${BINARY_OUTPUT_DIR}")
# pragma_install(freetype "${BINARY_OUTPUT_DIR}")
pragma_install(glfw "${BINARY_OUTPUT_DIR}")
pragma_install(iclient "${BINARY_OUTPUT_DIR}")
pragma_install(iglfw "${BINARY_OUTPUT_DIR}")
pragma_install(iserver "${BINARY_OUTPUT_DIR}")
pragma_install(ishared "${BINARY_OUTPUT_DIR}")
pragma_install(luabind "${BINARY_OUTPUT_DIR}")
pragma_install(luasystem "${BINARY_OUTPUT_DIR}")
pragma_install(materialsystem "${BINARY_OUTPUT_DIR}")
pragma_install(mathutil "${BINARY_OUTPUT_DIR}")
pragma_install(prosper "${BINARY_OUTPUT_DIR}")
pragma_install(server "${BINARY_OUTPUT_DIR}")
pragma_install(servermanager "${BINARY_OUTPUT_DIR}")
pragma_install(shared "${BINARY_OUTPUT_DIR}")
pragma_install(sharedutils "${BINARY_OUTPUT_DIR}")
pragma_install(util_image "${BINARY_OUTPUT_DIR}")
pragma_install(util_sound "${BINARY_OUTPUT_DIR}")
pragma_install(vfilesystem "${BINARY_OUTPUT_DIR}")
pragma_install(wgui "${BINARY_OUTPUT_DIR}")
pragma_install(util_unicode "${BINARY_OUTPUT_DIR}")

message("Custom install targets: ${PRAGMA_INSTALL_CUSTOM_TARGETS}")
set(PRAGMA_INSTALL_DEPENDENCIES pragma pragma_server iclient iserver udm_convert ${PRAGMA_INSTALL_CUSTOM_TARGETS})
if(WIN32)
    list(APPEND PRAGMA_INSTALL_DEPENDENCIES pragma_console)
endif()
add_dependencies(pragma-install ${PRAGMA_INSTALL_DEPENDENCIES})
add_dependencies(pragma-install-full pragma iclient iserver)

pragma_install_create_directory(modules)
# pragma_install_lib("${DEPENDENCY_ASSIMP_LIBRARY}" "${BINARY_OUTPUT_DIR}")

pragma_install_lib("${DEPENDENCY_BOOST_CHRONO_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_BOOST_DATE_TIME_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_BOOST_FILESYSTEM_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_BOOST_IOSTREAMS_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_BOOST_REGEX_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_BOOST_SYSTEM_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_BOOST_THREAD_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_BZIP2_LIBRARY}" "${BINARY_OUTPUT_DIR}")
# In Linux there is a cyclic deps between freetype,harfbuzz,pango,cairo and most importantly fontconfig. Fontconfig in linux is reposnsible for discovery of
# fonts. (think C:\Windows\Fonts for linux but customizable via configs) recently I hit a snag in which harfbuzz failed to load due to missing pango symbols.
# The AppImage team hit similar snag too, see https://github.com/AppImageCommunity/pkg2appimage/pull/323 and
# https://github.com/probonopd/linuxdeployqt/issues/261 Even if I would include pango and cairo into our pipeline, CEF would complain (coincidentally hitting
# AppImage's snag too), since that DOES use system fonts. (fontconfig has persistent presence in all graphical managers in linux)
if(WIN32)
    pragma_install_lib("${DEPENDENCY_FREETYPE_LIBRARY}" "${BINARY_OUTPUT_DIR}")
endif()
pragma_install_lib("${DEPENDENCY_LUA_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_LUABIND_BINARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_LIBNOISE_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_LIBOGG_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_LIBVORBIS_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_LIBVORBISFILE_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_VTFLIB_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_LIBZIP_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_LPNG_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_SPDLOG_LIBRARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_MPAHEADER_LIBRARY}" "${BINARY_OUTPUT_DIR}")
if(WIN32)
    pragma_install_lib_c("${DEPENDENCY_ZLIB_LIBRARY}" zlib1 "${BINARY_OUTPUT_DIR}")
else()
    pragma_install_lib("${DEPENDENCY_ZLIB_LIBRARY}" "${BINARY_OUTPUT_DIR}")
endif()

function(pragma_install_file FILE_PATH INSTALL_PATH)
    if("${FILE_PATH}" STREQUAL "")
        return()
    endif()
    set(extra_macro_args ${ARGN})
    message("Adding install rule for \"${FILE_PATH}\" to \"${INSTALL_PATH}\"...")
    install(
        FILES "${FILE_PATH}"
        DESTINATION "${INSTALL_PATH}"
        COMPONENT ${PRAGMA_INSTALL_COMPONENT}
        ${extra_macro_args})
endfunction(pragma_install_file)

pragma_install_lib("${DEPENDENCY_ICU_ICUUC_BINARY}" "${BINARY_OUTPUT_DIR}")
pragma_install_lib("${DEPENDENCY_ICU_ICUDT_BINARY}" "${BINARY_OUTPUT_DIR}")

function(pragma_install_program FILE_PATH INSTALL_PATH)
    if("${FILE_PATH}" STREQUAL "")
        return()
    endif()
    set(extra_macro_args ${ARGN})
    message("Adding install rule for \"${FILE_PATH}\" to \"${INSTALL_PATH}\"...")
    install(
        PROGRAMS "${FILE_PATH}"
        DESTINATION "${INSTALL_PATH}"
        COMPONENT ${PRAGMA_INSTALL_COMPONENT}
        ${extra_macro_args})
endfunction(pragma_install_program)

function(pragma_install_directory FILE_PATH INSTALL_PATH)
    if("${FILE_PATH}" STREQUAL "")
        return()
    endif()
    set(extra_macro_args ${ARGN})
    message("Adding install rule for \"${FILE_PATH}\" to \"${INSTALL_PATH}\"...")
    install(
        DIRECTORY "${FILE_PATH}"
        DESTINATION "${INSTALL_PATH}"
        COMPONENT ${PRAGMA_INSTALL_COMPONENT}
        ${extra_macro_args})
endfunction(pragma_install_directory)

# Modules
file(GLOB PR_MODULES "modules/pr_*")
foreach(MODULE_PATH ${PR_MODULES})
    get_filename_component(MODULE_NAME "${MODULE_PATH}" NAME)
    if(WIN32)
        set(PR_MODULE_PATH_WITH_EXT "${CMAKE_CURRENT_BINARY_DIR}/modules/${MODULE_NAME}/${PR_MODULE_INSTALL_SUBDIR}/${MODULE_NAME}.dll")
    else()
        set(PR_MODULE_PATH_WITH_EXT "${CMAKE_CURRENT_BINARY_DIR}/modules/${MODULE_NAME}/${PR_MODULE_INSTALL_SUBDIR}/lib${MODULE_NAME}.so")
    endif()
    if(EXISTS "${MODULE_PATH}/CMakeInstall.txt")
        message("Running install script for module '${MODULE_PATH}'...")
        include("${MODULE_PATH}/CMakeInstall.txt" OPTIONAL)
    else()
        # Default behavior
        message("Adding default installation rule for module \"${MODULE_PATH}\": \"${PR_MODULE_PATH_WITH_EXT}\" to \"${CMAKE_INSTALL_PREFIX}/modules/\"...")
        add_custom_command(
            TARGET pragma-install
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -DInstallSrc=${PR_MODULE_PATH_WITH_EXT} -DInstallDst=\"${CMAKE_INSTALL_PREFIX}/modules/\" -P
                    ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/cmake/install_file.cmake
            COMMENT "Copying ${PR_MODULE_PATH_WITH_EXT} to ${CMAKE_INSTALL_PREFIX}/modules/...")
    endif()
endforeach()

# Tools
file(GLOB PR_MODULES "tools/*")
foreach(MODULE_PATH ${PR_MODULES})
    get_filename_component(MODULE_NAME "${MODULE_PATH}" NAME)
    if(EXISTS "${MODULE_PATH}/CMakeInstall.txt")
        message("Running install script for tool '${MODULE_PATH}'...")
        include("${MODULE_PATH}/CMakeInstall.txt" OPTIONAL)
    endif()
endforeach()

# Install assets
install(
    DIRECTORY assets/
    DESTINATION "."
    COMPONENT ${PRAGMA_INSTALL_COMPONENT_FULL})

if(UNIX)
    add_custom_command(
        TARGET pragma-install
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DINSTALL_LOCATION=${CMAKE_INSTALL_PREFIX} -P ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/cmake/patch_libraries.cmake
        COMMENT "Patching libraries...")
    add_custom_command(
        TARGET pragma-install-full
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DINSTALL_LOCATION=${CMAKE_INSTALL_PREFIX} -P ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/cmake/patch_libraries.cmake
        COMMENT "Patching libraries...")

endif()

set_target_folder(pragma-install core)
set_target_folder(pragma-install-full core)
