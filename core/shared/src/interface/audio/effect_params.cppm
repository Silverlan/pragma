// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:audio.effect_params;

export namespace pragma::audio {
	struct DLLNETWORK SoundEffectParams {
		SoundEffectParams(float gain = 1.f, float gainHF = 1.f, float gainLF = 1.f);
		float gain = 1.f;
		float gainHF = 1.f; // For low-pass and band-pass filters
		float gainLF = 1.f; // For high-pass and band-pass filters
	};
};
