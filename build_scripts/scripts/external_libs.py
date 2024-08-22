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
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","42f5b7ec887562f28820222f542db7ef0bd902dd")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","90a6cbb229cf3b2e7af1eb9bc58b3ff22db5a611")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","981bc5809c1a768267ddace778205e1be0262730")
get_submodule("panima","https://github.com/Silverlan/panima.git","08492a7bd8b214ff8472ae3288a32020aff1f60e")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","37b3c30e0690ec98c72f8f13a431d8fb804e61ef")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","b9703eef0b4510d1b631cb501517144da8922572")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","227b126bdd26a78164946b4b49332b841122e551")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","3fbb26580f732d0297cd6738f28636f6a1fd1f0c")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","901be63a3b9bb26dfe6670b1c5ba2c5025f2773a")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","49d78bd7725cc320fae67c6eff5c25524d1363c0")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","71b9593872633adafa8dada0797bdf4fabf74c4f")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","1fae16eb43b1f403be7a5f6dce646fbfa84de8c0")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","dcc284ca1affd33c9964ff1646e7aa7cf902eb96")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","2d45b2c76921475b61647ea63aeb180b37a17d44")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","f56f97c347e7829f5053fb5b22cbc314148512b9")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","2278d986633254d5c3d7fe90b746c4bc9d736bfe")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","8213f4c27aba72e683978a1a00101e8dcd59f50b")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","77531a4e93ded49dc8e5fe402db4198ab9aaa369")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","3080ba05280ae5b0a76ef283870864c16d1c7826")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","1f13d86fef96462248de222d7978a078a38efe00")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","25c839e313b1bb1c2b4dee56c4a0298c53f546b4")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","525f76a73545c4bbe31a425ca6ec9a65122797e8")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","7d02aa0e66d043ce139c0f7f3885b59e6a398fb7")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","eb08f35ad74c1124f3cd4ef3a8958cded6a04b0e")

os.chdir(curDir)
