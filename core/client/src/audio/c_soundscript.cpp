#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/audio/c_soundscript.h"
#include <sharedutils/util_string.h>
#include "pragma/audio/c_sound_efx.hpp"
#include <alsoundsystem.hpp>
#include <algorithm>

extern DLLCENGINE CEngine *c_engine;
extern ClientState *client;

#undef CreateEvent

CSoundScript::CSoundScript(SoundScriptManager *manager,std::string identifier)
	: SoundScript(manager,identifier)
{}
CSoundScript::~CSoundScript()
{}

//////////////////////////////////////

CSSEPlaySound::CSSEPlaySound(SoundScriptManager *manager)
	: SSEPlaySound(manager),m_dspEffect(NULL)
{}
SSESound *CSSEPlaySound::CreateSound(double tStart,const std::function<std::shared_ptr<ALSound>(const std::string&,ALChannel,ALCreateFlags)> &createSound)
{
	auto *s = SSEPlaySound::CreateSound(tStart,createSound);
	if(s == nullptr)
		return s;
	auto *cs = dynamic_cast<CALSound*>(s->sound.get());
	if(cs == nullptr)
		return s;
	if(m_dspEffect != nullptr)
		cs->AddEffect(*m_dspEffect);
	for(auto &effect : effects)
		cs->AddEffect(*effect);
	return s;
}
void CSSEPlaySound::PrecacheSound(const char *name) {client->PrecacheSound(name,GetChannel());}
void CSSEPlaySound::Initialize(const std::shared_ptr<ds::Block> &data)
{
	SSEPlaySound::Initialize(data);
	auto dsp = data->GetString("dsp");
	if(dsp.empty() == false)
		m_dspEffect = c_engine->GetAuxEffect(dsp);
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	for(auto &type : al::get_aux_types())
	{
		auto dataBlock = data->GetBlock(type,0);
		if(dataBlock == nullptr)
			continue;
		auto effect = al::create_aux_effect(type,*dataBlock);
		if(effect != nullptr)
			effects.push_back(effect);
	}
}

//////////////////////////////////////

CSoundScriptManager::CSoundScriptManager()
	: SoundScriptManager()
{}
CSoundScriptManager::~CSoundScriptManager()
{}
bool CSoundScriptManager::Load(const char *fname,std::vector<std::shared_ptr<SoundScript>> *scripts)
{
	return SoundScriptManager::Load<CSoundScript>(fname,scripts);
}
SoundScriptEvent *CSoundScriptManager::CreateEvent(std::string name)
{
	if(name == "playsound")
		return new CSSEPlaySound(this);
	else if(name == "lua")
		return new SSELua(this);
	return new SoundScriptEvent(this);
}