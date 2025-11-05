import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","6263507578feeb51a09c1396a19de83cdb72b9f3", "feat/cxx_module")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","66785193b7de5c8a7f6532e358096b54938295a7", "feat/cxx_module")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","2b3ca380e9677eea61e62ed49e09fb65610f034e", "feat/cxx_module")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","833386ec73585432016af52d556b512fc4a7c1be", "feat/cxx_module")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","def529d455c201c40d612de6739b658f8c9e8d60", "feat/cxx_module")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","e8865bf96312b0eafe44917ad45cb2a343c727fe", "feat/cxx_module")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","bddbdc36a8ba7c2533973390798d365373bd4f90", "feat/cxx_module")
get_submodule("oskit","https://github.com/Silverlan/oskit.git","892cc8a1f6298e80ffe86a7342e198d49ccc6868", "feat/cxx_module")
get_submodule("panima","https://github.com/Silverlan/panima.git","397e557ada0daf320dbfc2271a7dd2362c21088e", "feat/cxx_module")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","330698ff2caceb43552cf9fb8c5ece50af217048", "feat/cxx_module")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","738e471c025e921ab59943cbb8dd977ef086767c", "feat/cxx_module")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","8b0e95931f6c27b728999d021cea80379abdc0e9", "feat/cxx_module")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","a90e94bda2d29578d4fda8159170c08e393108db", "feat/cxx_module")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","81000f64876c6fae95023596e04bd4e7881a2d83", "feat/cxx_module")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","c78c540e75178bc779f0f25e96dc1fc055a0a2b9", "feat/cxx_module")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","ff2dca2aebc2a487be84afefa0948b4baee7c998", "feat/cxx_module")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","497206363b546dfea7da0f814599db0f48b98cb4", "feat/cxx_module")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","3ae58a7b3ccc2c89b07b8d6260f6db5feb5b9767", "feat/cxx_module")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","892f7370248e3192fa1deaa66554dcaa865a1c9f", "wip/cxx_module")
get_submodule("util_string","https://github.com/Silverlan/util_string.git","b4ac9788b7a750c65a0c04b5a8e7c3229e5fd840", "feat/cxx_module")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","7a0492d8625f8fe38934db40bc56208cd7b1a2e9", "feat/cxx_module")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","f701e000be140dcfdcfc913629d3fa8ded781f3d")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","68168b42d3a22a2e5c5dc2af50449074e79986f8", "feat/cxx_module")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","37811f3415bc7b9711e62b898bdc071dad529376", "feat/cxx_module")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","c9e72259901bb68fc10b45c84a1cd6d3eae8e4a0")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","c1f2113d97d2bdb20e4a2901e4ed3277bbeba57d", "feat/cxx_module")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","612d32255150568b5fa25e96c3207ad2890251d4")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","83ee78082532ad1a54fe1508d00ea1e03896e8b0", "feat/cxx_module")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","be8f20dc6b7c6592559407ea2b9b069effbf8f57", "feat/cxx_modules2")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","cfc0cc215af1578e23ad06c209acfd115f6eb556", "feat/cxx_module")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","6c9f1a0111380a02ec08994653e1955d1122693a", "feat/cxx_module")

os.chdir(curDir)
