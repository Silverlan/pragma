// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.sound;
import :engine;

void Lua::sound::set_distance_model(pragma::audio::DistanceModel distanceModel)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	soundSys->SetDistanceModel(distanceModel);
}
pragma::audio::DistanceModel Lua::sound::get_distance_model()
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	auto distanceModel = pragma::audio::DistanceModel::LinearClamped;
	if(soundSys != nullptr)
		distanceModel = soundSys->GetDistanceModel();
	return distanceModel;
}

pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxEaxReverbProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxChorusProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxDistortionProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxEchoProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxFlangerProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxFrequencyShifterProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxVocalMorpherProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxPitchShifterProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxRingModulatorProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxAutoWahProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxCompressor &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
pragma::audio::PEffect Lua::sound::register_aux_effect(const std::string &name, const pragma::audio::EfxEqualizer &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }

pragma::audio::PEffect Lua::sound::get_aux_effect(const std::string &name)
{
	auto effect = pragma::get_cengine()->GetAuxEffect(name);
	if(effect == nullptr)
		return nullptr;
	return effect;
}

bool Lua::sound::is_supported(pragma::audio::ChannelConfig channels, pragma::audio::SampleType type)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return false;
	return soundSys->IsSupported(channels, type);
}

float Lua::sound::get_doppler_factor()
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(!soundSys)
		return 0.f;
	return soundSys->GetDopplerFactor();
}
void Lua::sound::set_doppler_factor(float dopplerFactor)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys != nullptr)
		soundSys->SetDopplerFactor(dopplerFactor);
}
float Lua::sound::get_speed_of_sound()
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(!soundSys)
		return 0.f;
	return soundSys->GetSpeedOfSound();
}
void Lua::sound::set_speed_of_sound(float speedOfSound)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys != nullptr)
		soundSys->SetSpeedOfSound(speedOfSound);
}
std::optional<std::string> Lua::sound::get_device_name()
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(!soundSys)
		return {};
	return soundSys->GetDeviceName();
}
bool Lua::sound::add_global_effect(const std::string &name, pragma::audio::ISoundSystem::GlobalEffectFlag flags, const pragma::audio::EffectParams &params)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return false;
	auto effect = pragma::get_cengine()->GetAuxEffect(name);
	if(effect == nullptr)
		return false;
	auto r = soundSys->AddGlobalEffect(*effect, flags, params);
	return (r != std::numeric_limits<uint32_t>::max()) ? true : false;
}
bool Lua::sound::add_global_effect(const std::string &name) { return add_global_effect(name, pragma::audio::ISoundSystem::GlobalEffectFlag::All, {}); }
void Lua::sound::remove_global_effect(const std::string &name)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto effect = pragma::get_cengine()->GetAuxEffect(name);
	if(effect == nullptr)
		return;
	soundSys->RemoveGlobalEffect(*effect);
}
void Lua::sound::set_global_effect_parameters(const std::string &name, const pragma::audio::EffectParams &params)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto effect = pragma::get_cengine()->GetAuxEffect(name);
	if(effect == nullptr)
		return;
	soundSys->SetGlobalEffectParameters(*effect, params);
}
