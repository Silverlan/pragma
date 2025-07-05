// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_CMD_HPP__
#define __UTIL_CMD_HPP__

#include "pragma/networkdefinitions.h"
#include <vector>

class Game;
class BaseEntity;
namespace pragma {
	class BasePlayerComponent;
};
namespace util {
	namespace cmd {
		DLLNETWORK std::vector<std::pair<BaseEntity *, float>> get_sorted_entities(Game &game, ::pragma::BasePlayerComponent *pl = nullptr);
	};
};

#endif
