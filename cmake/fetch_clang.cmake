include("cmake/install_helper.cmake")

# We need the very latest version of clang available, so for the time being, we're downloading the prebuilt version.
# Since we can't actually detect which compiler is being used at this point in time, we'll always download clang, even if it's not needed.
function(pr_fetch_clang)
    if(WIN32)
        set(version "21.1.8")
        set(url "https://github.com/llvm/llvm-project/releases/download/llvmorg-21.1.8/clang+llvm-21.1.8-x86_64-pc-windows-msvc.tar.xz")
    else()
        set(version "2025-12-19")
        set(url "https://github.com/Silverlan/clang_prebuilt/releases/download/2025-12-19/linux_x64.tar.xz")
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
