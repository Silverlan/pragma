import os
from pathlib import Path
import subprocess
from sys import platform
import pathlib
import argparse
import shutil
import logging
import tarfile
import urllib.request
from urllib.parse import urlparse
import zipfile
import sys

def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

parser = argparse.ArgumentParser(description='Pragma build script', allow_abbrev=False, formatter_class=argparse.ArgumentDefaultsHelpFormatter, epilog="")

# See https://stackoverflow.com/a/43357954/1879228 for boolean args
if platform == "linux":
	parser.add_argument('--c-compiler', help='The C-compiler to use.', default='clang-14')
	parser.add_argument('--cxx-compiler', help='The C++-compiler to use.', default='clang++-14')
	defaultGenerator = "Unix Makefiles"
else:
	defaultGenerator = "Visual Studio 17 2022"
parser.add_argument('--generator', help='The generator to use.', default=defaultGenerator)
if platform == "win32":
	parser.add_argument('--vcvars', help='Path to vcvars64.bat.', default="\"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"")
parser.add_argument("--with-essential-client-modules", type=str2bool, nargs='?', const=True, default=True, help="Include essential modules required to run Pragma.")
parser.add_argument("--with-common-modules", type=str2bool, nargs='?', const=True, default=True, help="Include non-essential but commonly used modules (e.g. audio and physics modules).")
parser.add_argument("--with-pfm", type=str2bool, nargs='?', const=True, default=False, help="Include the Pragma Filmmaker.")
parser.add_argument("--with-core-pfm-modules", type=str2bool, nargs='?', const=True, default=True, help="Include essential PFM modules.")
parser.add_argument("--with-all-pfm-modules", type=str2bool, nargs='?', const=True, default=False, help="Include non-essential PFM modules (e.g. chromium and cycles).")
parser.add_argument("--with-vr", type=str2bool, nargs='?', const=True, default=False, help="Include Virtual Reality support.")
parser.add_argument("--build", type=str2bool, nargs='?', const=True, default=True, help="Build Pragma after configurating and generating build files.")
parser.add_argument('--build-config', help='The build configuration to use.', default='RelWithDebInfo')
parser.add_argument('--build-directory', help='Directory to write the build files to. Can be relative or absolute.', default='build')
parser.add_argument('--deps-directory', help='Directory to write the dependency files to. Can be relative or absolute.', default='deps')
parser.add_argument('--install-directory', help='Installation directory. Can be relative (to build directory) or absolute.', default='install')
parser.add_argument('--module', help='Custom modules to install. Use this argument multiple times to use multiple modules. Usage example: --module pr_physx:\"https://github.com/Silverlan/pr_physx.git\"', action='append', default=[])
# parser.add_argument('--log-file', help='Script output will be written to this file.', default='build_log.txt')
parser.add_argument("--verbose", type=str2bool, nargs='?', const=True, default=False, help="Print additional verbose output.")
args = parser.parse_args()

#log_file = args.log_file
#if log_file != "":
#	if not os.path.isabs(log_file):
#		log_file = os.getcwd() +"/" +log_file
#
#		logging.basicConfig(filename=log_file,
#	        filemode='a',
#	        format='%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s',
#	        datefmt='%H:%M:%S',
#	        level=logging.DEBUG)
#
#		logging.info("Running Pragma Build Script")

if platform == "linux":
	c_compiler = args.c_compiler
	cxx_compiler = args.cxx_compiler
generator = args.generator
#if platform == "win32":
#	vcvars = args.vcvars
with_essential_client_modules = args.with_essential_client_modules
with_common_modules = args.with_common_modules
with_pfm = args.with_pfm
with_core_pfm_modules = args.with_core_pfm_modules
with_all_pfm_modules = args.with_all_pfm_modules
with_vr = args.with_vr
build = args.build
build_config = args.build_config
build_directory = args.build_directory
deps_directory = args.deps_directory
install_directory = args.install_directory
#log_file = args.log_file
verbose = args.verbose
modules = args.module
modules_prebuilt = []


print("Inputs:")
if platform == "linux":
	print("cxx_compiler: " +cxx_compiler)
	print("c_compiler: " +c_compiler)
print("generator: " +generator)
#if platform == "win32":
#	print("vcvars: " +vcvars)
print("with_essential_client_modules: " +str(with_essential_client_modules))
print("with_common_modules: " +str(with_common_modules))
print("with_pfm: " +str(with_pfm))
print("with_core_pfm_modules: " +str(with_core_pfm_modules))
print("with_all_pfm_modules: " +str(with_all_pfm_modules))
print("with_vr: " +str(with_vr))
print("build: " +str(build))
print("build_config: " +build_config)
print("build_directory: " +build_directory)
print("deps_directory: " +deps_directory)
print("install_directory: " +install_directory)
print("modules: " +', '.join(modules))

if platform == "linux":
	os.environ["CC"] = c_compiler
	os.environ["CXX"] = cxx_compiler

def normalize_path(path):
	normalizedPath = path
	normalizedPath = normalizedPath.replace('\\','/')
	return normalizedPath

def mkpath(path):
	pathlib.Path(path).mkdir(parents=True, exist_ok=True)

root = normalize_path(os.getcwd())
build_dir = normalize_path(build_directory)
deps_dir = normalize_path(deps_directory)
install_dir = install_directory
tools = root +"/tools"

if not os.path.isabs(build_dir):
	build_dir = os.getcwd() +"/" +build_dir

if not os.path.isabs(deps_dir):
	deps_dir = os.getcwd() +"/" +deps_dir

if not os.path.isabs(install_dir):
	install_dir = build_dir +"/" +install_dir

mkpath(build_dir)
mkpath(deps_dir)
mkpath(install_dir)
mkpath(tools)

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def print_msg(msg):
	print(bcolors.OKGREEN +msg +bcolors.ENDC)
	sys.stdout.flush()

def print_warning(msg):
	print(bcolors.FAIL +msg +bcolors.ENDC)
	sys.stdout.flush()

def git_clone(url,directory=None):
	args = ["git","clone",url,"--recurse-submodules"]
	if directory:
		args.append(directory)
	subprocess.run(args,check=True)

def cmake_configure(scriptPath,generator,additionalArgs=[]):
	args = ["cmake",scriptPath,"-G",generator]
	args += additionalArgs
	subprocess.run(args,check=True)

def cmake_build(buildConfig,targets=None):
	args = ["cmake","--build",".","--config",buildConfig]
	if targets:
		args.append("--target")
		args += targets
	subprocess.run(args,check=True)

def mkdir(dirName,cd=False):
	if not Path(dirName).is_dir():
		os.mkdir(dirName)
	if cd:
		os.chdir(dirName)

def http_download(url,fileName=None):
	if not fileName:
		a = urlparse(url)
		fileName = os.path.basename(a.path)
	urllib.request.urlretrieve(url,fileName)
	return fileName

# See https://stackoverflow.com/a/54748564
from zipfile import ZipFile, ZipInfo
class ZipFileWithPermissions(ZipFile):
    def _extract_member(self, member, targetpath, pwd):
        if not isinstance(member, ZipInfo):
            member = self.getinfo(member)

        targetpath = super()._extract_member(member, targetpath, pwd)

        attr = member.external_attr >> 16
        if attr != 0:
            os.chmod(targetpath, attr)
        return targetpath

def extract(zipName,removeZip=True,format="zip"):
	if format == "zip":
		with ZipFileWithPermissions(zipName, 'r') as zip_ref:
			zip_ref.extractall(".")
	elif format == "tar.bz2":
		tar = tarfile.open(zipName, "r:bz2")  
		tar.extractall()
		tar.close()
	elif format == "tar.gz":
		tar = tarfile.open(zipName, "r:gz")  
		tar.extractall()
		tar.close()
	if removeZip:
		os.remove(zipName)

def http_extract(url,removeZip=True,format="zip"):
	fileName = http_download(url)
	extract(fileName,removeZip,format)

def cp(src,dst):
	shutil.copy2(src,dst)

def replace_text_in_file(filepath,srcStr,dstStr):
	filedata = None
	with open(filepath, 'r') as file :
		filedata = file.read()

	if filedata:
		# Replace the target string
		filedata = filedata.replace(srcStr, dstStr)

		# Write the file out again
		with open(filepath, 'w') as file:
			file.write(filedata)

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
	cmake_configure(root +"/third_party_libs/luajit",generator)
	cmake_build("Release")

	lua_jit_lib = normalize_path(deps_dir +"/luajit_build/src/Release/luajit.lib")

########## GeometricTools ##########
os.chdir(deps_dir)
geometric_tools_root = normalize_path(os.getcwd() +"/GeometricTools")
if not Path(geometric_tools_root).is_dir():
	print_msg("GeometricTools not found. Downloading...")
	git_clone("https://github.com/davideberly/GeometricTools")
os.chdir("GeometricTools")
subprocess.run(["git","reset","--hard","bd7a27d18ac9f31641b4e1246764fe30816fae74"],check=True)
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
subprocess.run(["git","reset","--hard","7826e19"],check=True)
os.chdir("../../")

########## SPIRV-Headers ##########
print_msg("Downloading SPIRV-Headers...")
os.chdir(deps_dir)
os.chdir("SPIRV-Tools/external")
if not Path(os.getcwd() +"/SPIRV-Headers").is_dir():
	git_clone("https://github.com/KhronosGroup/SPIRV-Headers")
os.chdir("SPIRV-Headers")
subprocess.run(["git","reset","--hard","4995a2f2723c401eb0ea3e10c81298906bf1422b"],check=True)
os.chdir("../../")
os.chdir("../../")

########## Modules ##########
print_msg("Downloading modules...")
os.chdir(root +"/modules")

module_list = []
cmake_args = []
additional_build_targets = []

if with_essential_client_modules:
	modules.append( "pr_prosper_vulkan:\"https://github.com/Silverlan/pr_prosper_vulkan.git\"" )

if with_common_modules:
	modules.append( "pr_bullet:\"https://github.com/Silverlan/pr_bullet.git\"" )
	modules.append( "pr_audio_soloud:\"https://github.com/Silverlan/pr_soloud.git\"" )
	modules_prebuilt.append( "Silverlan/pr_mount_external_prebuilt" )

if with_pfm:
	if with_core_pfm_modules or with_all_pfm_modules:
		modules.append( "pr_curl:https://github.com/Silverlan/pr_curl.git\"" )
		modules.append( "pr_dmx:https://github.com/Silverlan/pr_dmx.git\"" )
	if with_all_pfm_modules:
		modules.append( "pr_chromium:https://github.com/Silverlan/pr_chromium.git" )
		modules.append( "pr_unirender:https://github.com/Silverlan/pr_cycles.git" )
		modules.append( "pr_curl:https://github.com/Silverlan/pr_curl.git" )
		modules.append( "pr_dmx:https://github.com/Silverlan/pr_dmx.git" )
		modules.append( "pr_xatlas:https://github.com/Silverlan/pr_xatlas.git" )
	
if with_vr:
	modules.append( "pr_openvr:https://github.com/Silverlan/pr_openvr.git" )

def execfile(filepath, globals=None, locals=None):
    if globals is None:
        globals = {}
    globals.update({
        "__file__": filepath,
        "__name__": "__main__",
    })
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

		"zlib_root": zlib_root,
		"zlib_lib": zlib_lib,
		"zlib_lib_dir": zlib_lib_path,
		"zlib_include_dirs": zlib_include_dirs,
		"boost_root": boost_root,
		"geometric_tools_root": geometric_tools_root,

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
		"replace_text_in_file": replace_text_in_file,
		"extract": extract,
		"execfile": execfile,
		"execbuildscript": execbuildscript
	}
	if platform == "linux":
		l["c_compiler"] = "c_compiler"
		l["cxx_compiler"] = "cxx_compiler"
	#else:
	#	l["vcvars"] = "vcvars"

	execfile(filepath,g,l)

	module_list = g["module_list"]
	cmake_args = g["cmake_args"]
	additional_build_targets = g["additional_build_targets"]

	os.chdir(curDir)

os.chdir(root)
subprocess.run(["git","submodule","update","--recursive"],check=True)

# These modules are shipped with the Pragma repository and will have to be excluded from the
# CMake configuration explicitly if they should be disabled.
shippedModules = ["pr_audio_dummy","pr_prosper_opengl","pr_prosper_vulkan","pr_curl"]

for module in modules:
	os.chdir(root +"/modules")
	global moduleName
	index = module.find(':')
	if index == -1:
		moduleName = module
		moduleUrl = ""
	else:
		moduleName = module[0:index].strip('\"')
		moduleUrl = module[index +1:].strip('\"')

	moduleDir = os.getcwd() +"/" +moduleName +"/"

	print("Module Name: " +moduleName)
	print("Module URL: " +moduleUrl)
	print("Module Directory: " +moduleDir)

	if not Path(moduleDir).is_dir():
		print_msg("Downloading module '" +moduleName +"'...")
		git_clone(moduleUrl,moduleName)
	elif not moduleName in shippedModules: # Shipped modules are already updated by the git submodule update command above
		curDir = os.getcwd()
		os.chdir(moduleDir)
		print_msg("Updating module '" +moduleName +"'...")
		subprocess.run(["git","pull"],check=True)
		os.chdir(curDir)

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
	baseUrl = "https://github.com/" +module +"/releases/download/latest/"
	if platform == "linux":
		http_extract(baseUrl +"binaries_linux64.tar.gz",format="tar.gz")
	else:
		http_extract(baseUrl +"binaries_windows64.zip")

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
    "-DCMAKE_INSTALL_PREFIX:PATH=" +install_dir +""
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

########## Addons ##########
def download_addon(name,addonName,url):
    print_msg("Downloading " +name +" addon...")
    mkdir(install_dir +"/addons",cd=True)
    if not Path(install_dir +"/addons/" +addonName).is_dir():
    	git_clone(url,addonName)
    else:
        os.chdir(install_dir +"/addons/" +addonName)
        print_msg("Updating " +name +"...")
        subprocess.run(["git","pull"],check=True)
        os.chdir("..")

curDir = os.getcwd()
if with_pfm:
	download_addon("PFM","filmmaker","https://github.com/Silverlan/pfm.git")
	download_addon("model editor","tool_model_editor","https://github.com/Silverlan/pragma_model_editor.git")

if with_vr:
	download_addon("VR","virtual_reality","https://github.com/Silverlan/PragmaVR.git")
os.chdir(curDir)

########## Build Pragma ##########
if build:
    print_msg("Building Pragma...")

    targets = ["pragma-install-full"] +module_list
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
