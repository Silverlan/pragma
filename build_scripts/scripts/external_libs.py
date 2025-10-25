import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","4fa3070334aa71509fb89435c0b864074ace15b7", "feat/cxx_module")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","5052591181fb77a5355ca4a682e9a9e25de41c99", "feat/cxx_module")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","3f2b2596ce601fb07ddb4f90d482d24e9fa8bd72", "feat/cxx_module")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","88f594780c634a7217bf0d5da5eeba3d4228058a", "feat/cxx_module")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","ac7300d8a1a7544d6a94cf30296d621c21d7bfaa", "feat/cxx_module")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","123554c031827ee6b54f1e70d4937eeea2c6b526", "feat/cxx_module")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","969fd5fb9eab6cc329e7a68d468366ff862dfc3a", "feat/cxx_module")
get_submodule("oskit","https://github.com/Silverlan/oskit.git","a8ca578f44099349bfb0a9017dbf880f95e261c3", "feat/cxx_module")
get_submodule("panima","https://github.com/Silverlan/panima.git","9c952a8c0b847673648dc683781804f98eb7715d", "feat/cxx_module")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","857496350b7c926d424f5151a44330249512fbcf", "feat/cxx_module")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","1d97164ad146e8953b780c19b379fff55696b6d6", "feat/cxx_module")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","29da4cb6a0ed5471774765bf54873adbd50ff343", "feat/cxx_module")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","10b58bd5dd08c8d321eb02dab60cfa3cd9c73ebc", "feat/cxx_module")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","6929076c000954f66d2f4fea8496d4c99578be07", "feat/cxx_module")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","4f1a10ce151ea036a59a6d23739b1050c698528d", "feat/cxx_module")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","ad8e9f8908070e45c8ceccba2db27e64c1acc313", "feat/cxx_module")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","e57ee9da05fe907620b7c016db85a5de7be938d0", "feat/cxx_module")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","274bcbad351ce46ca7456c384c20e98a72b97910", "feat/cxx_module")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","e0571f1ab54bfbce907caa2383e203b0214b6ede", "wip/cxx_module")
get_submodule("util_string","https://github.com/Silverlan/util_string.git","10c448f17749dda7715328f915660ef52e3f7997", "feat/cxx_module")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","86961fd12513a257968b63efbc229fedf20091fd", "feat/cxx_module")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","456429ff129076fff4c6396a5809ea05fa82e267")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","3ef78f3b3460527abc755185cba29fb512ffeee7", "feat/cxx_module")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","23275d3626e46fa0b13709d9c20636ff1f0687db", "feat/cxx_module")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","4faf4f9fefe0d06f65b307f8fbd6cb5e08a688fa")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","da9b3f18fd641a5107f0da3edfcae3f28b9ae994", "feat/cxx_module")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","2562b83e1b663967916e106d7b0667401a0d5f73")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","5114c4a4eb55c6e99b67df505b61f8408687ec58", "feat/cxx_module")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","7e64fac2bafa9af61a71e4e14ad33f533e844291", "feat/cxx_modules2")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","d3d5cf3e23d41bbfd3a90c9014904355c895b320", "feat/cxx_module")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","3ecdf855eb967cad7b997d85af4252a5a9c3b4f7", "feat/cxx_module")

os.chdir(curDir)
