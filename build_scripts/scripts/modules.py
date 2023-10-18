import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","874295b")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","282dd1a")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","150e9d0c3d5116745fb83a7da28eb32666125347")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","a268347")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","0b8ba03ddafbc720b352b0436055758ca8528283")

os.chdir(curDir)
