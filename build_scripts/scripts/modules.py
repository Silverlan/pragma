import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","48c1b84f2245324e90871924e4f606f846197818")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","f944362b82ef8d1c80c1ae2fe18785eb92ad32fe")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","4f2a574a67439104290388193cce699b64f1364d")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","a268347")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","899d164f5f2af5368cecd489274c04fb514370ce")

os.chdir(curDir)
