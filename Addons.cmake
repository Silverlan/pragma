include(ExternalProject)

function(pr_install_git_repository IDENTIFIER GIT_URL GIT_SHA INSTALL_PATH)
    set(ADDON_SHA   "${GIT_SHA}")
    set(ADDON_URL   "${GIT_URL}")
    set(ADDON_PREF  "${CMAKE_BINARY_DIR}/_deps/${IDENTIFIER}-prefix")

    set(FLATPAK_DEST_DIR "_deps/addon_${IDENTIFIER}")
    get_property(_sources GLOBAL PROPERTY PR_FLATPAK_SOURCES)
    string(APPEND _sources "
      - type: git
        url: ${GIT_URL}
        commit: ${GIT_SHA}
        dest: '${FLATPAK_DEST_DIR}'")
    set_property(GLOBAL PROPERTY PR_FLATPAK_SOURCES "${_sources}")

    if(NOT PRAGMA_DISABLE_BUILD_FETCH)
        include(ExternalProject)
        ExternalProject_Add(addon_${IDENTIFIER}
            PREFIX         ${ADDON_PREF}
            GIT_REPOSITORY ${ADDON_URL}
            GIT_TAG        ${ADDON_SHA}

            CONFIGURE_COMMAND ""
            BUILD_COMMAND     ""
            INSTALL_COMMAND   ""
        )
        # ExternalProject_Get_Property(addon_${IDENTIFIER} source_dir)
        set(_source_dir "${ADDON_PREF}/src/addon_${IDENTIFIER}")
        
        add_dependencies(pragma-install-full addon_${IDENTIFIER})
        add_dependencies(pragma-install-full-no-executable addon_${IDENTIFIER})
    else()
        set(_source_dir "${CMAKE_SOURCE_DIR}/${FLATPAK_DEST_DIR}")
    endif()
    pr_manifest_install(${IDENTIFIER} "${_source_dir}" "${INSTALL_PATH}" pragma-install-full)
    pr_manifest_install(${IDENTIFIER} "${_source_dir}" "${INSTALL_PATH}" pragma-install-full-no-executable)
endfunction()

function(fetch_checksum BASE_URL TAG_NAME TARGET_ASSET_NAME OUT_CHECKSUM)
    set(URL "https://api.github.com/repos/${BASE_URL}/releases/tags/${TAG_NAME}")
    file(DOWNLOAD
        "${URL}"
        "${CMAKE_BINARY_DIR}/release_info.json"
    )
    file(READ "${CMAKE_BINARY_DIR}/release_info.json" RELEASE_JSON)

    if("${RELEASE_JSON}" STREQUAL "")
        message(FATAL_ERROR "Failed to fetch release info from ${URL} .")
    endif()

    string(JSON ASSETS_JSON GET "${RELEASE_JSON}" "assets")
    string(JSON ASSET_COUNT LENGTH "${RELEASE_JSON}" "assets")

    math(EXPR LAST_INDEX "${ASSET_COUNT} - 1")
    foreach(i RANGE ${LAST_INDEX})
        string(JSON ASSET GET "${RELEASE_JSON}" "assets" ${i})
        string(JSON ASSET_NAME GET "${ASSET}" "name")
        if(ASSET_NAME STREQUAL TARGET_ASSET_NAME)
            string(JSON ASSET_DIGEST GET "${ASSET}" "digest")
            break()
        endif()
    endforeach()
    string(REGEX REPLACE "^[^:]+:" "" ASSET_DIGEST "${ASSET_DIGEST}")
    set(${OUT_CHECKSUM} "${ASSET_DIGEST}" PARENT_SCOPE)
endfunction()

function(pr_install_git_release IDENTIFIER BASE_URL BASE_DIR TAG_NAME)
    if(UNIX)
        set(ARCH_OS "linux")
        set(ARCH_EXT "tar.gz")
    else()
        set(ARCH_OS "windows")
        set(ARCH_EXT "zip")
    endif()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(COMPILER_NAME "clang")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(COMPILER_NAME "gcc")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(COMPILER_NAME "msvc")
    endif()

    set(ARCH_FILE_NAME "binaries-${ARCH_OS}-x64-${COMPILER_NAME}.${ARCH_EXT}")

    set(ARCHIVE_URL "https://github.com/${BASE_URL}/releases/download/${TAG_NAME}/${ARCH_FILE_NAME}")

    set(ADDON_URL   "${ARCHIVE_URL}")
    set(ADDON_PREF  "${CMAKE_BINARY_DIR}/_deps/${IDENTIFIER}-prefix")

    if(NOT PRAGMA_DISABLE_BUILD_FETCH)
        fetch_checksum(${BASE_URL} ${TAG_NAME} ${ARCH_FILE_NAME} CHECKSUM)
        set(FLATPAK_DEST_DIR "_deps/addon_${IDENTIFIER}")
        get_property(_sources GLOBAL PROPERTY PR_FLATPAK_SOURCES)
        string(APPEND _sources "
      - type: archive
        url: ${ADDON_URL}
        sha256: ${CHECKSUM}
        dest: '${FLATPAK_DEST_DIR}'")
        set_property(GLOBAL PROPERTY PR_FLATPAK_SOURCES "${_sources}")

        include(ExternalProject)
        ExternalProject_Add(addon_${IDENTIFIER}
            PREFIX            ${ADDON_PREF}
            SOURCE_DIR "${ADDON_PREF}/src/addon_${IDENTIFIER}/${BASE_DIR}"
            URL    ${ADDON_URL}

            CONFIGURE_COMMAND ""
            BUILD_COMMAND     ""
            INSTALL_COMMAND   ""
        )
        # ExternalProject_Get_Property(addon_${IDENTIFIER} source_dir)

        set(_source_dir "${ADDON_PREF}/src/addon_${IDENTIFIER}")
        
        add_dependencies(pragma-install-full addon_${IDENTIFIER})
        add_dependencies(pragma-install-full-no-executable addon_${IDENTIFIER})
    else()
        set(_source_dir "${CMAKE_SOURCE_DIR}/${FLATPAK_DEST_DIR}")
    endif()
    pr_manifest_install(${IDENTIFIER} "${ADDON_PREF}/src/addon_${IDENTIFIER}" "." pragma-install-full)
    pr_manifest_install(${IDENTIFIER} "${ADDON_PREF}/src/addon_${IDENTIFIER}" "." pragma-install-full-no-executable)
endfunction()

if(PRAGMA_WITH_PFM)
    pr_install_git_repository(pfm "https://github.com/Silverlan/pfm.git" "31989a6aede33d1b481125b6cf29673ccf22a616" "addons/filmmaker")
    pr_install_git_repository(model_editor "https://github.com/Silverlan/pragma_model_editor.git" "a815746863561ad4d5261319319d7ea685bd4e29" "addons/tool_model_editor")

    pr_install_git_repository(pfm_demo_living_room "https://github.com/Silverlan/pfm_demo_living_room.git" "4cbecad4a2d6f502b6d9709178883678101f7e2c" "addons/pfm_demo_living_room")
    pr_install_git_repository(pfm_demo_bedroom "https://github.com/Silverlan/pfm_demo_bedroom.git" "0fed1d5b54a25c3ded2ce906e7da80ca8dd2fb0d" "addons/pfm_demo_bedroom")
    pr_install_git_repository(pfm_tutorials "https://github.com/Silverlan/pfm_tutorials.git" "2828e3d811e0d90446b3722195440ac6b5dd0616" "addons/pfm_tutorials")
endif()
if(PRAGMA_WITH_VR)
    pr_install_git_repository(virtual_reality "https://github.com/Silverlan/PragmaVR.git" "158207111903a344378505c8269d2085271d0a51" "addons/virtual_reality")
endif()
if(PRAGMA_WITH_COMMON_ENTITIES)
    pr_install_git_repository(pragma_hl "https://github.com/Silverlan/pragma_hl.git" "4f42a0ab6b71b3b5b458bffca6cdc0fcb0de83ea" "addons/pragma_hl")
    pr_install_git_repository(pragma_tf2 "https://github.com/Silverlan/pragma_tf2.git" "1b36edc5804c87fde129e94c38fa52b2fd845e42" "addons/pragma_tf2")
endif()

pr_install_git_repository(matcaps "https://github.com/Silverlan/pragma_matcaps.git" "304800623fc6c07901053fb41428fa2c430f2d0d" "addons/matcaps")

if(PRAGMA_WITH_COMMON_MODULES)
    # Unfortunately ExternalProject_Add strips the first component of the tar.gz and there is no way to disable that behavior.
    # For this reason we have to re-add the root folder as argument here.
    if(NOT TARGET pr_mount_external)
        pr_install_git_release(mount_external "Silverlan/pr_mount_external_prebuilt" "modules" "2026-05-23")
    endif()
    if(NOT TARGET pr_rig)
        pr_install_git_release(rig "Silverlan/pr_rig_prebuilt" "addons" "2026-05-23")
    endif()
    if(NOT TARGET pr_ik)
        pr_install_git_release(ik "Silverlan/pr_ik_prebuilt" "modules" "2026-05-23")
    endif()
endif()
