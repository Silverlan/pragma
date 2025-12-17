from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	os.chdir(deps_dir)
	compressonator_root = normalize_path(os.getcwd() +"/compressonator")
	if not Path(compressonator_root).is_dir():
		print_msg("compressonator not found. Downloading...")
		git_clone("https://github.com/Silverlan/compressonator.git")
	os.chdir("compressonator")
	reset_to_commit("45a13326f5a86fcd4b655bdd90a2b9753e34d20c")

	print_msg("Fetching compressonator dependencies...")
	execfile(compressonator_root +"/build/fetch_dependencies.py")

	print_msg("Building compressonator...")
	mkdir("cmbuild",cd=True)
	cmake_configure_def_toolset("..",generator,["-DOpenCV_DIR=" +get_library_root_dir("opencv") +"/build", "-DOPTION_ENABLE_ALL_APPS=OFF", "-DOPTION_BUILD_CMP_SDK=ON", "-DOPTION_CMP_OPENCV=ON", "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"])

	compressonator_targets = ["Image_DDS", "Image_KTX", "Image_TGA", "CMP_Compressonator", "CMP_Framework", "CMP_Common", "CMP_Core"]
	if platform == "win32":
		compressonator_targets.append("Image_EXR")
	cmake_build("Release", compressonator_targets)

	return {
		"buildDir": compressonator_root
	}

if __name__ == "__main__":
	main()
