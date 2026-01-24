import os
import pathlib
import datetime
import time
import sys
import json
import subprocess
import shutil
import tarfile
import urllib.request
import zipfile
import shlex
import fnmatch
import filecmp
import importlib
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
	args = ["git", "clone", get_git_clone_url(url, config.prefer_git_https), "--recurse-submodules"]
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

def cmake_configure(scriptPath,generator,toolsetArgs=[],additionalArgs=[],cflags=[],env=None):
	args = [config.cmake_path,scriptPath,"-G",generator]
	if cflags:
		additionalArgs.append("-DCMAKE_C_FLAGS=" + " ".join(cflags))
		additionalArgs.append("-DCMAKE_CXX_FLAGS=" + " ".join(cflags))
	if toolsetArgs:
		args += toolsetArgs
	args += additionalArgs
	# print("Running CMake configure command...")

	def _quote(arg: str) -> str:
		if arg.startswith("-D") and "=" in arg:
			key, val = arg.split("=", 1)
			if " " in val:
				val = f'"{val}"'
			return f"{key}={val}"
		return f'"{arg}"' if " " in arg else arg

	cmd = shlex.join(args)
	print("Running CMake configure command:", cmd, flush=True)

	try:
		subprocess.run(args,check=True,env=env)
	except subprocess.CalledProcessError as e:
		if platform == "win32":
			cmd_line = subprocess.list2cmdline(e.cmd)
		else:
			cmd_line = shlex.join(e.cmd)
		print("Configure command failed:\n\n", cmd_line, flush=True)
		raise

def cmake_build(buildConfig,targets=None,env=None,verbose=False):
	args = [config.cmake_path,"--build",".","--config",buildConfig]
	if targets:
		args.append("--target")
		args += targets
	args.append("--parallel")
	args.append(str(multiprocessing.cpu_count()))

	if verbose:
		args.append("--")
		if config.generator in ("Ninja", "Ninja Multi-Config"):
			args.append("-v")
		elif config.generator == "Unix Makefiles":
			args.append("VERBOSE=1")
		elif is_msbuild_generator(config.generator):
			args.append("/verbosity:detailed")

	print("Running CMake build command...")
	# print("Running CMake build command:", ' '.join(f'"{arg}"' for arg in args))
	try:
		subprocess.run(args,check=True,env=env)
	except subprocess.CalledProcessError as e:
		if platform == "win32":
			cmd_line = subprocess.list2cmdline(e.cmd)
		else:
			cmd_line = shlex.join(e.cmd)
		print("Build command failed:\n\n", cmd_line)
		raise

def cmake_configure_def_toolset(scriptPath,generator,additionalArgs=[],additionalCFlags=[],env=None,additionalToolsetArgs=[]):
	toolsetArgs = (config.toolsetArgs or []) +additionalToolsetArgs
	cflags = additionalCFlags
	if config.toolsetCFlags is not None:
		cflags += config.toolsetCFlags
	cmake_configure(scriptPath,generator,toolsetArgs,additionalArgs,cflags,env)

def mkdir(dirName,cd=False):
	if not Path(dirName).is_dir():
		os.makedirs(dirName)
	if cd:
		os.chdir(dirName)

def make_reporthook(bar_len=40, min_interval_tty=0.05, min_interval_nontty=2.0, nontty_step_percent=5.0):
	is_tty = sys.stdout.isatty()
	last_time = [0.0]
	last_percent = [-1.0]

	def reporthook(block_num, block_size, total_size):
		downloaded = block_num * block_size
		now = time.time()

		if total_size > 0:
			percent = downloaded * 100.0 / total_size
			if percent > 100:
				percent = 100.0

			if is_tty:
				# interactive terminal: update inline, but not *too* frequently
				if now - last_time[0] >= min_interval_tty or percent == 100.0:
					last_time[0] = now
					filled = int(bar_len * downloaded / total_size)
					bar = '=' * filled + ' ' * (bar_len - filled)
					sys.stdout.write(
						f"\rDownloading: [{bar}] {percent:6.2f}% "
						f"({downloaded/1024/1024:6.2f}MB/{total_size/1024/1024:6.2f}MB)")
					sys.stdout.flush()
			else:
				# non-interactive (CI): print only at coarse steps or end
				if percent - last_percent[0] >= nontty_step_percent or percent == 100.0:
					last_percent[0] = percent
					print(f"Downloading: {percent:6.2f}% "
						  f"({downloaded/1024/1024:6.2f}MB/{total_size/1024/1024:6.2f}MB)")
		else:
			# unknown total_size: behave similarly but with bytes
			if is_tty:
				sys.stdout.write(f"\rDownloading: {downloaded} bytes")
				sys.stdout.flush()
			else:
				# rate-limit by time for unknown length
				if now - last_time[0] >= min_interval_nontty:
					last_time[0] = now
					print(f"Downloading: {downloaded} bytes")
	return reporthook

def http_download(url,fileName=None):
	if not fileName:
		a = urlparse(url)
		fileName = os.path.basename(a.path)
	hook = make_reporthook()
	try:
		urllib.request.urlretrieve(url, fileName, reporthook=hook)
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

def extract(zipName, removeZip=True, format="zip"):
	print_msg("Extracting " + zipName + "...")
	if format == "zip":
		with ZipFileWithPermissions(zipName, 'r') as zip_ref:
			names = zip_ref.namelist()
			zip_ref.extractall(".")
	elif format == "tar.bz2":
		import tarfile
		tar = tarfile.open(zipName, "r:bz2")
		names = tar.getnames()
		tar.extractall()
		tar.close()
	elif format == "tar.gz":
		import tarfile
		tar = tarfile.open(zipName, "r:gz")
		names = tar.getnames()
		tar.extractall()
		tar.close()
	elif format == "tar.xz":
		import tarfile
		tar = tarfile.open(zipName, "r:xz")
		names = tar.getnames()
		tar.extractall()
		tar.close()

	from pathlib import Path
	tops = {n.lstrip("./").split("/", 1)[0] for n in names if n.lstrip("./")}
	dirs = [t for t in tops if Path(t).is_dir()]

	if removeZip:
		os.remove(zipName)

	return dirs

def http_extract(url,removeZip=True,format="zip"):
	from scripts.shared import print_msg
	print_msg("Downloading " +url +"...")
	fileName = http_download(url)
	return extract(fileName,removeZip,format)

def install_prebuilt_binaries(baseUrl, fileName = None, version = None, cacheDir = None, filepaths = None, toolset = None):
	global config
	cacheFileName = "prebuilt_binary_version.json"
	if version is not None:
		curDir = os.getcwd()
		if cacheDir is None:
			cacheDir = curDir
			os.chdir(Path(cacheDir).parent)
		if not check_content_version(cacheDir, version, cacheFileName):
			update_content_version(cacheDir, version, cacheFileName)
			os.chdir(curDir)
		else:
			os.chdir(curDir)
			return
	if toolset is None:
		toolset = config.toolset
	if platform == "linux":
		if not fileName:
			fileName = "binaries-linux-x64-" +toolset +".tar.gz"
		extractedDirs = http_extract(baseUrl +fileName,format="tar.gz")
		if version is not None:
			add_filepaths_to_content_version(cacheDir, cacheFileName, extractedDirs)
	else:
		if not fileName:
			fileName = "binaries-windows-x64-" +toolset +".zip"
		extractedDirs = http_extract(baseUrl +fileName)
		if version is not None:
			add_filepaths_to_content_version(cacheDir, cacheFileName, extractedDirs)


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

def check_repository_commit(path, commitId, libName=None):
	if not os.path.isdir(path) or not os.path.isdir(os.path.join(path, '.git')):
		return False
	curDir = os.getcwd()
	os.chdir(path)
	full_sha = subprocess.check_output(
		["git", "rev-parse", "HEAD"], cwd=path
	).decode('utf-8').strip()
	short_current = full_sha[: len(commitId)]
	os.chdir(curDir)
	if short_current == commitId:
		if libName:
			print_msg(f"{libName} is already up-to-date at commit '{commitId}', skipping...")
		return True
	return False

def get_git_clone_url(idOrUrl, preferGitHttps):
    s = idOrUrl.strip()

    def normalize_repo_part(part):
        part = part.strip().rstrip('/')
        if part.endswith('.git'):
            part = part[:-4]
        return part

    # 1) SSH input e.g. git@github.com:owner/repo.git
    if s.startswith('git@'):
        try:
            after_at = s.split('@', 1)[1]
        except IndexError:
            return s
        if ':' in after_at:
            host, repo = after_at.split(':', 1)
            host = host.strip()
            repo = normalize_repo_part(repo)
            if host in ('github.com', 'gitlab.com'):
                if preferGitHttps:
                    return f"https://{host}/{repo}.git"
                else:
                    return f"git@{host}:{repo}.git"
            else:
                return s
        else:
            return s

    # 2+3) URL with scheme OR host without scheme (e.g. "github.com/owner/repo" or "https://github.com/owner/repo")
    if '://' in s or s.startswith('github.com') or s.startswith('gitlab.com'):
        # If scheme present, split it off; otherwise treat the whole string as rest
        if '://' in s:
            _, rest = s.split('://', 1)
        else:
            rest = s

        if '/' in rest:
            host = rest.split('/', 1)[0].strip()
            repo_part = rest.split('/', 1)[1]
        else:
            host = rest.strip()
            repo_part = ''

        repo = normalize_repo_part(repo_part)

        if host in ('github.com', 'gitlab.com'):
            if not repo:
                # nothing after host -> return original (can't build repo)
                return s
            if preferGitHttps:
                return f"https://{host}/{repo}.git"
            else:
                return f"git@{host}:{repo}.git"
        else:
            # other hosts unchanged
            return s

    return s


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
		full_sha = subprocess.check_output(
			["git", "rev-parse", "HEAD"], cwd=absDir
		).decode('utf-8').strip()
		short_current = full_sha[: len(commitId)]
		if short_current == commitId:
			print_msg(f"Already up-to-date at commit '{commitId}'.")
			os.chdir(curDir)
			return

		# Reset to the requested commit
		os.chdir(absDir)
		print_msg(f"Resetting to commit '{commitId}'...")
		reset_to_commit(commitId)
	else:
		subprocess.run(["git","pull"],check=True)
	subprocess.run(["git","submodule","update","--init","--recursive"],check=True)
	os.chdir(curDir)

def get_gh_submodule(directory,ghRepoId,commitId=None,branch=None):
	get_submodule(directory,"https://github.com/" +ghRepoId +".git",commitId,branch)

def get_gl_submodule(directory,ghRepoId,commitId=None,branch=None):
	get_submodule(directory,"https://gitlab.com/" +ghRepoId +".git",commitId,branch)

def chdir_mkdir(path):
	path = Path(path)
	path.mkdir(parents=True, exist_ok=True)
	os.chdir(path)

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

	def is_vs_env_active():
		return "VSCMD_VER" in os.environ or "VCINSTALLDIR" in os.environ

	def load_vs_env(deps_dir, arch="x64"):
		if is_vs_env_active():
			return
		bat_path = determine_vsdevcmd_path(deps_dir)
		if not os.path.exists(bat_path):
			raise FileNotFoundError(f"Could not find vsdevcmd/vcvarsall at: {bat_path}")

		# Construct a command that runs the bat file, then runs 'set' to dump env vars
		# We use "&&" to ensure 'set' only runs if the bat file succeeds.
		cmd = f'"{bat_path}" -arch={arch} && set'

		# Run the command and capture stdout
		print(f"Initializing VS environment from: {bat_path}...")
		result = subprocess.run(
			cmd, 
			capture_output=True, 
			text=True, 
			shell=True, 
			check=True
		)

		# Parse the output of 'set' to find environment variables
		new_env = {}
		for line in result.stdout.splitlines():
			# 'set' outputs in format KEY=VALUE
			if '=' in line:
				key, _, value = line.partition('=')
				new_env[key.upper()] = value

		# Update Python process's environment
		os.environ.update(new_env)
		print("VS Environment initialized successfully.")

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
	copy_files(["*.h", "*.hpp", "*.ipp", "*.inl"], source_dir, include_dir, exclude_terms)
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

def get_library_bin_dir(lib_name):
	return get_library_root_dir(lib_name) +"bin/"

def get_zlib_lib_path():
	if platform == "linux":
		return get_library_lib_dir("zlib") +"libz.a"
	return get_library_lib_dir("zlib") +"zs.lib"

def check_content_version(base_path: str, contents: str, filename: str) -> bool:
	target_file = Path(base_path) / filename
	if not target_file.exists():
		return False
	try:
		with target_file.open("r", encoding="utf-8") as f:
			data = json.load(f)
			if data.get("version") != contents:
				filepaths: List[str] = data.get("filepaths", [])
				for p in filepaths:
					print(f"Removing directory '{p}'...")
					shutil.rmtree(p)
				return False
			return True
		return False
	except Exception:
		return False

def update_content_version(base_path: str, commit_id: str, filename: str) -> None:
	base = Path(base_path)
	if base.exists():
		print(f"Removing directory '{base}'...")
		shutil.rmtree(base)

	print(f"Creating directory '{base}'...")
	base.mkdir(parents=True, exist_ok=True)

	target_file = base / filename
	print(f"Writing content version to '{target_file}'...")

	payload = {"version": commit_id}
	json_text = json.dumps(payload, ensure_ascii=False, indent=2)
	target_file.write_text(json_text, encoding="utf-8")

def add_filepaths_to_content_version(base_path: str, filename: str, filepaths) -> None:
	target_file = Path(base_path) / filename
	if not target_file.exists():
		raise FileNotFoundError(f"File not found: {target_file}")

	with target_file.open("r", encoding="utf-8") as f:
		data = json.load(f)

	if not isinstance(data, dict):
		raise ValueError("JSON root must be an object")

	existing = data.get("filepaths") or []
	if not isinstance(existing, list):
		raise ValueError("Existing 'filepaths' must be a list")

	existing.extend(str(p) for p in filepaths)
	data["filepaths"] = existing

	target_file.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")

def prefer_pacman():
    return shutil.which("pacman") is not None

def apply_patch(path_path: str):
	print_msg("Applying patch '{path_path}'...")
	subprocess.run(["git","apply",path_path],check=False)

def build_third_party_library(name, *args, **kwargs):
	module = importlib.import_module(f"third_party.{name}")
	return module.main(*args, **kwargs)

def run_cmake_script(fileName):
	os.chdir(config.pragma_root)
	subprocess.run(["cmake", "-DTOOLSET=" +config.toolset, "-DCMAKE_SOURCE_DIR=" +config.pragma_root, "-DPRAGMA_DEPS_DIR=" +config.prebuilt_bin_dir, "-P", fileName],check=True)
