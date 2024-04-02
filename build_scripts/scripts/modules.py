import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","874295b")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","f944362b82ef8d1c80c1ae2fe18785eb92ad32fe")
get_submodule("pr_curl","https://github.com/Slaweknowy/pr_curl.git","1e35c603ad2211476a8498f7dbded470d1b85cd9")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","a268347")
get_submodule("pr_prosper_vulkan","https://github.com/Slaweknowy/pr_prosper_vulkan.git","f59f11c54e942155fedc4243960ab7c487c49204")

os.chdir(curDir)
