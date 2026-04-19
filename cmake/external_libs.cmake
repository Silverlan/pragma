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
pr_fetch_external_lib("datasystem"              "https://github.com/Silverlan/datasystem"             "e7c134bdecf8537ce873025ec68d53f4ca7d40ac")
pr_fetch_external_lib("iglfw"                   "https://github.com/Silverlan/iglfw"                  "34b0b1dc1047aed014758ea0a7d85dd3d2267851")
pr_fetch_external_lib("luasystem"               "https://github.com/Silverlan/luasystem"              "e26dfd343eed040d3e5e5faa810ca2aa31ea4df4")
pr_fetch_external_lib("materialsystem"          "https://github.com/Silverlan/materialsystem"         "a494305b543ebf9d26d653f2e17668d9c1659983")
pr_fetch_external_lib("mathutil"                "https://github.com/Silverlan/mathutil"               "416b5094787bf4384965d58326f0bd004a58e90b")
pr_fetch_external_lib("networkmanager"          "https://github.com/Silverlan/networkmanager"         "307e83426280669624ee25f8e3058af96e4e0db3")
pr_fetch_external_lib("oskit"                   "https://github.com/Silverlan/oskit"                  "2c13e0963ef719d8c3a806f5c4ea0e151db58367")
pr_fetch_external_lib("panima"                  "https://github.com/Silverlan/panima"                 "33816fd14ac7996f930587918b17f349992a3f24")
pr_fetch_external_lib("prosper"                 "https://github.com/Silverlan/prosper"                "9e6454f6ff1cff8d94ea48d5f699e3d4ba428dd7")
pr_fetch_external_lib("sharedutils"             "https://github.com/Silverlan/sharedutils"            "707777f783723c45ac9e29a1154999d9e968c6af")
pr_fetch_external_lib("util_bsp"                "https://github.com/Silverlan/util_bsp"               "9eeb876a6fafe1b92966bb8f0fe6bd93ebb7b41f")
pr_fetch_external_lib("util_formatted_text"     "https://github.com/Silverlan/util_formatted_text"    "50bd69695d55a61d9ecab8c3453a88fd8bc17a99")
pr_fetch_external_lib("util_image"              "https://github.com/Silverlan/util_image"             "3bcf960eab9bd9c0149f18551d1dc4de4d83ac01")
pr_fetch_external_lib("util_pad"                "https://github.com/Silverlan/util_pad"               "9f353dbad247e742a67f87b171dd30810f908b66")
pr_fetch_external_lib("util_pragma_doc"         "https://github.com/Silverlan/util_pragma_doc"        "e235cea9acf0e59eefabf7027aff3315753991db")
pr_fetch_external_lib("util_smdmodel"           "https://github.com/Silverlan/util_smdmodel"          "d56004a9920e1477a19d45bdfc76ccfa605caaf0")
pr_fetch_external_lib("util_sound"              "https://github.com/Silverlan/util_sound"             "e1652c5b4bb8a18b9cc545862810959f42f90d13")
pr_fetch_external_lib("util_source2"            "https://github.com/Silverlan/util_source2"           "6afc530201ddeacf7ed1e9d3f25c86969f08e5c9")
pr_fetch_external_lib("util_string"             "https://github.com/Silverlan/util_string"            "ef7bd3436cc9e62e159f858aa30c465bf2d8e882")
pr_fetch_external_lib("util_source_script"      "https://github.com/Silverlan/util_source_script"     "82e7537a2b562d036492568bc61be3e93507c380")
pr_fetch_external_lib("util_timeline_scene"     "https://github.com/Silverlan/util_timeline_scene"    "6092f5ebeef6be95dc9553fe32dd424cc9af92b6")
pr_fetch_external_lib("util_udm"                "https://github.com/Silverlan/util_udm"               "d05d746aa709d58d226d31da58c435b0b861bfc9")
pr_fetch_external_lib("util_versioned_archive"  "https://github.com/Silverlan/util_versioned_archive" "0b0e419a112f654c2fdc27e0660309afc4c581b8")
pr_fetch_external_lib("util_vmf"                "https://github.com/Silverlan/util_vmf"               "346aaeb9446349371010423e9a39a31c7cd35afc")
pr_fetch_external_lib("util_zip"                "https://github.com/Silverlan/util_zip"               "308709b8fc505f01c81b782ab9aed2ff6b0dc210")
pr_fetch_external_lib("util_shadergraph"        "https://github.com/Silverlan/util_shadergraph"       "29af203509275aae73ef65868fa1c136d7fdf750")
pr_fetch_external_lib("vfilesystem"             "https://github.com/Silverlan/vfilesystem"            "2eeaa6ca8da0e17e55bed2f971f474fb2c2a2343")
pr_fetch_external_lib("wgui"                    "https://github.com/Silverlan/wgui"                   "88a5b07cf27f1c25c4e31579d6a9a1e11ab2d862")
pr_fetch_external_lib("util_unicode"            "https://github.com/Silverlan/util_unicode"           "355003f2679e8f80e52026bfd8d3b7e4cb57d982")
pr_fetch_external_lib("cppbezierfit"            "https://github.com/Silverlan/cppbezierfit"           "63dbcaeddba39c80b885fd7eadbbbad560bc1791")
