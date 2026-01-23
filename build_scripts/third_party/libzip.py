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
		cmake_configure_def_toolset("..",generator,["-DLIBZIP_DO_INSTALL=OFF", "-DENABLE_BZIP2=OFF", "-DENABLE_LZMA=OFF", "-DZLIB_INCLUDE_DIR=" +get_library_include_dir("zlib"),"-DZLIB_LIBRARY=" +get_zlib_lib_path()])
		cmake_build(build_config_tp)
		os.chdir(deps_dir)

		copy_prebuilt_binaries(libzip_root +"/build/lib/" +build_config_tp +"/", "libzip")
		copy_prebuilt_headers(libzip_root +"/lib/", "libzip")
		copy_prebuilt_headers(libzip_root +"/build/", "libzip")

	return {
		"buildDir": libzip_root,
		"subLibs": {
			"zlib": zlib_info
		}
	}

if __name__ == "__main__":
	main()
