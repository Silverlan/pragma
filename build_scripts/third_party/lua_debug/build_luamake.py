import subprocess
import os
import sys
from sys import platform

deps_dir = sys.argv[1]
os.chdir(deps_dir)

subprocess.run(["git", "clone", "https://github.com/actboy168/luamake"])
os.chdir("luamake")
subprocess.run(["git", "submodule", "init"])
subprocess.run(["git", "submodule", "update"])

def reset_to_commit(sha):
	subprocess.run(["git","fetch"],check=True)
	subprocess.run(["git","checkout",sha,"--recurse-submodules"],check=True)

reset_to_commit("a83fa224d083ad5ee4d39367621ee8c8c8db379b")

if platform == "win32":
    subprocess.run([".\\compile\\install.bat", "notest"])
else:
    subprocess.run(["./compile/install.sh", "notest"])

os.chdir("..")
