// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.sound;

export import pragma.soundsystem;

export namespace Lua {
	namespace sound {
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxEaxReverbProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxChorusProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxDistortionProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxEchoProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxFlangerProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxFrequencyShifterProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxVocalMorpherProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxPitchShifterProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxRingModulatorProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxAutoWahProperties &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxCompressor &props);
		DLLCLIENT pragma::audio::PEffect register_aux_effect(const std::string &name, const pragma::audio::EfxEqualizer &props);

		DLLCLIENT pragma::audio::PEffect get_aux_effect(const std::string &name);
		DLLCLIENT void set_distance_model(pragma::audio::DistanceModel distanceModel);
		DLLCLIENT pragma::audio::DistanceModel get_distance_model();
		DLLCLIENT bool is_supported(pragma::audio::ChannelConfig channels, pragma::audio::SampleType type);

		DLLCLIENT float get_doppler_factor();
		DLLCLIENT void set_doppler_factor(float dopplerFactor);
		DLLCLIENT float get_speed_of_sound();
		DLLCLIENT void set_speed_of_sound(float speedOfSound);

		DLLCLIENT std::optional<std::string> get_device_name();
		DLLCLIENT bool add_global_effect(const std::string &name);
		DLLCLIENT bool add_global_effect(const std::string &name, pragma::audio::ISoundSystem::GlobalEffectFlag flags, const pragma::audio::EffectParams &params);
		DLLCLIENT void remove_global_effect(const std::string &name);
		DLLCLIENT void set_global_effect_parameters(const std::string &name, const pragma::audio::EffectParams &params);
	};
};
