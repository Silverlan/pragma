/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lalsound.h"
#include "pragma/audio/alsound.h"
#include "luasystem.h"
#include <mathutil/umath.h>
#include "pragma/lua/classes/ldef_entity.h"

namespace Lua
{
	namespace ALSound
	{
		static void GetFlags(lua_State *l,::ALSound &snd);
		static void SetFlags(lua_State *l,::ALSound &snd,unsigned int flags);
		static void AddFlags(lua_State *l,::ALSound &snd,unsigned int flags);
		static void RemoveFlags(lua_State *l,::ALSound &snd,unsigned int flags);

		static void Play(lua_State *l,::ALSound &snd);
		static void Stop(lua_State *l,::ALSound &snd);
		static void SetOffset(lua_State *l,::ALSound &snd,float offset);
		static void GetOffset(lua_State *l,::ALSound &snd);
		static void SetPitch(lua_State *l,::ALSound &snd,float pitch);
		static void GetPitch(lua_State *l,::ALSound &snd);
		static void SetLooping(lua_State *l,::ALSound &snd,bool loop);
		static void IsLooping(lua_State *l,::ALSound &snd);
		static void IsPlaying(lua_State *l,::ALSound &snd);
		static void IsStopped(lua_State *l,::ALSound &snd);
		static void GetState(lua_State *l,::ALSound &snd);
		static void Rewind(lua_State *l,::ALSound &snd);
		static void Pause(lua_State *l,::ALSound &snd);
		static void IsPaused(lua_State *l,::ALSound &snd);
		static void SetGain(lua_State *l,::ALSound &snd,float gain);
		static void GetGain(lua_State *l,::ALSound &snd);
		static void SetPos(lua_State *l,::ALSound &snd,Vector3 pos);
		static void GetPos(lua_State *l,::ALSound &snd);
		static void SetVelocity(lua_State *l,::ALSound &snd,Vector3 vel);
		static void GetVelocity(lua_State *l,::ALSound &snd);
		static void SetDirection(lua_State *l,::ALSound &snd,Vector3 dir);
		static void GetDirection(lua_State *l,::ALSound &snd);
		static void SetRelativeToListener(lua_State *l,::ALSound &snd,bool b);
		static void IsRelativeToListener(lua_State *l,::ALSound &snd);
		static void SetSecOffset(lua_State *l,::ALSound &snd,float sec);
		static void GetSecOffset(lua_State *l,::ALSound &snd);
		static void GetDuration(lua_State *l,::ALSound &snd);
		static void CallOnStateChanged(lua_State *l,::ALSound &snd,luabind::object o);
		static void SetPriority(lua_State *l,::ALSound &snd,unsigned char priority);
		static void GetPriority(lua_State *l,::ALSound &snd);
		static void FadeIn(lua_State *l,::ALSound &snd,int time);
		static void FadeOut(lua_State *l,::ALSound &snd,int time);

		static void GetReferenceDistance(lua_State *l,::ALSound &snd);
		static void SetReferenceDistance(lua_State *l,::ALSound &snd,float dist);
		static void GetRolloffFactor(lua_State *l,::ALSound &snd);
		static void SetRolloffFactor(lua_State *l,::ALSound &snd,float rolloff);
		static void GetRoomRolloffFactor(lua_State *l,::ALSound &snd);
		static void SetRoomRolloffFactor(lua_State *l,::ALSound &snd,float rolloff);
		static void GetMaxDistance(lua_State *l,::ALSound &snd);
		static void SetMaxDistance(lua_State *l,::ALSound &snd,float dist);
		static void GetMinGain(lua_State *l,::ALSound &snd);
		static void SetMinGain(lua_State *l,::ALSound &snd,float gain);
		static void GetMaxGain(lua_State *l,::ALSound &snd);
		static void SetMaxGain(lua_State *l,::ALSound &snd,float gain);
		static void GetConeInnerAngle(lua_State *l,::ALSound &snd);
		static void SetConeInnerAngle(lua_State *l,::ALSound &snd,float ang);
		static void GetConeOuterAngle(lua_State *l,::ALSound &snd);
		static void SetConeOuterAngle(lua_State *l,::ALSound &snd,float ang);
		static void GetConeOuterGain(lua_State *l,::ALSound &snd);
		static void SetConeOuterGain(lua_State *l,::ALSound &snd,float gain);
		static void GetConeOuterGainHF(lua_State *l,::ALSound &snd);
		static void SetConeOuterGainHF(lua_State *l,::ALSound &snd,float gain);
		static void GetType(lua_State *l,::ALSound &snd);
		static void SetType(lua_State *l,::ALSound &snd,int type);
		static void AddType(lua_State *l,::ALSound &snd,int type);

		static void GetMaxAudibleDistance(lua_State *l,::ALSound &snd);
		static void GetSoundIntensity(lua_State *l,::ALSound &snd,const Vector3 &origin);

		static void GetSource(lua_State *l,::ALSound &snd);
		static void SetSource(lua_State *l,::ALSound &snd);
		static void SetSource(lua_State *l,::ALSound &snd,EntityHandle &hEnt);

		static void SetRange(lua_State *l,::ALSound &snd,float start,float end);
		static void ClearRange(lua_State *l,::ALSound &snd);
		static void HasRange(lua_State *l,::ALSound &snd);
		static void GetRange(lua_State *l,::ALSound &snd);
		static void GetRangeOffsets(lua_State *l,::ALSound &snd);

		static void SetFadeInDuration(lua_State *l,::ALSound &snd,float t);
		static void SetFadeOutDuration(lua_State *l,::ALSound &snd,float t);
		static void GetFadeInDuration(lua_State *l,::ALSound &snd);
		static void GetFadeOutDuration(lua_State *l,::ALSound &snd);

		static void SetOrientation(lua_State *l,::ALSound &snd,const Vector3 &at,const Vector3 &up);
		static void GetOrientation(lua_State *l,::ALSound &snd);
		static void SetDopplerFactor(lua_State *l,::ALSound &snd,float factor);
		static void GetDopplerFactor(lua_State *l,::ALSound &snd);
		static void SetLeftStereoAngle(lua_State *l,::ALSound &snd,float ang);
		static void GetLeftStereoAngle(lua_State *l,::ALSound &snd);
		static void SetRightStereoAngle(lua_State *l,::ALSound &snd,float ang);
		static void GetRightStereoAngle(lua_State *l,::ALSound &snd);
		static void SetAirAbsorptionFactor(lua_State *l,::ALSound &snd,float airAbsorption);
		static void GetAirAbsorptionFactor(lua_State *l,::ALSound &snd);
		static void SetGainAuto(lua_State *l,::ALSound &snd,bool directHF,bool send,bool sendHF);
		static void GetGainAuto(lua_State *l,::ALSound &snd);
		static void SetDirectFilter(lua_State *l,::ALSound &snd,float gain,float gainHF,float gainLF);
		static void SetDirectFilter(lua_State *l,::ALSound &snd,float gain);
		static void GetDirectFilter(lua_State *l,::ALSound &snd);
		static void AddEffect(lua_State *l,::ALSound &snd,const std::string &name,float gain,float gainHF,float gainLF);
		static void AddEffect(lua_State *l,::ALSound &snd,const std::string &name,float gain);
		static void RemoveEffect(lua_State *l,::ALSound &snd,const std::string &name);
		static void SetEffectParameters(lua_State *l,::ALSound &snd,const std::string &name,float gain,float gainHF,float gainLF);
		static void SetEffectParameters(lua_State *l,::ALSound &snd,const std::string &name,float gain);

		static void SetGainRange(lua_State *l,::ALSound &snd,float minGain,float maxGain);
		static void GetGainRange(lua_State *l,::ALSound &snd);
		static void SetDistanceRange(lua_State *l,::ALSound &snd,float refDist,float maxDist);
		static void GetDistanceRange(lua_State *l,::ALSound &snd);
		static void SetConeAngles(lua_State *l,::ALSound &snd,float inner,float outer);
		static void GetConeAngles(lua_State *l,::ALSound &snd);
		static void GetOuterConeGains(lua_State *l,::ALSound &snd);
		static void SetStereoAngles(lua_State *l,::ALSound &snd,float leftAng,float rightAng);
		static void GetStereoAngles(lua_State *l,::ALSound &snd);
		static void SetOuterConeGains(lua_State *l,::ALSound &snd,float gain,float gainHF);
		static void GetDirectGainHFAuto(lua_State *l,::ALSound &snd);
		static void GetSendGainAuto(lua_State *l,::ALSound &snd);
		static void GetSendGainHFAuto(lua_State *l,::ALSound &snd);
		static void SetEffectGain(lua_State *l,::ALSound &snd,const std::string &name,float gain);
		static void GetRolloffFactors(lua_State *l,::ALSound &snd);
		static void SetRolloffFactors(lua_State *l,::ALSound &snd,float factor,float roomFactor);
	};
};

void Lua::ALSound::register_class(luabind::class_<::ALSound> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("Play",&Lua::ALSound::Play);
	classDef.def("Stop",&Lua::ALSound::Stop);
	classDef.def("SetOffset",&Lua::ALSound::SetOffset);
	classDef.def("GetOffset",&Lua::ALSound::GetOffset);
	classDef.def("SetTimeOffset",&Lua::ALSound::SetSecOffset);
	classDef.def("GetTimeOffset",&Lua::ALSound::GetSecOffset);
	classDef.def("SetPitch",&Lua::ALSound::SetPitch);
	classDef.def("GetPitch",&Lua::ALSound::GetPitch);
	classDef.def("SetLooping",&Lua::ALSound::SetLooping);
	classDef.def("IsLooping",&Lua::ALSound::IsLooping);
	classDef.def("IsPlaying",&Lua::ALSound::IsPlaying);
	classDef.def("IsStopped",&Lua::ALSound::IsStopped);
	classDef.def("GetState",&Lua::ALSound::GetState);
	classDef.def("Rewind",&Lua::ALSound::Rewind);
	classDef.def("Pause",&Lua::ALSound::Pause);
	classDef.def("IsPaused",&Lua::ALSound::IsPaused);
	classDef.def("SetGain",&Lua::ALSound::SetGain);
	classDef.def("GetGain",&Lua::ALSound::GetGain);
	classDef.def("SetPos",&Lua::ALSound::SetPos);
	classDef.def("GetPos",&Lua::ALSound::GetPos);
	classDef.def("SetVelocity",&Lua::ALSound::SetVelocity);
	classDef.def("GetVelocity",&Lua::ALSound::GetVelocity);
	classDef.def("SetDirection",&Lua::ALSound::SetDirection);
	classDef.def("GetDirection",&Lua::ALSound::GetDirection);
	classDef.def("SetRelative",&Lua::ALSound::SetRelativeToListener);
	classDef.def("IsRelative",&Lua::ALSound::IsRelativeToListener);
	classDef.def("GetDuration",&Lua::ALSound::GetDuration);
	classDef.def("GetReferenceDistance",&Lua::ALSound::GetReferenceDistance);
	classDef.def("SetReferenceDistance",&Lua::ALSound::SetReferenceDistance);
	classDef.def("GetRolloffFactor",&Lua::ALSound::GetRolloffFactor);
	classDef.def("SetRolloffFactor",&Lua::ALSound::SetRolloffFactor);
	classDef.def("GetRoomRolloffFactor",&Lua::ALSound::GetRoomRolloffFactor);
	classDef.def("SetRoomRolloffFactor",&Lua::ALSound::SetRoomRolloffFactor);
	classDef.def("GetMaxDistance",&Lua::ALSound::GetMaxDistance);
	classDef.def("SetMaxDistance",&Lua::ALSound::SetMaxDistance);
	classDef.def("GetMinGain",&Lua::ALSound::GetMinGain);
	classDef.def("SetMinGain",&Lua::ALSound::SetMinGain);
	classDef.def("GetMaxGain",&Lua::ALSound::GetMaxGain);
	classDef.def("SetMaxGain",&Lua::ALSound::SetMaxGain);
	classDef.def("GetInnerConeAngle",&Lua::ALSound::GetConeInnerAngle);
	classDef.def("SetInnerConeAngle",&Lua::ALSound::SetConeInnerAngle);
	classDef.def("GetOuterConeAngle",&Lua::ALSound::GetConeOuterAngle);
	classDef.def("SetOuterConeAngle",&Lua::ALSound::SetConeOuterAngle);
	classDef.def("GetOuterConeGain",&Lua::ALSound::GetConeOuterGain);
	classDef.def("SetOuterConeGain",&Lua::ALSound::SetConeOuterGain);
	classDef.def("GetOuterConeGainHF",&Lua::ALSound::GetConeOuterGainHF);
	classDef.def("SetOuterConeGainHF",&Lua::ALSound::SetConeOuterGainHF);
	classDef.def("GetType",&Lua::ALSound::GetType);
	classDef.def("SetType",&Lua::ALSound::SetType);
	classDef.def("AddType",&Lua::ALSound::AddType);
	classDef.def("CallOnStateChanged",&Lua::ALSound::CallOnStateChanged);
	classDef.def("SetPriority",&Lua::ALSound::SetPriority);
	classDef.def("GetPriority",&Lua::ALSound::GetPriority);
	classDef.def("FadeIn",&Lua::ALSound::FadeIn);
	classDef.def("FadeOut",&Lua::ALSound::FadeOut);
	classDef.def("GetMaxAudibleDistance",&Lua::ALSound::GetMaxAudibleDistance);
	classDef.def("GetIntensity",&Lua::ALSound::GetSoundIntensity);
	classDef.def("GetSource",&Lua::ALSound::GetSource);
	classDef.def("SetSource",static_cast<void(*)(lua_State*,::ALSound&,EntityHandle&)>(&Lua::ALSound::SetSource));
	classDef.def("SetSource",static_cast<void(*)(lua_State*,::ALSound&)>(&Lua::ALSound::SetSource));
	classDef.def("SetRange",&Lua::ALSound::SetRange);
	classDef.def("ClearRange",&Lua::ALSound::ClearRange);
	classDef.def("HasRange",&Lua::ALSound::HasRange);
	classDef.def("GetRange",&Lua::ALSound::GetRange);
	classDef.def("GetRangeOffsets",&Lua::ALSound::GetRangeOffsets);
	classDef.def("SetFadeInDuration",&Lua::ALSound::SetFadeInDuration);
	classDef.def("SetFadeOutDuration",&Lua::ALSound::SetFadeOutDuration);
	classDef.def("GetFadeInDuration",&Lua::ALSound::GetFadeInDuration);
	classDef.def("GetFadeOutDuration",&Lua::ALSound::GetFadeOutDuration);

	classDef.def("SetOrientation",&Lua::ALSound::SetOrientation);
	classDef.def("GetOrientation",&Lua::ALSound::GetOrientation);
	classDef.def("SetDopplerFactor",&Lua::ALSound::SetDopplerFactor);
	classDef.def("GetDopplerFactor",&Lua::ALSound::GetDopplerFactor);
	classDef.def("SetLeftStereoAngle",&Lua::ALSound::SetLeftStereoAngle);
	classDef.def("GetLeftStereoAngle",&Lua::ALSound::GetLeftStereoAngle);
	classDef.def("SetRightStereoAngle",&Lua::ALSound::SetRightStereoAngle);
	classDef.def("GetRightStereoAngle",&Lua::ALSound::GetRightStereoAngle);
	classDef.def("SetAirAbsorptionFactor",&Lua::ALSound::SetAirAbsorptionFactor);
	classDef.def("GetAirAbsorptionFactor",&Lua::ALSound::GetAirAbsorptionFactor);
	classDef.def("SetGainAuto",&Lua::ALSound::SetGainAuto);
	classDef.def("GetGainAuto",&Lua::ALSound::GetGainAuto);
	classDef.def("SetDirectFilter",static_cast<void(*)(lua_State*,::ALSound&,float,float,float)>(&Lua::ALSound::SetDirectFilter));
	classDef.def("SetDirectFilter",static_cast<void(*)(lua_State*,::ALSound&,float)>(&Lua::ALSound::SetDirectFilter));
	classDef.def("GetDirectFilter",&Lua::ALSound::GetDirectFilter);
	classDef.def("AddEffect",static_cast<void(*)(lua_State*,::ALSound&,const std::string&,float,float,float)>(&Lua::ALSound::AddEffect));
	classDef.def("AddEffect",static_cast<void(*)(lua_State*,::ALSound&,const std::string&,float)>(&Lua::ALSound::AddEffect));
	classDef.def("RemoveEffect",&Lua::ALSound::RemoveEffect);
	classDef.def("SetEffectParameters",static_cast<void(*)(lua_State*,::ALSound&,const std::string&,float,float,float)>(&Lua::ALSound::AddEffect));
	classDef.def("SetEffectParameters",static_cast<void(*)(lua_State*,::ALSound&,const std::string&,float)>(&Lua::ALSound::AddEffect));

	classDef.def("SetGainRange",&Lua::ALSound::SetGainRange);
	classDef.def("GetGainRange",&Lua::ALSound::GetGainRange);
	classDef.def("SetDistanceRange",&Lua::ALSound::SetDistanceRange);
	classDef.def("GetDistanceRange",&Lua::ALSound::GetDistanceRange);
	classDef.def("SetConeAngles",&Lua::ALSound::SetConeAngles);
	classDef.def("GetConeAngles",&Lua::ALSound::GetConeAngles);
	classDef.def("GetOuterConeGains",&Lua::ALSound::GetOuterConeGains);
	classDef.def("SetStereoAngles",&Lua::ALSound::SetStereoAngles);
	classDef.def("GetStereoAngles",&Lua::ALSound::GetStereoAngles);
	classDef.def("SetOuterConeGains",&Lua::ALSound::SetOuterConeGains);
	classDef.def("GetDirectGainHFAuto",&Lua::ALSound::GetDirectGainHFAuto);
	classDef.def("GetSendGainAuto",&Lua::ALSound::GetSendGainAuto);
	classDef.def("GetSendGainHFAuto",&Lua::ALSound::GetSendGainHFAuto);
	classDef.def("SetEffectGain",&Lua::ALSound::SetEffectGain);
	classDef.def("GetRolloffFactors",&Lua::ALSound::GetRolloffFactors);
	classDef.def("SetRolloffFactors",&Lua::ALSound::SetRolloffFactors);
}

void Lua::ALSound::GetFlags(lua_State *l,::ALSound &snd)
{
	Lua::PushInt(l,snd.GetFlags());
}

void Lua::ALSound::SetFlags(lua_State*,::ALSound &snd,unsigned int flags)
{
	snd.SetFlags(flags);
}

void Lua::ALSound::AddFlags(lua_State*,::ALSound &snd,unsigned int flags)
{
	snd.AddFlags(flags);
}

void Lua::ALSound::RemoveFlags(lua_State*,::ALSound &snd,unsigned int flags)
{
	snd.RemoveFlags(flags);
}

void Lua::ALSound::Play(lua_State*,::ALSound &snd)
{
	snd.Play();
}

void Lua::ALSound::Stop(lua_State*,::ALSound &snd)
{
	snd.Stop();
}

void Lua::ALSound::SetOffset(lua_State*,::ALSound &snd,float offset)
{
	snd.SetOffset(offset);
}

void Lua::ALSound::GetOffset(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetOffset());
}

void Lua::ALSound::SetPitch(lua_State*,::ALSound &snd,float pitch)
{
	snd.SetPitch(pitch);
}

void Lua::ALSound::GetPitch(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetPitch());
}

void Lua::ALSound::SetLooping(lua_State*,::ALSound &snd,bool loop)
{
	snd.SetLooping(loop);
}

void Lua::ALSound::IsLooping(lua_State *l,::ALSound &snd)
{
	lua_pushboolean(l,snd.IsLooping());
}

void Lua::ALSound::IsPlaying(lua_State *l,::ALSound &snd)
{
	lua_pushboolean(l,snd.IsPlaying());
}

void Lua::ALSound::IsStopped(lua_State *l,::ALSound &snd) {Lua::PushBool(l,snd.IsStopped());}

void Lua::ALSound::GetState(lua_State *l,::ALSound &snd)
{
	Lua::PushInt(l,snd.GetState());
}

void Lua::ALSound::Rewind(lua_State*,::ALSound &snd)
{
	snd.Rewind();
}

void Lua::ALSound::Pause(lua_State*,::ALSound &snd)
{
	snd.Pause();
}

void Lua::ALSound::IsPaused(lua_State *l,::ALSound &snd)
{
	lua_pushboolean(l,snd.IsPaused());
}

void Lua::ALSound::SetGain(lua_State*,::ALSound &snd,float gain)
{
	snd.SetGain(gain);
}

void Lua::ALSound::GetGain(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetGain());
}

void Lua::ALSound::SetPos(lua_State*,::ALSound &snd,Vector3 pos)
{
	snd.SetPosition(pos);
}

void Lua::ALSound::GetPos(lua_State *l,::ALSound &snd)
{
	Vector3 pos = snd.GetPosition();
	luabind::object(l,pos).push(l);
}

void Lua::ALSound::SetVelocity(lua_State*,::ALSound &snd,Vector3 vel)
{
	snd.SetVelocity(vel);
}

void Lua::ALSound::GetVelocity(lua_State *l,::ALSound &snd)
{
	Vector3 vel = snd.GetVelocity();
	luabind::object(l,vel).push(l);
}

void Lua::ALSound::SetDirection(lua_State*,::ALSound &snd,Vector3 dir)
{
	snd.SetDirection(dir);
}

void Lua::ALSound::GetDirection(lua_State *l,::ALSound &snd)
{
	Vector3 dir = snd.GetDirection();
	luabind::object(l,dir).push(l);
}

void Lua::ALSound::SetRelativeToListener(lua_State*,::ALSound &snd,bool b)
{
	snd.SetRelative(b);
}

void Lua::ALSound::IsRelativeToListener(lua_State *l,::ALSound &snd)
{
	lua_pushboolean(l,snd.IsRelative());
}

void Lua::ALSound::SetSecOffset(lua_State*,::ALSound &snd,float sec)
{
	snd.SetTimeOffset(sec);
}

void Lua::ALSound::GetSecOffset(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetTimeOffset());
}

void Lua::ALSound::GetDuration(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetDuration());
}

void Lua::ALSound::CallOnStateChanged(lua_State *l,::ALSound &snd,luabind::object o)
{
	Lua::CheckFunction(l,2);
	auto hCallback = snd.AddLuaCallback("OnStateChanged",o);
	Lua::Push<CallbackHandle>(l,hCallback);
}

void Lua::ALSound::SetPriority(lua_State*,::ALSound &snd,unsigned char)
{
	//snd.SetPriority(priority); // TODO
}

void Lua::ALSound::GetPriority(lua_State*,::ALSound &snd)
{
	//Lua::PushNumber(l,snd.GetPriority()); // TODO
}

void Lua::ALSound::FadeIn(lua_State*,::ALSound &snd,int time)
{
	snd.FadeIn(CFloat(time));
}

void Lua::ALSound::FadeOut(lua_State*,::ALSound &snd,int time)
{
	snd.FadeOut(CFloat(time));
}

void Lua::ALSound::GetReferenceDistance(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetReferenceDistance());
}

void Lua::ALSound::SetReferenceDistance(lua_State*,::ALSound &snd,float dist)
{
	snd.SetReferenceDistance(dist);
}

void Lua::ALSound::GetRolloffFactor(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetRolloffFactor());
}

void Lua::ALSound::SetRolloffFactor(lua_State*,::ALSound &snd,float rolloff)
{
	snd.SetRolloffFactor(rolloff);
}

void Lua::ALSound::GetRoomRolloffFactor(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetRoomRolloffFactor());
}

void Lua::ALSound::SetRoomRolloffFactor(lua_State*,::ALSound &snd,float rolloff)
{
	snd.SetRoomRolloffFactor(rolloff);
}

void Lua::ALSound::GetMaxDistance(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetMaxDistance());
}

void Lua::ALSound::SetMaxDistance(lua_State*,::ALSound &snd,float dist)
{
	snd.SetMaxDistance(dist);
}

void Lua::ALSound::GetMinGain(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetMinGain());
}

void Lua::ALSound::SetMinGain(lua_State*,::ALSound &snd,float gain)
{
	snd.SetMinGain(gain);
}

void Lua::ALSound::GetMaxGain(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetMaxGain());
}

void Lua::ALSound::SetMaxGain(lua_State*,::ALSound &snd,float gain)
{
	snd.SetMaxGain(gain);
}

void Lua::ALSound::GetConeInnerAngle(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetInnerConeAngle());
}

void Lua::ALSound::SetConeInnerAngle(lua_State*,::ALSound &snd,float ang)
{
	snd.SetInnerConeAngle(ang);
}

void Lua::ALSound::GetConeOuterAngle(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetOuterConeAngle());
}

void Lua::ALSound::SetConeOuterAngle(lua_State*,::ALSound &snd,float ang)
{
	snd.SetOuterConeAngle(ang);
}

void Lua::ALSound::GetConeOuterGain(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetOuterConeGain());
}

void Lua::ALSound::SetConeOuterGain(lua_State*,::ALSound &snd,float gain)
{
	snd.SetOuterConeGain(gain);
}

void Lua::ALSound::GetConeOuterGainHF(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetOuterConeGainHF());
}
void Lua::ALSound::SetConeOuterGainHF(lua_State *l,::ALSound &snd,float gain)
{
	snd.SetOuterConeGainHF(gain);
}

void Lua::ALSound::GetType(lua_State *l,::ALSound &snd)
{
	Lua::PushInt(l,CInt32(snd.GetType()));
}

void Lua::ALSound::SetType(lua_State*,::ALSound &snd,int type)
{
	snd.SetType(static_cast<ALSoundType>(type));
}

void Lua::ALSound::AddType(lua_State*,::ALSound &snd,int type)
{
	snd.AddType(static_cast<ALSoundType>(type));
}

void Lua::ALSound::GetMaxAudibleDistance(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetMaxAudibleDistance());
}

void Lua::ALSound::GetSoundIntensity(lua_State *l,::ALSound &snd,const Vector3 &origin)
{
	Lua::PushNumber(l,snd.GetSoundIntensity(origin));
}
void Lua::ALSound::GetSource(lua_State *l,::ALSound &snd)
{
	auto *entSrc = snd.GetSource();
	if(entSrc == nullptr)
		return;
	entSrc->GetLuaObject()->push(l);
}
void Lua::ALSound::SetSource(lua_State*,::ALSound &snd)
{
	snd.SetSource(nullptr);
}
void Lua::ALSound::SetSource(lua_State *l,::ALSound &snd,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	snd.SetSource(hEnt.get());
}
void Lua::ALSound::SetRange(lua_State *l,::ALSound &snd,float start,float end)
{
	snd.SetRange(start,end);
}
void Lua::ALSound::ClearRange(lua_State *l,::ALSound &snd)
{
	snd.ClearRange();
}
void Lua::ALSound::HasRange(lua_State *l,::ALSound &snd)
{
	Lua::PushBool(l,snd.HasRange());
}
void Lua::ALSound::GetRange(lua_State *l,::ALSound &snd)
{
	auto range = snd.GetRange();
	Lua::PushNumber(l,range.first);
	Lua::PushNumber(l,range.second);
}
void Lua::ALSound::GetRangeOffsets(lua_State *l,::ALSound &snd)
{
	auto range = snd.GetRangeOffsets();
	Lua::PushNumber(l,range.first);
	Lua::PushNumber(l,range.second);
}
void Lua::ALSound::SetFadeInDuration(lua_State *l,::ALSound &snd,float t)
{
	snd.SetFadeInDuration(t);
}
void Lua::ALSound::SetFadeOutDuration(lua_State *l,::ALSound &snd,float t)
{
	snd.SetFadeOutDuration(t);
}
void Lua::ALSound::GetFadeInDuration(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetFadeInDuration());
}
void Lua::ALSound::GetFadeOutDuration(lua_State *l,::ALSound &snd)
{
	Lua::PushNumber(l,snd.GetFadeOutDuration());
}
void Lua::ALSound::SetOrientation(lua_State *l,::ALSound &snd,const Vector3 &at,const Vector3 &up) {snd.SetOrientation(at,up);}
void Lua::ALSound::GetOrientation(lua_State *l,::ALSound &snd)
{
	auto orientation = snd.GetOrientation();
	Lua::Push<Vector3>(l,orientation.first);
	Lua::Push<Vector3>(l,orientation.second);
}
void Lua::ALSound::SetDopplerFactor(lua_State *l,::ALSound &snd,float factor) {snd.SetDopplerFactor(factor);}
void Lua::ALSound::GetDopplerFactor(lua_State *l,::ALSound &snd) {Lua::PushNumber(l,snd.GetDopplerFactor());}
void Lua::ALSound::SetLeftStereoAngle(lua_State *l,::ALSound &snd,float ang) {snd.SetLeftStereoAngle(ang);}
void Lua::ALSound::GetLeftStereoAngle(lua_State *l,::ALSound &snd) {Lua::PushNumber(l,snd.GetLeftStereoAngle());}
void Lua::ALSound::SetRightStereoAngle(lua_State *l,::ALSound &snd,float ang) {snd.SetRightStereoAngle(ang);}
void Lua::ALSound::GetRightStereoAngle(lua_State *l,::ALSound &snd) {Lua::PushNumber(l,snd.GetRightStereoAngle());}
void Lua::ALSound::SetAirAbsorptionFactor(lua_State *l,::ALSound &snd,float airAbsorption) {snd.SetAirAbsorptionFactor(airAbsorption);}
void Lua::ALSound::GetAirAbsorptionFactor(lua_State *l,::ALSound &snd) {Lua::PushNumber(l,snd.GetAirAbsorptionFactor());}
void Lua::ALSound::SetGainAuto(lua_State *l,::ALSound &snd,bool directHF,bool send,bool sendHF) {snd.SetGainAuto(directHF,send,sendHF);}
void Lua::ALSound::GetGainAuto(lua_State *l,::ALSound &snd)
{
	auto gainAuto = snd.GetGainAuto();
	Lua::PushBool(l,std::get<0>(gainAuto));
	Lua::PushBool(l,std::get<1>(gainAuto));
	Lua::PushBool(l,std::get<2>(gainAuto));
}
void Lua::ALSound::SetDirectFilter(lua_State *l,::ALSound &snd,float gain,float gainHF,float gainLF) {snd.SetDirectFilter({gain,gainHF,gainLF});}
void Lua::ALSound::SetDirectFilter(lua_State *l,::ALSound &snd,float gain) {snd.SetDirectFilter({gain});}
void Lua::ALSound::GetDirectFilter(lua_State *l,::ALSound &snd)
{
	auto &filter = snd.GetDirectFilter();
	Lua::PushNumber(l,filter.gain);
	Lua::PushNumber(l,filter.gainHF);
	Lua::PushNumber(l,filter.gainLF);
}
void Lua::ALSound::AddEffect(lua_State *l,::ALSound &snd,const std::string &name,float gain,float gainHF,float gainLF)
{
	auto r = snd.AddEffect(name,{gain,gainHF,gainLF});
	Lua::PushBool(l,r);
}
void Lua::ALSound::AddEffect(lua_State *l,::ALSound &snd,const std::string &name,float gain)
{
	auto r = snd.AddEffect(name,{gain});
	Lua::PushBool(l,r);
}
void Lua::ALSound::RemoveEffect(lua_State *l,::ALSound &snd,const std::string &name)
{
	snd.RemoveEffect(name);
}
void Lua::ALSound::SetEffectParameters(lua_State *l,::ALSound &snd,const std::string &name,float gain,float gainHF,float gainLF) {snd.SetEffectParameters(name,{gain,gainHF,gainLF});}
void Lua::ALSound::SetEffectParameters(lua_State *l,::ALSound &snd,const std::string &name,float gain) {snd.SetEffectParameters(name,{gain});}

void Lua::ALSound::SetGainRange(lua_State *l,::ALSound &snd,float minGain,float maxGain) {snd.SetGainRange(minGain,maxGain);}
void Lua::ALSound::GetGainRange(lua_State *l,::ALSound &snd)
{
	auto range = snd.GetGainRange();
	Lua::PushNumber(l,range.first);
	Lua::PushNumber(l,range.second);
}
void Lua::ALSound::SetDistanceRange(lua_State *l,::ALSound &snd,float refDist,float maxDist) {snd.SetDistanceRange(refDist,maxDist);}
void Lua::ALSound::GetDistanceRange(lua_State *l,::ALSound &snd)
{
	auto range = snd.GetDistanceRange();
	Lua::PushNumber(l,range.first);
	Lua::PushNumber(l,range.second);
}
void Lua::ALSound::SetConeAngles(lua_State *l,::ALSound &snd,float inner,float outer) {snd.SetConeAngles(inner,outer);}
void Lua::ALSound::GetConeAngles(lua_State *l,::ALSound &snd)
{
	auto angles = snd.GetConeAngles();
	Lua::PushNumber(l,angles.first);
	Lua::PushNumber(l,angles.second);
}
void Lua::ALSound::GetOuterConeGains(lua_State *l,::ALSound &snd)
{
	auto gains = snd.GetOuterConeGains();
	Lua::PushNumber(l,gains.first);
	Lua::PushNumber(l,gains.second);
}
void Lua::ALSound::SetStereoAngles(lua_State *l,::ALSound &snd,float leftAng,float rightAng) {snd.SetStereoAngles(leftAng,rightAng);}
void Lua::ALSound::GetStereoAngles(lua_State *l,::ALSound &snd)
{
	auto angles = snd.GetStereoAngles();
	Lua::PushNumber(l,angles.first);
	Lua::PushNumber(l,angles.second);
}
void Lua::ALSound::SetOuterConeGains(lua_State *l,::ALSound &snd,float gain,float gainHF) {snd.SetOuterConeGains(gain,gainHF);}
void Lua::ALSound::GetDirectGainHFAuto(lua_State *l,::ALSound &snd) {Lua::PushBool(l,snd.GetDirectGainHFAuto());}
void Lua::ALSound::GetSendGainAuto(lua_State *l,::ALSound &snd) {Lua::PushBool(l,snd.GetSendGainAuto());}
void Lua::ALSound::GetSendGainHFAuto(lua_State *l,::ALSound &snd) {Lua::PushBool(l,snd.GetSendGainHFAuto());}
void Lua::ALSound::SetEffectGain(lua_State *l,::ALSound &snd,const std::string &name,float gain) {snd.SetEffectGain(name,gain);}
void Lua::ALSound::GetRolloffFactors(lua_State *l,::ALSound &snd)
{
	auto factors = snd.GetRolloffFactors();
	Lua::PushNumber(l,factors.first);
	Lua::PushNumber(l,factors.second);
}
void Lua::ALSound::SetRolloffFactors(lua_State *l,::ALSound &snd,float factor,float roomFactor) {snd.SetRolloffFactors(factor,roomFactor);}
