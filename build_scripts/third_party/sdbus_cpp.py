from scripts.shared import *

def main():
	if platform == "linux":
		build_config_tp = config.build_config_tp
		deps_dir = config.deps_dir
		generator = config.generator
		chdir_mkdir(deps_dir)
		
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

		return {
			"buildDir": sdbus_root
		}

if __name__ == "__main__":
	main()
