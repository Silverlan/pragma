/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
