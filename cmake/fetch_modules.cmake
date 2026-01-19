include("cmake/git_helper.cmake")

set(PRAGMA_FETCHED_MODULES "")
function(pr_fetch_module IDENTIFIER GIT_URL GIT_SHA)
    list(APPEND PRAGMA_FETCHED_MODULES "${IDENTIFIER}")
    set(PRAGMA_FETCHED_MODULES "${PRAGMA_FETCHED_MODULES}" PARENT_SCOPE)

    pr_checkout_submodule(${IDENTIFIER} ${GIT_URL} ${GIT_SHA} "modules/${IDENTIFIER}")
endfunction()

pr_fetch_module("interfaces"                      "https://github.com/Silverlan/pragma_interfaces.git"      "eb90acc68573c1c6b83f8d0a405e070ce3690986")

if(PRAGMA_WITH_ESSENTIAL_CLIENT_MODULES)
    pr_fetch_module("pr_prosper_vulkan"           "https://github.com/Silverlan/pr_prosper_vulkan.git"      "8a064d1bb006959684e1e0d7e2e97f0c09bdfbab")
    pr_fetch_module("pr_nsight_aftermath"         "https://github.com/Silverlan/pr_nsight_aftermath.git"    "52231161cfd48511ca797e266a1e7cf684b1806b")
endif()

if(PRAGMA_WITH_COMMON_MODULES)
    pr_fetch_module("pr_bullet"                    "https://github.com/Silverlan/pr_bullet.git"           "f579e156daeec2094c05ec702af5a8d31fd902be")
    pr_fetch_module("pr_audio_soloud"              "https://github.com/Silverlan/pr_soloud.git"           "36763abe39d597c65b82f8747171f8a38650e342")
    pr_fetch_module("pr_audio_dummy"               "https://github.com/Silverlan/pr_audio_dummy.git"      "b49c0c3621e0d97e8702ce33abda5d3364f3d9c1")
    pr_fetch_module("pr_prosper_opengl"            "https://github.com/Silverlan/pr_prosper_opengl.git"   "ecfcaad223adeae285b7eea0ca3e70ea46af50db")
endif()

if(PRAGMA_WITH_PFM)
	if(PRAGMA_WITH_CORE_PFM_MODULES OR PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_curl"                  "https://github.com/Silverlan/pr_curl.git"             "41d4f9d39d18cfca5ff0af1a310c0bf4f01d5adc")
        pr_fetch_module("pr_dmx"                   "https://github.com/Silverlan/pr_dmx.git"              "6b919c1af80c4ed2ded5b41784b84184ceb754ac")
    endif()
	if(PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_chromium"              "https://github.com/Silverlan/pr_chromium.git"         "5a0e85d49b8db8c3a56c88ccdc65ee796f721dc2")
        pr_fetch_module("pr_unirender"             "https://github.com/Silverlan/pr_cycles.git"           "421c8ba37edcea5f36a43a5998cbfca6180ae148")
        pr_fetch_module("pr_xatlas"                "https://github.com/Silverlan/pr_xatlas.git"           "e7e77b6af5bdc518ea4e81d23a869d43fbfa2a25")
        pr_fetch_module("pr_davinci"               "https://github.com/Silverlan/pr_davinci.git"          "be8461e71ea5d01fdd59c9bc9fa91994387c009c")
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
    pr_fetch_module("pr_steam_networking_sockets" "https://github.com/Silverlan/pr_steam_networking_sockets.git" "47f09fe87964703c209ddb987aa59266916ce100")
endif()

foreach(MODULE_NAME ${PRAGMA_ADDITIONAL_MODULES})
    list(APPEND PRAGMA_FETCHED_MODULES "${MODULE_NAME}")
endforeach()
