cmake_minimum_required(VERSION 4.2.0)

message(STATUS "CMake version: ${CMAKE_VERSION}")

# Required for "import std;" as long as its experimental in CMake
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_MODULE_STD 1)

# set(CMAKE_C_STANDARD 23)
# set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Use response files to avoid "The command line is too long." errors
set(CMAKE_ASM_USE_RESPONSE_FILE_FOR_INCLUDES 1)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)

set(CMAKE_ASM_USE_RESPONSE_FILE_FOR_OBJECTS 1)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)

set(CMAKE_ASM_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES 1)

if(CMAKE_GENERATOR MATCHES "Visual Studio.*")
    message(FATAL_ERROR "Detected generator: ${CMAKE_GENERATOR} (Visual Studio generator is currently not supported. Please use Ninja generator for the time being).")
endif()
if ("${CMAKE_GENERATOR}" MATCHES "Ninja")
    set(CMAKE_NINJA_FORCE_RESPONSE_FILE TRUE CACHE BOOL "Force Ninja to use response files for long command lines")
endif()

set(PRAGMA_DEPS_DIR "${CMAKE_SOURCE_DIR}/deps/staging/" CACHE PATH "Directory where dependencies will be extracted to.")
set(PRAGMA_BUILD_TOOLS_DIR "${CMAKE_SOURCE_DIR}/build_tools/" CACHE PATH "Directory where additional build tool dependencies will be extracted to.")

include("cmake/fetch_clang.cmake")

if(UNIX)
    # Compilers are still somewhat unreliable at finding the modules.json (which is required for import std), so
    # we'll help them out for now. For newer distros (e.g. arch-based), it's typically available on the system,
    # for other cases (e.g. for Ubuntu 24), we'll use our own shipped version of gcc.
    if(EXISTS "/usr/lib/libstdc++.modules.json")
        set(CMAKE_CXX_STDLIB_MODULES_JSON "/usr/lib/libstdc++.modules.json")
    else()
        include("cmake/fetch_gcc.cmake")
        set(CMAKE_CXX_STDLIB_MODULES_JSON "${PRAGMA_BUILD_TOOLS_DIR}/gcc/lib64/libstdc++.modules.json")
    endif()
endif()

include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/pr_legacy.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/pr_install.cmake)

include(FetchContent)

function(pr_fetch_repository IDENTIFIER GIT_URL GIT_SHA CHECKOUT_PATH)
    FetchContent_Declare(
        ${IDENTIFIER}
        GIT_REPOSITORY ${GIT_URL}
        GIT_TAG        ${GIT_SHA}
        SOURCE_DIR     "${CMAKE_SOURCE_DIR}/${CHECKOUT_PATH}"
    )
    FetchContent_MakeAvailable(${IDENTIFIER})
endfunction()

project(pragma)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    message(FATAL_ERROR "Detected generator: ${CMAKE_GENERATOR} (clang-cl is currently not supported. Please use clang instead.).")
endif()

set(TOOLSET "clang")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(TOOLSET "clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(TOOLSET "msvc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(TOOLSET "gcc")
endif()

include("cmake/fetch_deps.cmake")

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

option(PRAGMA_DEBUG "Build in debug mode?" OFF)
option(PRAGMA_WITH_VR "Build with VR support?" OFF)
option(PRAGMA_WITH_NETWORKING "Build with networking support?" OFF)
option(PRAGMA_WITH_COMMON_ENTITIES "Build with common entity scripts?" OFF)
option(PRAGMA_WITH_COMMON_MODULES "Build with common modules?" ON)
option(PRAGMA_WITH_ESSENTIAL_CLIENT_MODULES "Build with essential client modules?" ON)

option(PRAGMA_WITH_PFM "Build with Pragma Filmmaker?" OFF)
option(PRAGMA_WITH_CORE_PFM_MODULES "Include essential PFM modules." ON)
option(PRAGMA_WITH_ALL_PFM_MODULES "Include non-essential PFM modules (e.g. chromium and cycles)." ON)

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
else()
    find_package(wintoast REQUIRED)
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

set_property(GLOBAL PROPERTY PR_INSTALL_COMMANDS "")
set(install_targets "pragma-install-base" "pragma-install" "pragma-install-full" "pragma-install-full-no-executable")
set_property(GLOBAL PROPERTY PR_INSTALL_TARGETS ${install_targets})
foreach(install_target ${install_targets})
    add_custom_target(${install_target}
        ALL
        COMMAND ${CMAKE_COMMAND}
                --install ${CMAKE_BINARY_DIR}
                --config ${CONFIG_BUILD_TYPE}
                --prefix "${INSTALL_PREFIX}"
                --component ${install_target}
        COMMENT "Installing '${install_target}' component into ${INSTALL_PREFIX}"
        VERBATIM
    )
endforeach()

add_custom_target(pragma-core)
set_target_properties(pragma-core PROPERTIES FOLDER "Hidden")

include(${CMAKE_SOURCE_DIR}/cmake/third_party_libs.cmake)

message("Processing external libraries...")
include(${CMAKE_SOURCE_DIR}/cmake/external_libs.cmake)
pr_set_target_folder(wgui external_libs/wgui)
pr_set_target_folder(sample_application external_libs/wgui)

set_target_properties(networkmanager PROPERTIES FOLDER external_libs/networkmanager)
set_target_properties(clientmanager PROPERTIES FOLDER external_libs/networkmanager)
set_target_properties(servermanager PROPERTIES FOLDER external_libs/networkmanager)

set_target_properties(materialsystem PROPERTIES FOLDER external_libs/materialsystem)
set_target_properties(cmaterialsystem PROPERTIES FOLDER external_libs/materialsystem)

target_include_directories(util_image PUBLIC ${DEPENDENCY_ZLIB_INCLUDE})
target_include_directories(cmaterialsystem PUBLIC ${DEPENDENCY_ZLIB_INCLUDE})

# Registers a convenience target, which will do a full-build +install and can be used to run an executable target.
function(pr_add_executable_install_target TARGET_NAME EXECUTABLE_TARGET_NAME)
    add_custom_target(${TARGET_NAME}
        ALL
        COMMAND ${CMAKE_COMMAND}
                --install ${CMAKE_BINARY_DIR}
                --config ${CONFIG_BUILD_TYPE}
                --prefix "${INSTALL_PREFIX}"
                --component ${TARGET_NAME}
        COMMENT "Installing '${TARGET_NAME}' component into ${INSTALL_PREFIX}"
        VERBATIM
    )
    pr_set_install_target(${TARGET_NAME})
    add_dependencies(${TARGET_NAME} ${EXECUTABLE_TARGET_NAME} pragma-install-full-no-executable)
    add_dependencies(${EXECUTABLE_TARGET_NAME} pragma-install-full-no-executable)
    pr_set_target_folder(${TARGET_NAME} core)
endfunction()

function(pr_set_output_directory_to_install TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_INSTALL_PREFIX}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_INSTALL_PREFIX}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_INSTALL_PREFIX}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_INSTALL_PREFIX}"
    )
endfunction()

message("Processing core libraries...")
add_subdirectory(core)

message("Processing modules...")
include(${CMAKE_SOURCE_DIR}/cmake/modules.cmake)

message("Processing tools...")
add_subdirectory(tools)

target_include_directories(materialsystem PRIVATE ${DEPENDENCY_ZLIB_INCLUDE})

set_target_properties(pragma_executable PROPERTIES DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
set_target_properties(pragma_server_executable PROPERTIES DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
if(WIN32)
    set_target_properties(pragma_executable PROPERTIES VS_DEBUGGER_COMMAND "${CMAKE_INSTALL_PREFIX}/pragma.exe")
    set_target_properties(pragma_server_executable PROPERTIES VS_DEBUGGER_COMMAND "${CMAKE_INSTALL_PREFIX}/pragma_server.exe")

    set_target_properties(pragma_executable PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext -graphics_api vulkan")
    set_target_properties(pragma_server_executable PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-console -luaext")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT pragma_executable)

# Installation
pr_install_targets(pragma_executable pragma_server_executable INSTALL_DIR ".")
if(WIN32)
    pr_install_targets(pragma_console INSTALL_DIR "." RENAME "pragma.com")
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
add_dependencies(pragma-install pragma_executable pragma_server_executable)
set(PRAGMA_INSTALL_DEPENDENCIES iclient iserver udm_convert prad pragma_updater ${PRAGMA_INSTALL_CUSTOM_TARGETS})
if(WIN32)
    list(APPEND PRAGMA_INSTALL_DEPENDENCIES pragma_console)
endif()
add_dependencies(pragma-core ${PRAGMA_INSTALL_DEPENDENCIES})

pr_install_create_directory(modules)

pr_install_targets(
    luabind
    ogg
    vorbis
    vorbisfile
    VTFLib13
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
get_property(MODULE_NAMES GLOBAL PROPERTY PRAGMA_MODULE_NAMES)
foreach(MODULE_NAME ${MODULE_NAMES})
    set(MODULE_PATH "${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}")
    if(EXISTS "${MODULE_PATH}/Install.cmake")
        message("Running install script for module '${MODULE_PATH}'...")
        pr_install_component(${MODULE_NAME})
        pr_include_install("${MODULE_PATH}")
    else()
        # Default behavior
        get_property(skip_install GLOBAL PROPERTY PRAGMA_MODULE_${MODULE_NAME}_SKIP_INSTALL)
        if(NOT skip_install)
            get_property(_targets GLOBAL PROPERTY PRAGMA_MODULE_${MODULE_NAME}_TARGETS)
            foreach(MODULE_TARGET IN LISTS _targets)
                get_target_property(MODULE_BIN_PATH ${MODULE_TARGET} BINARY_DIR)
                if(WIN32)
                    set(PR_MODULE_PATH_WITH_EXT "${MODULE_BIN_PATH}/${PR_MODULE_INSTALL_SUBDIR}/${MODULE_TARGET}.dll")
                else()
                    set(PR_MODULE_PATH_WITH_EXT "${MODULE_BIN_PATH}/${PR_MODULE_INSTALL_SUBDIR}/lib${MODULE_TARGET}.so")
                endif()

                message("Adding default installation rule for module \"${MODULE_PATH}\": \"${PR_MODULE_PATH_WITH_EXT}\" to \"${CMAKE_INSTALL_PREFIX}/modules/\"...")
                add_custom_command(
                    TARGET pragma-install-base
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -DInstallSrc=${PR_MODULE_PATH_WITH_EXT} -DInstallDst=\"${CMAKE_INSTALL_PREFIX}/modules/\" -P
                            ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/cmake/install_file.cmake
                    COMMENT "Copying ${PR_MODULE_PATH_WITH_EXT} to ${CMAKE_INSTALL_PREFIX}/modules/...")
                pr_include_cmake_modules("${MODULE_PATH}")
            endforeach()
        endif()
    endif()
endforeach()

get_property(MODULE_TARGETS GLOBAL PROPERTY PRAGMA_MODULE_TARGETS)
foreach(MODULE_TARGET ${MODULE_TARGETS})
    add_dependencies(pragma-core ${MODULE_TARGET})
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

# Link components to pragma-install-base
get_property(COMPONENT_LIST GLOBAL PROPERTY PRAGMA_INSTALL_COMPONENTS)
foreach(_COMP IN LISTS COMPONENT_LIST)
    add_custom_target(pragma-install-${_COMP} COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=${_COMP} -DBUILD_TYPE=${CONFIG_BUILD_TYPE} -P
                                         "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    # We need to make sure all build-targets run first, before any install targets
    add_dependencies(pragma-install-${_COMP} pragma-core)

    add_dependencies(pragma-install-base pragma-install-${_COMP})

    set_target_properties(pragma-install-${_COMP} PROPERTIES FOLDER "Hidden/Install")
endforeach()

function(pr_manifest_install IDENTIFIER SRC_DIR REL_DST_DIR COMPONENT)
    set(MANIFEST_PATH "${CMAKE_INSTALL_PREFIX}/.install_cache/manifests/${IDENTIFIER}.txt")
    set(FULL_DST_DIR "${CMAKE_INSTALL_PREFIX}/${REL_DST_DIR}")

    install(CODE "
        set(SRC_DIR \"${SRC_DIR}\")
        set(DST_DIR \"${FULL_DST_DIR}\")
        set(MANIFEST_FILE \"${MANIFEST_PATH}\")
        include(\"${CMAKE_CURRENT_SOURCE_DIR}/cmake/ManifestInstall.cmake\")
    " COMPONENT ${COMPONENT})
endfunction()

# Install assets
pr_manifest_install("core" "${CMAKE_CURRENT_SOURCE_DIR}/assets" "" pragma-install-full)
pr_manifest_install("core" "${CMAKE_CURRENT_SOURCE_DIR}/assets" "" pragma-install-full-no-executable)

message("Processing addons...")
include(${CMAKE_SOURCE_DIR}/Addons.cmake)

pr_set_target_folder(pragma-install-base core)
pr_set_target_folder(pragma-install core)
pr_set_target_folder(pragma-install-full core)

get_property(install_targets GLOBAL PROPERTY PR_INSTALL_TARGETS)
get_property(install_commands GLOBAL PROPERTY PR_INSTALL_COMMANDS)
foreach(component IN LISTS install_targets)
    add_dependencies(${component} pragma-install-base)
    cmake_language(EVAL CODE "${install_commands}")
endforeach()
