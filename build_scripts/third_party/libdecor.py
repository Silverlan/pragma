from scripts.shared import *

def main():
	deps_dir = config.deps_dir
	chdir_mkdir(deps_dir)

	# We need the latest version of libdecor for Wayland-support with GLFW
	if platform == "linux":
		os.chdir(deps_dir)
		libdecor_root = os.getcwd() +"/libdecor"
		commit_sha = "700aa1e6fb5dddc085d94dddf480e74b84c0ad88"
		if not check_repository_commit(libdecor_root, commit_sha, "libdecor"): 
			if not Path(libdecor_root).is_dir():
				print_msg("libdecor not found. Downloading...")
				git_clone("https://gitlab.freedesktop.org/libdecor/libdecor.git")
				os.chdir("libdecor")
				reset_to_commit(commit_sha)

				os.chdir("../")
			os.chdir(libdecor_root)

			print_msg("Building libdecor...")
			subprocess.run(["meson", "build", "--buildtype", "release"],check=True)
			os.chdir("build")
			subprocess.run(["ninja"],check=True)

			copy_prebuilt_binaries(libdecor_root +"/build/src/", "libdecor", ["libdecor-0.so.0.200.2.p"])
			copy_prebuilt_headers(libdecor_root +"/src/", "libdecor")

		return {
			"buildDir": libdecor_root
		}

if __name__ == "__main__":
	main()
