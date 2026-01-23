from scripts.shared import *

def main():
	if platform == "win32":
		deps_dir = config.deps_dir
		chdir_mkdir(deps_dir)
		
		os.chdir(deps_dir)
		cmake_dir = get_library_root_dir("cmake")
		if not Path(cmake_dir).is_dir():
			http_extract("https://github.com/Silverlan/CMake/releases/download/2025-12-23/windows_x64.tar.xz",format="tar.xz")
			mv("cmake.git-Windows-X64", cmake_dir)

if __name__ == "__main__":
	main()
