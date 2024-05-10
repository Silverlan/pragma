import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","874295b")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","f944362b82ef8d1c80c1ae2fe18785eb92ad32fe")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","025c6d150ba88031f1b7b9a1bcc387b746e1ac89")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","a268347")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","4e1f498386511a1370b4da1d1f466d7784b03cba")

os.chdir(curDir)
