import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_gh_submodule("interfaces",          "Silverlan/pragma_interfaces",  "26b10cc3d9200944b1458cc35e53cb2e59e178b4")

os.chdir(curDir)
