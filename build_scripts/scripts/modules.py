import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","c584b9535df51cbd9161686eda63046dfe99f1b3")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","6fff8474681ffac9f365b068c20a93f4479162f1")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","c243468a77de16ae4657e257ea965d1cac05fe8e")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","d7d80184ed7b94657a92c793d3d5c79efdaf0197")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","cea2de3151521e364efddfb506b78fe782b23464")

os.chdir(curDir)
