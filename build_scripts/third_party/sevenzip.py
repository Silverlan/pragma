from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	if platform == "win32":
		print_msg("Building 7zip...")
		subprocess.run([vcpkg_root +"/vcpkg","install","7zip"],check=True)
		mkpath(install_dir +"/bin/")
		copy_prebuilt_binaries(deps_dir +"/vcpkg/installed/x64-windows/bin/", "7zip")

if __name__ == "__main__":
	main()
