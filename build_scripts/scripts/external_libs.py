import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","4dc79d3180bdf62ab63043efac797409309d63a8")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","7642296ea0f8316957e1123034e87034294f8b46")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","9d0bf8a7123bf7ea54a373df61d240206bddf117")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","a93cc69e611d928e6f8d80c68c7a79afece06ba5")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","420ff85c93f75815cba136776ae4deed32074f4e")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","83ceab3777b73509585839e8fdd79ccc9bb1bf2a")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","68d2f5c")
get_submodule("panima","https://github.com/Silverlan/panima.git","6a9b46ec20ea319bf38a79a3a8a1f993e0ceec57")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","de1f7b1d3fa9f80802812698b50a2a92836429ca")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","acfdb6f30e4fdf6b8ed2b52c4607f83f8914b8e1")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","49c6f2ec30d944a37cce631aa70db32e44ace336")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","c2d5e06409ea8543493f9157da6c0335ded36a8d")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","2ba5c8a4fdb5dd88183598925c6034172ae70975")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","7a827f8")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","f4009c8")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","04677c3a9f5fa6a9eaef81368d816177855f8fa2")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","9e1c77f233e6e50d3e3dc4b07fda134d38a961d5")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","6eec5630738db6716cc61c795e68b40b648ec13b")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","f56f97c347e7829f5053fb5b22cbc314148512b9")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","76d02ff")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","fa45848adf9dbb29757335536930223029afc4cb")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","be6d3fe")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","3080ba05280ae5b0a76ef283870864c16d1c7826")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","7405a2df9839a3473f6026ceff64630f602f0a19")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","c4370a95cfa28d4b4a539512ae378fe3c05f6e98")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","7fc86999a0ef9716273993347022f7e40b22110d")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","6936adeaa9a054a9530078c9a47e3fa94cf4bffa")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","2a3eea546f4d07494b2d6793ec863be8d35c023f")

os.chdir(curDir)
