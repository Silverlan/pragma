from scripts.shared import *

def main():
	if platform == "win32":
		zlib_info = build_third_party_library("vcpkg")
		
		build_config_tp = config.build_config_tp
		deps_dir = config.deps_dir
		generator = config.generator
		chdir_mkdir(deps_dir)
	
		print_msg("Building 7zip...")
		subprocess.run([str(Path(deps_dir) / "vcpkg/vcpkg"),"install","7zip"],check=True)
		copy_prebuilt_binaries(str(Path(deps_dir) / "vcpkg/installed/x64-windows/bin/"), "7zip")

if __name__ == "__main__":
	main()
