import subprocess
import os
import sys

deps_dir = sys.argv[1]
os.chdir(deps_dir)

subprocess.run(["git", "clone", "https://github.com/actboy168/lua-debug"])
os.chdir("lua-debug")
subprocess.run(["git", "submodule", "init"])
subprocess.run(["git", "submodule", "update"])

def reset_to_commit(sha):
	subprocess.run(["git","fetch"],check=True)
	subprocess.run(["git","checkout",sha,"--recurse-submodules"],check=True)

reset_to_commit("693549d")

subprocess.run(["../luamake/luamake", "lua", "compile/download_deps.lua"])
subprocess.run(["../luamake/luamake", "-mode", "release"])
