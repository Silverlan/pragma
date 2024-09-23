/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lsound.h"
#include <pragma/engine.h>
#include "pragma/lua/classes/ldef_vector.h"
#include "luasystem.h"
#include "pragma/audio/alsound.h"
#include "pragma/audio/alsound_type.h"
#include <sharedutils/util_markup_file.hpp>

import se_script;

extern DLLNETWORK Engine *engine;

void Lua::sound::register_enums(lua_State *l)
{
	Lua::RegisterLibraryEnums(l, "sound",
	  {{"FCREATE_NONE", umath::to_integral(ALCreateFlags::None)}, {"FCREATE_MONO", umath::to_integral(ALCreateFlags::Mono)}, {"FCREATE_STREAM", umath::to_integral(ALCreateFlags::Stream)}, {"FCREATE_DONT_TRANSMIT", umath::to_integral(ALCreateFlags::DontTransmit)}});
}

int Lua::sound::create(lua_State *l, const std::function<std::shared_ptr<ALSound>(NetworkState *, const std::string &, ALSoundType, ALCreateFlags)> &f)
{
	auto *state = engine->GetNetworkState(l);
	int32_t argId = 1;
	auto *snd = Lua::CheckString(l, argId++);
	auto type = static_cast<ALSoundType>(Lua::CheckInt(l, argId++));
	auto flags = ALCreateFlags::None;
	if(Lua::IsSet(l, argId))
		flags = static_cast<ALCreateFlags>(Lua::CheckInt(l, argId++));
	auto pAl = f(state, snd, type, flags);
	if(pAl == nullptr)
		return 0;
	pAl->SetType(type);
	luabind::object(l, pAl).push(l);
	return 1;
}

int Lua::sound::create(lua_State *l)
{
	return create(l, [](NetworkState *nw, const std::string &name, ALSoundType type, ALCreateFlags flags) -> std::shared_ptr<ALSound> { return nw->CreateSound(name, type, flags); });
}

int Lua::sound::play(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	int32_t argId = 1;
	auto *sndName = Lua::CheckString(l, argId++);
	auto type = static_cast<ALSoundType>(Lua::CheckInt(l, argId++));
	auto flags = ALCreateFlags::None;
	if(Lua::IsSet(l, argId))
		flags = static_cast<ALCreateFlags>(Lua::CheckInt(l, argId++));
	auto gain = 1.f;
	auto pitch = 1.f;
	Vector3 *origin = nullptr;
	if(Lua::IsVector(l, argId))
		origin = Lua::CheckVector(l, argId++);
	else {
		if(Lua::IsSet(l, argId))
			gain = Lua::CheckNumber(l, argId++);
		if(Lua::IsSet(l, argId))
			pitch = Lua::CheckNumber(l, argId++);
		if(Lua::IsSet(l, argId))
			origin = Lua::CheckVector(l, argId++);
	}

	auto snd = state->CreateSound(sndName, type, flags);
	if(snd == nullptr)
		return 0;
	if(origin != nullptr) {
		snd->SetPosition(*origin);
		snd->SetRelative(false);
	}
	else
		snd->SetRelative(true);
	snd->SetGain(gain);
	snd->SetPitch(pitch);
	snd->Play();
	Lua::Push<std::shared_ptr<ALSound>>(l, snd);
	return 1;
}

int Lua::sound::is_music_playing(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto &sounds = state->GetSounds();
	auto it = std::find_if(sounds.begin(), sounds.end(), [](ALSoundRef &rsnd) {
		auto &snd = rsnd.get();
		return (snd.IsPlaying() == true && (snd.GetType() & ALSoundType::Music) != ALSoundType::Generic) ? true : false;
	});
	Lua::PushBool(l, (it != sounds.end()) ? true : false);
	return 1;
}

int Lua::sound::get_duration(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	std::string snd = luaL_checkstring(l, 1);
	float dur = state->GetSoundDuration(snd);
	Lua::PushNumber(l, dur);
	return 1;
}

int Lua::sound::get_all(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	auto &sounds = state->GetSounds();
	lua_newtable(l);
	int top = lua_gettop(l);
	int n = 1;
	for(auto &rsnd : sounds) {
		auto &snd = rsnd.get();
		luabind::object(l, snd.shared_from_this()).push(l);
		lua_rawseti(l, top, n);
		n++;
	}
	return 1;
}

int Lua::sound::find_by_type(lua_State *l)
{
	auto type = static_cast<ALSoundType>(Lua::CheckInt(l, 1));
	if(type == ALSoundType::Generic)
		return get_all(l);
	auto bExactMatch = false;
	if(Lua::IsSet(l, 2))
		bExactMatch = Lua::CheckBool(l, 2);
	auto *state = engine->GetNetworkState(l);
	auto &sounds = state->GetSounds();
	auto t = Lua::CreateTable(l);
	int32_t n = 1;
	for(auto &rsnd : sounds) {
		auto &snd = rsnd.get();
		if((bExactMatch == false && (snd.GetType() & type) != ALSoundType::Generic) || (bExactMatch == true && snd.GetType() == type)) {
			Lua::PushInt(l, n);
			Lua::Push<std::shared_ptr<ALSound>>(l, snd.shared_from_this());
			Lua::SetTableValue(l, t);
			n++;
		}
	}
	return 1;
}

int Lua::sound::precache(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	std::string snd = luaL_checkstring(l, 1);
	auto mode = ALChannel::Auto;
	if(Lua::IsSet(l, 2))
		mode = static_cast<ALChannel>(Lua::CheckInt<int>(l, 2));
	Lua::PushBool(l, state->PrecacheSound(snd.c_str(), mode));
	return 1;
}

int Lua::sound::stop_all(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	state->StopSounds();
	return 0;
}

int Lua::sound::load_scripts(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	std::string file = luaL_checkstring(l, 1);
	state->LoadSoundScripts(file.c_str());
	return 0;
}

int Lua::sound::read_wav_phonemes(lua_State *l)
{
	auto fname = "sounds\\" + FileManager::GetCanonicalizedPath(Lua::CheckString(l, 1));
	auto f = FileManager::OpenFile(fname.c_str(), "rb");
	if(f == nullptr)
		return 0;
	source_engine::script::SoundPhonemeData sp {};
	if(source_engine::script::read_wav_phonemes(f, sp) != util::MarkupFile::ResultCode::Ok)
		return 0;

	auto t = Lua::CreateTable(l);

	Lua::PushString(l, "plainText");
	Lua::PushString(l, sp.plainText);
	Lua::SetTableValue(l, t);

	Lua::PushString(l, "words");
	auto tWords = Lua::CreateTable(l);

	auto wordId = 1u;
	for(auto &word : sp.words) {
		Lua::PushInt(l, wordId++);
		auto tWord = Lua::CreateTable(l);

		Lua::PushString(l, "startTime");
		Lua::PushNumber(l, word.tStart);
		Lua::SetTableValue(l, tWord);

		Lua::PushString(l, "endTime");
		Lua::PushNumber(l, word.tEnd);
		Lua::SetTableValue(l, tWord);

		Lua::PushString(l, "word");
		Lua::PushString(l, word.word);
		Lua::SetTableValue(l, tWord);

		Lua::PushString(l, "phonemes");
		auto tPhonemes = Lua::CreateTable(l);

		auto phonemeId = 1u;
		for(auto &phoneme : word.phonemes) {
			Lua::PushInt(l, phonemeId++);
			auto tPhoneme = Lua::CreateTable(l);

			Lua::PushString(l, "phoneme");
			Lua::PushString(l, phoneme.phoneme);
			Lua::SetTableValue(l, tPhoneme);

			Lua::PushString(l, "startTime");
			Lua::PushNumber(l, phoneme.tStart);
			Lua::SetTableValue(l, tPhoneme);

			Lua::PushString(l, "endTime");
			Lua::PushNumber(l, phoneme.tEnd);
			Lua::SetTableValue(l, tPhoneme);

			Lua::SetTableValue(l, tPhonemes);
		}

		Lua::SetTableValue(l, tWord);
		Lua::SetTableValue(l, tWords);
	}

	Lua::SetTableValue(l, t);
	return 1;
}
