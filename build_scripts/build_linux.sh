cxx_compiler="clang++-14"
c_compiler="clang-14"
generator="Unix Makefiles"
vulkan_lib="/lib/x86_64-linux-gnu"
with_essential_client_modules=1
with_common_modules=1
with_pfm=0
with_core_pfm_modules=1
with_all_pfm_modules=0
with_vr=0
modules=""
build=1
build_config="RelWithDebInfo"
build_directory="build"
deps_directory="deps"
install_directory="install"
help=0

arg_to_bool () {
    if [ "$1" = true ] || [ "$1" = "TRUE" ] || [ "$1" = 1 ] || [ "$1" = "on" ] || [ "$1" = "ON" ] || [ "$1" = "enabled" ] || [ "$1" = "ENABLED" ]; then
        return 1
    else
        return 0
    fi
}

print_hmsg () {
    GREEN='\033[0;32m'
    NC='\033[0m' # No Color
    printf "${GREEN}$1${NC}\n"
}

is_abs_path () {
	if [ "${1:0:1}" = "/" ]; then
		echo 1
	else
	  echo 0
  fi
}

display_help () {
	RED='\033[0;31m'
    GREEN='\033[0;32m'
    GRAY='\033[0;37m'
    NC='\033[0m' # No Color
    echo "This script will download and setup all of the required dependencies for building Pragma."
    echo "Usage: ./build_scripts/build_windows.ps1 [option...]"
    echo ""

    echo "   -c_compiler                       The C-compiler to use. Default: " -n
    echo "\"clang-14\""

    echo "   -cxx_compiler                     The C++-compiler to use. Default: " -n
    echo "\"clang++-14\""

    echo "   -generator                        The generator to use. Default: " -n
    echo "\"Unix Makefiles\""

    echo "   -with_essential_client_modules    Include essential modules required to run Pragma. Default: " -n
    echo "${GREEN}true${NC}"

    echo "   -with_common_modules              Include non-essential but commonly used modules (e.g. audio and physics modules). Default: " -n
    echo "${GREEN}true${NC}"

    echo "   -with_pfm                         Include the Pragma Filmmaker. Default: " -n
    echo "${RED}false${NC}"

    echo "   -with_core_pfm_modules            Include essential PFM modules. Default: " -n
    echo "${GREEN}true${NC}"

    echo "   -with_all_pfm_modules             Include non-essential PFM modules (e.g. chromium and cycles). Default: " -n
    echo "${RED}false${NC}"

    echo "   -with_vr                          Include Virtual Reality support. Default: " -n
    echo "${RED}false${NC}"

    echo "   -build                            Build Pragma after configurating and generating build files. Default: " -n
    echo "${GREEN}true${NC}"

    echo "   -build_config                     The build configuration to use. Default: " -n
    echo "RelWithDebInfo"

    echo "   -build_directory                  Directory to write the build files to. Can be relative or absolute. Default: " -n
    echo "build"

    echo "   -deps_directory                   Directory to write the dependency files to. Can be relative or absolute. Default: " -n
    echo "deps"

    echo "   -vulkan_lib                       Directory for vulkan libraries. Default: " -n
    echo "/lib/x86_64-linux-gnu"

    echo "   -install_directory                Installation directory. Can be relative (to build directory) or absolute. Default: " -n
    echo "install"

    echo "   -help                             Display this help"
    echo "   -modules                          Custom modules to install. Usage example: " -n
    echo "${GRAY}-modules pr_prosper_vulkan:\"https://github.com/Silverlan/pr_prosper_vulkan.git\",pr_bullet:\"https://github.com/Silverlan/pr_bullet.git\"${NC}"
    echo ""

    echo "Examples:"
    echo "- Build Pragma with clang-14:"
    echo "${GRAY}bash ./build_scripts/build_linux.sh -cxx_compiler \"clang++-14\" -c_compiler \"clang-14\" -generator \"Unix Makefiles\"${NC}"
    echo ""
    echo "- Build Pragma with PFM and VR support with clang-14:"
    echo "${GRAY}bash ./build_scripts/build_linux.sh -with_pfm -with_all_pfm_modules -with_vr -cxx_compiler \"clang++-14\" -c_compiler \"clang-14\" -generator \"Unix Makefiles\"${NC}"
    exit 0
}

displayHelp=0
for i in "$@"; do
  case $i in
    -t=*|--cxx_compiler=*)
      cxx_compiler="${i#*=}"
      shift # past argument=value
      ;;
    -c=*|--c_compiler=*)
      c_compiler="${i#*=}"
      shift # past argument=value
      ;;
    -g=*|--generator=*)
      arg_to_bool "${i#*=}"
      generator=$?
      shift # past argument=value
      ;;
    --with_essential_client_modules=*)
      arg_to_bool "${i#*=}"
      with_essential_client_modules=$?
      shift # past argument=value
      ;;
    --with_essential_client_modules)
      with_essential_client_modules=1
      shift # past argument with no value
      ;;
    --with_common_modules=*)
      arg_to_bool "${i#*=}"
      with_common_modules=$?
      shift # past argument=value
      ;;
    --with_common_modules)
      with_common_modules=1
      shift # past argument with no value
      ;;
    --with_pfm=*)
      arg_to_bool "${i#*=}"
      with_pfm=$?
      shift # past argument=value
      ;;
    --with_pfm)
      with_pfm=1
      shift # past argument with no value
      ;;
    --with_core_pfm_modules=*)
      arg_to_bool "${i#*=}"
      with_core_pfm_modules=$?
      shift # past argument=value
      ;;
    --with_core_pfm_modules)
      with_core_pfm_modules=1
      shift # past argument with no value
      ;;
    --with_all_pfm_modules=*)
      arg_to_bool "${i#*=}"
      with_all_pfm_modules=$?
      shift # past argument=value
      ;;
    --with_all_pfm_modules)
      with_all_pfm_modules=1
      shift # past argument with no value
      ;;
    --with_vr=*)
      arg_to_bool "${i#*=}"
      with_vr=$?
      shift # past argument=value
      ;;
    --with_vr)
      with_vr=1
      shift # past argument with no value
      ;;
    --modules=*)
      modules="${i#*=}"
      shift # past argument=value
      ;;
    --build_config=*)
      build_config="${i#*=}"
      shift # past argument=value
      ;;
    --build_directory=*)
      build_directory="${i#*=}"
      shift # past argument=value
      ;;
    --deps_directory=*)
      deps_directory="${i#*=}"
      shift # past argument=value
      ;;
    --install_directory=*)
      install_directory="${i#*=}"
      shift # past argument=value
      ;;
    --help)
      displayHelp=1
      shift # past argument with no value
      ;;
    -*|--*)
      echo "Unknown option $i"
      exit 1
      ;;
    *)
      ;;
  esac
done

if [ "$displayHelp" == "1" ]; then
	display_help
	exit 0
fi

# Print Inputs
echo "Inputs:"
echo "cxx_compiler: $cxx_compiler"
echo "c_compiler: $c_compiler"
echo "generator: $generator"
echo "with_essential_client_modules: $with_essential_client_modules"
echo "with_common_modules: $with_common_modules"
echo "with_pfm: $with_pfm"
echo "with_core_pfm_modules: $with_core_pfm_modules"
echo "with_all_pfm_modules: $with_all_pfm_modules"
echo "with_vr: $with_vr"
echo "build: $build"
echo "build_config: $build_config"
echo "build_directory: $build_directory"
echo "deps_directory: $deps_directory"
echo "install_directory: $install_directory"
echo "modules: $modules"
#

strindex() { 
  x="${1%%"$2"*}"
  [[ "$x" = "$1" ]] && return -1 || return "${#x}"
}

validate_result() {
    resultCode=$?
    if [ $resultCode -ne 0 ]; then
        RED='\033[0;31m'
        NC='\033[0m' # No Color
        printf "${RED}Critical failure detected, execution will halt!${NC}\n"
        exit 1
    fi
}

# Linux
export CC="$c_compiler"
export CXX="$cxx_compiler"
#

buildConfig="$build_config"
root="$PWD"
buildDir="$build_directory"
depsDir="$deps_directory"
installDir="$install_directory"
tools="$root/tools"

isBuildDirAbs=$(is_abs_path $buildDir)
if [ "$isBuildDirAbs" != "1" ]; then
	buildDir="$PWD/$buildDir"
fi

isDepsDirAbs=$(is_abs_path $depsDir)
if [ "$isDepsDirAbs" != "1" ]; then
	depsDir="$PWD/$depsDir"
fi
isInstallDirAbs=$(is_abs_path $installDir)
if [ "$isInstallDirAbs" != "1" ]; then
	installDir="$buildDir/$installDir"
fi

mkdir -p "$buildDir"
mkdir -p "$depsDir"
mkdir -p "$installDir"
mkdir -p "$tools"

cd $depsDir

deps="$PWD"
# Get zlib
zlibRoot="$PWD/zlib-1.2.8"
if [ ! -d "$zlibRoot" ]; then
	echo "zlib not found. Downloading..."
	git clone https://github.com/fmrico/zlib-1.2.8.git
	echo "Done!"
fi

cd zlib-1.2.8

# Build zlib
echo "Building zlib..."
if [ ! -d "$PWD/build" ]; then
	mkdir build
fi
cd build
cmake .. -G "$generator"
cmake --build "." --config "$buildConfig"
cp zconf.h ../
cd ../..
echo "Done!"

cd $depsDir
if [ ! -d "$PWD/boost_1_78_0" ]; then
	echo "boost not found. Downloading..."
	wget https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.zip
	echo "Done!"

	# Extract Boost
	echo "Extracting boost..."
	7z x "$PWD/boost_1_78_0.zip"

	rm boost_1_78_0.zip
	echo "Done!"
fi

cd boost_1_78_0

# Build Boost
echo "Building boost..."
./bootstrap.sh
./b2 address-model=64 stage variant=release link=shared runtime-link=shared -j3
./b2 address-model=64 stage variant=release link=static runtime-link=shared -j3
echo "Done!"

ZLIB_SOURCE="$PWD/../zlib-1.2.8"
ZLIB_INCLUDE="$PWD/../zlib-1.2.8"
ZLIB_LIBPATH="$PWD/../zlib-1.2.8/build"
echo "Building boost zlib libraries..."
./b2 address-model=64 stage variant=release link=shared runtime-link=shared --with-iostreams -sZLIB_SOURCE="$ZLIB_SOURCE" -sZLIB_INCLUDE="$ZLIB_INCLUDE" -sZLIB_LIBPATH="$ZLIB_LIBPATH"
./b2 address-model=64 stage variant=release link=static runtime-link=shared --with-iostreams -sZLIB_SOURCE="$ZLIB_SOURCE" -sZLIB_INCLUDE="$ZLIB_INCLUDE" -sZLIB_LIBPATH="$ZLIB_LIBPATH"
cd ../
echo "Done!"

cd ../

# Build LuaJIT
echo "Building LuaJIT..."
cd third_party_libs/luajit/src
make
cd ../../../
echo "Done!"

# Download GeometricTools
echo "Downloading GeometricTools..."
cd $deps
git clone https://github.com/davideberly/GeometricTools
cd GeometricTools
git reset --hard bd7a27d18ac9f31641b4e1246764fe30816fae74
cd ../../
echo "Done!"

# Download SPIRV-Tools
echo "Downloading SPIRV-Tools..."
cd $deps
git clone https://github.com/KhronosGroup/SPIRV-Tools.git
cd SPIRV-Tools
# Note: When updating to a newer version, the SPIRV-Headers commit below has to match
# the one defined in https://github.com/KhronosGroup/SPIRV-Tools/blob/master/DEPS for the
# timestamp of this commit
git reset --hard 7826e19
cd ../../
echo "Done!"

# Download SPIRV-Headers
echo "Downloading SPIRV-Headers..."
cd $deps
cd SPIRV-Tools/external
git clone https://github.com/KhronosGroup/SPIRV-Headers
cd SPIRV-Headers
git reset --hard 4995a2f2723c401eb0ea3e10c81298906bf1422b
cd ../../
cd ../../
echo "Done!"

# Download modules
echo "Downloading modules..."
cd modules

# TODO: Allow defining custom modules via arguments
modules=()
if [ "$with_essential_client_modules" == 1 ]; then
    modules+=( "pr_prosper_vulkan:\"https://github.com/Silverlan/pr_prosper_vulkan.git\"" )
fi

if [ "$with_common_modules" == 1 ]; then
	modules+=( "pr_bullet:\"https://github.com/Silverlan/pr_bullet.git\"" )
  modules+=( "pr_audio_soloud:\"https://github.com/Silverlan/pr_soloud.git\"" )
fi

if [ "$with_pfm" == 1 ]; then
	if [ "$with_core_pfm_modules" == 1 ] || [ "$with_all_pfm_modules" == 1 ]; then
		modules+=( "pr_curl:https://github.com/Silverlan/pr_curl.git" )
		modules+=( "pr_dmx:https://github.com/Silverlan/pr_dmx.git" )
	fi
	if [ "$with_all_pfm_modules" == 1 ]; then
		modules+=( "pr_chromium:https://github.com/Silverlan/pr_chromium.git" )
		modules+=( "pr_unirender:https://github.com/Silverlan/pr_cycles.git" )
		modules+=( "pr_curl:https://github.com/Silverlan/pr_curl.git" )
		modules+=( "pr_dmx:https://github.com/Silverlan/pr_dmx.git" )
		modules+=( "pr_xatlas:https://github.com/Silverlan/pr_xatlas.git" )
	fi
fi

if [ "$with_vr" == 1 ]; then
  modules+=( "pr_openvr:https://github.com/Silverlan/pr_openvr.git" )
fi

moduleList=""
cmakeArgs=""
additionalBuildTargets=""
for module in "${modules[@]}"
do
    strindex "$module" ":"
    index=$?
    moduleName=${module:0:$index}
    moduleUrl=${module:$index +1:${#module}}
    moduleDir="$PWD/$moduleName/"

    # Remove quotes
    moduleUrl=$(echo "$moduleUrl" | sed "s/\"//g")

    if [ ! -d "$moduleDir" ]; then
		echo "Downloading module '$moduleName'..."
		git clone "$moduleUrl" --recurse-submodules $moduleName
		echo "Done!"
    else
		echo "Updating module '$moduleName'..."
		git pull
		echo "Done!"
	fi

  if [ -f "$moduleDir/build_scripts/setup_linux.sh" ]; then
		echo "Executing module setup script..."
		curDir=$PWD
        source "$PWD/$moduleName/build_scripts/setup_linux.sh"
		cd $curDir
		echo "Done!"
	fi

	moduleList="$moduleList $moduleName"
done

cd ..
echo "Done!"

# Configure
echo "Configuring Pragma..."
rootDir=$PWD
cd $buildDir
echo "Additional CMake args: $cmakeArgs"

cmakeConfigure="cmake .. -G \"$generator\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_BOOST_INCLUDE=\"$deps/boost_1_78_0\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_BOOST_LIBRARY_LOCATION=\"$deps/boost_1_78_0/stage/lib\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_BOOST_CHRONO_LIBRARY=\"$deps/boost_1_78_0/stage/lib/boost_chrono.a\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_BOOST_DATE_TIME_LIBRARY=\"$deps/boost_1_78_0/stage/lib/boost_date_time.a\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_BOOST_REGEX_LIBRARY=\"$deps/boost_1_78_0/stage/lib/boost_regex.a\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_BOOST_SYSTEM_LIBRARY=\"$deps/boost_1_78_0/stage/lib/boost_system.a\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_BOOST_THREAD_LIBRARY=\"$deps/boost_1_78_0/stage/lib/boost_thread.a\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_GEOMETRIC_TOOLS_INCLUDE=\"$deps/GeometricTools/GTE\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_LIBZIP_CONF_INCLUDE=\"$buildDir/third_party_libs/libzip\" "
cmakeConfigure="$cmakeConfigure -DCMAKE_INSTALL_PREFIX:PATH=\"$installDir\" "
cmakeConfigure="$cmakeConfigure -DDEPENDENCY_SPIRV_TOOLS_DIR=\"$deps/SPIRV-Tools\" "
# cmakeConfigure="$cmakeConfigure -DDEPENDENCY_VULKAN_LIBRARY=\"$vulkan_lib\" "
cmakeConfigure="$cmakeConfigure -DBUILD_TESTING=OFF "
cmakeConfigure="$cmakeConfigure $cmakeArgs "
echo "Running configure command:"
echo "$cmakeConfigure"
eval "$cmakeConfigure"

print_hmsg "Done!"

print_hmsg "Build files have been written to \"$buildDir\"."

curDir="$PWD"
if [ "$with_pfm" == 1 ]; then
	print_hmsg "Downloading PFM addon..."
  mkdir -p "$installDir/addons"
	cd "$installDir/addons"
  if [ ! -d "$installDir/addons/filmmaker" ]; then
		git clone "https://github.com/Silverlan/pfm.git" filmmaker
		validate_result
  else
		print_hmsg "Updating PFM..."
    cd filmmaker
		git pull
    cd ..
		validate_result
  fi
	print_hmsg "Done!"

	print_hmsg "Downloading model editor addon..."
  mkdir -p "$installDir/addons"
	cd "$installDir/addons"
  if [ ! -d "$installDir/addons/tool_model_editor" ]; then
		git clone "https://github.com/Silverlan/pragma_model_editor.git" tool_model_editor
		validate_result
  else
		print_hmsg "Updating model editor addon..."
    cd tool_model_editor
		git pull
    cd ..
		validate_result
  fi
	print_hmsg "Done!"
fi

if [ "$with_vr" == 1 ]; then
	print_hmsg "Downloading VR addon..."
  mkdir -p "$installDir/addons"
	cd "$installDir/addons"
  if [ ! -d "$installDir/addons/virtual_reality" ]; then
		git clone "https://github.com/Silverlan/PragmaVR.git" virtual_reality
		validate_result
  else
		print_hmsg "Updating VR..."
    cd virtual_reality
		git pull
    cd ..
		validate_result
  fi
	print_hmsg "Done!"
fi
cd $curDir

if [ "$build" == 1 ]; then
	print_hmsg "Building Pragma..."
	targets="pragma-install-full $moduleList"
	if [ "$with_pfm" == 1 ]; then
	  targets="$targets pfm"
	fi
	targets="$targets pragma-install"

	cd "$buildDir"
	cmakeBuild="cmake --build \".\" --config \"$buildConfig\" --target $targets $additionalBuildTargets"
	echo "Running build command:"
	echo "$cmakeBuild"
  	eval "$cmakeBuild"

	validate_result

	print_hmsg "Build Successful! Pragma has been installed to \"$installDir\"."
	print_hmsg "If you make any changes to the core source code, you can build the \"pragma-install\" target to compile the changes and re-install the binaries automatically."
	print_hmsg "If you make any changes to a module, you will have to build the module target first, and then build \"pragma-install\"."
	print_hmsg ""
fi
