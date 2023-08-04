import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

# These are required for Lua debugging with Visual Studio Code
print_msg("Building luamake...")
curDir = os.getcwd()
os.chdir(deps_dir)
luamake_root = deps_dir +"/luamake"
if not Path(luamake_root).is_dir():
	git_clone("https://github.com/actboy168/luamake")
os.chdir(luamake_root)
reset_to_commit("295950da763e2bf75956c56fdc7c8c2877aac4ad")
subprocess.run(["git","submodule","init"],check=True)
subprocess.run(["git","submodule","update"],check=True)
if platform == "linux":
	subprocess.run([luamake_root +"/compile/install.sh"],check=True,shell=True)
else:
	subprocess.run([luamake_root +"/compile/install.bat"],check=True,shell=True)

tmp_env = os.environ.copy()
if platform == "linux":
	tmp_env["PATH"] = luamake_root +":" + tmp_env["PATH"]
else:
	tmp_env["PATH"] = luamake_root +";" + tmp_env["PATH"]

print_msg("Building lua-debug...")
os.chdir(deps_dir)
luadebug_root = deps_dir +"/lua-debug"
if not Path(luadebug_root).is_dir():
	git_clone("https://github.com/actboy168/lua-debug")
os.chdir(luadebug_root)
subprocess.run(["git","fetch"],check=True)
reset_to_commit("b92f9760dac71f361bf6b85de550ab72701555ab")

subprocess.run(["luamake","lua","compile/download_deps.lua"],shell=True,check=True,env=tmp_env)
subprocess.run(["luamake"],shell=True,check=True,env=tmp_env)

if platform == "linux":
	luadebug_bin_path = "runtime/linux-x64/luajit"
	luadebug_bin_name = "luadebug.so"
else:
	luadebug_bin_path = "runtime/win32-x64/luajit"
	luadebug_bin_name = "luadebug.dll"

mkpath(install_dir +"/lua/modules/lua-debug/" +luadebug_bin_path)

# Copy required files
print_msg("Copying lua-debug files...")
cp_dir(luadebug_root +"/publish/script",install_dir +"/lua/modules/lua-debug/script/")
cp(luadebug_root +"/publish/" +luadebug_bin_path +"/" +luadebug_bin_name,install_dir +"/lua/modules/lua-debug/" +luadebug_bin_path +"/")

os.chdir(curDir)
