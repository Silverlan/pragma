import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","4dc79d3180bdf62ab63043efac797409309d63a8")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","a6dfff6")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","e0531a55f5bdd9c9a1dadfbb7a8a610474d69d19")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","e22584368ce0f9a5c0c223b8a632d968377d1ae3")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","d900667907c42e64d70a9e49eab16fcb39073089")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","64e456ca0f7eb7037fd8528812c0188caeefe4fd")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","68d2f5c")
get_submodule("panima","https://github.com/Silverlan/panima.git","d4f07c33d89cc6d59875c14260565dcaab9b9488")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","d229b0598b99d039b77329ed92c270936bb98297")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","dacb0b17bfebb8259082686b468e39bf9021a271")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","3c11053")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","3bd9e2de37d0cb14bf8228fde5e25c97698e927b")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","7a66f93")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","7a827f8")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","6a4a089")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","5e581ab")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","83693a562cab0c5a536f9ed81cb15100e53f827a")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","cf553d6")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","ea0d03a")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","76d02ff")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","bfa81d76858d3c6e0e745066dde7fa96e73cba8c")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","be6d3fe")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","cdba99d")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","c484726740580b6a6583a13e23e4a3136a8fd815")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","28dfb77eeedeb001d08ef4bcb95c9da0ca4f5918")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","6a9cedd95e34b9ebf7f9ddf3f4344e4d5d22d3d4")

os.chdir(curDir)
