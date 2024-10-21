import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","48c1b84f2245324e90871924e4f606f846197818")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","1a806a1a7b2283bd8551d07e4f1d680499f68b90")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","974c67cc76710809a9595fcfbc4167554799cd7f")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","d73bf6dea11b1a79d5dc4715e224aa4cb15d0d48")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","9a7475a881d006e421f4c729f7009c140a5e5866")

os.chdir(curDir)
