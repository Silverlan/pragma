#!/usr/bin/env python3
import os
import shutil
import subprocess
from pathlib import Path
import argparse

def str2bool(v):
	if isinstance(v, bool):
		return v
	if v.lower() in ('yes', 'true', 't', 'y', '1'):
		return True
	elif v.lower() in ('no', 'false', 'f', 'n', '0'):
		return False
	else:
		raise argparse.ArgumentTypeError('Boolean value expected.')

parser = argparse.ArgumentParser(description='Pragma AppImage Generator', allow_abbrev=False, formatter_class=argparse.ArgumentDefaultsHelpFormatter, epilog="")

parser.add_argument('--build-directory', help='Pragma build directory.', required=True)
parser.add_argument('--install-directory', help='Pragma installation directory.', required=True)
parser.add_argument('--build-tools-directory', help='Directory where additional build tools are located. Can be relative or absolute.', default='build_tools')
parser.add_argument('--deps-directory', help='Directory to write the dependency files to. Can be relative or absolute.', default='deps')
parser.add_argument("--no-strip", type=str2bool, nargs='?', const=True, default=False, help="Disable stripping binaries. This may be needed on rolling release distributions.")

args,unknown = parser.parse_known_args()
args = vars(args)
input_args = args

build_directory = args["build_directory"]
install_directory = args["install_directory"]
build_tools_directory = args["build_tools_directory"]
deps_directory = args["deps_directory"]
no_strip = args["no_strip"]

def main():
    appimage_base_path = Path(build_directory) / "appimage"
    appimage_data_path = appimage_base_path / "data"
    appimage_install_path = appimage_data_path / "usr/bin"
    os.makedirs(appimage_data_path, exist_ok=True)

    shutil.rmtree(appimage_install_path, ignore_errors=True)
    shutil.rmtree(appimage_data_path / "usr/lib", ignore_errors=True)
    print(f"Copying install files to {str(appimage_install_path)}...")
    shutil.copytree(install_directory, appimage_install_path)

    repo_path = Path.cwd().parent
    
    appimage_asset_path = Path.cwd() / "appimage"
    shutil.copy(repo_path / "tools/pfm_executable/logo/logo_dark.png", appimage_base_path / "icon.png")
    shutil.copy(repo_path / "tools/pfm_executable/pfm.desktop", appimage_base_path / "pfm.desktop")
    shutil.copy(appimage_asset_path / "run.sh", appimage_base_path / "run.sh")

    os.chdir(build_tools_directory)
    if not Path("linuxdeploy-x86_64.AppImage").exists():
        print("Fetching linuxdeploy...")
        subprocess.run(["wget", "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"], check=True)
        subprocess.run(["chmod", "+x", "linuxdeploy-x86_64.AppImage"], check=True)

    extra_execs = []

    modules_dir = Path(appimage_data_path / "usr/bin/modules")
    if modules_dir.exists():
        for file in modules_dir.rglob("*.so"):
            if file.is_file():
                extra_execs.extend(["-e", str(file)])

    # Executables
    extra_execs.extend(["-e", appimage_data_path / "usr/bin/modules/chromium/pr_chromium_subprocess"])
    extra_execs.extend(["-e", appimage_data_path / "usr/bin/modules/chromium/chrome-sandbox"])

    appdir_lib_path = str(appimage_data_path / "usr/bin/lib")

    # Add deps library paths
    deps_path = Path(deps_directory).resolve()
    deps_lib_paths = []
    if deps_path.exists():
        for lib_dir in deps_path.glob("*/lib"):
            if lib_dir.is_dir():
                deps_lib_paths.append(str(lib_dir))

    ld_paths_to_add = [appdir_lib_path] + deps_lib_paths
    ld_path_string = ":".join(ld_paths_to_add)

    existing_ld_path = os.environ.get("LD_LIBRARY_PATH", "")

    if existing_ld_path:
        os.environ["LD_LIBRARY_PATH"] = f"{ld_path_string}:{existing_ld_path}"
    else:
        os.environ["LD_LIBRARY_PATH"] = ld_path_string

    if no_strip:
        os.environ["NO_STRIP"] = "1"

    os.chdir(appimage_base_path)
    cmd = [
        str(Path(build_tools_directory) / "linuxdeploy-x86_64.AppImage"),
        "--appdir",
        "data",
        "--executable",
        "data/usr/bin/pfm",
    ]

    cmd.extend(extra_execs)

    cmd.extend(
        [
            "--desktop-file",
            str(appimage_base_path / "pfm.desktop"),
            "--icon-file",
            str(appimage_base_path / "icon.png"),
            "--custom-apprun",
            str(appimage_base_path / "run.sh"),
            "--output",
            "appimage",
        ]
    )

    print(f"Launching linuxdeploy with command {cmd}")
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"linuxdeploy failed with exit code {e.returncode}")
        exit(e.returncode)

if __name__ == "__main__":
    main()
