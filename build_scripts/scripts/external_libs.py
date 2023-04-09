import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git")
get_submodule("panima","https://github.com/Silverlan/panima.git")
get_submodule("prosper","https://github.com/Silverlan/prosper.git")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git")
get_submodule("util_image","https://github.com/Silverlan/util_image.git")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git")
get_submodule("wgui","https://github.com/Silverlan/wgui.git")

os.chdir(curDir)
