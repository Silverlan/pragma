import os
from sys import platform
from pathlib import Path
import subprocess
import shutil
import glob
import config

from scripts.shared import *

def cleanup_build_files(resultData):
	if resultData is not None:
		buildDir = resultData.get("buildDir")
		if buildDir:
			shutil.rmtree(buildDir, ignore_errors=True)
		subLibs = resultData.get("subLibs")
		if subLibs:
			for name, info in subLibs.items():
				cleanup_build_files(info)

def build_library(name, *args, **kwargs):
	res = build_third_party_library(name, *args, **kwargs)
	if config.clean_deps_build_files:
		cleanup_build_files(res)

def main():
	mkpath(config.prebuilt_bin_dir)
	os.chdir(config.prebuilt_bin_dir)

	build_library("libdecor")
	build_library("zlib")
	build_library("libzip")
	build_library("libpng")
	build_library("icu")
	build_library("boost")
	build_library("luajit")
	build_library("geometrictools")
	build_library("opencv")
	build_library("spirv_tools")
	build_library("spirv_headers")

	if config.with_swiftshader:
		build_library("swiftshader", config.build_swiftshader)

	build_library("vcpkg")

	if platform == "win32":
		build_library("sevenzip")

	
	build_library("bit7z")
	build_library("sevenzlib")
	build_library("cpptrace")

	# build_library("compressonator")

	build_library("ispctc")

	if platform == "linux":
		build_library("sdbus_cpp")
	else:
		build_library("wintoast")

	# We use system freetype on linux
	if platform == "win32":
		build_library("freetype")

	if config.with_lua_debugger:
		build_library("modebug")
		build_library("luasocket")
		build_library("lua_debug")

if __name__ == "__main__":
	main()
