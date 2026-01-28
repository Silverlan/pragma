include("cmake/install_helper.cmake")

# We need libstdc++.modules.json which is not shipped until gcc-16. Since some major distributions do not ship gcc-16 yet, we'll
# use our own prebuilt version for now.
function(pr_fetch_gcc)
    set(version "c3fa37ceeb5-releases-gcc-15")
    set(url "https://github.com/Silverlan/gcc_prebuilt/releases/download/${version}/gcc-15.2.1-releases-gcc-15.tar.xz")
    check_content_version("${PRAGMA_BUILD_TOOLS_DIR}/gcc" "${version}" "version.json" IS_VALID)
    if(NOT IS_VALID)
        clear_content("${PRAGMA_BUILD_TOOLS_DIR}/gcc")
        pr_http_extract(
            "${url}"
            "${PRAGMA_BUILD_TOOLS_DIR}/gcc"
            STRIP_TOP_DIR
        )
        update_content_version("${PRAGMA_BUILD_TOOLS_DIR}/gcc" "${version}" "version.json")
    endif()
endfunction()
pr_fetch_gcc()
