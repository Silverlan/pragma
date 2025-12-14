// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :engine;

void pragma::Engine::PreloadConfig(NwStateType type, const std::string &configName)
{
	auto &cfg = GetConVarConfig(type);
	cfg = std::make_unique<ConVarInfoList>();
	auto &cmds = *cfg.get();
	ExecConfig(configName, cmds);
}
void pragma::Engine::PreloadConfig(StateInstance &instance, const std::string &configName)
{
	if(!instance.state)
		return;
	if(instance.state->IsServer())
		PreloadConfig(NwStateType::Server, configName);
	else
		PreloadConfig(NwStateType::Client, configName);
}

pragma::Engine::ConVarInfoList::ConVarArgs *pragma::Engine::ConVarInfoList::Find(const std::string &cmd)
{
	auto it = m_cvarMap.find(cmd);
	return (it != m_cvarMap.end()) ? &it->second : nullptr;
}

void pragma::Engine::ConVarInfoList::Add(const std::string &cmd, const ConVarArgs &args)
{
	if(m_cvars.size() == m_cvars.capacity())
		m_cvars.reserve(m_cvars.size() * 1.5 + 50);
	m_cvars.push_back({cmd, args});
	m_cvarMap[cmd] = args;
}

bool pragma::Engine::ExecConfig(const std::string &cfg, ConVarInfoList &infoList)
{
	return ExecConfig(cfg, [&infoList](std::string &cmd, std::vector<std::string> &argv) { infoList.Add(cmd, argv); });
}

void pragma::Engine::ExecCommands(ConVarInfoList &cmds)
{
	for(auto &cmd : cmds.GetConVars())
		RunConsoleCommand(cmd.cmd, cmd.args);
}

bool pragma::Engine::ExecConfig(const std::string &cfg, const std::function<void(std::string &, std::vector<std::string> &)> &callback)
{
	std::string path = cfg;
	if(path.substr(path.length() - 4) != ".cfg")
		path += ".cfg";
	path = "cfg\\" + fs::get_canonicalized_path(path);
	auto f = pragma::fs::open_file(path.c_str(), fs::FileMode::Read);
	if(f == nullptr) {
		spdlog::warn("'{}' not present; not executing.", cfg);
		return false;
	}
	while(!f->Eof()) {
		auto line = f->ReadLine();
		string::get_sequence_commands(line, [this, &callback](std::string cmd, std::vector<std::string> &argv) { callback(cmd, argv); });
	}
	return true;
}

bool pragma::Engine::ExecConfig(const std::string &cfg)
{
	return ExecConfig(cfg, [this](std::string &cmd, std::vector<std::string> &argv) { RunConsoleCommand(cmd, argv); });
}

void pragma::Engine::LoadServerConfig()
{
	ExecConfig("engine.cfg");
	PreloadConfig(NwStateType::Server, "server.cfg");
	auto &cfg = GetConVarConfig(NwStateType::Server);
	assert(cfg);
	ExecCommands(*cfg);
}

void pragma::Engine::LoadConfig() { LoadServerConfig(); }

void pragma::Engine::SaveServerConfig()
{
	fs::create_path("cfg");
	std::string path = "cfg\\server.cfg";
	auto f = fs::open_file<fs::VFilePtrReal>(path, fs::FileMode::Write);
	if(f == nullptr) {
		spdlog::warn("Unable to save server.cfg");
		return;
	}
	WriteServerConfig(f);
}

void pragma::Engine::SaveEngineConfig()
{
	fs::create_path("cfg");
	std::string path = "cfg\\engine.cfg";
	auto f = fs::open_file<fs::VFilePtrReal>(path, fs::FileMode::Write);
	if(f == nullptr) {
		spdlog::warn("Unable to save engine.cfg");
		return;
	}
	WriteEngineConfig(f);
}

void pragma::Engine::RestoreConVarsForUnknownCommands(fs::VFilePtrReal f, const ConVarInfoList &origCvarValues, const std::map<std::string, std::shared_ptr<console::ConConf>> &stateConVars)
{
	// We need to restore commands from the previous config in cases where we don't know the command.
	// In this case the command may be from a script or module that hasn't been loaded during this instance and
	// we don't want to lose its value.
	for(auto &conVarInfo : origCvarValues.GetConVars()) {
		auto it = stateConVars.find(conVarInfo.cmd);
		if(it == stateConVars.end()) {
			std::string l = conVarInfo.cmd;
			for(auto &arg : conVarInfo.args) {
				l += " ";
				l += "\"" + arg + "\"";
			}
			l += "\n";
			f->WriteString(l.c_str());
		}
	}
}

void pragma::Engine::WriteEngineConfig(fs::VFilePtrReal f)
{
	auto &cvars = GetConVars();
	for(auto it = cvars.begin(); it != cvars.end(); it++) {
		auto &cf = it->second;
		if(cf->GetType() == console::ConType::Var) {
			auto *cv = static_cast<console::ConVar *>(cf.get());
			if((cv->GetFlags() & console::ConVarFlags::Archive) == console::ConVarFlags::Archive && cv->GetString() != cv->GetDefault()) {
				std::string l = it->first + " \"" + cv->GetString() + "\"\n";
				f->WriteString(l.c_str());
			}
		}
	}
}

void pragma::Engine::WriteServerConfig(fs::VFilePtrReal f)
{
	auto *stateSv = GetServerNetworkState();
	if(stateSv != nullptr) {
		auto &cvars = stateSv->GetConVars();

		auto &cfg = GetConVarConfig(NwStateType::Server);
		if(cfg)
			RestoreConVarsForUnknownCommands(f, *cfg, cvars);

		for(auto it = cvars.begin(); it != cvars.end(); it++) {
			auto &cf = it->second;
			if(cf->GetType() == console::ConType::Var) {
				auto *cv = static_cast<console::ConVar *>(cf.get());
				if((cv->GetFlags() & console::ConVarFlags::Archive) == console::ConVarFlags::Archive && cv->GetString() != cv->GetDefault()) {
					std::string l = it->first + " \"" + cv->GetString() + "\"\n";
					f->WriteString(l.c_str());
				}
			}
		}
	}
}
