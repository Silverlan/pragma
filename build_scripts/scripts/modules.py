import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_gh_submodule("interfaces",          "Silverlan/pragma_interfaces",  "0b1e1b502384f91f4458a0e369a8851fb933ee77", "feat/cxx_module2")

os.chdir(curDir)
