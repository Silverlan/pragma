# import os
from pathlib import Path
from sys import platform
from urllib.error import URLError, HTTPError
import tarfile
import argparse
import re
# import logging

from scripts.shared import *

parser = argparse.ArgumentParser(description='Pragma build script', allow_abbrev=False, formatter_class=argparse.ArgumentDefaultsHelpFormatter, epilog="")

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

if not os.path.isabs(install_dir):
	install_dir = build_dir +"/" +install_dir

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
print("install_directory: " +install_directory)
if platform == "linux":
	print("no_sudo: " +str(no_sudo))
	print("no_confirm: " +str(no_confirm))
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

########## clang-20 ##########
# Due to a compiler bug with C++20 Modules in clang, we have to use clang-20 for now,
# which is not available in package managers yet.
if platform == "linux" and (c_compiler == "clang-20" or c_compiler == "clang++-20"):
	curDir = os.getcwd()
	os.chdir(deps_dir)
	clang20_root = os.getcwd() +"/LLVM-20.1.1-Linux-X64"
	if not Path(clang20_root).is_dir():
		print_msg("Downloading clang-20...")
		http_extract("https://github.com/llvm/llvm-project/releases/download/llvmorg-20.1.1/LLVM-20.1.1-Linux-X64.tar.xz",format="tar.xz")
	if c_compiler == "clang-20":
		c_compiler = clang20_root +"/bin/clang"
	if cxx_compiler == "clang++-20":
		cxx_compiler = clang20_root +"/bin/clang++"
	print_msg("Setting c_compiler override to '" +c_compiler +"'")
	print_msg("Setting cxx_compiler override to '" +cxx_compiler +"'")
	os.chdir(curDir)

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
	global verbose
	global root
	global build_dir
	global deps_dir
	global install_dir
	global tools

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
		"reset_to_commit": reset_to_commit
	}
	if platform == "linux":
		l["c_compiler"] = c_compiler
		l["cxx_compiler"] = cxx_compiler
		l["no_confirm"] = no_confirm
		l["no_sudo"] = no_sudo

	execfile(filepath,g,l)

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
			"apt-get install ninja-build"
		]
		install_system_packages(commands, no_confirm)

module_list = []
cmake_args = []
additional_build_targets = []

########## submodules ##########
print_msg("Updating external libraries...")
execscript(scripts_dir +"/scripts/external_libs.py")

print_msg("Updating third-party libraries...")
execscript(scripts_dir +"/scripts/third_party_libs.py")

print_msg("Updating modules...")
execscript(scripts_dir +"/scripts/modules.py")

########## zlib ##########
# Download
os.chdir(deps_dir)
zlib_root = os.getcwd() +"/zlib"
zlib_lib_path = zlib_root +"/build/" +build_config
if platform == "linux":
	zlib_lib = zlib_lib_path +"/libz.a"
else:
	zlib_lib = zlib_lib_path +"/zs.lib"
zlib_include_dirs = zlib_root +" " +zlib_lib_path
if not Path(zlib_root).is_dir():
	print_msg("zlib not found. Downloading...")
	git_clone("https://github.com/Silverlan/zlib.git")
	os.chdir("zlib")
	reset_to_commit("6bc8ac0") # v1.3.1

	os.chdir("../")
os.chdir("zlib")

# Build
print_msg("Building zlib...")
mkdir("build",cd=True)
zlib_build_dir = os.getcwd()
cmake_configure_def_toolset("..",generator,["-DZLIB_BUILD_TESTING=OFF", "-DZLIB_BUILD_MINIZIP=OFF", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"])
cmake_build(build_config, ["zlib", "zlibstatic"])
if platform == "win32":
	zlib_conf_root = normalize_path(os.getcwd())
cp("zconf.h","../")
os.chdir("../..")

cmake_args += [
	"-DDEPENDENCY_ZLIB_INCLUDE=" +zlib_root,
	"-DDEPENDENCY_ZLIB_LIBRARY=" +zlib_lib
]

########## libzip ##########
ZLIB_SOURCE = normalize_path(zlib_root)
ZLIB_INCLUDE = normalize_path(zlib_root)
ZLIB_LIBPATH = normalize_path(zlib_lib_path)

# Download
os.chdir(deps_dir)
libzip_root = os.getcwd() +"/libzip"
if not Path(libzip_root).is_dir():
	print_msg("libzip not found. Downloading...")
	git_clone("https://github.com/nih-at/libzip.git")
	os.chdir("libzip")
	reset_to_commit("f30f529") # v1.11.3

	os.chdir("../")
os.chdir("libzip")

# Build
print_msg("Building libzip...")
mkdir("build",cd=True)
cmake_configure_def_toolset("..",generator,["-DLIBZIP_DO_INSTALL=OFF", "-DENABLE_BZIP2=OFF", "-DENABLE_LZMA=OFF", "-DZLIB_INCLUDE_DIR=" +ZLIB_INCLUDE,"-DZLIB_LIBRARY=" +zlib_lib])
cmake_build(build_config)
os.chdir(deps_dir)

libzip_lib_path = libzip_root +"/build/lib/" +build_config +"/"
if platform == "linux":
	libzip_lib = libzip_lib_path +"libzip.so"
else:
	libzip_lib = libzip_lib_path +"zip.lib"

cmake_args += [
	"-DDEPENDENCY_LIBZIP_INCLUDE=" +libzip_root +"/lib/",
	"-DDEPENDENCY_LIBZIP_BUILD_INCLUDE=" +libzip_root +"/build/",
	"-DDEPENDENCY_LIBZIP_LIBRARY=" +libzip_lib
]
if platform == "win32":
	mkpath(install_dir +"/bin/")
	cp(libzip_lib_path +"zip.dll", install_dir +"/bin/")
else:
	mkpath(install_dir +"/lib/")
	cp(libzip_lib_path +"libzip.so", install_dir +"/lib/")
	cp(libzip_lib_path +"libzip.so.5", install_dir +"/lib/")
	cp(libzip_lib_path +"libzip.so.5.5", install_dir +"/lib/")

########## libpng ##########
# Download
os.chdir(deps_dir)
libpng_root = os.getcwd() +"/libpng"
if not Path(libpng_root).is_dir():
	print_msg("libpng not found. Downloading...")
	git_clone("https://github.com/glennrp/libpng.git", branch = "libpng16")
	os.chdir("libpng")
	reset_to_commit("ea127968204cc5d10f3fc9250c306b9e8cbd9b80") # v1.6.48

	os.chdir("../")
os.chdir("libpng")

# Build
print_msg("Building libpng...")
mkdir("build",cd=True)
cmake_configure_def_toolset("..",generator,["-DPNG_SHARED=OFF","-DCMAKE_POLICY_VERSION_MINIMUM=3.5","-DZLIB_INCLUDE_DIR=" +ZLIB_INCLUDE,"-DZLIB_LIBRARY=" +ZLIB_LIBPATH, "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"])
cmake_build(build_config, ["png_static"])
os.chdir(deps_dir)

cmake_args += [
	"-DDEPENDENCY_LIBPNG_INCLUDE=" +libpng_root,
	"-DDEPENDENCY_LIBPNG_BUILD_INCLUDE=" +libpng_root +"/build/"
]
if platform == "linux":
	cmake_args += ["-DDEPENDENCY_LIBPNG_LIBRARY=" +libpng_root +"/build/" +build_config +"/libpng16.a"]
else:
	cmake_args += ["-DDEPENDENCY_LIBPNG_LIBRARY=" +libpng_root +"/build/" +build_config +"/libpng16_static.lib"]

########## icu ##########
# Download
os.chdir(deps_dir)
icu_root = os.getcwd() +"/icu"
if not Path(icu_root).is_dir():
	print_msg("icu not found. Downloading...")
	mkpath(icu_root)
	os.chdir(icu_root)
	base_url = "https://github.com/unicode-org/icu/releases/download/release-75-1/"
	if platform == "win32":
		http_extract(base_url +"icu4c-75_1-Win64-MSVC2022.zip")
	else:
		http_extract(base_url +"icu4c-75_1-Ubuntu22.04-x64.tgz",format="tar.gz")
if platform == "win32":
	cmake_args += [
		"-DDEPENDENCY_ICU_INCLUDE=" +icu_root +"/include/",
		"-DDEPENDENCY_ICU_ICUUC_LIBRARY=" +icu_root +"/lib64/icuuc.lib",
		"-DDEPENDENCY_ICU_ICUUC_BINARY=" +icu_root +"/bin64/icuuc75.dll",
		"-DDEPENDENCY_ICU_ICUDT_BINARY=" +icu_root +"/bin64/icudt75.dll"
	]
else:
	cmake_args += [
		"-DDEPENDENCY_ICU_INCLUDE=" +icu_root +"/icu/usr/local/include/",
		"-DDEPENDENCY_ICU_ICUUC_LIBRARY=" +icu_root +"/icu/usr/local/lib/libicuuc.so.75",
		"-DDEPENDENCY_ICU_ICUUC_BINARY=" +icu_root +"/icu/usr/local/lib/libicuuc.so.75",
		"-DDEPENDENCY_ICU_ICUDT_BINARY=" +icu_root +"/icu/usr/local/lib/libicudata.so.75"
	]

########## boost ##########
# Download
os.chdir(deps_dir)
boost_root = os.getcwd() +"/boost-1.88.0"
if not Path(boost_root).is_dir():
	print_msg("boost not found. Downloading...")
	boost_url = "https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.tar.gz"
	http_extract(boost_url,format="tar.gz")

########## Download Official Boost Sources (Headers) ##########
os.chdir(deps_dir)
# Choose URL based on platform
if platform == "win32":
    archive_url = "https://archives.boost.io/release/1.88.0/source/boost_1_88_0.zip"
    archive_format = "zip"
else:
    archive_url = "https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.gz"
    archive_format = "tar.gz"

headers_extract_dir = deps_dir +"/boost-1.88.0-headers"
if not Path(headers_extract_dir).is_dir():
    print_msg("Official Boost headers not found. Downloading official release for headers...")
    http_extract(archive_url, format=archive_format)
    # After extract, the archive usually unpacks to 'boost_1_88_0'
    unpack_root = deps_dir +"/boost_1_88_0"
    if Path(unpack_root).is_dir():
        Path(unpack_root).rename(headers_extract_dir)

########## Merge headers into CMake-enabled boost ##########
# Copy the single 'boost' folder with headers into the CMake tree
src_headers = headers_extract_dir +"/boost"
dst_headers = boost_root +"/boost"
if Path(src_headers).is_dir():
    if Path(dst_headers).is_dir():
        print_msg("Removing existing 'boost' headers in CMake tree...")
        shutil.rmtree(dst_headers)
    print_msg("Copying official Boost headers into CMake boost tree...")
    shutil.copytree(src_headers, dst_headers)
else:
    print_msg("ERROR: expected headers folder not found at {}".format(src_headers))
    sys.exit(1)

# Build
print_msg("Building boost...")

os.chdir(boost_root)

mkdir("build",cd=True)

boostArgs = ["-DBOOST_DISABLE_TESTS=ON","-DZLIB_INCLUDE_DIR=" +ZLIB_INCLUDE,"-DZLIB_LIBRARY=" +ZLIB_LIBPATH]
cmake_configure_def_toolset("..",generator,boostArgs)
cmake_build("Release")
os.chdir(deps_dir)

boost_lib_dir = boost_root +"/build/stage/lib/Release/"

########## LuaJIT ##########
print_msg("Building LuaJIT...")
if platform == "linux":
	os.chdir(root +"/third_party_libs/luajit/src")
	subprocess.run(["make","amalg","BUILDMODE=dynamic"],check=True)
	lua_jit_lib = normalize_path(root +"/third_party_libs/luajit/src/libluajit-p.so")
else:
	#devcmd_path = determine_vsdevcmd_path(deps_dir)
	os.chdir(root +"/third_party_libs/luajit/src")
	vcvars_path = determine_vsdevcmd_path(deps_dir)
	luajit_build_script = root+"/third_party_libs/luajit/src/msvcbuild.bat"
	luajit_build_script_wrapper = os.path.join(deps_dir,"luajit_build","build_luajit.bat")
	Path(os.path.join(deps_dir,"luajit_build")).mkdir(parents=True,exist_ok=True)
	print_msg("Generating luajit batch-script...")
	print("Writing '" +luajit_build_script_wrapper +"'...")
	with open(luajit_build_script_wrapper, 'w') as file:
		file.write("call \""+vcvars_path +"\" -arch=amd64 -host_arch=amd64\n") #TODO: allow arm64 to be usable by this.
		file.write("call \""+luajit_build_script +"\"\n")
	
	subprocess.check_call( [luajit_build_script_wrapper] )
	#subprocess.run([devcmd_path+" -no_logo & msvcbuild.bat"],check=True)    
	lua_jit_lib = normalize_path(root +"/third_party_libs/luajit/src/lua51.lib")
	# os.chdir(deps_dir)
	# mkdir("luajit_build")
	# os.chdir("luajit_build")
	# cmake_configure_def_toolset(root +"/third_party_libs/luajit",generator,["-DBUILD_SHARED_LIBS=1"])
	# cmake_build("Release")
    
	# lua_jit_lib = normalize_path(deps_dir +"/luajit_build/src/Release/luajit.lib")

########## GeometricTools ##########
os.chdir(deps_dir)
geometric_tools_root = normalize_path(os.getcwd() +"/GeometricTools")
if not Path(geometric_tools_root).is_dir():
	print_msg("GeometricTools not found. Downloading...")
	git_clone("https://github.com/davideberly/GeometricTools")
os.chdir("GeometricTools")
reset_to_commit("bd7a27d18ac9f31641b4e1246764fe30816fae74")
os.chdir("../../")

########## SPIRV-Tools ##########
print_msg("Downloading SPIRV-Tools...")
os.chdir(deps_dir)
if not Path(os.getcwd() +"/SPIRV-Tools").is_dir():
	git_clone("https://github.com/KhronosGroup/SPIRV-Tools.git")
os.chdir("SPIRV-Tools")
# Note: See the branches on https://github.com/KhronosGroup/SPIRV-Tools to find the correct commit for
# the target Vulkan SDK version.
# When updating to a newer version, the SPIRV-Headers commit below has to match
# the one defined in https://github.com/KhronosGroup/SPIRV-Tools/blob/<SHA>/DEPS
reset_to_commit("a62abcb")
os.chdir("../../")

########## SPIRV-Headers ##########
print_msg("Downloading SPIRV-Headers...")
os.chdir(deps_dir)
os.chdir("SPIRV-Tools/external")
if not Path(os.getcwd() +"/spirv-headers").is_dir():
	git_clone("https://github.com/KhronosGroup/SPIRV-Headers", "spirv-headers")
os.chdir("spirv-headers")
reset_to_commit("aa6cef192b8e693916eb713e7a9ccadf06062ceb")
os.chdir("../../")
os.chdir("../../")

########## SwiftShader ##########
if with_swiftshader:
	os.chdir(deps_dir)
	swiftshader_root = normalize_path(os.getcwd() +"/swiftshader")
	swiftshader_modules_dir = install_dir +"/modules/swiftshader/"

	swiftshader_bin_dir = swiftshader_root +"/build/bin/"
	if build_swiftshader:
		if not Path(swiftshader_root).is_dir():
			print_msg("SwiftShader not found. Downloading...")
			git_clone("https://github.com/Silverlan/swiftshader.git")
		os.chdir("swiftshader")
		reset_to_commit("8f431ea")
		
		print_msg("Building SwiftShader...")
		os.chdir("build")
		cmake_configure_def_toolset("..",generator)
		cmake_build("Release")
	else:
		if not Path(swiftshader_root).is_dir():
			mkpath(swiftshader_bin_dir)
			os.chdir(swiftshader_bin_dir)
			print_msg("Downloading prebuilt SwiftShader...")
			if platform == "win32":
				http_extract("https://github.com/Silverlan/swiftshader/releases/download/latest/swiftshader.zip")
			else:
				http_extract("https://github.com/Silverlan/swiftshader/releases/download/latest/swiftshader.tar.gz",format="tar.gz")
	print_msg("Installing SwiftShader...")
	mkpath(swiftshader_modules_dir)
	if platform == "win32":
		cp(swiftshader_bin_dir +"/vulkan-1.dll",swiftshader_modules_dir)
	else:
		cp(swiftshader_bin_dir +"/libvulkan.so.1",swiftshader_modules_dir)

########## vcpkg ##########
os.chdir(deps_dir)
if platform == "win32":
	os.environ["VCPKG_DEFAULT_TRIPLET"] = "x64-windows"
vcpkg_root = deps_dir +"/vcpkg"
if not Path(vcpkg_root).is_dir():
	print_msg("vcpkg not found, downloading...")
	git_clone("https://github.com/Microsoft/vcpkg.git")

os.chdir("vcpkg")
reset_to_commit("ee2d2a1")
os.chdir("..")
if platform == "linux":
	subprocess.run([vcpkg_root +"/bootstrap-vcpkg.sh","-disableMetrics"],check=True,shell=True)
else:
	subprocess.run([vcpkg_root +"/bootstrap-vcpkg.bat","-disableMetrics"],check=True,shell=True)

########## 7zip ##########
if platform == "win32":
	print_msg("Building 7zip...")
	subprocess.run([vcpkg_root +"/vcpkg","install","7zip"],check=True)
	mkpath(install_dir +"/bin/")
	cp(deps_dir +"/vcpkg/installed/x64-windows/bin/7zip.dll",install_dir +"/bin/")

########## bit7z ##########
os.chdir(deps_dir)
bit7z_root = normalize_path(os.getcwd() +"/bit7z")
if not Path(bit7z_root).is_dir():
	print_msg("bit7z not found. Downloading...")
	git_clone("https://github.com/rikyoz/bit7z.git")
os.chdir("bit7z")
reset_to_commit("0f03717") # v4.0.10

print_msg("Building bit7z...")
mkdir("build",cd=True)
bit7z_cmake_args = ["-DBIT7Z_AUTO_FORMAT=ON"]
if platform == "linux":
	bit7z_cmake_args.append("-DCMAKE_CXX_FLAGS=-fPIC")
cmake_configure_def_toolset("..",generator,bit7z_cmake_args)
cmake_build("Release")
if platform == "linux":
	bit7z_lib_name = "libbit7z.a"
else:
	bit7z_lib_name = "bit7z.lib"
cmake_args += ["-DDEPENDENCY_BIT7Z_INCLUDE=" +bit7z_root +"/include/", "-DDEPENDENCY_BIT7Z_LIBRARY=" +bit7z_root +"/lib/x64/Release/" +bit7z_lib_name]

########## cpptrace ##########
os.chdir(deps_dir)
cpptrace_root = normalize_path(os.getcwd() +"/cpptrace")
if not Path(cpptrace_root).is_dir():
	print_msg("cpptrace not found. Downloading...")
	git_clone("https://github.com/jeremy-rifkin/cpptrace.git")
os.chdir("cpptrace")
reset_to_commit("34ea957") # v0.8.0

print_msg("Building cpptrace...")
mkdir("build",cd=True)
cpptrace_cmake_args = ["-DBUILD_SHARED_LIBS=ON"]
cmake_configure_def_toolset("..",generator,cpptrace_cmake_args)
cmake_build(build_config)
if platform == "linux":
	cpptrace_lib_name = "libcpptrace.so"
else:
	cpptrace_lib_name = "cpptrace.lib"
cpptrace_bin_dir = cpptrace_root +"/build/" +build_config +"/"
cmake_args += ["-DDEPENDENCY_CPPTRACE_INCLUDE=" +cpptrace_root +"/include/", "-DDEPENDENCY_CPPTRACE_LIBRARY=" +cpptrace_bin_dir +cpptrace_lib_name]

########## compressonator deps ##########
os.chdir(root+"/external_libs/util_image/third_party_libs/compressonator")
execfile(root+"/external_libs/util_image/third_party_libs/compressonator/build/fetch_dependencies.py")

########## freetype (built in win32, sys in linux (set in cmake)) ##########
freetype_include_dir = ""
freetype_lib = ""
if platform == "win32":
	print_msg("Downloading freetype...")
	os.chdir(deps_dir)
	if not Path(os.getcwd()+"/freetype").is_dir():
		git_clone("https://github.com/freetype/freetype")
	freetype_root = deps_dir+"/freetype"
	os.chdir("freetype")
	subprocess.run(["git","reset","--hard","42608f77"],check=True)
	mkdir("build",cd=True)
	freetype_cmake_args =[
		"-DCMAKE_MODULE_PATH="+deps_dir_fs+"/zlib_prefix",
		"-DCMAKE_PREFIX_PATH="+deps_dir_fs+"/zlib_prefix"
	]
	freetype_cmake_args += [
		"-DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE",
		"-DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE",
		"-DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE"
	]
	freetype_cmake_args.append("-DCMAKE_POLICY_VERSION_MINIMUM=4.0")

	print_msg("Building freetype...")
	cmake_configure_def_toolset(freetype_root,generator,freetype_cmake_args)
	cmake_build("Release")
	freetype_include_dir += freetype_root+"/include"
	freetype_lib += freetype_root+"/build/Release/freetype.lib"

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

def add_pragma_module_prebuilt(name,engineVersion=""):
	url = "https://github.com/" +name +"/releases/download/"
	if len(engineVersion) > 0:
		url = url +engineVersion +"/"
	else:
		url = url +"latest/"

	modules_prebuilt.append(url)

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

		"install_directory": install_directory,
		"verbose": verbose,

		"root": root,
		"build_dir": build_dir,
		"deps_dir": deps_dir,
		"install_dir": install_dir,
		"tools": tools,

		"build_all": build_all,
		"zlib_root": zlib_root,
		"zlib_lib": zlib_lib,
		"zlib_lib_dir": zlib_lib_path,
		"zlib_include_dirs": zlib_include_dirs,
		"boost_root": boost_root,
		"boost_lib_dir": boost_lib_dir,
		"geometric_tools_root": geometric_tools_root,
		"vcpkg_root": vcpkg_root,

		"freetype_include_dir":freetype_include_dir,
		"freetype_lib": freetype_lib,

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
		commitSha="1e59eb873ef59b7456cb20e3dc5073f823de3fc9",
		repositoryUrl="https://github.com/Silverlan/pr_prosper_vulkan.git"
	)

if with_common_modules:
	add_pragma_module(
		name="pr_bullet",
		commitSha="b4a0af6d3db48d322b63a757ba5574fda87b99d8",
		repositoryUrl="https://github.com/Silverlan/pr_bullet.git"
	)
	add_pragma_module(
		name="pr_audio_soloud",
		commitSha="ae89cb889a7fa89ca47cdf878cb7f91eef8ba0a3",
		repositoryUrl="https://github.com/Silverlan/pr_soloud.git"
	)
	add_pragma_module(
		name="pr_audio_dummy",
		commitSha="1a806a1a7b2283bd8551d07e4f1d680499f68b90",
		repositoryUrl="https://github.com/Silverlan/pr_audio_dummy.git"
	)
	add_pragma_module(
		name="pr_prosper_opengl",
		commitSha="e34cbca22a0a6ebd8a5d7f85c29f91e99555857a",
		repositoryUrl="https://github.com/Silverlan/pr_prosper_opengl.git"
	)
	add_pragma_module_prebuilt("Silverlan/pr_mount_external_prebuilt")
	add_pragma_module_prebuilt("Silverlan/pr_rig_prebuilt")
	add_pragma_module_prebuilt("Silverlan/pr_ik_prebuilt")

if with_pfm:
	if with_core_pfm_modules or with_all_pfm_modules:
		add_pragma_module(
			name="pr_curl",
			commitSha="974c67cc76710809a9595fcfbc4167554799cd7f",
			repositoryUrl="https://github.com/Silverlan/pr_curl.git"
		)
		add_pragma_module(
			name="pr_dmx",
			commitSha="eb6b4c8e0cd45dc27cc7ed46ff50eb5a9cae1fba",
			repositoryUrl="https://github.com/Silverlan/pr_dmx.git"
		)
	if with_all_pfm_modules:
		add_pragma_module(
			name="pr_chromium",
			commitSha="6ab1bc749bc8c4f35701c75974e03bd4fe21d671",
			repositoryUrl="https://github.com/Silverlan/pr_chromium.git"
		)
		add_pragma_module(
			name="pr_unirender",
			commitSha="c88dd27e8af2afc1bf9ab555a3c2b3ded5c0780a",
			repositoryUrl="https://github.com/Silverlan/pr_cycles.git"
		)
		add_pragma_module(
			name="pr_xatlas",
			commitSha="b6c7e6c466a3a70183b19b1f329e8bd79544a1e7",
			repositoryUrl="https://github.com/Silverlan/pr_xatlas.git"
		)
		add_pragma_module(
			name="pr_davinci",
			commitSha="65ffb75ab860d32e255a9d0d2009fd2006e48aee",
			repositoryUrl="https://github.com/Silverlan/pr_davinci.git"
		)
		add_pragma_module(
			name="pr_opencv",
			commitSha="430e1cc87e741306753d1bdc3091bd04d537d6b1",
			repositoryUrl="https://github.com/Silverlan/pr_opencv.git"
		)

if with_pfm:
	add_pragma_module(
		name="pr_git",
		commitSha="08b2d74f2c8641f791936a868d562e61423cc040",
		repositoryUrl="https://github.com/Silverlan/pr_git.git"
	)

if with_vr:
	add_pragma_module(
		name="pr_openvr",
		commitSha="31a765d7b3df1e9eb26decb7242b80635a40ed66",
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

cmake_args.append("-DPRAGMA_INSTALL_CUSTOM_TARGETS=" +";".join(module_list +additional_build_targets))

print("Modules:" +', '.join(module_list))
print("Additional CMake Arguments:" +', '.join(cmake_args))
print("Additional Build Targets:" +', '.join(additional_build_targets))


########## Configure Pragma ##########
print_msg("Configuring Pragma...")
os.chdir(build_dir)

print_msg("Running CMake configure...")
cmake_args += [
	"-DDEPENDENCY_GEOMETRIC_TOOLS_INCLUDE=" +deps_dir +"/GeometricTools/GTE",
	"-DDEPENDENCY_SPIRV_TOOLS_DIR=" +deps_dir +"/SPIRV-Tools",
	"-DBUILD_TESTING=OFF",
	"-DCMAKE_INSTALL_PREFIX:PATH=" +install_dir +"",
	"-DDEPENDENCY_FREETYPE_INCLUDE="+freetype_include_dir,
	"-DDEPENDENCY_FREETYPE_LIBRARY="+freetype_lib,
	"-DDEPENDENCY_LUAJIT_LIBRARY=" +lua_jit_lib +"",
	"-DDEPENDENCY_LUA_LIBRARY=" +lua_jit_lib +""
]

def find_boost_libs(lib_dir, components):
    """
    Scan lib_dir for Boost library files matching each component name.
    Returns a dict: { component_name: absolute-path-to-library or None }.
    """
    lib_dir = Path(lib_dir)
    if not lib_dir.is_dir():
        raise ValueError(f"Boost lib directory not found: {lib_dir}")

    # list all candidate files once
    all_libs = [p for p in lib_dir.iterdir() if p.is_file()]

    found = {}
    for comp in components:
        # regex to match boost_<component> in filename
        pat = re.compile(rf"boost_{re.escape(comp)}", re.IGNORECASE)
        matches = [p for p in all_libs if pat.search(p.name)]
        if not matches:
            found[comp] = None
            continue

        # prefer release (no "-gd" or "-debug")
        release = [p for p in matches if not re.search(r"-g(d|d-)|debug", p.name, re.IGNORECASE)]
        chosen = release[0] if release else matches[0]
        found[comp] = chosen.resolve()
    return found

components = ["chrono", "date_time", "thread"]
boost_libs = find_boost_libs(boost_lib_dir, components)

for comp in components:
	lib_path = boost_libs.get(comp)
	if not lib_path:
		raise RuntimeError(f"Could not locate Boost {comp} library in {boost_lib_dir}")
	var_name = comp.upper()
	cmake_args.append(f"-DDEPENDENCY_BOOST_{var_name}_LIBRARY={lib_path}")

cmake_args.append("-DDEPENDENCY_BOOST_LIBRARY_LOCATION=" +boost_lib_dir)
cmake_args.append("-DBOOST_LIBRARYDIR=" +boost_lib_dir)
cmake_args.append("-DBOOST_ROOT=" +boost_root)
cmake_args.append("-DDEPENDENCY_BOOST_INCLUDE=" +boost_root)


if platform == "linux":
	cmake_args += ["-DDEPENDENCY_LIBZIP_CONF_INCLUDE=" +build_dir +"/third_party_libs/libzip"]
else:
	cmake_args += ["-DZLIB_INCLUDE_DIRS=" +build_dir +"/third_party_libs/zlib " +zlib_conf_root]

cmake_args.append("-DPME_EXTERNAL_LIB_LOCATION=" +external_libs_dir)
cmake_args.append("-DPME_EXTERNAL_LIB_BIN_LOCATION=" +external_libs_bin_dir)
cmake_args.append("-DPME_THIRD_PARTY_LIB_LOCATION=" +third_party_libs_dir)

if len(vtune_include_path) > 0 or len(vtune_library_path) > 0:
	if len(vtune_include_path) > 0 and len(vtune_library_path) > 0:
		print_msg("VTune profiler support is enabled!")
		cmake_args += ["-DCONFIG_BUILD_WITH_VTUNE_SUPPORT=1"]
		cmake_args += ["-DDEPENDENCY_VTUNE_PROFILER_INCLUDE=" +vtune_include_path]
		cmake_args += ["-DDEPENDENCY_VTUNE_PROFILER_LIBRARY=" +vtune_library_path]
	else:
		raise argparse.ArgumentError(None,"Both the --vtune-include-path and --vtune-library-path options have to be specified to enable VTune support!")

cmake_args += additional_cmake_args
cmake_args.append("-DCMAKE_POLICY_VERSION_MINIMUM=4.0")
cmake_configure_def_toolset(root,generator,cmake_args)

print_msg("Build files have been written to \"" +build_dir +"\".")

if platform == "win32":
	# Hack: zlib is using a configured header which is generated during the configuration step.
	# Unfortunately this header is in a different location than the regular header files, which causes
	# a lot of issues, so we'll copy it over to where we need it.
	# TODO: Implement a proper solution for this
	curDir = os.getcwd()
	os.chdir(zlib_build_dir)
	cp("zconf.h",libzip_root +"/lib/")
	cp(libzip_root +"/build/zipconf.h",root +"/external_libs/util_zip/include")
	os.chdir(curDir)
	#

# 7z binaries (required for bit7z)
os.chdir(deps_dir)
sevenz_root = normalize_path(os.getcwd() +"/7z-lib")
if platform == "win32":
	if not Path(sevenz_root).is_dir():
		print_msg("7z-lib not found. Downloading...")
		git_clone("https://github.com/Silverlan/7z-lib.git")
	os.chdir("7z-lib")
	reset_to_commit("1a9ec9a")
	cp(sevenz_root +"/win-x64/7z.dll",install_dir +"/bin/")
else:
	if not Path(sevenz_root).is_dir():
		print_msg("7z-lib not found. Downloading...")
		mkdir("7z-lib",cd=True)
		http_extract("https://7-zip.org/a/7z2408-src.tar.xz",format="tar.xz")
	os.chdir(sevenz_root)
	sevenz_so_path = sevenz_root +"/CPP/7zip/Bundles/Format7zF"
	os.chdir(sevenz_so_path)
	subprocess.run(["make","-j","-f","../../cmpl_gcc.mak"],check=True)
	mkpath(install_dir +"/bin")
	cp(sevenz_so_path +"/b/g/7z.so",install_dir +"/bin/7z.so")

########## install cpptrace ##########
if platform == "win32":
	mkpath(install_dir +"/bin/")
	cp(cpptrace_bin_dir +"cpptrace.dll",install_dir +"/bin/cpptrace.dll")
else:
	mkpath(install_dir +"/lib/")
	cp(cpptrace_bin_dir +"libcpptrace.so",install_dir +"/lib/libcpptrace.so")
	cp(cpptrace_bin_dir +"libcpptrace.so.0",install_dir +"/lib/libcpptrace.so.0")
	cp(cpptrace_bin_dir +"libcpptrace.so.0.8.0",install_dir +"/lib/libcpptrace.so.0.8.0")

########## Lua Extensions ##########
lua_ext_dir = deps_dir +"/lua_extensions"
mkdir(lua_ext_dir,cd=True)

if with_lua_debugger:
	# MoDebug
	mob_debug_root = lua_ext_dir +"/MobDebug-0.80"
	if not Path(mob_debug_root).is_dir():
		print_msg("MobDebug not found. Downloading...")
		if platform == "win32":
			zipName = "0.80.zip"
			http_extract("https://github.com/pkulchenko/MobDebug/archive/refs/tags/" +zipName)
		else:
			zipName = "0.80.tar.gz"
			http_extract("https://github.com/pkulchenko/MobDebug/archive/refs/tags/" +zipName,format="tar.gz")
	mkdir(install_dir +"/lua/modules/")
	cp(lua_ext_dir +"/MobDebug-0.80/src/mobdebug.lua",install_dir +"/lua/modules/")

	# Socket
	curDir = os.getcwd()
	os.chdir(lua_ext_dir)
	luasocket_root = lua_ext_dir +"/luasocket"
	if not Path(luasocket_root).is_dir():
		print_msg("luasocket not found. Downloading...")
		git_clone("https://github.com/LuaDist/luasocket.git")

	print_msg("Building luasocket...")
	os.chdir(luasocket_root)
	mkdir("build",cd=True)
	luasocket_args = ["-DLUA_INCLUDE_DIR=" +root +"/third_party_libs/luajit/src"]
	if platform == "win32":
		luasocket_args.append("-DLUA_LIBRARY=" +root +"/third_party_libs/luajit/src/lua51.lib")
	else:
		luasocket_args.append("-DLUA_LIBRARY=" +root +"/third_party_libs/luajit/src/libluajit-p.so")
	luasocket_args.append("-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
	cmake_configure_def_toolset("..",generator,luasocket_args)
	cmake_build(build_config)
	cp(luasocket_root +"/src/socket.lua",install_dir +"/lua/modules/")
	mkdir(install_dir +"/modules/socket/")
	if platform == "win32":
		cp(luasocket_root +"/build/socket/" +build_config +"/core.dll",install_dir +"/modules/socket/")
	else:
		cp(luasocket_root +"/build/socket/"+build_config +"/core.so",install_dir +"/modules/socket/")
	os.chdir(curDir)

########## lua-debug ##########
if with_lua_debugger:
	curDir = os.getcwd()
	os.chdir(scripts_dir)
	execscript(scripts_dir +"/scripts/build_lua_debug.py")
	os.chdir(curDir)

########## Addons ##########
def download_addon(name,addonName,url,commitId=None):
	print_msg("Downloading " +name +" addon...")
	mkdir(install_dir +"/addons",cd=True)
	if not Path(install_dir +"/addons/" +addonName).is_dir():
		git_clone(url,addonName)
	if commitId is not None:
		os.chdir(install_dir +"/addons/" +addonName)
		reset_to_commit(commitId)
		os.chdir("..")

	# Write commit SHA info for debugging purposes
	os.chdir(install_dir +"/addons/" +addonName)
	try:
		commit_id = subprocess.check_output(["git", "rev-parse", "HEAD"]).decode().strip()
	except subprocess.CalledProcessError:
		if os.path.exists("git_info.txt"):
			os.remove("git_info.txt")
	else:
		with open("git_info.txt", "w") as f:
			f.write(f"commit: {commit_id}\n")
	os.chdir("..")

curDir = os.getcwd()
if not skip_repository_updates:
	if with_pfm:
		download_addon("PFM","filmmaker","https://github.com/Silverlan/pfm.git","6c92b31c3fe297e0610e597bdb0f208e0336cb25")
		download_addon("model editor","tool_model_editor","https://github.com/Silverlan/pragma_model_editor.git","a9ea4820f03be250bdf1e6951dad313561b75b17")

	if with_vr:
		download_addon("VR","virtual_reality","https://github.com/Silverlan/PragmaVR.git","e4728fc7c5fa92059167b73c4f56deaa5bfa062a")

	if with_pfm:
		download_addon("PFM Living Room Demo","pfm_demo_living_room","https://github.com/Silverlan/pfm_demo_living_room.git","4cbecad4a2d6f502b6d9709178883678101f7e2c")
		download_addon("PFM Bedroom Demo","pfm_demo_bedroom","https://github.com/Silverlan/pfm_demo_bedroom.git","0fed1d5b54a25c3ded2ce906e7da80ca8dd2fb0d")
		download_addon("PFM Tutorials","pfm_tutorials","https://github.com/Silverlan/pfm_tutorials.git","6a410c53a99194ae3ed9fe740306fe27150bfcf5")

	if with_common_entities:
		download_addon("HL","pragma_hl","https://github.com/Silverlan/pragma_hl.git","7d146f517a9d514e9c22ca918460b85b27694155")
		download_addon("TF2","pragma_tf2","https://github.com/Silverlan/pragma_tf2.git","9cf3dc9a1a5fef4cc18b85fc2646cf4263134e9b")

	download_addon("Matcaps","matcaps","https://github.com/Silverlan/pragma_matcaps.git","304800623fc6c07901053fb41428fa2c430f2d0d")

os.chdir(curDir)

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
	if with_pfm:
		targets.append("pfm")
	targets += additional_build_targets
	targets.append("pragma-install")

	print_msg("Running build command...")
	cmake_build(build_config,targets)

	print_msg("Build Successful! Pragma has been installed to \"" +normalize_path(install_dir) +"\".")
	print_msg("If you make any changes to the core source code, you can build the \"pragma-install\" target to compile the changes and re-install the binaries automatically.")
	print_msg("If you make any changes to a module, you will have to build the module target first, and then build \"pragma-install\".")
	print_msg("")

print_msg("All actions have been completed! Please make sure to re-run this script every time you pull any changes from the repository, and after adding any new modules.")

os.chdir(root)
