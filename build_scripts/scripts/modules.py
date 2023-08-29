import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","874295b")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","282dd1a")
get_submodule("pr_curl","https://github.com/Slaweknowy/pr_curl.git","7c724c2e8074663dd0d204a380e86cec794305ae")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","a268347")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","87a235e503550d431f862cfc6fe7916596406201")

os.chdir(curDir)
