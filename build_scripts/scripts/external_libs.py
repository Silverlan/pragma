import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_gh_submodule("alsoundsystem",           "Silverlan/alsoundsystem",          "95c6a1e7d40b21968529ce859ecfb0b90ab8da83")
get_gh_submodule("datasystem",              "Silverlan/datasystem",             "2d2e46832bb17aa673e7143435f2997c577b421b")
get_gh_submodule("iglfw",                   "Silverlan/iglfw",                  "332fcd2f940cf643a9e2bf158f0220b3edc8969b")
get_gh_submodule("luasystem",               "Silverlan/luasystem",              "6b23a85320d8c6ce95dbd60308f923e941f5e393")
get_gh_submodule("materialsystem",          "Silverlan/materialsystem",         "a8f282176499d197f3e7aa6f098719abdbb5eda3")
get_gh_submodule("mathutil",                "Silverlan/mathutil",               "53a27c1ff00f1c240f96913cc0adfe7dee2d4403")
get_gh_submodule("networkmanager",          "Silverlan/networkmanager",         "bd4f1a047f03eb20c2e02fdf9acb6d372859884f")
get_gh_submodule("oskit",                   "Silverlan/oskit",                  "2c13e0963ef719d8c3a806f5c4ea0e151db58367")
get_gh_submodule("panima",                  "Silverlan/panima",                 "1cb1b63f4141b3ec043de731748b371162daed46")
get_gh_submodule("prosper",                 "Silverlan/prosper",                "c7aeac9cec82390a06e326e5c87e79ad2fd039ed")
get_gh_submodule("sharedutils",             "Silverlan/sharedutils",            "9d61a4766f8c4adb7d2b5c782f2065c6befc356a")
get_gh_submodule("util_bsp",                "Silverlan/util_bsp",               "84add001374c25f198334699c8c948bd4ee90030")
get_gh_submodule("util_formatted_text",     "Silverlan/util_formatted_text",    "a7a1b35e0b26b4f1dee5107a0438b7daff4ede7b")
get_gh_submodule("util_image",              "Silverlan/util_image",             "855b1c4fa3631ebe48243d13f3618ff6bbff1566")
get_gh_submodule("util_pad",                "Silverlan/util_pad",               "e8fa4031e8a98fc09b478e00d806d07e08afc7bd")
get_gh_submodule("util_pragma_doc",         "Silverlan/util_pragma_doc",        "12c106673a38b827b1b2040352a36890a6980cd5")
get_gh_submodule("util_smdmodel",           "Silverlan/util_smdmodel",          "592fdb11421a32ea8e6f0d64d89d88682593b924")
get_gh_submodule("util_sound",              "Silverlan/util_sound",             "c69fd213b8c4b4579f318a57704d9099ae18e58f")
get_gh_submodule("util_source2",            "Silverlan/util_source2",           "3924ce5c909445ef7a63b3c5fb70e8eb4793fd09")
get_gh_submodule("util_string",             "Silverlan/util_string",            "3a44670db344d7e04ac941c1a5e91ffc12d31226")
get_gh_submodule("util_source_script",      "Silverlan/util_source_script",     "451427db8e44423de713887fc27d7308fb423573")
get_gh_submodule("util_timeline_scene",     "Silverlan/util_timeline_scene",    "6092f5ebeef6be95dc9553fe32dd424cc9af92b6")
get_gh_submodule("util_udm",                "Silverlan/util_udm",               "7ff078a9cff091acb813057499506f611e6ac3d4")
get_gh_submodule("util_versioned_archive",  "Silverlan/util_versioned_archive", "b2ab34d0183af8123fa6ddc477436cff6427d8f2")
get_gh_submodule("util_vmf",                "Silverlan/util_vmf",               "70ab16be2e0214bcb6fc11586cf21b36340e0c40")
get_gh_submodule("util_zip",                "Silverlan/util_zip",               "248a62584fd9c220a084af043b63cb349da43b88")
get_gh_submodule("util_shadergraph",        "Silverlan/util_shadergraph",       "88b51691d9928e78755bf7f6d4faf4aee8f9cc22")
get_gh_submodule("vfilesystem",             "Silverlan/vfilesystem",            "a4e061cfb48440a04e2c4c98e39af965f6e8b032")
get_gh_submodule("wgui",                    "Silverlan/wgui",                   "a2458cd72c61c7538cae07fd3478aa4208cd0cbe")
get_gh_submodule("util_unicode",            "Silverlan/util_unicode",           "b86a01e981e4944a428be7955088dc3d39573b89")
get_gh_submodule("cppbezierfit",            "Silverlan/cppbezierfit",           "63dbcaeddba39c80b885fd7eadbbbad560bc1791")

os.chdir(curDir)
