from pathlib import Path
from sys import platform
from urllib.error import URLError, HTTPError
import tarfile
import argparse
import re
import config

from scripts.shared import *

parser = argparse.ArgumentParser(description='Pragma build script', allow_abbrev=False, formatter_class=argparse.ArgumentDefaultsHelpFormatter, epilog="")

if platform == "linux":
	parser.add_argument('--c-compiler', help='The C-compiler to use.', default='clang-22')
	parser.add_argument('--cxx-compiler', help='The C++-compiler to use.', default='clang++-22')
defaultToolset = "clang"

# See https://stackoverflow.com/a/43357954/1879228 for boolean args
parser.add_argument('--generator', help='The generator to use.', default="Default")
parser.add_argument("--with-essential-client-modules", type=str2bool, nargs='?', const=True, default=True, help="Include essential modules required to run Pragma.")
parser.add_argument("--with-common-modules", type=str2bool, nargs='?', const=True, default=True, help="Include non-essential but commonly used modules (e.g. audio and physics modules).")
parser.add_argument("--with-pfm", type=str2bool, nargs='?', const=True, default=False, help="Include the Pragma Filmmaker.")
parser.add_argument("--with-core-pfm-modules", type=str2bool, nargs='?', const=True, default=True, help="Include essential PFM modules.")
parser.add_argument("--with-all-pfm-modules", type=str2bool, nargs='?', const=True, default=False, help="Include non-essential PFM modules (e.g. chromium and cycles).")
parser.add_argument("--with-vr", type=str2bool, nargs='?', const=True, default=False, help="Include Virtual Reality support.")
parser.add_argument("--with-networking", type=str2bool, nargs='?', const=True, default=False, help="Include networking module(s) for multiplayer support.")
parser.add_argument("--with-common-entities", type=str2bool, nargs='?', const=True, default=True, help="Include addons with support for common entity types.")
parser.add_argument("--with-lua-debugger", type=str2bool, nargs='?', const=True, default=False, help="Include Lua-debugger support.")
parser.add_argument("--with-swiftshader", type=str2bool, nargs='?', const=True, default=False, help="Include SwiftShader support for CPU-only rendering.")
parser.add_argument('--vtune-include-path', help='The include path to the VTune profiler (required for CPU profiling).', default='')
parser.add_argument('--vtune-library-path', help='The path to the "libittnotify" library of the VTune profiler (required for CPU profiling).', default='')
parser.add_argument("--build", type=str2bool, nargs='?', const=True, default=True, help="Build Pragma after configurating and generating build files.")
parser.add_argument("--build-swiftshader", type=str2bool, nargs='?', const=True, default=False, help="Builds SwiftShader from source instead of downloading prebuilt binaries.")
parser.add_argument("--build-all", type=str2bool, nargs='?', const=True, default=False, help="Build all dependencies instead of downloading prebuilt binaries where available. Enabling this may significantly increase the disk space requirement and build time.")
parser.add_argument('--build-config', help='The build configuration to use.', default='RelWithDebInfo')
parser.add_argument('--build-directory', help='Directory to write the build files to. Can be relative or absolute.', default='build')
parser.add_argument('--deps-directory', help='Directory to write the dependency files to. Can be relative or absolute.', default='deps')
parser.add_argument("--deps-only", type=str2bool, nargs='?', const=True, default=False, help="Configuration, build and installation of Pragma will be skipped.")
parser.add_argument("--clean-deps-build-files", type=str2bool, nargs='?', const=True, default=False, help="Automatically clean up build files of third-party dependencies to save disk space.")
parser.add_argument('--install-directory', help='Installation directory. Can be relative (to build directory) or absolute.', default='install')
parser.add_argument('--cmake-arg', help='Additional cmake argument for configuring Pragma. This parameter can be used multiple times.', action='append', default=[])
parser.add_argument("--cmake-cxx-flag", action="append", help="Additional flags to add to CMAKE_CXX_FLAGS.", default=[])
parser.add_argument("--ignore-warnings", type=str2bool, nargs='?', const=True, default=False, help="Ignore all warnings and allow building with unsupported configurations.")
parser.add_argument('--module', help='Custom modules to install. Use this parameter multiple times to use multiple modules. Usage example: --module pr_physx:\"https://github.com/Silverlan/pr_physx.git\"', action='append', default=[])
# parser.add_argument('--log-file', help='Script output will be written to this file.', default='build_log.txt')
parser.add_argument("--verbose", type=str2bool, nargs='?', const=True, default=False, help="Print additional verbose output.")
parser.add_argument("--update", type=str2bool, nargs='?', const=True, default=False, help="Update Pragma and all submodules and modules to the latest versions.")
parser.add_argument("--rerun", type=str2bool, nargs='?', const=True, default=False, help="Re-run the build script with the previous arguments.")
parser.add_argument("--prefer-git-https", type=str2bool, nargs='?', const=True, default=True, help="Clone repositories via HTTPS instead of SSH.")
parser.add_argument("--skip-repository-updates", type=str2bool, nargs='?', const=True, default=False, help=argparse.SUPPRESS)
if platform == "linux":
	parser.add_argument("--no-sudo", type=str2bool, nargs='?', const=True, default=False, help="Will not run sudo commands. System packages will have to be installed manually.")
	parser.add_argument("--no-confirm", type=str2bool, nargs='?', const=True, default=False, help="Disable any interaction with user (suitable for automated run).")
	parser.add_argument("--debug", type=str2bool, nargs='?', const=True, default=False, help="Enable debug assertions and disable code optimizations.")
	parser.add_argument('--toolset', help='The toolset to use. Supported toolsets: clang', default=defaultToolset) # gcc currently not supported
else:
	parser.add_argument('--toolset', help='The toolset to use. Supported toolsets: msvc, clang', default=defaultToolset) # clang-cl currently not supported
args,unknown = parser.parse_known_args()
args = vars(args)
input_args = args

if platform == "linux":
	if args["generator"] == "Default":
		args["generator"] = config.default_generator
else:
	# Note: CMake (v4.2.0) does not support the Visual Studio generator yet when using
	# C++23 import std. Until CMake adds support, we fall back to the Ninja generator,
	# except for third-party dependencies, which are usually designed for
	# being built with Visual Studio.
	# Once CMake supports Visual Studio with import std, the default generator should be
	# changed back to "Visual Studio 17 2022" for all cases.
	if args["generator"] == "Default":
		if args["deps_only"]:
			args["generator"] = config.default_generator
		else:
			args["generator"] = "Ninja Multi-Config"

if args["update"]:
	args["rerun"] = True
	args["update"] = True
elif args["rerun"]:
	print_msg("--rerun option has been set, restoring previous build script options...")
	build_dir = normalize_path(args["build_directory"])
	if not os.path.isabs(build_dir):
		build_dir = os.getcwd() +"/" +build_dir

	buildConfigLocation = build_dir +"/build_config.json"
	if Path(buildConfigLocation).is_file():
		import json
		cfg = json.load(open(build_dir +"/build_config.json"))
		for key,value in cfg["args"].items():
			args[key] = value

if platform == "linux":
	c_compiler = args["c_compiler"]
	cxx_compiler = args["cxx_compiler"]
	no_sudo = args["no_sudo"]
	no_confirm = args["no_confirm"]
	with_debug = args["debug"]
toolset = args["toolset"]
generator = args["generator"]
with_essential_client_modules = args["with_essential_client_modules"]
with_common_modules = args["with_common_modules"]
with_pfm = args["with_pfm"]
with_core_pfm_modules = args["with_core_pfm_modules"]
with_all_pfm_modules = args["with_all_pfm_modules"]
with_vr = args["with_vr"]
with_networking = args["with_networking"]
with_common_entities = args["with_common_entities"]
with_lua_debugger = args["with_lua_debugger"]
with_swiftshader = args["with_swiftshader"]
build_swiftshader = args["build_swiftshader"]
vtune_include_path = args["vtune_include_path"]
vtune_library_path = args["vtune_library_path"]
build = args["build"]
build_all = args["build_all"]
build_config = args["build_config"]
build_directory = args["build_directory"]
deps_directory = args["deps_directory"]
deps_only = args["deps_only"]
clean_deps_build_files = args["clean_deps_build_files"]
install_directory = args["install_directory"]
additional_cmake_args = args["cmake_arg"]
additional_cmake_flags = args["cmake_cxx_flag"]
skip_repository_updates = args["skip_repository_updates"]
scripts_dir = os.getcwd() +"/build_scripts"
#log_file = args["log_file"]
verbose = args["verbose"]
modules = args["module"]
rerun = args["rerun"]
prefer_git_https = args["prefer_git_https"]
update = args["update"]
ignore_warnings = args["ignore_warnings"]

root = normalize_path(os.getcwd())
build_dir = normalize_path(build_directory)
deps_dir = normalize_path(deps_directory)
install_dir = install_directory
tools = root +"/tools"

if not os.path.isabs(build_dir):
	build_dir = os.getcwd() +"/" +build_dir

if not os.path.isabs(deps_dir):
	deps_dir = os.getcwd() +"/" +deps_dir
deps_dir_fs = deps_dir.replace("\\", "/")

if not os.path.isabs(install_dir):
	install_dir = build_dir +"/" +install_dir

try:
	branch = subprocess.check_output(
		["git", "rev-parse", "--abbrev-ref", "HEAD"], text=True
	).strip()
	if branch == "HEAD":
		branch = None  # detached
	commit = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()

	branch = branch or "(detached HEAD)"
	print("on pragma branch \"" +branch +"\", commit " +str(commit))
except (subprocess.CalledProcessError, FileNotFoundError):
	print("Not a git repo")

print("Inputs:")
if platform == "linux":
	print("cxx_compiler: " +cxx_compiler)
	print("c_compiler: " +c_compiler)
else:
	if toolset == "clang":
		generator = "Ninja Multi-Config"
print("toolset: " +toolset)

config.build_swiftshader = build_swiftshader
config.clean_deps_build_files = clean_deps_build_files
config.deps_dir = deps_dir
config.prefer_git_https = prefer_git_https
config.toolset = toolset
config.with_lua_debugger = with_lua_debugger
config.with_swiftshader = with_swiftshader
if platform == "linux":
	config.no_sudo = no_sudo
	config.no_confirm = no_confirm
config.generator = generator

print("generator: " +generator)
print("with_essential_client_modules: " +str(with_essential_client_modules))
print("with_common_modules: " +str(with_common_modules))
print("with_pfm: " +str(with_pfm))
print("with_core_pfm_modules: " +str(with_core_pfm_modules))
print("with_all_pfm_modules: " +str(with_all_pfm_modules))
print("with_vr: " +str(with_vr))
print("with_lua_debugger: " +str(with_lua_debugger))
print("with_swiftshader: " +str(with_swiftshader))
print("build_swiftshader: " +str(build_swiftshader))
print("rerun: " +str(rerun))
print("update: " +str(update))
print("build: " +str(build))
print("build_all: " +str(build_all))
print("build_config: " +build_config)
print("build_directory: " +build_directory)
print("deps_directory: " +deps_directory)
print("deps_only: " +str(deps_only))
print("clean_deps_build_files: " +str(clean_deps_build_files))
print("install_directory: " +install_directory)
if platform == "linux":
	print("no_sudo: " +str(no_sudo))
	print("no_confirm: " +str(no_confirm))
	print("debug: " +str(with_debug))
print("cmake_args: " +', '.join(additional_cmake_args))
print("cmake_flags: " +', '.join(additional_cmake_flags))
print("modules: " +', '.join(modules))

mkpath(build_dir)
mkpath(install_dir)
mkpath(tools)

config.prebuilt_bin_dir = deps_dir +"/" +config.deps_staging_dir

def check_cmake(min_version):
	cmake_path = shutil.which("cmake")
	if not cmake_path:
		# CMake not found
		return False

	try:
		result = subprocess.run(
			[cmake_path, "--version"], 
			capture_output=True, 
			text=True, 
			check=True
		)
		
		match = re.search(r"(\d+\.\d+\.\d+)", result.stdout)
		if not match:
			# Failed to parse version
			return False

		version_str = match.group(1)
		current_version = tuple(map(int, version_str.split(".")))

		if current_version >= min_version:
			return True
		else:
			# CMake is too old
			return False

	except (subprocess.CalledProcessError, OSError) as e:
		# CMake execution failed
		return False

def fetch_cmake():
	from third_party import cmake
	cmake.main()
	if platform == "win32":
		cmake_exe = "cmake.exe"
	else:
		cmake_exe = "cmake"
	config.cmake_path = str(Path(config.build_tools_dir) / "cmake" / "bin" / cmake_exe)

# At least CMake 4.2.0 is needed, which is still very new and not available in the package managers of
# most distros yet. If the detected CMake version is too old (or none was found), we'll download it here.
use_custom_cmake = False
if not check_cmake((4, 2, 0)):
	use_custom_cmake = True
	fetch_cmake()

# Use prebuilt binaries if --build-all is not set
os.chdir(root)
if build_all == False:
	subprocess.run([config.cmake_path, "-DPRAGMA_DEPS_DIR=" +config.prebuilt_bin_dir +"", "-DTOOLSET=" +toolset, "-P", "cmake/fetch_deps.cmake"],check=True)
subprocess.run([config.cmake_path, "-DPRAGMA_BUILD_TOOLS_DIR=" +config.build_tools_dir, "-DPRAGMA_DEPS_DIR=" +config.prebuilt_bin_dir, "-P", "cmake/fetch_clang.cmake"],check=True)

import shutil
import subprocess
import re
import sys

if platform == "win32":
	load_vs_env(deps_dir)
	if toolset == "msvc":
		print_warning(f"Visual Studio toolset is currently not recommended and may not work. If you run into issues, try using the clang toolset instead.")
	elif toolset == "clang":
		# We need an up-to-date version of clang, so we'll use our shipped version for now.
		#from third_party import clang
		#clang.main()

		clang_dir = str(Path(config.build_tools_dir) / "clang/bin/")
		config.toolsetArgs = [
			"-DCMAKE_C_COMPILER=" +str(Path(clang_dir) / "clang.exe"),
			"-DCMAKE_CXX_COMPILER=" +str(Path(clang_dir) / "clang++.exe"),
			"-DCMAKE_MAKE_PROGRAM=ninja.exe"
		]
		config.toolsetCFlags = ["-fexceptions", "-fcxx-exceptions", "--target=x86_64-pc-windows-msvc"]

		# Due to "import std;" support still being experimental in CMake, we have to use a custom, patched
		# version of CMake to build Pragma with clang on Windows.
		if not use_custom_cmake:
			use_custom_cmake = True
			fetch_cmake()
	elif toolset == "clang-cl":
		clang_dir = str(Path(config.build_tools_dir) / "clang/bin")
		
		config.toolsetArgs = [
			"-DCMAKE_C_COMPILER=" +clang_dir +"/clang-cl.exe",
			"-DCMAKE_CXX_COMPILER=" +clang_dir +"/clang-cl.exe",
			"-DCMAKE_CXX_COMPILER_AR=" +clang_dir +"/llvm-ar.exe",
			"-DCMAKE_CXX_COMPILER_CLANG_SCAN_DEPS=" +clang_dir +"/clang-scan-deps.exe",
			"-DCMAKE_CXX_COMPILER_RANLIB=" +clang_dir +"/llvm-ranlib.exe"
		]
		config.toolsetCFlags = ["-Wno-error", "-Wno-unused-command-line-argument", "-Wno-enum-constexpr-conversion", "-fexceptions", "-fcxx-exceptions"]
		print_warning(f"Toolset {toolset} for platform {platform} is currently not supported!")
		if not ignore_warnings:
			sys.exit(1)
		else:
			print_msg("--ignore-warnings has been enabled, continuing...")
	if generator != "Ninja Multi-Config":
		if not deps_only:
			print_warning(f"Generator {generator} for platform {platform} is currently not supported! Please use \"Ninja Multi-Config\".")
			print_msg("If you want to try using this generator anyway, follow these steps:")
			print_msg("1) Open CMakePresets.json in the root Pragma directory and remove the '\"hidden\": true' fields from the \"config-windows-msvc-*\" and \"build-windows-msvc-*\" presets.")
			print_msg("2) Re-run this script with --ignore-warnings")
			if not ignore_warnings:
				sys.exit(1)
			else:
				print_msg("--ignore-warnings has been enabled, continuing...")
elif platform == "linux" and (c_compiler == "clang-22" or c_compiler == "clang++-22"):
	# Due to a compiler bug with C++20 Modules in clang, we need the
	# very latest version of clang, which is not available in package managers yet.
	# We'll use our own prebuilt version for now.
	# from third_party import clang
	# clang.main()
	clang_staging_path = Path(config.build_tools_dir) / "clang/"
	if c_compiler == "clang-22":
		c_compiler = str(clang_staging_path / "bin/clang")
	if cxx_compiler == "clang++-22":
		cxx_compiler = str(clang_staging_path / "bin/clang++")
	print_msg("Setting c_compiler override to '" +c_compiler +"'")
	print_msg("Setting cxx_compiler override to '" +cxx_compiler +"'")

if update:
	os.chdir(root)

	print_msg("Updating Pragma repository...")
	subprocess.run(["git","pull"],check=True)

	argv = sys.argv
	argv.remove("--update")
	argv.append("--rerun")
	print_msg("Build script may have changed, re-running...")
	print("argv: ",argv)
	os.execv(sys.executable, ['python'] +argv)
	sys.exit(0)

if platform == "linux":
	os.environ["CC"] = c_compiler
	os.environ["CXX"] = cxx_compiler

def execscript(filepath):
	global generator
	global config
	global build_config
	global build_directory
	global deps_directory
	global install_directory
	global scripts_dir
	global verbose
	global root
	global build_dir
	global deps_dir
	global install_dir
	global tools
	global cmake_args
	global with_swiftshader
	global with_lua_debugger

	curDir = os.getcwd()

	g = {}
	l = {
		"generator": generator,
		"toolsetArgs": config.toolsetArgs,
		"toolsetCFlags": config.toolsetCFlags,
		"build_config": build_config,
		"build_directory": build_directory,
		"deps_directory": deps_directory,
		"install_directory": install_directory,
		"scripts_dir": scripts_dir,
		"verbose": verbose,

		"root": root,
		"build_dir": build_dir,
		"deps_dir": deps_dir,
		"install_dir": install_dir,
		"tools": tools,

		"build_all": build_all,

		"normalize_path": normalize_path,
		"mkpath": mkpath,
		"print_msg": print_msg,
		"print_warning": print_warning,
		"git_clone": git_clone,
		"cmake_configure": cmake_configure,
		"cmake_configure_def_toolset": cmake_configure_def_toolset,
		"cmake_build": cmake_build,
		"mkdir": mkdir,
		"http_download": http_download,
		"http_extract": http_extract,
		"cp": cp,
		"cp_dir": cp_dir,
		"replace_text_in_file": replace_text_in_file,
		"extract": extract,
		"execfile": execfile,
		"execscript": execscript,
		"str2bool": str2bool,
		"install_prebuilt_binaries": install_prebuilt_binaries,
		"reset_to_commit": reset_to_commit,
		"check_repository_commit": check_repository_commit,
		"check_content_version": check_content_version,
		"update_content_version": update_content_version,
		"run_cmake_script": run_cmake_script,
		
		"cmake_args": cmake_args,

		"with_swiftshader": with_swiftshader,
		"with_lua_debugger": with_lua_debugger,
		"build_swiftshader": build_swiftshader
	}
	if platform == "linux":
		l["c_compiler"] = c_compiler
		l["cxx_compiler"] = cxx_compiler
		l["no_confirm"] = no_confirm
		l["no_sudo"] = no_sudo

	execfile(filepath,l,l)
	cmake_args = l["cmake_args"]

	os.chdir(curDir)

########## System packages ##########
if platform == "linux":
	if(no_sudo):
		print_msg("--no-sudo has been specified. System packages will be skipped, this may cause errors later on...")
	else:
		commands = []
		if(prefer_pacman()):
			packages = [
				# "cmake",
				"ninja"
			]

			# Required for vcpkg
			packages += ["base-devel git curl zip unzip tar cmake ninja"]

			for pck in packages:
				commands.append("pacman -S " +pck)
		else:
			packages = [
				# "cmake",
				"ninja-build",
				"gcc",
				"g++",
				"libfreetype6-dev",
				"libwayland-dev",
				"libxkbcommon-dev",
				"libx11-dev",
				"libxrandr-dev",
				"libxinerama-dev",
				"libxcursor-dev",
				"libxi-dev",
				"libxcb-keysyms1-dev",
				"libx11-xcb-dev",
				"pkg-config"
			]

			# Required for curl
			# packages += ["libssl-dev"]

			for pck in packages:
				commands.append("apt install " +pck)
		install_system_packages(commands, no_confirm)

module_list = []
cmake_args = config.cmake_args
additional_build_targets = []

########## Third-Party Libraries ##########
os.chdir(root)
print_msg("Building third-party libraries...")
if build_all:
	import build_third_party_libs
	build_third_party_libs.main()

########## Modules ##########
print_msg("Downloading modules...")
os.chdir(root +"/modules")

module_info = []
def add_pragma_module(name,repositoryUrl=None,commitSha=None,branch=None,skipBuildTarget=False):
	for module in module_info:
		if module["name"] == name:
			return
	module = {
		"name": name,
		"repositoryUrl": repositoryUrl,
		"commitSha": commitSha,
		"branch": branch,
		"skipBuildTarget": skipBuildTarget
	}
	module_info.append(module)

def execfile(filepath, globals=None, locals=None, args=None):
	if globals is None:
		globals = {}
	globals.update({
		"__file__": filepath,
		"__name__": "__main__",
	})
	if args is not None:
		sys.argv = [filepath] + args
	with open(filepath, 'rb') as file:
		exec(compile(file.read(), filepath, 'exec'), globals, locals)

external_libs_dir = root +"/external_libs"
modules_dir = root +"/modules"
third_party_libs_dir = root +"/third_party_libs"
external_libs_bin_dir = build_dir +"/external_libs"
modules_bin_dir = build_dir +"/modules"
third_party_libs_bin_dir = build_dir +"/third_party_libs"

def execbuildscript(filepath):
	global module_list
	global cmake_args
	global additional_build_targets
	global generator
	global config
	global moduleUrl
	global moduleDir
	global build_config
	global build_directory
	global deps_directory
	global external_libs_dir
	global modules_dir
	global third_party_libs_dir
	global external_libs_bin_dir
	global modules_bin_dir
	global third_party_libs_bin_dir
	global install_directory
	global verbose
	global root
	global build_dir
	global deps_dir
	global install_dir
	global tools
	global copy_files
	global copy_prebuilt_binaries
	global copy_prebuilt_headers
	global copy_prebuilt_directory
	global get_library_include_dir
	global get_staging_dir
	global get_library_root_dir
	global get_library_lib_dir
	global get_library_bin_dir

	curDir = os.getcwd()

	g = {
		"module_list": module_list,
		"cmake_args": cmake_args,
		"additional_build_targets": additional_build_targets
	}
	l = {
		"generator": generator,
		"toolsetArgs": config.toolsetArgs,
		"toolsetCFlags": config.toolsetCFlags,
		"moduleUrl": moduleUrl,
		"moduleDir": moduleDir,
		"build_config": build_config,
		"build_directory": build_directory,
		"deps_directory": deps_directory,
		"external_libs_dir": external_libs_dir,
		"modules_dir": modules_dir,
		"third_party_libs_dir": third_party_libs_dir,
		"external_libs_bin_dir": external_libs_bin_dir,
		"modules_bin_dir": modules_bin_dir,
		"third_party_libs_bin_dir": third_party_libs_bin_dir,

		"copy_files": copy_files,
		"copy_prebuilt_binaries": copy_prebuilt_binaries,
		"copy_prebuilt_headers": copy_prebuilt_headers,
		"copy_prebuilt_directory": copy_prebuilt_directory,
		"get_library_include_dir": get_library_include_dir,
		"get_staging_dir": get_staging_dir,
		"get_library_root_dir": get_library_root_dir,
		"get_library_lib_dir": get_library_lib_dir,
		"get_library_bin_dir": get_library_bin_dir,

		"install_directory": install_directory,
		"verbose": verbose,

		"root": root,
		"build_dir": build_dir,
		"deps_dir": deps_dir,
		"install_dir": install_dir,
		"tools": tools,

		"build_all": build_all,

		"normalize_path": normalize_path,
		"mkpath": mkpath,
		"print_msg": print_msg,
		"print_warning": print_warning,
		"git_clone": git_clone,
		"git_clone_commit": git_clone_commit,
		"cmake_configure": cmake_configure,
		"cmake_configure_def_toolset": cmake_configure_def_toolset,
		"cmake_build": cmake_build,
		"mkdir": mkdir,
		"http_download": http_download,
		"http_extract": http_extract,
		"cp": cp,
		"cp_dir": cp_dir,
		"replace_text_in_file": replace_text_in_file,
		"extract": extract,
		"execfile": execfile,
		"execbuildscript": execbuildscript,
		"str2bool": str2bool,
		"install_prebuilt_binaries": install_prebuilt_binaries,
		"reset_to_commit": reset_to_commit,
		"check_repository_commit": check_repository_commit,
		"check_content_version": check_content_version,
		"update_content_version": update_content_version,
		"run_cmake_script": run_cmake_script,
		"add_pragma_module": add_pragma_module
	}
	if platform == "linux":
		l["c_compiler"] = c_compiler
		l["cxx_compiler"] = cxx_compiler
		l["no_confirm"] = no_confirm
		l["no_sudo"] = no_sudo
		l["install_system_packages"] = install_system_packages
	#	l["harfbuzz_include_dir"] = harfbuzz_include_dir
	#	l["harfbuzz_lib"] = harfbuzz_lib

	if platform == "win32":
		l["determine_vs_installation_path"] = determine_vs_installation_path
		l["determine_vsdevcmd_path"] = determine_vsdevcmd_path

	execfile(filepath,g,l)

	module_list = g["module_list"]
	cmake_args = g["cmake_args"]
	additional_build_targets = g["additional_build_targets"]

	os.chdir(curDir)

# Register modules that were added using the --module argument
for module in modules:
	os.chdir(root +"/modules")
	parts = re.split(r':(?!\/\/)', module)
	moduleArgName = parts[0]
	repositoryUrl = None
	skipBuildTarget = False
	if len(parts) > 1:
		for part in parts[1:]:
			if part == "skipBuildTarget":
				skipBuildTarget = True
			else:
				repositoryUrl = part.strip('\"')
	add_pragma_module(
		name=moduleArgName.strip('\"'),
		repositoryUrl=repositoryUrl,
		skipBuildTarget=skipBuildTarget
	)

g = {}
l = {
	"add_pragma_module": add_pragma_module
}
# Register user modules
execfile(scripts_dir +"/user_modules.py",g,l)

cmake_with_args = []
cmake_with_args.append(f"-DPRAGMA_WITH_PFM={1 if with_pfm else 0}")
cmake_with_args.append(f"-DPRAGMA_WITH_VR={1 if with_vr else 0}")
cmake_with_args.append(f"-DPRAGMA_WITH_NETWORKING={1 if with_networking else 0}")
cmake_with_args.append(f"-DPRAGMA_WITH_COMMON_ENTITIES={1 if with_common_entities else 0}")
cmake_with_args.append(f"-DPRAGMA_WITH_COMMON_MODULES={1 if with_common_modules else 0}")
cmake_with_args.append(f"-DPRAGMA_WITH_ESSENTIAL_CLIENT_MODULES={1 if with_essential_client_modules else 0}")

cmake_with_args.append(f"-DPRAGMA_WITH_CORE_PFM_MODULES={1 if with_core_pfm_modules else 0}")
cmake_with_args.append(f"-DPRAGMA_WITH_ALL_PFM_MODULES={1 if with_all_pfm_modules else 0}")

# Fetch base modules
os.chdir(root)
subprocess.run([config.cmake_path] +cmake_with_args +["-P", "cmake/fetch_modules.cmake"],check=True)

# Fetch additional modules
index = 0
# The module list can be modified during iteration, so we have to use a while loop here.
while index < len(module_info):
	module = module_info[index]
	global moduleName
	moduleName = module["name"]
	moduleUrl = module["repositoryUrl"]
	commitId = module["commitSha"]
	branch = module["branch"]
	skipBuildTarget = module["skipBuildTarget"]
	print("Module Name:", moduleName)
	print("Repository URL:", moduleUrl)
	print("Commit SHA:", commitId)
	print("Branch:", branch)

	os.chdir(root +"/modules")

	moduleDir = os.getcwd() +"/" +moduleName +"/"

	print("Module Directory: " +moduleDir)

	if not skip_repository_updates:
		if moduleUrl:
			get_submodule(moduleName,moduleUrl,commitId,branch)
				
	module_list.append(moduleName)
	index += 1

# Run module build scripts
modules_dir = Path(root) / "modules"
for item in modules_dir.iterdir():
	if item.is_dir():
		moduleDir = str(item)
		os.chdir(moduleDir)
		scriptPath = str(Path(moduleDir) / "build_scripts/setup.py")
		if Path(scriptPath).is_file():
			print_msg("Executing module setup script...")
			
			moduleName = item.name
			moduleUrl = ""
			commitId = ""
			branch = ""
			skipBuildTarget = False

			execbuildscript(scriptPath)

print("Modules:" +', '.join(module_list))
print("Additional CMake Arguments:" +', '.join(cmake_args))
print("Additional Build Targets:" +', '.join(additional_build_targets))

if not deps_only:
	########## Configure Pragma ##########
	print_msg("Configuring Pragma...")
	os.chdir(build_dir)

	cmake_args.append("-DCMAKE_INSTALL_PREFIX:PATH=" +install_dir +"")
	cmake_args.append("-DPRAGMA_ADDITIONAL_MODULES=" + ";".join(module_list))

	if ignore_warnings:
		cmake_args.append("-DPRAGMA_IGNORE_WARNINGS=ON")

	vtune_enabled = False
	if len(vtune_include_path) > 0 or len(vtune_library_path) > 0:
		if len(vtune_include_path) > 0 and len(vtune_library_path) > 0:
			print_msg("VTune profiler support is enabled!")
			vtune_enabled = True
			cmake_args += ["-DDEPENDENCY_VTUNE_PROFILER_INCLUDE=" +vtune_include_path]
			cmake_args += ["-DDEPENDENCY_VTUNE_PROFILER_LIBRARY=" +vtune_library_path]
		else:
			raise argparse.ArgumentError(None,"Both the --vtune-include-path and --vtune-library-path options have to be specified to enable VTune support!")

	cmake_args.append(f"-DCONFIG_BUILD_WITH_VTUNE_SUPPORT={1 if vtune_enabled else 0}")

	cmake_args += cmake_with_args
	if platform == "linux":
		cmake_args.append(f"-DPRAGMA_DEBUG={1 if with_debug else 0}")

	cmake_args += additional_cmake_args
	cmake_args.append("-DPRAGMA_DEPS_DIR=" +config.deps_dir +"/" +config.deps_staging_dir)

	if platform == "win32":
		if toolset == "msvc":
			if generator == "Visual Studio 17 2022":
				preset = "config-windows-msvc-vs2022"
			elif generator == "Visual Studio 18 2026":
				preset = "config-windows-msvc-vs2026"
			else:
				preset = "config-windows-msvc-ninja"
		elif toolset == "clang":
			preset = "config-windows-clang-ninja"
	else:
		preset = "config-linux-clang"

	cmake_args.append("--preset")
	cmake_args.append(preset)

	cmake_args.append("-B")
	cmake_args.append(build_dir)

	cmake_configure_def_toolset(root,generator,cmake_args,additional_cmake_flags)

	print_msg("Build files have been written to \"" +build_dir +"\".")

	########## Write Build Configuration ##########
	cfg = {}
	cfg["args"] = {}
	for key, value in input_args.items():
		cfg["args"][key] = value

	cfg["original_args"] = ' '.join(sys.argv[1:])

	import json

	if rerun:
		# Keep the "original_args" info from the existing build_config
		try:
			oldCfg = None
			with open(build_dir +"/build_config.json", "r") as file:
				oldCfg = json.load(file)

			if oldCfg:
				if "original_args" in oldCfg:
					cfg["original_args"] = cfg["original_args"]


		except json.JSONDecodeError as e:
			print("Failed to load build_config.json:", e)

	json.dump(cfg,open(build_dir +"/build_config.json",'w'))

	########## Build Pragma ##########
	if build:
		print_msg("Building Pragma...")

		os.chdir(build_dir)
		targets = ["pragma-install-full"]

		print_msg("Running build command...")
		cmake_build(build_config,targets,verbose=verbose)

		print_msg("Build Successful! Pragma has been installed to \"" +normalize_path(install_dir) +"\".")
		print_msg("If you make any changes to the core source code, you can build the \"pragma-install\" target to compile the changes and re-install the binaries automatically.")
		print_msg("If you make any changes to a module, you will have to build the module target first, and then build \"pragma-install\".")
		print_msg("")

print_msg("All actions have been completed! Please make sure to re-run this script every time you pull any changes from the repository, and after adding any new modules.")

os.chdir(root)
