include("cmake/install_helper.cmake")

# We need the very latest version of clang, which is currently not available in any package managers yet, so we'll download it manually for now.
function(pr_fetch_clang)
    set(version "22.1.0-rc1")
    if(WIN32)
        set(url "https://github.com/llvm/llvm-project/releases/download/llvmorg-${version}/clang+llvm-${version}-x86_64-pc-windows-msvc.tar.xz")
    else()
        set(url "https://github.com/llvm/llvm-project/releases/download/llvmorg-${version}/LLVM-${version}-Linux-X64.tar.xz")
    endif()
    check_content_version("${PRAGMA_BUILD_TOOLS_DIR}/clang" "${version}" "version.json" IS_VALID)
    if(NOT IS_VALID)
        clear_content("${PRAGMA_BUILD_TOOLS_DIR}/clang")
        pr_http_extract(
            "${url}"
            "${PRAGMA_BUILD_TOOLS_DIR}/clang"
            STRIP_TOP_DIR
        )
        update_content_version("${PRAGMA_BUILD_TOOLS_DIR}/clang" "${version}" "version.json")
    endif()
endfunction()
pr_fetch_clang()
