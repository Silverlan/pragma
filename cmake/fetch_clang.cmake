include("cmake/install_helper.cmake")

# We need the very latest version of clang, which is currently not available in any package managers yet, so we'll download it manually for now.
function(pr_fetch_clang)
	if(WIN32)
		# Note: clang-22 causes build errors on Windows.
		# TODO: Investigate this further once a stable version of clang-22 has been released.
		set(minimum_required_version "21.1.8")
		set(install_version "21.1.8")
	else()
		set(minimum_required_version "22.1.0")

		# Install the latest version if no supported version of clang was found (this should be updated from time to time)
		set(install_version "22.1.8")
	endif()

	find_program(CLANG_EXECUTABLE clang)
	if(CLANG_EXECUTABLE)
		execute_process(
			COMMAND ${CLANG_EXECUTABLE} --version
			OUTPUT_VARIABLE CLANG_VERSION_OUTPUT
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		if(CLANG_VERSION_OUTPUT MATCHES "version ([0-9]+)\\.([0-9]+)\\.([0-9]+)")
			set(installed_version "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
			if(installed_version VERSION_GREATER_EQUAL minimum_required_version)
				# Supported version of clang is already installed
				return()
			endif()
		endif()
	endif()

	if(WIN32)
		set(url "https://github.com/llvm/llvm-project/releases/download/llvmorg-${install_version}/clang+llvm-${install_version}-x86_64-pc-windows-msvc.tar.xz")
	else()
		set(url "https://github.com/llvm/llvm-project/releases/download/llvmorg-${install_version}/LLVM-${install_version}-Linux-X64.tar.xz")
	endif()

	check_content_version("${PRAGMA_BUILD_TOOLS_DIR}/clang" "${install_version}" "version.json" IS_VALID)
	if(NOT IS_VALID)
		clear_content("${PRAGMA_BUILD_TOOLS_DIR}/clang")
		pr_http_extract(
			"${url}"
			"${PRAGMA_BUILD_TOOLS_DIR}/clang"
			STRIP_TOP_DIR
		)
		update_content_version("${PRAGMA_BUILD_TOOLS_DIR}/clang" "${install_version}" "version.json")
	endif()

	set(use_custom_clang 0)
	if(UNIX)
		# Only clang is supported on linux right now, so we'll just always enable it
		set(use_custom_clang 1)
	else()
		# Only use clang if it was explicitely specified (e.g. through a preset), and only if no absolute path to clang was specified.
		# (i.e. "clang"/"clang.exe" and "clang++"/"clang++.exe")
		if(CMAKE_C_COMPILER MATCHES "^clang(\\.exe)?$" AND CMAKE_CXX_COMPILER MATCHES "^clang\\+\\+(\\.exe)?$")
			set(use_custom_clang 1)
		endif()
	endif()

	if(use_custom_clang)
		if(WIN32)
			set(CMAKE_C_COMPILER   "${PRAGMA_BUILD_TOOLS_DIR}/clang/bin/clang.exe"   CACHE FILEPATH "C compiler"   FORCE)
			set(CMAKE_CXX_COMPILER "${PRAGMA_BUILD_TOOLS_DIR}/clang/bin/clang++.exe" CACHE FILEPATH "C++ compiler" FORCE)
		else()
			set(CMAKE_C_COMPILER   "${PRAGMA_BUILD_TOOLS_DIR}/clang/bin/clang"   CACHE FILEPATH "C compiler"   FORCE)
			set(CMAKE_CXX_COMPILER "${PRAGMA_BUILD_TOOLS_DIR}/clang/bin/clang++" CACHE FILEPATH "C++ compiler" FORCE)

			set(CLANG_EXECUTABLE   "${PRAGMA_BUILD_TOOLS_DIR}/clang/bin/clang"   CACHE FILEPATH "clang executable"   FORCE)
			set(CMAKE_C_COMPILER_CLANG_SCAN_DEPS "${PRAGMA_BUILD_TOOLS_DIR}/clang/bin/clang-scan-deps" CACHE FILEPATH "Clang-scan-deps" FORCE)
			set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS "${PRAGMA_BUILD_TOOLS_DIR}/clang/bin/clang-scan-deps" CACHE FILEPATH "Clang-scan-deps" FORCE)
		endif()
	endif()
endfunction()
pr_fetch_clang()
