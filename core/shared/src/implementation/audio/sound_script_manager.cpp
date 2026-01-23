// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :audio.sound_script_manager;

#undef CreateEvent

pragma::audio::SoundScript::SoundScript(SoundScriptManager *manager, const std::string &identifier) : SoundScriptEventContainer(manager), m_identifier(identifier) {}

pragma::audio::SoundScript::~SoundScript() {}

const std::string &pragma::audio::SoundScript::GetIdentifier() const { return m_identifier; }

//////////////////////////////

pragma::audio::SoundScriptManager::SoundScriptManager() {}

pragma::audio::SoundScriptManager::~SoundScriptManager() { Clear(); }

const std::string &pragma::audio::SoundScriptManager::GetSoundScriptPath()
{
	static std::string r = "scripts\\sounds\\";
	return r;
}

const std::unordered_map<std::string, std::shared_ptr<pragma::audio::SoundScript>> &pragma::audio::SoundScriptManager::GetScripts() const { return m_soundScripts; }
const std::vector<std::string> &pragma::audio::SoundScriptManager::GetSoundScriptFiles() const { return m_soundScriptFiles; }

void pragma::audio::SoundScriptManager::Clear() { m_soundScripts.clear(); }

bool pragma::audio::SoundScriptManager::Load(const char *fname, std::vector<std::shared_ptr<SoundScript>> *scripts) { return Load<SoundScript>(fname, scripts); }

pragma::audio::SoundScript *pragma::audio::SoundScriptManager::FindScript(const char *name)
{
	std::string sname = name;
	string::to_lower(sname);
	auto it = m_soundScripts.find(sname);
	if(it != m_soundScripts.end())
		return it->second.get();
	return nullptr;
}

bool pragma::audio::SoundScriptManager::Load(const char *fname, const std::function<std::shared_ptr<SoundScript>(const std::string &)> fCreateSoundScript, std::vector<std::shared_ptr<SoundScript>> *scripts)
{
	std::string err;
	auto udmData = util::load_udm_asset(fname, &err);
	if(udmData == nullptr)
		return false;
	auto &data = *udmData;
	auto udm = data.GetAssetData().GetData();
	for(auto pair : udm.ElIt()) {
		std::string name {pair.key};
		string::to_lower(name);
		// Note: std::shared_ptr<TSoundScript>(new TSoundScript{this,it->first}); causes weird compiler errors for CSoundScript (clientside), but this works
		// auto script = std::static_pointer_cast<TSoundScript>(std::shared_ptr<void>(static_cast<void*>(new TSoundScript{this,it->first}))); // Does not work with gcc
		auto script = fCreateSoundScript(name);

		script->InitializeEvents(pair.property);
		auto it = m_soundScripts.find(name);
		if(it == m_soundScripts.end()) {
			m_soundScripts.insert(std::make_pair(name, script));
			if(scripts != nullptr)
				scripts->push_back(script);
		}
		else {
			script = nullptr;
			if(scripts != nullptr)
				scripts->push_back(it->second);
		}
	}
	return true;
}

pragma::audio::SoundScriptEvent *pragma::audio::SoundScriptManager::CreateEvent(std::string name)
{
	if(name == "playsound")
		return new SSEPlaySound(this);
	else if(name == "lua")
		return new SSELua(this);
	return new SoundScriptEvent(this);
}
pragma::audio::SoundScriptEvent *pragma::audio::SoundScriptManager::CreateEvent() { return CreateEvent(""); }
