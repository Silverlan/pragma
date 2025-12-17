from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
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

	return {
		"buildDir": cpptrace_root
	}

if __name__ == "__main__":
	main()
