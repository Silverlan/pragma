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
	bool Load(const char *fname,const std::function<std::shared_ptr<SoundScript>(const std::string&)> fCreateSoundScript,std::vector<std::shared_ptr<SoundScript>> *scripts=NULL);
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
	return Load(fname,[this](const std::string &name) -> std::shared_ptr<SoundScript> {return std::make_shared<TSoundScript>(this,name);},scripts);
}

#endif