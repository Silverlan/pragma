import os
from sys import platform
from pathlib import Path
import subprocess
import shutil
import stat
import tempfile
import time
import uuid
import glob
import config

from scripts.shared import *

def on_rm_error(func, path, exc_info):
	# Try to make writable and try removal once more
	try:
		os.chmod(path, stat.S_IWRITE)
	except Exception:
		pass
	try:
		func(path)
	except Exception:
		raise

def safe_remove_tree(path, retries=5, delay=0.5):
	p = Path(path)
	if not p.exists():
		return True

	# .git dir is often problematic because it may still be in use by the system, so we'll try to delete it first
	git_dir = p / '.git'
	if git_dir.exists():
		try:
			shutil.rmtree(git_dir, onerror=on_rm_error)
		except Exception:
			pass

	# Move the tree to temp trash
	trash_parent = Path(tempfile.gettempdir())
	trash_name = "build-trash-" + uuid.uuid4().hex
	trash_path = trash_parent / trash_name

	try:
		os.rename(p, trash_path)
		moved = True
	except Exception:
		# rename failed, try shutil.move
		try:
			shutil.move(str(p), str(trash_path))
			moved = True
		except Exception:
			moved = False

	target = trash_path if moved else p

	# attempt deletion
	for attempt in range(1, retries + 1):
		try:
			if target.exists():
				shutil.rmtree(str(target), onerror=_on_rm_error)
			if not target.exists():
				return True
		except Exception as exc:
			# wait and retry
			time.sleep(delay * attempt)

	# last-resort fallback on Linux: use rm -rf
	if platform == "linux" and target.exists():
		try:
			subprocess.run(['rm', '-rf', str(target)], check=True)
			return not target.exists()
		except Exception:
			pass

	# Deletion failed
	return False

def cleanup_build_files(resultData):
	if resultData is not None:
		buildDir = resultData.get("buildDir")
		if buildDir:
			safe_remove_tree(buildDir)
		subLibs = resultData.get("subLibs")
		if subLibs:
			for name, info in subLibs.items():
				cleanup_build_files(info)

def build_library(name, *args, **kwargs):
	# If --clean-deps-build-files was specified, we'll skip the build if the binaries already exist in the staging
	# target location. This prevents the rebuilding of libraries we have already built previously.
	# If the flag wasn't specified, we always build. In this case the build cache should skip the build anyway unless
	# something has changed.
	if not config.clean_deps_build_files or not Path(get_library_root_dir(name)).is_dir():
		res = build_third_party_library(name, *args, **kwargs)
		os.chdir(config.deps_dir)
		if config.clean_deps_build_files:
			cleanup_build_files(res)

def main():
	mkpath(config.prebuilt_bin_dir)
	os.chdir(config.prebuilt_bin_dir)

	build_library("libdecor")
	build_library("zlib")
	build_library("libzip")
	build_library("libpng")
	build_library("icu")
	build_library("boost")
	build_library("luajit")
	build_library("geometrictools")
	build_library("opencv")
	build_library("spirv_tools")
	build_library("spirv_headers")

	if config.with_swiftshader:
		build_library("swiftshader", config.build_swiftshader)

	build_library("vcpkg")

	if platform == "win32":
		build_library("sevenzip")

	
	build_library("bit7z")
	build_library("sevenzlib")
	build_library("cpptrace")

	# build_library("compressonator")

	build_library("ispctc")

	if platform == "linux":
		build_library("sdbus_cpp")
	else:
		build_library("wintoast")

	# We use system freetype on linux
	if platform == "win32":
		build_library("freetype")

	if config.with_lua_debugger:
		build_library("modebug")
		build_library("luasocket")
		build_library("lua_debug")

if __name__ == "__main__":
	main()
