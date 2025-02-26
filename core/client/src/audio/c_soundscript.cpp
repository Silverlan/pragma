/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/audio/c_alsound.h"
#include "pragma/audio/c_soundscript.h"
#include <sharedutils/util_string.h>
#include "pragma/audio/c_sound_efx.hpp"
#include <alsoundsystem.hpp>
#include <algorithm>
#include <udm.hpp>

extern DLLCLIENT CEngine *c_engine;
extern ClientState *client;

#undef CreateEvent

CSoundScript::CSoundScript(SoundScriptManager *manager, std::string identifier) : SoundScript(manager, identifier) {}
CSoundScript::~CSoundScript() {}

//////////////////////////////////////

CSSEPlaySound::CSSEPlaySound(SoundScriptManager *manager) : SSEPlaySound(manager), m_dspEffect(NULL) {}
SSESound *CSSEPlaySound::CreateSound(double tStart, const std::function<std::shared_ptr<ALSound>(const std::string &, ALChannel, ALCreateFlags)> &createSound)
{
	auto *s = SSEPlaySound::CreateSound(tStart, createSound);
	if(s == nullptr)
		return s;
	auto *cs = dynamic_cast<CALSound *>(s->sound.get());
	if(cs == nullptr)
		return s;
	if(m_dspEffect != nullptr)
		cs->AddEffect(*m_dspEffect);
	for(auto &effect : effects)
		cs->AddEffect(*effect);
	return s;
}
void CSSEPlaySound::PrecacheSound(const char *name) { client->PrecacheSound(name, GetChannel()); }
void CSSEPlaySound::Initialize(udm::LinkedPropertyWrapper &prop)
{
	SSEPlaySound::Initialize(prop);
	std::string dsp;
	prop["dsp"](dsp);
	if(dsp.empty() == false)
		m_dspEffect = c_engine->GetAuxEffect(dsp);
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	for(auto &type : al::get_aux_types()) {
		auto dataBlock = prop[type];
		if(!dataBlock)
			continue;
		auto effect = al::create_aux_effect(type, dataBlock);
		if(effect != nullptr)
			effects.push_back(effect);
	}
}

//////////////////////////////////////

CSoundScriptManager::CSoundScriptManager() : SoundScriptManager() {}
CSoundScriptManager::~CSoundScriptManager() {}
bool CSoundScriptManager::Load(const char *fname, std::vector<std::shared_ptr<SoundScript>> *scripts) { return SoundScriptManager::Load<CSoundScript>(fname, scripts); }
SoundScriptEvent *CSoundScriptManager::CreateEvent(std::string name)
{
	if(name == "playsound")
		return new CSSEPlaySound(this);
	else if(name == "lua")
		return new SSELua(this);
	return new SoundScriptEvent(this);
}
