// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include <alsoundsystem.hpp>
#include <alsoundsystem_create_effect.hpp>
#include "pragma/game/c_game.h"
#include "pragma/lua/libraries/c_lsound.h"
#include "pragma/audio/c_engine_sound.hpp"
#include "luasystem.h"
#include <alsoundsystem.hpp>

extern DLLCLIENT CEngine *c_engine;

lua_registercheck(ALEfxEaxReverbProperties, al::EfxEaxReverbProperties);
lua_registercheck(ALEfxChorusProperties, al::EfxChorusProperties);
lua_registercheck(ALEfxDistortionProperties, al::EfxDistortionProperties);
lua_registercheck(ALEfxEchoProperties, al::EfxEchoProperties);
lua_registercheck(ALEfxFlangerProperties, al::EfxFlangerProperties);
lua_registercheck(ALEfxFrequencyShifterProperties, al::EfxFrequencyShifterProperties);
lua_registercheck(ALEfxVocalMorpherProperties, al::EfxVocalMorpherProperties);
lua_registercheck(ALEfxPitchShifterProperties, al::EfxPitchShifterProperties);
lua_registercheck(ALEfxRingModulatorProperties, al::EfxRingModulatorProperties);
lua_registercheck(ALEfxAutoWahProperties, al::EfxAutoWahProperties);
lua_registercheck(ALEfxCompressor, al::EfxCompressor);
lua_registercheck(ALEfxEqualizer, al::EfxEqualizer);

int Lua::sound::set_distance_model(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return 0;
	auto distanceModel = Lua::CheckInt(l, 1);
	soundSys->SetDistanceModel(static_cast<al::DistanceModel>(distanceModel));
	return 0;
}
int Lua::sound::get_distance_model(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	auto distanceModel = al::DistanceModel::LinearClamped;
	if(soundSys != nullptr)
		distanceModel = soundSys->GetDistanceModel();
	Lua::PushInt(l, umath::to_integral(distanceModel));
	return 1;
}

int Lua::sound::register_aux_effect(lua_State *l)
{
	auto *name = Lua::CheckString(l, 1);
	al::PEffect effect = nullptr;
	if(Lua::IsALEfxEaxReverbProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxEaxReverbProperties(l, 2));
	else if(Lua::IsALEfxChorusProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxChorusProperties(l, 2));
	else if(Lua::IsALEfxDistortionProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxDistortionProperties(l, 2));
	else if(Lua::IsALEfxEchoProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxEchoProperties(l, 2));
	else if(Lua::IsALEfxFlangerProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxFlangerProperties(l, 2));
	else if(Lua::IsALEfxFrequencyShifterProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxFrequencyShifterProperties(l, 2));
	else if(Lua::IsALEfxVocalMorpherProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxVocalMorpherProperties(l, 2));
	else if(Lua::IsALEfxPitchShifterProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxPitchShifterProperties(l, 2));
	else if(Lua::IsALEfxRingModulatorProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxRingModulatorProperties(l, 2));
	else if(Lua::IsALEfxAutoWahProperties(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxAutoWahProperties(l, 2));
	else if(Lua::IsALEfxCompressor(l, 2))
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxCompressor(l, 2));
	else
		effect = c_engine->CreateAuxEffect(name, *Lua::CheckALEfxEqualizer(l, 2));
	if(effect == nullptr)
		return 0;
	Lua::Push<al::PEffect>(l, effect);
	return 1;
}

int Lua::sound::get_aux_effect(lua_State *l)
{
	auto *name = Lua::CheckString(l, 1);
	auto effect = c_engine->GetAuxEffect(name);
	if(effect == nullptr)
		return 0;
	Lua::Push<al::PEffect>(l, effect);
	return 1;
}

int Lua::sound::is_supported(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		Lua::PushBool(l, false);
	else {
		auto channels = static_cast<al::ChannelConfig>(Lua::CheckInt(l, 1));
		auto type = static_cast<al::SampleType>(Lua::CheckInt(l, 2));
		Lua::PushBool(l, soundSys->IsSupported(channels, type));
	}
	return 1;
}

int Lua::sound::get_doppler_factor(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	Lua::PushNumber(l, (soundSys != nullptr) ? soundSys->GetDopplerFactor() : 0.f);
	return 1;
}
int Lua::sound::set_doppler_factor(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys != nullptr)
		soundSys->SetDopplerFactor(Lua::CheckNumber(l, 1));
	return 0;
}
int Lua::sound::get_speed_of_sound(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	Lua::PushNumber(l, (soundSys != nullptr) ? soundSys->GetSpeedOfSound() : 0.f);
	return 1;
}
int Lua::sound::set_speed_of_sound(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys != nullptr)
		soundSys->SetSpeedOfSound(Lua::CheckNumber(l, 1));
	return 0;
}
int Lua::sound::get_device_name(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	Lua::PushString(l, (soundSys != nullptr) ? soundSys->GetDeviceName() : "");
	return 1;
}
int Lua::sound::add_global_effect(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		Lua::PushBool(l, false);
	else {
		int32_t argId = 1;
		auto *name = Lua::CheckString(l, argId++);
		auto effect = c_engine->GetAuxEffect(name);
		if(effect == nullptr)
			Lua::PushBool(l, false);
		else {
			auto flags = al::ISoundSystem::GlobalEffectFlag::All;
			if(Lua::IsSet(l, argId))
				flags = static_cast<al::ISoundSystem::GlobalEffectFlag>(Lua::CheckInt(l, argId++));

			al::EffectParams params {};
			if(Lua::IsSet(l, argId))
				params.gain = Lua::CheckNumber(l, argId++);
			if(Lua::IsSet(l, argId))
				params.gainHF = Lua::CheckNumber(l, argId++);
			if(Lua::IsSet(l, argId))
				params.gainLF = Lua::CheckNumber(l, argId++);
			auto r = soundSys->AddGlobalEffect(*effect, flags, params);
			Lua::PushBool(l, (r != std::numeric_limits<uint32_t>::max()) ? true : false);
		}
	}
	return 1;
}
int Lua::sound::remove_global_effect(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return 0;
	auto *name = Lua::CheckString(l, 1);
	auto effect = c_engine->GetAuxEffect(name);
	if(effect == nullptr)
		return 0;
	soundSys->RemoveGlobalEffect(*effect);
	return 0;
}
int Lua::sound::set_global_effect_parameters(lua_State *l)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return 0;
	int32_t argId = 1;
	auto *name = Lua::CheckString(l, argId++);
	auto effect = c_engine->GetAuxEffect(name);
	if(effect == nullptr)
		return 0;
	al::EffectParams params {};
	if(Lua::IsSet(l, argId))
		params.gain = Lua::CheckNumber(l, argId++);
	if(Lua::IsSet(l, argId))
		params.gainHF = Lua::CheckNumber(l, argId++);
	if(Lua::IsSet(l, argId))
		params.gainLF = Lua::CheckNumber(l, argId++);
	soundSys->SetGlobalEffectParameters(*effect, params);
	return 0;
}
