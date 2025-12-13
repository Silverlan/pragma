// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

static void reload_sound_cache()
{
	if(pragma::get_cgame() == nullptr)
		return;
	static CallbackHandle hCacheCallback = {};
	if(hCacheCallback.IsValid())
		hCacheCallback.Remove();
	// Delay cache reloading to next frame, in case multiple steam audio commands have been executed in rapid succession
	hCacheCallback = pragma::get_cgame()->AddCallback("Think", FunctionCallback<void>::Create([]() {
		pragma::get_cgame()->ReloadSoundCache();
		if(hCacheCallback.IsValid())
			hCacheCallback.Remove();
	}));
}

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>("cl_steam_audio_number_of_rays", +[](pragma::NetworkState *state, ConVar *, int32_t, int32_t val) { reload_sound_cache(); });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>("cl_steam_audio_number_of_diffuse_samples", +[](pragma::NetworkState *state, ConVar *, int32_t, int32_t val) { reload_sound_cache(); });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>("cl_steam_audio_number_of_bounces", [](pragma::NetworkState *state, ConVar *, int32_t, int32_t val) { reload_sound_cache(); });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>("cl_steam_audio_ir_duration", +[](pragma::NetworkState *state, ConVar *, int32_t, int32_t val) { reload_sound_cache(); });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>("cl_steam_audio_ambisonics_order", +[](pragma::NetworkState *state, ConVar *, int32_t, int32_t val) { reload_sound_cache(); });
}

static void cl_steam_audio_enabled(pragma::NetworkState *state, ConVar *, bool, bool val)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	if(val == false && pragma::get_cgame() != nullptr)
		pragma::get_cgame()->ClearSoundCache();

	auto *en = pragma::get_cengine();
	soundSys->SetSteamAudioSpatializerEnabled(pragma::get_cengine()->GetConVarBool("cl_steam_audio_spatialize_enabled"));
	soundSys->SetSteamAudioReverbEnabled(en->GetConVarBool("cl_steam_audio_reverb_enabled"));

	soundSys->SetSteamAudioEnabled(val);

	auto &props = soundSys->GetSteamAudioProperties();
	props.spatializer.directBinaural = en->GetConVarBool("cl_steam_audio_spatialize_direct_binaural");
	props.spatializer.HRTFInterpolation = static_cast<pragma::audio::steam_audio::SpatializerInterpolation>(en->GetConVarInt("cl_steam_audio_spatialize_hrtf_interpolation"));
	props.spatializer.distanceAttenuation = en->GetConVarBool("cl_steam_audio_spatialize_distance_attenuation");
	props.spatializer.airAbsorption = en->GetConVarBool("cl_steam_audio_spatialize_air_absorption");
	props.spatializer.occlusionMode = static_cast<pragma::audio::steam_audio::SpatializerOcclusionMode>(en->GetConVarInt("cl_steam_audio_spatialize_occlusion_mode"));
	props.spatializer.occlusionMethod = static_cast<pragma::audio::steam_audio::OcclusionMethod>(en->GetConVarInt("cl_steam_audio_spatialize_occlusion_method"));
	props.spatializer.directLevel = en->GetConVarFloat("cl_steam_audio_spatialize_direct_level");
	props.spatializer.indirect = en->GetConVarBool("cl_steam_audio_spatialize_indirect");
	props.spatializer.indirectBinaural = en->GetConVarBool("cl_steam_audio_spatialize_indirect_binaural");
	props.spatializer.indirectLevel = en->GetConVarFloat("cl_steam_audio_spatialize_indirect_level");
	props.spatializer.simulationType = static_cast<pragma::audio::steam_audio::SimulationType>(en->GetConVarInt("cl_steam_audio_spatialize_simulation_type"));
	props.spatializer.staticListener = en->GetConVarBool("cl_steam_audio_spatialize_static_listener");

	props.reverb.indirectBinaural = en->GetConVarBool("cl_steam_audio_reverb_indirect_binaural");
	props.reverb.simulationType = static_cast<pragma::audio::steam_audio::SimulationType>(en->GetConVarInt("cl_steam_audio_reverb_simulation_type"));

	if(val == true)
		reload_sound_cache();
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("cl_steam_audio_enabled", &cl_steam_audio_enabled);
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_spatialize_enabled", +[](pragma::NetworkState *state, ConVar *, bool, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  //soundSys->SetSteamAudioSpatializerDSPEnabled(val);
	  })
};

namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_reverb_enabled", +[](pragma::NetworkState *state, ConVar *, bool, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  //soundSys->SetSteamAudioReverbDSPEnabled(val);
	  });
};

namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_propagation_delay_enabled", +[](pragma::NetworkState *state, ConVar *, bool, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto *iplScene = soundSys->GetSteamAudioScene();
		  iplScene->SetPropagationDelayEnabled(val);
	  });
};

namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_spatialize_direct_binaural", +[](pragma::NetworkState *state, ConVar *, bool oldVal, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.directBinaural = val;
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>(
	  "cl_steam_audio_spatialize_hrtf_interpolation", +[](pragma::NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.HRTFInterpolation = static_cast<pragma::audio::steam_audio::SpatializerInterpolation>(val);
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_spatialize_distance_attenuation", +[](pragma::NetworkState *state, ConVar *, bool oldVal, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.distanceAttenuation = val;
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_spatialize_air_absorption", +[](pragma::NetworkState *state, ConVar *, bool oldVal, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.airAbsorption = val;
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>(
	  "cl_steam_audio_spatialize_occlusion_mode", +[](pragma::NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.occlusionMode = static_cast<pragma::audio::steam_audio::SpatializerOcclusionMode>(val);
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>(
	  "cl_steam_audio_spatialize_occlusion_method", +[](pragma::NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.occlusionMethod = static_cast<pragma::audio::steam_audio::OcclusionMethod>(val);
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>(
	  "cl_steam_audio_spatialize_direct_level", +[](pragma::NetworkState *state, ConVar *, float oldVal, float val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.directLevel = val;
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_spatialize_indirect", +[](pragma::NetworkState *state, ConVar *, bool oldVal, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.indirect = val;
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_spatialize_indirect_binaural", +[](pragma::NetworkState *state, ConVar *, bool oldVal, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.indirectBinaural = val;
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>(
	  "cl_steam_audio_spatialize_indirect_level", +[](pragma::NetworkState *state, ConVar *, float oldVal, float val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.indirectLevel = val;
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>(
	  "cl_steam_audio_spatialize_simulation_type", +[](pragma::NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.simulationType = static_cast<pragma::audio::steam_audio::SimulationType>(val);
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_spatialize_static_listener", +[](pragma::NetworkState *state, ConVar *, bool oldVal, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.spatializer.staticListener = val;
	  });
};

namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_steam_audio_reverb_indirect_binaural", +[](pragma::NetworkState *state, ConVar *, bool oldVal, bool val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.reverb.indirectBinaural = val;
	  });
};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>(
	  "cl_steam_audio_reverb_simulation_type", +[](pragma::NetworkState *state, ConVar *, int32_t oldVal, int32_t val) {
		  auto *soundSys = pragma::get_cengine()->GetSoundSystem();
		  if(soundSys == nullptr)
			  return;
		  auto &props = soundSys->GetSteamAudioProperties();
		  props.reverb.simulationType = static_cast<pragma::audio::steam_audio::SimulationType>(val);
	  });
};

#endif
