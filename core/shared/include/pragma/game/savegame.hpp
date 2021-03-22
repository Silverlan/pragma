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
namespace udm {struct AssetData;};
namespace pragma
{
	namespace savegame
	{
		static constexpr uint32_t FORMAT_VERSION = 1u;
		static constexpr auto PSAV_IDENTIFIER = "PSAV";
		bool save(Game &game,const std::string &fileName,std::string &outErr);
		bool load(Game &game,const std::string &fileName,std::string &outErr);
	};
};

#endif
