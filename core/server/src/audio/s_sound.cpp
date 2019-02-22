#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include <pragma/serverstate/serverstate.h>
#include "pragma/audio/s_alsound.h"
#include <pragma/audio/soundscript.h>
#include "pragma/audio/s_alsoundscript.h"
#include "pragma/scripts.h"
#include <fsys/filesystem.h>
#include <pragma/lua/luacallback.h>
#include <pragma/networking/nwm_util.h>
#include "luasystem.h"
#include <pragma/lua/luafunction_call.h>
#include <sharedutils/util_file.h>
#include <pragma/audio/sound_util.hpp>
#include <pragma/game/game_resources.hpp>
#include <util_sound.hpp>

std::shared_ptr<ALSound> ServerState::CreateSound(std::string snd,ALSoundType type,ALCreateFlags flags)
{
	std::transform(snd.begin(),snd.end(),snd.begin(),::tolower);
	snd = FileManager::GetCanonicalizedPath(snd);
	SoundScript *script = m_soundScriptManager->FindScript(snd.c_str());
	float duration = 0.f;
	if(script == NULL)
	{
		sound::get_full_sound_path(snd,true);
		auto it = m_soundsPrecached.find(snd);
		if(it == m_soundsPrecached.end())
		{
			static auto bSkipPrecache = false;
			if(bSkipPrecache == false)
			{
				Con::cwar<<"WARNING: Attempted to create unprecached sound '"<<snd<<"'! Precaching now..."<<Con::endl;
				auto channel = ((flags &ALCreateFlags::Mono) != ALCreateFlags::None) ? ALChannel::Mono : ALChannel::Auto;
				if(PrecacheSound(snd,channel) == true)
				{
					bSkipPrecache = true;
					auto r = CreateSound(snd,type,flags);
					bSkipPrecache = false;
					return r;
				}
			}
			return std::shared_ptr<ALSound>();
		}
		else
		{
			auto &inf = it->second;
			if((flags &ALCreateFlags::Mono) != ALCreateFlags::None && inf->mono == false)
			{
				static auto bSkipPrecache = false;
				if(bSkipPrecache == false)
				{
					Con::cwar<<"WARNING: Attempted to create sound '"<<snd<<"' as unprecached mono! Precaching now..."<<Con::endl;
					auto channel = ((flags &ALCreateFlags::Mono) != ALCreateFlags::None) ? ALChannel::Mono : ALChannel::Auto;
					if(PrecacheSound(snd,channel) == true)
					{
						bSkipPrecache = true;
						auto r = CreateSound(snd,type,flags);
						bSkipPrecache = false;
						return r;
					}
				}
				return std::shared_ptr<ALSound>();
			}
		}
		duration = it->second->duration;
	}
	unsigned int idx;
	if(!m_alsoundIndex.empty())
	{
		idx = m_alsoundIndex[0];
		m_alsoundIndex.erase(m_alsoundIndex.begin());
	}
	else
	{
		idx = m_alsoundID;
		m_alsoundID++;
	}
	auto bTransmit = (flags &ALCreateFlags::DontTransmit) == ALCreateFlags::None;
	auto bStream = (flags &ALCreateFlags::Stream) != ALCreateFlags::None;
	ALSound *as = NULL;
	std::shared_ptr<ALSound> pAs;
	if(script == NULL)
		as = new SALSound(this,idx,duration,bTransmit);
	else
		as = new SALSoundScript(this,idx,script,this,bStream,bTransmit);
	pAs = std::shared_ptr<ALSound>(as);
	m_sounds.push_back(*as);
	m_serverSounds.push_back(pAs);
	as->SetType(type);
	as->AddCallback("OnDestroyed",FunctionCallback<void>::Create([this,as]() {
		auto it = std::find_if(m_sounds.begin(),m_sounds.end(),[as](const ALSoundRef &sndOther) {
			return (&sndOther.get() == as) ? true : false;
		});
		if(it == m_sounds.end())
			return;
		auto idx = it -m_sounds.begin();
		m_sounds.erase(it);
		m_serverSounds.erase(m_serverSounds.begin() +idx);
	}));
	as->Initialize();
	Game *game = GetGameState();
	if(game != NULL)
	{
		game->CallCallbacks<void,ALSound*>("OnSoundCreated",as);
		game->CallLuaCallbacks<void,std::shared_ptr<ALSound>>("OnSoundCreated",pAs);
	}
	if(bTransmit == true)
	{
		NetPacket p;
		p->WriteString(snd);
		p->Write<unsigned int>(idx);
		p->Write<uint32_t>(umath::to_integral(flags));
		BroadcastUDP("snd_create",p);
	}
	return pAs;
}

void ServerState::UpdateSounds()
{
	assert(m_sounds.size() == m_serverSounds.size());
	NetworkState::UpdateSounds(m_serverSounds);
}

void ServerState::StopSounds() {}

void ServerState::StopSound(std::shared_ptr<ALSound> pSnd) {}

bool ServerState::PrecacheSound(std::string snd,ALChannel mode)
{
	std::transform(snd.begin(),snd.end(),snd.begin(),::tolower);
	snd = FileManager::GetCanonicalizedPath(snd);
	sound::get_full_sound_path(snd,true);

	SoundCacheInfo *inf = NULL;
	auto it = m_soundsPrecached.find(snd);
	if(it != m_soundsPrecached.end())
	{
		inf = it->second.get();
		if(mode == ALChannel::Mono)
		{
			if(inf->mono == true)
				return true;
		}
		else if(mode == ALChannel::Both)
		{
			if(inf->mono == true && inf->stereo == true)
				return true;
		}
		else if(mode == ALChannel::Auto && inf->stereo == true)
			return true;
	}
	auto subPath = FileManager::GetCanonicalizedPath("sounds\\" +snd);
	if(!FileManager::IsFile(subPath))
	{
		auto bPort = false;
		std::string ext;
		if(ufile::get_extension(subPath,&ext) == true)
			bPort = util::port_file(this,subPath);
		else
		{
			auto audioFormats = engine_info::get_supported_audio_formats();
			for(auto &extFormat : audioFormats)
			{
				auto extPath = subPath +'.' +extFormat;
				bPort = util::port_file(this,extPath);
				if(bPort == true)
					break;
			}
		}
		if(bPort == false)
		{
			Con::cwar<<"WARNING: Unable to precache sound '"<<snd<<"': File not found!"<<Con::endl;
			return false;
		}
	}
	auto duration = 0.f;
	if(util::sound::get_duration(subPath,duration) == false || duration == 0.f)
	{
		Con::cwar<<"WARNING: Unable to precache sound '"<<snd<<"': Invalid format!"<<Con::endl;
		return false;
	}
	if(inf == NULL)
		inf = new SoundCacheInfo;
	if(mode == ALChannel::Mono || mode == ALChannel::Both)
		inf->mono = true;
	if(mode == ALChannel::Auto || mode == ALChannel::Both)
		inf->stereo = true;
	inf->duration = duration;
	m_soundsPrecached.insert(std::unordered_map<std::string,SoundCacheInfo*>::value_type(snd,inf));
	if(m_game != nullptr)
		m_game->RegisterGameResource(subPath);
	NetPacket p;
	p->WriteString(snd);
	p->Write<uint8_t>(umath::to_integral(mode));
	BroadcastTCP("snd_precache",p);
	return true;
}
