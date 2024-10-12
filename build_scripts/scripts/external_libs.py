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
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","b5cac074a5dd59c47968a3fcf120e019b51297c1")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","90a6cbb229cf3b2e7af1eb9bc58b3ff22db5a611")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","981bc5809c1a768267ddace778205e1be0262730")
get_submodule("panima","https://github.com/Silverlan/panima.git","08492a7bd8b214ff8472ae3288a32020aff1f60e")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","6a76fbffc7c87ec26d882a3eaceb11598eecf177")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","feceb9a138150177c2d9fd8e29411fcba8399b8e")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","f08aae384bab99373a6d6bf67ffa1a7b11697599")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","c473a2bdc1ad84ef52d391226d6983ef3076958e")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","bc1af9d7a0e4e4e51550a4730d65e072ae97d592")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","d3e649427bc8109814920e9b38ecf413f0e6075c")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","88e85503b120f820143f6f4566e194b539297ffa")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","10fb2d10eb62188c8b9639a62da646152277a748")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","729915ed7eaa7c9669a57c4e0d3dc4bf02a62cea")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","3ed4a4e751cab9c1f24427fa3033252a8db5a84f")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","eb71bb79736ccc693832c98d69069f3ffbbcb897")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","8cac3eae6f4bc892e2ea229a97261385651a90f6")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","c8cf73d53d45398409d4f976f8a3dbd04e3b5fc1")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","f2194a6445094b2fb0c3d350961cc46810e6fc7d")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","4f88c8994708b01b8a57c7b297b30f0c83cb4b91")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","d9bf05a5cbf71bf53f9fbea82c7352f870989ed1")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","25c839e313b1bb1c2b4dee56c4a0298c53f546b4")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","66d3758336d6e58046813a30298a6a3d9af74318")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","5a0ac6c02f199e42d7d38d99231503cf42e26f8a")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","eb08f35ad74c1124f3cd4ef3a8958cded6a04b0e")

os.chdir(curDir)
