// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :engine;

std::string pragma::Engine::GetConVarString(const std::string &cv)
{
	auto *cvar = GetConVar(cv);
	if(cvar == nullptr)
		return "";
	return static_cast<console::ConVar *>(cvar)->GetString();
}

int pragma::Engine::GetConVarInt(const std::string &cv)
{
	auto *cvar = GetConVar(cv);
	if(cvar == nullptr)
		return 0;
	return static_cast<console::ConVar *>(cvar)->GetInt();
}

float pragma::Engine::GetConVarFloat(const std::string &cv)
{
	auto *cvar = GetConVar(cv);
	if(cvar == nullptr)
		return 0.f;
	return static_cast<console::ConVar *>(cvar)->GetFloat();
}

bool pragma::Engine::GetConVarBool(const std::string &cv)
{
	auto *cvar = GetConVar(cv);
	if(cvar == nullptr)
		return false;
	return static_cast<console::ConVar *>(cvar)->GetBool();
}

pragma::console::ConConf *pragma::Engine::GetConVar(const std::string &cv)
{
	auto *stateSv = GetServerNetworkState();
	auto *cvar = (stateSv != nullptr) ? stateSv->GetConVar(cv) : nullptr;
	if(cvar == nullptr)
		cvar = CVarHandler::GetConVar(cv);
	return cvar;
}
