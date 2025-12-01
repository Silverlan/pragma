import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_gh_submodule("alsoundsystem",           "Silverlan/alsoundsystem",          "93a5bea5ca01bcc08517b83ccb24c7a24672e44f")
get_gh_submodule("datasystem",              "Silverlan/datasystem",             "2d2e46832bb17aa673e7143435f2997c577b421b")
get_gh_submodule("iglfw",                   "Silverlan/iglfw",                  "7e56d1adbe0ed88b7a6dba949675acad3bd797fb")
get_gh_submodule("luasystem",               "Silverlan/luasystem",              "6a67d304e2725779ce3d4ad23e9e2282d08a4b4f")
get_gh_submodule("materialsystem",          "Silverlan/materialsystem",         "dac3c2e9d35a13f126b5c79dd2847016b5163712")
get_gh_submodule("mathutil",                "Silverlan/mathutil",               "7b086dd545b1f4f21ce58c2f6ef551e6a2f2073d")
get_gh_submodule("networkmanager",          "Silverlan/networkmanager",         "dd21c506c0b982b99cd5b004503bd964739aa67a")
get_gh_submodule("oskit",                   "Silverlan/oskit",                  "2c13e0963ef719d8c3a806f5c4ea0e151db58367")
get_gh_submodule("panima",                  "Silverlan/panima",                 "84968030fd1d423b276d69021d63946f1a59bde3")
get_gh_submodule("prosper",                 "Silverlan/prosper",                "66062ac2ddb28a1cfaf4e12a0d6784240a4c3eed")
get_gh_submodule("sharedutils",             "Silverlan/sharedutils",            "a1720b95eea39c6d0775a10cff9c9ca6a7029c19")
get_gh_submodule("util_bsp",                "Silverlan/util_bsp",               "84add001374c25f198334699c8c948bd4ee90030")
get_gh_submodule("util_formatted_text",     "Silverlan/util_formatted_text",    "a7a1b35e0b26b4f1dee5107a0438b7daff4ede7b")
get_gh_submodule("util_image",              "Silverlan/util_image",             "a8be8cca5f361f0e0d66a4e23fcaa0f6cd5425d0")
get_gh_submodule("util_pad",                "Silverlan/util_pad",               "e8fa4031e8a98fc09b478e00d806d07e08afc7bd")
get_gh_submodule("util_pragma_doc",         "Silverlan/util_pragma_doc",        "9bccf727f6d0da0eaa66d0b0971e715b87bacf1d")
get_gh_submodule("util_smdmodel",           "Silverlan/util_smdmodel",          "592fdb11421a32ea8e6f0d64d89d88682593b924")
get_gh_submodule("util_sound",              "Silverlan/util_sound",             "c69fd213b8c4b4579f318a57704d9099ae18e58f")
get_gh_submodule("util_source2",            "Silverlan/util_source2",           "3924ce5c909445ef7a63b3c5fb70e8eb4793fd09")
get_gh_submodule("util_string",             "Silverlan/util_string",            "3a44670db344d7e04ac941c1a5e91ffc12d31226")
get_gh_submodule("util_source_script",      "Silverlan/util_source_script",     "451427db8e44423de713887fc27d7308fb423573")
get_gh_submodule("util_timeline_scene",     "Silverlan/util_timeline_scene",    "6092f5ebeef6be95dc9553fe32dd424cc9af92b6")
get_gh_submodule("util_udm",                "Silverlan/util_udm",               "e32bcc0f378d98766178bf4f7e363df8be039128")
get_gh_submodule("util_versioned_archive",  "Silverlan/util_versioned_archive", "8b0b375bbfdbb81e81e3726e02feae94838789c0")
get_gh_submodule("util_vmf",                "Silverlan/util_vmf",               "70ab16be2e0214bcb6fc11586cf21b36340e0c40")
get_gh_submodule("util_zip",                "Silverlan/util_zip",               "248a62584fd9c220a084af043b63cb349da43b88")
get_gh_submodule("util_shadergraph",        "Silverlan/util_shadergraph",       "79cb4eaefa7e24cc963db82ea170c91fed9a35da")
get_gh_submodule("vfilesystem",             "Silverlan/vfilesystem",            "627158a544d0a257c8fdc42f96c0a44dd9aad49b")
get_gh_submodule("wgui",                    "Silverlan/wgui",                   "a5c1a514a21e0dcdbbb77d5357e79daa3c4d54f2")
get_gh_submodule("util_unicode",            "Silverlan/util_unicode",           "b86a01e981e4944a428be7955088dc3d39573b89")
get_gh_submodule("cppbezierfit",            "Silverlan/cppbezierfit",           "63dbcaeddba39c80b885fd7eadbbbad560bc1791")

os.chdir(curDir)
