import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_gh_submodule("interfaces",          "Silverlan/pragma_interfaces",  "f4e38a9ead15f74a757238e05f6bbc0f6b76df1c")

os.chdir(curDir)
