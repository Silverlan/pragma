/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"
#include "pragma/logging.hpp"
#include <fsys/filesystem.h>
#include <pragma/serverstate/serverstate.h>
#include <pragma/console/convars.h>
#include <sharedutils/util_string.h>

bool Engine::ExecConfig(const std::string &cfg, const std::function<void(std::string &, std::vector<std::string> &)> &callback)
{
	std::string path = cfg;
	if(path.substr(path.length() - 4) != ".cfg")
		path += ".cfg";
	path = "cfg\\" + FileManager::GetCanonicalizedPath(path);
	auto f = FileManager::OpenFile(path.c_str(), "r");
	if(f == NULL) {
		spdlog::warn("'{}' not present; not executing.", cfg);
		return false;
	}
	while(!f->Eof()) {
		auto line = f->ReadLine();
		ustring::get_sequence_commands(line, [this, &callback](std::string cmd, std::vector<std::string> &argv) { callback(cmd, argv); });
	}
	return true;
}

bool Engine::ExecConfig(const std::string &cfg)
{
	return ExecConfig(cfg, [this](std::string &cmd, std::vector<std::string> &argv) { RunConsoleCommand(cmd, argv); });
}

void Engine::LoadServerConfig()
{
	ExecConfig("engine.cfg");
	ExecConfig("server.cfg");
}

void Engine::LoadConfig() { LoadServerConfig(); }

void Engine::SaveServerConfig()
{
	FileManager::CreatePath("cfg");
	std::string path = "cfg\\server.cfg";
	auto f = FileManager::OpenFile<VFilePtrReal>(path.c_str(), "w");
	if(f == NULL) {
		spdlog::warn("Unable to save server.cfg");
		return;
	}
	WriteServerConfig(f);
}

void Engine::SaveEngineConfig()
{
	FileManager::CreatePath("cfg");
	std::string path = "cfg\\engine.cfg";
	auto f = FileManager::OpenFile<VFilePtrReal>(path.c_str(), "w");
	if(f == NULL) {
		spdlog::warn("Unable to save engine.cfg");
		return;
	}
	WriteEngineConfig(f);
}

void Engine::WriteEngineConfig(VFilePtrReal f)
{
	auto &cvars = GetConVars();
	for(auto it = cvars.begin(); it != cvars.end(); it++) {
		auto &cf = it->second;
		if(cf->GetType() == ConType::Var) {
			auto *cv = static_cast<ConVar *>(cf.get());
			if((cv->GetFlags() & ConVarFlags::Archive) == ConVarFlags::Archive && cv->GetString() != cv->GetDefault()) {
				std::string l = it->first + " \"" + cv->GetString() + "\"\n";
				f->WriteString(l.c_str());
			}
		}
	}
}

void Engine::WriteServerConfig(VFilePtrReal f)
{
	auto *stateSv = GetServerState();
	if(stateSv != nullptr) {
		auto &cvars = stateSv->GetConVars();
		for(auto it = cvars.begin(); it != cvars.end(); it++) {
			auto &cf = it->second;
			if(cf->GetType() == ConType::Var) {
				auto *cv = static_cast<ConVar *>(cf.get());
				if((cv->GetFlags() & ConVarFlags::Archive) == ConVarFlags::Archive && cv->GetString() != cv->GetDefault()) {
					std::string l = it->first + " \"" + cv->GetString() + "\"\n";
					f->WriteString(l.c_str());
				}
			}
		}
	}
}
