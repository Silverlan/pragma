import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","c584b9535df51cbd9161686eda63046dfe99f1b3")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","96e5b0359a770a640ebe1801a2babd888e2e8329")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","bb2ea373963de3d291a259dc0302ae4c8c5cb4cd")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","0635a9ada3e95511a493bc0e97d4dda570d6dbea")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","c580a9d194a875cef0ee1ee51850836e4e2bda05")

os.chdir(curDir)
