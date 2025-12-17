from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
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

	return {
		"buildDir": zlib_root
	}

if __name__ == "__main__":
	main()
