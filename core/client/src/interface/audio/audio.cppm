// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <vector>
#include <string>

export module pragma.client:audio;

export import :audio.aux_util;
export import :audio.sound;
export import :audio.sound_script;

export namespace pragma::audio {
	DLLCLIENT std::vector<std::string> get_available_audio_apis();
	DLLCLIENT std::string get_audio_api_module_location(const std::string &audioAPI);
};
