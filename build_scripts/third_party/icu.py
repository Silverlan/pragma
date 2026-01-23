from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	# Download
	os.chdir(deps_dir)
	icu_root = os.getcwd() +"/icu"
	if not Path(icu_root).is_dir():
		print_msg("icu not found. Downloading...")
		mkpath(icu_root)
		os.chdir(icu_root)
		base_url = "https://github.com/unicode-org/icu/releases/download/release-78.1/"
		if platform == "win32":
			http_extract(base_url +"icu4c-78.1-Win64-MSVC2022.zip")
		else:
			http_extract(base_url +"icu4c-78.1-Ubuntu22.04-x64.tgz",format="tar.gz")

	if platform == "win32":
		copy_prebuilt_binaries(icu_root +"/lib64/", "icu")
		copy_prebuilt_binaries(icu_root +"/bin64/", "icu")
		copy_prebuilt_headers(icu_root +"/include", "icu")
	else:
		copy_prebuilt_binaries(icu_root +"/icu/usr/local/lib/", "icu")
		copy_prebuilt_headers(icu_root +"/icu/usr/local/include", "icu")

	return {
		"buildDir": icu_root
	}

if __name__ == "__main__":
	main()
