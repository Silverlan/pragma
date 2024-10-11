/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/c_engine.h"
#include <pragma/audio/soundscript.h>
#include "pragma/audio/alsoundscript.h"
#include <fsys/filesystem.h>
#include "pragma/audio/c_sound_load.h"
#include <pragma/lua/luacallback.h>
#include "pragma/audio/c_alsound.h"
#include <pragma/audio/alsound_type.h>
#include "luasystem.h"
#include <pragma/lua/luafunction_call.h>
#include "pragma/console/c_cvar.h"
#include "pragma/console/c_cvar_global_functions.h"
#include <alsoundsystem.hpp>
#include <alsound_buffer.hpp>
#include <pragma/game/game_resources.hpp>
#include <pragma/audio/sound_util.hpp>
#include <sharedutils/util_file.h>
#include <steam_audio/alsound_steam_audio.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/logging.hpp>
#include <sharedutils/util_markup_file.hpp>

import se_script;
import pragma.audio.util;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
#pragma message("TODO: See DDSLoader; MAKE SURE TO RELEASE BUFFER ON ENGINE REMOVE")

void Console::commands::cl_steam_audio_reload_scene(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	if(c_game == nullptr)
		return;
	auto cacheFlags = CGame::SoundCacheFlags::All;
	auto spacing = 1'024.f;
	auto info = ipl::Scene::FinalizeInfo {};
	auto argIdx = 0u;
	if(argv.size() > argIdx) {
		if(util::to_int(argv.at(argIdx++)) == 0)
			cacheFlags &= ~CGame::SoundCacheFlags::BakeConvolution;
		if(argv.size() > argIdx) {
			if(util::to_int(argv.at(argIdx++)) == 0)
				cacheFlags &= ~CGame::SoundCacheFlags::BakeParametric;
			if(argv.size() > argIdx) {
				if(util::to_int(argv.at(argIdx++)) == 0)
					cacheFlags &= ~CGame::SoundCacheFlags::SaveProbeBoxes;
				if(argv.size() > argIdx)
					spacing = util::to_float(argv.at(argIdx++));
			}
		}
	}
	c_game->ReloadSoundCache(true, cacheFlags, spacing);
#endif
}
void Console::commands::debug_audio_sounds(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	const auto fPrint = [](NetworkState *state) {
		auto &sounds = state->GetSounds();
		Con::cout << "Number of sounds: " << sounds.size() << Con::endl;
		for(auto &rsnd : sounds) {
			auto &snd = rsnd.get();
			Con::cout << " - " << &snd;
			if(state->IsClient()) {
				auto *buf = static_cast<CALSound &>(snd)->GetBuffer();
				Con::cout << " (File: " << ((buf != nullptr) ? buf->GetFilePath() : "Unknown") << ")";
			}
			auto *src = snd.GetSource();
			if(src != nullptr)
				Con::cout << " (Source: " << *src << ")" << Con::endl;
			Con::cout << " (State: ";
			if(snd.IsPlaying() == true)
				Con::cout << "Playing";
			else if(snd.IsPaused() == true)
				Con::cout << "Paused";
			else if(snd.IsStopped() == true)
				Con::cout << "Stopped";
			else
				Con::cout << "Unknown";
			Con::cout << ") (Offset: " << snd.GetTimeOffset() << " / " << snd.GetDuration() << ") (Gain: " << snd.GetGain() << ") (Pitch: " << snd.GetPitch() << ") (Looping: " << snd.IsLooping() << ") (Relative: " << snd.IsRelative() << ") (Pos: " << snd.GetPosition()
			          << ") (Vel: " << snd.GetVelocity() << ")" << Con::endl;
		}
	};
	auto *server = c_engine->GetServerNetworkState();
	if(server != nullptr) {
		Con::cout << "Serverside sounds:" << Con::endl;
		fPrint(server);
	}
	if(client != nullptr) {
		Con::cout << "Clientside sounds:" << Con::endl;
		fPrint(client);
	}
}

static auto cvAudioStreaming = GetClientConVar("cl_audio_streaming_enabled");
bool ClientState::PrecacheSound(std::string snd, std::pair<al::ISoundBuffer *, al::ISoundBuffer *> *buffers, ALChannel mode, bool bLoadInstantly)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return false;
	snd = FileManager::GetCanonicalizedPath(snd);
	auto lsnd = snd;
	ustring::to_lower(lsnd);
	auto *script = m_soundScriptManager->FindScript(lsnd.c_str());
	if(script != nullptr)
		return true;
	auto path = FileManager::GetCanonicalizedPath("sounds\\" + snd);
	sound::get_full_sound_path(path);

	if(FileManager::IsFile(path) == false) {
		auto bPort = false;
		std::string ext;
		if(ufile::get_extension(path, &ext) == true)
			bPort = util::port_file(this, path);
		else {
			auto audioFormats = engine_info::get_supported_audio_formats();
			for(auto &extFormat : audioFormats) {
				auto extPath = path + '.' + extFormat;
				bPort = util::port_file(this, extPath);
				if(bPort == true)
					break;
			}
		}
		if(bPort == false) {
			spdlog::warn("Unable to precache sound '{}': File not found!", snd);
			if(c_game != nullptr)
				c_game->RequestResource(path);
			return false;
		}
	}

	auto duration = 0.f;
	if(pragma::audio::util::get_duration(path, duration) == false || duration == 0.f) {
		spdlog::warn("Unable to precache sound '{}': Invalid format!", snd);
		return false;
	}

	if(cvAudioStreaming->GetBool() == false)
		bLoadInstantly = true;
	auto bMono = (mode == ALChannel::Mono || mode == ALChannel::Both) ? true : false;
	auto bStereo = (mode == ALChannel::Auto || mode == ALChannel::Both) ? true : false;
	al::ISoundBuffer *buf = nullptr;
	std::pair<al::ISoundBuffer *, al::ISoundBuffer *> tmpBuffers = {nullptr, nullptr};
	auto *tgtBuffers = (buffers != nullptr) ? buffers : &tmpBuffers;
	try {
		if(bStereo == true) {
			auto *bufStereo = buf = soundSys->LoadSound(path, false, !bLoadInstantly);
			tgtBuffers->second = bufStereo;
		}
		if(bMono == true) {
			auto *bufMono = soundSys->LoadSound(path, true, !bLoadInstantly);
			if(buf == nullptr)
				buf = bufMono;
			tgtBuffers->first = bufMono;
		}
	}
	catch(const std::runtime_error &err) {
		spdlog::warn("Unable to precache sound '{}': {}!", snd, err.what());
		return false;
	}
	if(buf == nullptr) {
		spdlog::warn("Unable to precache sound '{}': Invalid format!", snd);
		return false;
	}
	std::string ext;
	if(ufile::get_extension(path, &ext) == true && ustring::compare<std::string>(ext, "wav", false) == true) {
		auto f = FileManager::OpenFile(path.c_str(), "rb");
		if(f != nullptr) {
			auto phonemeData = std::make_shared<source_engine::script::SoundPhonemeData>();
			if(source_engine::script::read_wav_phonemes(f, *phonemeData) == util::MarkupFile::ResultCode::Ok) {
				if(tgtBuffers->first != nullptr)
					tgtBuffers->first->SetUserData(phonemeData);
				if(tgtBuffers->second != nullptr)
					tgtBuffers->second->SetUserData(phonemeData);
			}
		}
	}
	return true;
}
bool ClientState::PrecacheSound(std::string snd, ALChannel mode)
{
	std::pair<al::ISoundBuffer *, al::ISoundBuffer *> buffers = {nullptr, nullptr};
	return PrecacheSound(snd, &buffers, mode);
}

bool ClientState::LoadSoundScripts(const char *file, bool bPrecache)
{
	auto r = NetworkState::LoadSoundScripts(file, bPrecache);
	if(r == false && c_game != nullptr)
		c_game->RequestResource(SoundScriptManager::GetSoundScriptPath() + std::string(file));
	return r;
}

void ClientState::IndexSound(std::shared_ptr<ALSound> snd, unsigned int idx) { CALSound::SetIndex(snd.get(), idx); }

void ClientState::StopSounds()
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	soundSys->StopSounds();
}

void ClientState::StopSound(std::shared_ptr<ALSound> pSnd) { pSnd->Stop(); }

std::shared_ptr<ALSound> ClientState::CreateSound(std::string snd, ALSoundType type, ALCreateFlags flags)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	auto normPath = FileManager::GetNormalizedPath(snd);
	if(m_missingSoundCache.find(normPath) != m_missingSoundCache.end())
		return nullptr;
	auto *script = m_soundScriptManager->FindScript(normPath.c_str());
	if(script == nullptr) {
		auto path = FileManager::GetCanonicalizedPath("sounds\\" + snd);
		sound::get_full_sound_path(path);
		auto *buf = soundSys->GetBuffer(path, ((flags & ALCreateFlags::Mono) == ALCreateFlags::None) ? true : false);
		if((flags & ALCreateFlags::Stream) == ALCreateFlags::None || buf != nullptr) // No point in streaming if the buffer is already in memory
		{
			if(buf == nullptr) {
				static auto bSkipPrecache = false;
				if(bSkipPrecache == false) {
					spdlog::warn("Attempted to create unprecached sound '{}'! Loading asynchronously...", snd);
					auto channel = ((flags & ALCreateFlags::Mono) != ALCreateFlags::None) ? ALChannel::Mono : ALChannel::Auto;
					if(PrecacheSound(snd, nullptr, channel) == true) {
						bSkipPrecache = true;
						auto r = CreateSound(snd, type, flags);
						bSkipPrecache = false;
						return r;
					}
				}
				m_missingSoundCache.insert(normPath);
				return nullptr;
			}
			return CreateSound(*buf, type);
		}
		else {
			auto decoder = soundSys->CreateDecoder(path, ((flags & ALCreateFlags::Mono) != ALCreateFlags::None) ? true : false);
			if(decoder == nullptr) {
				spdlog::warn("Unable to create streaming decoder for sound '{}'!", snd);
				m_missingSoundCache.insert(normPath);
				return nullptr;
			}
			return CreateSound(*decoder, type);
		}
	}
	auto *as = new ALSoundScript(this, std::numeric_limits<uint32_t>::max(), script, this, (flags & ALCreateFlags::Stream) != ALCreateFlags::None);
	std::shared_ptr<ALSound> pAs(as, [](ALSound *snd) {
		snd->OnRelease();
		delete snd;
	});
	m_soundScripts.push_back(pAs);
	as->Initialize();
	Game *game = GetGameState();
	if(game != NULL) {
		game->CallCallbacks<void, ALSound *>("OnSoundCreated", as);
		game->CallLuaCallbacks<void, std::shared_ptr<ALSound>>("OnSoundCreated", pAs);
	}
	return pAs;
}

void ClientState::InitializeSound(CALSound &snd)
{
	m_sounds.push_back(snd);
	snd.Initialize();
	auto *game = GetGameState();
	if(game != nullptr) {
		game->CallCallbacks<void, ALSound *>("OnSoundCreated", &snd);
		game->CallLuaCallbacks<void, std::shared_ptr<ALSound>>("OnSoundCreated", static_cast<ALSound &>(snd).shared_from_this());
	}
}

std::shared_ptr<ALSound> ClientState::CreateSound(al::ISoundBuffer &buffer, ALSoundType type)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	auto snd = std::static_pointer_cast<CALSound>(soundSys->CreateSource(buffer));
	if(snd == nullptr) {
		spdlog::warn("Error creating sound '{}'!", buffer.GetFilePath());
		return nullptr;
	}
	snd->SetType(type);
	InitializeSound(*snd);
	return snd;
}

std::shared_ptr<ALSound> ClientState::CreateSound(al::Decoder &decoder, ALSoundType type)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	auto snd = std::static_pointer_cast<CALSound>(soundSys->CreateSource(decoder));
	if(snd == nullptr) {
		spdlog::warn("Error creating sound '{}'!", decoder.GetFilePath());
		return nullptr;
	}
	snd->SetType(type);
	InitializeSound(*snd);
	return snd;
}

std::shared_ptr<ALSound> ClientState::PlaySound(std::string snd, ALSoundType type, ALCreateFlags flags)
{
	auto pAl = CreateSound(snd, type, flags);
	if(pAl == nullptr)
		return pAl;
	auto *als = pAl.get();
	als->SetRelative(true);
	als->Play();
	return pAl;
}

std::shared_ptr<ALSound> ClientState::PlaySound(al::ISoundBuffer &buffer, ALSoundType type)
{
	auto pAl = CreateSound(buffer, type);
	if(pAl == nullptr)
		return pAl;
	auto *als = pAl.get();
	als->SetRelative(true);
	als->Play();
	return pAl;
}

std::shared_ptr<ALSound> ClientState::PlaySound(al::Decoder &decoder, ALSoundType type)
{
	auto pAl = CreateSound(decoder, type);
	if(pAl == nullptr)
		return pAl;
	auto *als = pAl.get();
	als->SetRelative(true);
	als->Play();
	return pAl;
}

std::shared_ptr<ALSound> ClientState::PlayWorldSound(al::ISoundBuffer &buffer, ALSoundType type, const Vector3 &pos)
{
	auto ptr = PlaySound(buffer, type);
	auto *alSnd = ptr.get();
	if(alSnd == nullptr)
		return ptr;
	alSnd->SetRelative(false);
	alSnd->SetPosition(pos);
	return ptr;
}

std::shared_ptr<ALSound> ClientState::PlayWorldSound(std::string snd, ALSoundType type, const Vector3 &pos)
{
	auto ptr = PlaySound(snd, type, ALCreateFlags::Mono);
	auto *alSnd = ptr.get();
	if(alSnd == nullptr)
		return ptr;
	alSnd->SetRelative(false);
	alSnd->SetPosition(pos);
	return ptr;
}

void ClientState::UpdateSounds()
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys != nullptr) {
		for(auto &snd : soundSys->GetSources()) {
			auto *source = static_cast<CALSound *>(snd.get())->GetSource();
			if(source == nullptr)
				continue;
			auto pTrComponent = source->GetTransformComponent();
			auto srcPos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
			auto sndPos = (*snd)->GetPosition();
			if(uvec::cmp(srcPos, sndPos) == false)
				(*snd)->SetPosition(srcPos);
		}
		soundSys->Update();
		for(auto &snd : soundSys->GetSources())
			static_cast<CALSound *>(snd.get())->PostUpdate();
	}
	NetworkState::UpdateSounds(m_soundScripts);
}

void ClientState::UpdateSoundVolume()
{
	for(auto &rsnd : GetSounds()) {
		auto &snd = static_cast<CALSound &>(rsnd.get());
		snd.UpdateVolume();
	}
}
void ClientState::SetMasterSoundVolume(float vol)
{
	m_volMaster = vol;
	UpdateSoundVolume();
}
float ClientState::GetMasterSoundVolume() { return m_volMaster; }
void ClientState::SetSoundVolume(ALSoundType type, float vol)
{
	auto values = umath::get_power_of_2_values(CUInt64(type));
	for(auto it = values.begin(); it != values.end(); it++)
		m_volTypes[static_cast<ALSoundType>(*it)] = vol;
	UpdateSoundVolume();
}
float ClientState::GetSoundVolume(ALSoundType type)
{
	auto it = m_volTypes.find(type);
	if(it == m_volTypes.end())
		return 1.f;
	return it->second;
}
std::unordered_map<ALSoundType, float> &ClientState::GetSoundVolumes() { return m_volTypes; }

REGISTER_CONVAR_CALLBACK_CL(cl_audio_master_volume, [](NetworkState *, const ConVar &, float, float vol) {
	if(client == nullptr)
		return;
	client->SetMasterSoundVolume(vol);
})

REGISTER_CONVAR_CALLBACK_CL(cl_audio_hrtf_enabled, [](NetworkState *, const ConVar &, bool, bool bEnabled) { c_engine->SetHRTFEnabled(bEnabled); })

REGISTER_CONVAR_CALLBACK_CL(cl_effects_volume, [](NetworkState *, const ConVar &, float, float vol) {
	if(client == nullptr)
		return;
	client->SetSoundVolume(ALSoundType::Effect, vol);
})

REGISTER_CONVAR_CALLBACK_CL(cl_music_volume, [](NetworkState *, const ConVar &, float, float vol) {
	if(client == nullptr)
		return;
	client->SetSoundVolume(ALSoundType::Music, vol);
})

REGISTER_CONVAR_CALLBACK_CL(cl_voice_volume, [](NetworkState *, const ConVar &, float, float vol) {
	if(client == nullptr)
		return;
	client->SetSoundVolume(ALSoundType::Voice, vol);
})

REGISTER_CONVAR_CALLBACK_CL(cl_gui_volume, [](NetworkState *, const ConVar &, float, float vol) {
	if(client == nullptr)
		return;
	client->SetSoundVolume(ALSoundType::GUI, vol);
})
