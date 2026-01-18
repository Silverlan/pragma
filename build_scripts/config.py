from pathlib import Path
from sys import platform

build_config_tp = "Release"
cleanBuildFiles = False
cmake_args = []
cmake_path = "cmake"
deps_dir = str(Path.cwd() / "deps")
deps_staging_dir = "staging"
prebuilt_bin_dir = str(Path(deps_dir) / deps_staging_dir)
build_tools_dir = str(Path.cwd() / "build_tools")
prefer_git_https = True
if platform == "win32":
	generator_msvc = "Visual Studio 17 2022"
	default_generator = generator_msvc
else:
	default_generator = "Ninja Multi-Config"
	no_sudo = False
	no_confirm = False

pragma_root = str(Path(__file__).parent.parent)

toolset = None
toolsetArgs = None
toolsetCFlags = None

build_swiftshader = False
clean_deps_build_files = False
generator = default_generator
with_lua_debugger = False
with_swiftshader = False
