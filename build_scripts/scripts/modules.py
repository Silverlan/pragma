import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","fc446d4a1e7e3820a8a648b0a0bb84e8b9d362f8")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","1a806a1a7b2283bd8551d07e4f1d680499f68b90")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","974c67cc76710809a9595fcfbc4167554799cd7f")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","e34cbca22a0a6ebd8a5d7f85c29f91e99555857a")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","d71e88cc072963c21f560d74dfb36e1cb28bce00")

os.chdir(curDir)
