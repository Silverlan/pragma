include("cmake/git_helper.cmake")

set(PRAGMA_FETCHED_MODULES "")
function(pr_fetch_module IDENTIFIER GIT_URL GIT_SHA)
    list(APPEND PRAGMA_FETCHED_MODULES "${IDENTIFIER}")
    set(PRAGMA_FETCHED_MODULES "${PRAGMA_FETCHED_MODULES}" PARENT_SCOPE)

    if(NOT PRAGMA_DISABLE_BUILD_FETCH)
        pr_checkout_submodule(${IDENTIFIER} ${GIT_URL} ${GIT_SHA} "modules/${IDENTIFIER}")
    endif()

    file(RELATIVE_PATH _relative_dir "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    get_property(_sources GLOBAL PROPERTY PR_FLATPAK_SOURCES)
    string(APPEND _sources "
      - type: git
        url: ${GIT_URL}
        commit: ${GIT_SHA}
        dest: 'pragma/${_relative_dir}/modules/${IDENTIFIER}'")
    set_property(GLOBAL PROPERTY PR_FLATPAK_SOURCES "${_sources}")
endfunction()

pr_fetch_module("interfaces"                      "https://github.com/Silverlan/pragma_interfaces.git"      "adda2aba1fbf570cf5f251ec5c1a503bab89544b")

if(PRAGMA_WITH_ESSENTIAL_CLIENT_MODULES)
    pr_fetch_module("pr_prosper_vulkan"           "https://github.com/Silverlan/pr_prosper_vulkan.git"      "793beccd231221af3a1576ff7550163c92103810")
    pr_fetch_module("pr_nsight_aftermath"         "https://github.com/Silverlan/pr_nsight_aftermath.git"    "66615caf80d9bff53d8347055a8c00d820fdd585")
endif()

if(PRAGMA_WITH_COMMON_MODULES)
    pr_fetch_module("pr_bullet"                    "https://github.com/Silverlan/pr_bullet.git"           "a1097f685ba2d18cb4397c48d17f3e7615b376db")
    pr_fetch_module("pr_audio_soloud"              "https://github.com/Silverlan/pr_soloud.git"           "6e2b627232abc0410d92c614f767ca4759c0104a")
    pr_fetch_module("pr_audio_dummy"               "https://github.com/Silverlan/pr_audio_dummy.git"      "c54a035cf61f4c47f88bd6274e43e54054e6cb21")
    pr_fetch_module("pr_prosper_opengl"            "https://github.com/Silverlan/pr_prosper_opengl.git"   "c2fe8c95934c4728c5379df848585d36e93122cb")
endif()

if(PRAGMA_WITH_PFM)
	if(PRAGMA_WITH_CORE_PFM_MODULES OR PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_curl"                  "https://github.com/Silverlan/pr_curl.git"             "ede447cae6703b9ebf226f3dab8d06a8d4b9a021")
        pr_fetch_module("pr_dmx"                   "https://github.com/Silverlan/pr_dmx.git"              "60477b5c8aca757af16f531258b76b50e9887798")
    endif()
	if(PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_chromium"              "https://github.com/Silverlan/pr_chromium.git"         "723835a5deea6fbbe3500803fd026f88729bccaf")
        pr_fetch_module("pr_unirender"             "https://github.com/Silverlan/pr_cycles.git"           "dafd159b0daf1baf00bdc42cb823faaa7041f041")
        pr_fetch_module("pr_xatlas"                "https://github.com/Silverlan/pr_xatlas.git"           "06013606c4efa7a9e7e96fc61af59d142c494daf")
        pr_fetch_module("pr_davinci"               "https://github.com/Silverlan/pr_davinci.git"          "8235a3e9159cb7fe7861da91dcc8f235af1a88a9")
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
    pr_fetch_module("pr_steam_networking_sockets" "https://github.com/Silverlan/pr_steam_networking_sockets.git" "dab2fe7653e637e1b5c24f0eb665ca48b245ba4d")
endif()

foreach(MODULE_NAME ${PRAGMA_ADDITIONAL_MODULES})
    list(APPEND PRAGMA_FETCHED_MODULES "${MODULE_NAME}")
endforeach()
