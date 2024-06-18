import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","48c1b84f2245324e90871924e4f606f846197818")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","921b0bfb2df4e17d7dab439cce9066e71fa4cdd3")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","5d09bb6ed75fc1d6217069bad759c10c425d4d7b")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","0b2676a6213c209e65ba7196d20f855776e72b21")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","8f67850e5d4f4f6206a4e217d03d4277af464764")

os.chdir(curDir)
