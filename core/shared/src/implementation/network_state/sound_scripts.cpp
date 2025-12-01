// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :network_state;

SoundScriptManager *NetworkState::GetSoundScriptManager() { return m_soundScriptManager.get(); }
SoundScript *NetworkState::FindSoundScript(const char *name) { return m_soundScriptManager->FindScript(name); }

bool NetworkState::LoadSoundScripts(const char *file, bool bPrecache)
{
	std::string path = SoundScriptManager::GetSoundScriptPath();
	path += file;
	std::vector<std::shared_ptr<SoundScript>> scripts;
	if(m_soundScriptManager->Load(path.c_str(), &scripts) == false) {
		static auto bSkipPort = false;
		if(bSkipPort == false) {
			if(util::port_sound_script(this, "scripts\\" + std::string(file)) == true) {
				bSkipPort = true;
				auto r = LoadSoundScripts(file, bPrecache);
				bSkipPort = false;
				return r;
			}
		}
		return false;
	}
	if(bPrecache == false)
		return true;
	for(unsigned int i = 0; i < scripts.size(); i++)
		scripts[i]->PrecacheSounds();
	return true;
}
