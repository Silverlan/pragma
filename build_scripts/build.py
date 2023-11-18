import os
from pathlib import Path
from sys import platform
from distutils.dir_util import copy_tree
import argparse
import re
# import logging

from scripts.shared import *

parser = argparse.ArgumentParser(description='Pragma build script', allow_abbrev=False, formatter_class=argparse.ArgumentDefaultsHelpFormatter, epilog="")

# See https://stackoverflow.com/a/43357954/1879228 for boolean args
if platform == "linux":
	parser.add_argument('--c-compiler', help='The C-compiler to use.', default='clang-15')
	parser.add_argument('--cxx-compiler', help='The C++-compiler to use.', default='clang++-15')
	defaultGenerator = "Unix Makefiles"
else:
	defaultGenerator = "Visual Studio 17 2022"
parser.add_argument('--generator', help='The generator to use.', default=defaultGenerator)
if platform == "win32":
	parser.add_argument('--vcvars', help='Path to vcvars64.bat.', default="\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"")
parser.add_argument("--with-essential-client-modules", type=str2bool, nargs='?', const=True, default=True, help="Include essential modules required to run Pragma.")
parser.add_argument("--with-common-modules", type=str2bool, nargs='?', const=True, default=True, help="Include non-essential but commonly used modules (e.g. audio and physics modules).")
parser.add_argument("--with-pfm", type=str2bool, nargs='?', const=True, default=False, help="Include the Pragma Filmmaker.")
parser.add_argument("--with-core-pfm-modules", type=str2bool, nargs='?', const=True, default=True, help="Include essential PFM modules.")
parser.add_argument("--with-all-pfm-modules", type=str2bool, nargs='?', const=True, default=False, help="Include non-essential PFM modules (e.g. chromium and cycles).")
parser.add_argument("--with-vr", type=str2bool, nargs='?', const=True, default=False, help="Include Virtual Reality support.")
parser.add_argument("--with-source-engine-entities", type=str2bool, nargs='?', const=True, default=True, help="Include addons with support for Source Engine entities.")
parser.add_argument("--with-lua-debugger", type=str2bool, nargs='?', const=True, default=False, help="Include Lua-debugger support.")
parser.add_argument("--with-lua-doc-generator", type=str2bool, nargs='?', const=True, default=False, help="Include Lua documentation generator.")
parser.add_argument("--build", type=str2bool, nargs='?', const=True, default=True, help="Build Pragma after configurating and generating build files.")
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
if platform == "linux":
	parser.add_argument("--no-sudo", type=str2bool, nargs='?', const=True, default=False, help="Will not run sudo commands. System packages will have to be installed manually.")
	parser.add_argument("--no-confirm", type=str2bool, nargs='?', const=True, default=False, help="Disable any interaction with user (suitable for automated run).")
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

if args["rerun"]:
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
generator = args["generator"]
#if platform == "win32":
#	vcvars = args["vcvars
with_essential_client_modules = args["with_essential_client_modules"]
with_common_modules = args["with_common_modules"]
with_pfm = args["with_pfm"]
with_core_pfm_modules = args["with_core_pfm_modules"]
with_all_pfm_modules = args["with_all_pfm_modules"]
with_vr = args["with_vr"]
with_source_engine_entities = args["with_source_engine_entities"]
with_lua_debugger = args["with_lua_debugger"]
with_lua_doc_generator = args["with_lua_doc_generator"]
build = args["build"]
build_all = args["build_all"]
build_config = args["build_config"]
build_directory = args["build_directory"]
deps_directory = args["deps_directory"]
install_directory = args["install_directory"]
additional_cmake_args = args["cmake_arg"]
scripts_dir = os.getcwd() +"/build_scripts"
#log_file = args["log_file"]
verbose = args["verbose"]
modules = args["module"]
rerun = args["rerun"]
update = args["update"]
modules_prebuilt = []

print("Inputs:")
if platform == "linux":
	print("cxx_compiler: " +cxx_compiler)
	print("c_compiler: " +c_compiler)

	if with_lua_doc_generator:
		with_lua_doc_generator = 0
		print_warning("Lua documentation generator is only supported on Windows! --with-lua-doc-generator flag will be ignored.")

print("generator: " +generator)
#if platform == "win32":
#	print("vcvars: " +vcvars)
print("with_essential_client_modules: " +str(with_essential_client_modules))
print("with_common_modules: " +str(with_common_modules))
print("with_pfm: " +str(with_pfm))
print("with_core_pfm_modules: " +str(with_core_pfm_modules))
print("with_all_pfm_modules: " +str(with_all_pfm_modules))
print("with_vr: " +str(with_vr))
print("with_lua_debugger: " +str(with_lua_debugger))
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

if platform == "linux":
	os.environ["CC"] = c_compiler
	os.environ["CXX"] = cxx_compiler

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

if update:
	os.chdir(root)

	print_msg("Updating Pragma repository...")
	subprocess.run(["git","pull"],check=True)


mkpath(build_dir)
mkpath(deps_dir)
mkpath(install_dir)
mkpath(tools)

def execscript(filepath):
	global generator
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
		"git_clone": git_clone,
		"cmake_configure": cmake_configure,
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
			"add-apt-repository ppa:savoury1/llvm-defaults-14",
			"apt update",
			"apt install clang-15",
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

			# Required for Curl
			"apt-get install libssl-dev",
			"apt install libssh2-1",

			# Required for OIIO
			"apt-get install python3-distutils",

            #install freetype for linking. X server frontends (Gnome, KDE etc) already include it somewhere down the line. Also install pkg-config for easy export of flags.
            "apt-get install pkg-config libfreetype-dev",


			#Ninja
			"apt-get install ninja-build"
		]

		print("")
		print_msg("The following system packages will be installed:")
		for cmd in commands:
			print(cmd)

		if not no_confirm:
			user_input = input("Your password may be required to install them. Do you want to continue (Y/n)?")
			if user_input.lower() == 'yes' or user_input.lower() == 'y':
				pass
			elif user_input.lower() == 'no' or user_input.lower() == 'n':
				sys.exit(0)
			else:
				print("Invalid input, please type 'y' for yes or 'n' for no. Aborting...")
				sys.exit(0)

		print_msg("Installing system packages...")
		for cmd in commands:
			print_msg("Running " +cmd +"...")
			subprocess.run(["sudo"] +cmd.split() +["-y"],check=True)

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
zlib_root = os.getcwd() +"/zlib-1.2.8"
if platform == "linux":
	zlib_lib_path = zlib_root +"/build"
	zlib_lib = zlib_lib_path +"/libz.a"
else:
	zlib_lib_path = zlib_root +"/build/" +build_config
	zlib_lib = zlib_lib_path +"/zlibstatic.lib"
zlib_include_dirs = zlib_root +" " +zlib_lib_path
if not Path(zlib_root).is_dir():
	print_msg("zlib not found. Downloading...")
	git_clone("https://github.com/fmrico/zlib-1.2.8.git")

os.chdir("zlib-1.2.8")

# Build
print_msg("Building zlib...")
mkdir("build",cd=True)
cmake_configure("..",generator)
cmake_build(build_config)
if platform == "win32":
	zlib_conf_root = normalize_path(os.getcwd())
cp("zconf.h","../")
os.chdir("../..")

########## boost ##########
# Download
os.chdir(deps_dir)
if platform == "linux":
	boost_root = os.getcwd() +"/boost_1_78_0"
	if not Path(boost_root).is_dir():
		print_msg("boost not found. Downloading...")
		zipName = "boost_1_78_0.zip"
		http_extract("https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/" +zipName)
else:
	boost_root = os.getcwd() +"/boost"
	if not Path(boost_root).is_dir():
		print_msg("boost not found. Downloading...")
		zipName = "boost_1_78_0.zip"
		git_clone("https://github.com/ClausKlein/boost-cmake.git","boost")

# Build
print_msg("Building boost...")

os.chdir(boost_root)
ZLIB_SOURCE = normalize_path(zlib_root)
ZLIB_INCLUDE = normalize_path(zlib_root)
ZLIB_LIBPATH = normalize_path(zlib_lib_path)
if platform == "linux":
	subprocess.run([boost_root +"/bootstrap.sh"],check=True,shell=True)
	subprocess.run(["./b2","address-model=64","stage","variant=release","link=shared","runtime-link=shared","-j3"],check=True,shell=True)
	subprocess.run(["./b2","address-model=64","stage","variant=release","link=static","runtime-link=shared","-j3"],check=True,shell=True)

	print_msg("Building boost zlib libraries...")
	subprocess.run(["./b2","address-model=64","stage","variant=release","link=shared","runtime-link=shared","--with-iostreams","-sZLIB_SOURCE=" +ZLIB_SOURCE,"-sZLIB_INCLUDE=" +ZLIB_INCLUDE,"-sZLIB_LIBPATH=" +ZLIB_LIBPATH],check=True,shell=True)
	subprocess.run(["./b2","address-model=64","stage","variant=release","link=static","runtime-link=shared","--with-iostreams","-sZLIB_SOURCE=" +ZLIB_SOURCE,"-sZLIB_INCLUDE=" +ZLIB_INCLUDE,"-sZLIB_LIBPATH=" +ZLIB_LIBPATH],check=True,shell=True)
else:
	mkdir("build",cd=True)

	cmake_configure("..",generator,["-DBOOST_DISABLE_TESTS=ON","-DZLIB_INCLUDE_DIR=" +ZLIB_INCLUDE,"-DZLIB_LIBRARY=" +ZLIB_LIBPATH])
	cmake_build("Release")
	os.chdir("../..")

	os.chdir("..")

########## LuaJIT ##########
print_msg("Building LuaJIT...")
if platform == "linux":
	os.chdir(root +"/third_party_libs/luajit/src")
	subprocess.run(["make"],check=True)
else:
	os.chdir(deps_dir)
	mkdir("luajit_build")
	os.chdir("luajit_build")
	cmake_configure(root +"/third_party_libs/luajit",generator,["-DBUILD_SHARED_LIBS=1"])
	cmake_build("Release")

	lua_jit_lib = normalize_path(deps_dir +"/luajit_build/src/Release/luajit.lib")

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

# Note: When updating to a newer version, the SPIRV-Headers commit below has to match
# the one defined in https://github.com/KhronosGroup/SPIRV-Tools/blob/master/DEPS for the
# timestamp of this commit
reset_to_commit("7826e19")
os.chdir("../../")

########## SPIRV-Headers ##########
print_msg("Downloading SPIRV-Headers...")
os.chdir(deps_dir)
os.chdir("SPIRV-Tools/external")
if not Path(os.getcwd() +"/SPIRV-Headers").is_dir():
	git_clone("https://github.com/KhronosGroup/SPIRV-Headers")
os.chdir("SPIRV-Headers")
reset_to_commit("4995a2f2723c401eb0ea3e10c81298906bf1422b")
os.chdir("../../")
os.chdir("../../")

########## vcpkg ##########
os.chdir(deps_dir)
if platform == "win32":
	os.environ["VCPKG_DEFAULT_TRIPLET"] = "x64-windows"
vcpkg_root = deps_dir +"/vcpkg"
if not Path(vcpkg_root).is_dir():
	print_msg("vcpkg not found, downloading...")
	git_clone("https://github.com/Microsoft/vcpkg.git")

os.chdir("vcpkg")
reset_to_commit("3b7578831da081ba164be30da8d9382a64841059")
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

########## zlib (for freetype) ############
if platform == "win32":
    os.chdir(deps_dir)
    mkdir("zlib_build",cd=True)
    zlib_cmake_args = [
    		"-DCMAKE_INSTALL_PREFIX="+deps_dir_fs+"/zlib_prefix",
    		"-DBUILD_SHARED_LIBS=ON"
    		]
    cmake_configure(root+"/third_party_libs/zlib",generator,zlib_cmake_args)
    cmake_build("Release")
    cmake_build("Release",["install"])

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
    subprocess.run(["git","reset","--hard","fbbcf50367403a6316a013b51690071198962920"],check=True)
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

    print_msg("Building freetype...")
    cmake_configure(freetype_root,generator,freetype_cmake_args)
    cmake_build("Release")
    freetype_include_dir += freetype_root+"/include"
    freetype_lib += freetype_root+"/build/Release/freetype.lib"

########## Modules ##########
print_msg("Downloading modules...")
os.chdir(root +"/modules")

module_info = []
def add_pragma_module(name,repositoryUrl=None,commitSha=None,branch=None):
    for module in module_info:
        if module["name"] == name:
            return
    module = {
        "name": name,
        "repositoryUrl": repositoryUrl,
        "commitSha": commitSha,
        "branch": branch
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

def execbuildscript(filepath):
	global module_list
	global cmake_args
	global additional_build_targets
	global generator
	global moduleUrl
	global moduleDir
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

	g = {
		"module_list": module_list,
		"cmake_args": cmake_args,
		"additional_build_targets": additional_build_targets
	}
	l = {
		"generator": generator,
		"moduleUrl": moduleUrl,
		"moduleDir": moduleDir,
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
		"zlib_root": zlib_root,
		"zlib_lib": zlib_lib,
		"zlib_lib_dir": zlib_lib_path,
		"zlib_include_dirs": zlib_include_dirs,
		"boost_root": boost_root,
		"geometric_tools_root": geometric_tools_root,
		"vcpkg_root": vcpkg_root,

		"freetype_include_dir":freetype_include_dir,
		"freetype_lib": freetype_lib,

		"normalize_path": normalize_path,
		"mkpath": mkpath,
		"print_msg": print_msg,
		"git_clone": git_clone,
		"cmake_configure": cmake_configure,
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
		"reset_to_commit": reset_to_commit
	}
	if platform == "linux":
		l["c_compiler"] = c_compiler
		l["cxx_compiler"] = cxx_compiler
		l["no_confirm"] = no_confirm
		l["no_sudo"] = no_sudo
	#	l["harfbuzz_include_dir"] = harfbuzz_include_dir
	#	l["harfbuzz_lib"] = harfbuzz_lib
	#else:
	#	l["vcvars"] = "vcvars"

	execfile(filepath,g,l)

	module_list = g["module_list"]
	cmake_args = g["cmake_args"]
	additional_build_targets = g["additional_build_targets"]

	os.chdir(curDir)

# Register modules that were added using the --module argument
for module in modules:
	os.chdir(root +"/modules")
	index = module.find(':')
	if index == -1:
		add_pragma_module(
			name=module
		)
	else:
		add_pragma_module(
			name=module[0:index].strip('\"'),
			repositoryUrl=module[index +1:].strip('\"')
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
        commitSha="6f5df27d12158ed13e4f70d69828895c531accb4",
        repositoryUrl="https://github.com/Silverlan/pr_prosper_vulkan.git"
    )

if with_common_modules:
    add_pragma_module(
        name="pr_bullet",
        commitSha="4f1aea9",
        repositoryUrl="https://github.com/Silverlan/pr_bullet.git"
    )
    add_pragma_module(
        name="pr_audio_soloud",
        commitSha="17652f0",
        repositoryUrl="https://github.com/Silverlan/pr_soloud.git"
    )
    modules_prebuilt.append("Silverlan/pr_mount_external_prebuilt")

if with_pfm:
    if with_core_pfm_modules or with_all_pfm_modules:
        add_pragma_module(
            name="pr_curl",
            commitSha="ba0e2e7",
            repositoryUrl="https://github.com/Silverlan/pr_curl.git"
        )
        add_pragma_module(
            name="pr_dmx",
            commitSha="ce90ad2",
            repositoryUrl="https://github.com/Silverlan/pr_dmx.git"
        )
    if with_all_pfm_modules:
        add_pragma_module(
            name="pr_chromium",
            commitSha="15c88b4",
            repositoryUrl="https://github.com/Silverlan/pr_chromium.git"
        )
        add_pragma_module(
            name="pr_unirender",
            commitSha="0e09d47be6249d78f58f28688e338ad9dd377621",
            repositoryUrl="https://github.com/Silverlan/pr_cycles.git"
        )
        add_pragma_module(
            name="pr_curl",
            commitSha="ba0e2e7",
            repositoryUrl="https://github.com/Silverlan/pr_curl.git"
        )
        add_pragma_module(
            name="pr_dmx",
            commitSha="ce90ad2",
            repositoryUrl="https://github.com/Silverlan/pr_dmx.git"
        )
        add_pragma_module(
            name="pr_xatlas",
            commitSha="485eaad",
            repositoryUrl="https://github.com/Silverlan/pr_xatlas.git"
        )
        add_pragma_module(
            name="pr_davinci",
            commitSha="4752379de1752de3ec713e60920c2f7a684c9902",
            repositoryUrl="https://github.com/Silverlan/pr_davinci.git"
        )

if with_lua_doc_generator or with_pfm:
    add_pragma_module(
        name="pr_git",
        commitSha="84d7c32",
        repositoryUrl="https://github.com/Silverlan/pr_git.git"
    )

if with_vr:
    add_pragma_module(
        name="pr_openvr",
        commitSha="c9ce1901c1a523855328c2c1a2c6820559ddaaf9",
        repositoryUrl="https://github.com/Silverlan/pr_openvr.git"
    )

# These modules are shipped with the Pragma repository and will have to be excluded from the
# CMake configuration explicitly if they should be disabled.
shippedModules = ["pr_audio_dummy","pr_prosper_opengl","pr_prosper_vulkan","pr_curl"]

for module in module_info:
	global moduleName
	moduleName = module["name"]
	moduleUrl = module["repositoryUrl"]
	commitId = module["commitSha"]
	branch = module["branch"]
	print("Module Name:", moduleName)
	print("Repository URL:", moduleUrl)
	print("Commit SHA:", commitId)
	print("Branch:", branch)

	os.chdir(root +"/modules")

	moduleDir = os.getcwd() +"/" +moduleName +"/"

	print("Module Directory: " +moduleDir)

	if not moduleName in shippedModules:
		if moduleUrl:
			get_submodule(moduleName,moduleUrl,commitId,branch)

	scriptPath = moduleDir +"build_scripts/setup.py"
	if Path(scriptPath).is_file():
		print_msg("Executing module setup script...")
		execbuildscript(scriptPath)

	module_list.append(moduleName)

for module in shippedModules:
	if module != "pr_curl": # Curl is currently required
		if not module in module_list:
			cmake_args.append("-DPRAGMA_DISABLE_MODULE_" +module +"=ON")
		else:
			cmake_args.append("-DPRAGMA_DISABLE_MODULE_" +module +"=OFF")

os.chdir(install_dir)
for module in modules_prebuilt:
	print_msg("Downloading prebuilt binaries for module '" +module +"'...")
	install_prebuilt_binaries("https://github.com/" +module +"/releases/download/latest/")

cmake_args.append("-DPRAGMA_INSTALL_CUSTOM_TARGETS=" +";".join(module_list))

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
	"-DDEPENDENCY_FREETYPE_LIBRARY="+freetype_lib
]

if platform == "linux":
	cmake_args += [
		"-DDEPENDENCY_BOOST_INCLUDE=" +boost_root,
		"-DDEPENDENCY_BOOST_LIBRARY_LOCATION=" +boost_root +"/stage/lib",
		"-DDEPENDENCY_BOOST_CHRONO_LIBRARY=" +boost_root +"/stage/lib/boost_chrono.a",
		"-DDEPENDENCY_BOOST_DATE_TIME_LIBRARY=" +boost_root +"/stage/lib/boost_date_time.a",
		"-DDEPENDENCY_BOOST_REGEX_LIBRARY=" +boost_root +"/stage/lib/boost_regex.a",
		"-DDEPENDENCY_BOOST_SYSTEM_LIBRARY=" +boost_root +"/stage/lib/boost_system.a",
		"-DDEPENDENCY_BOOST_THREAD_LIBRARY=" +boost_root +"/stage/lib/boost_thread.a",
		"-DDEPENDENCY_LIBZIP_CONF_INCLUDE=" +build_dir +"/third_party_libs/libzip"
	]
else:
	cmake_args += [
		"-DDEPENDENCY_BOOST_INCLUDE=" +boost_root +"/build/_deps/boost-src",
		"-DDEPENDENCY_BOOST_LIBRARY_LOCATION=" +boost_root +"/build/lib/Release",
		"-DDEPENDENCY_BOOST_CHRONO_LIBRARY=" +boost_root +"/build/lib/Release/boost_chrono.lib",
		"-DDEPENDENCY_BOOST_DATE_TIME_LIBRARY=" +boost_root +"/build/lib/Release/boost_date_time.lib",
		"-DDEPENDENCY_BOOST_REGEX_LIBRARY=" +boost_root +"/build/lib/Release/boost_regex.lib",
		"-DDEPENDENCY_BOOST_SYSTEM_LIBRARY=" +boost_root +"/build/lib/Release/boost_system.lib",
		"-DDEPENDENCY_BOOST_THREAD_LIBRARY=" +boost_root +"/build/lib/Release/boost_thread.lib",
		"-DBOOST_ROOT=" +boost_root +"",
		"-DBOOST_LIBRARYDIR=" +boost_root +"/build/lib/Release/",
		"-DZLIB_INCLUDE_DIRS=" +build_dir +"/third_party_libs/zlib " +zlib_conf_root +"",
		"-DDEPENDENCY_LUAJIT_LIBRARY=" +lua_jit_lib +"",
		"-DDEPENDENCY_LUA_LIBRARY=" +lua_jit_lib +""
	]

if with_lua_doc_generator:
	cmake_args += ["-DCONFIG_BUILD_WITH_LAD=1"]

cmake_args += additional_cmake_args
cmake_configure(root,generator,cmake_args)

print_msg("Build files have been written to \"" +build_dir +"\".")

if platform == "win32":
	# Hack: zlib is using a configured header which is generated during the configuration step.
	# Unfortunately this header is in a different location than the regular header files, which causes
	# a lot of issues, so we'll copy it over to where we need it.
	# TODO: Implement a proper solution for this
	curDir = os.getcwd()
	os.chdir(root +"/third_party_libs/zlib")
	cp("zconf.h","../libzip/lib/")
	os.chdir(root +"/third_party_libs/libzip")
	cp(build_dir +"/third_party_libs/libzip/zipconf.h",root +"/external_libs/util_zip/include")
	os.chdir(curDir)
	#

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
		luasocket_args.append("-DLUA_LIBRARY=" +deps_dir +"/luajit_build/src/Release/luajit.lib")
	else:
		luasocket_args.append("-DLUA_LIBRARY=" +root +"/third_party_libs/luajit/src/libluajit-p.so")
	cmake_configure("..",generator,luasocket_args)
	cmake_build(build_config)
	cp(luasocket_root +"/src/socket.lua",install_dir +"/lua/modules/")
	mkdir(install_dir +"/modules/socket/")
	if platform == "win32":
		cp(luasocket_root +"/build/socket/" +build_config +"/core.dll",install_dir +"/modules/socket/")
	else:
		cp(luasocket_root +"/build/socket/core.so",install_dir +"/modules/socket/")
	os.chdir(curDir)

########## lua-debug ##########
if with_lua_debugger:
	execscript(scripts_dir +"/scripts/build_lua_debug.py")

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
if with_pfm:
	download_addon("PFM","filmmaker","https://github.com/Silverlan/pfm.git","39214f05720f2d76bc3e56e92ad79e3bffd60a4a")
	download_addon("model editor","tool_model_editor","https://github.com/Silverlan/pragma_model_editor.git","362981334d7b2f023dbcb1a2d1972fdc843b15e7")

if with_vr:
	download_addon("VR","virtual_reality","https://github.com/Silverlan/PragmaVR.git","9ce92cca7cca68f71027ed3770a62fc7a3dbaf4b")

if with_pfm:
	download_addon("PFM Living Room Demo","pfm_demo_living_room","https://github.com/Silverlan/pfm_demo_living_room.git","4cbecad4a2d6f502b6d9709178883678101f7e2c")
	download_addon("PFM Bedroom Demo","pfm_demo_bedroom","https://github.com/Silverlan/pfm_demo_bedroom.git","0fed1d5b54a25c3ded2ce906e7da80ca8dd2fb0d")
	download_addon("PFM Tutorials","pfm_tutorials","https://github.com/Silverlan/pfm_tutorials.git","a584457716ffa8be1611ae3becc10f5139f7083e")

if with_source_engine_entities:
	download_addon("HL","pragma_hl","https://github.com/Silverlan/pragma_hl.git","a70f575")
	download_addon("TF2","pragma_tf2","https://github.com/Silverlan/pragma_tf2.git","eddee1f")

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


	#HACK: For some reason hafbuzz is not named libharfbuzz.so.0. Fix that by adding a symlink.
	if platform=="linux":
		os.chdir(install_dir+"/lib")
		if not Path(os.getcwd()+"/libharfbuzz.so.0").is_symlink():
			os.symlink("libharfbuzz.so","libharfbuzz.so.0")



	print_msg("Build Successful! Pragma has been installed to \"" +normalize_path(install_dir) +"\".")
	print_msg("If you make any changes to the core source code, you can build the \"pragma-install\" target to compile the changes and re-install the binaries automatically.")
	print_msg("If you make any changes to a module, you will have to build the module target first, and then build \"pragma-install\".")
	print_msg("")

print_msg("All actions have been completed! Please make sure to re-run this script every time you pull any changes from the repository, and after adding any new modules.")

os.chdir(root)
