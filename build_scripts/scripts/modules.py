import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","c0ec4a56b4fd5abdf42eead29836df2270d69cd2")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","1a806a1a7b2283bd8551d07e4f1d680499f68b90")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","bb814eb232b9724720e2317d1e19bf5803fab48c")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","e34cbca22a0a6ebd8a5d7f85c29f91e99555857a")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","37826be12406a491050c2cc7d5a8d0a29d023fe1")

os.chdir(curDir)
