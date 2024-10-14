import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","cf9d6139d3a6812333cc991cceff204a9927d6d1")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","7642296ea0f8316957e1123034e87034294f8b46")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","0a32680dcf085f4541252110229439125e35c3c6")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","b36cfa22c79906270dec09a0d882e92c07a12104")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","fc162908056a67e8a3ae9508a0eb1cbdd7cf6447")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","90a6cbb229cf3b2e7af1eb9bc58b3ff22db5a611")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","981bc5809c1a768267ddace778205e1be0262730")
get_submodule("panima","https://github.com/Silverlan/panima.git","c133a82e4acabfe88c3ae45c482bd743229e6f18")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","ce6533ccbd9dd0f276c01e747fc7ae089fb10b6f")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","feceb9a138150177c2d9fd8e29411fcba8399b8e")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","2d912cceaaa59199a86431aa9d194e922b2ebea4")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","c473a2bdc1ad84ef52d391226d6983ef3076958e")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","bc1af9d7a0e4e4e51550a4730d65e072ae97d592")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","17b760636565502c82c751fb0243b455f1deb044")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","88e85503b120f820143f6f4566e194b539297ffa")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","10fb2d10eb62188c8b9639a62da646152277a748")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","729915ed7eaa7c9669a57c4e0d3dc4bf02a62cea")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","1c992e07b0bfb3f312182518dcc2e726ce4da25e")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","eb71bb79736ccc693832c98d69069f3ffbbcb897")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","8cac3eae6f4bc892e2ea229a97261385651a90f6")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","c8cf73d53d45398409d4f976f8a3dbd04e3b5fc1")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","e8db74a6ad9ed44d7e8a4492eb8fe44eae3452f6")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","4e477b1425e1a7b9898975effaf527f661e792da")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","d9bf05a5cbf71bf53f9fbea82c7352f870989ed1")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","25c839e313b1bb1c2b4dee56c4a0298c53f546b4")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","66d3758336d6e58046813a30298a6a3d9af74318")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","5a0ac6c02f199e42d7d38d99231503cf42e26f8a")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","4d4c89e25835b4c9be0d9aa53f273fec47f4d621")

os.chdir(curDir)
