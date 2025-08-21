include(ExternalProject)

function(pr_install_git_repository IDENTIFIER GIT_URL GIT_SHA INSTALL_PATH)
    set(ADDON_SHA   "${GIT_SHA}")
    set(ADDON_URL   "${GIT_URL}")
    set(ADDON_SRC   "${CMAKE_BINARY_DIR}/_deps/${IDENTIFIER}-src")
    set(ADDON_BUILD "${CMAKE_BINARY_DIR}/_deps/${IDENTIFIER}-build")
    set(ADDON_PREF  "${CMAKE_BINARY_DIR}/_deps/${IDENTIFIER}-prefix")

    include(ExternalProject)
    ExternalProject_Add(addon_${IDENTIFIER}
        PREFIX         ${ADDON_PREF}
        GIT_REPOSITORY ${ADDON_URL}
        GIT_TAG        ${ADDON_SHA}

        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND   ""
    )
    ExternalProject_Get_Property(addon_${IDENTIFIER} source_dir)
    add_dependencies(pragma-install-full addon_${IDENTIFIER})
    install(
        DIRECTORY "${source_dir}/"
        DESTINATION "${INSTALL_PATH}"
        COMPONENT   pragma-install-full
    )
endfunction()

function(pr_install_git_release IDENTIFIER BASE_URL BASE_DIR TAG_NAME)
    if(UNIX)
        set(ARCH_FILE_NAME "binaries_linux64.tar.gz")
    else()
        set(ARCH_FILE_NAME "binaries_windows64.zip")
    endif()

    set(ARCHIVE_URL "https://github.com/${BASE_URL}/releases/download/${TAG_NAME}/${ARCH_FILE_NAME}")

    set(ADDON_URL   "${ARCHIVE_URL}")
    set(ADDON_SRC   "${CMAKE_BINARY_DIR}/_deps/${IDENTIFIER}-src")
    set(ADDON_BUILD "${CMAKE_BINARY_DIR}/_deps/${IDENTIFIER}-build")
    set(ADDON_PREF  "${CMAKE_BINARY_DIR}/_deps/${IDENTIFIER}-prefix")

    include(ExternalProject)
    ExternalProject_Add(addon_${IDENTIFIER}
        PREFIX            ${ADDON_PREF}
        SOURCE_DIR "${ADDON_PREF}/src/addon_${IDENTIFIER}/${BASE_DIR}"
        URL    ${ADDON_URL}

        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND   ""
    )
    ExternalProject_Get_Property(addon_${IDENTIFIER} source_dir)
    add_dependencies(pragma-install-full addon_${IDENTIFIER})
    install(
        DIRECTORY "${source_dir}/"
        DESTINATION "${BASE_DIR}"
        COMPONENT   pragma-install-full
    )
endfunction()

if(WITH_PFM)
    pr_install_git_repository(pfm "https://github.com/Silverlan/pfm.git" "bbee9c06540c7411da8f201489f92eeae236c518" "addons/filmmaker")
    pr_install_git_repository(model_editor "https://github.com/Silverlan/pragma_model_editor.git" "3147f31cf902f718255a4e42ef95b33184e8f88b" "addons/tool_model_editor")

    pr_install_git_repository(pfm_demo_living_room "https://github.com/Silverlan/pfm_demo_living_room.git" "4cbecad4a2d6f502b6d9709178883678101f7e2c" "addons/pfm_demo_living_room")
    pr_install_git_repository(pfm_demo_bedroom "https://github.com/Silverlan/pfm_demo_bedroom.git" "0fed1d5b54a25c3ded2ce906e7da80ca8dd2fb0d" "addons/pfm_demo_bedroom")
    pr_install_git_repository(pfm_tutorials "https://github.com/Silverlan/pfm_tutorials.git" "86e567e8ef3faa0fff0ceea601871b11ac028b78" "addons/pfm_tutorials")
endif()
if(WITH_VR)
    pr_install_git_repository(virtual_reality "https://github.com/Silverlan/PragmaVR.git" "2d5360ac6cafefaecb5bf2b5078bad191e7ecc57" "addons/virtual_reality")
endif()
if(WITH_COMMON_ENTITIES)
    pr_install_git_repository(pragma_hl "https://github.com/Silverlan/pragma_hl.git" "4f42a0ab6b71b3b5b458bffca6cdc0fcb0de83ea" "addons/pragma_hl")
    pr_install_git_repository(pragma_tf2 "https://github.com/Silverlan/pragma_tf2.git" "1b36edc5804c87fde129e94c38fa52b2fd845e42" "addons/pragma_tf2")
endif()

pr_install_git_repository(matcaps "https://github.com/Silverlan/pragma_matcaps.git" "304800623fc6c07901053fb41428fa2c430f2d0d" "addons/matcaps")

if(WITH_COMMON_MODULES)
    # Unfortunately ExternalProject_Add strips the first component of the tar.gz and there is no way to disable that behavior.
    # For this reason we have to re-add the root folder as argument here.
    if(NOT TARGET pr_mount_external)
        pr_install_git_release(mount_external "Silverlan/pr_mount_external_prebuilt" "modules" "2025-08-21")
    endif()
    if(NOT TARGET pr_rig)
        pr_install_git_release(rig "Silverlan/pr_rig_prebuilt" "addons" "2025-08-21")
    endif()
    if(NOT TARGET pr_ik)
        pr_install_git_release(ik "Silverlan/pr_ik_prebuilt" "modules" "2025-08-21")
    endif()
endif()
