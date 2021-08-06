/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/audio/c_lalsound.hpp"
#include <pragma/lua/policies/pair_policy.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <se_scene.hpp>

namespace Lua
{
	namespace ALSound
	{
		namespace Client
		{
			static void GetWorldPosition(lua_State *l,::ALSound &snd);

			static void GetFrameLength(lua_State *l,::ALSound &snd);
			static void SetFrameOffset(lua_State *l,::ALSound &snd,uint64_t offset);
			static void GetFrameOffset(lua_State *l,::ALSound &snd);
			static void GetInverseFrequency(lua_State *l,::ALSound &snd);
			//static void GetChannelConfigName(lua_State *l,::ALSound &snd);
			//static void GetSampleTypeName(lua_State *l,::ALSound &snd);
			static void IsMono(lua_State *l,::ALSound &snd);
			static void IsStereo(lua_State *l,::ALSound &snd);
			static void GetFrequency(lua_State *l,::ALSound &snd);
			static void GetChannelConfig(lua_State *l,::ALSound &snd);
			static void GetSampleType(lua_State *l,::ALSound &snd);
			static void GetLength(lua_State *l,::ALSound &snd);
		};
	};
	namespace ALBuffer
	{
		static Lua::opt<luabind::tableT<void>> GetPhonemeData(lua_State *l,al::ISoundBuffer&);
	};
};

void Lua::ALSound::Client::register_buffer(luabind::class_<al::ISoundBuffer> &classDef)
{
	classDef.def("GetPhonemeData",&Lua::ALBuffer::GetPhonemeData);
}

void Lua::ALSound::Client::register_class(luabind::class_<::ALSound> &classDef)
{
	ALSound::register_class(classDef);
	classDef.def("GetWorldPos",&Lua::ALSound::Client::GetWorldPosition);

	classDef.def("GetFrameLength",&Lua::ALSound::Client::GetFrameLength);
	classDef.def("SetFrameOffset",&Lua::ALSound::Client::SetFrameOffset);
	classDef.def("GetFrameOffset",&Lua::ALSound::Client::GetFrameOffset);
	classDef.def("GetInverseFrequency",&Lua::ALSound::Client::GetInverseFrequency);
	//classDef.def("GetChannelConfigName",&Lua::ALSound::Client::GetChannelConfigName);
	//classDef.def("GetSampleTypeName",&Lua::ALSound::Client::GetSampleTypeName);
	classDef.def("IsMono",&Lua::ALSound::Client::IsMono);
	classDef.def("IsStereo",&Lua::ALSound::Client::IsStereo);
	classDef.def("GetFrequency",&Lua::ALSound::Client::GetFrequency);
	classDef.def("GetChannelConfig",&Lua::ALSound::Client::GetChannelConfig);
	classDef.def("GetSampleType",&Lua::ALSound::Client::GetSampleType);
	classDef.def("GetLength",&Lua::ALSound::Client::GetLength);
	classDef.def("GetLoopFramePoints",static_cast<std::pair<uint64_t,uint64_t>(*)(::ALSound&)>([](::ALSound &sound) {
		return static_cast<CALSound&>(sound)->GetLoopFramePoints();
	}));
	classDef.def("GetLoopTimePoints",static_cast<std::pair<float,float>(*)(::ALSound&)>([](::ALSound &sound) {
		return static_cast<CALSound&>(sound)->GetLoopTimePoints();
	}));
	classDef.def("SetPropagationIdentifier",static_cast<void(*)(::ALSound&,const std::string&)>([](::ALSound &sound,const std::string &identifier) {
		static_cast<CALSound&>(sound)->SetIdentifier(identifier);
	}));
	classDef.def("GetPropagationIdentifier",static_cast<std::string(*)(::ALSound&)>([](::ALSound &sound) {
		return static_cast<CALSound&>(sound)->GetIdentifier();
	}));
	classDef.def("GetBuffer",static_cast<al::ISoundBuffer*(*)(::ALSound&)>([](::ALSound &sound) -> al::ISoundBuffer* {
		return static_cast<CALSound&>(sound)->GetBuffer();
	}));
}

void Lua::ALSound::Client::GetWorldPosition(lua_State *l,::ALSound &snd)
{
	Lua::Push<Vector3>(l,static_cast<CALSound&>(snd)->GetWorldPosition());
}

void Lua::ALSound::Client::GetFrameLength(lua_State *l,::ALSound &snd) {Lua::PushInt(l,static_cast<CALSound&>(snd)->GetFrameLength());}
void Lua::ALSound::Client::SetFrameOffset(lua_State *l,::ALSound &snd,uint64_t offset) {static_cast<CALSound&>(snd)->SetFrameOffset(offset);}
void Lua::ALSound::Client::GetFrameOffset(lua_State *l,::ALSound &snd) {Lua::PushInt(l,static_cast<CALSound&>(snd)->GetFrameOffset());}
void Lua::ALSound::Client::GetInverseFrequency(lua_State *l,::ALSound &snd) {Lua::PushNumber(l,static_cast<CALSound&>(snd)->GetInverseFrequency());}
//void Lua::ALSound::Client::GetChannelConfigName(lua_State *l,::ALSound &snd) {Lua::PushString(l,static_cast<CALSound&>(snd)->GetChannelConfigName());}
//void Lua::ALSound::Client::GetSampleTypeName(lua_State *l,::ALSound &snd) {Lua::PushString(l,static_cast<CALSound&>(snd)->GetSampleTypeName());}
void Lua::ALSound::Client::IsMono(lua_State *l,::ALSound &snd) {Lua::PushBool(l,static_cast<CALSound&>(snd)->IsMono());}
void Lua::ALSound::Client::IsStereo(lua_State *l,::ALSound &snd) {Lua::PushBool(l,static_cast<CALSound&>(snd)->IsStereo());}
void Lua::ALSound::Client::GetFrequency(lua_State *l,::ALSound &snd) {Lua::PushInt(l,static_cast<CALSound&>(snd)->GetFrequency());}
void Lua::ALSound::Client::GetChannelConfig(lua_State *l,::ALSound &snd) {Lua::PushInt(l,umath::to_integral(static_cast<CALSound&>(snd)->GetChannelConfig()));}
void Lua::ALSound::Client::GetSampleType(lua_State *l,::ALSound &snd) {Lua::PushInt(l,umath::to_integral(static_cast<CALSound&>(snd)->GetSampleType()));}
void Lua::ALSound::Client::GetLength(lua_State *l,::ALSound &snd) {Lua::PushInt(l,static_cast<CALSound&>(snd)->GetLength());}

/////////////////

Lua::opt<luabind::tableT<void>> Lua::ALBuffer::GetPhonemeData(lua_State *l,al::ISoundBuffer &buffer)
{
	auto pUserData = buffer.GetUserData();
	if(pUserData == nullptr)
		return nil;
	auto &phonemeData = *static_cast<se::SoundPhonemeData*>(pUserData.get());
	
	auto t = luabind::newtable(l);
	t["plainText"] = phonemeData.plainText;

	auto tWords = t["words"];
	auto idx = 1u;
	for(auto &word : phonemeData.words)
	{
		auto tWord = tWords[idx++];

		tWord["tStart"] = word.tStart;
		tWord["tEnd"] = word.tEnd;
		tWord["word"] = word.word;

		auto tPhonemes = tWord["phonemes"];
		auto idxPhoneme = 1u;
		for(auto &phoneme : word.phonemes)
		{
			auto tPhoneme = tPhonemes[idxPhoneme++];
			tPhoneme["phoneme"] = phoneme.phoneme;
			tPhoneme["tStart"] = phoneme.tStart;
			tPhoneme["tEnd"] = phoneme.tEnd;
		}
	}
	return t;
}
