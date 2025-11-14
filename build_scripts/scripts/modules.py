import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_gh_submodule("interfaces",          "Silverlan/pragma_interfaces",  "0b1e1b502384f91f4458a0e369a8851fb933ee77", "feat/cxx_module2")
get_gh_submodule("pr_audio_dummy",      "Silverlan/pr_audio_dummy",     "96973bd6fd5542d827fccaf811f677b3f8477f2f", "feat/cxx_module")
get_gh_submodule("pr_curl",             "Silverlan/pr_curl",            "daa230f4024a6a9a9fd22744c7d93d826ac07e68", "feat/cxx_module")
get_gh_submodule("pr_prosper_opengl",   "Silverlan/pr_prosper_opengl",  "11db43a6878ec0d0fd285a2033ae72ee81a077f0", "feat/cxx_module")
get_gh_submodule("pr_prosper_vulkan",   "Silverlan/pr_prosper_vulkan",  "29c18e2645cdca083a775b7a0d743d3c05263e42", "feat/cxx_module")

os.chdir(curDir)
