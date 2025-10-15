import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","4fa3070334aa71509fb89435c0b864074ace15b7", "feat/cxx_module")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","b828ac4f8abdabebe1604e2a2bbc6115c6612041")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","916dc67ad198763d029c9726132491db6e511828", "feat/cxx_module")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","8ed27d16c3ff8706176d2b8e7581dc80425a1841")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","b052c797daf493dbcae47e1adfbf8d2d098e7e9b", "feat/cxx_module")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","eb0b3a4803337dfb164b8cf71dc23d0fd7820f5e", "feat/cxx_module")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","397a0c6919678e80412f02560582d80bcf1c43b4", "feat/cxx_module")
get_submodule("oskit","https://github.com/Silverlan/oskit.git","77d4f8c353fd315f1255383ff4e02182dd882472")
get_submodule("panima","https://github.com/Silverlan/panima.git","1ee050cc64db898505e788299305540f151acd30")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","7b6476603a0ea9e401a1b9d2020449e48e6780a3", "feat/cxx_module")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","f658fcc4c321819a063b2e9f0333e4ba6c43de98", "feat/cxx_module")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","74a569cd8b9071d461abd76c62bdf5878c79bb12")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","a6635489ac97a76a65bdc4c608bb199f8e78144e")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","ed1af2937125a8d17e534b52a15f8a84ea6dbcab", "feat/cxx_module")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","a7a39bf6ed56dd0a5c3fe1c65d786a068fed5b67", "feat/cxx_module")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","b04bb220ad8db49c0afa3c265bc14d33029d4912")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","d9b4d3e3fe7cf9eaaa53d6b4c78906a3302157d2")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","93b9389a36291dcf85d987a279ac4df7bf9aec78")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","2849e1e2b9803243abc52b79e1de19a7b7d92897")
get_submodule("util_string","https://github.com/Silverlan/util_string.git","1f938c758ae6612c4d5b34b8f84abf4cf7a5c124")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","f5ee2fef2ff3bb64d0a5bd37d133bdf9f8fce9cc")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","456429ff129076fff4c6396a5809ea05fa82e267")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","dc4ace6e05f6aa7e11fb2fec988ee62ef59cd68c", "feat/cxx_module")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","a9b767fe72a24e7407632f1dd842b14a6a081190")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","9817fd13e5a02999ff07ef430da15a970af5473d")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","52932696e9addd146046f51a69d4c897e1237161")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","49e63c9a1dedf5cc5a2ad04ed67607b94a2e9d8c")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","69d332b6a98dfe877a3f411665aded2f13636d31", "feat/cxx_module")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","045656b2cfd87a1a247008d5ba3db6858963b993", "feat/cxx_modules2")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","ded7f7c9c1c8af389c7efa8135cdb0b0615f31cf")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","f691045dfdb3313205a09ef4f460203be0a3152c")

os.chdir(curDir)
