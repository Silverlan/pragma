// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __GAMEMODEMANAGER_H__
#define __GAMEMODEMANAGER_H__

#include "pragma/networkdefinitions.h"
#include <unordered_map>
#include <sharedutils/util_version.h>

struct DLLNETWORK GameModeInfo {
	std::string id;
	std::string name;
	std::string component_name;
	std::string author;
	std::string initial_map;
	util::Version version;
	std::unordered_map<std::string, int32_t> gameMountPriorities;
};

class DLLNETWORK GameModeManager {
  private:
	GameModeManager() = delete;
	static std::unordered_map<std::string, GameModeInfo> m_gameModes;
  public:
	static void Initialize();
	static std::unordered_map<std::string, GameModeInfo> &GetGameModes();
	static GameModeInfo *GetGameModeInfo(const std::string &id);
};

#endif
