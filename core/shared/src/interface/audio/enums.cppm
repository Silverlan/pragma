// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:audio.enums;

export import pragma.math;

export namespace pragma::audio {
	class ALSound;
	using ALSoundRef = std::reference_wrapper<ALSound>;

	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_MAX_DISTANCE = std::numeric_limits<float>::max();

	CONSTEXPR_DLL_COMPAT bool ALSOUND_DEFAULT_RELATIVE = false;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_REFERENCE_DISTANCE = 1.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_ROLLOFF_FACTOR = 1.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_ROOM_ROLLOFF_FACTOR = 0.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_MIN_GAIN = 0.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_MAX_GAIN = 1.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_CONE_INNER_ANGLE = 360.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_CONE_OUTER_ANGLE = 360.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_CONE_OUTER_GAIN = 0.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_CONE_OUTER_GAIN_HF = 1.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_PRIORITY = 0u;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_DOPPLER_FACTOR = 1.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_AIR_ABSORPTION_FACTOR = 0.f;
	CONSTEXPR_DLL_COMPAT Vector3 ALSOUND_DEFAULT_ORIENTATION_AT = Vector3 {0.f, 0.f, -1.f};
	CONSTEXPR_DLL_COMPAT Vector3 ALSOUND_DEFAULT_ORIENTATION_UP = Vector3 {0.f, 1.f, 0.f};
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_STEREO_ANGLE_LEFT = 0.523599;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_STEREO_ANGLE_RIGHT = -0.523599;
	CONSTEXPR_DLL_COMPAT bool ALSOUND_DEFAULT_DIRECT_GAIN_HF_AUTO = true;
	CONSTEXPR_DLL_COMPAT bool ALSOUND_DEFAULT_SEND_GAIN_AUTO = true;
	CONSTEXPR_DLL_COMPAT bool ALSOUND_DEFAULT_SEND_GAIN_HF_AUTO = true;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_DIRECT_FILTER_GAIN = 1.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_DIRECT_FILTER_GAIN_HF = 1.f;
	CONSTEXPR_DLL_COMPAT float ALSOUND_DEFAULT_DIRECT_FILTER_GAIN_LF = 1.f;

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
	using namespace pragma::math::scoped_enum::bitwise;
};

export {
	REGISTER_ENUM_FLAGS(pragma::audio::ALCreateFlags)
	REGISTER_ENUM_FLAGS(pragma::audio::ALSoundType)
}
