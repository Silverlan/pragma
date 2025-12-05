// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:audio.util;

export import std;

export namespace pragma::audio {
	// Returns the full path, including extension (if the file was found)
	DLLNETWORK void get_full_sound_path(std::string &inOutPath, bool bPathRelativeToSounds = false);
};
