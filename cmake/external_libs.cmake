function(pr_fetch_external_lib IDENTIFIER GIT_URL GIT_SHA)
	get_property(IS_REGISTERED GLOBAL PROPERTY PR_REG_LIB_${IDENTIFIER})
	if(IS_REGISTERED)
		# Don't register library if it has already been registered
		return()
	endif()

	if(NOT PRAGMA_DISABLE_BUILD_FETCH)
		pr_fetch_repository(${IDENTIFIER} ${GIT_URL} ${GIT_SHA} "external_libs/${IDENTIFIER}")
	else()
		add_subdirectory(
			"${CMAKE_SOURCE_DIR}/external_libs/${IDENTIFIER}"
			"${CMAKE_BINARY_DIR}/external_libs/${IDENTIFIER}"
		)
	endif()

	get_property(_sources GLOBAL PROPERTY PR_FLATPAK_SOURCES)
	string(APPEND _sources "
      - type: git
        url: ${GIT_URL}
        commit: ${GIT_SHA}
        dest: 'pragma/external_libs/${IDENTIFIER}'")
	set_property(GLOBAL PROPERTY PR_FLATPAK_SOURCES "${_sources}")

	if(IDENTIFIER MATCHES "util_*")
		set_target_properties(${IDENTIFIER} PROPERTIES FOLDER external_libs/util)
	else()
		set_target_properties(${IDENTIFIER} PROPERTIES FOLDER external_libs)
	endif()
	set_property(GLOBAL PROPERTY PR_REG_LIB_${IDENTIFIER} TRUE)
endfunction()

pr_fetch_external_lib("alsoundsystem"           "https://github.com/Silverlan/alsoundsystem.git"      "b4a1c80e3952f5be8eaa5a60903d366bfd4fafdf")
pr_fetch_external_lib("datasystem"              "https://github.com/Silverlan/datasystem"             "0feba2d2a0036d58c144ade31a6fe9f49a582626")
pr_fetch_external_lib("iglfw"                   "https://github.com/Silverlan/iglfw"                  "acb72da92badbfd50375ae1fa245f389a3715920")
pr_fetch_external_lib("luasystem"               "https://github.com/Silverlan/luasystem"              "274f3e8d6f1217667d7c1a268d7acfa9a78acc74")
pr_fetch_external_lib("materialsystem"          "https://github.com/Silverlan/materialsystem"         "f118c5a721d48ba85f8827326fa2bd5398eeb3ea")
pr_fetch_external_lib("mathutil"                "https://github.com/Silverlan/mathutil"               "27fa92ba59e79f619f8bb29d8e62b53d1607f4d4")
pr_fetch_external_lib("networkmanager"          "https://github.com/Silverlan/networkmanager"         "307e83426280669624ee25f8e3058af96e4e0db3")
pr_fetch_external_lib("oskit"                   "https://github.com/Silverlan/oskit"                  "2c13e0963ef719d8c3a806f5c4ea0e151db58367")
pr_fetch_external_lib("panima"                  "https://github.com/Silverlan/panima"                 "33816fd14ac7996f930587918b17f349992a3f24")
pr_fetch_external_lib("prosper"                 "https://github.com/Silverlan/prosper"                "d9e4fe85af77ef9967540e44d9ba6fa540bb6cf2")
pr_fetch_external_lib("sharedutils"             "https://github.com/Silverlan/sharedutils"            "8c4646a2fc8414320195c9a19ce7e0ba9baa1fa7")
pr_fetch_external_lib("util_bsp"                "https://github.com/Silverlan/util_bsp"               "9eeb876a6fafe1b92966bb8f0fe6bd93ebb7b41f")
pr_fetch_external_lib("util_formatted_text"     "https://github.com/Silverlan/util_formatted_text"    "50bd69695d55a61d9ecab8c3453a88fd8bc17a99")
pr_fetch_external_lib("util_image"              "https://github.com/Silverlan/util_image"             "61a4b1ebbf98c57cf9cee88c2c3f4cd91165c21e")
pr_fetch_external_lib("util_pad"                "https://github.com/Silverlan/util_pad"               "9f353dbad247e742a67f87b171dd30810f908b66")
pr_fetch_external_lib("util_pragma_doc"         "https://github.com/Silverlan/util_pragma_doc"        "e235cea9acf0e59eefabf7027aff3315753991db")
pr_fetch_external_lib("util_smdmodel"           "https://github.com/Silverlan/util_smdmodel"          "d56004a9920e1477a19d45bdfc76ccfa605caaf0")
pr_fetch_external_lib("util_sound"              "https://github.com/Silverlan/util_sound"             "e1652c5b4bb8a18b9cc545862810959f42f90d13")
pr_fetch_external_lib("util_source2"            "https://github.com/Silverlan/util_source2"           "2a5c5218dd6551ad91557adac2add0aa168e27c7")
pr_fetch_external_lib("util_string"             "https://github.com/Silverlan/util_string"            "50abcced35a5936d7fae2e3fc8962d860f272313")
pr_fetch_external_lib("util_source_script"      "https://github.com/Silverlan/util_source_script"     "82e7537a2b562d036492568bc61be3e93507c380")
pr_fetch_external_lib("util_timeline_scene"     "https://github.com/Silverlan/util_timeline_scene"    "6092f5ebeef6be95dc9553fe32dd424cc9af92b6")
pr_fetch_external_lib("util_udm"                "https://github.com/Silverlan/util_udm"               "3383a555f2ce2fe84103bbcdf6bc24d5113dd50c")
pr_fetch_external_lib("util_versioned_archive"  "https://github.com/Silverlan/util_versioned_archive" "0b0e419a112f654c2fdc27e0660309afc4c581b8")
pr_fetch_external_lib("util_vmf"                "https://github.com/Silverlan/util_vmf"               "346aaeb9446349371010423e9a39a31c7cd35afc")
pr_fetch_external_lib("util_zip"                "https://github.com/Silverlan/util_zip"               "a8b0b3e1e126849b929ea04f4fe9afeb2b40cdc7")
pr_fetch_external_lib("util_shadergraph"        "https://github.com/Silverlan/util_shadergraph"       "f6b39f3d067fb022cc456a979cef6bfe2c031fd4")
pr_fetch_external_lib("vfilesystem"             "https://github.com/Silverlan/vfilesystem"            "9a135bde2c2032f3bd919c1d9637eb9383336c50")
pr_fetch_external_lib("wgui"                    "https://github.com/Silverlan/wgui"                   "2214be1918ac9b2ea878e48f7ad8811a55eadefd")
pr_fetch_external_lib("util_unicode"            "https://github.com/Silverlan/util_unicode"           "5f0519049aecfd52e02ffbabeb84d735d027d9f0")
pr_fetch_external_lib("cppbezierfit"            "https://github.com/Silverlan/cppbezierfit"           "63dbcaeddba39c80b885fd7eadbbbad560bc1791")
