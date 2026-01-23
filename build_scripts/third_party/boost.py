from scripts.shared import *
import glob

def main():
	# Requires zlib
	zlib_info = build_third_party_library("zlib")

	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)

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

		boostArgs = ["-DBOOST_DISABLE_TESTS=ON","-DZLIB_INCLUDE_DIR=" +get_library_include_dir("zlib"),"-DZLIB_LIBRARY=" +get_zlib_lib_path()]
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

	return {
		"buildDir": boost_root,
		"subLibs": {
			"zlib": zlib_info
		}
	}
