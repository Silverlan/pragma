// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.sound;

import se_script;

void Lua::sound::register_enums(lua::State *l)
{
	RegisterLibraryEnums(l, "sound",
	  {{"FCREATE_NONE", pragma::math::to_integral(pragma::audio::ALCreateFlags::None)}, {"FCREATE_MONO", pragma::math::to_integral(pragma::audio::ALCreateFlags::Mono)}, {"FCREATE_STREAM", pragma::math::to_integral(pragma::audio::ALCreateFlags::Stream)},
	    {"FCREATE_DONT_TRANSMIT", pragma::math::to_integral(pragma::audio::ALCreateFlags::DontTransmit)}});
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

	soundMod[luabind::def("create", static_cast<std::shared_ptr<pragma::audio::ALSound> (*)(lua::State *, const std::string &, pragma::audio::ALSoundType, pragma::audio::ALCreateFlags)>(&create))];
	soundMod[luabind::def("create", static_cast<std::shared_ptr<pragma::audio::ALSound> (*)(lua::State *, const std::string &, pragma::audio::ALSoundType)>(&create))];
	soundMod[luabind::def("play", static_cast<std::shared_ptr<pragma::audio::ALSound> (*)(lua::State *, const std::string &, pragma::audio::ALSoundType, const SoundPlayInfo &)>(&play))];
	soundMod[luabind::def("play", static_cast<std::shared_ptr<pragma::audio::ALSound> (*)(lua::State *, const std::string &, pragma::audio::ALSoundType)>(&play))];
	soundMod[luabind::def("get_duration", &get_duration)];
	soundMod[luabind::def("get_all", &get_all)];
	soundMod[luabind::def("is_music_playing", &is_music_playing)];
	soundMod[luabind::def("find_by_type", static_cast<std::vector<std::shared_ptr<pragma::audio::ALSound>> (*)(lua::State *, pragma::audio::ALSoundType, bool)>(&find_by_type))];
	soundMod[luabind::def("find_by_type", static_cast<std::vector<std::shared_ptr<pragma::audio::ALSound>> (*)(lua::State *, pragma::audio::ALSoundType)>(&find_by_type))];
	soundMod[luabind::def("precache", static_cast<bool (*)(lua::State *, const std::string &, pragma::audio::ALChannel)>(&precache))];
	soundMod[luabind::def("precache", static_cast<bool (*)(lua::State *, const std::string &)>(&precache))];
	soundMod[luabind::def("stop_all", &stop_all)];
	soundMod[luabind::def("load_scripts", &load_scripts)];
	soundMod[luabind::def("read_wav_phonemes", &read_wav_phonemes)];
}

std::shared_ptr<pragma::audio::ALSound> Lua::sound::create(lua::State *l, const std::string &snd, pragma::audio::ALSoundType type, pragma::audio::ALCreateFlags flags)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto pAl = state->CreateSound(snd, type, flags);
	if(pAl == nullptr)
		return nullptr;
	pAl->SetType(type);
	return pAl;
}

std::shared_ptr<pragma::audio::ALSound> Lua::sound::create(lua::State *l, const std::string &snd, pragma::audio::ALSoundType type) { return create(l, snd, type, pragma::audio::ALCreateFlags::None); }

std::shared_ptr<pragma::audio::ALSound> Lua::sound::play(lua::State *l, const std::string &sndName, pragma::audio::ALSoundType type, const SoundPlayInfo &playInfo)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
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

std::shared_ptr<pragma::audio::ALSound> Lua::sound::play(lua::State *l, const std::string &sndName, pragma::audio::ALSoundType type) { return play(l, sndName, type, {}); }

bool Lua::sound::is_music_playing(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto &sounds = state->GetSounds();
	auto it = std::find_if(sounds.begin(), sounds.end(), [](pragma::audio::ALSoundRef &rsnd) {
		auto &snd = rsnd.get();
		return (snd.IsPlaying() == true && (snd.GetType() & pragma::audio::ALSoundType::Music) != pragma::audio::ALSoundType::Generic) ? true : false;
	});
	return it != sounds.end();
}

float Lua::sound::get_duration(lua::State *l, const std::string &snd)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	return state->GetSoundDuration(snd);
}

std::vector<std::shared_ptr<pragma::audio::ALSound>> Lua::sound::get_all(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto &sounds = state->GetSounds();
	std::vector<std::shared_ptr<pragma::audio::ALSound>> rsounds;
	rsounds.reserve(sounds.size());
	for(auto &rsnd : sounds) {
		auto &snd = rsnd.get();
		rsounds.push_back(snd.shared_from_this());
	}
	return rsounds;
}

std::vector<std::shared_ptr<pragma::audio::ALSound>> Lua::sound::find_by_type(lua::State *l, pragma::audio::ALSoundType type, bool bExactMatch)
{
	if(type == pragma::audio::ALSoundType::Generic)
		return get_all(l);
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto &sounds = state->GetSounds();
	std::vector<std::shared_ptr<pragma::audio::ALSound>> rsounds;
	auto t = CreateTable(l);
	int32_t n = 1;
	for(auto &rsnd : sounds) {
		auto &snd = rsnd.get();
		if((bExactMatch == false && (snd.GetType() & type) != pragma::audio::ALSoundType::Generic) || (bExactMatch == true && snd.GetType() == type))
			rsounds.push_back(snd.shared_from_this());
	}
	return rsounds;
}
std::vector<std::shared_ptr<pragma::audio::ALSound>> Lua::sound::find_by_type(lua::State *l, pragma::audio::ALSoundType type) { return find_by_type(l, type, false); }

bool Lua::sound::precache(lua::State *l, const std::string &snd, pragma::audio::ALChannel mode)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	return state->PrecacheSound(snd.c_str(), mode);
}
bool Lua::sound::precache(lua::State *l, const std::string &snd) { return precache(l, snd, pragma::audio::ALChannel::Auto); }

void Lua::sound::stop_all(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	state->StopSounds();
}

void Lua::sound::load_scripts(lua::State *l, const std::string &file)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	state->LoadSoundScripts(file.c_str());
}

luabind::object Lua::sound::read_wav_phonemes(lua::State *l, const std::string &fileName)
{
	auto fname = "sounds\\" + pragma::fs::get_canonicalized_path(fileName);
	auto f = pragma::fs::open_file(fname.c_str(), pragma::fs::FileMode::Read | pragma::fs::FileMode::Binary);
	if(f == nullptr)
		return nil;
	source_engine::script::SoundPhonemeData sp {};
	if(source_engine::script::read_wav_phonemes(f, sp) != pragma::util::MarkupFile::ResultCode::Ok)
		return nil;

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
