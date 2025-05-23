import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

# These are required for Lua debugging with Visual Studio Code

if platform == "win32":
    python_exec_name = "python"
else:
    python_exec_name = "python3"

# luamake
print("Running build_luamake.py...")
subprocess.run([python_exec_name, "scripts/lua_debug/build_luamake.py", deps_dir],check=True)

# lua-debug
print("Running build_luadebug.py...")
subprocess.run([python_exec_name, "scripts/lua_debug/build_luadebug.py", deps_dir],check=True)

# install
print("Running install_files.py...")
subprocess.run([python_exec_name, "scripts/lua_debug/install_files.py", deps_dir, install_dir],check=True)

os.chdir(curDir)
