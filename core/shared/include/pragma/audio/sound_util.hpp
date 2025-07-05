// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __SOUND_UTIL_HPP__
#define __SOUND_UTIL_HPP__

#include "pragma/networkdefinitions.h"
#include <string>

namespace sound {
	// Returns the full path, including extension (if the file was found)
	DLLNETWORK void get_full_sound_path(std::string &inOutPath, bool bPathRelativeToSounds = false);
};

#endif
