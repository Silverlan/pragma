set_property(GLOBAL PROPERTY PRAGMA_MODULE_NAMES "")
set_property(GLOBAL PROPERTY PRAGMA_MODULE_TARGETS "")
function(pr_fetch_module IDENTIFIER GIT_URL GIT_SHA)
    set(MODULE_PATH "modules/${IDENTIFIER}")
    get_filename_component(MODULE_NAME ${MODULE_PATH} NAME)

    get_property(global_names GLOBAL PROPERTY PRAGMA_MODULE_NAMES)
    set(global_names ${global_names} ${IDENTIFIER})
    set_property(GLOBAL PROPERTY PRAGMA_MODULE_NAMES ${global_names})

    set_property(GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_TARGETS "")
    set_property(GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_SKIP_TARGET_PROPERTY_FOLDER 0)
    set_property(GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_SKIP_INSTALL 0)
    
    pr_fetch_repository(${IDENTIFIER} ${GIT_URL} ${GIT_SHA} "${MODULE_PATH}")
    
    get_property(PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_SKIP_TARGET_PROPERTY_FOLDER)
    get_property(_targets GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_TARGETS)

    if (NOT _targets)
        set(_targets ${IDENTIFIER})
    endif()

    set_property(GLOBAL PROPERTY PRAGMA_MODULE_${IDENTIFIER}_TARGETS ${_targets})

    get_property(global_targets GLOBAL PROPERTY PRAGMA_MODULE_TARGETS)
    set(global_targets ${global_targets} ${_targets})
    set_property(GLOBAL PROPERTY PRAGMA_MODULE_TARGETS ${global_targets})
    
    set(MODULE_TARGETS ${MODULE_TARGETS} ${_targets} PARENT_SCOPE)

    if (PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER EQUAL 0)
        if(NOT "${MODULE_NAME}" STREQUAL "interfaces")
            if(TARGET ${MODULE_NAME})
                set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules)
            endif()
        endif()
    endif()
endfunction()

pr_fetch_module("interfaces"                      "https://github.com/Silverlan/pragma_interfaces.git"      "05cb78dfadbbb7e9fcc3e4b822f207518158eaf1")

if(WITH_ESSENTIAL_CLIENT_MODULES)
    pr_fetch_module("pr_prosper_vulkan"           "https://github.com/Silverlan/pr_prosper_vulkan.git"      "1a15646981268a4e6222eeca43bc73c1e8f4d1e9")
endif()

if(WITH_COMMON_MODULES)
    pr_fetch_module("pr_bullet"                    "https://github.com/Silverlan/pr_bullet.git"           "094fe87b217a9985fdc3cdce29f614b2d469f293")
    pr_fetch_module("pr_audio_soloud"              "https://github.com/Silverlan/pr_soloud.git"           "a7c91941de48d63e4ec06444f714240f46c0ced8")
    pr_fetch_module("pr_audio_dummy"               "https://github.com/Silverlan/pr_audio_dummy.git"      "c95ca9ecdd7a33397a5253343117f4e242c45928")
    pr_fetch_module("pr_prosper_opengl"            "https://github.com/Silverlan/pr_prosper_opengl.git"   "ecfcaad223adeae285b7eea0ca3e70ea46af50db")
endif()

if(WITH_PFM)
	if(WITH_CORE_PFM_MODULES OR WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_curl"                  "https://github.com/Silverlan/pr_curl.git"             "41d4f9d39d18cfca5ff0af1a310c0bf4f01d5adc")
        pr_fetch_module("pr_dmx"                   "https://github.com/Silverlan/pr_dmx.git"              "6b919c1af80c4ed2ded5b41784b84184ceb754ac")
    endif()
	if(WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_chromium"              "https://github.com/Silverlan/pr_chromium.git"         "1477b93859c9ae5058c66a6b436ac24dd03ebff7")
        pr_fetch_module("pr_unirender"             "https://github.com/Silverlan/pr_cycles.git"           "a74b751aeb873a62139fff50db2af1488bd903ac")
        pr_fetch_module("pr_xatlas"                "https://github.com/Silverlan/pr_xatlas.git"           "5d7ff7f4728ed5bf1290757b161df4bb3ef58b69")
        pr_fetch_module("pr_davinci"               "https://github.com/Silverlan/pr_davinci.git"          "c09482375a0ff1098283513b6d7e06a67b30b9f9")
        pr_fetch_module("pr_opencv"                "https://github.com/Silverlan/pr_opencv.git"           "6899a8504225e232db73a4f9be514fcb889a14f9")
    endif()
endif()

if(WITH_PFM)
    pr_fetch_module("pr_git"                 "https://github.com/Silverlan/pr_git.git"           "5af153f8d5c3d969f3df2cdd05f2929e680845aa")
endif()

if(WITH_VR)
    pr_fetch_module("pr_openvr"              "https://github.com/Silverlan/pr_openvr.git"        "91c3d5175bb229c9afc0b02a91256d3673dfc221")
endif()

if(WITH_NETWORKING)
    pr_fetch_module("pr_steam_networking_sockets" "https://github.com/Silverlan/pr_steam_networking_sockets.git" "a98cbb8c8a9166e63176d0dfe89f39fef1bf2181")
endif()
