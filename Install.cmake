set(INSTALL_PATH "${BINARY_OUTPUT_DIR}")

if(UNIX)
    if(ENABLE_ISPC_TEXTURE_COMPRESSOR)
        list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external_libs/util_image/cmake/modules")
        pr_install_binaries(ispctc)
    endif()

    # libdecor
    find_package(libdecor REQUIRED)
    pr_install_binaries(libdecor)
    pr_install_directory("${libdecor_PLUGIN_DIR}" INSTALL_DIR "modules/graphics/vulkan/libdecor/")
endif()

pr_install_binaries(luajit)
if(UNIX)
    pr_install_binaries(sdbus_cpp)
endif()
pr_install_binaries(libzip)

if(lua_debugger_RESOURCE_DIR)
    pr_install_directory("${lua_debugger_RESOURCE_DIR}/" INSTALL_DIR ".")
endif()

if(WIN32)
    pr_install_binaries(zlib)
endif()

# swiftshader
find_package(swiftshader)
if(swiftshader_FOUND)
    pr_install_binaries(swiftshader INSTALL_DIR "modules/swiftshader/")
endif()

# 7z (required for bit7z)
find_package(7z REQUIRED)
pr_install_binaries(7z)

# cpptrace
pr_install_binaries(cpptrace)

# spdlog
pr_install_targets(spdlog)

pr_install_binaries(icu)

if(TARGET util_ocio)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external_libs/util_ocio/cmake/modules")
    pr_install_binaries(opencolorio)
    pr_install_binaries(openimageio)
endif()

if(TARGET util_raytracing)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external_libs/util_raytracing/cmake/modules")
    pr_install_binaries(openimagedenoise)
    pr_install_binaries(openimagedenoise_device PACKAGE openimagedenoise)
    if(UNIX)
        pr_install_binaries(opensubdiv)
    endif()
endif()
