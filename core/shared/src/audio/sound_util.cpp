/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
