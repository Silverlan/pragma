import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

external_libs_dir = curDir +"/external_libs"
os.chdir(external_libs_dir)
get_submodule("alsoundsystem","https://github.com/Silverlan/alsoundsystem.git","179fef153bd7d67ab1ae9b46d3bf07e7571c130b", "feat/cxx_module")
get_submodule("datasystem","https://github.com/Silverlan/datasystem.git","d074c2ec24547c8e6c677979250dd8d04ac0049f", "feat/cxx_module")
get_submodule("iglfw","https://github.com/Silverlan/iglfw.git","470ec20df6383fa12b5d03766576c90b4011de7d", "feat/cxx_module")
get_submodule("luasystem","https://github.com/Silverlan/luasystem.git","26a3388a790415e0c00fb3965c99d5af9d21be0a", "feat/cxx_module")
get_submodule("materialsystem","https://github.com/Silverlan/materialsystem.git","49bba10d5f97f872ada03222d452a2c4e639b419", "feat/cxx_module")
get_submodule("mathutil","https://github.com/Silverlan/mathutil.git","244b4bb2c4516d573705042c04c7af9f64f0663c", "feat/cxx_module")
get_submodule("networkmanager","https://github.com/Silverlan/networkmanager.git","492eb00e2912c2e8a83761dc600ce593382ed003", "feat/cxx_module")
get_submodule("oskit","https://github.com/Silverlan/oskit.git","574a522c0968d3dff5fdd70b64a0f2288aa42cbc", "feat/cxx_module")
get_submodule("panima","https://github.com/Silverlan/panima.git","9c952a8c0b847673648dc683781804f98eb7715d", "feat/cxx_module")
get_submodule("prosper","https://github.com/Silverlan/prosper.git","62ebb07143bdee5f689096db5d1e55caf0ff7672", "feat/cxx_module")
get_submodule("sharedutils","https://github.com/Silverlan/sharedutils.git","01a57de731dde6d7f256bf0b36c12319c5b63d28", "feat/cxx_module")
get_submodule("util_bsp","https://github.com/Silverlan/util_bsp.git","8b0e95931f6c27b728999d021cea80379abdc0e9", "feat/cxx_module")
get_submodule("util_formatted_text","https://github.com/Silverlan/util_formatted_text.git","e2035f72d9bb9282e9f0f8d32f08a3bbf4d449dc", "feat/cxx_module")
get_submodule("util_image","https://github.com/Silverlan/util_image.git","515705a6aec8492fa848eadca773b78be7b50bfc", "feat/cxx_module")
get_submodule("util_pad","https://github.com/Silverlan/util_pad.git","372d46b250a1d6407f1082ce10cd84fbcc17441c", "feat/cxx_module")
get_submodule("util_pragma_doc","https://github.com/Silverlan/util_pragma_doc.git","d5a5a2c8df93df38c96c0269db7c27bcedc7c301", "feat/cxx_module")
get_submodule("util_smdmodel","https://github.com/Silverlan/util_smdmodel.git","18ca447476d72e48885c86993849e97ca0a32a18", "feat/cxx_module")
get_submodule("util_sound","https://github.com/Silverlan/util_sound.git","3ae58a7b3ccc2c89b07b8d6260f6db5feb5b9767", "feat/cxx_module")
get_submodule("util_source2","https://github.com/Silverlan/util_source2.git","1fa9bde579a0c99cc9c65b65a121dcd8e910d6df", "wip/cxx_module")
get_submodule("util_string","https://github.com/Silverlan/util_string.git","94e2928ddb44d59270926ed7e54fcd4d0be48d18", "feat/cxx_module")
get_submodule("util_source_script","https://github.com/Silverlan/util_source_script.git","d223be61bdbec2a10d05fb094bc31a68b493bb61", "feat/cxx_module")
get_submodule("util_timeline_scene","https://github.com/Silverlan/util_timeline_scene.git","f701e000be140dcfdcfc913629d3fa8ded781f3d")
get_submodule("util_udm","https://github.com/Silverlan/util_udm.git","aeda1970b0728a800e0fc091ae4509177ba242f9", "feat/cxx_module")
get_submodule("util_versioned_archive","https://github.com/Silverlan/util_versioned_archive.git","a5a73f4577809715a8056b33d4544f1052143991", "feat/cxx_module")
get_submodule("util_vmf","https://github.com/Silverlan/util_vmf.git","c9e72259901bb68fc10b45c84a1cd6d3eae8e4a0")
get_submodule("util_zip","https://github.com/Silverlan/util_zip.git","b0b14206da5b69c595e19112fb5ec2b7cd1d9fd6", "feat/cxx_module")
get_submodule("util_shadergraph","https://github.com/Silverlan/util_shadergraph.git","612d32255150568b5fa25e96c3207ad2890251d4")
get_submodule("vfilesystem","https://github.com/Silverlan/vfilesystem.git","9bc9b86ea330b1e4bdaf112a3452fb9e82a98f35", "feat/cxx_module")
get_submodule("wgui","https://github.com/Silverlan/wgui.git","efdb2ad79f96c6d796f03668b74d0d925ca8ecf0", "feat/cxx_modules2")
get_submodule("util_unicode","https://github.com/Silverlan/util_unicode.git","8fc1459fc4a562579725aa45793164520c4f6e66", "feat/cxx_module")
get_submodule("cppbezierfit","https://github.com/Silverlan/cppbezierfit.git","6c9f1a0111380a02ec08994653e1955d1122693a", "feat/cxx_module")

os.chdir(curDir)
