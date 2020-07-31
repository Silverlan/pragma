/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <pragma/serverstate/serverstate.h>
#include <pragma/console/convars.h>
#include "pragma/localization.h"
#include <fsys/filesystem.h>
#include "pragma/input/inputhelper.h"

CEngine::ConVarInfo *CEngine::ConVarInfoList::find(const std::string &cmd)
{
	for(auto it=cvars.begin();it!=cvars.end();++it)
	{
		auto &info = *it;
		if(info.cvar == cmd)
			return &info;
	}
	return nullptr;
}

bool CEngine::ExecConfig(const std::string &cfg,std::vector<ConVarInfo> &cmds)
{
	return Engine::ExecConfig(cfg,[&cmds](std::string &cmd,std::vector<std::string> &argv) {
		cmds.push_back(ConVarInfo());
		auto &info = cmds.back();
		info.cvar = cmd;
		info.argv = argv;
	});
}

void CEngine::ExecCommands(ConVarInfoList &cmds)
{
	for(auto it=cmds.cvars.begin();it!=cmds.cvars.end();++it)
	{
		auto &info = *it;
		RunConsoleCommand(info.cvar,info.argv);
	}
}

void CEngine::SaveClientConfig()
{
	FileManager::CreatePath("cfg");
	std::string path = "cfg\\client.cfg";
	auto f = FileManager::OpenFile<VFilePtrReal>(path.c_str(),"w");
	if(f == NULL)
	{
		Con::cwar<<"WARNING: Unable to save client.cfg"<<Con::endl;
		return;
	}
	WriteClientConfig(f);
}

void CEngine::WriteClientConfig(VFilePtrReal f)
{
	f->WriteString("unbindall\n");
	for(auto &pair : GetKeyMappings())
	{
		auto &kb = pair.second;
		if(kb.GetType() == KeyBind::Type::Regular)
		{
			std::string key;
			if(KeyToString(pair.first,&key))
			{
				std::string l = "bind \"" +key +"\" \"" +kb.GetBind() +"\"\n";
				f->WriteString(l.c_str());
			}
		}
		// Deprecated (Replaced by "toggle" console command")
		/*else if(kb.GetType() == KeyBind::Type::Toggle)
		{
			std::string key;
			if(KeyToString(pair.first,&key))
			{
				std::string l = "bindtoggle \"" +key +"\" \"" +kb.GetBind() +"\"\n";
				f->WriteString(l.c_str());
			}
		}*/
	}
	auto *stateSv = GetServerState();
	auto *stateCl = GetClientState();
	if(stateCl != NULL)
	{
		auto &cvars = stateCl->GetConVars();
		for(auto &pair : cvars)
		{
			if(stateSv == NULL || stateSv->GetConVar(pair.first) == NULL)
			{
				auto &cf = pair.second;
				if(cf->GetType() == ConType::Var)
				{
					auto *cv = static_cast<ConVar*>(cf.get());
					if((cv->GetFlags() &ConVarFlags::Archive) == ConVarFlags::Archive && cv->GetString() != cv->GetDefault())
					{
						std::string l = pair.first +" \"" +cv->GetString() +"\"\n";
						f->WriteString(l.c_str());
					}
				}
			}
		}
	}
}

void CEngine::LoadConfig()
{
	Engine::LoadConfig();
	PreloadClientConfig();
}

void CEngine::PreloadClientConfig()
{
	m_preloadedConfig = std::make_unique<ConVarInfoList>();
	auto &cmds = *m_preloadedConfig.get();
	ExecConfig("client.cfg",cmds.cvars);
	std::string lan = "en";
	auto res = cmds.find("cl_language");
	if(res != nullptr && !res->argv.empty())
		lan = res->argv[0];
	Locale::SetLanguage(lan);
	Locale::Load("inputs.txt");
	Locale::Load("menu.txt");
	Locale::Load("misc.txt");
}

void CEngine::LoadClientConfig()
{
	if(m_preloadedConfig != nullptr)
	{
		ExecCommands(*m_preloadedConfig.get());
		m_preloadedConfig = nullptr;
	}
}