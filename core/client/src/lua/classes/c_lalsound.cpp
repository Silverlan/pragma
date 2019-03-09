#include "stdafx_client.h"
#include "pragma/audio/c_lalsound.hpp"
#include <se_scene.hpp>

namespace Lua
{
	namespace ALSound
	{
		namespace Client
		{
			static void GetWorldPosition(lua_State *l,std::shared_ptr<::ALSound> pAl);

			static void GetFrameLength(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void SetFrameOffset(lua_State *l,std::shared_ptr<::ALSound> pAl,uint64_t offset);
			static void GetFrameOffset(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetInverseFrequency(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetChannelConfigName(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetSampleTypeName(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void IsMono(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void IsStereo(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetFrequency(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetChannelConfig(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetSampleType(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetLength(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetLoopFramePoints(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetLoopTimePoints(lua_State *l,std::shared_ptr<::ALSound> pAl);

			static void SetPropagationIdentifier(lua_State *l,std::shared_ptr<::ALSound> pAl,const std::string &identifier);
			static void GetPropagationIdentifier(lua_State *l,std::shared_ptr<::ALSound> pAl);
			static void GetBuffer(lua_State *l,std::shared_ptr<::ALSound> pAl);
		};
	};
	namespace ALBuffer
	{
		static void GetPhonemeData(lua_State *l,al::PSoundBuffer);
	};
};

void Lua::ALSound::Client::register_buffer(luabind::class_<al::PSoundBuffer> &classDef)
{
	classDef.def("GetPhonemeData",&Lua::ALBuffer::GetPhonemeData);
}

void Lua::ALSound::Client::register_class(luabind::class_<std::shared_ptr<::ALSound>> &classDef)
{
	ALSound::register_class(classDef);
	classDef.def("GetWorldPos",&Lua::ALSound::Client::GetWorldPosition);

	classDef.def("GetFrameLength",&Lua::ALSound::Client::GetFrameLength);
	classDef.def("SetFrameOffset",&Lua::ALSound::Client::SetFrameOffset);
	classDef.def("GetFrameOffset",&Lua::ALSound::Client::GetFrameOffset);
	classDef.def("GetInverseFrequency",&Lua::ALSound::Client::GetInverseFrequency);
	classDef.def("GetChannelConfigName",&Lua::ALSound::Client::GetChannelConfigName);
	classDef.def("GetSampleTypeName",&Lua::ALSound::Client::GetSampleTypeName);
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

void Lua::ALSound::Client::GetWorldPosition(lua_State *l,std::shared_ptr<::ALSound> pAl)
{
	auto *snd = static_cast<CALSound*>(pAl.get());
	Lua::Push<Vector3>(l,snd->GetWorldPosition());
}

void Lua::ALSound::Client::GetFrameLength(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushInt(l,static_cast<CALSound*>(pAl.get())->GetFrameLength());}
void Lua::ALSound::Client::SetFrameOffset(lua_State *l,std::shared_ptr<::ALSound> pAl,uint64_t offset) {static_cast<CALSound*>(pAl.get())->SetFrameOffset(offset);}
void Lua::ALSound::Client::GetFrameOffset(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushInt(l,static_cast<CALSound*>(pAl.get())->GetFrameOffset());}
void Lua::ALSound::Client::GetInverseFrequency(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushNumber(l,static_cast<CALSound*>(pAl.get())->GetInverseFrequency());}
void Lua::ALSound::Client::GetChannelConfigName(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushString(l,static_cast<CALSound*>(pAl.get())->GetChannelConfigName());}
void Lua::ALSound::Client::GetSampleTypeName(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushString(l,static_cast<CALSound*>(pAl.get())->GetSampleTypeName());}
void Lua::ALSound::Client::IsMono(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushBool(l,static_cast<CALSound*>(pAl.get())->IsMono());}
void Lua::ALSound::Client::IsStereo(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushBool(l,static_cast<CALSound*>(pAl.get())->IsStereo());}
void Lua::ALSound::Client::GetFrequency(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushInt(l,static_cast<CALSound*>(pAl.get())->GetFrequency());}
void Lua::ALSound::Client::GetChannelConfig(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushInt(l,umath::to_integral(static_cast<CALSound*>(pAl.get())->GetChannelConfig()));}
void Lua::ALSound::Client::GetSampleType(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushInt(l,umath::to_integral(static_cast<CALSound*>(pAl.get())->GetSampleType()));}
void Lua::ALSound::Client::GetLength(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushInt(l,static_cast<CALSound*>(pAl.get())->GetLength());}
void Lua::ALSound::Client::GetLoopFramePoints(lua_State *l,std::shared_ptr<::ALSound> pAl)
{
	auto points = static_cast<CALSound*>(pAl.get())->GetLoopFramePoints();
	Lua::PushInt(l,points.first);
	Lua::PushInt(l,points.second);
}
void Lua::ALSound::Client::GetLoopTimePoints(lua_State *l,std::shared_ptr<::ALSound> pAl)
{
	auto points = static_cast<CALSound*>(pAl.get())->GetLoopTimePoints();
	Lua::PushNumber(l,points.first);
	Lua::PushNumber(l,points.second);
}
void Lua::ALSound::Client::SetPropagationIdentifier(lua_State *l,std::shared_ptr<::ALSound> pAl,const std::string &identifier) {static_cast<CALSound*>(pAl.get())->SetIdentifier(identifier);}
void Lua::ALSound::Client::GetPropagationIdentifier(lua_State *l,std::shared_ptr<::ALSound> pAl) {Lua::PushString(l,static_cast<CALSound*>(pAl.get())->GetIdentifier());}
void Lua::ALSound::Client::GetBuffer(lua_State *l,std::shared_ptr<::ALSound> pAl)
{
	auto *pBuffer = static_cast<CALSound*>(pAl.get())->GetBuffer();
	if(pBuffer == nullptr)
		return;
	Lua::Push<al::PSoundBuffer>(l,pBuffer->shared_from_this());
}

/////////////////

void Lua::ALBuffer::GetPhonemeData(lua_State *l,al::PSoundBuffer buffer)
{
	auto pUserData = buffer->GetUserData();
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
