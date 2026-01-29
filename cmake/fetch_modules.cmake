include("cmake/git_helper.cmake")

set(PRAGMA_FETCHED_MODULES "")
function(pr_fetch_module IDENTIFIER GIT_URL GIT_SHA)
    list(APPEND PRAGMA_FETCHED_MODULES "${IDENTIFIER}")
    set(PRAGMA_FETCHED_MODULES "${PRAGMA_FETCHED_MODULES}" PARENT_SCOPE)

    pr_checkout_submodule(${IDENTIFIER} ${GIT_URL} ${GIT_SHA} "modules/${IDENTIFIER}")
endfunction()

pr_fetch_module("interfaces"                      "https://github.com/Silverlan/pragma_interfaces.git"      "42d550ec9450080042d32b6f959e71d1a0bfbec2")

if(PRAGMA_WITH_ESSENTIAL_CLIENT_MODULES)
    pr_fetch_module("pr_prosper_vulkan"           "https://github.com/Silverlan/pr_prosper_vulkan.git"      "de9237173562903f9e3ae06b183505f214d68a60")
    pr_fetch_module("pr_nsight_aftermath"         "https://github.com/Silverlan/pr_nsight_aftermath.git"    "77d54ab8b4aa90362feb58f02d451f91ed91cd2d")
endif()

if(PRAGMA_WITH_COMMON_MODULES)
    pr_fetch_module("pr_bullet"                    "https://github.com/Silverlan/pr_bullet.git"           "31f97cf2535d42fe61964e0767912670ac8fda76")
    pr_fetch_module("pr_audio_soloud"              "https://github.com/Silverlan/pr_soloud.git"           "6e2b627232abc0410d92c614f767ca4759c0104a")
    pr_fetch_module("pr_audio_dummy"               "https://github.com/Silverlan/pr_audio_dummy.git"      "c54a035cf61f4c47f88bd6274e43e54054e6cb21")
    pr_fetch_module("pr_prosper_opengl"            "https://github.com/Silverlan/pr_prosper_opengl.git"   "6d79e77069149593b0dcf59e63084c835f438b65")
endif()

if(PRAGMA_WITH_PFM)
	if(PRAGMA_WITH_CORE_PFM_MODULES OR PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_curl"                  "https://github.com/Silverlan/pr_curl.git"             "ede447cae6703b9ebf226f3dab8d06a8d4b9a021")
        pr_fetch_module("pr_dmx"                   "https://github.com/Silverlan/pr_dmx.git"              "cc40a0769554de7206ba912c23c36845fe7bde5b")
    endif()
	if(PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_chromium"              "https://github.com/Silverlan/pr_chromium.git"         "6b6f67d647556b6a1dd500fd194bb5910a8327af")
        pr_fetch_module("pr_unirender"             "https://github.com/Silverlan/pr_cycles.git"           "ad6c38506c60e077123dd9aa3b53e0322ea517ae")
        pr_fetch_module("pr_xatlas"                "https://github.com/Silverlan/pr_xatlas.git"           "06013606c4efa7a9e7e96fc61af59d142c494daf")
        pr_fetch_module("pr_davinci"               "https://github.com/Silverlan/pr_davinci.git"          "dac2ff0e771ee1ad03490ea11e9b55d6bb9ca6ae")
        pr_fetch_module("pr_opencv"                "https://github.com/Silverlan/pr_opencv.git"           "83e71b956793de6e317f1f2aa960bf5039ec17fe")
    endif()
endif()

if(PRAGMA_WITH_PFM)
    pr_fetch_module("pr_git"                 "https://github.com/Silverlan/pr_git.git"           "d67ae478c835cdb4ad35f12dc7a2ee5c018d7a14")
endif()

if(PRAGMA_WITH_VR)
    pr_fetch_module("pr_openvr"              "https://github.com/Silverlan/pr_openvr.git"        "1f84e606d379e888b53edf5f90861f7a6d27487f")
endif()

if(PRAGMA_WITH_NETWORKING)
    pr_fetch_module("pr_steam_networking_sockets" "https://github.com/Silverlan/pr_steam_networking_sockets.git" "d514677f8598cea34408e59699964ce8f2cb01ac")
endif()

foreach(MODULE_NAME ${PRAGMA_ADDITIONAL_MODULES})
    list(APPEND PRAGMA_FETCHED_MODULES "${MODULE_NAME}")
endforeach()
