import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","cf9d6139d3a6812333cc991cceff204a9927d6d1")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","7642296ea0f8316957e1123034e87034294f8b46")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","ebf72bc34e645abdb2f2c245861bbda1b3b87eaf")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","8eef5fe5593f9b9496d2d03158ecc1f485a34d7d")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","9b8ce309a2a5f3e53fed7909f8ade9927ebdc583")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","e872f599805cf696ff6a84a4253fc44ae8e83a15")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","981bc5809c1a768267ddace778205e1be0262730")
get_submodule("panima","https://github.com/Silverlan/panima.git","06916dd30cde319f31b1eee25cfed7dea8f14630")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","a9b568c360fc06d8159215e3f0dd84f595d1fbce")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","c32209f7cde687d89aa3f516d3ef0e3977918f6f")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","2d912cceaaa59199a86431aa9d194e922b2ebea4")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","c473a2bdc1ad84ef52d391226d6983ef3076958e")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","72269cdc0be30e8498626659d3f46e4731eeadb2")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","17b760636565502c82c751fb0243b455f1deb044")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","88e85503b120f820143f6f4566e194b539297ffa")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","10fb2d10eb62188c8b9639a62da646152277a748")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","729915ed7eaa7c9669a57c4e0d3dc4bf02a62cea")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","1c992e07b0bfb3f312182518dcc2e726ce4da25e")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","eb71bb79736ccc693832c98d69069f3ffbbcb897")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","8cac3eae6f4bc892e2ea229a97261385651a90f6")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","6c70fdf885a33054d2c61f5d420927554275865b")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","e8db74a6ad9ed44d7e8a4492eb8fe44eae3452f6")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","4e477b1425e1a7b9898975effaf527f661e792da")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","d9bf05a5cbf71bf53f9fbea82c7352f870989ed1")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","6128eccd5f289dce0521f6dbc9fb736af665cdf5")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","a150f67ab21993715d116bc31faaa2da5923e37e")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","d2fb58bd5a07285d6c103389e32b6e669c39302b")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","5a0ac6c02f199e42d7d38d99231503cf42e26f8a")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","b1f76c0187411230ebe59fa606292b0947e730b6")

os.chdir(curDir)
