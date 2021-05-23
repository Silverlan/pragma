/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_AUDIO_HPP__
#define __C_AUDIO_HPP__

#include "pragma/clientdefinitions.h"
#include <vector>
#include <string>

namespace pragma::audio
{
	DLLCLIENT std::vector<std::string> get_available_audio_apis();
	DLLCLIENT std::string get_audio_api_module_location(const std::string &audioAPI);
};

#endif
