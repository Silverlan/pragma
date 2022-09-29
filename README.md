[![Build Windows](https://github.com/Silverlan/pragma/actions/workflows/pragma-windows-ci.yml/badge.svg?branch=master)](https://github.com/Silverlan/pragma/actions/workflows/pragma-windows-ci.yml)

What is this?
------
This is the repository for the Pragma Game Engine. For more information, visit the official website: https://pragma-engine.com/

What platforms and hardware does it work on?
------
- 64-Bit Windows (8.1, 10, 11)
- Graphics card: GTX 1050 Ti or newer
- Memory: 4 GiB RAM or more

What are Pragma's requirements?
------
- Windows compiler with C++-20 support (Linux support will follow)
- CMake 3.20.2 or newer: https://cmake.org/download/#latest
- Boost 1.76: https://www.boost.org/users/history/version_1_76_0.html
- zlib 1.2.8: https://github.com/fmrico/zlib-1.2.8
- Geometric Tools Engine: https://github.com/davideberly/GeometricTools
- Python 3.8 or newer: https://www.python.org/downloads/

If you want to use Vulkan, you'll also need these:
- Vulkan SDK 1.2.162.1 or newer: https://vulkan.lunarg.com/sdk/home
- SPIRV-Tools: https://github.com/KhronosGroup/SPIRV-Tools
- SPIRV-Headers: https://github.com/KhronosGroup/SPIRV-Headers

`SPIRV-Tools` has to be located in `%VULKAN_SDK%/spirv-tools`, and `SPIRV-Headers` in `%VULKAN_SDK%/spirv-tools/external/spirv-headers`.

Build Instructions (Windows)
------
Before building Pragma, you have to build several third-party libraries by hand first:

### Building Boost
Build zlib 1.2.8, then open the Windows command line and navigate to the boost directory, then execute the following commands:
```
bootstrap.bat
b2 toolset=msvc-14.2 address-model=64 stage variant=release link=shared runtime-link=shared -j3 > build.log
b2 toolset=msvc-14.2 address-model=64 stage variant=release link=static runtime-link=shared -j3 > build.log

set ZLIB_SOURCE="E:/zlib-1.2.8"
set ZLIB_INCLUDE="E:/zlib-1.2.8"
set ZLIB_LIBPATH="E:/zlib-1.2.8/build/RelWithDebInfo"
b2 toolset=msvc-14.2 address-model=64 stage variant=release link=shared runtime-link=shared --with-iostreams -sZLIB_SOURCE=%ZLIB_SOURCE% -sZLIB_INCLUDE=%ZLIB_INCLUDE% -sZLIB_LIBPATH=%ZLIB_LIBPATH%
b2 toolset=msvc-14.2 address-model=64 stage variant=release link=static runtime-link=shared --with-iostreams -sZLIB_SOURCE=%ZLIB_SOURCE% -sZLIB_INCLUDE=%ZLIB_INCLUDE% -sZLIB_LIBPATH=%ZLIB_LIBPATH%
```
(Change `ZLIB_SOURCE`, `ZLIB_INCLUDE` and `ZLIB_LIBPATH` to your zlib path.)

### Building LuaJIT
- Open Visual Studio command prompt (Available from the menu bar under "Tools -> Visual Studio Command Prompt")
- Go to `%PROGRAMFILES(X86)%/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build` (or the equivalent directory for your Visual Studio version)
- Run `vcvarsall.bat x64`
- Go to `Pragma/third_party_libs/luajit/src`
- Run `msvcbuild.bat`

4) Run configure with CMake in the Pragma root directory
5) Set the following variables:
- `DEPENDENCY_BOOST_INCLUDE`: Boost include directory
- `DEPENDENCY_GEOMETRIC_TOOLS_INCLUDE`: GTE include directory
- `DEPENDENCY_BOOST_CHRONO_LIBRARY`: Path to boost chrono library (boost_chrono-vc142-mt-x64-1_72.lib)
- `DEPENDENCY_BOOST_DATE_TIME_LIBRARY`: Path to boost date-time library (boost_date_time-vc142-mt-x64-1_72.lib)
- `DEPENDENCY_BOOST_FILESYSTEM_LIBRARY`: Path to boost filesystem library (boost_filesystem-vc142-mt-x64-1_72.lib)
- `DEPENDENCY_BOOST_IOSTREAMS_LIBRARY`: Path to boost iostreams library (boost_iostreams-vc142-mt-x64-1_72.lib)
- `DEPENDENCY_BOOST_REGEX_LIBRARY`: Path to boost regex library (boost_regex-vc142-mt-x64-1_72.lib)
- `DEPENDENCY_BOOST_SYSTEM_LIBRARY`: Path to boost system library (boost_system-vc142-mt-x64-1_72.lib)
- `DEPENDENCY_BOOST_THREAD_LIBRARY`: Path to boost thread library (boost_thread-vc142-mt-x64-1_72.lib)
6) Run configure again, then generate and open the solution
7) Build Pragma

The first time you should build Pragma using the `core/pragma-install-full` target, which is equal to `core/pragma-install`, but also copies all of the base assets required to run Pragma to the install directory (default is `install`).
For all subsequent builds, you can use `core/pragma-install` instead, which includes all of the core libraries, required dependencies and the OpenGL module for Pragma. All available module builds will automatically get installed every time you use the `core/pragma-install` target.

Pragma uses OpenGL by default if the Vulkan module has not been built, however Pragma is optimized for Vulkan. To use the Vulkan renderer:
- Make sure the VulkanSDK is installed
- Build `modules/pr_prosper_vulkan`
- Build `core/pragma-install`

Vulkan will automatically be used by default if available. You can also force a particular renderer by adding `-graphics_api opengl` or `-graphics_api vulkan` to the launch options respectively.

Build Instructions (Linux)
------
TODO

Modules
------
There are various optional modules available for Pragma, some of which are already included with this repository, and some of which can be found online:
- pr_chromium: https://github.com/Silverlan/pr_chromium
- pr_curl: https://github.com/Silverlan/pr_curl
- pr_mysql: https://github.com/Silverlan/pragma/tree/master/modules/pr_mysql
- pr_pcl: https://github.com/Silverlan/pragma/tree/master/modules/pr_pcl
- pr_prosper_opengl: https://github.com/Silverlan/pr_prosper_opengl
- pr_prosper_vulkan: https://github.com/Silverlan/pr_prosper_vulkan
- pr_socket: https://github.com/Silverlan/pragma/tree/master/modules/pr_socket
- pr_video_recorder: https://github.com/Silverlan/pragma/tree/master/modules/pr_video_recorder
- pr_xml: https://github.com/Silverlan/pragma/tree/master/modules/pr_xml
- pr_audio_dummy: https://github.com/Silverlan/pragma/tree/master/modules/pr_audio_dummy

- pr_cycles: https://github.com/Silverlan/pr_cycles
- pr_source: https://github.com/Silverlan/pr_source
- pr_xatlas: https://github.com/Silverlan/pr_xatlas
- pr_openvr: https://github.com/Silverlan/pr_openvr
- pr_truesky: https://github.com/Silverlan/pr_truesky
- pr_opensubdiv: https://github.com/Silverlan/pr_opensubdiv
- pr_open_color_io: https://github.com/Silverlan/pr_open_color_io
- pr_sqlite: https://github.com/Silverlan/pr_sqlite
- pr_xnalara: https://github.com/Silverlan/pr_xnalara
- pr_mpv: https://github.com/Silverlan/pr_mpv
- pr_dmx: https://github.com/Silverlan/pr_dmx
- pr_steamworks: https://github.com/Silverlan/pr_steamworks
- pr_luxcore: https://github.com/Silverlan/pr_luxcore
- pr_mmd: https://github.com/Silverlan/pr_mmd
- pr_uvatlas: https://github.com/Silverlan/pr_uvatlas
- pr_gpl: https://github.com/Silverlan/pr_gpl
- pr_steam_networking_sockets: https://github.com/Silverlan/pr_steam_networking_sockets
- pr_physx: https://github.com/Silverlan/pr_physx
- pr_bullet: https://github.com/Silverlan/pr_bullet
- pr_audio_fmod: https://github.com/Silverlan/pr_audio_fmod
- pr_audio_alure: https://github.com/Silverlan/pr_audio_alure

To install a module, check it out into `Pragma/modules`, re-configure and re-generate the solution using CMake, then build the module and then build `core/pragma-install` to install it.
