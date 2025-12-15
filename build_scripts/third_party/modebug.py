from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	lua_ext_dir = deps_dir +"/lua_extensions"
	chdir_mkdir(lua_ext_dir)
	
	mob_debug_root = lua_ext_dir +"/MobDebug-0.80"
	if not Path(mob_debug_root).is_dir():
		print_msg("MobDebug not found. Downloading...")
		if platform == "win32":
			zipName = "0.80.zip"
			http_extract("https://github.com/pkulchenko/MobDebug/archive/refs/tags/" +zipName)
		else:
			zipName = "0.80.tar.gz"
			http_extract("https://github.com/pkulchenko/MobDebug/archive/refs/tags/" +zipName,format="tar.gz")
	res_dir = get_library_root_dir("lua_debugger") +"resources/"
	mkpath(res_dir +"lua/modules/")
	cp(lua_ext_dir +"/MobDebug-0.80/src/mobdebug.lua",res_dir +"lua/modules/")

if __name__ == "__main__":
	main()
