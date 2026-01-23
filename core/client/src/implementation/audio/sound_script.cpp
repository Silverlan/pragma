// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :audio.sound_script;
import :client_state;
import :engine;

#undef CreateEvent

pragma::audio::CSoundScript::CSoundScript(SoundScriptManager *manager, std::string identifier) : SoundScript(manager, identifier) {}
pragma::audio::CSoundScript::~CSoundScript() {}

//////////////////////////////////////

pragma::audio::CSSEPlaySound::CSSEPlaySound(SoundScriptManager *manager) : SSEPlaySound(manager), m_dspEffect(nullptr) {}
pragma::audio::SSESound *pragma::audio::CSSEPlaySound::CreateSound(double tStart, const std::function<std::shared_ptr<ALSound>(const std::string &, ALChannel, ALCreateFlags)> &createSound)
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
void pragma::audio::CSSEPlaySound::PrecacheSound(const char *name) { get_client_state()->PrecacheSound(name, GetChannel()); }
void pragma::audio::CSSEPlaySound::Initialize(udm::LinkedPropertyWrapper &prop)
{
	SSEPlaySound::Initialize(prop);
	std::string dsp;
	prop["dsp"](dsp);
	if(dsp.empty() == false)
		m_dspEffect = get_cengine()->GetAuxEffect(dsp);
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	for(auto &type : get_aux_types()) {
		auto dataBlock = prop[type];
		if(!dataBlock)
			continue;
		auto effect = create_aux_effect(type, dataBlock);
		if(effect != nullptr)
			effects.push_back(effect);
	}
}

//////////////////////////////////////

pragma::audio::CSoundScriptManager::CSoundScriptManager() : SoundScriptManager() {}
pragma::audio::CSoundScriptManager::~CSoundScriptManager() {}
bool pragma::audio::CSoundScriptManager::Load(const char *fname, std::vector<std::shared_ptr<SoundScript>> *scripts) { return SoundScriptManager::Load<CSoundScript>(fname, scripts); }
pragma::audio::SoundScriptEvent *pragma::audio::CSoundScriptManager::CreateEvent(std::string name)
{
	if(name == "playsound")
		return new CSSEPlaySound(this);
	else if(name == "lua")
		return new SSELua(this);
	return new SoundScriptEvent(this);
}
