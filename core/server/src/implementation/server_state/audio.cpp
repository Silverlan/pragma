// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.server;
import :server_state;

import pragma.audio.util;
import :audio;
import :game;

void pragma::ServerState::SendSoundSourceToClient(audio::SALSound &sound, bool sendFullUpdate, const networking::ClientRecipientFilter *rf)
{
	NetPacket p;
	p->WriteString(sound.GetSoundName());
	p->Write<audio::ALSoundType>(sound.GetType());
	p->Write<unsigned int>(sound.GetIndex());
	p->Write<audio::ALCreateFlags>(sound.GetCreateFlags());
	p->Write<bool>(sendFullUpdate);
	if(sendFullUpdate) {
		p->Write<audio::ALState>(sound.GetState());
		p->Write<float>(sound.GetOffset());
		p->Write<float>(sound.GetPitch());
		p->Write<bool>(sound.IsLooping());
		p->Write<float>(sound.GetGain());
		p->Write<Vector3>(sound.GetPosition());
		p->Write<Vector3>(sound.GetVelocity());
		p->Write<Vector3>(sound.GetDirection());
		p->Write<bool>(sound.IsRelative());
		p->Write<float>(sound.GetReferenceDistance());
		p->Write<float>(sound.GetRolloffFactor());
		p->Write<float>(sound.GetRoomRolloffFactor());
		p->Write<float>(sound.GetMaxDistance());
		p->Write<float>(sound.GetMinGain());
		p->Write<float>(sound.GetMaxGain());
		p->Write<float>(sound.GetInnerConeAngle());
		p->Write<float>(sound.GetOuterConeAngle());
		p->Write<float>(sound.GetOuterConeGain());
		p->Write<float>(sound.GetOuterConeGainHF());
		p->Write<uint32_t>(sound.GetFlags());

		auto hasRange = sound.HasRange();
		p->Write<bool>(hasRange);
		if(hasRange) {
			auto range = sound.GetRange();
			p->Write<float>(range.first);
			p->Write<float>(range.second);
		}

		p->Write<float>(sound.GetFadeInDuration());
		p->Write<float>(sound.GetFadeOutDuration());
		p->Write<uint32_t>(sound.GetPriority());

		auto orientation = sound.GetOrientation();
		p->Write<Vector3>(orientation.first);
		p->Write<Vector3>(orientation.second);

		p->Write<float>(sound.GetDopplerFactor());
		p->Write<float>(sound.GetLeftStereoAngle());
		p->Write<float>(sound.GetRightStereoAngle());

		p->Write<float>(sound.GetAirAbsorptionFactor());

		auto gainAuto = sound.GetGainAuto();
		p->Write<bool>(std::get<0>(gainAuto));
		p->Write<bool>(std::get<1>(gainAuto));
		p->Write<bool>(std::get<2>(gainAuto));

		auto directFilter = sound.GetDirectFilter();
		p->Write<float>(directFilter.gain);
		p->Write<float>(directFilter.gainHF);
		p->Write<float>(directFilter.gainLF);

		networking::write_unique_entity(p, sound.GetSource());
	}
	if(rf != nullptr)
		SendPacket(networking::net_messages::client::SND_CREATE, p, networking::Protocol::FastUnreliable, *rf);
	else
		SendPacket(networking::net_messages::client::SND_CREATE, p, networking::Protocol::FastUnreliable);
}
std::shared_ptr<pragma::audio::ALSound> pragma::ServerState::CreateSound(std::string snd, audio::ALSoundType type, audio::ALCreateFlags flags)
{
	string::to_lower(snd);
	snd = fs::get_normalized_path(snd);
	if(m_missingSoundCache.find(snd) != m_missingSoundCache.end())
		return nullptr;
	audio::SoundScript *script = m_soundScriptManager->FindScript(snd.c_str());
	float duration = 0.f;
	if(script == nullptr) {
		audio::get_full_sound_path(snd, true);
		auto it = m_soundsPrecached.find(snd);
		if(it == m_soundsPrecached.end()) {
			static auto bSkipPrecache = false;
			if(bSkipPrecache == false) {
				Con::CWAR << "Attempted to create unprecached sound '" << snd << "'! Precaching now..." << Con::endl;
				auto channel = ((flags & audio::ALCreateFlags::Mono) != audio::ALCreateFlags::None) ? audio::ALChannel::Mono : audio::ALChannel::Auto;
				if(PrecacheSound(snd, channel) == true) {
					bSkipPrecache = true;
					auto r = CreateSound(snd, type, flags);
					bSkipPrecache = false;
					return r;
				}
			}
			m_missingSoundCache.insert(snd);
			return std::shared_ptr<audio::ALSound>();
		}
		else {
			auto &inf = it->second;
			if((flags & audio::ALCreateFlags::Mono) != audio::ALCreateFlags::None && inf->mono == false) {
				static auto bSkipPrecache = false;
				if(bSkipPrecache == false) {
					Con::CWAR << "Attempted to create sound '" << snd << "' as unprecached mono! Precaching now..." << Con::endl;
					auto channel = ((flags & audio::ALCreateFlags::Mono) != audio::ALCreateFlags::None) ? audio::ALChannel::Mono : audio::ALChannel::Auto;
					if(PrecacheSound(snd, channel) == true) {
						bSkipPrecache = true;
						auto r = CreateSound(snd, type, flags);
						bSkipPrecache = false;
						return r;
					}
				}
				m_missingSoundCache.insert(snd);
				return std::shared_ptr<audio::ALSound>();
			}
		}
		duration = it->second->duration;
	}
	unsigned int idx;
	if(!m_alsoundIndex.empty()) {
		idx = m_alsoundIndex[0];
		m_alsoundIndex.erase(m_alsoundIndex.begin());
	}
	else {
		idx = m_alsoundID;
		m_alsoundID++;
	}
	audio::ALSound *as = nullptr;
	std::shared_ptr<audio::ALSound> pAs;
	if(script == nullptr)
		as = new audio::SALSound(this, idx, duration, snd, flags);
	else
		as = new audio::SALSoundScript(this, idx, script, this, snd, flags);
	pAs = std::shared_ptr<audio::ALSound>(as, [](audio::ALSound *snd) {
		snd->OnRelease();
		delete snd;
	});
	m_sounds.push_back(*as);
	m_serverSounds.push_back(pAs);
	as->SetType(type);
	as->AddCallback("OnDestroyed", FunctionCallback<void>::Create([this, as]() {
		auto it = std::find_if(m_sounds.begin(), m_sounds.end(), [as](const audio::ALSoundRef &sndOther) { return (&sndOther.get() == as) ? true : false; });
		if(it == m_sounds.end())
			return;
		auto idx = it - m_sounds.begin();
		m_sounds.erase(it);
		m_serverSounds.erase(m_serverSounds.begin() + idx);
	}));
	as->Initialize();
	Game *game = GetGameState();
	if(game != nullptr) {
		game->CallCallbacks<void, audio::ALSound *>("OnSoundCreated", as);
		game->CallLuaCallbacks<void, std::shared_ptr<audio::ALSound>>("OnSoundCreated", pAs);
	}
	if(math::is_flag_set(flags, audio::ALCreateFlags::DontTransmit) == false)
		SendSoundSourceToClient(dynamic_cast<audio::SALSound &>(*pAs), false);
	return pAs;
}

void pragma::ServerState::UpdateSounds()
{
	assert(m_sounds.size() == m_serverSounds.size());
	NetworkState::UpdateSounds(m_serverSounds);
}

void pragma::ServerState::StopSounds() {}

void pragma::ServerState::StopSound(std::shared_ptr<audio::ALSound> pSnd) {}

bool pragma::ServerState::PrecacheSound(std::string snd, audio::ALChannel mode)
{
	string::to_lower(snd);
	snd = fs::get_canonicalized_path(snd);
	audio::get_full_sound_path(snd, true);

	audio::SoundCacheInfo *inf = nullptr;
	auto it = m_soundsPrecached.find(snd);
	if(it != m_soundsPrecached.end()) {
		inf = it->second.get();
		if(mode == audio::ALChannel::Mono) {
			if(inf->mono == true)
				return true;
		}
		else if(mode == audio::ALChannel::Both) {
			if(inf->mono == true && inf->stereo == true)
				return true;
		}
		else if(mode == audio::ALChannel::Auto && inf->stereo == true)
			return true;
	}
	auto subPath = fs::get_canonicalized_path("sounds\\" + snd);
	if(!fs::is_file(subPath)) {
		auto bPort = false;
		std::string ext;
		if(ufile::get_extension(subPath, &ext) == true)
			bPort = pragma::util::port_file(this, subPath);
		else {
			auto audioFormats = engine_info::get_supported_audio_formats();
			for(auto &extFormat : audioFormats) {
				auto extPath = subPath + '.' + extFormat;
				bPort = pragma::util::port_file(this, extPath);
				if(bPort == true)
					break;
			}
		}
		if(bPort == false) {
			spdlog::warn("Unable to precache sound '{}': File not found!", snd);
			return false;
		}
	}
	auto duration = 0.f;
	if(audio::util::get_duration(subPath, duration) == false || duration == 0.f) {
		spdlog::warn("Unable to precache sound '{}': Invalid format!", snd);
		return false;
	}
	if(inf == nullptr)
		inf = new audio::SoundCacheInfo;
	if(mode == audio::ALChannel::Mono || mode == audio::ALChannel::Both)
		inf->mono = true;
	if(mode == audio::ALChannel::Auto || mode == audio::ALChannel::Both)
		inf->stereo = true;
	inf->duration = duration;
	m_soundsPrecached.insert(std::unordered_map<std::string, audio::SoundCacheInfo *>::value_type(snd, inf));
	if(m_game != nullptr)
		GetGameState()->RegisterGameResource(subPath);
	NetPacket p;
	p->WriteString(snd);
	p->Write<uint8_t>(math::to_integral(mode));
	SendPacket(networking::net_messages::client::SND_PRECACHE, p, networking::Protocol::SlowReliable);
	return true;
}
