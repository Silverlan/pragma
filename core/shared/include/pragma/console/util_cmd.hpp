/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __UTIL_CMD_HPP__
#define __UTIL_CMD_HPP__

#include "pragma/networkdefinitions.h"
#include <vector>

class Game;
class BaseEntity;
namespace pragma {class BasePlayerComponent;};
namespace util
{
	namespace cmd
	{
		DLLNETWORK std::vector<std::pair<BaseEntity*,float>> get_sorted_entities(Game &game,::pragma::BasePlayerComponent *pl=nullptr);
	};
};

#endif
