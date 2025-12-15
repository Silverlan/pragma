import os
from sys import platform
from pathlib import Path
import subprocess
import shutil
import glob
import config

from scripts.shared import *

def main():
	mkpath(config.prebuilt_bin_dir)
	os.chdir(config.prebuilt_bin_dir)

	from third_party import libdecor
	libdecor.main()

	from third_party import zlib
	zlib.main()

	from third_party import libzip
	libzip.main()

	from third_party import libpng
	libpng.main()

	from third_party import icu
	icu.main()

	from third_party import boost
	boost.main()

	from third_party import luajit
	luajit.main()

	from third_party import geometrictools
	geometrictools.main()

	from third_party import opencv
	opencv.main()

	from third_party import spirv_tools
	spirv_tools.main()

	from third_party import spirv_headers
	spirv_headers.main()

	if config.with_swiftshader:
		from third_party import swiftshader
		swiftshader.main(config.build_swiftshader)

	from third_party import vcpkg
	vcpkg.main()

	if platform == "win32":
		from third_party import sevenzip
		sevenzip.main()

	from third_party import bit7z
	bit7z.main()

	from third_party import sevenzlib
	sevenzlib.main()

	from third_party import cpptrace
	cpptrace.main()

	# from third_party import compressonator
	# compressonator.main()

	from third_party import ispctc
	ispctc.main()

	if platform == "linux":
		from third_party import sdbus_cpp
		sdbus_cpp.main()
	else:
		from third_party import wintoast
		wintoast.main()

	# We use system freetype on linux
	if platform == "win32":
		from third_party import freetype
		freetype.main()

	if config.with_lua_debugger:
		from third_party import modebug
		modebug.main()

		from third_party import luasocket
		luasocket.main()

		from third_party import lua_debug
		lua_debug.main()

if __name__ == "__main__":
	main()
