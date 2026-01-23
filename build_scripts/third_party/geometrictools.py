from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	os.chdir(deps_dir)
	commit_sha = "7cac1e1"
	geometric_tools_root = normalize_path(os.getcwd() +"/GeometricTools")
	if not check_repository_commit(geometric_tools_root, commit_sha, "GeometricTools"):
		if not Path(geometric_tools_root).is_dir():
			print_msg("GeometricTools not found. Downloading...")
			git_clone("https://github.com/davideberly/GeometricTools")
		os.chdir("GeometricTools")
		reset_to_commit(commit_sha)
		os.chdir("../../")

		copy_prebuilt_headers(geometric_tools_root +"/GTE", "geometrictools")

	return {
		"buildDir": geometric_tools_root
	}

if __name__ == "__main__":
	main()
