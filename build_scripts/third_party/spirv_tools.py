from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	print_msg("Downloading SPIRV-Tools...")
	commit_sha = "a62abcb"
	os.chdir(deps_dir)
	spirv_tools_root = os.getcwd() +"/SPIRV-Tools"
	if not check_repository_commit(spirv_tools_root, commit_sha, "SPIRV-Tools"):
		if not Path(spirv_tools_root).is_dir():
			git_clone("https://github.com/KhronosGroup/SPIRV-Tools.git")
		os.chdir("SPIRV-Tools")
		# Note: See the branches on https://github.com/KhronosGroup/SPIRV-Tools to find the correct commit for
		# the target Vulkan SDK version.
		# When updating to a newer version, the SPIRV-Headers commit below has to match
		# the one defined in https://github.com/KhronosGroup/SPIRV-Tools/blob/<SHA>/DEPS
		reset_to_commit(commit_sha)
		os.chdir(deps_dir)

if __name__ == "__main__":
	main()
