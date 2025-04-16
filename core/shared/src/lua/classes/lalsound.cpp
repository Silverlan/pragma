/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lalsound.h"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/audio/alsound.h"
#include "luasystem.h"
#include <mathutil/umath.h>
#include "pragma/lua/classes/ldef_entity.h"

namespace Lua {
	namespace ALSound {
		static CallbackHandle CallOnStateChanged(lua_State *l, ::ALSound &snd, const func<void, ALState, ALState> &function);
		static void SetSource(lua_State *l, ::ALSound &snd);

		static void SetDirectFilter(lua_State *l, ::ALSound &snd, float gain, float gainHF, float gainLF);
		static void SetDirectFilter(lua_State *l, ::ALSound &snd, float gain);
		static std::tuple<float, float, float> GetDirectFilter(lua_State *l, ::ALSound &snd);
		static bool AddEffect(lua_State *l, ::ALSound &snd, const std::string &name, float gain, float gainHF, float gainLF);
		static bool AddEffect(lua_State *l, ::ALSound &snd, const std::string &name, float gain);
		static void SetEffectParameters(lua_State *l, ::ALSound &snd, const std::string &name, float gain, float gainHF, float gainLF);
		static void SetEffectParameters(lua_State *l, ::ALSound &snd, const std::string &name, float gain);
	};
};

void Lua::ALSound::register_class(luabind::class_<::ALSound> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("Play", &::ALSound::Play);
	classDef.def("Stop", &::ALSound::Stop);
	classDef.def("SetOffset", &::ALSound::SetOffset);
	classDef.def("GetOffset", &::ALSound::GetOffset);
	classDef.def("SetPitch", &::ALSound::SetPitch);
	classDef.def("GetPitch", &::ALSound::GetPitch);
	classDef.def("SetLooping", &::ALSound::SetLooping);
	classDef.def("IsLooping", &::ALSound::IsLooping);
	classDef.def("IsPlaying", &::ALSound::IsPlaying);
	classDef.def("IsStopped", &::ALSound::IsStopped);
	classDef.def("GetState", &::ALSound::GetState);
	classDef.def("Rewind", &::ALSound::Rewind);
	classDef.def("Pause", &::ALSound::Pause);
	classDef.def("IsPaused", &::ALSound::IsPaused);
	classDef.def("SetGain", &::ALSound::SetGain);
	classDef.def("GetGain", &::ALSound::GetGain);
	classDef.def("SetPos", &::ALSound::SetPosition);
	classDef.def("GetPos", &::ALSound::GetPosition);
	classDef.def("SetVelocity", &::ALSound::SetVelocity);
	classDef.def("GetVelocity", &::ALSound::GetVelocity);
	classDef.def("SetDirection", &::ALSound::SetDirection);
	classDef.def("GetDirection", &::ALSound::GetDirection);
	classDef.def("SetRelative", &::ALSound::SetRelative);
	classDef.def("IsRelative", &::ALSound::IsRelative);
	classDef.def("GetDuration", &::ALSound::GetDuration);
	classDef.def("GetReferenceDistance", &::ALSound::GetReferenceDistance);
	classDef.def("SetReferenceDistance", &::ALSound::SetReferenceDistance);
	classDef.def("GetRolloffFactor", &::ALSound::GetRolloffFactor);
	classDef.def("SetRolloffFactor", &::ALSound::SetRolloffFactor);
	classDef.def("GetRoomRolloffFactor", &::ALSound::GetRoomRolloffFactor);
	classDef.def("SetRoomRolloffFactor", &::ALSound::SetRoomRolloffFactor);
	classDef.def("GetMaxDistance", &::ALSound::GetMaxDistance);
	classDef.def("SetMaxDistance", &::ALSound::SetMaxDistance);
	classDef.def("GetMinGain", &::ALSound::GetMinGain);
	classDef.def("SetMinGain", &::ALSound::SetMinGain);
	classDef.def("GetMaxGain", &::ALSound::GetMaxGain);
	classDef.def("SetMaxGain", &::ALSound::SetMaxGain);
	classDef.def("GetInnerConeAngle", &::ALSound::GetInnerConeAngle);
	classDef.def("SetInnerConeAngle", &::ALSound::SetInnerConeAngle);
	classDef.def("GetOuterConeAngle", &::ALSound::GetOuterConeAngle);
	classDef.def("SetOuterConeAngle", &::ALSound::SetOuterConeAngle);
	classDef.def("GetOuterConeGain", &::ALSound::GetOuterConeGain);
	classDef.def("SetOuterConeGain", &::ALSound::SetOuterConeGain);
	classDef.def("GetOuterConeGainHF", &::ALSound::GetOuterConeGainHF);
	classDef.def("SetOuterConeGainHF", &::ALSound::SetOuterConeGainHF);
	classDef.def("GetType", &::ALSound::GetType);
	classDef.def("SetType", &::ALSound::SetType);
	classDef.def("AddType", &::ALSound::AddType);
	classDef.def("CallOnStateChanged", &Lua::ALSound::CallOnStateChanged);
	classDef.def("SetPriority", &::ALSound::SetPriority);
	classDef.def("GetPriority", &::ALSound::GetPriority);
	classDef.def("FadeIn", &::ALSound::FadeIn);
	classDef.def("FadeOut", &::ALSound::FadeOut);
	classDef.def("GetMaxAudibleDistance", &::ALSound::GetMaxAudibleDistance);
	classDef.def("GetIntensity", &::ALSound::GetSoundIntensity);
	classDef.def("GetSource", &::ALSound::GetSource);
	classDef.def("SetSource", static_cast<void (::ALSound::*)(BaseEntity *)>(&::ALSound::SetSource));
	classDef.def("SetSource", static_cast<void (*)(lua_State *, ::ALSound &)>(&Lua::ALSound::SetSource));
	classDef.def("SetRange", &::ALSound::SetRange);
	classDef.def("ClearRange", &::ALSound::ClearRange);
	classDef.def("HasRange", &::ALSound::HasRange);
	classDef.def("GetRange", &::ALSound::GetRange);
	classDef.def("GetRangeOffsets", &::ALSound::GetRangeOffsets);
	classDef.def("SetFadeInDuration", &::ALSound::SetFadeInDuration);
	classDef.def("SetFadeOutDuration", &::ALSound::SetFadeOutDuration);
	classDef.def("GetFadeInDuration", &::ALSound::GetFadeInDuration);
	classDef.def("GetFadeOutDuration", &::ALSound::GetFadeOutDuration);

	classDef.def("SetOrientation", &::ALSound::SetOrientation);
	classDef.def("GetOrientation", &::ALSound::GetOrientation);
	classDef.def("SetDopplerFactor", &::ALSound::SetDopplerFactor);
	classDef.def("GetDopplerFactor", &::ALSound::GetDopplerFactor);
	classDef.def("SetLeftStereoAngle", &::ALSound::SetLeftStereoAngle);
	classDef.def("GetLeftStereoAngle", &::ALSound::GetLeftStereoAngle);
	classDef.def("SetRightStereoAngle", &::ALSound::SetRightStereoAngle);
	classDef.def("GetRightStereoAngle", &::ALSound::GetRightStereoAngle);
	classDef.def("SetAirAbsorptionFactor", &::ALSound::SetAirAbsorptionFactor);
	classDef.def("GetAirAbsorptionFactor", &::ALSound::GetAirAbsorptionFactor);
	classDef.def("SetGainAuto", &::ALSound::SetGainAuto);
	classDef.def("GetGainAuto", &::ALSound::GetGainAuto);
	classDef.def("SetDirectFilter", static_cast<void (*)(lua_State *, ::ALSound &, float, float, float)>(&Lua::ALSound::SetDirectFilter));
	classDef.def("SetDirectFilter", static_cast<void (*)(lua_State *, ::ALSound &, float)>(&Lua::ALSound::SetDirectFilter));
	classDef.def("GetDirectFilter", &Lua::ALSound::GetDirectFilter);
	classDef.def("AddEffect", static_cast<bool (*)(lua_State *, ::ALSound &, const std::string &, float, float, float)>(&Lua::ALSound::AddEffect));
	classDef.def("AddEffect", static_cast<bool (*)(lua_State *, ::ALSound &, const std::string &, float)>(&Lua::ALSound::AddEffect));
	classDef.def("RemoveEffect", &::ALSound::RemoveEffect);
	classDef.def("SetEffectParameters", static_cast<bool (*)(lua_State *, ::ALSound &, const std::string &, float, float, float)>(&Lua::ALSound::AddEffect));
	classDef.def("SetEffectParameters", static_cast<bool (*)(lua_State *, ::ALSound &, const std::string &, float)>(&Lua::ALSound::AddEffect));

	classDef.def("SetGainRange", &::ALSound::SetGainRange);
	classDef.def("GetGainRange", &::ALSound::GetGainRange);
	classDef.def("SetDistanceRange", &::ALSound::SetDistanceRange);
	classDef.def("GetDistanceRange", &::ALSound::GetDistanceRange);
	classDef.def("SetConeAngles", &::ALSound::SetConeAngles);
	classDef.def("GetConeAngles", &::ALSound::GetConeAngles);
	classDef.def("GetOuterConeGains", &::ALSound::GetOuterConeGains);
	classDef.def("SetStereoAngles", &::ALSound::SetStereoAngles);
	classDef.def("GetStereoAngles", &::ALSound::GetStereoAngles);
	classDef.def("SetOuterConeGains", &::ALSound::SetOuterConeGains);
	classDef.def("GetDirectGainHFAuto", &::ALSound::GetDirectGainHFAuto);
	classDef.def("GetSendGainAuto", &::ALSound::GetSendGainAuto);
	classDef.def("GetSendGainHFAuto", &::ALSound::GetSendGainHFAuto);
	classDef.def("SetEffectGain", &::ALSound::SetEffectGain);
	classDef.def("GetRolloffFactors", &::ALSound::GetRolloffFactors);
	classDef.def("SetRolloffFactors", &::ALSound::SetRolloffFactors);
}

CallbackHandle Lua::ALSound::CallOnStateChanged(lua_State *l, ::ALSound &snd, const func<void, ALState, ALState> &function) { return snd.AddLuaCallback("OnStateChanged", function); }

void Lua::ALSound::SetSource(lua_State *, ::ALSound &snd) { snd.SetSource(nullptr); }
void Lua::ALSound::SetDirectFilter(lua_State *l, ::ALSound &snd, float gain, float gainHF, float gainLF) { snd.SetDirectFilter({gain, gainHF, gainLF}); }
void Lua::ALSound::SetDirectFilter(lua_State *l, ::ALSound &snd, float gain) { snd.SetDirectFilter({gain}); }
std::tuple<float, float, float> Lua::ALSound::GetDirectFilter(lua_State *l, ::ALSound &snd)
{
	auto &filter = snd.GetDirectFilter();
	return {filter.gain, filter.gainHF, filter.gainLF};
}
bool Lua::ALSound::AddEffect(lua_State *l, ::ALSound &snd, const std::string &name, float gain, float gainHF, float gainLF) { return snd.AddEffect(name, {gain, gainHF, gainLF}); }
bool Lua::ALSound::AddEffect(lua_State *l, ::ALSound &snd, const std::string &name, float gain) { return snd.AddEffect(name, {gain}); }
void Lua::ALSound::SetEffectParameters(lua_State *l, ::ALSound &snd, const std::string &name, float gain, float gainHF, float gainLF) { snd.SetEffectParameters(name, {gain, gainHF, gainLF}); }
void Lua::ALSound::SetEffectParameters(lua_State *l, ::ALSound &snd, const std::string &name, float gain) { snd.SetEffectParameters(name, {gain}); }
