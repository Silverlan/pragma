import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","13e89b33ccd3d9ccacd38e2e69e8cb53f04224f2", "feat/cxx_module2")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","6fff8474681ffac9f365b068c20a93f4479162f1")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","40d0fbdcd50b3aec48d6da29e880891759e79638", "feat/cxx_module")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","d7d80184ed7b94657a92c793d3d5c79efdaf0197")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","29c18e2645cdca083a775b7a0d743d3c05263e42", "feat/cxx_module")

os.chdir(curDir)
