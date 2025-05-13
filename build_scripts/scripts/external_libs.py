import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","cf9d6139d3a6812333cc991cceff204a9927d6d1")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","209e2f3f991572801e6fcc0a9f74be7796a60d45")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","a90f59a2f3c3638c43c4359444721ed0cb0894d0")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","8eef5fe5593f9b9496d2d03158ecc1f485a34d7d")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","7a4123878e02257098d833018bd7acdcb8fa102f")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","554d67f7b621fe87fecea90269581c2ca50dadb9")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","d9805d99be996ac9b81f81ca48b630b765a5bb67")
get_submodule("panima","https://github.com/Silverlan/panima.git","1718348")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","60c4356e36db9e58e86955b3ab36595b80fbe7cb")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","0c04de918c8c1e936139eeeca275980bfd920df9")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","2d912cceaaa59199a86431aa9d194e922b2ebea4")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","62a19a1cac7527cba499a941757ac49ab68ec6fb")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","e75b48608a63c27df88c1a4ffa46a184190499ad")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","17b760636565502c82c751fb0243b455f1deb044")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","88e85503b120f820143f6f4566e194b539297ffa")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","10fb2d10eb62188c8b9639a62da646152277a748")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","93b9389a36291dcf85d987a279ac4df7bf9aec78")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","1c992e07b0bfb3f312182518dcc2e726ce4da25e")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","eb71bb79736ccc693832c98d69069f3ffbbcb897")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","8cac3eae6f4bc892e2ea229a97261385651a90f6")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","6c70fdf885a33054d2c61f5d420927554275865b")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","479ded2208bb24421ba70777b570dd8fda05bdda")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","ae1c498c20ff0ccff4a0245c6cc9b9a59561881b")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","71205cd")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","15231d501ca893d077c1a1de5cd173daf94fc1e5")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","7009bfb2d8233702493865bc5d78a59d621dc358")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","3aeadf0025eddf8a7cc3421c309b9aad67a37789")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","5a0ac6c02f199e42d7d38d99231503cf42e26f8a")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","b1f76c0187411230ebe59fa606292b0947e730b6")

os.chdir(curDir)
