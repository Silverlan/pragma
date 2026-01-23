from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	# On Windows NVTT is used, unless the clang compiler is used (which is not compatible with NVTT). In this case it falls back to ISPCTC.
	########## ISPCTC ##########
	# Required for ISPCTextureCompressor
	os.chdir(deps_dir)
	if platform == "win32":
		ispc_root = normalize_path(os.getcwd() +"/ispc-v1.28.0-windows")
		if not Path(ispc_root).is_dir():
			print_msg("ISPC not found. Downloading...")
			http_extract("https://github.com/ispc/ispc/releases/download/v1.28.0/ispc-v1.28.0-windows.zip",format="zip")
	else:
		ispc_root = normalize_path(os.getcwd() +"/ispc-v1.28.0-linux")
		if not Path(ispc_root).is_dir():
			print_msg("ISPC not found. Downloading...")
			http_extract("https://github.com/ispc/ispc/releases/download/v1.28.0/ispc-v1.28.0-linux.tar.gz",format="tar.gz")
	os.chdir(ispc_root)

	########## ISPCTextureCompressor ##########
	os.chdir(deps_dir)
	commit_sha = "3ddf95796a5bfea1789ee685fc432d5d4529acb7"
	ispctc_root = normalize_path(os.getcwd() +"/ISPCTextureCompressor")
	if not check_repository_commit(ispctc_root, commit_sha, "ISPCTextureCompressor"): 
		if not Path(ispctc_root).is_dir():
			print_msg("ISPCTextureCompressor not found. Downloading...")
			git_clone("https://github.com/Silverlan/ISPCTextureCompressor.git")
		os.chdir(ispctc_root)
		reset_to_commit(commit_sha)

		mkdir("build",cd=True)
		ispctc_args = [
			"-DCMAKE_BUILD_TYPE=" +build_config_tp,
			"-DVulkan_INCLUDE_DIR=" +get_library_include_dir("vulkan"),	
			"-DVulkan_LIBRARY=" +get_library_lib_dir("vulkan")
		]
		if platform == "win32":
			ispctc_args.append("-DISPC_EXECUTABLE:FILEPATH=" +ispc_root +"/bin/ispc.exe")
		else:
			ispctc_args.append("-DISPC_EXECUTABLE:FILEPATH=" +ispc_root +"/bin/ispc")
		cmake_configure_def_toolset("..",generator,ispctc_args)
		cmake_build(build_config_tp)

		copy_prebuilt_binaries(ispctc_root +"/build/" +build_config_tp, "ispctc")
		copy_prebuilt_headers(ispctc_root +"/ispc_texcomp", "ispctc")

	return {
		"buildDir": ispctc_root,
		"subLibs": {
			"ispc": {
				"buildDir": ispc_root
			}
		}
	}

if __name__ == "__main__":
	main()
