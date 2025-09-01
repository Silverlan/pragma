// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module pragma.client.audio;
export import :sound_script;

export namespace pragma::audio {
	DLLCLIENT std::vector<std::string> get_available_audio_apis();
	DLLCLIENT std::string get_audio_api_module_location(const std::string &audioAPI);
};
