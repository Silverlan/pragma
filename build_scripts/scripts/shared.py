import os
import pathlib
import datetime
import sys
import subprocess
import shutil
import tarfile
import urllib.request
import zipfile
import multiprocessing
from pathlib import Path
from urllib.parse import urlparse
from sys import platform
import argparse

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

def cmake_configure(scriptPath,generator,additionalArgs=[]):
	args = ["cmake",scriptPath,"-G",generator]
	args += additionalArgs
	print("Running CMake configure command:", ' '.join(f'"{arg}"' for arg in args))
	subprocess.run(args,check=True)

def cmake_build(buildConfig,targets=None):
	args = ["cmake","--build",".","--config",buildConfig]
	if targets:
		args.append("--target")
		args += targets
	args.append("--parallel")
	args.append(str(multiprocessing.cpu_count()))
	print("Running CMake build command:", ' '.join(f'"{arg}"' for arg in args))
	subprocess.run(args,check=True)

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
	if removeZip:
		os.remove(zipName)

def http_extract(url,removeZip=True,format="zip"):
	fileName = http_download(url)
	extract(fileName,removeZip,format)

def install_prebuilt_binaries(baseUrl):
	if platform == "linux":
		http_extract(baseUrl +"binaries_linux64.tar.gz",format="tar.gz")
	else:
		http_extract(baseUrl +"binaries_windows64.zip")


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

		return subprocess.check_output([vswhere_path, "-property", "installationPath"], text=True).strip()
	
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
