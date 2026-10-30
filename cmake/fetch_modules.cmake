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

pr_fetch_module("interfaces"                      "https://github.com/Silverlan/pragma_interfaces.git"      "4b9577e96e807f7a60846bb848569679d6e84cf1")

if(PRAGMA_WITH_ESSENTIAL_CLIENT_MODULES)
    pr_fetch_module("pr_prosper_vulkan"           "https://github.com/Silverlan/pr_prosper_vulkan.git"      "d31cd9143cbc7a2f73516a7be278bfe9f1fc6c14")
    pr_fetch_module("pr_nsight_aftermath"         "https://github.com/Silverlan/pr_nsight_aftermath.git"    "66615caf80d9bff53d8347055a8c00d820fdd585")
endif()

if(PRAGMA_WITH_COMMON_MODULES)
    pr_fetch_module("pr_bullet"                    "https://github.com/Silverlan/pr_bullet.git"           "a1097f685ba2d18cb4397c48d17f3e7615b376db")
    pr_fetch_module("pr_audio_soloud"              "https://github.com/Silverlan/pr_soloud.git"           "9bb3405fb73033402ae12b5bde09c5deb8de295c")
    pr_fetch_module("pr_audio_dummy"               "https://github.com/Silverlan/pr_audio_dummy.git"      "c54a035cf61f4c47f88bd6274e43e54054e6cb21")
    pr_fetch_module("pr_prosper_opengl"            "https://github.com/Silverlan/pr_prosper_opengl.git"   "198a8e10c718e098dd14a72e54709ec5599d0a8a")
endif()

if(PRAGMA_WITH_PFM)
	if(PRAGMA_WITH_CORE_PFM_MODULES OR PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_curl"                  "https://github.com/Silverlan/pr_curl.git"             "08f98d87892e73efcd0fbe3b91c4ff322f041e77")
        pr_fetch_module("pr_dmx"                   "https://github.com/Silverlan/pr_dmx.git"              "7e11da85b3c8d14fe90c32252dae2cf456982bcb")
    endif()
	if(PRAGMA_WITH_ALL_PFM_MODULES)
        pr_fetch_module("pr_chromium"              "https://github.com/Silverlan/pr_chromium.git"         "c87dcc80b1d04fa8277b4f18b6ded5ac8899883e")
        pr_fetch_module("pr_unirender"             "https://github.com/Silverlan/pr_cycles.git"           "1fc47fb757ae9f3ec9f2f490a31aa45b0301cd47")
        pr_fetch_module("pr_xatlas"                "https://github.com/Silverlan/pr_xatlas.git"           "06013606c4efa7a9e7e96fc61af59d142c494daf")
        pr_fetch_module("pr_davinci"               "https://github.com/Silverlan/pr_davinci.git"          "8235a3e9159cb7fe7861da91dcc8f235af1a88a9")
        pr_fetch_module("pr_opencv"                "https://github.com/Silverlan/pr_opencv.git"           "d03fd8b40e81ec4874f28bc4d6471844b1f5681b")
        pr_fetch_module("pr_servo"                 "https://github.com/Silverlan/pr_servo.git"            "c80a34dab9581e44433ed92994b7c86b9b55b807")
    endif()
endif()

if(PRAGMA_WITH_PFM)
    pr_fetch_module("pr_git"                 "https://github.com/Silverlan/pr_git.git"           "d67ae478c835cdb4ad35f12dc7a2ee5c018d7a14")
endif()

if(PRAGMA_WITH_VR)
    pr_fetch_module("pr_openvr"              "https://github.com/Silverlan/pr_openvr.git"        "1f84e606d379e888b53edf5f90861f7a6d27487f")
endif()

if(PRAGMA_WITH_NETWORKING)
    pr_fetch_module("pr_steam_networking_sockets" "https://github.com/Silverlan/pr_steam_networking_sockets.git" "584e4190b345d89bf69b8bc89b85f49f3e0b7864")
endif()

foreach(MODULE_NAME ${PRAGMA_ADDITIONAL_MODULES})
    list(APPEND PRAGMA_FETCHED_MODULES "${MODULE_NAME}")
endforeach()
