// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

export module pragma.shared:audio.enums;

export {
	enum class ALState : uint32_t { NoError = 0, Initial = 0x1011, Playing = 0x1012, Paused = 0x1013, Stopped = 0x1014 };

	struct DLLNETWORK SoundCacheInfo {
		SoundCacheInfo() : duration(0.f), mono(false), stereo(false) {}
		float duration;
		bool mono;
		bool stereo;
	};

	enum class ALChannel : uint32_t { Auto = 0, Mono, Both };

	enum class ALCreateFlags : uint32_t {
		None = 0,
		Mono = 1,
		Stream = Mono << 1,
		DontTransmit = Stream << 1 // Serverside only
	};
	REGISTER_BASIC_BITWISE_OPERATORS(ALCreateFlags);

	enum class ALSoundType : Int32 {
		Generic = 0,
		Effect = 1,
		Music = Effect << 1,
		Voice = Music << 1,
		Weapon = Voice << 1,
		NPC = Weapon << 1,
		Player = NPC << 1,
		Vehicle = Player << 1,
		Physics = Vehicle << 1,
		Environment = Physics << 1,
		GUI = Environment << 1,

		Count,
		All = Effect | Music | Voice | Weapon | NPC | Player | Vehicle | Physics | Environment | GUI
	};
	REGISTER_BASIC_BITWISE_OPERATORS(ALSoundType)
};
