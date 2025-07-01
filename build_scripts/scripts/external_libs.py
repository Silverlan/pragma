import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","6eb1882536fcaf22a747eb5896289cc86c6ba443")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","a556fbbeda753330b69c1825617f00ae242bf2aa")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","6448a1f19a6a7e379fb59b9db65a7e1ed191758a")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","72772909f7ed3166593f781d3986d22329fd2d63")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","ce8600fb8ffbda60a33e10531b16905ae3023145")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","554d67f7b621fe87fecea90269581c2ca50dadb9")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","116e1e8ce74b5e93784b4ddc6208ba747c12d76b")
get_submodule("panima","https://github.com/Silverlan/panima.git","e817c35fc85549eb27d599547e6f36b2cda891d4")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","faa8c5de108fbeb6674177102d74ad0d11942655")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","8675c8c9fa1c734779a3fe02d827f92556f0b9a4")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","4371245639953bb33b85c887fcefba03f38f9efe")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","a60a430ed17897a1e5834b21b1f76051ca71051d")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","9ad358a153e528c64eaac51bfce8db1286a9d92e")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","e9eb7688b6c80f978d7285e8e2b26c97cd9a4aca")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","88e85503b120f820143f6f4566e194b539297ffa")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","10fb2d10eb62188c8b9639a62da646152277a748")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","93b9389a36291dcf85d987a279ac4df7bf9aec78")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","7060216021d4ad145e771b7e95744333170bcb12")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","eb71bb79736ccc693832c98d69069f3ffbbcb897")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","8cac3eae6f4bc892e2ea229a97261385651a90f6")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","8cf12e3087e5fd4c9b86fff62cd566b7958a8d75")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","479ded2208bb24421ba70777b570dd8fda05bdda")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","6dedf4a8543544f1d8d4d2376b9b14cd1afd913a")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","a1e2c33a85241644260bf04494f6fe9f0ba52fda")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","15231d501ca893d077c1a1de5cd173daf94fc1e5")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","93d12615d08f57476a896111ba1b48e32dd369f2")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","921c6ce1eb1598848b5a80ab8dc5f521ef48f8e1")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","ee32acc4bcaac757c91ac4a0666c99b8333992ad")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","f691045dfdb3313205a09ef4f460203be0a3152c")

os.chdir(curDir)
