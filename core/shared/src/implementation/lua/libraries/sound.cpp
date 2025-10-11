// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/umath.h"

#include "algorithm"

#include "sharedutils/util.h"

#include "pragma/lua/luaapi.h"

#include "mathutil/uvec.h"

#include "luasystem.h"
#include <sharedutils/util_markup_file.hpp>

module pragma.shared;

import :scripting.lua.libraries.sound;

import se_script;

void Lua::sound::register_enums(lua_State *l)
{
	Lua::RegisterLibraryEnums(l, "sound",
	  {{"FCREATE_NONE", umath::to_integral(ALCreateFlags::None)}, {"FCREATE_MONO", umath::to_integral(ALCreateFlags::Mono)}, {"FCREATE_STREAM", umath::to_integral(ALCreateFlags::Stream)}, {"FCREATE_DONT_TRANSMIT", umath::to_integral(ALCreateFlags::DontTransmit)}});
}

void Lua::sound::register_library(luabind::module_ &soundMod)
{
	auto defPlayInfo = luabind::class_<SoundPlayInfo>("PlayInfo");
	defPlayInfo.def(luabind::constructor<>());
	defPlayInfo.def_readwrite("gain", &SoundPlayInfo::gain);
	defPlayInfo.def_readwrite("pitch", &SoundPlayInfo::pitch);
	defPlayInfo.def_readwrite("origin", &SoundPlayInfo::origin);
	defPlayInfo.def_readwrite("flags", &SoundPlayInfo::flags);
	soundMod[defPlayInfo];

	soundMod[luabind::def("create", static_cast<std::shared_ptr<::ALSound>(*)(lua_State *, const std::string &, ALSoundType, ALCreateFlags)>(&Lua::sound::create))];
	soundMod[luabind::def("create", static_cast<std::shared_ptr<::ALSound>(*)(lua_State *, const std::string &, ALSoundType)>(&Lua::sound::create))];
	soundMod[luabind::def("play", static_cast<std::shared_ptr<::ALSound>(*)(lua_State *, const std::string &, ALSoundType, const SoundPlayInfo &)>(&Lua::sound::play))];
	soundMod[luabind::def("play", static_cast<std::shared_ptr<::ALSound>(*)(lua_State *, const std::string &, ALSoundType)>(&Lua::sound::play))];
	soundMod[luabind::def("get_duration", &Lua::sound::get_duration)];
	soundMod[luabind::def("get_all", &Lua::sound::get_all)];
	soundMod[luabind::def("is_music_playing", &Lua::sound::is_music_playing)];
	soundMod[luabind::def("find_by_type", static_cast<std::vector<std::shared_ptr<::ALSound>>(*)(lua_State *, ALSoundType, bool)>(&Lua::sound::find_by_type))];
	soundMod[luabind::def("find_by_type", static_cast<std::vector<std::shared_ptr<::ALSound>>(*)(lua_State *, ALSoundType)>(&Lua::sound::find_by_type))];
	soundMod[luabind::def("precache", static_cast<bool(*)(lua_State *, const std::string &, ALChannel)>(&Lua::sound::precache))];
	soundMod[luabind::def("precache", static_cast<bool(*)(lua_State *, const std::string &)>(&Lua::sound::precache))];
	soundMod[luabind::def("stop_all", &Lua::sound::stop_all)];
	soundMod[luabind::def("load_scripts", &Lua::sound::load_scripts)];
	soundMod[luabind::def("read_wav_phonemes", &Lua::sound::read_wav_phonemes)];
}

std::shared_ptr<::ALSound> Lua::sound::create(lua_State *l, const std::string &snd, ALSoundType type, ALCreateFlags flags)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	auto pAl = state->CreateSound(snd, type, flags);
	if(pAl == nullptr)
		return nullptr;
	pAl->SetType(type);
	return pAl;
}

std::shared_ptr<::ALSound> Lua::sound::create(lua_State *l, const std::string &snd, ALSoundType type)
{
	return create(l, snd, type, ALCreateFlags::None);
}

std::shared_ptr<::ALSound> Lua::sound::play(lua_State *l, const std::string &sndName, ALSoundType type, const SoundPlayInfo &playInfo)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	auto snd = state->CreateSound(sndName, type, playInfo.flags);
	if(snd == nullptr)
		return nullptr;
	if(playInfo.origin) {
		snd->SetPosition(*playInfo.origin);
		snd->SetRelative(false);
	}
	else
		snd->SetRelative(true);
	snd->SetGain(playInfo.gain);
	snd->SetPitch(playInfo.pitch);
	snd->Play();
	return snd;
}

std::shared_ptr<::ALSound> Lua::sound::play(lua_State *l, const std::string &sndName, ALSoundType type) { return play(l, sndName, type, {}); }

bool Lua::sound::is_music_playing(lua_State *l)
{
	auto *state = Engine::Get()->GetNetworkState(l);
	auto &sounds = state->GetSounds();
	auto it = std::find_if(sounds.begin(), sounds.end(), [](ALSoundRef &rsnd) {
		auto &snd = rsnd.get();
		return (snd.IsPlaying() == true && (snd.GetType() & ALSoundType::Music) != ALSoundType::Generic) ? true : false;
	});
	return it != sounds.end();
}

float Lua::sound::get_duration(lua_State *l, const std::string &snd)
{
	NetworkState *state = Engine::Get()->GetNetworkState(l);
	return state->GetSoundDuration(snd);
}

std::vector<std::shared_ptr<::ALSound>> Lua::sound::get_all(lua_State *l)
{
	NetworkState *state = Engine::Get()->GetNetworkState(l);
	auto &sounds = state->GetSounds();
	std::vector<std::shared_ptr<::ALSound>> rsounds;
	rsounds.reserve(sounds.size());
	for(auto &rsnd : sounds) {
		auto &snd = rsnd.get();
		rsounds.push_back(snd.shared_from_this());
	}
	return rsounds;
}

std::vector<std::shared_ptr<::ALSound>> Lua::sound::find_by_type(lua_State *l, ALSoundType type, bool bExactMatch)
{
	if(type == ALSoundType::Generic)
		return get_all(l);
	auto *state = Engine::Get()->GetNetworkState(l);
	auto &sounds = state->GetSounds();
	std::vector<std::shared_ptr<::ALSound>> rsounds;
	auto t = Lua::CreateTable(l);
	int32_t n = 1;
	for(auto &rsnd : sounds) {
		auto &snd = rsnd.get();
		if((bExactMatch == false && (snd.GetType() & type) != ALSoundType::Generic) || (bExactMatch == true && snd.GetType() == type))
			rsounds.push_back(snd.shared_from_this());
	}
	return rsounds;
}
std::vector<std::shared_ptr<::ALSound>> Lua::sound::find_by_type(lua_State *l, ALSoundType type)
{
	return find_by_type(l, type, false);
}

bool Lua::sound::precache(lua_State *l, const std::string &snd, ALChannel mode)
{
	NetworkState *state = Engine::Get()->GetNetworkState(l);
	return state->PrecacheSound(snd.c_str(), mode);
}
bool Lua::sound::precache(lua_State *l, const std::string &snd)
{
	return precache(l, snd, ALChannel::Auto);
}

void Lua::sound::stop_all(lua_State *l)
{
	NetworkState *state = Engine::Get()->GetNetworkState(l);
	state->StopSounds();
}

void Lua::sound::load_scripts(lua_State *l, const std::string &file)
{
	NetworkState *state = Engine::Get()->GetNetworkState(l);
	state->LoadSoundScripts(file.c_str());
}

luabind::object Lua::sound::read_wav_phonemes(lua_State *l, const std::string &fileName)
{
	auto fname = "sounds\\" + FileManager::GetCanonicalizedPath(fileName);
	auto f = FileManager::OpenFile(fname.c_str(), "rb");
	if(f == nullptr)
		return Lua::nil;
	source_engine::script::SoundPhonemeData sp {};
	if(source_engine::script::read_wav_phonemes(f, sp) != ::util::MarkupFile::ResultCode::Ok)
		return Lua::nil;

	auto t = luabind::newtable(l);

	auto tWords = luabind::newtable(l);
	t["words"] = tWords;

	auto wordId = 1u;
	for(auto &word : sp.words) {
		auto tWord = luabind::newtable(l);
		tWords[wordId++] = tWord;

		tWord["startTime"] = word.tStart;
		tWord["endTime"] = word.tEnd;
		tWord["word"] = word.word;

		auto tPhonemes = luabind::newtable(l);
		tWord["phonemes"] = tPhonemes;

		auto phonemeId = 1u;
		for(auto &phoneme : word.phonemes) {
			auto tPhoneme = luabind::newtable(l);
			tPhonemes[phonemeId++] = tPhoneme;

			tPhoneme["phoneme"] = phoneme.phoneme;
			tPhoneme["startTime"] = phoneme.tStart;
			tPhoneme["endTime"] = phoneme.tEnd;
		}
	}
	return t;
}
