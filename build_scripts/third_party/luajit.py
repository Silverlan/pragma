from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	root = config.pragma_root
	chdir_mkdir(deps_dir)

	os.chdir(deps_dir)
	commit_sha = "cb42653" # Branch: "v2.1"
	luajit_root = deps_dir +"/luajit"
	if not check_repository_commit(luajit_root, commit_sha, "luajit"):
		if not Path(luajit_root).is_dir():
			print_msg("luajit not found. Downloading...")
			git_clone("https://github.com/Silverlan/LuaJIT", "luajit")

		os.chdir(luajit_root)
		reset_to_commit(commit_sha)

		print_msg("Building LuaJIT...")
		lua_jit_lib_dir = luajit_root +"/src/"
		if platform == "linux":
			os.chdir(lua_jit_lib_dir)
			subprocess.run(["make","amalg","BUILDMODE=dynamic"],check=True)
			lua_jit_lib = normalize_path(lua_jit_lib_dir +"libluajit-p.so")
		else:
			#devcmd_path = determine_vsdevcmd_path(deps_dir)
			os.chdir(lua_jit_lib_dir)
			vcvars_path = determine_vsdevcmd_path(deps_dir)
			luajit_build_script = lua_jit_lib_dir +"msvcbuild.bat"
			luajit_build_script_wrapper = os.path.join(deps_dir,"luajit_build","build_luajit.bat")
			Path(os.path.join(deps_dir,"luajit_build")).mkdir(parents=True,exist_ok=True)
			print_msg("Generating luajit batch-script...")
			print("Writing '" +luajit_build_script_wrapper +"'...")
			with open(luajit_build_script_wrapper, 'w') as file:
				file.write("call \""+vcvars_path +"\" -arch=amd64 -host_arch=amd64\n") #TODO: allow arm64 to be usable by this.
				file.write("call \""+luajit_build_script +"\"\n")
			
			subprocess.check_call( [luajit_build_script_wrapper] )
			#subprocess.run([devcmd_path+" -no_logo & msvcbuild.bat"],check=True)    
			lua_jit_lib = normalize_path(lua_jit_lib_dir +"lua51.lib")
			# os.chdir(deps_dir)
			# mkdir("luajit_build")
			# os.chdir("luajit_build")
			# cmake_configure_def_toolset(luajit_root,generator,["-DBUILD_SHARED_LIBS=1"])
			# cmake_build("Release")
			
			# lua_jit_lib = normalize_path(deps_dir +"/luajit_build/src/Release/luajit.lib")
		copy_prebuilt_binaries(lua_jit_lib_dir, "luajit")
		copy_prebuilt_headers(luajit_root +"/src/", "luajit")

	return {
		"buildDir": luajit_root
	}

if __name__ == "__main__":
	main()
