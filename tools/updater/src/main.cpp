/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#define MUTIL_STATIC

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <filesystem>
#include <sharedutils/util.h>
#include <sharedutils/util_path.hpp>

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

	auto launchParams = util::get_launch_parameters(argc, argv);
	auto itExe = launchParams.find("-executable");
	if(itExe != launchParams.end())
		executableName = itExe->second;

	std::cout << "Waiting for " << executableName << " to close... " << std::endl;
	// Wait until pragma has been closed
	while(util::is_process_running(executableName.c_str()))
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
	auto pUpdatePath = util::Path::CreatePath(updatePath);
	for(const auto &entry : std::filesystem::recursive_directory_iterator(updatePath)) {
		if(std::filesystem::is_regular_file(entry)) {
			auto fullSrcPath = entry.path().string();
			auto relPath = util::Path::CreateFile(fullSrcPath);
			relPath.MakeRelative(pUpdatePath);

			auto filename = relPath.GetString();
			auto newPath = path + pathSeparator + filename;
			std::cout << "Copying '" << relPath << "'..." << std::endl;
			auto success = true;
			try {
				std::filesystem::create_directories(ufile::get_path_from_filename(newPath));
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
