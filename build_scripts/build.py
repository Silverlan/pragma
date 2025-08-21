# import os
from pathlib import Path
from sys import platform
from urllib.error import URLError, HTTPError
import tarfile
import argparse
import re
import config
# import logging

from scripts.shared import *

parser = argparse.ArgumentParser(description='Pragma build script', allow_abbrev=False, formatter_class=argparse.ArgumentDefaultsHelpFormatter, epilog="")

###### Config section
# When using prebuilt binaries this tag will be used for the download from https://github.com/Silverlan/pragma-deps-lib
prebuilt_tag = "2025-08-20"
######

# See https://stackoverflow.com/a/43357954/1879228 for boolean args
if platform == "linux":
	parser.add_argument('--c-compiler', help='The C-compiler to use.', default='clang-20')
	parser.add_argument('--cxx-compiler', help='The C++-compiler to use.', default='clang++-20')
	defaultGenerator = "Ninja Multi-Config"
else:
	defaultGenerator = "Visual Studio 17 2022"
	defaultToolset = "msvc"

parser.add_argument('--generator', help='The generator to use.', default=defaultGenerator)
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
parser.add_argument('--install-directory', help='Installation directory. Can be relative (to build directory) or absolute.', default='install')
parser.add_argument('--cmake-arg', help='Additional cmake argument for configuring Pragma. This parameter can be used multiple times.', action='append', default=[])
parser.add_argument('--module', help='Custom modules to install. Use this parameter multiple times to use multiple modules. Usage example: --module pr_physx:\"https://github.com/Silverlan/pr_physx.git\"', action='append', default=[])
# parser.add_argument('--log-file', help='Script output will be written to this file.', default='build_log.txt')
parser.add_argument("--verbose", type=str2bool, nargs='?', const=True, default=False, help="Print additional verbose output.")
parser.add_argument("--update", type=str2bool, nargs='?', const=True, default=False, help="Update Pragma and all submodules and modules to the latest versions.")
parser.add_argument("--rerun", type=str2bool, nargs='?', const=True, default=False, help="Re-run the build script with the previous arguments.")
parser.add_argument("--skip-repository-updates", type=str2bool, nargs='?', const=True, default=False, help=argparse.SUPPRESS)
if platform == "linux":
	parser.add_argument("--no-sudo", type=str2bool, nargs='?', const=True, default=False, help="Will not run sudo commands. System packages will have to be installed manually.")
	parser.add_argument("--no-confirm", type=str2bool, nargs='?', const=True, default=False, help="Disable any interaction with user (suitable for automated run).")
	parser.add_argument("--enable-assertions", type=str2bool, nargs='?', const=True, default=False, help="Enable debug assertions.")
else:
	parser.add_argument('--toolset', help='The toolset to use. Supported toolsets: msvc, clang, clang-cl', default=defaultToolset)
args,unknown = parser.parse_known_args()
args = vars(args)
input_args = args

#log_file = args.log_file
#if log_file != "":
#	if not os.path.isabs(log_file):
#		log_file = os.getcwd() +"/" +log_file
#
#		logging.basicConfig(filename=log_file,
#			filemode='a',
#			format='%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s',
#			datefmt='%H:%M:%S',
#			level=logging.DEBUG)
#
#		logging.info("Running Pragma Build Script")

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
	enable_assertions = args["enable_assertions"]
else:
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
install_directory = args["install_directory"]
additional_cmake_args = args["cmake_arg"]
skip_repository_updates = args["skip_repository_updates"]
scripts_dir = os.getcwd() +"/build_scripts"
#log_file = args["log_file"]
verbose = args["verbose"]
modules = args["module"]
rerun = args["rerun"]
update = args["update"]
modules_prebuilt = []

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
config.deps_dir = deps_dir

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
	print("toolset: " +toolset)
	if toolset == "clang":
		generator = "Ninja Multi-Config"

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
print("install_directory: " +install_directory)
if platform == "linux":
	print("no_sudo: " +str(no_sudo))
	print("no_confirm: " +str(no_confirm))
	print("enable_assertions: " +str(enable_assertions))
print("cmake_args: " +', '.join(additional_cmake_args))
print("modules: " +', '.join(modules))

toolsetArgs = None
toolsetCFlags = None
def cmake_configure_def_toolset(scriptPath,generator,additionalArgs=[]):
	cmake_configure(scriptPath,generator,toolsetArgs,additionalArgs,toolsetCFlags)

if platform == "win32":
	if toolset == "msvc":
		toolset = None # Let the compiler use the default toolset
	elif toolset == "clang":
		toolsetArgs = [
			"-DCMAKE_C_COMPILER=clang.exe",
			"-DCMAKE_CXX_COMPILER=clang++.exe",
			"-DCMAKE_MAKE_PROGRAM=ninja.exe"
		]
		toolsetCFlags = ["-fexceptions", "-fcxx-exceptions", "--target=x86_64-pc-windows-msvc"]
	elif toolset == "clang-cl":
		toolsetArgs = [
			"-T", "ClangCL",
			"-A", "x64",
			"-DCMAKE_CXX_SCAN_FOR_MODULES=ON"
		]
		toolsetCFlags = ["-Wno-error", "-Wno-unused-command-line-argument", "-Wno-enum-constexpr-conversion", "-fexceptions", "-fcxx-exceptions", "/EHsc"]

if platform == "linux" and enable_assertions:
	toolsetCFlags = ["-D_GLIBCXX_ASSERTIONS"]

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


mkpath(build_dir)
mkpath(deps_dir)
mkpath(install_dir)
mkpath(tools)

config.prebuilt_bin_dir = deps_dir +"/" +config.deps_staging_dir

# Use prebuilt binaries if --build-all is not set
if build_all == False:
    def is_commit_current(base_path: str, commit_id: str, filename: str = "commit_id.txt") -> bool:
        target_file = Path(base_path) / filename
        if not target_file.exists():
            return False
        try:
            return target_file.read_text(encoding="utf-8").strip() == commit_id
        except Exception:
            return False


    def update_commit_directory(base_path: str, commit_id: str, filename: str = "commit_id.txt") -> None:
        base = Path(base_path)
        if base.exists():
            print(f"Removing directory '{base}'...")
            shutil.rmtree(base)

        print(f"Creating directory '{base}'...")
        base.mkdir(parents=True, exist_ok=True)

        target_file = base / filename
        print(f"Writing commit ID to '{target_file}'...")
        target_file.write_text(commit_id, encoding="utf-8")

    base_path = get_staging_dir()
    if not is_commit_current(base_path, prebuilt_tag, "tag_id.txt"):
        update_commit_directory(base_path, prebuilt_tag, "tag_id.txt")
        os.chdir(base_path)

        print_msg("Downloading prebuilt third-party binaries...")

        if platform == "linux":
            prebuilt_archive_name = "lib-linux_x64.tar.gz"
            prebuilt_archive_format = "tar.gz"
        else:
            prebuilt_archive_name = "lib-windows_x64.zip"
            prebuilt_archive_format = "zip"

        http_extract("https://github.com/Silverlan/pragma-deps-lib/releases/download/" +prebuilt_tag +"/" +prebuilt_archive_name,format=prebuilt_archive_format)
    else:
        print(f"Directory '{base_path}' is already up-to-date.")

########## clang-20 ##########
# Due to a compiler bug with C++20 Modules in clang, we have to use clang-20 for now,
# which is not available in package managers yet.
if platform == "linux" and (c_compiler == "clang-20" or c_compiler == "clang++-20"):
	clang_staging_path = get_library_root_dir("clang")
	if build_all:
		curDir = os.getcwd()
		os.chdir(deps_dir)
		clang20_root = os.getcwd() +"/LLVM-20.1.8-Linux-X64"
		if not Path(clang20_root).is_dir():
			print_msg("Downloading clang-20...")
			http_extract("https://github.com/llvm/llvm-project/releases/download/llvmorg-20.1.8/LLVM-20.1.8-Linux-X64.tar.xz",format="tar.xz")
		os.chdir(curDir)

		copy_preserving_symlink(Path(clang20_root +"/bin/clang"), Path(clang_staging_path +"/bin"))
		copy_preserving_symlink(Path(clang20_root +"/bin/clang++"), Path(clang_staging_path +"/bin"))
		copy_preserving_symlink(Path(clang20_root +"/bin/clang-scan-deps"), Path(clang_staging_path +"/bin"))

		copytree(clang20_root +"/include/c++", clang_staging_path +"/include/c++")
		copytree(clang20_root +"/include/clang", clang_staging_path +"/include/clang")
		copytree(clang20_root +"/include/clang-c", clang_staging_path +"/include/clang-c")
		copytree(clang20_root +"/lib/clang", clang_staging_path +"/lib/clang")

	if c_compiler == "clang-20":
		c_compiler = clang_staging_path +"/bin/clang"
	if cxx_compiler == "clang++-20":
		cxx_compiler = clang_staging_path +"/bin/clang++"
	print_msg("Setting c_compiler override to '" +c_compiler +"'")
	print_msg("Setting cxx_compiler override to '" +cxx_compiler +"'")

if platform == "linux":
	os.environ["CC"] = c_compiler
	os.environ["CXX"] = cxx_compiler

def execscript(filepath):
	global generator
	global toolsetArgs
	global toolsetCFlags
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
		"toolsetArgs": toolsetArgs,
		"toolsetCFlags": toolsetCFlags,
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
		commands = [
			# Required for the build script
			"apt-get install python3",
			
			# Required for Pragma core
			"apt install build-essential",
			# "add-apt-repository ppa:savoury1/llvm-defaults-14",
			"apt update",
			"apt install clang-18",
			"apt-get install clang-tools-18", # Required for C++20 Modules
			"apt install libstdc++-12-dev",
			"apt install libstdc++6",
			"apt-get install patchelf",

			# Required for Vulkan
			"apt-get -qq install -y libwayland-dev libxrandr-dev",

			"apt-get install libxcb-keysyms1-dev",
			"apt-get install xcb libxcb-xkb-dev x11-xkb-utils libx11-xcb-dev libxkbcommon-x11-dev",

			# Required for GLFW
			"apt install xorg-dev",

			# Required for OIDN
			"apt install git-lfs",

			# Required for Cycles
			"apt-get install subversion",
			"apt-get install meson", # epoxy
			
			# CMake
			"apt-get install cmake",

			# Required for Curl
			"apt-get install libssl-dev",
			
			# Curl
			"apt-get install curl zip unzip tar",

			# Required for OIIO
			# "apt-get install python3-distutils",

			#install freetype for linking. X server frontends (Gnome, KDE etc) already include it somewhere down the line. Also install pkg-config for easy export of flags.
			"apt-get install pkg-config libfreetype-dev",

			# Ninja
			"apt-get install ninja-build",

			# libdecor (required for Wayland)
			"apt-get install wayland-protocols",
			"apt-get install libdbus-1-dev",
			"apt-get install libgtk-3-dev",

			# Required for libsdbus-c++
			"apt-get install meson ninja-build libcap-dev libsystemd-dev pkg-config gperf"
		]
		install_system_packages(commands, no_confirm)

module_list = []
cmake_args = config.cmake_args
additional_build_targets = []

########## submodules ##########
os.chdir(root)
print_msg("Updating external libraries...")
execscript(scripts_dir +"/scripts/external_libs.py")

print_msg("Updating third-party libraries...")
execscript(scripts_dir +"/scripts/third_party_libs.py")

print_msg("Updating modules...")
execscript(scripts_dir +"/scripts/modules.py")

########## Third-Party Libraries ##########
print_msg("Building third-party libraries...")
if build_all:
	execscript(scripts_dir +"/build_third_party_libs.py")

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
	global toolsetArgs
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

	curDir = os.getcwd()

	g = {
		"module_list": module_list,
		"cmake_args": cmake_args,
		"additional_build_targets": additional_build_targets
	}
	l = {
		"generator": generator,
		"toolsetArgs": toolsetArgs,
		"toolsetCFlags": toolsetCFlags,
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

# Register argument-dependent modules
if with_essential_client_modules:
	add_pragma_module(
		name="pr_prosper_vulkan",
		commitSha="cea2de3151521e364efddfb506b78fe782b23464",
		repositoryUrl="https://github.com/Silverlan/pr_prosper_vulkan.git"
	)

if with_common_modules:
	add_pragma_module(
		name="pr_bullet",
		commitSha="8276afc2a19ebb51862acaecebf44fd6630c4d39",
		repositoryUrl="https://github.com/Silverlan/pr_bullet.git"
	)
	add_pragma_module(
		name="pr_audio_soloud",
		commitSha="8428da359ce2cb5e2fb302ceb9709ad6bc138ffd",
		repositoryUrl="https://github.com/Silverlan/pr_soloud.git"
	)
	add_pragma_module(
		name="pr_audio_dummy",
		commitSha="6fff8474681ffac9f365b068c20a93f4479162f1",
		repositoryUrl="https://github.com/Silverlan/pr_audio_dummy.git"
	)
	add_pragma_module(
		name="pr_prosper_opengl",
		commitSha="d7d80184ed7b94657a92c793d3d5c79efdaf0197",
		repositoryUrl="https://github.com/Silverlan/pr_prosper_opengl.git"
	)

if with_pfm:
	if with_core_pfm_modules or with_all_pfm_modules:
		add_pragma_module(
			name="pr_curl",
			commitSha="c243468a77de16ae4657e257ea965d1cac05fe8e",
			repositoryUrl="https://github.com/Silverlan/pr_curl.git"
		)
		add_pragma_module(
			name="pr_dmx",
			commitSha="34c268365b0d35ff2fa15f781c0f2ddfda65b993",
			repositoryUrl="https://github.com/Silverlan/pr_dmx.git"
		)
	if with_all_pfm_modules:
		add_pragma_module(
			name="pr_chromium",
			commitSha="03bea6f6e6d46e3d1687a85350ca9993a5f70151",
			repositoryUrl="https://github.com/Silverlan/pr_chromium.git"
		)
		add_pragma_module(
			name="pr_unirender",
			commitSha="9870ec431a456109cabc0e56867d7845c32292be",
			repositoryUrl="https://github.com/Silverlan/pr_cycles.git"
		)
		add_pragma_module(
			name="pr_xatlas",
			commitSha="800ee6203138ab59dc90bf7b9b351b55abf8d052",
			repositoryUrl="https://github.com/Silverlan/pr_xatlas.git"
		)
		add_pragma_module(
			name="pr_davinci",
			commitSha="9df824d14527767682caaec6237816900116b559",
			repositoryUrl="https://github.com/Silverlan/pr_davinci.git"
		)
		add_pragma_module(
			name="pr_opencv",
			commitSha="fdd1b1ec230b0cca6c822fe4dfa204415550a014",
			repositoryUrl="https://github.com/Silverlan/pr_opencv.git"
		)

if with_pfm:
	add_pragma_module(
		name="pr_git",
		commitSha="1d9d81afe838d2c1e28da45058cf7060c06cdbab",
		repositoryUrl="https://github.com/Silverlan/pr_git.git"
	)

if with_vr:
	add_pragma_module(
		name="pr_openvr",
		commitSha="b51c0b4692402469644901cb17dcaf15910df0a2",
		repositoryUrl="https://github.com/Silverlan/pr_openvr.git"
	)

if with_networking:
	add_pragma_module(
		name="pr_steam_networking_sockets",
		commitSha="ffef84e5bba8467370c7d447017ebf8e864c2a0f",
		repositoryUrl="https://github.com/Silverlan/pr_steam_networking_sockets.git",
		skipBuildTarget=True
	)

# These modules are shipped with the Pragma repository and will have to be excluded from the
# CMake configuration explicitly if they should be disabled.
shippedModules = ["pr_audio_dummy","pr_prosper_opengl","pr_prosper_vulkan","pr_curl"]

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
		if not moduleName in shippedModules:
			if moduleUrl:
				get_submodule(moduleName,moduleUrl,commitId,branch)

	os.chdir(moduleDir)
	scriptPath = moduleDir +"build_scripts/setup.py"
	if Path(scriptPath).is_file():
		print_msg("Executing module setup script...")
		execbuildscript(scriptPath)

	if not skipBuildTarget:
		module_list.append(moduleName)
	index += 1

for module in shippedModules:
	if module != "pr_curl": # Curl is currently required
		if not module in module_list:
			cmake_args.append("-DPRAGMA_DISABLE_MODULE_" +module +"=ON")
		else:
			cmake_args.append("-DPRAGMA_DISABLE_MODULE_" +module +"=OFF")

os.chdir(install_dir)
for url in modules_prebuilt:
	print_msg("Downloading prebuilt binaries for module '" +url +"'...")
	install_prebuilt_binaries(url)

if with_pfm:
	additional_build_targets.append("pfm")

cmake_args.append("-DPRAGMA_INSTALL_CUSTOM_TARGETS=" +";".join(module_list +additional_build_targets))

print("Modules:" +', '.join(module_list))
print("Additional CMake Arguments:" +', '.join(cmake_args))
print("Additional Build Targets:" +', '.join(additional_build_targets))

if not deps_only:
	########## Configure Pragma ##########
	print_msg("Configuring Pragma...")
	os.chdir(build_dir)

	print_msg("Running CMake configure...")
	cmake_args += [
		"-DCMAKE_INSTALL_PREFIX:PATH=" +install_dir +""
	]

	vtune_enabled = False
	if len(vtune_include_path) > 0 or len(vtune_library_path) > 0:
		if len(vtune_include_path) > 0 and len(vtune_library_path) > 0:
			print_msg("VTune profiler support is enabled!")
			vtune_enabled = True
			cmake_args += ["-DDEPENDENCY_VTUNE_PROFILER_INCLUDE=" +vtune_include_path]
			cmake_args += ["-DDEPENDENCY_VTUNE_PROFILER_LIBRARY=" +vtune_library_path]
		else:
			raise argparse.ArgumentError(None,"Both the --vtune-include-path and --vtune-library-path options have to be specified to enable VTune support!")

	cmake_args += [f"-DCONFIG_BUILD_WITH_VTUNE_SUPPORT={1 if vtune_enabled else 0}"]

	cmake_args += [f"-DWITH_PFM={1 if with_pfm else 0}"]
	cmake_args += [f"-DWITH_VR={1 if with_vr else 0}"]
	cmake_args += [f"-DWITH_COMMON_ENTITIES={1 if with_common_entities else 0}"]
	cmake_args += [f"-DWITH_COMMON_MODULES={1 if with_common_modules else 0}"]

	cmake_args += additional_cmake_args
	cmake_args.append("-DCMAKE_POLICY_VERSION_MINIMUM=4.0")
	cmake_args.append("-DPRAGMA_DEPS_DIR=" +config.deps_dir +"/" +config.deps_staging_dir)
	cmake_configure_def_toolset(root,generator,cmake_args)

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
		cmake_build(build_config,targets)

		print_msg("Build Successful! Pragma has been installed to \"" +normalize_path(install_dir) +"\".")
		print_msg("If you make any changes to the core source code, you can build the \"pragma-install\" target to compile the changes and re-install the binaries automatically.")
		print_msg("If you make any changes to a module, you will have to build the module target first, and then build \"pragma-install\".")
		print_msg("")

print_msg("All actions have been completed! Please make sure to re-run this script every time you pull any changes from the repository, and after adding any new modules.")

os.chdir(root)
