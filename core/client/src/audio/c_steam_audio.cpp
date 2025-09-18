// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include <alsoundsystem.hpp>
#include <steam_audio/alsound_steam_audio.hpp>
#include <pragma/console/convars.h>

import pragma.client.client_state;
import pragma.client.engine;
import pragma.client.game;

extern CEngine *c_engine;
extern CGame *c_game;

static void reload_sound_cache()
{
	if(c_game == nullptr)
		return;
	static CallbackHandle hCacheCallback = {};
	if(hCacheCallback.IsValid())
		hCacheCallback.Remove();
	// Delay cache reloading to next frame, in case multiple steam audio commands have been executed in rapid succession
	hCacheCallback = c_game->AddCallback("Think", FunctionCallback<void>::Create([]() {
		c_game->ReloadSoundCache();
		if(hCacheCallback.IsValid())
			hCacheCallback.Remove();
	}));
}

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_number_of_rays, [](NetworkState *state, ConVar *, int32_t, int32_t val) { reload_sound_cache(); });
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_number_of_diffuse_samples, [](NetworkState *state, ConVar *, int32_t, int32_t val) { reload_sound_cache(); });
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_number_of_bounces, [](NetworkState *state, ConVar *, int32_t, int32_t val) { reload_sound_cache(); });
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_ir_duration, [](NetworkState *state, ConVar *, float, float val) { reload_sound_cache(); });
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_ambisonics_order, [](NetworkState *state, ConVar *, int32_t, int32_t val) { reload_sound_cache(); });

static void cl_steam_audio_enabled(NetworkState *state, ConVar *, bool, bool val)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	if(val == false && c_game != nullptr)
		c_game->ClearSoundCache();

	soundSys->SetSteamAudioSpatializerEnabled(c_engine->GetConVarBool("cl_steam_audio_spatialize_enabled"));
	soundSys->SetSteamAudioReverbEnabled(c_engine->GetConVarBool("cl_steam_audio_reverb_enabled"));

	soundSys->SetSteamAudioEnabled(val);

	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.directBinaural = c_engine->GetConVarBool("cl_steam_audio_spatialize_direct_binaural");
	props.spatializer.HRTFInterpolation = static_cast<al::steam_audio::SpatializerInterpolation>(c_engine->GetConVarInt("cl_steam_audio_spatialize_hrtf_interpolation"));
	props.spatializer.distanceAttenuation = c_engine->GetConVarBool("cl_steam_audio_spatialize_distance_attenuation");
	props.spatializer.airAbsorption = c_engine->GetConVarBool("cl_steam_audio_spatialize_air_absorption");
	props.spatializer.occlusionMode = static_cast<al::steam_audio::SpatializerOcclusionMode>(c_engine->GetConVarInt("cl_steam_audio_spatialize_occlusion_mode"));
	props.spatializer.occlusionMethod = static_cast<al::steam_audio::OcclusionMethod>(c_engine->GetConVarInt("cl_steam_audio_spatialize_occlusion_method"));
	props.spatializer.directLevel = c_engine->GetConVarFloat("cl_steam_audio_spatialize_direct_level");
	props.spatializer.indirect = c_engine->GetConVarBool("cl_steam_audio_spatialize_indirect");
	props.spatializer.indirectBinaural = c_engine->GetConVarBool("cl_steam_audio_spatialize_indirect_binaural");
	props.spatializer.indirectLevel = c_engine->GetConVarFloat("cl_steam_audio_spatialize_indirect_level");
	props.spatializer.simulationType = static_cast<al::steam_audio::SimulationType>(c_engine->GetConVarInt("cl_steam_audio_spatialize_simulation_type"));
	props.spatializer.staticListener = c_engine->GetConVarBool("cl_steam_audio_spatialize_static_listener");

	props.reverb.indirectBinaural = c_engine->GetConVarBool("cl_steam_audio_reverb_indirect_binaural");
	props.reverb.simulationType = static_cast<al::steam_audio::SimulationType>(c_engine->GetConVarInt("cl_steam_audio_reverb_simulation_type"));

	if(val == true)
		reload_sound_cache();
}
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_enabled, cl_steam_audio_enabled);

REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_enabled, [](NetworkState *state, ConVar *, bool, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	//soundSys->SetSteamAudioSpatializerDSPEnabled(val);
});

REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_reverb_enabled, [](NetworkState *state, ConVar *, bool, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	//soundSys->SetSteamAudioReverbDSPEnabled(val);
});

REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_propagation_delay_enabled, [](NetworkState *state, ConVar *, bool, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto *iplScene = soundSys->GetSteamAudioScene();
	iplScene->SetPropagationDelayEnabled(val);
});

REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_direct_binaural, [](NetworkState *state, ConVar *, bool oldVal, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.directBinaural = val;
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_hrtf_interpolation, [](NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.HRTFInterpolation = static_cast<al::steam_audio::SpatializerInterpolation>(val);
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_distance_attenuation, [](NetworkState *state, ConVar *, bool oldVal, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.distanceAttenuation = val;
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_air_absorption, [](NetworkState *state, ConVar *, bool oldVal, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.airAbsorption = val;
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_occlusion_mode, [](NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.occlusionMode = static_cast<al::steam_audio::SpatializerOcclusionMode>(val);
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_occlusion_method, [](NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.occlusionMethod = static_cast<al::steam_audio::OcclusionMethod>(val);
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_direct_level, [](NetworkState *state, ConVar *, float oldVal, float val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.directLevel = val;
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_indirect, [](NetworkState *state, ConVar *, bool oldVal, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.indirect = val;
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_indirect_binaural, [](NetworkState *state, ConVar *, bool oldVal, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.indirectBinaural = val;
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_indirect_level, [](NetworkState *state, ConVar *, float oldVal, float val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.indirectLevel = val;
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_simulation_type, [](NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.simulationType = static_cast<al::steam_audio::SimulationType>(val);
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_spatialize_static_listener, [](NetworkState *state, ConVar *, bool oldVal, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.staticListener = val;
});

REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_reverb_indirect_binaural, [](NetworkState *state, ConVar *, bool oldVal, bool val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.reverb.indirectBinaural = val;
});
REGISTER_CONVAR_CALLBACK_CL(cl_steam_audio_reverb_simulation_type, [](NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	auto &props = soundSys->GetSteamAudioProperties();
	props.reverb.simulationType = static_cast<al::steam_audio::SimulationType>(val);
});

#endif
