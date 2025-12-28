from scripts.shared import *

def main():
	if platform == "win32":
		build_config_tp = config.build_config_tp
		deps_dir = config.deps_dir
		generator = config.generator
		chdir_mkdir(deps_dir)
		
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
			build_args = ["-DCMAKE_BUILD_TYPE=" +build_config_tp]
			build_args = ["-DWINTOASTLIB_BUILD_EXAMPLES=OFF"]
			cmake_configure_def_toolset("..",generator,build_args)
			cmake_build(build_config_tp, ["WinToast"])

			copy_prebuilt_binaries(wintoast_root +"/build/" +build_config_tp, "WinToast")
			copy_prebuilt_headers(wintoast_root +"/include", "WinToast")

		return {
			"buildDir": wintoast_root
		}

if __name__ == "__main__":
	main()
