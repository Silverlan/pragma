/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __SAVEGAME_HPP__
#define __SAVEGAME_HPP__

#include "pragma/networkdefinitions.h"
#include <memory>

class Game;
namespace pragma
{
	namespace savegame
	{
		const uint32_t VERSION = 1u;
		static bool save(Game &game,const std::string &fileName);
		static bool load(Game &game,const std::string &fileName);
	};
};

#endif
