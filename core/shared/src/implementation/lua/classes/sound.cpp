// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.sound;

namespace Lua {
	namespace ALSound {
		static CallbackHandle CallOnStateChanged(lua::State *l, pragma::audio::ALSound &snd, const func<void, pragma::audio::ALState, pragma::audio::ALState> &function);
		static void SetSource(lua::State *l, pragma::audio::ALSound &snd);

		static void SetDirectFilter(lua::State *l, pragma::audio::ALSound &snd, float gain, float gainHF, float gainLF);
		static void SetDirectFilter(lua::State *l, pragma::audio::ALSound &snd, float gain);
		static std::tuple<float, float, float> GetDirectFilter(lua::State *l, pragma::audio::ALSound &snd);
		static bool AddEffect(lua::State *l, pragma::audio::ALSound &snd, const std::string &name, float gain, float gainHF, float gainLF);
		static bool AddEffect(lua::State *l, pragma::audio::ALSound &snd, const std::string &name, float gain);
		static void SetEffectParameters(lua::State *l, pragma::audio::ALSound &snd, const std::string &name, float gain, float gainHF, float gainLF);
		static void SetEffectParameters(lua::State *l, pragma::audio::ALSound &snd, const std::string &name, float gain);
	};
};

void Lua::ALSound::register_class(luabind::class_<pragma::audio::ALSound> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("Play", &pragma::audio::ALSound::Play);
	classDef.def("Stop", &pragma::audio::ALSound::Stop);
	classDef.def("SetOffset", &pragma::audio::ALSound::SetOffset);
	classDef.def("GetOffset", &pragma::audio::ALSound::GetOffset);
	classDef.def("SetPitch", &pragma::audio::ALSound::SetPitch);
	classDef.def("GetPitch", &pragma::audio::ALSound::GetPitch);
	classDef.def("SetLooping", &pragma::audio::ALSound::SetLooping);
	classDef.def("IsLooping", &pragma::audio::ALSound::IsLooping);
	classDef.def("IsPlaying", &pragma::audio::ALSound::IsPlaying);
	classDef.def("IsStopped", &pragma::audio::ALSound::IsStopped);
	classDef.def("GetState", &pragma::audio::ALSound::GetState);
	classDef.def("Rewind", &pragma::audio::ALSound::Rewind);
	classDef.def("Pause", &pragma::audio::ALSound::Pause);
	classDef.def("IsPaused", &pragma::audio::ALSound::IsPaused);
	classDef.def("SetGain", &pragma::audio::ALSound::SetGain);
	classDef.def("GetGain", &pragma::audio::ALSound::GetGain);
	classDef.def("SetPos", &pragma::audio::ALSound::SetPosition);
	classDef.def("GetPos", &pragma::audio::ALSound::GetPosition);
	classDef.def("SetVelocity", &pragma::audio::ALSound::SetVelocity);
	classDef.def("GetVelocity", &pragma::audio::ALSound::GetVelocity);
	classDef.def("SetDirection", &pragma::audio::ALSound::SetDirection);
	classDef.def("GetDirection", &pragma::audio::ALSound::GetDirection);
	classDef.def("SetRelative", &pragma::audio::ALSound::SetRelative);
	classDef.def("IsRelative", &pragma::audio::ALSound::IsRelative);
	classDef.def("GetDuration", &pragma::audio::ALSound::GetDuration);
	classDef.def("GetReferenceDistance", &pragma::audio::ALSound::GetReferenceDistance);
	classDef.def("SetReferenceDistance", &pragma::audio::ALSound::SetReferenceDistance);
	classDef.def("GetRolloffFactor", &pragma::audio::ALSound::GetRolloffFactor);
	classDef.def("SetRolloffFactor", &pragma::audio::ALSound::SetRolloffFactor);
	classDef.def("GetRoomRolloffFactor", &pragma::audio::ALSound::GetRoomRolloffFactor);
	classDef.def("SetRoomRolloffFactor", &pragma::audio::ALSound::SetRoomRolloffFactor);
	classDef.def("GetMaxDistance", &pragma::audio::ALSound::GetMaxDistance);
	classDef.def("SetMaxDistance", &pragma::audio::ALSound::SetMaxDistance);
	classDef.def("GetMinGain", &pragma::audio::ALSound::GetMinGain);
	classDef.def("SetMinGain", &pragma::audio::ALSound::SetMinGain);
	classDef.def("GetMaxGain", &pragma::audio::ALSound::GetMaxGain);
	classDef.def("SetMaxGain", &pragma::audio::ALSound::SetMaxGain);
	classDef.def("GetInnerConeAngle", &pragma::audio::ALSound::GetInnerConeAngle);
	classDef.def("SetInnerConeAngle", &pragma::audio::ALSound::SetInnerConeAngle);
	classDef.def("GetOuterConeAngle", &pragma::audio::ALSound::GetOuterConeAngle);
	classDef.def("SetOuterConeAngle", &pragma::audio::ALSound::SetOuterConeAngle);
	classDef.def("GetOuterConeGain", &pragma::audio::ALSound::GetOuterConeGain);
	classDef.def("SetOuterConeGain", &pragma::audio::ALSound::SetOuterConeGain);
	classDef.def("GetOuterConeGainHF", &pragma::audio::ALSound::GetOuterConeGainHF);
	classDef.def("SetOuterConeGainHF", &pragma::audio::ALSound::SetOuterConeGainHF);
	classDef.def("GetType", &pragma::audio::ALSound::GetType);
	classDef.def("SetType", &pragma::audio::ALSound::SetType);
	classDef.def("AddType", &pragma::audio::ALSound::AddType);
	classDef.def("CallOnStateChanged", &CallOnStateChanged);
	classDef.def("SetPriority", &pragma::audio::ALSound::SetPriority);
	classDef.def("GetPriority", &pragma::audio::ALSound::GetPriority);
	classDef.def("FadeIn", &pragma::audio::ALSound::FadeIn);
	classDef.def("FadeOut", &pragma::audio::ALSound::FadeOut);
	classDef.def("GetMaxAudibleDistance", &pragma::audio::ALSound::GetMaxAudibleDistance);
	classDef.def("GetIntensity", &pragma::audio::ALSound::GetSoundIntensity);
	classDef.def("GetSource", &pragma::audio::ALSound::GetSource);
	classDef.def("SetSource", static_cast<void (pragma::audio::ALSound::*)(pragma::ecs::BaseEntity *)>(&pragma::audio::ALSound::SetSource));
	classDef.def("SetSource", static_cast<void (*)(lua::State *, pragma::audio::ALSound &)>(&SetSource));
	classDef.def("SetRange", &pragma::audio::ALSound::SetRange);
	classDef.def("ClearRange", &pragma::audio::ALSound::ClearRange);
	classDef.def("HasRange", &pragma::audio::ALSound::HasRange);
	classDef.def("GetRange", &pragma::audio::ALSound::GetRange);
	classDef.def("GetRangeOffsets", &pragma::audio::ALSound::GetRangeOffsets);
	classDef.def("SetFadeInDuration", &pragma::audio::ALSound::SetFadeInDuration);
	classDef.def("SetFadeOutDuration", &pragma::audio::ALSound::SetFadeOutDuration);
	classDef.def("GetFadeInDuration", &pragma::audio::ALSound::GetFadeInDuration);
	classDef.def("GetFadeOutDuration", &pragma::audio::ALSound::GetFadeOutDuration);

	classDef.def("SetOrientation", &pragma::audio::ALSound::SetOrientation);
	classDef.def("GetOrientation", &pragma::audio::ALSound::GetOrientation);
	classDef.def("SetDopplerFactor", &pragma::audio::ALSound::SetDopplerFactor);
	classDef.def("GetDopplerFactor", &pragma::audio::ALSound::GetDopplerFactor);
	classDef.def("SetLeftStereoAngle", &pragma::audio::ALSound::SetLeftStereoAngle);
	classDef.def("GetLeftStereoAngle", &pragma::audio::ALSound::GetLeftStereoAngle);
	classDef.def("SetRightStereoAngle", &pragma::audio::ALSound::SetRightStereoAngle);
	classDef.def("GetRightStereoAngle", &pragma::audio::ALSound::GetRightStereoAngle);
	classDef.def("SetAirAbsorptionFactor", &pragma::audio::ALSound::SetAirAbsorptionFactor);
	classDef.def("GetAirAbsorptionFactor", &pragma::audio::ALSound::GetAirAbsorptionFactor);
	classDef.def("SetGainAuto", &pragma::audio::ALSound::SetGainAuto);
	classDef.def("GetGainAuto", &pragma::audio::ALSound::GetGainAuto);
	classDef.def("SetDirectFilter", static_cast<void (*)(lua::State *, pragma::audio::ALSound &, float, float, float)>(&SetDirectFilter));
	classDef.def("SetDirectFilter", static_cast<void (*)(lua::State *, pragma::audio::ALSound &, float)>(&SetDirectFilter));
	classDef.def("GetDirectFilter", &GetDirectFilter);
	classDef.def("AddEffect", static_cast<bool (*)(lua::State *, pragma::audio::ALSound &, const std::string &, float, float, float)>(&AddEffect));
	classDef.def("AddEffect", static_cast<bool (*)(lua::State *, pragma::audio::ALSound &, const std::string &, float)>(&AddEffect));
	classDef.def("RemoveEffect", &pragma::audio::ALSound::RemoveEffect);
	classDef.def("SetEffectParameters", static_cast<bool (*)(lua::State *, pragma::audio::ALSound &, const std::string &, float, float, float)>(&AddEffect));
	classDef.def("SetEffectParameters", static_cast<bool (*)(lua::State *, pragma::audio::ALSound &, const std::string &, float)>(&AddEffect));

	classDef.def("SetGainRange", &pragma::audio::ALSound::SetGainRange);
	classDef.def("GetGainRange", &pragma::audio::ALSound::GetGainRange);
	classDef.def("SetDistanceRange", &pragma::audio::ALSound::SetDistanceRange);
	classDef.def("GetDistanceRange", &pragma::audio::ALSound::GetDistanceRange);
	classDef.def("SetConeAngles", &pragma::audio::ALSound::SetConeAngles);
	classDef.def("GetConeAngles", &pragma::audio::ALSound::GetConeAngles);
	classDef.def("GetOuterConeGains", &pragma::audio::ALSound::GetOuterConeGains);
	classDef.def("SetStereoAngles", &pragma::audio::ALSound::SetStereoAngles);
	classDef.def("GetStereoAngles", &pragma::audio::ALSound::GetStereoAngles);
	classDef.def("SetOuterConeGains", &pragma::audio::ALSound::SetOuterConeGains);
	classDef.def("GetDirectGainHFAuto", &pragma::audio::ALSound::GetDirectGainHFAuto);
	classDef.def("GetSendGainAuto", &pragma::audio::ALSound::GetSendGainAuto);
	classDef.def("GetSendGainHFAuto", &pragma::audio::ALSound::GetSendGainHFAuto);
	classDef.def("SetEffectGain", &pragma::audio::ALSound::SetEffectGain);
	classDef.def("GetRolloffFactors", &pragma::audio::ALSound::GetRolloffFactors);
	classDef.def("SetRolloffFactors", &pragma::audio::ALSound::SetRolloffFactors);
}

CallbackHandle Lua::ALSound::CallOnStateChanged(lua::State *l, pragma::audio::ALSound &snd, const func<void, pragma::audio::ALState, pragma::audio::ALState> &function) { return snd.AddLuaCallback("OnStateChanged", function); }

void Lua::ALSound::SetSource(lua::State *, pragma::audio::ALSound &snd) { snd.SetSource(nullptr); }
void Lua::ALSound::SetDirectFilter(lua::State *l, pragma::audio::ALSound &snd, float gain, float gainHF, float gainLF) { snd.SetDirectFilter({gain, gainHF, gainLF}); }
void Lua::ALSound::SetDirectFilter(lua::State *l, pragma::audio::ALSound &snd, float gain) { snd.SetDirectFilter({gain}); }
std::tuple<float, float, float> Lua::ALSound::GetDirectFilter(lua::State *l, pragma::audio::ALSound &snd)
{
	auto &filter = snd.GetDirectFilter();
	return {filter.gain, filter.gainHF, filter.gainLF};
}
bool Lua::ALSound::AddEffect(lua::State *l, pragma::audio::ALSound &snd, const std::string &name, float gain, float gainHF, float gainLF) { return snd.AddEffect(name, {gain, gainHF, gainLF}); }
bool Lua::ALSound::AddEffect(lua::State *l, pragma::audio::ALSound &snd, const std::string &name, float gain) { return snd.AddEffect(name, {gain}); }
void Lua::ALSound::SetEffectParameters(lua::State *l, pragma::audio::ALSound &snd, const std::string &name, float gain, float gainHF, float gainLF) { snd.SetEffectParameters(name, {gain, gainHF, gainLF}); }
void Lua::ALSound::SetEffectParameters(lua::State *l, pragma::audio::ALSound &snd, const std::string &name, float gain) { snd.SetEffectParameters(name, {gain}); }
