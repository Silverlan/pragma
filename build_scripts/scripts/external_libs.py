import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","4fa3070334aa71509fb89435c0b864074ace15b7", "feat/cxx_module")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","d9f80c15faa9749b040bc4643b0a235a046adaca", "feat/cxx_module")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","69aeebd142fcb872515ef15f50ecf67643b6eb2b", "feat/cxx_module")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","cbcaaf7519fae6297044281776cd87e1641981ad", "feat/cxx_module")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","246472bc6d6edb3f016d8eef4b3406b648cffec2", "feat/cxx_module")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","95a9c7057f30b2c25a7b34a5b78d9d95591f4db2", "feat/cxx_module")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","b0334c5b217d8b5e5b1c0e967aba72df8083248a", "feat/cxx_module")
get_submodule("oskit","https://github.com/Silverlan/oskit.git","34c00cae839455fc2943d853bf41ffb925e9adb3", "feat/cxx_module")
get_submodule("panima","https://github.com/Silverlan/panima.git","ff315beff89e8231cc16ce08a215fadbb65d9d1c", "feat/cxx_module")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","8d8bc457382aa51cf7e071d11a1f3d896d56b289", "feat/cxx_module")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","5df1833a1fc487a6b6d21da8d244bf83b4337efd", "feat/cxx_module")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","74a569cd8b9071d461abd76c62bdf5878c79bb12")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","a6635489ac97a76a65bdc4c608bb199f8e78144e")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","cac15702964bfc6322a84e9219f2d95e2ff02d5d", "feat/cxx_module")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","3377bbe8730b3df7fbd4cb3dfd2ee635f318ae48", "feat/cxx_module")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","9dbd26d08d020d7f37f9dd7f25acd6fc8efdce78", "feat/cxx_module")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","019e590df7acf60e073b8053dd941e675cce1ff9", "feat/cxx_module")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","274bcbad351ce46ca7456c384c20e98a72b97910", "feat/cxx_module")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","0c0da9b7e15de0854d84d39015621d22f1fd5872", "feat/cxx_module")
get_submodule("util_string","https://github.com/Silverlan/util_string.git","1f938c758ae6612c4d5b34b8f84abf4cf7a5c124")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","dc5d38ea59dfcb2d9be93ac84aef3ca8cccafb3d", "feat/cxx_module")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","456429ff129076fff4c6396a5809ea05fa82e267")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","8c64971e20d6322cdf3bbcc3bf7a9eb87ed39f31", "feat/cxx_module")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","26d76dfbae9bc9f745299ae0629c83dec7d2ed10", "feat/cxx_module")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","9817fd13e5a02999ff07ef430da15a970af5473d")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","499d10b552383bda351d37fe764c3d42f2479b62", "feat/cxx_module")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","49e63c9a1dedf5cc5a2ad04ed67607b94a2e9d8c")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","69d332b6a98dfe877a3f411665aded2f13636d31", "feat/cxx_module")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","045656b2cfd87a1a247008d5ba3db6858963b993", "feat/cxx_modules2")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","d3d5cf3e23d41bbfd3a90c9014904355c895b320", "feat/cxx_module")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","f691045dfdb3313205a09ef4f460203be0a3152c")

os.chdir(curDir)
