// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :client_state;
import :entities.components;
import :engine;

pragma::console::ConConf *pragma::CEngine::GetConVar(const std::string &cv)
{
	auto *cvar = Engine::GetConVar(cv);
	if(cvar != nullptr)
		return cvar;
	auto *stateCl = GetClientState();
	return (stateCl != nullptr) ? stateCl->GetConVar(cv) : nullptr;
}

bool pragma::CEngine::RunConsoleCommand(std::string cmd, std::vector<std::string> &argv, KeyState pressState, float magnitude, const std::function<bool(console::ConConf *, float &)> &callback)
{
	string::to_lower(cmd);
	auto *stateCl = static_cast<ClientState *>(GetClientState());
	BasePlayerComponent *pl = nullptr;
	if(stateCl != nullptr) {
		auto *game = stateCl->GetGameState();
		if(game != nullptr)
			pl = game->GetLocalPlayer();
	}
	if(stateCl == nullptr)
		return RunEngineConsoleCommand(cmd, argv, pressState, magnitude, callback);
	if(stateCl == nullptr || !stateCl->RunConsoleCommand(cmd, argv, pl, pressState, magnitude, callback)) {
		Con::CWAR << "Unknown console command '" << cmd << "'!" << Con::endl;
		auto similar = (stateCl != nullptr) ? stateCl->FindSimilarConVars(cmd) : FindSimilarConVars(cmd);
		if(similar.empty() == true)
			Con::COUT << "No similar matches found!" << Con::endl;
		else {
			Con::COUT << "Were you looking for one of the following?" << Con::endl;
			for(auto &sim : similar)
				Con::COUT << "- " << sim << Con::endl;
		}
		return false;
	}
	return true;
}
