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

unset(WGUI_INCLUDE_DEPENDENCIES CACHE)
set(WGUI_INCLUDE_DEPENDENCIES
    OFF
    CACHE BOOL "Include dependencies required to build the wgui library?")

pr_set_include_path(lz4 "${CMAKE_CURRENT_LIST_DIR}/external_libs/util_source2/third_party_libs/lz4/lib" FORCE)
pr_set_library_path(lz4)

pr_set_include_path(liblzma "${CMAKE_CURRENT_LIST_DIR}/external_libs/util_bsp/third_party_libs/liblzma/include" FORCE)
pr_set_library_path(liblzma lzma)

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

pr_set_include_path(rapidxml "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/rapidxml")
pr_set_include_path(ba2 "${CMAKE_CURRENT_LIST_DIR}/modules/pr_source/third_party/ba2lib/BA2Lib")

pr_set_include_path(exprtk "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/exprtk")

pr_set_include_path(miniball "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/miniball/cpp/main")
pr_set_include_path(bvh "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/bvh/src")

pr_set_library_path(luajit)
pr_set_library_path(lua luajit)

pr_set_include_path(eigen "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/eigen")
pr_set_include_path(tinygltf "${CMAKE_CURRENT_LIST_DIR}/third_party_libs/tinygltf")

set(BOOST_INCLUDEDIR
    ${DEPENDENCY_BOOST_INCLUDE}
    CACHE PATH "Path to include directory." FORCE)
set(BOOST_ROOT
    ${DEPENDENCY_BOOST_INCLUDE}
    CACHE PATH "Path to include directory." FORCE)

if(is_multi_config)
    set(PR_MODULE_INSTALL_SUBDIR "${CONFIG_BUILD_TYPE}")
else()
    set(PR_MODULE_INSTALL_SUBDIR ".")
endif()

message("Processing third-party libaries...")

set(LIBRARY_OUTPUT_PATH
    ""
    CACHE PATH "" FORCE)

set(PRAGMA_INSTALL_COMPONENT "pragma")
set(PRAGMA_INSTALL_COMPONENT_FULL "pragma-full")
add_custom_target(pragma-install COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=${PRAGMA_INSTALL_COMPONENT} -DBUILD_TYPE=${CONFIG_BUILD_TYPE} -P
                                         "${CMAKE_BINARY_DIR}/cmake_install.cmake")
add_custom_target(pragma-install-full COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=${PRAGMA_INSTALL_COMPONENT_FULL} -DBUILD_TYPE=${CONFIG_BUILD_TYPE}
                                              -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")

include(${CMAKE_SOURCE_DIR}/ThirdPartyLibs.cmake)

if(UNIX)
    pr_set_raw_library_path(boost_chrono "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_chrono.a" FORCE)
    pr_set_raw_library_path(boost_date_time "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_date_time.a" FORCE)
    pr_set_raw_library_path(boost_filesystem "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_filesystem.a" FORCE)
    pr_set_raw_library_path(boost_iostreams "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_iostreams.a" FORCE)
    pr_set_raw_library_path(boost_regex "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_regex.a" FORCE)
    pr_set_raw_library_path(boost_system "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_system.a" FORCE)
    pr_set_raw_library_path(boost_thread "${DEPENDENCY_BOOST_LIBRARY_LOCATION}/libboost_thread.a" FORCE)
endif()

# ZLIB_LIBRARY is being changed by one of the external libraries, even though it shouldn't be. Until the cause is found, we'll restore the original values after
# the external libraries have been processed.
set(TMP_ZLIB_INCLUDE_DIR "${ZLIB_INCLUDE_DIR}")
set(TMP_ZLIB_LIBRARY "${ZLIB_LIBRARY}")

message("Processing external libraries...")
add_subdirectory(external_libs)
pr_set_target_folder(wgui external_libs/wgui)
pr_set_target_folder(sample_application external_libs/wgui)
target_include_directories(util_image PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
target_include_directories(cmaterialsystem PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)

# Restore original values
set(ZLIB_INCLUDE_DIR
    "${TMP_ZLIB_INCLUDE_DIR}"
    CACHE STRING "" FORCE)
set(ZLIB_LIBRARY
    "${TMP_ZLIB_LIBRARY}"
    CACHE STRING "" FORCE)

message("Processing core libraries...")
add_subdirectory(core)

message("Processing modules...")
add_subdirectory(modules)

message("Processing tools...")
add_subdirectory(tools)

target_include_directories(libcurl_shared PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)
target_include_directories(materialsystem PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party_libs/zlib)

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
pr_install_targets(pragma pragma_server INSTALL_DIR ".")
if(WIN32)
    pr_install_targets(pragma_console INSTALL_DIR ".")
endif()
pr_install_targets(
    alsoundsystem
    client
    cmaterialsystem
    datasystem
    glfw
    iclient
    iglfw
    iserver
    ishared
    luabind
    luasystem
    materialsystem
    mathutil
    prosper
    server
    shared
    sharedutils
    util_image
    util_sound
    util_unicode
    vfilesystem
    wgui)

message("Custom install targets: ${PRAGMA_INSTALL_CUSTOM_TARGETS}")
set(PRAGMA_INSTALL_DEPENDENCIES pragma pragma_server iclient iserver udm_convert prad pragma_updater ${PRAGMA_INSTALL_CUSTOM_TARGETS})
if(WIN32)
    list(APPEND PRAGMA_INSTALL_DEPENDENCIES pragma_console)
endif()
add_dependencies(pragma-install ${PRAGMA_INSTALL_DEPENDENCIES})
add_dependencies(pragma-install-full pragma iclient iserver)

pr_install_create_directory(modules)

pr_install_targets(
    luabind
    ogg
    vorbis
    vorbisfile
    VTFLib13
    zip
    spdlog
    mpaheader
    zlib
    libz2
    KVParser)

if(WIN32)
    pr_install_files("${CMAKE_CURRENT_LIST_DIR}/third_party_libs/luajit/src/lua51.dll")
else()
    pr_install_files("${CMAKE_CURRENT_LIST_DIR}/third_party_libs/luajit/src/libluajit-p.so")
endif()

pr_install_files("${DEPENDENCY_ICU_ICUUC_BINARY}" "${DEPENDENCY_ICU_ICUDT_BINARY}")

# Modules
file(GLOB PR_MODULES "modules/pr_*")
foreach(MODULE_PATH ${PR_MODULES})
    get_filename_component(MODULE_NAME "${MODULE_PATH}" NAME)
    if(NOT DEFINED PRAGMA_DISABLE_MODULE_${MODULE_NAME} OR NOT PRAGMA_DISABLE_MODULE_${MODULE_NAME})
        if(WIN32)
            set(PR_MODULE_PATH_WITH_EXT "${CMAKE_CURRENT_BINARY_DIR}/modules/${MODULE_NAME}/${PR_MODULE_INSTALL_SUBDIR}/${MODULE_NAME}.dll")
        else()
            set(PR_MODULE_PATH_WITH_EXT "${CMAKE_CURRENT_BINARY_DIR}/modules/${MODULE_NAME}/${PR_MODULE_INSTALL_SUBDIR}/lib${MODULE_NAME}.so")
        endif()
        if(EXISTS "${MODULE_PATH}/Install.cmake")
            message("Running install script for module '${MODULE_PATH}'...")
            include("${MODULE_PATH}/Install.cmake" OPTIONAL)
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
    endif()
endforeach()

# Tools
file(GLOB PR_MODULES "tools/*")
foreach(MODULE_PATH ${PR_MODULES})
    get_filename_component(MODULE_NAME "${MODULE_PATH}" NAME)
    if(EXISTS "${MODULE_PATH}/Install.cmake")
        message("Running install script for tool '${MODULE_PATH}'...")
        include("${MODULE_PATH}/Install.cmake" OPTIONAL)
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

pr_set_target_folder(pragma-install core)
pr_set_target_folder(pragma-install-full core)
