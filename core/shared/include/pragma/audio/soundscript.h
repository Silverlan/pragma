/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __SOUNDSCRIPT_H__
#define __SOUNDSCRIPT_H__
#include "pragma/networkdefinitions.h"
#include "pragma/audio/alsound.h"
#include "pragma/audio/soundscript_events.h"
#include <sharedutils/util_string.h>
#include <fsys/filesystem.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#undef CreateEvent

class DLLNETWORK SoundScriptManager;
class DLLNETWORK SoundScript
	: public SoundScriptEventContainer
{
public:
	friend SoundScriptManager;
	SoundScript(SoundScriptManager *manager,const std::string &identifier);
	const std::string &GetIdentifier() const;
	virtual ~SoundScript() override;
protected:
	std::string m_identifier;
};

class DLLNETWORK SoundScriptManager
{
protected:
	std::unordered_map<std::string,std::shared_ptr<SoundScript>> m_soundScripts;
	std::vector<std::string> m_soundScriptFiles;
	template<class TSoundScript>
		bool Load(const char *fname,std::vector<std::shared_ptr<SoundScript>> *scripts=NULL);
public:
	static const std::string &GetSoundScriptPath();
	SoundScriptManager();
	virtual ~SoundScriptManager();
	virtual bool Load(const char *fname,std::vector<std::shared_ptr<SoundScript>> *scripts=NULL);
	SoundScript *FindScript(const char *name);
	void Clear();
	virtual SoundScriptEvent *CreateEvent(std::string name);
	SoundScriptEvent *CreateEvent();
	const std::unordered_map<std::string,std::shared_ptr<SoundScript>> &GetScripts() const;
	const std::vector<std::string> &GetSoundScriptFiles() const;
};

template<class TSoundScript>
	bool SoundScriptManager::Load(const char *fname,std::vector<std::shared_ptr<SoundScript>> *scripts)
{
	auto f = FileManager::OpenFile(fname,"r");
	std::shared_ptr<ds::Block> root = nullptr;
	if(f != NULL)
		root = ds::System::ReadData(f);
	if(root == NULL)
		return false;
	auto *data = root->GetData();
	if(data == NULL)
		return false;
	m_soundScriptFiles.push_back(fname);
	for(auto it=data->begin();it!=data->end();it++)
	{
		std::shared_ptr<ds::Block> block = nullptr;
		if(it->second->IsBlock())
			block = std::static_pointer_cast<ds::Block>(it->second);
		else if(it->second->IsContainer()) // There is more than one entry with the same name; Just use the first one
			block = static_cast<ds::Container&>(*it->second).GetBlock(0u);
		if(block != NULL)
		{
			std::string name = it->first;
			StringToLower(name);
			// Note: std::shared_ptr<TSoundScript>(new TSoundScript{this,it->first}); causes weird compiler errors for CSoundScript (clientside), but this works
			// auto script = std::static_pointer_cast<TSoundScript>(std::shared_ptr<void>(static_cast<void*>(new TSoundScript{this,it->first}))); // Does not work with gcc
			auto script = std::make_shared<TSoundScript>(this,it->first);

			script->InitializeEvents(block);
			auto it = m_soundScripts.find(name);
			if(it == m_soundScripts.end())
			{
				m_soundScripts.insert(std::make_pair(name,script));
				if(scripts != NULL)
					scripts->push_back(script);
			}
			else
			{
				script = nullptr;
				if(scripts != NULL)
					scripts->push_back(it->second);
			}
		}
	}
	return true;
}

#endif