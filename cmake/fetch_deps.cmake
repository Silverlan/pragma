include("cmake/install_helper.cmake")

function(pr_get_deps version)    
    check_content_version("${PRAGMA_DEPS_DIR}" "${version}-${TOOLSET}" "version.json" IS_VALID)
    if(NOT IS_VALID)
        clear_content("${PRAGMA_DEPS_DIR}")
        update_content_version("${PRAGMA_DEPS_DIR}" "${version}-${TOOLSET}" "version.json")
        if(UNIX)
            set(platform_name "linux")
            set(prebuilt_archive_format "tar.gz")
        else()
            set(platform_name "windows")
            set(prebuilt_archive_format "zip")
        endif()

        set(prebuilt_archive_name "lib-${platform_name}-x64-${TOOLSET}.${prebuilt_archive_format}")
        pr_http_extract(
            "https://github.com/Silverlan/pragma-deps-lib/releases/download/${version}/${prebuilt_archive_name}"
            "${PRAGMA_DEPS_DIR}"
        )
    endif()
endfunction()
pr_get_deps("2026-01-19")
