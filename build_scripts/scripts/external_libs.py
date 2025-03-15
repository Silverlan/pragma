import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","cf9d6139d3a6812333cc991cceff204a9927d6d1")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","3e38f8da1d8b8a3f8075de9a84881130678bbf0a")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","473bb63")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","8eef5fe5593f9b9496d2d03158ecc1f485a34d7d")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","3300f997e2cd0b7b21151687abac829e271726aa")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","554d67f7b621fe87fecea90269581c2ca50dadb9")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","981bc5809c1a768267ddace778205e1be0262730")
get_submodule("panima","https://github.com/Silverlan/panima.git","1718348")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","c0d585ea8507b68ad2c9f00b8152211830479630")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","556a0f5d80ff64c3bd3931b45a2d280668314f68")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","2d912cceaaa59199a86431aa9d194e922b2ebea4")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","62a19a1cac7527cba499a941757ac49ab68ec6fb")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","72269cdc0be30e8498626659d3f46e4731eeadb2")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","17b760636565502c82c751fb0243b455f1deb044")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","88e85503b120f820143f6f4566e194b539297ffa")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","10fb2d10eb62188c8b9639a62da646152277a748")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","93b9389a36291dcf85d987a279ac4df7bf9aec78")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","1c992e07b0bfb3f312182518dcc2e726ce4da25e")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","eb71bb79736ccc693832c98d69069f3ffbbcb897")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","8cac3eae6f4bc892e2ea229a97261385651a90f6")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","6c70fdf885a33054d2c61f5d420927554275865b")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","e8db74a6ad9ed44d7e8a4492eb8fe44eae3452f6")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","ae1c498c20ff0ccff4a0245c6cc9b9a59561881b")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","6b1695545f66ec19db8b81ca081f8502358b823f")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","15231d501ca893d077c1a1de5cd173daf94fc1e5")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","6663b7b90b927febd14eee641a791766ad8b55d3")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","26fd4f3077878ddd4d8041bfb2222cf19ddf94e1")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","5a0ac6c02f199e42d7d38d99231503cf42e26f8a")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","b1f76c0187411230ebe59fa606292b0947e730b6")

os.chdir(curDir)
