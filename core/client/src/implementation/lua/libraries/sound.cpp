// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"


#include "stdafx_client.h"

module pragma.client;


import :scripting.lua.libraries.sound;
import :engine;

void Lua::sound::set_distance_model(al::DistanceModel distanceModel)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	soundSys->SetDistanceModel(distanceModel);
}
al::DistanceModel Lua::sound::get_distance_model()
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	auto distanceModel = al::DistanceModel::LinearClamped;
	if(soundSys != nullptr)
		distanceModel = soundSys->GetDistanceModel();
	return distanceModel;
}

al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxEaxReverbProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxChorusProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxDistortionProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxEchoProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxFlangerProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxFrequencyShifterProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxVocalMorpherProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxPitchShifterProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxRingModulatorProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxAutoWahProperties &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxCompressor &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }
al::PEffect Lua::sound::register_aux_effect(const std::string &name, const al::EfxEqualizer &props) { return pragma::get_cengine()->CreateAuxEffect(name, props); }

al::PEffect Lua::sound::get_aux_effect(const std::string &name)
{
	auto effect = pragma::get_cengine()->GetAuxEffect(name);
	if(effect == nullptr)
		return nullptr;
	return effect;
}

bool Lua::sound::is_supported(al::ChannelConfig channels, al::SampleType type)
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
bool Lua::sound::add_global_effect(const std::string &name, al::ISoundSystem::GlobalEffectFlag flags, const al::EffectParams &params)
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
bool Lua::sound::add_global_effect(const std::string &name)
{
	return add_global_effect(name, al::ISoundSystem::GlobalEffectFlag::All, {});
}
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
void Lua::sound::set_global_effect_parameters(const std::string &name, const al::EffectParams &params)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto effect = pragma::get_cengine()->GetAuxEffect(name);
	if(effect == nullptr)
		return;
	soundSys->SetGlobalEffectParameters(*effect, params);
}
