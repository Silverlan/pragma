cmake_minimum_required(VERSION 3.28)

include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/pr_legacy.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/pr_install.cmake)

project(pragma)

option(WITH_PFM "Build with Pragma Filmmaker?" OFF)
option(WITH_VR "Build with VR support?" OFF)
option(WITH_COMMON_ENTITIES "Build with common entity scripts?" OFF)
option(WITH_COMMON_MODULES "Build with common modules?" ON)

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

# Dependencies
set(BOOST_ROOT        "${PRAGMA_DEPS_DIR}/boost/cmake")
set(BOOST_INCLUDEDIR  "${PRAGMA_DEPS_DIR}/boost/include")
set(BOOST_LIBRARYDIR  "${PRAGMA_DEPS_DIR}/boost/lib")

if(NOT DEFINED PRAGMA_DEPS_DIR)
    message(FATAL_ERROR "PRAGMA_DEPS_DIR is not defined")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(bit7z REQUIRED)
find_package(boost REQUIRED)
find_package(cpptrace REQUIRED)
find_package(geometric_tools REQUIRED)
find_package(icu REQUIRED)
find_package(libpng REQUIRED)
find_package(libzip REQUIRED)
find_package(luajit REQUIRED)
find_package(lua_debugger)
find_package(zlib REQUIRED)
if(UNIX)
    find_package(sdbus_cpp REQUIRED)
endif()
#

# This generates a compile_commands.json file in the build directory to help clangd parse the project
# and provide code completion.
# This is only useful if you are using clangd as your language server.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

unset(CONFIG_ENABLE_EXECUTABLE_FLAG CACHE)
set(CONFIG_ENABLE_EXECUTABLE_FLAG
    OFF
    CACHE BOOL "")

unset(WGUI_INCLUDE_DEPENDENCIES CACHE)
set(WGUI_INCLUDE_DEPENDENCIES
    OFF
    CACHE BOOL "Include dependencies required to build the wgui library?")

if(UNIX)
    find_package(Freetype REQUIRED)
endif()

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
add_custom_target(pragma-core)
set_target_properties(pragma-core PROPERTIES FOLDER "Hidden")

include(${CMAKE_SOURCE_DIR}/ThirdPartyLibs.cmake)

message("Processing external libraries...")
add_subdirectory(external_libs)
pr_set_target_folder(wgui external_libs/wgui)
pr_set_target_folder(sample_application external_libs/wgui)
target_include_directories(util_image PUBLIC ${DEPENDENCY_ZLIB_INCLUDE})
target_include_directories(cmaterialsystem PUBLIC ${DEPENDENCY_ZLIB_INCLUDE})

message("Processing core libraries...")
add_subdirectory(core)

message("Processing tools...")
add_subdirectory(tools)

message("Processing modules...")
add_subdirectory(modules)

target_include_directories(libcurl_shared PRIVATE ${DEPENDENCY_ZLIB_INCLUDE})
target_include_directories(materialsystem PRIVATE ${DEPENDENCY_ZLIB_INCLUDE})

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
    oskit
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
add_dependencies(pragma-core ${PRAGMA_INSTALL_DEPENDENCIES})
add_dependencies(pragma-install-full pragma-install)

pr_install_create_directory(modules)

pr_install_targets(
    luabind
    ogg
    vorbis
    vorbisfile
    VTFLib13
    spdlog
    mpaheader
    bz2
    KVParser
    lunasvg
    plutovg
    efsw)

pr_install_files("${DEPENDENCY_ICU_ICUUC_BINARY}" "${DEPENDENCY_ICU_ICUDT_BINARY}")

pr_install_component("core")
message("Installing core components...")
pr_include_install(".")

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
	        pr_install_component(${MODULE_NAME})
            pr_include_install("${MODULE_PATH}")
        else()
            # Default behavior
            message("Adding default installation rule for module \"${MODULE_PATH}\": \"${PR_MODULE_PATH_WITH_EXT}\" to \"${CMAKE_INSTALL_PREFIX}/modules/\"...")
            add_custom_command(
                TARGET pragma-install
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -DInstallSrc=${PR_MODULE_PATH_WITH_EXT} -DInstallDst=\"${CMAKE_INSTALL_PREFIX}/modules/\" -P
                        ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/cmake/install_file.cmake
                COMMENT "Copying ${PR_MODULE_PATH_WITH_EXT} to ${CMAKE_INSTALL_PREFIX}/modules/...")
            pr_include_cmake_modules("${MODULE_PATH}")
        endif()
    endif()
endforeach()

# Tools
file(GLOB PR_MODULES "tools/*")
foreach(MODULE_PATH ${PR_MODULES})
    get_filename_component(MODULE_NAME "${MODULE_PATH}" NAME)
    if(EXISTS "${MODULE_PATH}/Install.cmake")
        message("Running install script for tool '${MODULE_PATH}'...")
	    pr_install_component(${MODULE_NAME})
        pr_include_install("${MODULE_PATH}")
    endif()
endforeach()

# Link components to pragma-install
get_property(COMPONENT_LIST GLOBAL PROPERTY PRAGMA_INSTALL_COMPONENTS)
foreach(_COMP IN LISTS COMPONENT_LIST)
    add_custom_target(pragma-install-${_COMP} COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=${_COMP} -DBUILD_TYPE=${CONFIG_BUILD_TYPE} -P
                                         "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    # We need to make sure all build-targets run first, before any install targets
    add_dependencies(pragma-install-${_COMP} pragma-core)

    add_dependencies(pragma-install pragma-install-${_COMP})

    set_target_properties(pragma-install-${_COMP} PROPERTIES FOLDER "Hidden/Install")
endforeach()

# Install assets
install(
    DIRECTORY assets/
    DESTINATION "."
    COMPONENT ${PRAGMA_INSTALL_COMPONENT_FULL})

message("Processing addons...")
include(${CMAKE_SOURCE_DIR}/Addons.cmake)

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
