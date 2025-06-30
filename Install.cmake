set(INSTALL_PATH "${BINARY_OUTPUT_DIR}")

# Compressonator
if(ENABLE_COMPRESSONATOR)
    pr_install_binary(compressonator WIN "CMP_Compressonator.dll" LIN "libCMP_Compressonator.so" BIN_DIR "${DEPENDENCY_COMPRESSONATOR_BINARY_DIR}" INSTALL_DIR "${INSTALL_PATH}")
    pr_install_binary(compressonator WIN "CMP_Framework.dll" LIN "libCMP_Framework.so" BIN_DIR "${DEPENDENCY_COMPRESSONATOR_BINARY_DIR}" INSTALL_DIR "${INSTALL_PATH}")
    if(WIN32)
        # We need to copy dxcompiler.dll and dxil.dll to bin directory. Otherwise pragma with compressonator will fail.
        pr_install_binary(compressonator WIN "dxcompiler.dll" BIN_DIR "${DEPENDENCY_COMPRESSONATOR_COMMON_DIR}/lib/ext/dxc/bin/x64" INSTALL_DIR "${INSTALL_PATH}")
        pr_install_binary(compressonator WIN "dxil.dll" BIN_DIR "${DEPENDENCY_COMPRESSONATOR_COMMON_DIR}/lib/ext/dxc/bin/x64" INSTALL_DIR "${INSTALL_PATH}")
    endif()
endif()

if(UNIX)
    if(ENABLE_ISPC_TEXTURE_COMPRESSOR)
        pr_install_binaries(ispctc)
    endif()

    # libdecor
    find_package(libdecor REQUIRED)
    pr_install_binaries(libdecor)
    pr_install_directory("${libdecor_PLUGIN_DIR}" INSTALL_DIR "modules/graphics/vulkan/libdecor/")
endif()

# libzip
pr_install_binaries(libzip)

# swiftshader
find_package(swiftshader)
if(swiftshader_FOUND)
    pr_install_binaries(swiftshader INSTALL_DIR "modules/swiftshader/")
endif()

# 7zip
if(WIN32)
    pr_install_binary(7zip WIN "7zip.dll" INSTALL_DIR "${INSTALL_PATH}")
endif()

# 7z (required for bit7z)
find_package(7z REQUIRED)
pr_install_binaries(7z)

# cpptrace
pr_install_binaries(cpptrace)

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
    pr_install_binaries(opensubdiv)
endif()
