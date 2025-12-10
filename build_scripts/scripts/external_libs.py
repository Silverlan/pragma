import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_gh_submodule("alsoundsystem",           "Silverlan/alsoundsystem",          "93a5bea5ca01bcc08517b83ccb24c7a24672e44f")
get_gh_submodule("datasystem",              "Silverlan/datasystem",             "c273b7d666a52d7833acc14ad9aee49980db1130")
get_gh_submodule("iglfw",                   "Silverlan/iglfw",                  "7e56d1adbe0ed88b7a6dba949675acad3bd797fb")
get_gh_submodule("luasystem",               "Silverlan/luasystem",              "f180ac83170a2fba60f201e9b54c32ca2a2a4aa7")
get_gh_submodule("materialsystem",          "Silverlan/materialsystem",         "dac3c2e9d35a13f126b5c79dd2847016b5163712")
get_gh_submodule("mathutil",                "Silverlan/mathutil",               "3ed437c938a9462a1e233a44eb6e3c302c3bd525")
get_gh_submodule("networkmanager",          "Silverlan/networkmanager",         "dd21c506c0b982b99cd5b004503bd964739aa67a")
get_gh_submodule("oskit",                   "Silverlan/oskit",                  "2c13e0963ef719d8c3a806f5c4ea0e151db58367")
get_gh_submodule("panima",                  "Silverlan/panima",                 "84968030fd1d423b276d69021d63946f1a59bde3")
get_gh_submodule("prosper",                 "Silverlan/prosper",                "7afd2ac9676b2b880a703aca3f94d7d637b3f176")
get_gh_submodule("sharedutils",             "Silverlan/sharedutils",            "6a112723c4b7acbc18437bead183641076de17af")
get_gh_submodule("util_bsp",                "Silverlan/util_bsp",               "84add001374c25f198334699c8c948bd4ee90030")
get_gh_submodule("util_formatted_text",     "Silverlan/util_formatted_text",    "a7a1b35e0b26b4f1dee5107a0438b7daff4ede7b")
get_gh_submodule("util_image",              "Silverlan/util_image",             "a8be8cca5f361f0e0d66a4e23fcaa0f6cd5425d0")
get_gh_submodule("util_pad",                "Silverlan/util_pad",               "e8fa4031e8a98fc09b478e00d806d07e08afc7bd")
get_gh_submodule("util_pragma_doc",         "Silverlan/util_pragma_doc",        "9bccf727f6d0da0eaa66d0b0971e715b87bacf1d")
get_gh_submodule("util_smdmodel",           "Silverlan/util_smdmodel",          "6a8b90db0d88729f8d329f9c57b7396ac95360a5")
get_gh_submodule("util_sound",              "Silverlan/util_sound",             "c69fd213b8c4b4579f318a57704d9099ae18e58f")
get_gh_submodule("util_source2",            "Silverlan/util_source2",           "3924ce5c909445ef7a63b3c5fb70e8eb4793fd09")
get_gh_submodule("util_string",             "Silverlan/util_string",            "8e7993d47489b3a25417f1af199cf294bc157164")
get_gh_submodule("util_source_script",      "Silverlan/util_source_script",     "451427db8e44423de713887fc27d7308fb423573")
get_gh_submodule("util_timeline_scene",     "Silverlan/util_timeline_scene",    "6092f5ebeef6be95dc9553fe32dd424cc9af92b6")
get_gh_submodule("util_udm",                "Silverlan/util_udm",               "b816b34d567c9ed7d1108f93fd5cf1002cc6d450")
get_gh_submodule("util_versioned_archive",  "Silverlan/util_versioned_archive", "8b0b375bbfdbb81e81e3726e02feae94838789c0")
get_gh_submodule("util_vmf",                "Silverlan/util_vmf",               "70ab16be2e0214bcb6fc11586cf21b36340e0c40")
get_gh_submodule("util_zip",                "Silverlan/util_zip",               "248a62584fd9c220a084af043b63cb349da43b88")
get_gh_submodule("util_shadergraph",        "Silverlan/util_shadergraph",       "79cb4eaefa7e24cc963db82ea170c91fed9a35da")
get_gh_submodule("vfilesystem",             "Silverlan/vfilesystem",            "627158a544d0a257c8fdc42f96c0a44dd9aad49b")
get_gh_submodule("wgui",                    "Silverlan/wgui",                   "3e4d276f639c73a0e69329b636de87a427c1bf50")
get_gh_submodule("util_unicode",            "Silverlan/util_unicode",           "b86a01e981e4944a428be7955088dc3d39573b89")
get_gh_submodule("cppbezierfit",            "Silverlan/cppbezierfit",           "63dbcaeddba39c80b885fd7eadbbbad560bc1791")

os.chdir(curDir)
