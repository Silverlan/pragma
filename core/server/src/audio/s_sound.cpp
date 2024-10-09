/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include <pragma/serverstate/serverstate.h>
#include "pragma/audio/s_alsound.h"
#include <pragma/audio/soundscript.h>
#include "pragma/audio/s_alsoundscript.h"
#include "pragma/scripts.h"
#include <pragma/networking/enums.hpp>
#include <fsys/filesystem.h>
#include <pragma/lua/luacallback.h>
#include <pragma/networking/nwm_util.h>
#include "luasystem.h"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/networking/s_nwm_util.h"
#include <pragma/lua/luafunction_call.h>
#include <sharedutils/util_file.h>
#include <pragma/audio/sound_util.hpp>
#include <pragma/game/game_resources.hpp>
#include <pragma/logging.hpp>

import pragma.audio.util;

void ServerState::SendSoundSourceToClient(SALSound &sound, bool sendFullUpdate, const pragma::networking::ClientRecipientFilter *rf)
{
	NetPacket p;
	p->WriteString(sound.GetSoundName());
	p->Write<ALSoundType>(sound.GetType());
	p->Write<unsigned int>(sound.GetIndex());
	p->Write<ALCreateFlags>(sound.GetCreateFlags());
	p->Write<bool>(sendFullUpdate);
	if(sendFullUpdate) {
		p->Write<ALState>(sound.GetState());
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

		nwm::write_unique_entity(p, sound.GetSource());
	}
	if(rf != nullptr)
		SendPacket("snd_create", p, pragma::networking::Protocol::FastUnreliable, *rf);
	else
		SendPacket("snd_create", p, pragma::networking::Protocol::FastUnreliable);
}
std::shared_ptr<ALSound> ServerState::CreateSound(std::string snd, ALSoundType type, ALCreateFlags flags)
{
	std::transform(snd.begin(), snd.end(), snd.begin(), ::tolower);
	snd = FileManager::GetNormalizedPath(snd);
	if(m_missingSoundCache.find(snd) != m_missingSoundCache.end())
		return nullptr;
	SoundScript *script = m_soundScriptManager->FindScript(snd.c_str());
	float duration = 0.f;
	if(script == NULL) {
		sound::get_full_sound_path(snd, true);
		auto it = m_soundsPrecached.find(snd);
		if(it == m_soundsPrecached.end()) {
			static auto bSkipPrecache = false;
			if(bSkipPrecache == false) {
				Con::cwar << "Attempted to create unprecached sound '" << snd << "'! Precaching now..." << Con::endl;
				auto channel = ((flags & ALCreateFlags::Mono) != ALCreateFlags::None) ? ALChannel::Mono : ALChannel::Auto;
				if(PrecacheSound(snd, channel) == true) {
					bSkipPrecache = true;
					auto r = CreateSound(snd, type, flags);
					bSkipPrecache = false;
					return r;
				}
			}
			m_missingSoundCache.insert(snd);
			return std::shared_ptr<ALSound>();
		}
		else {
			auto &inf = it->second;
			if((flags & ALCreateFlags::Mono) != ALCreateFlags::None && inf->mono == false) {
				static auto bSkipPrecache = false;
				if(bSkipPrecache == false) {
					Con::cwar << "Attempted to create sound '" << snd << "' as unprecached mono! Precaching now..." << Con::endl;
					auto channel = ((flags & ALCreateFlags::Mono) != ALCreateFlags::None) ? ALChannel::Mono : ALChannel::Auto;
					if(PrecacheSound(snd, channel) == true) {
						bSkipPrecache = true;
						auto r = CreateSound(snd, type, flags);
						bSkipPrecache = false;
						return r;
					}
				}
				m_missingSoundCache.insert(snd);
				return std::shared_ptr<ALSound>();
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
	ALSound *as = NULL;
	std::shared_ptr<ALSound> pAs;
	if(script == NULL)
		as = new SALSound(this, idx, duration, snd, flags);
	else
		as = new SALSoundScript(this, idx, script, this, snd, flags);
	pAs = std::shared_ptr<ALSound>(as, [](ALSound *snd) {
		snd->OnRelease();
		delete snd;
	});
	m_sounds.push_back(*as);
	m_serverSounds.push_back(pAs);
	as->SetType(type);
	as->AddCallback("OnDestroyed", FunctionCallback<void>::Create([this, as]() {
		auto it = std::find_if(m_sounds.begin(), m_sounds.end(), [as](const ALSoundRef &sndOther) { return (&sndOther.get() == as) ? true : false; });
		if(it == m_sounds.end())
			return;
		auto idx = it - m_sounds.begin();
		m_sounds.erase(it);
		m_serverSounds.erase(m_serverSounds.begin() + idx);
	}));
	as->Initialize();
	Game *game = GetGameState();
	if(game != NULL) {
		game->CallCallbacks<void, ALSound *>("OnSoundCreated", as);
		game->CallLuaCallbacks<void, std::shared_ptr<ALSound>>("OnSoundCreated", pAs);
	}
	if(umath::is_flag_set(flags, ALCreateFlags::DontTransmit) == false)
		SendSoundSourceToClient(dynamic_cast<SALSound &>(*pAs), false);
	return pAs;
}

void ServerState::UpdateSounds()
{
	assert(m_sounds.size() == m_serverSounds.size());
	NetworkState::UpdateSounds(m_serverSounds);
}

void ServerState::StopSounds() {}

void ServerState::StopSound(std::shared_ptr<ALSound> pSnd) {}

bool ServerState::PrecacheSound(std::string snd, ALChannel mode)
{
	std::transform(snd.begin(), snd.end(), snd.begin(), ::tolower);
	snd = FileManager::GetCanonicalizedPath(snd);
	sound::get_full_sound_path(snd, true);

	SoundCacheInfo *inf = NULL;
	auto it = m_soundsPrecached.find(snd);
	if(it != m_soundsPrecached.end()) {
		inf = it->second.get();
		if(mode == ALChannel::Mono) {
			if(inf->mono == true)
				return true;
		}
		else if(mode == ALChannel::Both) {
			if(inf->mono == true && inf->stereo == true)
				return true;
		}
		else if(mode == ALChannel::Auto && inf->stereo == true)
			return true;
	}
	auto subPath = FileManager::GetCanonicalizedPath("sounds\\" + snd);
	if(!FileManager::IsFile(subPath)) {
		auto bPort = false;
		std::string ext;
		if(ufile::get_extension(subPath, &ext) == true)
			bPort = util::port_file(this, subPath);
		else {
			auto audioFormats = engine_info::get_supported_audio_formats();
			for(auto &extFormat : audioFormats) {
				auto extPath = subPath + '.' + extFormat;
				bPort = util::port_file(this, extPath);
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
	if(pragma::audio::util::get_duration(subPath, duration) == false || duration == 0.f) {
		spdlog::warn("Unable to precache sound '{}': Invalid format!", snd);
		return false;
	}
	if(inf == NULL)
		inf = new SoundCacheInfo;
	if(mode == ALChannel::Mono || mode == ALChannel::Both)
		inf->mono = true;
	if(mode == ALChannel::Auto || mode == ALChannel::Both)
		inf->stereo = true;
	inf->duration = duration;
	m_soundsPrecached.insert(std::unordered_map<std::string, SoundCacheInfo *>::value_type(snd, inf));
	if(m_game != nullptr)
		GetGameState()->RegisterGameResource(subPath);
	NetPacket p;
	p->WriteString(snd);
	p->Write<uint8_t>(umath::to_integral(mode));
	SendPacket("snd_precache", p, pragma::networking::Protocol::SlowReliable);
	return true;
}
