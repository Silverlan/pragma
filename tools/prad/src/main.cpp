/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <filesystem>
#include <sharedutils/util.h>
#include <sharedutils/util_path.hpp>
#include <sharedutils/util_string.h>
#include <sharedutils/util_command_manager.hpp>
#include <sharedutils/scope_guard.h>
#include <udm.hpp>

static bool launch_child_console_process(const char *childProcess, const char *args)
{
	std::string fullCmd = std::string("\"") + childProcess + "\" " + args;
#ifdef _WIN32
	// Initialize the structures for process and startup information
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&sa, sizeof(sa));

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	// Create pipes for standard output and standard error
	HANDLE hStdoutRead, hStdoutWrite;
	HANDLE hStderrRead, hStderrWrite;

	if(!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0)) {
		std::cerr << "Failed to create stdout pipe (" << GetLastError() << ")." << std::endl;
		return false;
	}

	util::ScopeGuard sgStdoutRead {[hStdoutRead]() { CloseHandle(hStdoutRead); }};
	util::ScopeGuard sgStdoutWrite {[hStdoutWrite]() { CloseHandle(hStdoutWrite); }};

	if(!CreatePipe(&hStderrRead, &hStderrWrite, &sa, 0)) {
		std::cerr << "Failed to create stderr pipe (" << GetLastError() << ")." << std::endl;
		return false;
	}

	util::ScopeGuard sgStderrRead {[hStderrRead]() { CloseHandle(hStderrRead); }};
	util::ScopeGuard sgStderrWrite {[hStderrWrite]() { CloseHandle(hStderrWrite); }};

	// Ensure the write handle to the pipe for stdout is not inherited.
	if(!SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0)) {
		std::cerr << "Failed to set handle information for stdout (" << GetLastError() << ")." << std::endl;
		return false;
	}

	if(!SetHandleInformation(hStderrRead, HANDLE_FLAG_INHERIT, 0)) {
		std::cerr << "Failed to set handle information for stderr (" << GetLastError() << ")." << std::endl;
		return false;
	}

	// Redirect the child process's stdout and stderr to the write end of the pipes
	si.hStdOutput = hStdoutWrite;
	si.hStdError = hStderrWrite;
	si.dwFlags |= STARTF_USESTDHANDLES;

	char *cmdLine = const_cast<char *>(fullCmd.c_str());
	if(!CreateProcessA(NULL, // No module name (use command line)
	     cmdLine,            // Command line
	     NULL,               // Process handle not inheritable
	     NULL,               // Thread handle not inheritable
	     TRUE,               // Set handle inheritance to TRUE
	     CREATE_NO_WINDOW,   // No creation flags
	     NULL,               // Use parent's environment block
	     NULL,               // Use parent's starting directory
	     &si,                // Pointer to STARTUPINFO structure
	     &pi)                // Pointer to PROCESS_INFORMATION structure
	) {
		std::cerr << "Failed to create process: " << GetLastError() << std::endl;
		return false;
	}

	util::ScopeGuard sgProcess {[&pi]() {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}};

	sgStdoutWrite.release();
	sgStderrWrite.release();

	// Read output from the child process and write to std::cout and std::cerr
	char buffer[4096];
	DWORD bytesRead;
	BOOL success;

	// Read from stdout
	while((success = ReadFile(hStdoutRead, buffer, sizeof(buffer), &bytesRead, NULL)) && bytesRead > 0)
		std::cout.write(buffer, bytesRead);

	// Read from stderr
	while((success = ReadFile(hStderrRead, buffer, sizeof(buffer), &bytesRead, NULL)) && bytesRead > 0)
		std::cerr.write(buffer, bytesRead);

	// Wait until child process exits
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Get the exit code of the process
	DWORD exitCode;
	if(!GetExitCodeProcess(pi.hProcess, &exitCode)) {
		// If GetExitCodeProcess fails, output an error message and return
		std::cerr << "GetExitCodeProcess failed (" << GetLastError() << ")." << std::endl;
		return false;
	}

	return true;
#else
	std::vector<std::string> argv;
	ustring::explode(args, " ", argv);
	if(!util::start_process(childProcess, argv, true)) {
		std::cerr << "Command '" << fullCmd << "' has failed!" << std::endl;
		return false;
	}
	return true;
#endif
}

int main(int argc, char *argv[])
{
	/*std::cout << "argc: " << argc << std::endl;
	for(int i = 0; i < argc; ++i) {
		std::cout << i << ": " << argv[i] << std::endl;
	}

	if(true)
		return EXIT_SUCCESS;*/

	std::cout << "Running prad.exe..." << std::endl;

	if(argc <= 1) {
		std::cout << "Insufficient number of arguments supplied!" << std::endl;
		return EXIT_FAILURE;
	}

	std::optional<uint32_t> width {};
	std::optional<uint32_t> height {};
	std::optional<uint32_t> samples {};

	auto checkArg = [argc, argv](size_t &i) -> std::optional<std::string> {
		std::optional<std::string> val {};
		if(i < argc - 1) // -1 because last argument is reserved for map name
			val = argv[i];
		++i;
		return val;
	};

	auto fastRender = false;
	// TODO: Add parameter for re-using previous lightmaps
	size_t i = 1;
	while(i < argc - 1) {
		std::string arg = argv[i];
		++i;
		if(arg == "-game") {
			++i; // Skip next argument
			continue;
		}
		if(arg == "-noextra" || arg == "-fast") {
			fastRender = true;
			continue;
		}
		if(arg == "-width") {
			auto value = checkArg(i);
			if(value)
				width = util::to_int(*value);
			continue;
		}
		if(arg == "-height") {
			auto value = checkArg(i);
			if(value)
				height = util::to_int(*value);
			continue;
		}
		if(arg == "-samples") {
			auto value = checkArg(i);
			if(value)
				samples = util::to_int(*value);
			continue;
		}
	}

	if(fastRender)
		std::cout << "Fast render mode enabled, lightmaps will be low quality. Do not use for production!" << std::endl;

	if(!width || !height) {
		width = fastRender ? 512 : 2'048;
		height = width;
	}
	if(!samples)
		samples = fastRender ? 200 : 20'000;

	std::cout << "Using resolution " << *width << "x" << *height << std::endl;
	std::cout << "Using sample count " << *samples << std::endl;

	std::string mapPath = argv[argc - 1];
	std::cout << "Map Path: " << mapPath << std::endl;
	auto bspPath = mapPath + ".bsp";
	if(filemanager::is_system_file(bspPath) == false) {
		std::cerr << "BSP File '" << bspPath << "' not found!" << std::endl;
		return EXIT_FAILURE;
	}

	auto pragmaPath = util::Path::CreatePath(util::get_program_path());
	pragmaPath.PopBack();
	std::cout << "Pragma installation path: " << pragmaPath.GetString() << std::endl;
	auto rootPath = pragmaPath.GetString();
	rootPath = rootPath.substr(0, rootPath.length() - 1);

	auto importAddonPath = pragmaPath + "addons/imported/maps/";
	if(filemanager::create_system_path(rootPath, "addons/imported/maps/") == false) {
		std::cerr << "Failed to create addon path '" << importAddonPath.GetString() << "'!" << std::endl;
		return EXIT_FAILURE;
	}

	auto convertedAddonPath = pragmaPath + "addons/converted/maps/";
	if(filemanager::create_system_path(rootPath, "addons/converted/maps/") == false) {
		std::cerr << "Failed to create addon path '" << convertedAddonPath.GetString() << "'!" << std::endl;
		return EXIT_FAILURE;
	}

	auto mapName = ufile::get_file_from_filename(mapPath);
	std::cout << "Map Name: " << mapName << std::endl;
	if(mapName.empty()) {
		std::cerr << "Map name is empty! This is not allowed!" << std::endl;
		return EXIT_FAILURE;
	}

	auto outputFileName = convertedAddonPath.GetString() + mapName + ".pmap_b";
	std::cout << "Output file path: " << outputFileName << std::endl;
	if(filemanager::exists_system(outputFileName)) {
		std::cout << "Map already exists at target location. Deleting existing file..." << std::endl;
		if(filemanager::remove_system_file(outputFileName) == false) {
			std::cerr << "Failed to delete file '" << outputFileName << "'! Aborting..." << std::endl;
			return EXIT_FAILURE;
		}
	}

	auto importPath = importAddonPath.GetString() + mapName + ".bsp";
	std::cout << "Copying BSP from '" << bspPath << "' to '" << importPath << "'..." << std::endl;
	if(filemanager::copy_system_file(bspPath, importPath) == false) {
		std::cerr << "Failed to copy BSP file! Aborting..." << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Launching Pragma..." << std::endl;

#ifdef _WIN32
	std::string exeName = "pragma.com";
#else
	std::string exeName = "pragma";
#endif;
	std::string exePath = rootPath + "/" + exeName;

	std::stringstream bakeArgs;
	bakeArgs << "-shutdown";
	bakeArgs << " -width " << *width;
	bakeArgs << " -height " << *height;
	bakeArgs << " -samples " << *samples;

	std::string args = exePath + " -windowless -disable_ansi_color_codes -log 2 2 -log_file \"log_prad.txt\" -non_interactive +map " + mapName + std::string {" +\"map_bake_lightmaps " + bakeArgs.str() + "\""};
	std::cout << "------------ PRAGMA LOG START ------------" << std::endl;
	auto res = launch_child_console_process(exePath.c_str(), args.c_str());
	std::cout << "------------  PRAGMA LOG END  ------------" << std::endl;
	if(res == false) {
		std::cerr << "Lightmap baking has FAILED!" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Lightmap baking completed successfully!" << std::endl;
	return EXIT_SUCCESS;
}
