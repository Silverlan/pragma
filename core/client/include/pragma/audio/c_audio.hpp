// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_AUDIO_HPP__
#define __C_AUDIO_HPP__

#include "pragma/clientdefinitions.h"
#include <vector>
#include <string>

namespace pragma::audio {
	DLLCLIENT std::vector<std::string> get_available_audio_apis();
	DLLCLIENT std::string get_audio_api_module_location(const std::string &audioAPI);
};

#endif
