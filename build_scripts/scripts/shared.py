import os
import pathlib
import datetime
import sys
import subprocess
import shutil
import tarfile
import urllib.request
import zipfile
import shlex
import fnmatch
import filecmp
import multiprocessing
from pathlib import Path
from urllib.parse import urlparse
from sys import platform
import argparse
import config

def init_global_vars():
	if("--deps_dir" in  sys.argv):
		global deps_dir
		deps_dir = sys.argv[sys.argv.index("--deps_dir") + 1]
	if("--install_dir" in  sys.argv):
		global install_dir
		install_dir = sys.argv[sys.argv.index("--install_dir") + 1]

def str2bool(v):
	if isinstance(v, bool):
		return v
	if v.lower() in ('yes', 'true', 't', 'y', '1'):
		return True
	elif v.lower() in ('no', 'false', 'f', 'n', '0'):
		return False
	else:
		raise argparse.ArgumentTypeError('Boolean value expected.')

def normalize_path(path):
	normalizedPath = path
	normalizedPath = normalizedPath.replace('\\','/')
	return normalizedPath

class bcolors:
	HEADER = '\033[95m'
	OKBLUE = '\033[94m'
	OKCYAN = '\033[96m'
	OKGREEN = '\033[92m'
	WARNING = '\033[93m'
	FAIL = '\033[91m'
	ENDC = '\033[0m'
	BOLD = '\033[1m'
	UNDERLINE = '\033[4m'

def print_msg(msg):
	timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
	msg_with_timestamp = f"[{timestamp}] {msg}"
	print(bcolors.OKGREEN +msg_with_timestamp +bcolors.ENDC)
	sys.stdout.flush()

def print_warning(msg):
	timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
	msg_with_timestamp = f"[{timestamp}] {msg}"
	print(bcolors.FAIL +msg_with_timestamp +bcolors.ENDC)
	sys.stdout.flush()

def mkpath(path):
	pathlib.Path(path).mkdir(parents=True, exist_ok=True)

def git_clone(url,directory=None,branch=None):
	args = ["git", "clone", url, "--recurse-submodules"]
	if branch:
		args.extend(["-b", branch])
	if directory:
		args.append(directory)
	subprocess.run(args, check=True)

def git_clone_commit(name, path, url, commitSha, branch=None):
	if not Path(path).is_dir():
		parent_path = Path(path).parent
		Path(parent_path).mkdir(parents=True, exist_ok=True)
		print_msg(name +" not found, downloading...")
		os.chdir(parent_path)
		directory_name = Path(path).name
		git_clone(url,directory=directory_name,branch=branch)

	os.chdir(path)
	reset_to_commit(commitSha)
	return path

def cmake_configure(scriptPath,generator,toolsetArgs=None,additionalArgs=[],cflags=[]):
	args = ["cmake",scriptPath,"-G",generator]
	if cflags:
		additionalArgs.append("-DCMAKE_C_FLAGS=" + " ".join(cflags))
		additionalArgs.append("-DCMAKE_CXX_FLAGS=" + " ".join(cflags))
	if toolsetArgs:
		args += toolsetArgs
	args += additionalArgs
	# print("Running CMake configure command...")

	def _quote(arg: str) -> str:
		# Handle -DKEY=VALUE specially
		if arg.startswith("-D") and "=" in arg:
			key, val = arg.split("=", 1)
			if " " in val:
				val = f'"{val}"'
			return f"{key}={val}"
		# Otherwise, only quote if there's whitespace
		return f'"{arg}"' if " " in arg else arg

	cmd = shlex.join(args)
	print("Running CMake configure command:", cmd)

	try:
		subprocess.run(args,check=True)
	except subprocess.CalledProcessError as e:
		if platform == "win32":
			cmd_line = subprocess.list2cmdline(e.cmd)
		else:
			cmd_line = shlex.join(e.cmd)
		print("Configure command failed:\n\n", cmd_line)
		raise

def cmake_build(buildConfig,targets=None):
	args = ["cmake","--build",".","--config",buildConfig]
	if targets:
		args.append("--target")
		args += targets
	args.append("--parallel")
	args.append(str(multiprocessing.cpu_count()))
	print("Running CMake build command...")
	# print("Running CMake build command:", ' '.join(f'"{arg}"' for arg in args))
	try:
		subprocess.run(args,check=True)
	except subprocess.CalledProcessError as e:
		if platform == "win32":
			cmd_line = subprocess.list2cmdline(e.cmd)
		else:
			cmd_line = shlex.join(e.cmd)
		print("Build command failed:\n\n", cmd_line)
		raise

def mkdir(dirName,cd=False):
	if not Path(dirName).is_dir():
		os.makedirs(dirName)
	if cd:
		os.chdir(dirName)

def http_download(url,fileName=None):
	if not fileName:
		a = urlparse(url)
		fileName = os.path.basename(a.path)
	try:
		urllib.request.urlretrieve(url,fileName)
	except PermissionError as e:
		print_warning("Failed to download '" +url +"' as '" +fileName +"' (PermissionError) (cwd: " + os.getcwd() +"): {}".format(e))
		raise
	except urllib.error.URLError as e:
		print_warning("Failed to download '" +url +"' as '" +fileName +"' (URLError) (cwd: " + os.getcwd() +"): {}".format(e))
		raise
	return fileName

# See https://stackoverflow.com/a/54748564
from zipfile import ZipFile, ZipInfo
class ZipFileWithPermissions(ZipFile):
	def _extract_member(self, member, targetpath, pwd):
		if not isinstance(member, ZipInfo):
			member = self.getinfo(member)

		targetpath = super()._extract_member(member, targetpath, pwd)

		attr = member.external_attr >> 16
		if attr != 0:
			os.chmod(targetpath, attr)
		return targetpath

def extract(zipName,removeZip=True,format="zip"):
	if format == "zip":
		with ZipFileWithPermissions(zipName, 'r') as zip_ref:
			zip_ref.extractall(".")
	elif format == "tar.bz2":
		tar = tarfile.open(zipName, "r:bz2")  
		tar.extractall()
		tar.close()
	elif format == "tar.gz":
		tar = tarfile.open(zipName, "r:gz")  
		tar.extractall()
		tar.close()
	elif format == "tar.xz":
		tar = tarfile.open(zipName, "r:xz")
		tar.extractall()
		tar.close()
	if removeZip:
		os.remove(zipName)

def http_extract(url,removeZip=True,format="zip"):
	from scripts.shared import print_msg
	print_msg("Downloading and extracting " +url +"...")
	fileName = http_download(url)
	extract(fileName,removeZip,format)

def install_prebuilt_binaries(baseUrl, fileName = None):
	if platform == "linux":
		if not fileName:
			fileName = "binaries_linux64.tar.gz"
		http_extract(baseUrl +fileName,format="tar.gz")
	else:
		if not fileName:
			fileName = "binaries_windows64.zip"
		http_extract(baseUrl +fileName)


def cp(src,dst):
	shutil.copy2(src,dst)

def mv(src, dst):
	shutil.move(src,dst)

def cp_dir(src,dst):
	shutil.copytree(src,dst,dirs_exist_ok=True)

# https://stackoverflow.com/a/22331852/1879228
def copytree(src, dst, symlinks = False, ignore = None):
	if not os.path.exists(dst):
		os.makedirs(dst)
		shutil.copystat(src, dst)
	lst = os.listdir(src)
	if ignore:
		excl = ignore(src, lst)
		lst = [x for x in lst if x not in excl]
	for item in lst:
		s = os.path.join(src, item)
		d = os.path.join(dst, item)
		if symlinks and os.path.islink(s):
			if os.path.lexists(d):
				os.remove(d)
			os.symlink(os.readlink(s), d)
			try:
				st = os.lstat(s)
				mode = stat.S_IMODE(st.st_mode)
				os.lchmod(d, mode)
			except:
				pass # lchmod not available
		elif os.path.isdir(s):
			copytree(s, d, symlinks, ignore)
		else:
			shutil.copy2(s, d)

def replace_text_in_file(filepath,srcStr,dstStr):
	filedata = None
	with open(filepath, 'r') as file :
		filedata = file.read()

	if filedata:
		# Replace the target string
		filedata = filedata.replace(srcStr, dstStr)

		# Write the file out again
		with open(filepath, 'w') as file:
			file.write(filedata)

def execfile(filepath, globals=None, locals=None, args=None):
	if globals is None:
		globals = {}
	globals.update({
		"__file__": filepath,
		"__name__": "__main__",
	})
	if args is not None:
		sys.argv = [filepath] + args
	with open(filepath, 'rb') as file:
		exec(compile(file.read(), filepath, 'exec'), globals, locals)

def reset_to_commit(sha):
	subprocess.run(["git","fetch"],check=True)
	subprocess.run(["git","checkout",sha,"--recurse-submodules"],check=True)
	# subprocess.run(["git","submodule","init"],check=True)
	# subprocess.run(["git","update","--recursive"],check=True)

def get_submodule(directory,url,commitId=None,branch=None):
	from scripts.shared import print_msg
	from scripts.shared import git_clone
	from scripts.shared import reset_to_commit
	import os
	import subprocess
	from pathlib import Path

	print_msg("Updating submodule '" +directory +"'...")

	if os.path.isdir(directory) and not os.path.isdir(os.path.join(directory, '.git')):
		print_msg("Submodule directory already exists, but is not a git directory. Skipping update...")
		return

	curDir = os.getcwd()
	absDir = os.getcwd() +"/" +directory
	if not Path(absDir).is_dir() or not os.listdir(absDir):
		git_clone(url,directory,branch)
	if commitId is not None:
		os.chdir(absDir)
		reset_to_commit(commitId)
	else:
		subprocess.run(["git","pull"],check=True)
	subprocess.run(["git","submodule","update","--init","--recursive"],check=True)
	os.chdir(curDir)

def compile_lua_file(deps_dir, luaFile):
	from scripts.shared import normalize_path
	curDir = os.getcwd()
	os.chdir(deps_dir)
	lua_compile_root = normalize_path(deps_dir +"/lua_compile")
	if not Path(lua_compile_root).is_dir():
		mkdir("lua_compile",True)
		print_msg("lua_compile not found. Downloading...")
		if platform == "win32":
			http_extract("https://github.com/Silverlan/lua_compile/releases/download/latest/binaries_windows64.zip")
		else:
			http_extract("https://github.com/Silverlan/lua_compile/releases/download/latest/binaries_linux64.tar.gz",format="tar.gz")
	os.chdir(lua_compile_root)
	if platform == "win32":
		subprocess.run([os.getcwd() +"/lua_compile.exe",luaFile],check=True)
	else:
		subprocess.run([os.getcwd() +"/lua_compile",luaFile],check=True)
	os.chdir(curDir)

if platform == "win32":
	def determine_vs_installation_path(deps_dir):
		# Create the deps_dir if it doesn't exist
		if not os.path.exists(deps_dir):
			os.makedirs(deps_dir)

		vswhere_url = "https://github.com/microsoft/vswhere/releases/download/3.1.7/vswhere.exe"

		# Download vswhere.exe to deps_dir
		vswhere_path = os.path.join(deps_dir, "vswhere.exe")
		if not os.path.exists(vswhere_path):
			urllib.request.urlretrieve(vswhere_url, vswhere_path)

		return subprocess.check_output([vswhere_path, "-latest", "-products", "*", "-requires", "Microsoft.Component.MSBuild", "-property", "installationPath"], text=True).strip()
	
	def determine_vsdevcmd_path(deps_dir):
		installation_path = determine_vs_installation_path(deps_dir)
		vsdevcmd_path = os.path.join(installation_path, "Common7", "Tools", "vsdevcmd.bat")
		return vsdevcmd_path

if platform == "linux":
	def install_system_packages(packages, no_confirm):
		print("")
		print_msg("The following system packages will be installed:")
		for cmd in packages:
			print(cmd)

		if not no_confirm:
			user_input = input("Your password may be required to install them. Do you want to continue (Y/n)?")
			if user_input.lower() == 'yes' or user_input.lower() == 'y':
				pass
			elif user_input.lower() == 'no' or user_input.lower() == 'n':
				sys.exit(0)
			else:
				print("Invalid input, please type 'y' for yes or 'n' for no. Aborting...")
				sys.exit(0)

		print_msg("Installing system packages...")
		for cmd in packages:
			print_msg("Running " +cmd +"...")
			subprocess.run(["sudo"] +cmd.split() +["-y"],check=True)

def copy_preserving_symlink(src: Path, dst_dir: Path):
    dst_dir.mkdir(parents=True, exist_ok=True)
    dst = dst_dir / src.name

    if src.is_symlink():
        link_target = os.readlink(src)
        target_path = (src.parent / link_target).resolve()

        target_dst = dst_dir / target_path.name

        if target_dst.exists() or target_dst.is_symlink():
            target_dst.unlink()

        shutil.copy2(target_path, target_dst)

        if dst.exists() or dst.is_symlink():
            dst.unlink()

        os.symlink(target_dst.name, dst)

    else:
        # regular file: if dst exists, unlink first
        if dst.exists() or dst.is_symlink():
            dst.unlink()
        shutil.copy2(src, dst)

def copy_files(include_patterns, source_dir, dest_dir, exclude_terms=None):
    if exclude_terms is None:
        exclude_terms = []

    source_dir = os.path.abspath(source_dir)
    dest_dir = os.path.abspath(dest_dir)
    os.makedirs(dest_dir, exist_ok=True)

    def is_excluded(name):
        return any(term in name for term in exclude_terms)

    entries = os.listdir(source_dir)
    copied_trees = set()
    for name in entries:
        if is_excluded(name):
            continue
        src_path = os.path.join(source_dir, name)
        if not os.path.isdir(src_path) or os.path.islink(src_path):
            continue
        if any(fnmatch.fnmatch(name, pat) for pat in include_patterns):
            dst_tree = os.path.join(dest_dir, name)
            for root, dirs, files in os.walk(src_path, followlinks=False):
                rel_root = os.path.relpath(root, source_dir)
                dst_root = os.path.join(dest_dir, rel_root)
                dirs[:] = [d for d in dirs if not is_excluded(d)]
                os.makedirs(dst_root, exist_ok=True)
                for fname in files:
                    if is_excluded(fname):
                        continue
                    src_file = os.path.join(root, fname)
                    rel_file = os.path.join(rel_root, fname)
                    _copy_entry(src_file, os.path.join(dest_dir, rel_file), source_dir, dest_dir)
            copied_trees.add(src_path)

    for root, dirs, files in os.walk(source_dir, followlinks=False):
        if any(root.startswith(tree) for tree in copied_trees):
            dirs[:] = []
            continue
        dirs[:] = [d for d in dirs if not is_excluded(d)]
        rel_root = os.path.relpath(root, source_dir)
        for fname in files:
            if is_excluded(fname):
                continue
            if any(fnmatch.fnmatch(fname, pat) for pat in include_patterns):
                src_file = os.path.join(root, fname)
                dst_file = os.path.join(dest_dir, rel_root, fname)
                _copy_entry(src_file, dst_file, source_dir, dest_dir)


def _copy_entry(src_path, dst_path, source_dir, dest_dir):
    os.makedirs(os.path.dirname(dst_path), exist_ok=True)
    if os.path.islink(src_path):
        link_target = os.readlink(src_path)
        if not os.path.isabs(link_target):
            abs_target = os.path.abspath(os.path.join(os.path.dirname(src_path), link_target))
        else:
            abs_target = link_target
        if abs_target.startswith(source_dir):
            rel_target = os.path.relpath(abs_target, source_dir)
            new_abs = os.path.join(dest_dir, rel_target)
            rel_link = os.path.relpath(new_abs, os.path.dirname(dst_path))
        else:
            rel_link = link_target
        if os.path.lexists(dst_path):
            os.remove(dst_path)
        os.symlink(rel_link, dst_path)
    else:
        shutil.copy2(src_path, dst_path)

def move_dll_files(src: str, dst: str) -> None:
    src = os.path.abspath(src)
    dst = os.path.abspath(dst)

    for root, dirs, files in os.walk(src):
        rel_path = os.path.relpath(root, src)
        for file in files:
            if file.lower().endswith('.dll'):
                src_file_path = os.path.join(root, file)
                dest_dir_path = os.path.join(dst, rel_path)
                os.makedirs(dest_dir_path, exist_ok=True)
                dst_file_path = os.path.join(dest_dir_path, file)
                print(f"Moving: {src_file_path} -> {dst_file_path}")
                shutil.move(src_file_path, dst_file_path)

def copy_prebuilt_binaries(source_dir, lib_name, exclude_terms=None):
	global config
	if platform == "win32":
		include_patterns = ["*.lib", "*.dll"]
	else:
		include_patterns = ["*.a", "*.so*"]
	lib_dir = config.prebuilt_bin_dir +"/" +lib_name +"/lib/"
	dst_dir = config.prebuilt_bin_dir +"/" +lib_name +"/lib/"
	copy_files(include_patterns, source_dir, dst_dir, exclude_terms)
	move_dll_files(dst_dir, config.prebuilt_bin_dir +"/" +lib_name +"/bin/")
	return lib_dir

def copy_prebuilt_headers(source_dir, lib_name, exclude_terms=None):
	global config
	include_dir = config.prebuilt_bin_dir +"/" +lib_name +"/include/"
	copy_files(["*.h", "*.hpp", "*.ipp"], source_dir, include_dir, exclude_terms)
	return include_dir

def sync_dirs(src: str, dst: str) -> None:
	"""
	Sync contents of src/ into dst/, mimicking `rsync -a --links`.
	"""
	src = src.rstrip(os.sep)
	dst = dst.rstrip(os.sep)

	# Ensure destination exists
	os.makedirs(dst, exist_ok=True)

	# Walk the source tree
	for root, dirs, files in os.walk(src):
		# Compute relative path & corresponding destination root
		rel = os.path.relpath(root, src)
		dest_root = os.path.join(dst, rel)

		# 1) Create any subdirs in dst
		for d in dirs:
			src_d = os.path.join(root, d)
			dst_d = os.path.join(dest_root, d)
			if os.path.islink(src_d):
				# Recreate symlink
				if os.path.lexists(dst_d):
					os.remove(dst_d)
				target = os.readlink(src_d)
				os.symlink(target, dst_d)
				# Tell os.walk not to descend into the link
				dirs.remove(d)
			else:
				os.makedirs(dst_d, exist_ok=True)
				# Copy over directory metadata
				shutil.copystat(src_d, dst_d, follow_symlinks=False)

		# 2) Copy/update files
		for f in files:
			src_f = os.path.join(root, f)
			dst_f = os.path.join(dest_root, f)

			if os.path.islink(src_f):
				# Handle symlink
				if os.path.lexists(dst_f):
					os.remove(dst_f)
				target = os.readlink(src_f)
				os.symlink(target, dst_f)
			else:
				# Only copy if missing or contents differ
				if not os.path.exists(dst_f) or not filecmp.cmp(src_f, dst_f, shallow=False):
					shutil.copy2(src_f, dst_f)
	# TODO: Delete files in dst that don't exist in src

def copy_prebuilt_directory(source_dir, lib_name=None, exclude_terms=None, dest_dir=None):
	if dest_dir == None:
		dest_dir = get_library_root_dir(lib_name)
	# dirs_exist_ok is only available in python 3.8, so we'll use
	# sync_dirs for now instead
	# shutil.copytree(source_dir, dest_dir, dirs_exist_ok=True, symlinks=True)
	mkpath(dest_dir)
	sync_dirs(f"{source_dir.rstrip(os.sep)}/", f"{dest_dir.rstrip(os.sep)}/")

def get_staging_dir():
	global config
	return config.deps_dir +"/" +config.deps_staging_dir +"/"

def get_library_root_dir(lib_name):
	return get_staging_dir() +lib_name +"/"

def get_library_include_dir(lib_name):
	return get_library_root_dir(lib_name) +"include/"

def get_library_lib_dir(lib_name):
	return get_library_root_dir(lib_name) +"lib/"
