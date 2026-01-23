from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	os.chdir(deps_dir)
	commit_sha = "49486f6" # v4.12.0
	opencv_root = deps_dir +"/opencv"
	if not check_repository_commit(opencv_root, commit_sha, "opencv"):
		if not Path(opencv_root).is_dir():
			print_msg("opencv not found. Downloading...")
			git_clone("https://github.com/opencv/opencv.git")

		os.chdir(opencv_root)
		reset_to_commit(commit_sha)

		print_msg("Build opencv")
		mkdir("build",cd=True)

		cmake_configure("..",generator)
		cmake_build("Release",["opencv_imgproc","opencv_imgcodecs"])

		lib_dir = opencv_root +"/build"
		copy_prebuilt_binaries(lib_dir +"/lib/" +build_config_tp, "opencv")
		copy_prebuilt_headers(opencv_root +"/include", "opencv")
		copy_prebuilt_headers(lib_dir, "opencv")

		opencv_modules = ["video", "stitching",  "photo",  "objdetect",  "ml",  "imgproc",  "videoio",  "imgcodecs",  "highgui",  "dnn",  "flann",  "features2d",  "calib3d",  "core"]
		for module in opencv_modules:
			copy_prebuilt_headers(opencv_root +"/modules/" +module +"/include", "opencv")

	return {
		"buildDir": opencv_root
	}

if __name__ == "__main__":
	main()
