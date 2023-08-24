import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","4ffcd98")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","a6dfff6")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","4b9cf5c8f7f5e38e84c7e8529f956cd35897401e")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","e22584368ce0f9a5c0c223b8a632d968377d1ae3")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","6214d38")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","5aa9ec46b104b086ddfd090d1be0030c9f2788b5")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","68d2f5c")
get_submodule("panima","https://github.com/Silverlan/panima.git","0481a4ca7763a01d39768c7fb93cf02fb3ba5593")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","1062716010d53b5c350cad431fcc30b218ef3541")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","ea6e60be7f59c300484addba83ba41359671df9d")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","3c11053")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","3bd9e2de37d0cb14bf8228fde5e25c97698e927b")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","e22c12f")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","7a827f8")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","6a4a089")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","5e581ab")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","83693a562cab0c5a536f9ed81cb15100e53f827a")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","cf553d6")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","ea0d03a")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","76d02ff")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","575b1c29087aabc6c8a31e3b4a8a1e1511a7f807")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","be6d3fe")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","cdba99d")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","63b2609")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","9c22a7c113b0c2839e685a8de3546558a9993824")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","b2e8b6e9da357d99da11490a9b5bcc2dc952ea1c")

os.chdir(curDir)
