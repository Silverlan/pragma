// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include <cstdlib>

import pragma.updater;
import std;

static int update_failed()
{
	std::cout << "Press any key to exit..." << std::endl;
	std::cin.get();
	return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
	if(argc == 0)
		return EXIT_FAILURE;
	std::string path = argv[0];

	std::string executableName;
#ifdef _WIN32
	executableName = "pragma.exe";
#else
	executableName = "pragma";
#endif

	auto launchParams = pragma::util::get_launch_parameters(argc, argv);
	auto itExe = launchParams.find("-executable");
	if(itExe != launchParams.end())
		executableName = itExe->second;

	std::cout << "Waiting for " << executableName << " to close... " << std::endl;
	// Wait until pragma has been closed
	while(pragma::util::is_process_running(executableName.c_str()))
		std::this_thread::sleep_for(std::chrono::milliseconds(250));

	auto pos = path.find_last_of("\\/");
	if(pos != std::string::npos)
		path = path.substr(0, pos);

	pos = path.find_last_of("\\/");
	if(pos != std::string::npos)
		path = path.substr(0, pos);

	char pathSeparator;
#ifdef _WIN32
	pathSeparator = '\\';
#else
	pathSeparator = '/';
#endif

	std::string updatePath = path + pathSeparator + "update";
	if(!std::filesystem::is_directory(updatePath)) {
		std::cout << "Unable to find update folder '" << updatePath << "'!" << std::endl;
		return update_failed();
	}

	auto deleteFromUpdateFiles = [&updatePath, &pathSeparator](const std::string &fileName) {
		auto updaterPath = updatePath + pathSeparator + fileName;
		if(std::filesystem::exists(updaterPath)) {
			std::cout << "Deleting '" << updaterPath << "'..." << std::endl;
			std::filesystem::remove(updaterPath);
		}
	};

	// We can't replace the updater executable because it's currently running, so we'll just skip it.
#ifdef _WIN32
	deleteFromUpdateFiles(std::string {"bin"} + pathSeparator + "updater.exe");
#else
	//This shouldn't be the case here, as we can still replace an file during self-run.
	deleteFromUpdateFiles(std::string {"bin"} + pathSeparator + "updater");
#endif

	// The Ubuntu Mono font is used as the console font in Pragma. Unfortunately, once loaded, it cannot be unloaded until
	// the operating system is restarted, so we can't overwrite this file. Since this font is unlikely to change in an update,
	// we'll just ignore it by deleting it from the update files.
	deleteFromUpdateFiles(std::string {"fonts"} + pathSeparator + "ubuntu" + pathSeparator + "UbuntuMono-R.ttf");

	// Copy files from update folder to root folder
	auto pUpdatePath = std::filesystem::path(updatePath).lexically_normal();
	for(const auto &entry : std::filesystem::recursive_directory_iterator(updatePath)) {
		if(std::filesystem::is_regular_file(entry)) {
			auto fullSrcPath = entry.path().string();
			auto relPath = std::filesystem::path(fullSrcPath).lexically_normal().lexically_relative(pUpdatePath);

			auto filename = relPath.string();
			std::filesystem::path newPath = std::filesystem::path(path) / relPath;
			std::cout << "Copying '" << relPath << "'..." << std::endl;
			auto success = true;
			try {
				std::filesystem::create_directories(newPath.parent_path());
				success = std::filesystem::copy_file(fullSrcPath, newPath, std::filesystem::copy_options::overwrite_existing);
			}
			catch(const std::filesystem::filesystem_error &err) {
				std::cout << "Failed to copy file '" << relPath << "': " << err.what() << "!" << std::endl;
				return update_failed();
			}
			if(success == false) {
				std::cout << "Failed to copy file '" << relPath << "'!" << std::endl;
				return update_failed();
			}
		}
	}

	// Delete "update" directory
	std::cout << "Deleting '" << updatePath << "'..." << std::endl;
	std::filesystem::remove_all(updatePath);

	return EXIT_SUCCESS;
}
