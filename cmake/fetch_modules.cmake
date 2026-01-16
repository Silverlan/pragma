include("cmake/git_helper.cmake")

set(PRAGMA_FETCHED_MODULES "")
function(pr_fetch_module IDENTIFIER GIT_URL GIT_SHA)
    list(APPEND PRAGMA_FETCHED_MODULES "${IDENTIFIER}")
    set(PRAGMA_FETCHED_MODULES "${PRAGMA_FETCHED_MODULES}" PARENT_SCOPE)

    pr_checkout_submodule(${IDENTIFIER} ${GIT_URL} ${GIT_SHA} "modules/${IDENTIFIER}")
endfunction()

pr_fetch_module("interfaces"                      "https://github.com/Silverlan/pragma_interfaces.git"      "05cb78dfadbbb7e9fcc3e4b822f207518158eaf1")

if(PRAGMA_WITH_ESSENTIAL_CLIENT_MODULES)
    pr_fetch_module("pr_prosper_vulkan"           "https://github.com/Silverlan/pr_prosper_vulkan.git"      "1a15646981268a4e6222eeca43bc73c1e8f4d1e9")
endif()

if(PRAGMA_WITH_COMMON_MODULES)
    pr_fetch_module("pr_bullet"                    "https://github.com/Silverlan/pr_bullet.git"           "094fe87b217a9985fdc3cdce29f614b2d469f293")
    pr_fetch_module("pr_audio_soloud"              "https://github.com/Silverlan/pr_soloud.git"           "a7c91941de48d63e4ec06444f714240f46c0ced8")
    pr_fetch_module("pr_audio_dummy"               "https://github.com/Silverlan/pr_audio_dummy.git"      "c95ca9ecdd7a33397a5253343117f4e242c45928")
    pr_fetch_module("pr_prosper_opengl"            "https://github.com/Silverlan/pr_prosper_opengl.git"   "ecfcaad223adeae285b7eea0ca3e70ea46af50db")
endif()

if(PRAGMA_WITH_PFM)
	if(PRAGMA_WITH_CORE_PFM_MODULES OR PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_curl"                  "https://github.com/Silverlan/pr_curl.git"             "41d4f9d39d18cfca5ff0af1a310c0bf4f01d5adc")
        pr_fetch_module("pr_dmx"                   "https://github.com/Silverlan/pr_dmx.git"              "6b919c1af80c4ed2ded5b41784b84184ceb754ac")
    endif()
	if(PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_chromium"              "https://github.com/Silverlan/pr_chromium.git"         "1477b93859c9ae5058c66a6b436ac24dd03ebff7")
        pr_fetch_module("pr_unirender"             "https://github.com/Silverlan/pr_cycles.git"           "d42e19085041bb8b91175e8289ef6c43c7365535")
        pr_fetch_module("pr_xatlas"                "https://github.com/Silverlan/pr_xatlas.git"           "5d7ff7f4728ed5bf1290757b161df4bb3ef58b69")
        pr_fetch_module("pr_davinci"               "https://github.com/Silverlan/pr_davinci.git"          "c09482375a0ff1098283513b6d7e06a67b30b9f9")
        pr_fetch_module("pr_opencv"                "https://github.com/Silverlan/pr_opencv.git"           "6899a8504225e232db73a4f9be514fcb889a14f9")
    endif()
endif()

if(PRAGMA_WITH_PFM)
    pr_fetch_module("pr_git"                 "https://github.com/Silverlan/pr_git.git"           "5af153f8d5c3d969f3df2cdd05f2929e680845aa")
endif()

if(PRAGMA_WITH_VR)
    pr_fetch_module("pr_openvr"              "https://github.com/Silverlan/pr_openvr.git"        "91c3d5175bb229c9afc0b02a91256d3673dfc221")
endif()

if(PRAGMA_WITH_NETWORKING)
    pr_fetch_module("pr_steam_networking_sockets" "https://github.com/Silverlan/pr_steam_networking_sockets.git" "6775da3dc7d3cda4fbd46ef7c35c220ab5426fd0")
endif()

foreach(MODULE_NAME ${PRAGMA_ADDITIONAL_MODULES})
    pr_add_module(${MODULE_NAME})
endforeach()
