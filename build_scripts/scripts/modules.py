import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_gh_submodule("interfaces",          "Silverlan/pragma_interfaces",  "bf5b81c8c5ae8fff6394dbc52906e793b1b50c43")

os.chdir(curDir)
