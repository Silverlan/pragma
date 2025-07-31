import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","9aa413861327d0ee8efad9c912a3a11e1c4d6501")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","d813ff2e0ad77f9bb5c05a4a0852c9ad53b128e1")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","120e25e46296b8c74933602643c644a7e0c889fc")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","8c3ed08c20c05a3b4a27e9906627db8a00687eeb")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","33b3400474af587fe6a1ce80f8ed07627b3b8cd8")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","c697dec889b30ef224b742355195ec72fc91ab79")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","3384b867ad5e5610cf69b8dbdd469cfb04c24567")
get_submodule("panima","https://github.com/Silverlan/panima.git","1267b03ac8378a8a81c30eae4e8f157d163a0aa5")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","74e5643ff1384087e15800df93b4aabb2836f1fe")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","e40dbfb405cb4fc3cfd2ddc571adfe971ac52009")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","74a569cd8b9071d461abd76c62bdf5878c79bb12")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","a25fea8d7d19c5b98eca93160a59eb6433e4e81a")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","8e830e14cf1a4faed4650726697b917779911062")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","4e09825f16f53274be991b7a441586b68bc406f9")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","b04bb220ad8db49c0afa3c265bc14d33029d4912")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","4d8fc8c0e2766934f00fd49d4ca36e0787f61f32")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","93b9389a36291dcf85d987a279ac4df7bf9aec78")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","2849e1e2b9803243abc52b79e1de19a7b7d92897")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","e09621b5e6bf9fdd29f1dc0d6d61d3213f38ed48")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","456429ff129076fff4c6396a5809ea05fa82e267")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","b425a3b8d8216b969228c25ae12699bd032a29bf")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","e3c610bd4faca1b33a3f11381a3ddb03a4defbdc")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","85140d4d48fc57fb104a8404acbafb30fe3ec3cf")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","fbf0c3454937b782f2aa9ce6908624f43e08a582")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","49e63c9a1dedf5cc5a2ad04ed67607b94a2e9d8c")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","47876c3e8c1fc168480db7f6400e9d9c03327701")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","038c2a2354484f1e69a54f8e7eabc6e1ea6f1061")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","8ab57d1ceb17f31b4c5810d14f6355eb41b2e28d")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","f691045dfdb3313205a09ef4f460203be0a3152c")

os.chdir(curDir)
