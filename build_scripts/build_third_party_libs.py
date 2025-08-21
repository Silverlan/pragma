import os
from sys import platform
from pathlib import Path
import subprocess
import shutil
import glob
import config

from scripts.shared import *

os.chdir(deps_dir)

# TODO: Add input variable
build_config_tp = "Release"

mkpath(config.prebuilt_bin_dir)
os.chdir(config.prebuilt_bin_dir)

########## libdecor ##########
# We need the latest version of libdecor for Wayland-support with GLFW
if platform == "linux":
	os.chdir(deps_dir)
	libdecor_root = os.getcwd() +"/libdecor"
	commit_sha = "c4540b4a92d371b47e14fd10ff42efb3826c89b9"
	if not check_repository_commit(libdecor_root, commit_sha, "libdecor"): 
		if not Path(libdecor_root).is_dir():
			print_msg("libdecor not found. Downloading...")
			git_clone("https://gitlab.freedesktop.org/libdecor/libdecor.git")
			os.chdir("libdecor")
			reset_to_commit(commit_sha)

			os.chdir("../")
		os.chdir(libdecor_root)

		print_msg("Building libdecor...")
		subprocess.run(["meson", "build", "--buildtype", "release"],check=True)
		os.chdir("build")
		subprocess.run(["ninja"],check=True)

		copy_prebuilt_binaries(libdecor_root +"/build/src/", "libdecor", ["libdecor-0.so.0.200.2.p"])
		copy_prebuilt_headers(libdecor_root +"/src/", "libdecor")

########## zlib ##########
# Download
os.chdir(deps_dir)
zlib_root = os.getcwd() +"/zlib"
commit_sha = "6bc8ac0"
zlib_lib_path = zlib_root +"/build/" +build_config_tp
if not check_repository_commit(zlib_root, commit_sha, "zlib"):
	zlib_include_dirs = zlib_root +" " +zlib_lib_path
	if not Path(zlib_root).is_dir():
		print_msg("zlib not found. Downloading...")
		git_clone("https://github.com/Silverlan/zlib.git")
		os.chdir("zlib")
		reset_to_commit(commit_sha) # v1.3.1

		os.chdir("../")
	os.chdir("zlib")

	# Build
	print_msg("Building zlib...")
	mkdir("build",cd=True)
	zlib_build_dir = os.getcwd()
	cmake_configure_def_toolset("..",generator,["-DZLIB_BUILD_TESTING=OFF", "-DZLIB_BUILD_MINIZIP=OFF", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"])
	cmake_build(build_config_tp, ["zlib", "zlibstatic"])
	if platform == "win32":
		zlib_conf_root = normalize_path(os.getcwd())
	os.chdir("../..")

	copy_prebuilt_binaries(zlib_lib_path, "zlib")
	copy_prebuilt_headers(zlib_root, "zlib")
	copy_prebuilt_headers(zlib_root +"/build", "zlib")

if platform == "linux":
	zlib_lib = get_library_lib_dir("zlib") +"libz.a"
else:
	zlib_lib = get_library_lib_dir("zlib") +"zs.lib"

########## libzip ##########
ZLIB_SOURCE = normalize_path(zlib_root)
ZLIB_INCLUDE = normalize_path(zlib_root)
ZLIB_LIBPATH = normalize_path(zlib_lib_path)

# Download
os.chdir(deps_dir)
commit_sha = "6f8a0cd" # v1.11.4
libzip_root = os.getcwd() +"/libzip"
if not check_repository_commit(libzip_root, commit_sha, "libzip"): 
	if not Path(libzip_root).is_dir():
		print_msg("libzip not found. Downloading...")
		git_clone("https://github.com/nih-at/libzip.git")
		os.chdir("libzip")
		reset_to_commit(commit_sha)

		os.chdir("../")
	os.chdir("libzip")

	# Build
	print_msg("Building libzip...")
	mkdir("build",cd=True)
	cmake_configure_def_toolset("..",generator,["-DLIBZIP_DO_INSTALL=OFF", "-DENABLE_BZIP2=OFF", "-DENABLE_LZMA=OFF", "-DZLIB_INCLUDE_DIR=" +ZLIB_INCLUDE,"-DZLIB_LIBRARY=" +zlib_lib])
	cmake_build(build_config_tp)
	os.chdir(deps_dir)

	copy_prebuilt_binaries(libzip_root +"/build/lib/" +build_config_tp +"/", "libzip")
	copy_prebuilt_headers(libzip_root +"/lib/", "libzip")
	copy_prebuilt_headers(libzip_root +"/build/", "libzip")

########## libpng ##########
# Download
os.chdir(deps_dir)
commit_sha = "2b97891" # v1.6.50
libpng_root = os.getcwd() +"/libpng"
if not check_repository_commit(libpng_root, commit_sha, "libpng"):
	if not Path(libpng_root).is_dir():
		print_msg("libpng not found. Downloading...")
		git_clone("https://github.com/glennrp/libpng.git", branch = "libpng16")
		os.chdir("libpng")
		reset_to_commit(commit_sha)

		os.chdir("../")
	os.chdir("libpng")

	# Build
	print_msg("Building libpng...")
	mkdir("build",cd=True)
	cmake_configure_def_toolset("..",generator,["-DPNG_SHARED=OFF","-DCMAKE_POLICY_VERSION_MINIMUM=3.5","-DZLIB_INCLUDE_DIR=" +ZLIB_INCLUDE,"-DZLIB_LIBRARY=" +ZLIB_LIBPATH, "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"])
	cmake_build(build_config_tp, ["png_static"])
	os.chdir(deps_dir)

	copy_prebuilt_binaries(libpng_root +"/build/" +build_config_tp +"/", "libpng")
	copy_prebuilt_headers(libpng_root, "libpng")
	copy_prebuilt_headers(libpng_root +"/build/", "libpng")

########## icu ##########
# Download
os.chdir(deps_dir)
icu_root = os.getcwd() +"/icu"
if not Path(icu_root).is_dir():
	print_msg("icu not found. Downloading...")
	mkpath(icu_root)
	os.chdir(icu_root)
	base_url = "https://github.com/unicode-org/icu/releases/download/release-77-1/"
	if platform == "win32":
		http_extract(base_url +"icu4c-77_1-Win64-MSVC2022.zip")
	else:
		http_extract(base_url +"icu4c-77_1-Ubuntu22.04-x64.tgz",format="tar.gz")

if platform == "win32":
	copy_prebuilt_binaries(icu_root +"/lib64/", "icu")
	copy_prebuilt_binaries(icu_root +"/bin64/", "icu")
	copy_prebuilt_headers(icu_root +"/include", "icu")
else:
	copy_prebuilt_binaries(icu_root +"/icu/usr/local/lib/", "icu")
	copy_prebuilt_headers(icu_root +"/icu/usr/local/include", "icu")

########## boost ##########
# Download
os.chdir(deps_dir)
boost_root = os.getcwd() +"/boost-1.89.0"
if not Path(boost_root).is_dir():
	print_msg("boost not found. Downloading...")
	boost_url = "https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-cmake.tar.gz"
	http_extract(boost_url,format="tar.gz")

########## Download Official Boost Sources (Headers) ##########
os.chdir(deps_dir)
headers_extract_dir = deps_dir +"/boost-1.89.0-headers"
if not os.path.isdir(headers_extract_dir):
	# Choose URL based on platform
	if platform == "win32":
		archive_url = "https://archives.boost.io/release/1.89.0/source/boost_1_89_0.zip"
		archive_format = "zip"
	else:
		archive_url = "https://archives.boost.io/release/1.89.0/source/boost_1_89_0.tar.gz"
		archive_format = "tar.gz"

	if not Path(headers_extract_dir).is_dir():
		print_msg("Official Boost headers not found. Downloading official release for headers...")
		http_extract(archive_url, format=archive_format)
		# After extract, the archive usually unpacks to 'boost_1_89_0'
		unpack_root = deps_dir +"/boost_1_89_0"
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

	copy_prebuilt_binaries(boost_root +"/build/stage/lib/Release/", "boost")

	for path in glob.glob(boost_root +"/libs/*"):
		include_dir = os.path.join(path, 'include')
		if os.path.isdir(include_dir):
			copy_prebuilt_headers(include_dir, "boost")

	copy_prebuilt_headers(headers_extract_dir, "boost")

	boost_cmake_dir = get_library_root_dir("boost") +"cmake"
	mkdir(boost_cmake_dir,cd=True)
	copy_prebuilt_directory(boost_root +"/build/tmpinst/", dest_dir=boost_cmake_dir +"/")
	copy_prebuilt_directory(boost_root +"/build/export/", dest_dir=boost_cmake_dir +"/")
	shutil.copy2(boost_root +"/tools/cmake/config/BoostConfig.cmake", boost_cmake_dir +"/")
else:
	print_msg("Boost is already up-to-date. Skipping...")

########## LuaJIT ##########
print_msg("Building LuaJIT...")
luajit_root = root +"/third_party_libs/luajit/"
lua_jit_lib_dir = luajit_root +"src/"
if platform == "linux":
	os.chdir(root +"/third_party_libs/luajit/src")
	subprocess.run(["make","amalg","BUILDMODE=dynamic"],check=True)
	lua_jit_lib = normalize_path(lua_jit_lib_dir +"libluajit-p.so")
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
	lua_jit_lib = normalize_path(lua_jit_lib_dir +"lua51.lib")
	# os.chdir(deps_dir)
	# mkdir("luajit_build")
	# os.chdir("luajit_build")
	# cmake_configure_def_toolset(root +"/third_party_libs/luajit",generator,["-DBUILD_SHARED_LIBS=1"])
	# cmake_build("Release")
    
	# lua_jit_lib = normalize_path(deps_dir +"/luajit_build/src/Release/luajit.lib")
copy_prebuilt_binaries(lua_jit_lib_dir, "luajit")
copy_prebuilt_headers(luajit_root +"src/", "luajit")

########## GeometricTools ##########
os.chdir(deps_dir)
commit_sha = "979d94e"
geometric_tools_root = normalize_path(os.getcwd() +"/GeometricTools")
if not check_repository_commit(geometric_tools_root, commit_sha, "GeometricTools"):
	if not Path(geometric_tools_root).is_dir():
		print_msg("GeometricTools not found. Downloading...")
		git_clone("https://github.com/davideberly/GeometricTools")
	os.chdir("GeometricTools")
	reset_to_commit(commit_sha)
	os.chdir("../../")

	copy_prebuilt_headers(geometric_tools_root +"/GTE", "geometrictools")

########## OpenCV ##########
os.chdir(deps_dir)
commit_sha = "49486f6" # v4.12.0
opencv_root = deps_dir +"/opencv"
if not check_repository_commit(opencv_root, commit_sha, "opencv"):
	if not Path(opencv_root).is_dir():
		print_msg("opencv not found. Downloading...")
		git_clone("https://github.com/opencv/opencv.git")

	os.chdir(opencv_root)
	reset_to_commit(commit_sha)

	print_msg("Build opencv")
	mkdir("build",cd=True)

	cmake_configure("..",generator)
	cmake_build("Release",["opencv_imgproc","opencv_imgcodecs"])

	lib_dir = opencv_root +"/build"
	copy_prebuilt_binaries(lib_dir +"/lib/" +build_config_tp, "opencv")
	copy_prebuilt_headers(opencv_root +"/include", "opencv")
	copy_prebuilt_headers(lib_dir, "opencv")

	opencv_modules = ["video", "stitching",  "photo",  "objdetect",  "ml",  "imgproc",  "videoio",  "imgcodecs",  "highgui",  "dnn",  "flann",  "features2d",  "calib3d",  "core"]
	for module in opencv_modules:
		copy_prebuilt_headers(opencv_root +"/modules/" +module +"/include", "opencv")

########## SPIRV-Tools ##########
print_msg("Downloading SPIRV-Tools...")
commit_sha = "a62abcb"
os.chdir(deps_dir)
spirv_tools_root = os.getcwd() +"/SPIRV-Tools"
if not check_repository_commit(spirv_tools_root, commit_sha, "SPIRV-Tools"):
	if not Path(spirv_tools_root).is_dir():
		git_clone("https://github.com/KhronosGroup/SPIRV-Tools.git")
	os.chdir("SPIRV-Tools")
	# Note: See the branches on https://github.com/KhronosGroup/SPIRV-Tools to find the correct commit for
	# the target Vulkan SDK version.
	# When updating to a newer version, the SPIRV-Headers commit below has to match
	# the one defined in https://github.com/KhronosGroup/SPIRV-Tools/blob/<SHA>/DEPS
	reset_to_commit(commit_sha)
	os.chdir(deps_dir)

########## SPIRV-Headers ##########
print_msg("Downloading SPIRV-Headers...")
commit_sha = "aa6cef192b8e693916eb713e7a9ccadf06062ceb"
os.chdir(deps_dir)
os.chdir("SPIRV-Tools/external")
spirv_headers_root = os.getcwd() +"/spirv-headers"
if not check_repository_commit(spirv_headers_root, commit_sha, "SPIRV-Headers"):
	if not Path(spirv_headers_root).is_dir():
		git_clone("https://github.com/KhronosGroup/SPIRV-Headers", "spirv-headers")
	os.chdir("spirv-headers")
	reset_to_commit(commit_sha)
	os.chdir(deps_dir)

########## SwiftShader ##########
if with_swiftshader:
	os.chdir(deps_dir)
	swiftshader_root = normalize_path(os.getcwd() +"/swiftshader")
	swiftshader_modules_dir = install_dir +"/modules/swiftshader/"

	swiftshader_bin_dir = swiftshader_root +"/build/bin/"
	if build_swiftshader:
		commit_sha = "dc1d9063cd83b2f72d7db157512bf687abed7c21"
		if not check_repository_commit(swiftshader_root, commit_sha, "swiftshader"):
			if not Path(swiftshader_root).is_dir():
				print_msg("SwiftShader not found. Downloading...")
				git_clone("https://github.com/Silverlan/swiftshader.git")
			os.chdir("swiftshader")
			reset_to_commit(commit_sha) # Commit id should match release below
			
			print_msg("Building SwiftShader...")
			os.chdir("build")
			cmake_configure_def_toolset("..",generator)
			cmake_build("Release")
	else:
		if not Path(swiftshader_root).is_dir():
			mkpath(swiftshader_bin_dir)
			os.chdir(swiftshader_bin_dir)
			print_msg("Downloading prebuilt SwiftShader...")
			base_url = "https://github.com/Silverlan/swiftshader/releases/download/2025-07-31/" # Should match commit id above
			if platform == "win32":
				http_extract(base_url +"swiftshader.zip")
			else:
				http_extract(base_url +"swiftshader.tar.gz",format="tar.gz")
	copy_prebuilt_binaries(swiftshader_bin_dir, "swiftshader")

########## vcpkg ##########
os.chdir(deps_dir)
if platform == "win32":
	os.environ["VCPKG_DEFAULT_TRIPLET"] = "x64-windows"
vcpkg_root = deps_dir +"/vcpkg"
commit_sha = "dd3097e" # v2025.07.25
if not check_repository_commit(vcpkg_root, commit_sha, "vcpkg"):
	if not Path(vcpkg_root).is_dir():
		print_msg("vcpkg not found, downloading...")
		git_clone("https://github.com/Microsoft/vcpkg.git")

	os.chdir("vcpkg")
	reset_to_commit(commit_sha)
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
	copy_prebuilt_binaries(deps_dir +"/vcpkg/installed/x64-windows/bin/", "7zip")

########## bit7z ##########
os.chdir(deps_dir)
commit_sha = "0f03717"
bit7z_root = normalize_path(os.getcwd() +"/bit7z")
if not check_repository_commit(bit7z_root, commit_sha, "bit7z"):
	if not Path(bit7z_root).is_dir():
		print_msg("bit7z not found. Downloading...")
		git_clone("https://github.com/rikyoz/bit7z.git")
	os.chdir("bit7z")
	reset_to_commit(commit_sha) # v4.0.10

	print_msg("Building bit7z...")
	mkdir("build",cd=True)
	bit7z_cmake_args = ["-DBIT7Z_AUTO_FORMAT=ON"]

	bit7z_cflags = []
	if toolsetCFlags:
		bit7z_cflags = toolsetCFlags.copy()
	if platform == "linux":
		bit7z_cflags += ["-fPIC"]
	cmake_configure("..",generator,toolsetArgs,bit7z_cmake_args,bit7z_cflags)
	cmake_build("Release")
	if platform == "linux":
		bit7z_lib_name = "libbit7z.a"
	else:
		bit7z_lib_name = "bit7z.lib"

	copy_prebuilt_binaries(bit7z_root +"/lib/x64/Release/", "bit7z")
	copy_prebuilt_headers(bit7z_root +"/include/", "bit7z")

# 7z binaries (required for bit7z)
os.chdir(deps_dir)
sevenz_root = normalize_path(os.getcwd() +"/7z-lib")
if platform == "win32":
	commit_sha = "1a9ec9a"
	if not check_repository_commit(sevenz_root, commit_sha, "7z-lib"):
		if not Path(sevenz_root).is_dir():
			print_msg("7z-lib not found. Downloading...")
			git_clone("https://github.com/Silverlan/7z-lib.git")
		os.chdir("7z-lib")
		reset_to_commit(commit_sha)
		copy_prebuilt_binaries(sevenz_root +"/win-x64/", "7z")
else:
	if not Path(sevenz_root).is_dir():
		print_msg("7z-lib not found. Downloading...")
		mkdir("7z-lib",cd=True)
		http_extract("https://7-zip.org/a/7z2408-src.tar.xz",format="tar.xz")
	os.chdir(sevenz_root)
	sevenz_so_path = sevenz_root +"/CPP/7zip/Bundles/Format7zF"
	os.chdir(sevenz_so_path)
	subprocess.run(["make","-j","-f","../../cmpl_gcc.mak"],check=True)
	mkpath(install_dir +"/lib")
	copy_prebuilt_binaries(sevenz_so_path +"/b/g/", "7z")

########## cpptrace ##########
os.chdir(deps_dir)
commit_sha = "3db8da8" # v1.0.4
cpptrace_root = normalize_path(os.getcwd() +"/cpptrace")
if not check_repository_commit(cpptrace_root, commit_sha, "cpptrace"): 
	if not Path(cpptrace_root).is_dir():
		print_msg("cpptrace not found. Downloading...")
		git_clone("https://github.com/jeremy-rifkin/cpptrace.git")
	os.chdir("cpptrace")
	reset_to_commit(commit_sha)

	print_msg("Building cpptrace...")
	mkdir("build",cd=True)
	cpptrace_cmake_args = ["-DBUILD_SHARED_LIBS=ON"]
	cmake_configure_def_toolset("..",generator,cpptrace_cmake_args)
	cmake_build(build_config_tp)
	if platform == "linux":
		cpptrace_lib_name = "libcpptrace.so"
	else:
		cpptrace_lib_name = "cpptrace.lib"
	cpptrace_bin_dir = cpptrace_root +"/build/" +build_config_tp +"/"

	copy_prebuilt_binaries(cpptrace_bin_dir, "cpptrace")
	copy_prebuilt_headers(cpptrace_root +"/include/", "cpptrace")

########## compressonator ##########
#os.chdir(deps_dir)
#compressonator_root = normalize_path(os.getcwd() +"/compressonator")
#if not Path(compressonator_root).is_dir():
#	print_msg("compressonator not found. Downloading...")
#	git_clone("https://github.com/Silverlan/compressonator.git")
#os.chdir("compressonator")
#reset_to_commit("45a13326f5a86fcd4b655bdd90a2b9753e34d20c")
#
#print_msg("Fetching compressonator dependencies...")
#execfile(compressonator_root +"/build/fetch_dependencies.py")
#
#print_msg("Building compressonator...")
#mkdir("cmbuild",cd=True)
#cmake_configure_def_toolset("..",generator,["-DOpenCV_DIR=" +opencv_root +"/build", "-DOPTION_ENABLE_ALL_APPS=OFF", "-DOPTION_BUILD_CMP_SDK=ON", "-DOPTION_CMP_OPENCV=ON", "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"])
#
#compressonator_targets = ["Image_DDS", "Image_KTX", "Image_TGA", "CMP_Compressonator", "CMP_Framework", "CMP_Common", "CMP_Core"]
#if platform == "win32":
#	compressonator_targets.append("Image_EXR")
#cmake_build("Release", compressonator_targets)

# On Windows NVTT is used
if platform == "linux":
	########## ISPC ##########
	# Required for ISPCTextureCompressor
	os.chdir(deps_dir)
	ispc_root = normalize_path(os.getcwd() +"/ispc-v1.28.0-linux")
	if not Path(ispc_root).is_dir():
		print_msg("ISPC not found. Downloading...")
		http_extract("https://github.com/ispc/ispc/releases/download/v1.28.0/ispc-v1.28.0-linux.tar.gz",format="tar.gz")
	os.chdir(ispc_root)

	########## ISPCTextureCompressor ##########
	os.chdir(deps_dir)
	commit_sha = "79ddbc90334fc31edd438e68ccb0fe99b4e15aab"
	ispctc_root = normalize_path(os.getcwd() +"/ISPCTextureCompressor")
	if not check_repository_commit(ispctc_root, commit_sha, "ISPCTextureCompressor"): 
		if not Path(ispctc_root).is_dir():
			print_msg("ISPCTextureCompressor not found. Downloading...")
			git_clone("https://github.com/GameTechDev/ISPCTextureCompressor.git")
			cp(ispc_root +"/bin/ispc",ispctc_root +"/ISPC/linux/")
		os.chdir(ispctc_root)
		reset_to_commit(commit_sha)

		print_msg("Building ISPCTextureCompressor...")
		subprocess.run(["make","-f","Makefile.linux"],check=True)

		copy_prebuilt_binaries(ispctc_root +"/build", "ispctc")
		copy_prebuilt_headers(ispctc_root +"/ispc_texcomp", "ispctc")

if platform == "linux":
	########## sdbus-cpp ##########
	os.chdir(deps_dir)
	sdbus_root = normalize_path(os.getcwd() +"/sdbus-cpp")
	commit_sha = "7fbfcec455a2af6efe3910baa3089ecba48a9d6d"
	if not check_repository_commit(sdbus_root, commit_sha, "sdbus-cpp"): 
		os.chdir(deps_dir)
		if not Path(sdbus_root).is_dir():
			print_msg("sdbus-cpp not found. Downloading...")
			git_clone("https://github.com/Kistler-Group/sdbus-cpp.git")
		os.chdir(sdbus_root)
		reset_to_commit(commit_sha)

		mkdir("build",cd=True)
		sdbus_args = ["-DCMAKE_BUILD_TYPE=" +build_config_tp, "-DSDBUSCPP_BUILD_LIBSYSTEMD=OFF"]
		cmake_configure_def_toolset("..",generator,sdbus_args)
		cmake_build(build_config_tp)

		copy_prebuilt_binaries(sdbus_root +"/build/" +build_config_tp, "sdbus-cpp")
		copy_prebuilt_headers(sdbus_root +"/include", "sdbus-cpp")
else:
	########## WinToast ##########
	os.chdir(deps_dir)
	wintoast_root = normalize_path(os.getcwd() +"/WinToast")
	commit_sha = "9c14f6cc22739c7fd4d4a97e45ceccd8438acc62"
	if not check_repository_commit(wintoast_root, commit_sha, "WinToast"): 
		os.chdir(deps_dir)
		if not Path(wintoast_root).is_dir():
			print_msg("WinToast not found. Downloading...")
			git_clone("https://github.com/mohabouje/WinToast.git")
		os.chdir(wintoast_root)
		reset_to_commit(commit_sha)

		mkdir("build",cd=True)
		sdbus_args = ["-DCMAKE_BUILD_TYPE=" +build_config_tp]
		cmake_configure_def_toolset("..",generator,sdbus_args)
		cmake_build(build_config_tp)

		copy_prebuilt_binaries(wintoast_root +"/build/" +build_config_tp, "WinToast")
		copy_prebuilt_headers(wintoast_root +"/include", "WinToast")

########## freetype (built in win32, sys in linux (set in cmake)) ##########
if platform == "win32":
	print_msg("Downloading freetype...")
	commit_sha = "9a2d6d9"
	os.chdir(deps_dir)
	if not Path(os.getcwd()+"/freetype").is_dir():
		git_clone("https://github.com/aseprite/freetype2.git", directory="freetype")
	freetype_root = deps_dir+"/freetype"
	os.chdir("freetype")
	subprocess.run(["git","reset","--hard",commit_sha],check=True)
	mkdir("build",cd=True)
	deps_dir_fs = deps_dir.replace("\\", "/")
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

	lib_dir = copy_prebuilt_binaries(freetype_root +"/build/Release", "freetype")
	inc_dir = copy_prebuilt_headers(freetype_root +"/include", "freetype")

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
	res_dir = get_library_root_dir("lua_debugger") +"resources/"
	mkpath(res_dir +"lua/modules/")
	cp(lua_ext_dir +"/MobDebug-0.80/src/mobdebug.lua",res_dir +"lua/modules/")

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
	luasocket_args = ["-DLUA_INCLUDE_DIR=" +get_library_include_dir("luajit")]
	if platform == "win32":
		luasocket_args.append("-DLUA_LIBRARY=" +get_library_lib_dir("luajit") +"lua51.lib")
	else:
		luasocket_args.append("-DLUA_LIBRARY=" +get_library_lib_dir("luajit") +"libluajit-p.so")
	luasocket_args.append("-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
	cmake_configure_def_toolset("..",generator,luasocket_args)
	cmake_build(build_config)
	cp(luasocket_root +"/src/socket.lua",res_dir +"lua/modules/")
	socket_dir = res_dir +"modules/socket/"
	mkpath(socket_dir)
	if platform == "win32":
		cp(luasocket_root +"/build/socket/" +build_config +"/core.dll",socket_dir)
	else:
		cp(luasocket_root +"/build/socket/"+build_config +"/core.so",socket_dir)
	os.chdir(curDir)

########## lua-debug ##########
if with_lua_debugger:
	curDir = os.getcwd()
	os.chdir(scripts_dir)
	execscript(scripts_dir +"/scripts/build_lua_debug.py")
	os.chdir(curDir)
