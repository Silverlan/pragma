from scripts.shared import *

def main():
	# Requires zlib
	zlib_info = build_third_party_library("zlib")

	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
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
		cmake_configure_def_toolset("..",generator,["-DPNG_SHARED=OFF","-DCMAKE_POLICY_VERSION_MINIMUM=3.5","-DZLIB_INCLUDE_DIR=" +get_library_include_dir("zlib"),"-DZLIB_LIBRARY=" +get_zlib_lib_path(), "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"])
		cmake_build(build_config_tp, ["png_static"])
		os.chdir(deps_dir)

		copy_prebuilt_binaries(libpng_root +"/build/" +build_config_tp +"/", "libpng")
		copy_prebuilt_headers(libpng_root, "libpng")
		copy_prebuilt_headers(libpng_root +"/build/", "libpng")

	return {
		"buildDir": libpng_root,
		"subLibs": {
			"zlib": zlib_info
		}
	}

if __name__ == "__main__":
	main()
