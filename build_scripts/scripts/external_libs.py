import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","89744272f9adcbef399061b8a4b7f23e8c429154", "feat/cxx_module")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","66785193b7de5c8a7f6532e358096b54938295a7", "feat/cxx_module")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","2b3ca380e9677eea61e62ed49e09fb65610f034e", "feat/cxx_module")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","8ce4bc3b8648ae68606715cca47b45e5cb38197f", "feat/cxx_module")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","def529d455c201c40d612de6739b658f8c9e8d60", "feat/cxx_module")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","52a13143a99af4cfcd4e55ad7e32d80c2906bde3", "feat/cxx_module")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","1fe4baa44afe30ede44554a3a18adfdfce5774a7", "feat/cxx_module")
get_submodule("oskit","https://github.com/Silverlan/oskit.git","892cc8a1f6298e80ffe86a7342e198d49ccc6868", "feat/cxx_module")
get_submodule("panima","https://github.com/Silverlan/panima.git","397e557ada0daf320dbfc2271a7dd2362c21088e", "feat/cxx_module")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","153ff780cd24532ed992afdd695a025f11a4476b", "feat/cxx_module")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","41330026c41d609569cfeafcf547b6d69e1248be", "feat/cxx_module")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","8b0e95931f6c27b728999d021cea80379abdc0e9", "feat/cxx_module")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","a90e94bda2d29578d4fda8159170c08e393108db", "feat/cxx_module")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","6d12ca909d5f02c38cf85aa2073909744d2c0707", "feat/cxx_module")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","c78c540e75178bc779f0f25e96dc1fc055a0a2b9", "feat/cxx_module")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","ff2dca2aebc2a487be84afefa0948b4baee7c998", "feat/cxx_module")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","497206363b546dfea7da0f814599db0f48b98cb4", "feat/cxx_module")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","3ae58a7b3ccc2c89b07b8d6260f6db5feb5b9767", "feat/cxx_module")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","94df471e9782b189d7873e4868373d58d859ae21", "wip/cxx_module")
get_submodule("util_string","https://github.com/Silverlan/util_string.git","94e2928ddb44d59270926ed7e54fcd4d0be48d18", "feat/cxx_module")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","7a0492d8625f8fe38934db40bc56208cd7b1a2e9", "feat/cxx_module")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","02c9d28304982cdfafd62d11918660492a09a305")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","50210f38f8f1f675c4adc998acb53943e9354697", "feat/cxx_module")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","37811f3415bc7b9711e62b898bdc071dad529376", "feat/cxx_module")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","c9e72259901bb68fc10b45c84a1cd6d3eae8e4a0")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","c1f2113d97d2bdb20e4a2901e4ed3277bbeba57d", "feat/cxx_module")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","2b13a7129d3464b176acd0c637adc7a82524919d")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","83ee78082532ad1a54fe1508d00ea1e03896e8b0", "feat/cxx_module")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","d66b365a1197844bf00444dbcc950accbf4df684", "feat/cxx_modules2")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","cfc0cc215af1578e23ad06c209acfd115f6eb556", "feat/cxx_module")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","6c9f1a0111380a02ec08994653e1955d1122693a", "feat/cxx_module")

os.chdir(curDir)
