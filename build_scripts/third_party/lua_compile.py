from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	lua_compile_root = os.getcwd() +"/lua_compile"
	if not Path(lua_compile_root).is_dir():
		print_msg("lua_compile not found. Downloading...")
		mkdir("lua_compile", cd=True)
		if platform == "win32":
			http_extract("https://github.com/Silverlan/lua_compile/releases/download/latest/binaries_windows64.zip")
		else:
			http_extract("https://github.com/Silverlan/lua_compile/releases/download/latest/binaries_linux64.tar.gz",format="tar.gz")
	copy_prebuilt_directory(lua_compile_root, "lua_compile")

if __name__ == "__main__":
	main()
