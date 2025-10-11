// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"
#include "alsoundsystem.hpp"
#include "alsound_effect.hpp"

export module pragma.client:scripting.lua.libraries.sound;
export namespace Lua {
	namespace sound {
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxEaxReverbProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxChorusProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxDistortionProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxEchoProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxFlangerProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxFrequencyShifterProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxVocalMorpherProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxPitchShifterProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxRingModulatorProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxAutoWahProperties &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxCompressor &props);
		DLLCLIENT al::PEffect register_aux_effect(const std::string &name, const al::EfxEqualizer &props);

		DLLCLIENT al::DistanceModel get_aux_effect();
		DLLCLIENT void set_distance_model(al::DistanceModel distanceModel);
		DLLCLIENT al::DistanceModel get_distance_model();
		DLLCLIENT bool is_supported(al::ChannelConfig channels, al::SampleType type);

		DLLCLIENT float get_doppler_factor();
		DLLCLIENT void set_doppler_factor(float dopplerFactor);
		DLLCLIENT float get_speed_of_sound();
		DLLCLIENT void set_speed_of_sound(float speedOfSound);

		DLLCLIENT std::optional<std::string> get_device_name();
		DLLCLIENT bool add_global_effect(const std::string &name);
		DLLCLIENT bool add_global_effect(const std::string &name, al::ISoundSystem::GlobalEffectFlag flags, const al::EffectParams &params);
		DLLCLIENT void remove_global_effect(const std::string &name);
		DLLCLIENT void set_global_effect_parameters(const std::string &name, const al::EffectParams &params);
	};
};
