/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/audio/c_lalsound.hpp"
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
			static void GetLoopFramePoints(lua_State *l,::ALSound &snd);
			static void GetLoopTimePoints(lua_State *l,::ALSound &snd);

			static void SetPropagationIdentifier(lua_State *l,::ALSound &snd,const std::string &identifier);
			static void GetPropagationIdentifier(lua_State *l,::ALSound &snd);
			static void GetBuffer(lua_State *l,::ALSound &snd);
		};
	};
	namespace ALBuffer
	{
		static void GetPhonemeData(lua_State *l,al::ISoundBuffer&);
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
	classDef.def("GetLoopFramePoints",&Lua::ALSound::Client::GetLoopFramePoints);
	classDef.def("GetLoopTimePoints",&Lua::ALSound::Client::GetLoopTimePoints);
	classDef.def("SetPropagationIdentifier",&Lua::ALSound::Client::SetPropagationIdentifier);
	classDef.def("GetPropagationIdentifier",&Lua::ALSound::Client::GetPropagationIdentifier);
	classDef.def("GetBuffer",&Lua::ALSound::Client::GetBuffer);
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
void Lua::ALSound::Client::GetLoopFramePoints(lua_State *l,::ALSound &snd)
{
	auto points = static_cast<CALSound&>(snd)->GetLoopFramePoints();
	Lua::PushInt(l,points.first);
	Lua::PushInt(l,points.second);
}
void Lua::ALSound::Client::GetLoopTimePoints(lua_State *l,::ALSound &snd)
{
	auto points = static_cast<CALSound&>(snd)->GetLoopTimePoints();
	Lua::PushNumber(l,points.first);
	Lua::PushNumber(l,points.second);
}
void Lua::ALSound::Client::SetPropagationIdentifier(lua_State *l,::ALSound &snd,const std::string &identifier) {static_cast<CALSound&>(snd)->SetIdentifier(identifier);}
void Lua::ALSound::Client::GetPropagationIdentifier(lua_State *l,::ALSound &snd) {Lua::PushString(l,static_cast<CALSound&>(snd)->GetIdentifier());}
void Lua::ALSound::Client::GetBuffer(lua_State *l,::ALSound &snd)
{
	auto *pBuffer = static_cast<CALSound&>(snd)->GetBuffer();
	if(pBuffer == nullptr)
		return;
	Lua::Push<al::PSoundBuffer>(l,pBuffer->shared_from_this());
}

/////////////////

void Lua::ALBuffer::GetPhonemeData(lua_State *l,al::ISoundBuffer &buffer)
{
	auto pUserData = buffer.GetUserData();
	if(pUserData == nullptr)
		return;
	auto &phonemeData = *static_cast<se::SoundPhonemeData*>(pUserData.get());

	auto t = Lua::CreateTable(l);

	Lua::PushString(l,"plainText");
	Lua::PushString(l,phonemeData.plainText);
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"words");
	auto tWords = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &word : phonemeData.words)
	{
		Lua::PushInt(l,idx++);
		auto tWord = Lua::CreateTable(l);

		Lua::PushString(l,"tStart");
		Lua::PushNumber(l,word.tStart);
		Lua::SetTableValue(l,tWord);

		Lua::PushString(l,"tEnd");
		Lua::PushNumber(l,word.tEnd);
		Lua::SetTableValue(l,tWord);

		Lua::PushString(l,"word");
		Lua::PushString(l,word.word);
		Lua::SetTableValue(l,tWord);

		Lua::PushString(l,"phonemes");
		auto tPhonemes = Lua::CreateTable(l);
		auto idxPhoneme = 1u;
		for(auto &phoneme : word.phonemes)
		{
			Lua::PushInt(l,idxPhoneme++);
			auto tPhoneme = Lua::CreateTable(l);

			Lua::PushString(l,"phoneme");
			Lua::PushString(l,phoneme.phoneme);
			Lua::SetTableValue(l,tPhoneme);

			Lua::PushString(l,"tStart");
			Lua::PushNumber(l,phoneme.tStart);
			Lua::SetTableValue(l,tPhoneme);

			Lua::PushString(l,"tEnd");
			Lua::PushNumber(l,phoneme.tEnd);
			Lua::SetTableValue(l,tPhoneme);

			Lua::SetTableValue(l,tPhonemes);
		}
		Lua::SetTableValue(l,tWord);

		Lua::SetTableValue(l,tWords);
	}
	Lua::SetTableValue(l,t);
}
