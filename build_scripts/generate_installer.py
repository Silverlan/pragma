import argparse

from scripts.shared import *

parser = argparse.ArgumentParser(description='Pragma installer build script', allow_abbrev=False, formatter_class=argparse.ArgumentDefaultsHelpFormatter, epilog="")

parser.add_argument('--version', help='The Pragma Version.', required=True)
parser.add_argument('--deps-directory', help='Pragma deps directory.', required=True)
parser.add_argument('--install-directory', help='Pragma installation directory.', required=True)

args,unknown = parser.parse_known_args()
args = vars(args)
input_args = args

version = args["version"]
deps_directory = args["deps_directory"]
install_directory = args["install_directory"]

print("version: " +str(version))

cur_dir = os.getcwd() +"/build_scripts"
deps_dir = deps_directory

os.chdir(deps_dir)
inno_installer_root = os.getcwd() +"/InnoSetup_installer"
if not Path(inno_installer_root).is_dir():
    print_msg("InnoSetup_installer not found. Downloading...")
    git_clone("https://github.com/Silverlan/innosetup_installer.git","InnoSetup_installer")
    os.chdir("InnoSetup_installer")
    reset_to_commit("c6cca8e")
    os.chdir("../")

os.chdir(deps_dir)
inno_root = os.getcwd() +"/InnoSetup"
if not Path(inno_root).is_dir():
	print_msg("InnoSetup not found. Downloading...")
	mkpath(inno_root)
	os.chdir(inno_root)
	subprocess.run([inno_installer_root +"/innosetup-6.4.2.exe", "/VERYSILENT", "/SUPPRESSMSGBOXES", "/NORESTART", "/DIR=" +inno_root],check=True)

iscc = inno_root +"/ISCC.exe"

definitions = [
    "MyAppName=Pragma",
    "MyAppVersion=" +version,
    "MyAppPublisher=Silverlan",
    "MyAppURL=https://pragma-engine.com/",
    "MyAppExeName=pfm.exe",
    "IconLocation=" +cur_dir +"/../core/pragma/appicon.ico",
    "PostInstallInfoLocation=" +cur_dir +"/installer/post_install.rtf",
    "OutputFilename=pragma_setup",
    "SourceLocation=" +install_directory
]

output_dir = deps_dir +"/pragma_installer"
output_filename = "pragma_setup"

cmd = [
    iscc,
    cur_dir + "/installer/pragma.iss",
    "/O" + output_dir,
    "/F" + output_filename
]

cmd += ["/D" + definition for definition in definitions]

subprocess.run(cmd, check=True, shell=True)
