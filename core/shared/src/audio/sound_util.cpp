// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/audio/sound_util.hpp"
#include <sharedutils/util_file.h>

void sound::get_full_sound_path(std::string &inOutPath, bool bPathRelativeToSounds)
{
	std::string ext;
	if(ufile::get_extension(inOutPath, &ext) == true)
		return;
	const std::vector<std::string> supportedExtensions = engine_info::get_supported_audio_formats();
	for(auto &extChk : supportedExtensions) {
		auto extPath = inOutPath + '.' + extChk;
		if(bPathRelativeToSounds ? FileManager::Exists("sounds\\" + extPath) : FileManager::Exists(extPath)) {
			inOutPath = extPath;
			break;
		}
	}
}
