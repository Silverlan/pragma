from scripts.shared import *

def main(build_swiftshader=False):
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	os.chdir(deps_dir)
	swiftshader_root = normalize_path(os.getcwd() +"/swiftshader")

	swiftshader_bin_dir = swiftshader_root +"/build/bin/"
	if build_swiftshader:
		commit_sha = "dc1d9063cd83b2f72d7db157512bf687abed7c21"
		if not check_repository_commit(swiftshader_root, commit_sha, "swiftshader"):
			if not Path(swiftshader_root).is_dir():
				print_msg("SwiftShader not found. Downloading...")
				git_clone("https://github.com/Silverlan/swiftshader.git")
			os.chdir("swiftshader")
			reset_to_commit(commit_sha) # Commit id should match release below
			
			print_msg("Building SwiftShader...")
			os.chdir("build")
			cmake_configure_def_toolset("..",generator)
			cmake_build("Release")
	else:
		if not Path(swiftshader_root).is_dir():
			mkpath(swiftshader_bin_dir)
			os.chdir(swiftshader_bin_dir)
			print_msg("Downloading prebuilt SwiftShader...")
			base_url = "https://github.com/Silverlan/swiftshader/releases/download/2025-07-31/" # Should match commit id above
			if platform == "win32":
				http_extract(base_url +"swiftshader.zip")
			else:
				http_extract(base_url +"swiftshader.tar.gz",format="tar.gz")
	copy_prebuilt_binaries(swiftshader_bin_dir, "swiftshader")

	return {
		"buildDir": swiftshader_root
	}

if __name__ == "__main__":
	main()
