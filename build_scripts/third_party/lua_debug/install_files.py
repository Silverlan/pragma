import subprocess
import os
import sys
import pathlib
import shutil
from sys import platform

deps_dir = sys.argv[1]
install_dir = deps_dir +"/staging"

luadebug_dir = deps_dir +"/lua-debug" 

if platform == "linux":
	luadebug_bin_path = "runtime/linux-x64/luajit"
	luadebug_bin_name = "luadebug.so"
else:
	luadebug_bin_path = "runtime/win32-x64/luajit"
	luadebug_bin_name = "luadebug.dll"

def mkpath(path):
	pathlib.Path(path).mkdir(parents=True, exist_ok=True)

def cp(src,dst):
	shutil.copy2(src,dst)

def mv(src, dst):
	shutil.move(src,dst)

def cp_dir(src,dst):
	shutil.copytree(src,dst,dirs_exist_ok=True)

mkpath(install_dir +"/lua/" +luadebug_bin_path)
mkpath(install_dir +"/lua/modules/lua-debug/script/")
mkpath(install_dir +"/lua/script/")

# Copy required files
print("Copying lua-debug files...")
cp_dir(luadebug_dir +"/publish/script",install_dir +"/lua/script/")

# Copy binary to "lua/runtime/" (the module won't be able to find it anywhere else)
cp(luadebug_dir +"/publish/" +luadebug_bin_path +"/" +luadebug_bin_name,install_dir +"/lua/" +luadebug_bin_path +"/")

# debugger.lua has to be located under "lua/modules/lua-debug/script/", or the module won't be able to find it.
# The other script files are located under "lua/script/".
mv(install_dir +"/lua/script/debugger.lua",install_dir +"/lua/modules/lua-debug/script/debugger.lua")
