from scripts.shared import *

def main():
	if platform == "win32":
		deps_dir = config.deps_dir
		generator = config.generator
		chdir_mkdir(deps_dir)

		print_msg("Downloading freetype...")
		commit_sha = "9a2d6d9"
		if not Path(os.getcwd()+"/freetype").is_dir():
			git_clone("https://github.com/aseprite/freetype2.git", directory="freetype")
		freetype_root = deps_dir+"/freetype"
		os.chdir("freetype")
		subprocess.run(["git","reset","--hard",commit_sha],check=True)
		mkdir("build",cd=True)
		deps_dir_fs = deps_dir.replace("\\", "/")
		freetype_cmake_args =[
			"-DCMAKE_MODULE_PATH="+deps_dir_fs+"/zlib_prefix",
			"-DCMAKE_PREFIX_PATH="+deps_dir_fs+"/zlib_prefix"
		]
		freetype_cmake_args += [
			"-DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE",
			"-DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE",
			"-DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE"
		]
		freetype_cmake_args.append("-DCMAKE_POLICY_VERSION_MINIMUM=4.0")

		print_msg("Building freetype...")
		cmake_configure_def_toolset(freetype_root,generator,freetype_cmake_args)
		cmake_build("Release")

		lib_dir = copy_prebuilt_binaries(freetype_root +"/build/Release", "freetype")
		inc_dir = copy_prebuilt_headers(freetype_root +"/include", "freetype")

		return {
			"buildDir": freetype_root
		}

if __name__ == "__main__":
	main()
