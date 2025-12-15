from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	os.chdir(deps_dir)
	if platform == "win32":
		os.environ["VCPKG_DEFAULT_TRIPLET"] = "x64-windows"
	vcpkg_root = deps_dir +"/vcpkg"
	commit_sha = "74e6536215718009aae747d86d84b78376bf9e09" # v2025.10.17
	if not check_repository_commit(vcpkg_root, commit_sha, "vcpkg"):
		if not Path(vcpkg_root).is_dir():
			print_msg("vcpkg not found, downloading...")
			git_clone("https://github.com/Microsoft/vcpkg.git")

		os.chdir("vcpkg")
		reset_to_commit(commit_sha)
		os.chdir("..")
		if platform == "linux":
			subprocess.run([vcpkg_root +"/bootstrap-vcpkg.sh","-disableMetrics"],check=True,shell=True)
		else:
			subprocess.run([vcpkg_root +"/bootstrap-vcpkg.bat","-disableMetrics"],check=True,shell=True)

if __name__ == "__main__":
	main()
