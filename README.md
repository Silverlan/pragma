[![Build Windows](https://github.com/Silverlan/pragma/actions/workflows/pragma-windows-ci.yml/badge.svg?branch=main)](https://github.com/Silverlan/pragma/actions/workflows/pragma-windows-ci.yml) [![Build Linux](https://github.com/Silverlan/pragma/actions/workflows/pragma-linux-ci.yml/badge.svg?branch=main)](https://github.com/Silverlan/pragma/actions/workflows/pragma-linux-ci.yml) [![License](https://img.shields.io/github/license/Silverlan/pragma)](#license) [![CodeFactor](https://img.shields.io/codefactor/grade/github/Silverlan/pragma)](https://www.codefactor.io/repository/github/Silverlan/pragma)

<img align="right" width="150" height="150" alt="Pragma Logo" title="Pragma Logo" src="https://wiki.pragma-engine.com/uploads/images/gallery/2022-09/scaled-1680-/pragma-logo-256.png">

What is this?
------
This is the repository for the Pragma Game Engine. For more information, visit the official website: https://pragma-engine.com/


Download
------
All versions include the core Engine, as well as the Pragma Filmmaker (PFM).
###### Stable
[![Download](https://wiki.pragma-engine.com/uploads/images/gallery/2020-08/firefox_2020-08-14_10-46-44.png)](https://github.com/Silverlan/pragma/releases/latest)

You can find older releases in the [releases section](https://github.com/Silverlan/pragma/releases) of this repository. No installation is required, simply extract the archive somewhere and launch the `pragma` or `pfm` executable.

Supported languages: English, Deutsch, Français, Español, Italiano, Polski, 日本語, 中文 (Zhōngwén), 

###### Nightly
In addition, there is also an automated nightly release with all of the latest features which you can find [here](https://github.com/Silverlan/pragma/releases/tag/nightly), but it is not guaranteed to be stable or functional.

###### Updating
PFM automatically checks for updates when you launch it, and can automatically install them as well, so you don't have to download new releases manually.

Media
------
<p float="left">
  <img src="https://wiki.pragma-engine.com/uploads/images/gallery/2022-11/scaled-1680-/pragma-2022-11-21-16-31-04.png" width="380" />
  <img src="https://wiki.pragma-engine.com/uploads/images/gallery/2022-11/scaled-1680-/pragma-2022-11-21-16-35-19.png" width="444" /> 
</p>

What platforms and hardware does it work on?
------
- Windows 10 (or newer) / Ubuntu 24.04 (or newer)
- Graphics card: GTX 1050 Ti or newer

Contributions
------
###### PFM
If you would like to contribute to the development of the Pragma Filmmaker, please go to the [PFM repository](https://github.com/Silverlan/pfm#contributions) for more information.

###### Pragma
The recommended way to work with Pragma is through the [Lua API](https://wiki.pragma-engine.com/books/lua-api). If this is not enough, and you need more control, you will have to build Pragma manually. You can find the build instructions below.

Please consider creating a [binary module](https://github.com/Silverlan/pragma#modules) if you're planning on adding new features that don't require any changes to the existing code files. This way the module can simply be installed into existing releases of Pragma. (You will still have to set up a build of Pragma before creating a binary module.)

Build Requirements
------
- ~60 GiB of disk space
- CMake 3.28.3 or newer
- Python 3.9.5 or newer, with NumPy

###### Windows
- Visual Studio 2022 or newer
- Windows SDK 10.0.22000.0 or newer

###### Linux
- Ubuntu 24.04 or newer
- clang-19 or newer (Pragma is *not* compatible with gcc!)

Build Instructions
------
To build Pragma, all you have to do is run the following command from a command-line interface:
```console
git clone https://github.com/Silverlan/pragma.git && cd pragma && python build_scripts/build.py --with-pfm --with-all-pfm-modules --with-vr
```

This will clone Pragma and run the build-script, which will automatically download all dependencies, configure CMake, and build and install the project (this will take several hours).
If you don't need the filmmaker, you can omit the `--with-pfm --with-all-pfm-modules` arguments, which will reduce the build time and the required amount of disk space.
> :warning: Linux<br/>
> Do **not** run the script as superuser.<br/>
> The script will automatically install all required system packages. Since this requires admin priviliges, you may be prompted for your password several times.<br/>
> You can disable confirmation prompts (e.g. for automated builds) by adding the `--no-confirm` argument, however entering your password may still be required.
<br/>

Once the build script has been completed, you can find the build files in `pragma/build`, and the install files in `pragma/build/install`. The `install` directory contains everything you need to run Pragma.

###### Update
To update Pragma to a newer version (assuming the command above has completed successfully at least once), you can use the following command:
```console
python build_scripts/build.py --update
```

This will pull all of the latest changes for the Pragma repository and the modules. The `--update` option will re-use all of the arguments used in the last execution of the build script, so you don't have to specify them again.

If you just wish to re-run the build script without updating to the latest commit, you can use the `--rerun` option instead. Like the `--update` option, this will also re-use the arguments used in the last execution of the build script.

###### Branches
The purpose of the branches is as follows:
### `main` Branch
- **Purpose:** The `main` branch is the primary branch that contains the latest functional code and should always be deployable. The [nightly pre-release builds](https://github.com/Silverlan/pragma/releases/tag/nightly) are generated from this branch. Please note that only [commits tagged with release points](https://github.com/Silverlan/pragma/tags) (e.g. v1.3.0) are considered stable!
- **Usage:** 
  - Only tested and approved features or bug fixes from the `develop` branch should be merged into `main`.
  - Direct commits to `main` are discouraged.
  - A stable release can be generated from `main` using the `Create Stable Release` action once the nightly release has been thoroughly tested.

### `develop` Branch
- **Purpose:** The `develop` branch is used as the integration branch for new features, bug fixes, and other changes. It acts as the "working" version of the codebase where ongoing development takes place. This branch is *not* considered stable and may not be functional or even build without errors.
- **Usage:** 
  - Developers create feature branches (e.g., `feature/feature-name`) off the `develop` branch for new features or improvements.
  - Once a feature is complete and tested, it is merged back into `develop`.

###### Code Changes
If you make any code changes to the source code, you can build the `pragma-install` target to build them. This will also re-install the binaries.

### Build Customization

Running the build-script with the arguments above will build and install Pragma and the Pragma Filmmaker with all dependencies. Alternatively you can also configure the build to your liking with the following parameters:

| Parameter                               | Description                                                                                  | Default          |
| --------------------------------------- | -------------------------------------------------------------------------------------------- | ---------------- |
| `--help`                                | Display this help                                                                            |                  |
| `--generator <generator>`               | The generator to use.                                                                        | Windows: `Visual Studio 17 2022`<br/>Linux: `Ninja Multi-Config` |
| `--c-compiler`                          | [Linux only] The C-compiler to use.                                                          | `clang-18`       |
| `--cxx-compiler`                        | [Linux only] The C++-compiler to use.                                                        | `clang++-18`     |
| `--no-sudo`                             | [Linux only] Will not run sudo commands. System packages will have to be installed manually. | `0`              |
| `--no-confirm`                          | [Linux only] Disable any interaction with user (suitable for automated run).                 | `0`              |
| `--with-essential-client-modules <1/0>` | Include essential modules required to run Pragma.                                            | `1`              |
| `--with-common-modules <1/0>`           | Include non-essential but commonly used modules (e.g. audio and physics modules).            | `1`              |
| `--with-pfm <1/0>`                      | Include the Pragma Filmmaker.                                                                | `0`              |
| `--with-core-pfm-modules <1/0>`         | Include essential PFM modules.                                                               | `1`              |
| `--with-all-pfm-modules <1/0>`          | Include non-essential PFM modules (e.g. chromium and cycles).                                | `0`              |
| `--with-vr <1/0>`                       | Include Virtual Reality support.                                                             | `0`              |
| `--with-networking <1/0>`               | Include networking module(s) for multiplayer support.                                        | `0`              |
| `--with-lua-debugger <1/0>`             | Include Lua-debugger support.                                                                | `0`              |
| `--with-swiftshader <1/0>`              | Include SwiftShader support for CPU-only rendering.                                          | `0`              |
| `--build-swiftshader <1/0>`             | Builds SwiftShader from source instead of downloading prebuilt binaries.                     | `0`              |
| `--build-cycles <1/0>`                  | Build the Cycles library (otherwise uses pre-built binaries). Requires --with-all-pfm-modules| `0`              |
| `--build <1/0>`                         | Build Pragma after configurating and generating build files.                                 | `1`              |
| `--build-all <1/0>`                     | Build all dependencies instead of downloading prebuilt binaries where available.             | `0`              |
| `--build-config <config>`               | The build configuration to use.                                                              | `RelWithDebInfo` |
| `--build-directory <path>`              | Directory to write the build files to. Can be relative or absolute.                          | `build`          |
| `--deps-directory <path>`               | Directory to write the dependency files to. Can be relative or absolute.                     | `deps`           |
| `--install-directory <path>`            | Installation directory. Can be relative (to build directory) or absolute.                    | `install`        |
| `--verbose <1/0>`                       | Print additional debug information.                                                          | `0`              |
| `--update <1/0>`                        | Update Pragma and all submodules and modules to the latest versions.                         | `0`              |
| `--rerun <1/0>`                         | Re-run the build script with the previous arguments.                                         | `0`              |
| `--module <moduleName>:<gitUrl>`        | Custom modules to install. Use this parameter multiple times to use multiple modules.        |                  |
| `--cmake-arg <arg>`                     | Custom CMake configuration option. Use this parameter multiple times for multiple options.   |                  |
| `--vtune-include-path <path>`           | Path to VTune include folder. Required for CPU profiling with Intel VTune Profiler.          |                  |
| `--vtune-library-path <libPath>`        | Path to "libittnotify" VTune library. Required for CPU profiling with Intel VTune Profiler.  |                  |

Example for using the `--module` parameter:
```console
--module pr_physx:"https://github.com/Silverlan/pr_physx.git"
```

Alternatively you can also add custom modules by editing `pragma/build_scripts/user_modules.py`. (This is the recommended method.)

###### PFM
To build Pragma with PFM, add the `--with-pfm --with-all-pfm-modules` options. Due to licensing issues, this will only include a pre-built version of the Cycles renderer **without** OptiX support.
If you want to have full CUDA and OptiX support when rendering with Cycles, you will have to add the `--build-cycles` option as well. You will also have to install the following before you do so:
- [SlikSVN](https://sliksvn.com/download/) (Windows only)
- [CUDA Toolkit 12.2](https://developer.nvidia.com/cuda-12-2-0-download-archive)
- [OptiX SDK 7.3.0](https://developer.nvidia.com/designworks/optix/download) (NVIDIA account required)

Please note that newer versions of CUDA or OptiX will likely not work.

Modules
------
If you want to add functionality to Pragma without changing the core source code, you can do so by creating a custom binary module. The setup for this is very simple and only takes a few minutes. For more information, please check out the [wiki article](https://wiki.pragma-engine.com/books/pragma-engine/page/binary-modules#bkmrk-custom-modules).

There are also various pre-made binary modules available for Pragma, some of which are already included with this repository, and some of which can be found online:
> :warning: Some of these modules have not been maintained and may no longer be compatible with the latest version of Pragma.
- pr_chromium: https://github.com/Silverlan/pr_chromium
- pr_curl: https://github.com/Silverlan/pr_curl
- pr_mysql: https://github.com/Silverlan/pragma/tree/main/modules/pr_mysql
- pr_pcl: https://github.com/Silverlan/pragma/tree/main/modules/pr_pcl
- pr_prosper_opengl: https://github.com/Silverlan/pr_prosper_opengl
- pr_prosper_vulkan: https://github.com/Silverlan/pr_prosper_vulkan
- pr_socket: https://github.com/Silverlan/pragma/tree/main/modules/pr_socket
- pr_video_recorder: https://github.com/Silverlan/pragma/tree/main/modules/pr_video_recorder
- pr_xml: https://github.com/Silverlan/pragma/tree/main/modules/pr_xml
- pr_audio_dummy: https://github.com/Silverlan/pragma/tree/main/modules/pr_audio_dummy
- pr_cycles: https://github.com/Silverlan/pr_cycles
- pr_source: https://github.com/Silverlan/pr_source
- pr_xatlas: https://github.com/Silverlan/pr_xatlas
- pr_openvr: https://github.com/Silverlan/pr_openvr
- pr_truesky: https://github.com/Silverlan/pr_truesky
- pr_opensubdiv: https://github.com/Silverlan/pr_opensubdiv
- pr_sqlite: https://github.com/Silverlan/pr_sqlite
- pr_xnalara: https://github.com/Silverlan/pr_xnalara
- pr_mpv: https://github.com/Silverlan/pr_mpv
- pr_dmx: https://github.com/Silverlan/pr_dmx
- pr_steamworks: https://github.com/Silverlan/pr_steamworks
- pr_gpl: https://github.com/Silverlan/pr_gpl
- pr_steam_networking_sockets: https://github.com/Silverlan/pr_steam_networking_sockets
- pr_physx: https://github.com/Silverlan/pr_physx
- pr_bullet: https://github.com/Silverlan/pr_bullet
- pr_audio_fmod: https://github.com/Silverlan/pr_audio_fmod
- pr_audio_alure: https://github.com/Silverlan/pr_audio_alure

To build a module, simply run the build-script with the following parameter:
```console
--module <moduleName>:<gitUrl>
```

The build script will clone, build and install the module automatically. Alternatively you can also download the module manually to `pragma/modules` and omit the `:<gitUrl>` portion of the parameter.

If you only want to install a module without building it, you can also run the following console command from within Pragma to download and install the module automatically:
```console
install_module <githubModuleName> [<version>]
```
The `githubModuleName` consists of the GitHub username and the repository name.
If no version is specified, the latest release binaries will be used.

Example:
```console
install_module Silverlan/pr_curl
```

Addons
------
In addition to binary modules, Pragma also uses Lua as scripting language, with thousands of available function, class and library bindings. To get more information about the Lua API, check out the [official wiki](https://wiki.pragma-engine.com/books/lua-api).
An example for a Lua-addon is the [Pragma Filmmaker](https://github.com/Silverlan/pfm).

Special Thanks
------
- [SlawekNowy](https://github.com/SlawekNowy): For helping to make Linux support possible
- [REDxEYE](https://github.com/REDxEYE): For creating the [Pragma asset import plugin for Blender](https://github.com/REDxEYE/pragma_udm_io)
- [ZeqMacaw](https://github.com/ZeqMacaw): For their work on [Crowbar](https://github.com/ZeqMacaw/Crowbar)
- All of the contributors of the [third-party libraries](https://github.com/Silverlan/pragma/blob/main/build_scripts/scripts/third_party_libs.py) used by Pragma/PFM
