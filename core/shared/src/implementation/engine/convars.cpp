// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :engine;

pragma::console::ConConf *pragma::Engine::GetConVar(std::string_view cv)
{
	auto *stateSv = GetServerNetworkState();
	auto *cvar = (stateSv != nullptr) ? stateSv->GetConVar(cv) : nullptr;
	if(cvar == nullptr)
		cvar = CVarHandler::GetConVar(cv);
	return cvar;
}
