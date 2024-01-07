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

reset_to_commit("0bf6041")

if platform == "win32":
    subprocess.run([".\compile\install.bat", "msvc"])
else:
    subprocess.run(["./compile/install.sh", "other"])

os.chdir("..")
