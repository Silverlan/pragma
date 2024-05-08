import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","4dc79d3180bdf62ab63043efac797409309d63a8")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","a6dfff6")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","e0531a55f5bdd9c9a1dadfbb7a8a610474d69d19")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","a93cc69e611d928e6f8d80c68c7a79afece06ba5")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","7b1f9e8d27903bf9779b9f5e1bc938621ed62112")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","bec60852c5006e4b4b62bd15dda6bb5af2e75ee5")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","68d2f5c")
get_submodule("panima","https://github.com/Silverlan/panima.git","ea32d806012d25a93680aed9ccbdc0d2ebeec9a4")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","b9281968ebce912f3c18dd893be4a9fd181d99d2")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","616eb852416df1c2e81d70c6f2fce1e513f2d1c3")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","3c11053")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","3bd9e2de37d0cb14bf8228fde5e25c97698e927b")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","7a66f93")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","7a827f8")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","6a4a089")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","5e581ab")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","83693a562cab0c5a536f9ed81cb15100e53f827a")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","cf553d6")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","ea0d03a")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","76d02ff")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","c7862e7eb8a5ef7229161b88cda9a253cfd5077e")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","be6d3fe")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","cdba99d")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","166f108587962b59391a37e14dcfe1a51f81a81b")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","ba2db128eec7a23cb65c67b396d77f2d9a93e948")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","f1d6e46db3f66209b428729520fed343fa3d900c")

os.chdir(curDir)
