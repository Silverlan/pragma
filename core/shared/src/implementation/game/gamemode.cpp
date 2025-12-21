// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.game;

std::string pragma::Game::GetGameModeScriptDirectoryPath() const
{
	auto *info = GetGameMode();
	if(info == nullptr)
		return "";
	auto &id = info->id;
	return "gamemodes\\" + id;
}
std::string pragma::Game::GetGameModeScriptDirectoryNetworkPath() const
{
	auto path = GetGameModeScriptDirectoryPath();
	if(path.empty())
		return "";
	return path + '\\' + GetLuaNetworkDirectoryName();
}
std::string pragma::Game::GetGameModeScriptFilePath() const
{
	auto path = GetGameModeScriptDirectoryNetworkPath();
	if(path.empty())
		return "";
	return path + '\\' + GetLuaNetworkFileName();
}

void pragma::Game::ReloadGameModeScripts()
{
	auto fileName = GetGameModeScriptFilePath();
	if(fileName.empty())
		return;
	ExecuteLuaFile(fileName, nullptr, true);
}

bool pragma::Game::InitializeGameMode()
{
	auto *info = GetGameMode();
	if(info == nullptr)
		return false;
	ReloadGameModeScripts();

	auto *entGamemode = CreateEntity("gamemode");
	if(entGamemode == nullptr)
		return false;
	m_entGamemode = entGamemode->GetHandle();
	entGamemode->AddComponent(info->component_name);
	entGamemode->Spawn();
	return true;
}

bool pragma::Game::IsGameModeInitialized() const
{
	if(m_entGamemode.valid() == false)
		return false;
	return true;
}
