import subprocess
import os
import pathlib
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

reset_to_commit("7d64c84ff2ff4bb88400c9374eb2dca588d54e03")

subprocess.run(["../luamake/luamake", "lua", "compile/download_deps.lua"])

# Temporary fix until https://github.com/fesily/gumpp/pull/2 has been merged
patch_path = str(pathlib.Path(os.path.abspath(__file__)).parent / "gumpp_hotfix.patch")
header_path = str(pathlib.Path(deps_dir) / "lua-debug/3rd/frida_gum/gumpp/")

print("Applying gumpp.hpp patch...")
cur_dir = os.getcwd()
os.chdir(header_path)
subprocess.run(["git","apply",patch_path],check=False)
os.chdir(cur_dir)

subprocess.run(["../luamake/luamake", "-mode", "release"])
