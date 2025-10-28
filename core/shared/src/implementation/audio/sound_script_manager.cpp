// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include <unordered_set>

#include <functional>
#include <map>

#include <cinttypes>
#include <optional>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <cmath>
#include "pragma/lua/core.hpp"

#include <memory>
#include <string>

module pragma.shared;

import :audio.sound_script_manager;

#undef CreateEvent

SoundScript::SoundScript(SoundScriptManager *manager, const std::string &identifier) : SoundScriptEventContainer(manager), m_identifier(identifier) {}

SoundScript::~SoundScript() {}

const std::string &SoundScript::GetIdentifier() const { return m_identifier; }

//////////////////////////////

SoundScriptManager::SoundScriptManager() {}

SoundScriptManager::~SoundScriptManager() { Clear(); }

const std::string &SoundScriptManager::GetSoundScriptPath()
{
	static std::string r = "scripts\\sounds\\";
	return r;
}

const std::unordered_map<std::string, std::shared_ptr<SoundScript>> &SoundScriptManager::GetScripts() const { return m_soundScripts; }
const std::vector<std::string> &SoundScriptManager::GetSoundScriptFiles() const { return m_soundScriptFiles; }

void SoundScriptManager::Clear() { m_soundScripts.clear(); }

bool SoundScriptManager::Load(const char *fname, std::vector<std::shared_ptr<SoundScript>> *scripts) { return Load<SoundScript>(fname, scripts); }

SoundScript *SoundScriptManager::FindScript(const char *name)
{
	std::string sname = name;
	ustring::to_lower(sname);
	auto it = m_soundScripts.find(sname);
	if(it != m_soundScripts.end())
		return it->second.get();
	return NULL;
}

bool SoundScriptManager::Load(const char *fname, const std::function<std::shared_ptr<SoundScript>(const std::string &)> fCreateSoundScript, std::vector<std::shared_ptr<SoundScript>> *scripts)
{
	std::string err;
	auto udmData = util::load_udm_asset(fname, &err);
	if(udmData == nullptr)
		return false;
	auto &data = *udmData;
	auto udm = data.GetAssetData().GetData();
	for(auto pair : udm.ElIt()) {
		std::string name {pair.key};
		ustring::to_lower(name);
		// Note: std::shared_ptr<TSoundScript>(new TSoundScript{this,it->first}); causes weird compiler errors for CSoundScript (clientside), but this works
		// auto script = std::static_pointer_cast<TSoundScript>(std::shared_ptr<void>(static_cast<void*>(new TSoundScript{this,it->first}))); // Does not work with gcc
		auto script = fCreateSoundScript(name);

		script->InitializeEvents(pair.property);
		auto it = m_soundScripts.find(name);
		if(it == m_soundScripts.end()) {
			m_soundScripts.insert(std::make_pair(name, script));
			if(scripts != NULL)
				scripts->push_back(script);
		}
		else {
			script = nullptr;
			if(scripts != NULL)
				scripts->push_back(it->second);
		}
	}
	return true;
}

SoundScriptEvent *SoundScriptManager::CreateEvent(std::string name)
{
	if(name == "playsound")
		return new SSEPlaySound(this);
	else if(name == "lua")
		return new SSELua(this);
	return new SoundScriptEvent(this);
}
SoundScriptEvent *SoundScriptManager::CreateEvent() { return CreateEvent(""); }
