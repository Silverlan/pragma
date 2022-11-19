param (
	[string]$toolset = "msvc-14.2",
	[string]$generator = "Visual Studio 17 2022",
	[string]$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat",
	[switch]$with_essential_client_modules = $true,
	[switch]$with_common_modules = $true,
	[switch]$with_pfm = $false,
	[switch]$with_core_pfm_modules = $true,
	[switch]$with_all_pfm_modules = $false,
	[switch]$with_vr = $false,
	[switch]$build = $true,
	[switch]$help = $false,
	[string[]]$modules = @()
)

$ErrorActionPreference="Stop"

Function display_help() {
	Write-Host "This script will download and setup all of the required dependencies for building Pragma."
    Write-Host "Usage: ./build_scripts/build_windows.ps1 [option...]"
    Write-Host ""

    Write-Host "   -toolset                          The toolset to use. Default: " -NoNewline
    Write-Host "`"msvc-14.2`""

    Write-Host "   -generator                        The generator to use. Default: " -NoNewline
    Write-Host "`"Visual Studio 17 2022`""

    Write-Host "   -vcvars                           Path to vcvars64.bat. Default: " -NoNewline
    Write-Host "`"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat`""

    Write-Host "   -with_essential_client_modules    Include essential modules required to run Pragma. Default: " -NoNewline
    Write-Host "true" -ForegroundColor Green

    Write-Host "   -with_common_modules              Include non-essential but commonly used modules (e.g. audio and physics modules). Default: " -NoNewline
    Write-Host "true" -ForegroundColor Green

    Write-Host "   -with_pfm                         Include the Pragma Filmmaker. Default: " -NoNewline
    Write-Host "false" -ForegroundColor Red

    Write-Host "   -with_core_pfm_modules            Include essential PFM modules. Default: " -NoNewline
    Write-Host "true" -ForegroundColor Green

    Write-Host "   -with_all_pfm_modules             Include non-essential PFM modules (e.g. chromium and cycles). Default: " -NoNewline
    Write-Host "false" -ForegroundColor Red

    Write-Host "   -with_vr                          Include Virtual Reality support. Default: " -NoNewline
    Write-Host "false" -ForegroundColor Red

    Write-Host "   -build                            Build Pragma after configurating and generating build files. Default: " -NoNewline
    Write-Host "true" -ForegroundColor Green

    Write-Host "   -help                             Display this help"
    Write-Host "   -modules                          Custom modules to install. Usage example: " -NoNewLine
    Write-Host "-modules pr_prosper_vulkan:`"https://github.com/Silverlan/pr_prosper_vulkan.git`",pr_bullet:`"https://github.com/Silverlan/pr_bullet.git`"" -ForegroundColor Gray
    Write-Host ""

    Write-Host "Examples:"
    Write-Host "- Build Pragma for Visual Studio 2022:"
    Write-Host "./build_scripts/build_windows.ps1 -toolset `"msvc-14.2`" -generator `"Visual Studio 17 2022`" -vcvars `"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat`"" -ForegroundColor Gray
    Write-Host ""
    Write-Host "- Build Pragma with PFM and VR support for Visual Studio 2022:"
    Write-Host "./build_scripts/build_windows.ps1 -with_pfm -with_all_pfm_modules -with_vr -toolset `"msvc-14.2`" -generator `"Visual Studio 17 2022`" -vcvars `"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat`"" -ForegroundColor Gray
    Exit 1
}

if($help){
	display_help
	Exit 0
}

$buildConfig="RelWithDebInfo"
$root="$PWD"

Function print_hmsg($msg)
{
    Write-Host "$msg" -ForegroundColor Green
}

Function validate_result()
{
	if (-not $?) {throw "Critical failure detected, execution will halt!"}
}

if(![System.IO.Directory]::Exists("$PWD/deps")){
	mkdir deps
}
cd deps

$deps="$PWD"
# Get zlib
$zlibRoot="$PWD/zlib-1.2.8"
if(![System.IO.Directory]::Exists("$zlibRoot")){
	print_hmsg "zlib not found. Downloading..."
	git clone "https://github.com/fmrico/zlib-1.2.8.git"
	validate_result
	print_hmsg "Done!"
}

cd zlib-1.2.8

# Build zlib
print_hmsg "Building zlib..."
if(![System.IO.Directory]::Exists("$PWD/build")){
	mkdir build
}
cd build
cmake .. -G $generator
validate_result
cmake --build "." --config "$buildConfig"
validate_result
cp zconf.h ..
cd ../..
print_hmsg "Done!"

$boostRoot="$PWD/boost_1_78_0"
if(![System.IO.Directory]::Exists("$boostRoot")){
	print_hmsg "boost not found. Downloading..."
	Invoke-WebRequest "https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.zip" -OutFile "boost_1_78_0.zip"
	validate_result
	print_hmsg "Done!"

	# Extract Boost
	print_hmsg "Extracting boost..."
	Add-Type -Assembly "System.IO.Compression.Filesystem"
	[System.IO.Compression.ZipFile]::ExtractToDirectory("$PWD/boost_1_78_0.zip", "$PWD")
	validate_result

	rm boost_1_78_0.zip
	print_hmsg "Done!"
}


cd boost_1_78_0

# Build Boost
print_hmsg "Building boost..."
cmd.exe /c "$PWD/bootstrap.bat"
validate_result
./b2 toolset=$toolset address-model=64 stage variant=release link=shared runtime-link=shared -j3
validate_result
./b2 toolset=$toolset address-model=64 stage variant=release link=static runtime-link=shared -j3
validate_result
print_hmsg "Done!"

$ZLIB_SOURCE="$PWD/../zlib-1.2.8"
$ZLIB_INCLUDE="$PWD/../zlib-1.2.8"
$ZLIB_LIBPATH="$PWD/../zlib-1.2.8/build/$buildConfig"
print_hmsg "Building boost zlib libraries..."
./b2 toolset="$toolset" address-model=64 stage variant=release link=shared runtime-link=shared --with-iostreams -sZLIB_SOURCE="$ZLIB_SOURCE" -sZLIB_INCLUDE="$ZLIB_INCLUDE" -sZLIB_LIBPATH="$ZLIB_LIBPATH"
validate_result
./b2 toolset="$toolset" address-model=64 stage variant=release link=static runtime-link=shared --with-iostreams -sZLIB_SOURCE="$ZLIB_SOURCE" -sZLIB_INCLUDE="$ZLIB_INCLUDE" -sZLIB_LIBPATH="$ZLIB_LIBPATH"
validate_result
cd ../
print_hmsg "Done!"

cd ../

# Build LuaJIT
print_hmsg "Building LuaJIT..."
cd third_party_libs/luajit/src
cmd.exe /c "`"$vcvars`" & msvcbuild.bat"
validate_result
cd ../../../
print_hmsg "Done!"

# Download GeometricTools
cd $deps
$geometricToolsRoot="$PWD/GeometricTools"
if(![System.IO.Directory]::Exists("$geometricToolsRoot")){
	print_hmsg "GeometricTools not found. Downloading..."
	git clone "https://github.com/davideberly/GeometricTools"
	validate_result
}
cd GeometricTools
git reset --hard bd7a27d18ac9f31641b4e1246764fe30816fae74
validate_result
cd ../../
print_hmsg "Done!"
#}
# Download modules
print_hmsg "Downloading modules..."
cd modules

if($with_essential_client_modules) {
	$modules += "pr_prosper_vulkan:`"https://github.com/Silverlan/pr_prosper_vulkan.git`""
}

if($with_common_modules) {
	$modules += "pr_bullet:`"https://github.com/Silverlan/pr_bullet.git`""
	$modules += "pr_audio_soloud:`"https://github.com/Silverlan/pr_soloud.git`""
}

if($with_pfm) {
	if($with_core_pfm_modules -Or $with_all_pfm_modules) {
		$modules += "pr_curl:https://github.com/Silverlan/pr_curl.git"
		$modules += "pr_dmx:https://github.com/Silverlan/pr_dmx.git"
	}
	if($with_all_pfm_modules) {
		$modules += "pr_chromium:https://github.com/Silverlan/pr_chromium.git"
		$modules += "pr_unirender:https://github.com/Silverlan/pr_cycles.git"
		$modules += "pr_curl:https://github.com/Silverlan/pr_curl.git"
		$modules += "pr_dmx:https://github.com/Silverlan/pr_dmx.git"
		$modules += "pr_xatlas:https://github.com/Silverlan/pr_xatlas.git"
	}
}

if($with_vr) {
	$modules += "pr_openvr:https://github.com/Silverlan/pr_openvr.git"
}

$moduleList=""
$global:cmakeArgs=""
foreach ( $module in $modules )
{
	$index=$module.IndexOf(":")
	$components=$module.Split(":")
	$moduleName=$module.SubString(0,$index)
	$moduleUrl=$module.SubString($index +1)
	$moduleDir="$PWD/$moduleName/"
	if(![System.IO.Directory]::Exists("$moduleDir")){
		print_hmsg "Downloading module '$moduleName'..."
		git clone "$moduleUrl" --recurse-submodules $moduleName
		validate_result
		print_hmsg "Done!"
	}
	else{
		print_hmsg "Updating module '$moduleName'..."
		git pull
		validate_result
		print_hmsg "Done!"
	}
	
	if([System.IO.File]::Exists("$moduleDir/build_scripts/setup_windows.ps1")){
		print_hmsg "Executing module setup script..."
		$curDir=$PWD
		& "$PWD/$moduleName/build_scripts/setup_windows.ps1"
		validate_result
		cd $curDir
		print_hmsg "Done!"
	}

	$moduleList += " "
	$moduleList += $moduleName
}

cd ..
print_hmsg "Done!"

#if($false){
# Configure
print_hmsg "Configuring Pragma..."
if(![System.IO.Directory]::Exists("$PWD/build")){
	mkdir build
}
$rootDir=$PWD
cd build
$installDir="$PWD/install"
print_hmsg "Additional CMake args: $cmakeArgs"

$cmdCmake="cmake .. -G `"$generator`" ```
	-DDEPENDENCY_BOOST_INCLUDE=`"$rootDir/deps/boost_1_78_0`" ```
	-DDEPENDENCY_BOOST_LIBRARY_LOCATION=`"$rootDir/deps/boost_1_78_0/stage/lib`" ```
	-DDEPENDENCY_BOOST_CHRONO_LIBRARY=`"$rootDir/deps/boost_1_78_0/stage/lib/boost_chrono-vc142-mt-x64-1_78.lib`" ```
	-DDEPENDENCY_BOOST_DATE_TIME_LIBRARY=`"$rootDir/deps/boost_1_78_0/stage/lib/boost_date_time-vc142-mt-x64-1_78.lib`" ```
	-DDEPENDENCY_BOOST_REGEX_LIBRARY=`"$rootDir/deps/boost_1_78_0/stage/lib/boost_regex-vc142-mt-x64-1_78.lib`" ```
	-DDEPENDENCY_BOOST_SYSTEM_LIBRARY=`"$rootDir/deps/boost_1_78_0/stage/lib/boost_system-vc142-mt-x64-1_78.lib`" ```
	-DDEPENDENCY_BOOST_THREAD_LIBRARY=`"$rootDir/deps/boost_1_78_0/stage/lib/boost_thread-vc142-mt-x64-1_78.lib`" ```
	-DDEPENDENCY_GEOMETRIC_TOOLS_INCLUDE=`"$rootDir/deps/GeometricTools/GTE`" ```
	-DDEPENDENCY_LIBZIP_CONF_INCLUDE=`"$rootDir/build/third_party_libs/libzip`" ```
	-DCMAKE_INSTALL_PREFIX:PATH=`"$installDir`" ```
"
$cmdCmake += $global:cmakeArgs

iex $cmdCmake
validate_result

print_hmsg "Done!"

print_hmsg "Build files have been written to \"$PWD/build\"."

$curDir=$PWD
if($with_pfm) {
	print_hmsg "Downloading PFM addon..."
	[System.IO.Directory]::CreateDirectory("$installDir/addons")
	cd "$installDir/addons"
	if(![System.IO.Directory]::Exists("$installDir/addons/pfm")){
		git clone "https://github.com/Silverlan/pfm.git"
		validate_result
	} else {
		print_hmsg "Updating PFM..."
		git pull
		validate_result
		print_hmsg "Done!"
	}
	print_hmsg "Done!"
}

if($with_vr) {
	print_hmsg "Downloading VR addon..."
	[System.IO.Directory]::CreateDirectory("$installDir/addons")
	cd "$installDir/addons"
	if(![System.IO.Directory]::Exists("$installDir/addons/virtual_reality")){
		git clone "https://github.com/Silverlan/PragmaVR.git" virtual_reality
		validate_result
	} else {
		print_hmsg "Updating VR..."
		git pull
		validate_result
		print_hmsg "Done!"
	}
	print_hmsg "Done!"
}
cd $curDir

if($build) {
	print_hmsg "Building Pragma..."
	$targets="pragma-install-full $moduleList"
	if($with_pfm) {
		$targets+=" pfm"
	}
	$targets+=" pragma-install"

	$cmakeBuild="cmake --build `".`" --config "$buildConfig" --target $targets"
	echo "Running build command:"
	echo "$cmakeBuild"
	iex $cmakeBuild
	validate_result

	print_hmsg "Build Successful! Pragma has been installed to `"$installDir`"."
	print_hmsg "If you make any changes to the core source code, you can build the `"pragma-install`" target to compile the changes and re-install the binaries automatically."
	print_hmsg "If you make any changes to a module, you will have to build the module target first, and then build `"pragma-install`"."
	print_hmsg ""
}

print_hmsg "All actions have been completed! Please make sure to re-run this script every time you pull any changes from the repository, and after adding any new modules."

cd $root
