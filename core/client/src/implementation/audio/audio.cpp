// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <fsys/filesystem.h>

module pragma.client.audio;

std::vector<std::string> pragma::audio::get_available_audio_apis()
{
	std::vector<std::string> dirs {};
	FileManager::FindFiles("modules/audio/*", nullptr, &dirs,
	  fsys::SearchFlags::Local // Binary modules can only be loaded from actual files
	);
	return dirs;
}
std::string pragma::audio::get_audio_api_module_location(const std::string &audioAPI) { return "audio/" + audioAPI + "/pr_audio_" + audioAPI; }
