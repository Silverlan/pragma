from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	print_msg("Downloading SPIRV-Headers...")
	commit_sha = "aa6cef192b8e693916eb713e7a9ccadf06062ceb"
	os.chdir(deps_dir)
	os.chdir("SPIRV-Tools/external")
	spirv_headers_root = os.getcwd() +"/spirv-headers"
	if not check_repository_commit(spirv_headers_root, commit_sha, "SPIRV-Headers"):
		if not Path(spirv_headers_root).is_dir():
			git_clone("https://github.com/KhronosGroup/SPIRV-Headers", "spirv-headers")
		os.chdir("spirv-headers")
		reset_to_commit(commit_sha)
		os.chdir(deps_dir)

if __name__ == "__main__":
	main()
