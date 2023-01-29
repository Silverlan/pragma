/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __SOUND_UTIL_HPP__
#define __SOUND_UTIL_HPP__

#include "pragma/networkdefinitions.h"
#include <string>

namespace sound {
	// Returns the full path, including extension (if the file was found)
	DLLNETWORK void get_full_sound_path(std::string &inOutPath, bool bPathRelativeToSounds = false);
};

#endif
