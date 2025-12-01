import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_gh_submodule("interfaces",          "Silverlan/pragma_interfaces",  "b6a15e112de4d3549b2d8591b8e3614feccd2c0f")

os.chdir(curDir)
