from scripts.shared import *

def main():
	deps_dir = config.deps_dir
	chdir_mkdir(deps_dir)
	
	os.chdir(deps_dir)
	clang_dir = str(Path(config.build_tools_dir) / "clang")
	if platform == "win32":
		if not Path(clang_dir).is_dir():
			http_extract("https://github.com/llvm/llvm-project/releases/download/llvmorg-21.1.8/clang+llvm-21.1.8-x86_64-pc-windows-msvc.tar.xz",format="tar.xz")
			mv("clang+llvm-21.1.8-x86_64-pc-windows-msvc", clang_dir)
	else:
		curDir = os.getcwd()
		os.chdir(deps_dir)
		# We need clang-22, which is not actually available as a release yet, so we use our own prebuilt binaries for now.
		clang20_root = os.getcwd() +"/LLVM-22.git-Linux-X64"
		clang_staging_path = clang_dir
		if not Path(clang20_root).is_dir():
			print_msg("Downloading clang-22...")
			http_extract("https://github.com/Silverlan/clang_prebuilt/releases/download/2025-12-19/linux_x64.tar.xz",format="tar.xz")
		os.chdir(curDir)

		copy_preserving_symlink(Path(clang20_root +"/bin/clang"), Path(clang_staging_path +"/bin"))
		copy_preserving_symlink(Path(clang20_root +"/bin/clang++"), Path(clang_staging_path +"/bin"))
		copy_preserving_symlink(Path(clang20_root +"/bin/clang-22"), Path(clang_staging_path +"/bin"))
		copy_preserving_symlink(Path(clang20_root +"/bin/clang-scan-deps"), Path(clang_staging_path +"/bin"))
		copy_preserving_symlink(Path(clang20_root +"/bin/llvm-ar"), Path(clang_staging_path +"/bin"))
		copy_preserving_symlink(Path(clang20_root +"/bin/llvm-ranlib"), Path(clang_staging_path +"/bin"))

		copytree(clang20_root +"/include/c++", clang_staging_path +"/include/c++")
		copytree(clang20_root +"/include/clang", clang_staging_path +"/include/clang")
		copytree(clang20_root +"/include/clang-c", clang_staging_path +"/include/clang-c")
		#copytree(clang20_root +"/include/x86_64-unknown-linux-gnu", clang_staging_path +"/include/x86_64-unknown-linux-gnu")

		copytree(clang20_root +"/lib/clang", clang_staging_path +"/lib/clang")
		#copytree(clang20_root +"/lib/x86_64-unknown-linux-gnu", clang_staging_path +"/lib/x86_64-unknown-linux-gnu")

		copytree(clang20_root +"/libexec", clang_staging_path +"/libexec")
		copytree(clang20_root +"/share", clang_staging_path +"/share")

if __name__ == "__main__":
	main()
