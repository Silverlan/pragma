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
        pr_install_binary(ispctc LIN "libispc_texcomp.so" INSTALL_DIR "${INSTALL_PATH}")
    endif()

    # libdecor
    pr_install_files("${LIBDECOR_BUILD_DIR}/src/libdecor-0.so" INSTALL_DIR "${INSTALL_PATH}")
    pr_install_directory("${LIBDECOR_BUILD_DIR}/src/plugins/" INSTALL_DIR "modules/graphics/vulkan/libdecor/")
endif()

# libzip
pr_install_binary(libzip LIN "libzip.so" WIN "zip.dll" INSTALL_DIR "${INSTALL_PATH}")

# swiftshader
pr_install_binary(swiftshader LIN "libvulkan.so.1" WIN "vulkan-1.dll" INSTALL_DIR "modules/swiftshader/")

# 7zip
if(WIN32)
    pr_install_binary(7zip WIN "7zip.dll" INSTALL_DIR "${INSTALL_PATH}")
endif()

# cpptrace
pr_install_binary(cpptrace LIN "libcpptrace.so" WIN "cpptrace.dll" INSTALL_DIR "${INSTALL_PATH}")
