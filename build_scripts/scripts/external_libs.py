import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_gh_submodule("alsoundsystem",           "Silverlan/alsoundsystem",          "52ce46181683ae0a68fc342918f2a2154503ba79")
get_gh_submodule("datasystem",              "Silverlan/datasystem",             "a703f6ba410b882f76a2a7f3de566f95d25b924f")
get_gh_submodule("iglfw",                   "Silverlan/iglfw",                  "f41e9ebee5fa0fb588c8d0a41ce099457290026d")
get_gh_submodule("luasystem",               "Silverlan/luasystem",              "986bc641790708b3caa36a0fe6715759ae4c6220")
get_gh_submodule("materialsystem",          "Silverlan/materialsystem",         "39b7881f820bc9bb7559a27db12ac43bbe7b0104")
get_gh_submodule("mathutil",                "Silverlan/mathutil",               "6367e44bbb1ed1a2f6eef48f0483388c213ee704")
get_gh_submodule("networkmanager",          "Silverlan/networkmanager",         "307e83426280669624ee25f8e3058af96e4e0db3")
get_gh_submodule("oskit",                   "Silverlan/oskit",                  "2c13e0963ef719d8c3a806f5c4ea0e151db58367")
get_gh_submodule("panima",                  "Silverlan/panima",                 "8faebe12b47542657adaec8afc95baff327f452f")
get_gh_submodule("prosper",                 "Silverlan/prosper",                "571bea28e6a095ed45d0725131fac0c33591ae07")
get_gh_submodule("sharedutils",             "Silverlan/sharedutils",            "8ae73761b233a1bff0aff656e7715a21a144ed3e")
get_gh_submodule("util_bsp",                "Silverlan/util_bsp",               "d808b8ec3a9cf57c9b63a800d38304d4286925b8")
get_gh_submodule("util_formatted_text",     "Silverlan/util_formatted_text",    "50bd69695d55a61d9ecab8c3453a88fd8bc17a99")
get_gh_submodule("util_image",              "Silverlan/util_image",             "1382b778b28565fa1462b8af04c80b70890dc2b0")
get_gh_submodule("util_pad",                "Silverlan/util_pad",               "9f353dbad247e742a67f87b171dd30810f908b66")
get_gh_submodule("util_pragma_doc",         "Silverlan/util_pragma_doc",        "e235cea9acf0e59eefabf7027aff3315753991db")
get_gh_submodule("util_smdmodel",           "Silverlan/util_smdmodel",          "e6bb48d9b0f1b0f22a48cb3d0def1e8850a889ed")
get_gh_submodule("util_sound",              "Silverlan/util_sound",             "e1652c5b4bb8a18b9cc545862810959f42f90d13")
get_gh_submodule("util_source2",            "Silverlan/util_source2",           "f938a9290c4129feb132e50b0100e8fcca1d5d1d")
get_gh_submodule("util_string",             "Silverlan/util_string",            "bd9d2f22e01ae68bef47d8efd8dc719a089f3a60")
get_gh_submodule("util_source_script",      "Silverlan/util_source_script",     "82e7537a2b562d036492568bc61be3e93507c380")
get_gh_submodule("util_timeline_scene",     "Silverlan/util_timeline_scene",    "6092f5ebeef6be95dc9553fe32dd424cc9af92b6")
get_gh_submodule("util_udm",                "Silverlan/util_udm",               "5861625eb8c794e30612c5c70d36d82f57ea90a8")
get_gh_submodule("util_versioned_archive",  "Silverlan/util_versioned_archive", "0b0e419a112f654c2fdc27e0660309afc4c581b8")
get_gh_submodule("util_vmf",                "Silverlan/util_vmf",               "346aaeb9446349371010423e9a39a31c7cd35afc")
get_gh_submodule("util_zip",                "Silverlan/util_zip",               "a8b0b3e1e126849b929ea04f4fe9afeb2b40cdc7")
get_gh_submodule("util_shadergraph",        "Silverlan/util_shadergraph",       "5a7ff01fb4e51bc599eeae5126098e8cd826a0b5")
get_gh_submodule("vfilesystem",             "Silverlan/vfilesystem",            "0aee81c583d2af494dc37d3071d52548a4b1f71b")
get_gh_submodule("wgui",                    "Silverlan/wgui",                   "d5b5599769fdb2daa3deb90168565cca7ef0014d")
get_gh_submodule("util_unicode",            "Silverlan/util_unicode",           "b86a01e981e4944a428be7955088dc3d39573b89")
get_gh_submodule("cppbezierfit",            "Silverlan/cppbezierfit",           "63dbcaeddba39c80b885fd7eadbbbad560bc1791")

os.chdir(curDir)
