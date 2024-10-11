/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <pragma/console/convars.h>
#include "pragma/input/input_binding_layer.hpp"

namespace pragma::string {
	class Utf8String;
	class Utf8StringView;
	class Utf8StringArg;
};

#include "pragma/localization.h"
#include <fsys/filesystem.h>
#include "pragma/input/inputhelper.h"
#include <pragma/logging.hpp>

void CEngine::SaveClientConfig()
{
	FileManager::CreatePath("cfg");
	std::string path = "cfg\\client.cfg";
	auto f = FileManager::OpenFile<VFilePtrReal>(path.c_str(), "w");
	if(f == NULL) {
		spdlog::warn("Unable to save client.cfg");
		return;
	}
	WriteClientConfig(f);
}

void CEngine::WriteClientConfig(VFilePtrReal f)
{
	f->WriteString("unbindall\n");
	auto inputLayer = GetCoreInputBindingLayer();
	if(inputLayer) {
		for(auto &pair : inputLayer->GetKeyMappings()) {
			auto &kb = pair.second;
			if(kb.GetType() == KeyBind::Type::Regular) {
				std::string key;
				if(KeyToString(pair.first, &key)) {
					std::string l = "bind \"" + key + "\" \"" + kb.GetBind() + "\"\n";
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
	}
	auto *stateSv = GetServerNetworkState();
	auto *stateCl = GetClientState();
	if(stateCl != NULL) {
		auto &cvars = stateCl->GetConVars();

		auto &cfg = GetConVarConfig(NwStateType::Client);
		if(cfg)
			RestoreConVarsForUnknownCommands(f, *cfg, cvars);

		for(auto &pair : cvars) {
			if(stateSv == NULL || stateSv->GetConVar(pair.first) == NULL) {
				auto &cf = pair.second;
				if(cf->GetType() == ConType::Var) {
					auto *cv = static_cast<ConVar *>(cf.get());
					if((cv->GetFlags() & ConVarFlags::Archive) == ConVarFlags::Archive && cv->GetString() != cv->GetDefault()) {
						std::string l = pair.first + " \"" + cv->GetString() + "\"\n";
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
	PreloadConfig(NwStateType::Client, "client.cfg");
}

void CEngine::LoadClientConfig()
{
	auto &cfg = GetConVarConfig(NwStateType::Client);
	if(cfg)
		ExecCommands(*cfg);
}

void CEngine::PreloadConfig(NwStateType type, const std::string &configName)
{
	Engine::PreloadConfig(type, configName);
	if(type != NwStateType::Client)
		return;
	auto &cfg = GetConVarConfig(NwStateType::Client);
	if(!cfg)
		return;
	std::string lan = Locale::DetermineSystemLanguage();
	auto *args = cfg->Find("cl_language");
	if(args && !args->empty())
		lan = args->front();
	Locale::SetLanguage(lan);
	Locale::Load("inputs.txt");
	Locale::Load("menu.txt");
	Locale::Load("misc.txt");
	Locale::Load("components.txt");
}
