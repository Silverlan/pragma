import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","4dc79d3180bdf62ab63043efac797409309d63a8")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","a6dfff6")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","1cf6d222aa3967ef4ee45c92ba210df22fb79afd")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","a93cc69e611d928e6f8d80c68c7a79afece06ba5")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","7b1f9e8d27903bf9779b9f5e1bc938621ed62112")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","08d718c3da05db1cbed7f6c444e77d4473d9f043")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","68d2f5c")
get_submodule("panima","https://github.com/Silverlan/panima.git","30132a1e4d74bc6b5a6e52d9905ea881a4ae8bdf")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","40b52a18ea02ab611e602f84c172ecd3afd2af19")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","0ff7f3fd51a2be9c768848ffed79629a70fe74ed")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","3c11053")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","5b441caacec34c3bdc89b6f529e07387f2fadb7b")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","7a66f93")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","7a827f8")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","d3b3e78e6ecf374cd1aa225bd3d56f1cc280f942")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","5e581ab")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","83693a562cab0c5a536f9ed81cb15100e53f827a")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","cf553d6")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","ea0d03a")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","76d02ff")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","376c09b8c74a5b0078c83e1964f28d28138bd097")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","be6d3fe")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","cdba99d")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","166f108587962b59391a37e14dcfe1a51f81a81b")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","ba2db128eec7a23cb65c67b396d77f2d9a93e948")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","7fc86999a0ef9716273993347022f7e40b22110d")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","454694b90e01a4b0697bd475c8e62b5903f2353f")

os.chdir(curDir)
