from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	os.chdir(deps_dir)
	sevenz_root = normalize_path(os.getcwd() +"/7z-lib")
	if platform == "win32":
		commit_sha = "1a9ec9a"
		if not check_repository_commit(sevenz_root, commit_sha, "7z-lib"):
			if not Path(sevenz_root).is_dir():
				print_msg("7z-lib not found. Downloading...")
				git_clone("https://github.com/Silverlan/7z-lib.git")
			os.chdir("7z-lib")
			reset_to_commit(commit_sha)
			copy_prebuilt_binaries(sevenz_root +"/win-x64/", "7z")
	else:
		if not Path(sevenz_root).is_dir():
			print_msg("7z-lib not found. Downloading...")
			mkdir("7z-lib",cd=True)
			http_extract("https://7-zip.org/a/7z2501-src.tar.xz",format="tar.xz")
		os.chdir(sevenz_root)
		sevenz_so_path = sevenz_root +"/CPP/7zip/Bundles/Format7zF"
		os.chdir(sevenz_so_path)
		subprocess.run(["make","-j","-f","../../cmpl_gcc.mak"],check=True)
		copy_prebuilt_binaries(sevenz_so_path +"/b/g/", "7z")

	return {
		"buildDir": sevenz_root
	}

if __name__ == "__main__":
	main()
