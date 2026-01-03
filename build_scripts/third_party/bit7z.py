from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	toolsetArgs = config.toolsetArgs
	toolsetCFlags = config.toolsetCFlags
	chdir_mkdir(deps_dir)
	
	os.chdir(deps_dir)
	commit_sha = "730860a75d5dc37256e25cab12af8336ca5a6cc5"
	bit7z_root = normalize_path(os.getcwd() +"/bit7z")
	if not check_repository_commit(bit7z_root, commit_sha, "bit7z"):
		if not Path(bit7z_root).is_dir():
			print_msg("bit7z not found. Downloading...")
			git_clone("https://github.com/Silverlan/bit7z.git")
		os.chdir("bit7z")
		reset_to_commit(commit_sha) # v4.0.10

		print_msg("Building bit7z...")
		mkdir("build",cd=True)
		bit7z_cmake_args = ["-DBIT7Z_AUTO_FORMAT=ON"]

		bit7z_cflags = []
		if toolsetCFlags:
			bit7z_cflags = toolsetCFlags.copy()
		if platform == "linux":
			bit7z_cflags += ["-fPIC"]
		cmake_configure("..",generator,toolsetArgs,bit7z_cmake_args,bit7z_cflags)
		cmake_build("Release")
		if platform == "linux":
			bit7z_lib_name = "libbit7z.a"
		else:
			bit7z_lib_name = "bit7z.lib"

		copy_prebuilt_binaries(bit7z_root +"/lib/x64/Release/", "bit7z")
		copy_prebuilt_headers(bit7z_root +"/include/", "bit7z")

	return {
		"buildDir": bit7z_root
	}

if __name__ == "__main__":
	main()
