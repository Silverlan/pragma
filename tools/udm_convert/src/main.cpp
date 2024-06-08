/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#define MUTIL_STATIC
#define SHUTIL_STATIC

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <filesystem>
#include <sharedutils/util.h>
#include <sharedutils/util_path.hpp>
#include <sharedutils/util_string.h>
#include <udm.hpp>

int main(int argc, char *argv[])
{
	if(argc <= 1)
		return EXIT_FAILURE;
	std::string fileName = argv[1];

	auto saveFlags = udm::AsciiSaveFlags::IncludeHeader;
	if(argc > 2) {
		auto launchParams = util::get_launch_parameters(argc, argv);
		auto itFileName = launchParams.find("-file");
		if(itFileName != launchParams.end())
			fileName = itFileName->second;
		auto checkOption = [&saveFlags, &launchParams](const std::string &name, udm::AsciiSaveFlags flag) {
			auto it = launchParams.find("-" + name);
			if(it == launchParams.end())
				return;
			auto val = util::to_boolean(it->second);
			umath::set_flag(saveFlags, flag, val);
		};
		checkOption("uncompressed", udm::AsciiSaveFlags::DontCompressLz4Arrays);
		checkOption("include_header", udm::AsciiSaveFlags::IncludeHeader);
	}

	auto rootPath = ufile::get_path_from_filename(fileName);
	filemanager::set_absolute_root_path(rootPath);

	auto relFilePath = util::Path::CreateFile(fileName);
	relFilePath.MakeRelative(rootPath);

	fileName = relFilePath.GetString();

	std::string err;
	auto formatType = udm::Data::GetFormatType(fileName, err);
	if(!formatType) {
		std::cout << "Failed to determine format type for '" << fileName << "': " << err << std::endl;
		return EXIT_FAILURE;
	}

	std::shared_ptr<udm::Data> udmData;
	try {
		udmData = udm::Data::Load(fileName);
	}
	catch(const udm::Exception &e) {
		std::cout << "Failed to load file '" << fileName << "': " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	if(!udmData) {
		std::cout << "Failed to load file '" << fileName << "': Unknown error!" << std::endl;
		return EXIT_FAILURE;
	}

	auto fileNameWithoutExt = fileName;

	auto ext = ufile::remove_extension_from_filename(fileNameWithoutExt);
	if(!ext)
		ext = "udm";

	auto targetFormatType = (*formatType == udm::FormatType::Ascii) ? udm::FormatType::Binary : udm::FormatType::Ascii;
	switch(targetFormatType) {
	case udm::FormatType::Ascii:
		if(ext->length() >= 2 && ustring::substr(*ext, ext->length() - 2) == "_b")
			ext = ustring::substr(*ext, 0, ext->length() - 2);
		break;
	case udm::FormatType::Binary:
		if(ext->length() < 2 || ustring::substr(*ext, ext->length() - 2) != "_b")
			*ext += "_b";
		break;
	}

	auto newFileName = fileNameWithoutExt + '.' + *ext;
	auto success = false;
	switch(targetFormatType) {
	case udm::FormatType::Ascii:
		success = udmData->SaveAscii(newFileName, saveFlags);
		break;
	case udm::FormatType::Binary:
		success = udmData->Save(newFileName);
		break;
	}
	if(!success) {
		std::cout << "Failed to save output file '" << newFileName << "'!" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
