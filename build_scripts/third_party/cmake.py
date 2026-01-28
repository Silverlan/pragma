from scripts.shared import *

def main():
	build_tools_dir = config.build_tools_dir
	chdir_mkdir(build_tools_dir)
	
	os.chdir(build_tools_dir)
	cmake_dir = Path(build_tools_dir) / "cmake"
	if not Path(cmake_dir).is_dir():
		if platform == "win32":
			http_extract("https://github.com/Silverlan/CMake/releases/download/2025-12-23/windows_x64.tar.xz",format="tar.xz")
			mv("cmake.git-Windows-X64", cmake_dir)
		else:
			http_extract("https://github.com/Kitware/CMake/releases/download/v4.2.2/cmake-4.2.2-linux-x86_64.tar.gz",format="tar.gz")
			mv("cmake-4.2.2-linux-x86_64", cmake_dir)

if __name__ == "__main__":
	main()
