// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

#undef PlaySound

module pragma.client;

import :client_state;
import se_script;
import pragma.audio.util;
import :audio;
import :engine;
import :game;

#pragma message("TODO: See DDSLoader; MAKE SURE TO RELEASE BUFFER ON ENGINE REMOVE")

static void cl_steam_audio_reload_scene(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	if(pragma::get_cgame() == nullptr)
		return;
	auto cacheFlags = CGame::SoundCacheFlags::All;
	auto spacing = 1'024.f;
	auto info = ipl::Scene::FinalizeInfo {};
	auto argIdx = 0u;
	if(argv.size() > argIdx) {
		if(pragma::util::to_int(argv.at(argIdx++)) == 0)
			cacheFlags &= ~CGame::SoundCacheFlags::BakeConvolution;
		if(argv.size() > argIdx) {
			if(pragma::util::to_int(argv.at(argIdx++)) == 0)
				cacheFlags &= ~CGame::SoundCacheFlags::BakeParametric;
			if(argv.size() > argIdx) {
				if(pragma::util::to_int(argv.at(argIdx++)) == 0)
					cacheFlags &= ~CGame::SoundCacheFlags::SaveProbeBoxes;
				if(argv.size() > argIdx)
					spacing = pragma::util::to_float(argv.at(argIdx++));
			}
		}
	}
	pragma::get_cgame()->ReloadSoundCache(true, cacheFlags, spacing);
#endif
}
namespace {
	auto UVN = pragma::console::client::register_command("cl_steam_audio_reload_scene", &cl_steam_audio_reload_scene, pragma::console::ConVarFlags::None, "Reloads the steam audio scene cache.");
}
static void debug_audio_sounds(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	const auto fPrint = [](pragma::NetworkState *state) {
		auto &sounds = state->GetSounds();
		Con::COUT << "Number of sounds: " << sounds.size() << Con::endl;
		for(auto &rsnd : sounds) {
			auto &snd = rsnd.get();
			Con::COUT << " - " << &snd;
			if(state->IsClient()) {
				auto *buf = static_cast<pragma::audio::CALSound &>(snd)->GetBuffer();
				Con::COUT << " (File: " << ((buf != nullptr) ? buf->GetFilePath() : "Unknown") << ")";
			}
			auto *src = snd.GetSource();
			if(src != nullptr)
				Con::COUT << " (Source: " << *src << ")" << Con::endl;
			Con::COUT << " (State: ";
			if(snd.IsPlaying() == true)
				Con::COUT << "Playing";
			else if(snd.IsPaused() == true)
				Con::COUT << "Paused";
			else if(snd.IsStopped() == true)
				Con::COUT << "Stopped";
			else
				Con::COUT << "Unknown";
			Con::COUT << ") (Offset: " << snd.GetTimeOffset() << " / " << snd.GetDuration() << ") (Gain: " << snd.GetGain() << ") (Pitch: " << snd.GetPitch() << ") (Looping: " << snd.IsLooping() << ") (Relative: " << snd.IsRelative() << ") (Pos: " << snd.GetPosition()
			          << ") (Vel: " << snd.GetVelocity() << ")" << Con::endl;
		}
	};
	auto *server = pragma::get_cengine()->GetServerNetworkState();
	if(server != nullptr) {
		Con::COUT << "Serverside sounds:" << Con::endl;
		fPrint(server);
	}
	auto *client = pragma::get_client_state();
	if(client != nullptr) {
		Con::COUT << "Clientside sounds:" << Con::endl;
		fPrint(client);
	}
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_audio_sounds", &debug_audio_sounds, pragma::console::ConVarFlags::None, "Prints information about all active server- and clientside sounds to the console.");
}

static auto cvAudioStreaming = pragma::console::get_client_con_var("cl_audio_streaming_enabled");
bool pragma::ClientState::PrecacheSound(std::string snd, std::pair<audio::ISoundBuffer *, audio::ISoundBuffer *> *buffers, audio::ALChannel mode, bool bLoadInstantly)
{
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return false;
	snd = fs::get_canonicalized_path(snd);
	auto lsnd = snd;
	string::to_lower(lsnd);
	auto *script = m_soundScriptManager->FindScript(lsnd.c_str());
	if(script != nullptr)
		return true;
	auto path = fs::get_canonicalized_path("sounds\\" + snd);
	audio::get_full_sound_path(path);

	if(fs::is_file(path) == false) {
		auto bPort = false;
		std::string ext;
		if(ufile::get_extension(path, &ext) == true)
			bPort = pragma::util::port_file(this, path);
		else {
			auto audioFormats = engine_info::get_supported_audio_formats();
			for(auto &extFormat : audioFormats) {
				auto extPath = path + '.' + extFormat;
				bPort = pragma::util::port_file(this, extPath);
				if(bPort == true)
					break;
			}
		}
		if(bPort == false) {
			spdlog::warn("Unable to precache sound '{}': File not found!", snd);
			if(get_cgame() != nullptr)
				get_cgame()->RequestResource(path);
			return false;
		}
	}

	auto duration = 0.f;
	if(audio::util::get_duration(path, duration) == false || duration == 0.f) {
		spdlog::warn("Unable to precache sound '{}': Invalid format!", snd);
		return false;
	}

	if(cvAudioStreaming->GetBool() == false)
		bLoadInstantly = true;
	auto bMono = (mode == audio::ALChannel::Mono || mode == audio::ALChannel::Both) ? true : false;
	auto bStereo = (mode == audio::ALChannel::Auto || mode == audio::ALChannel::Both) ? true : false;
	audio::ISoundBuffer *buf = nullptr;
	std::pair<audio::ISoundBuffer *, audio::ISoundBuffer *> tmpBuffers = {nullptr, nullptr};
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
	if(ufile::get_extension(path, &ext) == true && pragma::string::compare<std::string>(ext, "wav", false) == true) {
		auto f = pragma::fs::open_file(path.c_str(), fs::FileMode::Read | fs::FileMode::Binary);
		if(f != nullptr) {
			auto phonemeData = pragma::util::make_shared<source_engine::script::SoundPhonemeData>();
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
bool pragma::ClientState::PrecacheSound(std::string snd, audio::ALChannel mode)
{
	std::pair<audio::ISoundBuffer *, audio::ISoundBuffer *> buffers = {nullptr, nullptr};
	return PrecacheSound(snd, &buffers, mode);
}

bool pragma::ClientState::LoadSoundScripts(const char *file, bool bPrecache)
{
	auto r = NetworkState::LoadSoundScripts(file, bPrecache);
	if(r == false && get_cgame() != nullptr)
		get_cgame()->RequestResource(audio::SoundScriptManager::GetSoundScriptPath() + std::string(file));
	return r;
}

void pragma::ClientState::IndexSound(std::shared_ptr<audio::ALSound> snd, unsigned int idx) { audio::CALSound::SetIndex(snd.get(), idx); }

void pragma::ClientState::StopSounds()
{
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	soundSys->StopSounds();
}

void pragma::ClientState::StopSound(std::shared_ptr<audio::ALSound> pSnd) { pSnd->Stop(); }

std::shared_ptr<pragma::audio::ALSound> pragma::ClientState::CreateSound(std::string snd, audio::ALSoundType type, audio::ALCreateFlags flags)
{
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	auto normPath = fs::get_normalized_path(snd);
	if(m_missingSoundCache.find(normPath) != m_missingSoundCache.end())
		return nullptr;
	auto *script = m_soundScriptManager->FindScript(normPath.c_str());
	if(script == nullptr) {
		auto path = fs::get_canonicalized_path("sounds\\" + snd);
		audio::get_full_sound_path(path);
		auto *buf = soundSys->GetBuffer(path, ((flags & audio::ALCreateFlags::Mono) == audio::ALCreateFlags::None) ? true : false);
		if((flags & audio::ALCreateFlags::Stream) == audio::ALCreateFlags::None || buf != nullptr) // No point in streaming if the buffer is already in memory
		{
			if(buf == nullptr) {
				static auto bSkipPrecache = false;
				if(bSkipPrecache == false) {
					spdlog::warn("Attempted to create unprecached sound '{}'! Loading asynchronously...", snd);
					auto channel = ((flags & audio::ALCreateFlags::Mono) != audio::ALCreateFlags::None) ? audio::ALChannel::Mono : audio::ALChannel::Auto;
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
			auto decoder = soundSys->CreateDecoder(path, ((flags & audio::ALCreateFlags::Mono) != audio::ALCreateFlags::None) ? true : false);
			if(decoder == nullptr) {
				spdlog::warn("Unable to create streaming decoder for sound '{}'!", snd);
				m_missingSoundCache.insert(normPath);
				return nullptr;
			}
			return CreateSound(*decoder, type);
		}
	}
	auto *as = new audio::ALSoundScript(this, std::numeric_limits<uint32_t>::max(), script, this, (flags & audio::ALCreateFlags::Stream) != audio::ALCreateFlags::None);
	std::shared_ptr<audio::ALSound> pAs(as, [](audio::ALSound *snd) {
		snd->OnRelease();
		delete snd;
	});
	m_soundScripts.push_back(pAs);
	as->Initialize();
	Game *game = GetGameState();
	if(game != nullptr) {
		game->CallCallbacks<void, audio::ALSound *>("OnSoundCreated", as);
		game->CallLuaCallbacks<void, std::shared_ptr<audio::ALSound>>("OnSoundCreated", pAs);
	}
	return pAs;
}

void pragma::ClientState::InitializeSound(audio::CALSound &snd)
{
	m_sounds.push_back(snd);
	snd.Initialize();
	auto *game = GetGameState();
	if(game != nullptr) {
		game->CallCallbacks<void, audio::ALSound *>("OnSoundCreated", &snd);
		game->CallLuaCallbacks<void, std::shared_ptr<audio::ALSound>>("OnSoundCreated", static_cast<audio::ALSound &>(snd).shared_from_this());
	}
}

std::shared_ptr<pragma::audio::ALSound> pragma::ClientState::CreateSound(audio::ISoundBuffer &buffer, audio::ALSoundType type)
{
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	auto snd = std::static_pointer_cast<audio::CALSound>(soundSys->CreateSource(buffer));
	if(snd == nullptr) {
		spdlog::warn("Error creating sound '{}'!", buffer.GetFilePath());
		return nullptr;
	}
	snd->SetType(type);
	InitializeSound(*snd);
	return snd;
}

std::shared_ptr<pragma::audio::ALSound> pragma::ClientState::CreateSound(audio::Decoder &decoder, audio::ALSoundType type)
{
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	auto snd = std::static_pointer_cast<audio::CALSound>(soundSys->CreateSource(decoder));
	if(snd == nullptr) {
		spdlog::warn("Error creating sound '{}'!", decoder.GetFilePath());
		return nullptr;
	}
	snd->SetType(type);
	InitializeSound(*snd);
	return snd;
}

std::shared_ptr<pragma::audio::ALSound> pragma::ClientState::PlaySound(std::string snd, audio::ALSoundType type, audio::ALCreateFlags flags)
{
	auto pAl = CreateSound(snd, type, flags);
	if(pAl == nullptr)
		return pAl;
	auto *als = pAl.get();
	als->SetRelative(true);
	als->Play();
	return pAl;
}

std::shared_ptr<pragma::audio::ALSound> pragma::ClientState::PlaySound(audio::ISoundBuffer &buffer, audio::ALSoundType type)
{
	auto pAl = CreateSound(buffer, type);
	if(pAl == nullptr)
		return pAl;
	auto *als = pAl.get();
	als->SetRelative(true);
	als->Play();
	return pAl;
}

std::shared_ptr<pragma::audio::ALSound> pragma::ClientState::PlaySound(audio::Decoder &decoder, audio::ALSoundType type)
{
	auto pAl = CreateSound(decoder, type);
	if(pAl == nullptr)
		return pAl;
	auto *als = pAl.get();
	als->SetRelative(true);
	als->Play();
	return pAl;
}

std::shared_ptr<pragma::audio::ALSound> pragma::ClientState::PlayWorldSound(audio::ISoundBuffer &buffer, audio::ALSoundType type, const Vector3 &pos)
{
	auto ptr = PlaySound(buffer, type);
	auto *alSnd = ptr.get();
	if(alSnd == nullptr)
		return ptr;
	alSnd->SetRelative(false);
	alSnd->SetPosition(pos);
	return ptr;
}

std::shared_ptr<pragma::audio::ALSound> pragma::ClientState::PlayWorldSound(std::string snd, audio::ALSoundType type, const Vector3 &pos)
{
	auto ptr = PlaySound(snd, type, audio::ALCreateFlags::Mono);
	auto *alSnd = ptr.get();
	if(alSnd == nullptr)
		return ptr;
	alSnd->SetRelative(false);
	alSnd->SetPosition(pos);
	return ptr;
}

void pragma::ClientState::UpdateSounds()
{
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys != nullptr) {
		for(auto &snd : soundSys->GetSources()) {
			auto *source = static_cast<audio::CALSound *>(snd.get())->GetSource();
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
			static_cast<audio::CALSound *>(snd.get())->PostUpdate();
	}
	NetworkState::UpdateSounds(m_soundScripts);
}

void pragma::ClientState::UpdateSoundVolume()
{
	for(auto &rsnd : GetSounds()) {
		auto &snd = static_cast<audio::CALSound &>(rsnd.get());
		snd.UpdateVolume();
	}
}
void pragma::ClientState::SetMasterSoundVolume(float vol)
{
	m_volMaster = vol;
	UpdateSoundVolume();
}
float pragma::ClientState::GetMasterSoundVolume() { return m_volMaster; }
void pragma::ClientState::SetSoundVolume(audio::ALSoundType type, float vol)
{
	auto values = math::get_power_of_2_values(CUInt64(type));
	for(auto it = values.begin(); it != values.end(); it++)
		m_volTypes[static_cast<audio::ALSoundType>(*it)] = vol;
	UpdateSoundVolume();
}
float pragma::ClientState::GetSoundVolume(audio::ALSoundType type)
{
	auto it = m_volTypes.find(type);
	if(it == m_volTypes.end())
		return 1.f;
	return it->second;
}
std::unordered_map<pragma::audio::ALSoundType, float> &pragma::ClientState::GetSoundVolumes() { return m_volTypes; }

namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>(
	  "cl_audio_master_volume", +[](pragma::NetworkState *, const pragma::console::ConVar &, float, float vol) {
		  auto *client = pragma::get_client_state();
		  if(client == nullptr)
			  return;
		  client->SetMasterSoundVolume(vol);
	  });
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("cl_audio_hrtf_enabled", +[](pragma::NetworkState *, const pragma::console::ConVar &, bool, bool bEnabled) { pragma::get_cengine()->SetHRTFEnabled(bEnabled); });
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>(
	  "cl_effects_volume", +[](pragma::NetworkState *, const pragma::console::ConVar &, float, float vol) {
		  auto *client = pragma::get_client_state();
		  if(client == nullptr)
			  return;
		  client->SetSoundVolume(pragma::audio::ALSoundType::Effect, vol);
	  });
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>(
	  "cl_music_volume", +[](pragma::NetworkState *, const pragma::console::ConVar &, float, float vol) {
		  auto *client = pragma::get_client_state();
		  if(client == nullptr)
			  return;
		  client->SetSoundVolume(pragma::audio::ALSoundType::Music, vol);
	  });
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>(
	  "cl_voice_volume", +[](pragma::NetworkState *, const pragma::console::ConVar &, float, float vol) {
		  auto *client = pragma::get_client_state();
		  if(client == nullptr)
			  return;
		  client->SetSoundVolume(pragma::audio::ALSoundType::Voice, vol);
	  });
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>(
	  "cl_gui_volume", +[](pragma::NetworkState *, const pragma::console::ConVar &, float, float vol) {
		  auto *client = pragma::get_client_state();
		  if(client == nullptr)
			  return;
		  client->SetSoundVolume(pragma::audio::ALSoundType::GUI, vol);
	  });
}
