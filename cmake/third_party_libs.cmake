function(pr_fetch_third_party_lib)
    set(options)
    set(oneValueArgs DIR TARGET INC LINK_ONLY NAME)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 PA "${options}" "${oneValueArgs}" "${multiValueArgs}")

    set(SHOULD_INCLUDE_FOLDER 0)
    if(NOT "${PA_UNPARSED_ARGUMENTS}" STREQUAL "")
        list(GET PA_UNPARSED_ARGUMENTS 0 DIR_NAME)
        set(SHOULD_INCLUDE_FOLDER 1)
    else()
        set(DIR_NAME "${PA_TARGET}")
    endif()
    if(DEFINED PA_NAME)
        set(IDENTIFIER "${PA_NAME}")
    else()
        set(IDENTIFIER "${DIR_NAME}")
    endif()
    pr_get_normalized_identifier_name(${IDENTIFIER})

    if(NOT DEFINED PA_DIR)
        set(PA_DIR "${DIR_NAME}")
    endif()

    if(NOT DEFINED PA_TARGET)
        set(PA_TARGET "${PA_DIR}")
    endif()

    if(NOT DEFINED PA_INC)
        set(PA_INC "${CMAKE_SOURCE_DIR}/third_party_libs/${PA_TARGET}/include")
    else()
        set(PA_INC "${CMAKE_SOURCE_DIR}/third_party_libs/${PA_INC}")
    endif()

    message("Including third-party library \"${IDENTIFIER}\" with target \"${PA_TARGET}\" and directory \"${PA_DIR}\"...")

    set(DEPENDENCY_${NORMALIZED_IDENTIFIER}_LIBRARY "$<TARGET_LINKER_FILE:${PA_TARGET}>" CACHE STRING "Path to library." FORCE)
    if(NOT PA_LINK_ONLY)
        pr_set_include_path(${NORMALIZED_IDENTIFIER} "${PA_INC}")
    endif()

    if(SHOULD_INCLUDE_FOLDER)
        list(GET PA_UNPARSED_ARGUMENTS 1 GIT_URL)
        list(GET PA_UNPARSED_ARGUMENTS 2 GIT_SHA)
        pr_fetch_repository(${IDENTIFIER} ${GIT_URL} ${GIT_SHA} "third_party_libs/${PA_DIR}")
    endif()
    if(NOT PA_LINK_ONLY)
		# Some third-party libraries might be using older CMake functions that don't
		# make the include directory public, so we force it here.
		target_include_directories("${PA_TARGET}" INTERFACE "$<BUILD_INTERFACE:${PA_INC}>")
    endif()
    pr_set_target_folder("${PA_TARGET}" third_party_libs)
endfunction()

function(pr_fetch_third_party_repository IDENTIFIER GIT_URL GIT_SHA)
    FetchContent_Declare(
        ${IDENTIFIER}
        GIT_REPOSITORY ${GIT_URL}
        GIT_TAG        ${GIT_SHA}
        SOURCE_DIR     "${CMAKE_SOURCE_DIR}/third_party_libs/${IDENTIFIER}"
        SOURCE_SUBDIR  "${CMAKE_SOURCE_DIR}/devnull"
    )
    FetchContent_MakeAvailable(${IDENTIFIER})
endfunction()

# Misc
pr_fetch_third_party_repository("bvh"                 "https://github.com/madmann91/bvh"                    "ac41ab8")
pr_fetch_third_party_repository("exprtk"              "https://github.com/ArashPartow/exprtk"               "f46bffcd6966d38a09023fb37ba9335214c9b959")
pr_fetch_third_party_repository("freetype"            "https://github.com/aseprite/freetype2"               "9a2d6d97b2d8a5d22d02948b783df12b764afa2d")
pr_fetch_third_party_repository("miniball"            "https://github.com/Silverlan/miniball"               "609fbf16e7a9cc3dc8f88e4d1c7a1d8ead842bb1")

if(WIN32)
    pr_fetch_third_party_repository("nvtt"            "https://github.com/castano/nvidia-texture-tools"     "aeddd65f81d36d8cb7b169b469ef25156666077e")
endif()

pr_fetch_third_party_repository("spdlog"              "https://github.com/Silverlan/spdlog"                 "d1ebfb9")                                  # Branch: "v1.x"
pr_fetch_third_party_repository("squish"              "https://github.com/ivan-kulikov-dev/squish"          "aeb01b8f88f97c994baff8453870f3e8a347afd7")

# Luabind
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ".." FORCE)
set(LUABIND_BUILD_SHARED ON CACHE BOOL "Build luabind as a shared library?" FORCE)
set(LUA_INCLUDE_DIR ${luajit_INCLUDE_DIRS} CACHE STRING "" FORCE)
set(LUA_LIBRARY ${luajit_LIBRARIES} CACHE STRING "" FORCE)
set(LUA_FOUND ON CACHE BOOL ON FORCE)
set(LUA_LIBRARIES ${LUA_LIBRARY} CACHE STRING "" FORCE)
pr_fetch_third_party_lib("luabind" INC luabind "https://github.com/Silverlan/luabind-deboostified" "38b223d4a8538d08f7ddded7989f4297aa74ee6d")
pr_set_target_folder(luabind third_party_libs)
target_include_directories(luabind PUBLIC ${LUA_INCLUDE_DIR})
target_include_directories(luabind PUBLIC "${CMAKE_SOURCE_DIR}/third_party_libs/luabind")
unset(BUILD_SHARED_LIBS CACHE)
#

# efsw
set(VERBOSE
    OFF
    CACHE BOOL OFF FORCE)
set(EFSW_INSTALL
    OFF
    CACHE BOOL OFF FORCE)
pr_fetch_third_party_lib("efsw" INC "efsw" "https://github.com/SpartanJ/efsw" "1bb814c")
pr_set_target_folder(efsw third_party_libs)
#

# mpaheader
pr_fetch_third_party_lib("mpaheader" "https://github.com/Silverlan/mpaheader" "ebb53b9ae31c99dad8da2ce9024c37d4c26ce685")
pr_set_target_folder(mpaheader third_party_libs)
#

if(UNIX)
    # linenoise
    pr_fetch_third_party_lib("linenoise.cpp" TARGET linenoise "https://github.com/Silverlan/linenoise.cpp" "74ae32ebf22d18a7e22afe1eeecf04171311ebcf")
    pr_set_target_folder(linenoise third_party_libs)
    #
endif()

# VKVParser
pr_fetch_third_party_lib("vkvparser"
    TARGET KVParser
    DIR vkvparser
    INC "vkvparser/include"
    "https://github.com/Silverlan/VKVParser" "380f5527ec57ac7ce540da67a86a23dbfdb593d9"
)
#

# gli
pr_fetch_third_party_repository("gli" "https://github.com/Silverlan/gli" "2938076944739166235750f3ccdce37c6cac2d31")
unset(BUILD_SHARED_LIBS CACHE)
add_subdirectory("third_party_libs/gli/gli")
set(BUILD_SHARED_LIBS ON CACHE BOOL ".." FORCE)
#

# 7zip cpp
if(WIN32)
    set(BUILD_SHARED_LIBS
        OFF
        CACHE BOOL OFF FORCE)
    pr_fetch_third_party_lib("7zpp" "https://github.com/Silverlan/7zip-cpp" "afed5bccd4faf4f825a640aa94a7ed78f54c178c")
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
pr_fetch_third_party_lib("muparserx" INC "muparserx/parser" "https://github.com/Silverlan/muparserx" "910f6128494e174c3de81bd34c3d938b47873c08")
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ".." FORCE)
pr_set_target_folder(muparserx third_party_libs/muparserx)
#

# clip2tri
set(M_LIBRARY "")
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL OFF FORCE)
set(BUILD_STATIC_LIBS
    ON
    CACHE BOOL ON FORCE)
pr_fetch_third_party_lib("clip2tri" TARGET clip2tri-static INC "clip2tri" "https://github.com/raptor/clip2tri" "f62a734d22733814b8a970ed8a68a4d94c24fa5f")
pr_fetch_third_party_lib(NAME clipper TARGET clipper-static)
pr_fetch_third_party_lib(NAME poly2tri TARGET poly2tri-static)
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ".." FORCE)
pr_set_target_folder(clip2tri third_party_libs/clip2tri)
pr_set_target_folder(clip2tri-static third_party_libs/clip2tri)
pr_set_target_folder(clipper third_party_libs/clip2tri)
pr_set_target_folder(clipper-static third_party_libs/clip2tri)
pr_set_target_folder(poly2tri third_party_libs/clip2tri)
pr_set_target_folder(poly2tri-static third_party_libs/clip2tri)
#
# BUG: M_LIBRARY is not restored to previous value breaking anything using it.

# eigen
pr_fetch_third_party_repository("eigen" "https://gitlab.com/libeigen/eigen.git" "b66188b5") # v3.4.1
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
pr_fetch_third_party_lib("glfw" "https://github.com/Silverlan/glfw" "8d6f5d9bc8477057dccc1190d46b0945219e2f33") # Branch: "feat/drag-callback"
pr_set_target_folder(glfw third_party_libs/glfw)
#

# libogg
pr_fetch_third_party_lib("ogg" "https://github.com/xiph/ogg" "be05b13") # v1.3.6
pr_set_target_folder(ogg third_party_libs)
set(OGG_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/third_party_libs/ogg/include" CACHE STRING "" FORCE)
set(OGG_LIBRARIES "$<TARGET_LINKER_FILE:ogg>" CACHE STRING "" FORCE)
target_include_directories(ogg INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/ogg/include>")
#

# libvorbis
pr_fetch_third_party_lib("vorbis" TARGET vorbis "https://github.com/xiph/vorbis" "43bbff0141028e58d476c1d5fd45dd5573db576d")
pr_fetch_third_party_lib(TARGET vorbisfile)
pr_set_target_folder(vorbis third_party_libs/vorbis)
pr_set_target_folder(vorbisenc third_party_libs/vorbis)
pr_set_target_folder(vorbisfile third_party_libs/vorbis)

target_include_directories(vorbis PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/vorbis/include>")
target_include_directories(vorbis PUBLIC "$<BUILD_INTERFACE:${OGG_INCLUDE_DIRS}>")
target_include_directories(vorbis PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/ogg/include>")

add_dependencies(vorbis ogg)
#

# bzip2
set(ENABLE_LIB_ONLY ON CACHE BOOL ON FORCE)
pr_fetch_third_party_lib("bzip2" TARGET bz2 INC "bzip2" "https://github.com/Silverlan/bzip2" "6f08ffc2409d34a8f0f06ab3b0291bced4e9d08c")
pr_set_target_folder(bz2 third_party_libs/bzip2)
pr_set_target_folder(bz2_static third_party_libs/bzip2)
pr_set_target_folder(bz2_ObjLib third_party_libs/bzip2)
#

# libnoise
set(BUILD_LIBNOISE_EXAMPLES
    OFF
    CACHE BOOL ".." FORCE)
set(BUILD_LIBNOISE_UTILS
    ON
    CACHE BOOL ".." FORCE)
unset(BUILD_SHARED_LIBS CACHE)

pr_fetch_third_party_lib("libnoise" TARGET noise-static INC "libnoise/src" "https://github.com/qknight/libnoise" "9ce0737b55812f7de907e86dc633724524e3a8e8")
pr_fetch_third_party_lib(TARGET noiseutils-static INC "libnoise/noiseutils")
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ".." FORCE)
pr_set_target_folder(noise third_party_libs/libnoise)
pr_set_target_folder(noise-static third_party_libs/libnoise)
pr_set_target_folder(noiseutils third_party_libs/libnoise)
pr_set_target_folder(noiseutils-static third_party_libs/libnoise)

target_include_directories(noiseutils-static PRIVATE ${CMAKE_SOURCE_DIR}/third_party_libs/libnoise/src)

pr_find_library(libnoise NAMES "noise" PATHS "third_party_libs/libnoise/src/${BINARY_PRAGMA_DIR}")
pr_find_library(libnoiseutils NAMES "noiseutils-static" PATHS "third_party_libs/libnoise/noiseutils/${BINARY_PRAGMA_DIR}")
#

# lunasvg
pr_fetch_third_party_lib("lunasvg" "https://github.com/sammycage/lunasvg" "7b6970f") # Branch: v3.4.0
pr_set_target_folder(lunasvg third_party_libs/lunasvg)
pr_set_target_folder(plutovg third_party_libs/lunasvg)
pr_set_target_folder(smiley third_party_libs/lunasvg)
pr_set_target_folder(svg2png third_party_libs/lunasvg)
#

# libsamplerate (required for alsoundsystem with fmod enabled)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL OFF FORCE)
pr_fetch_third_party_lib("libsamplerate" TARGET samplerate INC "libsamplerate/src" "https://github.com/libsndfile/libsamplerate" "15c392d")
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ON FORCE)
pr_set_target_folder(callback_hang_test third_party_libs/libsamplerate)
pr_set_target_folder(callback_test third_party_libs/libsamplerate)
pr_set_target_folder(clone_test third_party_libs/libsamplerate)
pr_set_target_folder(downsample_test third_party_libs/libsamplerate)
pr_set_target_folder(float_short_test third_party_libs/libsamplerate)
pr_set_target_folder(misc_test third_party_libs/libsamplerate)
pr_set_target_folder(multi_channel_test third_party_libs/libsamplerate)
pr_set_target_folder(multichan_throughput_test third_party_libs/libsamplerate)
pr_set_target_folder(nullptr_test third_party_libs/libsamplerate)
pr_set_target_folder(reset_test third_party_libs/libsamplerate)
pr_set_target_folder(samplerate third_party_libs/libsamplerate)
pr_set_target_folder(simple_test third_party_libs/libsamplerate)
pr_set_target_folder(snr_bw_test third_party_libs/libsamplerate)
pr_set_target_folder(streaming_test third_party_libs/libsamplerate)
pr_set_target_folder(termination_test third_party_libs/libsamplerate)
pr_set_target_folder(throughput_test third_party_libs/libsamplerate)
pr_set_target_folder(timewarp-file third_party_libs/libsamplerate)
pr_set_target_folder(varispeed_test third_party_libs/libsamplerate)
pr_set_target_folder(varispeed-play third_party_libs/libsamplerate)

if(UNIX)
    set_target_properties(lunasvg PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
endif()
#

# vtflib
set(USE_LIBTXC_DXTN
    OFF
    CACHE BOOL OFF FORCE)
pr_fetch_third_party_lib("vtflib" TARGET VTFLib13 INC "vtflib/src" "https://github.com/Silverlan/VTFLib" "3e6b80bcd6dacf6bd4a35666869464fac55051cc")
pr_set_include_path(vtflib_build "${VTFLib_BINARY_DIR}/src")
pr_set_target_folder(VTFLib13 third_party_libs)

target_include_directories(VTFLib13 PUBLIC "$<BUILD_INTERFACE:${DEPENDENCY_LIBZIP_CONF_INCLUDE}>")
target_include_directories(VTFLib13 PUBLIC "$<BUILD_INTERFACE:${VTFLib_BINARY_DIR}/src>")
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
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL OFF FORCE)
pr_fetch_third_party_lib("recastnavigation" TARGET Recast INC "recastnavigation/Recast/Include" "https://github.com/recastnavigation/recastnavigation" "6dc1667") # v1.6.0
pr_fetch_third_party_lib(TARGET Detour INC "recastnavigation/Detour/Include")
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ON FORCE)
pr_set_target_folder(DebugUtils third_party_libs/recast)
pr_set_target_folder(Detour third_party_libs/recast)
pr_set_target_folder(DetourCrowd third_party_libs/recast)
pr_set_target_folder(DetourTileCache third_party_libs/recast)
pr_set_target_folder(Recast third_party_libs/recast)

pr_find_library(recast NAMES "Recast" PATHS "third_party_libs/recastnavigation/Recast/${BINARY_PRAGMA_DIR}")
pr_find_library(detour NAMES "Detour" PATHS "third_party_libs/recastnavigation/Detour/${BINARY_PRAGMA_DIR}")
#

# rectangle_bin_pack
pr_fetch_third_party_lib("rectangle_bin_pack" INC "rectangle_bin_pack/RectangleBinPack" "https://github.com/Silverlan/rectangle_bin_pack_cmake.git" "3e45db4b84ddaaece60a0985cc93c84ae1d2587f")
pr_set_target_folder(rectangle_bin_pack third_party_libs)
pr_find_library(rectanglebinpack NAMES "rectangle_bin_pack" PATHS "${rectangle_bin_pack_BINARY_DIR}/${BINARY_PRAGMA_DIR}")
#

# RapidXml
pr_fetch_third_party_repository("rapidxml" "https://github.com/discord/rapidxml" "2ae4b2888165a393dfb6382168825fddf00c27b9")
pr_set_include_path(rapidxml "${CMAKE_SOURCE_DIR}/third_party_libs/rapidxml")
#

# OpenFBX
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL OFF FORCE)
pr_fetch_third_party_lib("openfbx" TARGET OpenFBX INC "OpenFBX/src" "https://github.com/Silverlan/OpenFBX" "e757b8a8db76a9c4bb168eee5321bdc1ba704a54")
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ON FORCE)
pr_set_target_folder(OpenFBX third_party_libs)
#

# In Linux there is a cyclic deps between freetype,harfbuzz,pango,cairo and most importantly fontconfig. Fontconfig in linux is reposnsible for discovery of
# fonts. (think C:\Windows\Fonts for linux but customizable via configs) recently I hit a snag in which harfbuzz failed to load due to missing pango symbols.
# The AppImage team hit similar snag too, see https://github.com/AppImageCommunity/pkg2appimage/pull/323 and
# https://github.com/probonopd/linuxdeployqt/issues/261 Even if I would include pango and cairo into our pipeline, CEF would complain (coincidentally hitting
# AppImage's snag too), since that DOES use system fonts. (fontconfig has persistent presence in all graphical managers in linux)
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
#

# tinygltf
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL OFF FORCE)
pr_fetch_third_party_lib("tinygltf" "https://github.com/syoyo/tinygltf" "37250b3") # v2.9.6 Branch: "release"
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL ON FORCE)
#
