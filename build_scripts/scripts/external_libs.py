import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_gh_submodule("alsoundsystem",           "Silverlan/alsoundsystem",          "b4a1c80e3952f5be8eaa5a60903d366bfd4fafdf")
get_gh_submodule("datasystem",              "Silverlan/datasystem",             "0feba2d2a0036d58c144ade31a6fe9f49a582626")
get_gh_submodule("iglfw",                   "Silverlan/iglfw",                  "acb72da92badbfd50375ae1fa245f389a3715920")
get_gh_submodule("luasystem",               "Silverlan/luasystem",              "274f3e8d6f1217667d7c1a268d7acfa9a78acc74")
get_gh_submodule("materialsystem",          "Silverlan/materialsystem",         "f118c5a721d48ba85f8827326fa2bd5398eeb3ea")
get_gh_submodule("mathutil",                "Silverlan/mathutil",               "75efeea1276cc4ce07e85093e41ab0cd53f59293")
get_gh_submodule("networkmanager",          "Silverlan/networkmanager",         "307e83426280669624ee25f8e3058af96e4e0db3")
get_gh_submodule("oskit",                   "Silverlan/oskit",                  "2c13e0963ef719d8c3a806f5c4ea0e151db58367")
get_gh_submodule("panima",                  "Silverlan/panima",                 "8faebe12b47542657adaec8afc95baff327f452f")
get_gh_submodule("prosper",                 "Silverlan/prosper",                "d9e4fe85af77ef9967540e44d9ba6fa540bb6cf2")
get_gh_submodule("sharedutils",             "Silverlan/sharedutils",            "598fbf3bf920685c3e605092454525b336247d68")
get_gh_submodule("util_bsp",                "Silverlan/util_bsp",               "d808b8ec3a9cf57c9b63a800d38304d4286925b8")
get_gh_submodule("util_formatted_text",     "Silverlan/util_formatted_text",    "50bd69695d55a61d9ecab8c3453a88fd8bc17a99")
get_gh_submodule("util_image",              "Silverlan/util_image",             "2d556b6391979e6774409a0488415312c612f745")
get_gh_submodule("util_pad",                "Silverlan/util_pad",               "9f353dbad247e742a67f87b171dd30810f908b66")
get_gh_submodule("util_pragma_doc",         "Silverlan/util_pragma_doc",        "e235cea9acf0e59eefabf7027aff3315753991db")
get_gh_submodule("util_smdmodel",           "Silverlan/util_smdmodel",          "d56004a9920e1477a19d45bdfc76ccfa605caaf0")
get_gh_submodule("util_sound",              "Silverlan/util_sound",             "e1652c5b4bb8a18b9cc545862810959f42f90d13")
get_gh_submodule("util_source2",            "Silverlan/util_source2",           "f938a9290c4129feb132e50b0100e8fcca1d5d1d")
get_gh_submodule("util_string",             "Silverlan/util_string",            "bd9d2f22e01ae68bef47d8efd8dc719a089f3a60")
get_gh_submodule("util_source_script",      "Silverlan/util_source_script",     "82e7537a2b562d036492568bc61be3e93507c380")
get_gh_submodule("util_timeline_scene",     "Silverlan/util_timeline_scene",    "6092f5ebeef6be95dc9553fe32dd424cc9af92b6")
get_gh_submodule("util_udm",                "Silverlan/util_udm",               "c5c6ed3d258d074a076ae1e0d2cf97c6ead02929")
get_gh_submodule("util_versioned_archive",  "Silverlan/util_versioned_archive", "0b0e419a112f654c2fdc27e0660309afc4c581b8")
get_gh_submodule("util_vmf",                "Silverlan/util_vmf",               "346aaeb9446349371010423e9a39a31c7cd35afc")
get_gh_submodule("util_zip",                "Silverlan/util_zip",               "a8b0b3e1e126849b929ea04f4fe9afeb2b40cdc7")
get_gh_submodule("util_shadergraph",        "Silverlan/util_shadergraph",       "5a7ff01fb4e51bc599eeae5126098e8cd826a0b5")
get_gh_submodule("vfilesystem",             "Silverlan/vfilesystem",            "9a135bde2c2032f3bd919c1d9637eb9383336c50")
get_gh_submodule("wgui",                    "Silverlan/wgui",                   "8f3a06b07728da6c726b475c80d10a6a7e78ce3b")
get_gh_submodule("util_unicode",            "Silverlan/util_unicode",           "5f0519049aecfd52e02ffbabeb84d735d027d9f0")
get_gh_submodule("cppbezierfit",            "Silverlan/cppbezierfit",           "63dbcaeddba39c80b885fd7eadbbbad560bc1791")

os.chdir(curDir)
