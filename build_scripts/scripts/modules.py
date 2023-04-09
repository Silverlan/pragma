import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git")

os.chdir(curDir)
