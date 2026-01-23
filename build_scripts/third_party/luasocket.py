from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	lua_ext_dir = deps_dir +"/lua_extensions"
	chdir_mkdir(lua_ext_dir)
	
	curDir = os.getcwd()
	os.chdir(lua_ext_dir)
	luasocket_root = lua_ext_dir +"/luasocket"
	if not Path(luasocket_root).is_dir():
		print_msg("luasocket not found. Downloading...")
		git_clone("https://github.com/LuaDist/luasocket.git")

	print_msg("Building luasocket...")
	os.chdir(luasocket_root)
	mkdir("build",cd=True)
	luasocket_args = ["-DLUA_INCLUDE_DIR=" +get_library_include_dir("luajit")]
	if platform == "win32":
		luasocket_args.append("-DLUA_LIBRARY=" +get_library_lib_dir("luajit") +"lua51.lib")
	else:
		luasocket_args.append("-DLUA_LIBRARY=" +get_library_lib_dir("luajit") +"libluajit-p.so")
	luasocket_args.append("-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
	cmake_configure_def_toolset("..",generator,luasocket_args)
	cmake_build(build_config_tp)
	res_dir = get_library_root_dir("lua_debugger") +"resources/"
	cp(luasocket_root +"/src/socket.lua",res_dir +"lua/modules/")
	socket_dir = res_dir +"modules/socket/"
	mkpath(socket_dir)
	if platform == "win32":
		cp(luasocket_root +"/build/socket/" +build_config_tp +"/core.dll",socket_dir)
	else:
		cp(luasocket_root +"/build/socket/"+build_config_tp +"/core.so",socket_dir)
	os.chdir(curDir)

if __name__ == "__main__":
	main()
