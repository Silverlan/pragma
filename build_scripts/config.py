from pathlib import Path
from sys import platform

build_config_tp = "Release"
cmake_args = []
deps_dir = str(Path.cwd() / "deps")
deps_staging_dir = "staging"
prebuilt_bin_dir = str(Path(deps_dir) / deps_staging_dir)
prefer_git_https = False
if platform == "win32":
	generator = "Visual Studio 17 2022"
else:
	generator = "Ninja Multi-Config"

pragma_root = str(Path(__file__).parent.parent)

toolsetArgs = None
toolsetCFlags = None

with_lua_debugger = False
build_swiftshader = False
with_swiftshader = False
