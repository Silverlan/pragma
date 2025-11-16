import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_gh_submodule("alsoundsystem",           "Silverlan/alsoundsystem",          "89744272f9adcbef399061b8a4b7f23e8c429154", "feat/cxx_module")
get_gh_submodule("datasystem",              "Silverlan/datasystem",             "66785193b7de5c8a7f6532e358096b54938295a7", "feat/cxx_module")
get_gh_submodule("iglfw",                   "Silverlan/iglfw",                  "2b3ca380e9677eea61e62ed49e09fb65610f034e", "feat/cxx_module")
get_gh_submodule("luasystem",               "Silverlan/luasystem",              "78890ac5df62b7099e63096c3bfb90478b51174e", "feat/cxx_module")
get_gh_submodule("materialsystem",          "Silverlan/materialsystem",         "2a00db57dcfaf2219f614bf216e0d566d8c7396e", "feat/cxx_module")
get_gh_submodule("mathutil",                "Silverlan/mathutil",               "e81ed7f7ca6f590c3e235eebec6bf4440510d4bc", "feat/cxx_module")
get_gh_submodule("networkmanager",          "Silverlan/networkmanager",         "9f84a348193660aca0ed36db32c6cf6abd40cf75", "feat/cxx_module")
get_gh_submodule("oskit",                   "Silverlan/oskit",                  "892cc8a1f6298e80ffe86a7342e198d49ccc6868", "feat/cxx_module")
get_gh_submodule("panima",                  "Silverlan/panima",                 "bd3a24c8e06faaef562096b87dedadb89df68cfb", "feat/cxx_module")
get_gh_submodule("prosper",                 "Silverlan/prosper",                "41a9ad1d3289ef5a3d6bfb7ca61f6e22c4996061", "feat/cxx_module")
get_gh_submodule("sharedutils",             "Silverlan/sharedutils",            "f3f79650554dc73db36f967ba389942724f05579", "feat/cxx_module")
get_gh_submodule("util_bsp",                "Silverlan/util_bsp",               "8b0e95931f6c27b728999d021cea80379abdc0e9", "feat/cxx_module")
get_gh_submodule("util_formatted_text",     "Silverlan/util_formatted_text",    "a90e94bda2d29578d4fda8159170c08e393108db", "feat/cxx_module")
get_gh_submodule("util_image",              "Silverlan/util_image",             "6d12ca909d5f02c38cf85aa2073909744d2c0707", "feat/cxx_module")
get_gh_submodule("util_pad",                "Silverlan/util_pad",               "c78c540e75178bc779f0f25e96dc1fc055a0a2b9", "feat/cxx_module")
get_gh_submodule("util_pragma_doc",         "Silverlan/util_pragma_doc",        "ff2dca2aebc2a487be84afefa0948b4baee7c998", "feat/cxx_module")
get_gh_submodule("util_smdmodel",           "Silverlan/util_smdmodel",          "497206363b546dfea7da0f814599db0f48b98cb4", "feat/cxx_module")
get_gh_submodule("util_sound",              "Silverlan/util_sound",             "3ae58a7b3ccc2c89b07b8d6260f6db5feb5b9767", "feat/cxx_module")
get_gh_submodule("util_source2",            "Silverlan/util_source2",           "94df471e9782b189d7873e4868373d58d859ae21", "wip/cxx_module")
get_gh_submodule("util_string",             "Silverlan/util_string",            "94e2928ddb44d59270926ed7e54fcd4d0be48d18", "feat/cxx_module")
get_gh_submodule("util_source_script",      "Silverlan/util_source_script",     "7a0492d8625f8fe38934db40bc56208cd7b1a2e9", "feat/cxx_module")
get_gh_submodule("util_timeline_scene",     "Silverlan/util_timeline_scene",    "02c9d28304982cdfafd62d11918660492a09a305")
get_gh_submodule("util_udm",                "Silverlan/util_udm",               "50210f38f8f1f675c4adc998acb53943e9354697", "feat/cxx_module")
get_gh_submodule("util_versioned_archive",  "Silverlan/util_versioned_archive", "37811f3415bc7b9711e62b898bdc071dad529376", "feat/cxx_module")
get_gh_submodule("util_vmf",                "Silverlan/util_vmf",               "c9e72259901bb68fc10b45c84a1cd6d3eae8e4a0")
get_gh_submodule("util_zip",                "Silverlan/util_zip",               "c1f2113d97d2bdb20e4a2901e4ed3277bbeba57d", "feat/cxx_module")
get_gh_submodule("util_shadergraph",        "Silverlan/util_shadergraph",       "2b13a7129d3464b176acd0c637adc7a82524919d")
get_gh_submodule("vfilesystem",             "Silverlan/vfilesystem",            "83ee78082532ad1a54fe1508d00ea1e03896e8b0", "feat/cxx_module")
get_gh_submodule("wgui",                    "Silverlan/wgui",                   "c2116b990b89461a872b01f5ab5ce68d0a0696a3", "feat/cxx_modules2")
get_gh_submodule("util_unicode",            "Silverlan/util_unicode",           "cfc0cc215af1578e23ad06c209acfd115f6eb556", "feat/cxx_module")
get_gh_submodule("cppbezierfit",            "Silverlan/cppbezierfit",           "6c9f1a0111380a02ec08994653e1955d1122693a", "feat/cxx_module")

os.chdir(curDir)
