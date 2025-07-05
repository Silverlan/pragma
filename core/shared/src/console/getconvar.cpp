// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include <pragma/serverstate/serverstate.h>
#include <pragma/console/convars.h>

std::string Engine::GetConVarString(const std::string &cv)
{
	auto *cvar = GetConVar(cv);
	if(cvar == nullptr)
		return "";
	return static_cast<ConVar *>(cvar)->GetString();
}

int Engine::GetConVarInt(const std::string &cv)
{
	auto *cvar = GetConVar(cv);
	if(cvar == nullptr)
		return 0;
	return static_cast<ConVar *>(cvar)->GetInt();
}

float Engine::GetConVarFloat(const std::string &cv)
{
	auto *cvar = GetConVar(cv);
	if(cvar == nullptr)
		return 0.f;
	return static_cast<ConVar *>(cvar)->GetFloat();
}

bool Engine::GetConVarBool(const std::string &cv)
{
	auto *cvar = GetConVar(cv);
	if(cvar == nullptr)
		return false;
	return static_cast<ConVar *>(cvar)->GetBool();
}

ConConf *Engine::GetConVar(const std::string &cv)
{
	ServerState *stateSv = GetServerState();
	auto *cvar = (stateSv != nullptr) ? stateSv->GetConVar(cv) : nullptr;
	if(cvar == nullptr)
		cvar = CVarHandler::GetConVar(cv);
	return cvar;
}
