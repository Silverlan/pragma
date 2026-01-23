from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)
	
	init_global_vars()

	if platform == "win32":
		python_exec_name = "python"
	else:
		python_exec_name = "python3"

	# These are required for Lua debugging with Visual Studio Code
	# luamake
	print("Running build_luamake.py...")
	script_dir = Path(__file__).parent
	subprocess.run([python_exec_name, str(script_dir / "lua_debug/build_luamake.py"), deps_dir],check=True)

	# lua-debug
	print("Running build_luadebug.py...")
	subprocess.run([python_exec_name, str(script_dir / "lua_debug/build_luadebug.py"), deps_dir],check=True)

	# install
	print("Running install_files.py...")
	subprocess.run([python_exec_name, str(script_dir / "lua_debug/install_files.py"), deps_dir],check=True)

	return {
		"subLibs": {
			"luamake": {
				"buildDir": str(Path(deps_dir) / "luamake")
			},
			"luadebug": {
				"buildDir": str(Path(deps_dir) / "lua-debug")
			}
		}
	}

if __name__ == "__main__":
	main()
