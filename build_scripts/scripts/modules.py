import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","48c1b84f2245324e90871924e4f606f846197818")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","84e1249c296dfc6cf288c8262142170851286f48")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","974c67cc76710809a9595fcfbc4167554799cd7f")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","4dcc3045f37da63456f2a3ddcf66490497b87106")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","931374499f4a07bdb016ec616158dbdc1d4da6f3")

os.chdir(curDir)
