import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","99a12063afdfc68fa21515236e9c59d9433b1be7")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","7642296ea0f8316957e1123034e87034294f8b46")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","9d0bf8a7123bf7ea54a373df61d240206bddf117")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","aba7e57505fece6b3b6b051068fa01cb37bc567c")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","420ff85c93f75815cba136776ae4deed32074f4e")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","83ceab3777b73509585839e8fdd79ccc9bb1bf2a")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","981bc5809c1a768267ddace778205e1be0262730")
get_submodule("panima","https://github.com/Silverlan/panima.git","e4c2823c3ecd981575b97d3279f477d5215afa4d")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","1d4fcf8218eb01fb26abd9cf63185549cd0f9b99")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","acfdb6f30e4fdf6b8ed2b52c4607f83f8914b8e1")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","49c6f2ec30d944a37cce631aa70db32e44ace336")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","7e9764ccf5fe4a42f3183a654019f5a7923bc348")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","2ba5c8a4fdb5dd88183598925c6034172ae70975")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","a199b34afc527c33b88214fa03d9c074ac36f33a")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","71b9593872633adafa8dada0797bdf4fabf74c4f")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","04677c3a9f5fa6a9eaef81368d816177855f8fa2")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","9e1c77f233e6e50d3e3dc4b07fda134d38a961d5")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","6eec5630738db6716cc61c795e68b40b648ec13b")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","f56f97c347e7829f5053fb5b22cbc314148512b9")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","2278d986633254d5c3d7fe90b746c4bc9d736bfe")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","0304db5244b1692df723748f499ce8d2722ded45")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","2b32bb0993d2060a90c478e82e124e39fb68bb21")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","3080ba05280ae5b0a76ef283870864c16d1c7826")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","7405a2df9839a3473f6026ceff64630f602f0a19")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","c4370a95cfa28d4b4a539512ae378fe3c05f6e98")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","16f85fe78b24d7a79ccbdfaea71a102f4b59a41e")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","6936adeaa9a054a9530078c9a47e3fa94cf4bffa")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","eb08f35ad74c1124f3cd4ef3a8958cded6a04b0e")

os.chdir(curDir)
