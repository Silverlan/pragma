import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","eb2201c16c93a927711d9bd72030098619e7ddb5")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","b828ac4f8abdabebe1604e2a2bbc6115c6612041")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","0de1035d00d4a138316866d7bb74e13227232069")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","8ed27d16c3ff8706176d2b8e7581dc80425a1841")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","61ac7fefb4a286f515026940bb29cdf05eb6cea6")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","c697dec889b30ef224b742355195ec72fc91ab79")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","0b4c25fe35db71b057ad31fec76f50f04e2163fc")
get_submodule("oskit","https://github.com/Silverlan/oskit.git","77d4f8c353fd315f1255383ff4e02182dd882472")
get_submodule("panima","https://github.com/Silverlan/panima.git","1ee050cc64db898505e788299305540f151acd30")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","99acefb4e664511ccf1603d587337a995b7a64d6")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","043e965ec4356a7e5fd104a10f1ca59724d830f8")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","74a569cd8b9071d461abd76c62bdf5878c79bb12")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","1d982ac23b3ef5f8a7841e558ecb614ae865e574")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","08d90737e720a03b16838fc9942eda7c5b6dd087")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","9c31c05d62ea3a270411f130142560b90f2f226d")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","b04bb220ad8db49c0afa3c265bc14d33029d4912")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","d9b4d3e3fe7cf9eaaa53d6b4c78906a3302157d2")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","93b9389a36291dcf85d987a279ac4df7bf9aec78")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","2849e1e2b9803243abc52b79e1de19a7b7d92897")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","f5ee2fef2ff3bb64d0a5bd37d133bdf9f8fce9cc")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","456429ff129076fff4c6396a5809ea05fa82e267")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","b99a36f315acabb40e696773e24150b9f078e3fd")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","a9b767fe72a24e7407632f1dd842b14a6a081190")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","9817fd13e5a02999ff07ef430da15a970af5473d")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","52932696e9addd146046f51a69d4c897e1237161")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","49e63c9a1dedf5cc5a2ad04ed67607b94a2e9d8c")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","b3d123f670341a5724ca12b8195e3754c4d52f36")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","dd03cf7083d8f67489e823f86168a089ef87cf38")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","8ab57d1ceb17f31b4c5810d14f6355eb41b2e28d")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","f691045dfdb3313205a09ef4f460203be0a3152c")

os.chdir(curDir)
