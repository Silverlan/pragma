import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","4fa3070334aa71509fb89435c0b864074ace15b7", "feat/cxx_module")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","d074c2ec24547c8e6c677979250dd8d04ac0049f", "feat/cxx_module")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","632cae75f435387c271735b70e06aeb743a19abd", "feat/cxx_module")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","a70cdf37d7e9dbfb0c5ed7f617b383bf4b836823", "feat/cxx_module")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","0978e2f882c0615b42897f4ad64c16a1f10a9ab8", "feat/cxx_module")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","244b4bb2c4516d573705042c04c7af9f64f0663c", "feat/cxx_module")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","969fd5fb9eab6cc329e7a68d468366ff862dfc3a", "feat/cxx_module")
get_submodule("oskit","https://github.com/Silverlan/oskit.git","a8ca578f44099349bfb0a9017dbf880f95e261c3", "feat/cxx_module")
get_submodule("panima","https://github.com/Silverlan/panima.git","9c952a8c0b847673648dc683781804f98eb7715d", "feat/cxx_module")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","857496350b7c926d424f5151a44330249512fbcf", "feat/cxx_module")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","99b4246908f7a35c9170c24334afe4d4b03a5412", "feat/cxx_module")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","29da4cb6a0ed5471774765bf54873adbd50ff343", "feat/cxx_module")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","10b58bd5dd08c8d321eb02dab60cfa3cd9c73ebc", "feat/cxx_module")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","6929076c000954f66d2f4fea8496d4c99578be07", "feat/cxx_module")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","4f1a10ce151ea036a59a6d23739b1050c698528d", "feat/cxx_module")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","ad8e9f8908070e45c8ceccba2db27e64c1acc313", "feat/cxx_module")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","e57ee9da05fe907620b7c016db85a5de7be938d0", "feat/cxx_module")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","274bcbad351ce46ca7456c384c20e98a72b97910", "feat/cxx_module")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","e0571f1ab54bfbce907caa2383e203b0214b6ede", "wip/cxx_module")
get_submodule("util_string","https://github.com/Silverlan/util_string.git","94e2928ddb44d59270926ed7e54fcd4d0be48d18", "feat/cxx_module")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","86961fd12513a257968b63efbc229fedf20091fd", "feat/cxx_module")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","456429ff129076fff4c6396a5809ea05fa82e267")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","aeda1970b0728a800e0fc091ae4509177ba242f9", "feat/cxx_module")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","23275d3626e46fa0b13709d9c20636ff1f0687db", "feat/cxx_module")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","4faf4f9fefe0d06f65b307f8fbd6cb5e08a688fa")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","da9b3f18fd641a5107f0da3edfcae3f28b9ae994", "feat/cxx_module")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","2562b83e1b663967916e106d7b0667401a0d5f73")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","359684a3020c2b18a9c2db2ccf24d02587b85292", "feat/cxx_module")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","f8dbff7a88ed3cff9a0b991152e89964cac2d519", "feat/cxx_modules2")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","d3d5cf3e23d41bbfd3a90c9014904355c895b320", "feat/cxx_module")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","6c9f1a0111380a02ec08994653e1955d1122693a", "feat/cxx_module")

os.chdir(curDir)
