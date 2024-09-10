import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","7daad8c34ab5000fc92b6f326b08fdc81b984cfe")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","7642296ea0f8316957e1123034e87034294f8b46")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","c5ea5319c17e206e8e418a2549a195e662f59dfd")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","b36cfa22c79906270dec09a0d882e92c07a12104")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","b5ef761c20f5f6afa573bee079340d6da4db8bd2")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","90a6cbb229cf3b2e7af1eb9bc58b3ff22db5a611")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","981bc5809c1a768267ddace778205e1be0262730")
get_submodule("panima","https://github.com/Silverlan/panima.git","08492a7bd8b214ff8472ae3288a32020aff1f60e")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","d6593906e9a938b2dd61f47057d050a26af4c31d")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","dc88bfc3d87f9405aa585be44799580f5565bbb6")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","7c49f7b9a59b1e9776a355881aeecede8a8f9be3")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","3fbb26580f732d0297cd6738f28636f6a1fd1f0c")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","901be63a3b9bb26dfe6670b1c5ba2c5025f2773a")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","49d78bd7725cc320fae67c6eff5c25524d1363c0")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","71b9593872633adafa8dada0797bdf4fabf74c4f")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","10fb2d10eb62188c8b9639a62da646152277a748")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","dcc284ca1affd33c9964ff1646e7aa7cf902eb96")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","2d45b2c76921475b61647ea63aeb180b37a17d44")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","eb71bb79736ccc693832c98d69069f3ffbbcb897")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","9e5c0b95dda605d3d65e11ab3605e6de5c80641f")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","c8cf73d53d45398409d4f976f8a3dbd04e3b5fc1")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","77531a4e93ded49dc8e5fe402db4198ab9aaa369")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","3080ba05280ae5b0a76ef283870864c16d1c7826")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","c8a13c03a35a8f44138dd81e35eecd19b7dd538a")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","25c839e313b1bb1c2b4dee56c4a0298c53f546b4")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","bfa72cae6258f1233c60e5fd9ecae1bbf53ee7e7")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","7d02aa0e66d043ce139c0f7f3885b59e6a398fb7")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","eb08f35ad74c1124f3cd4ef3a8958cded6a04b0e")

os.chdir(curDir)
