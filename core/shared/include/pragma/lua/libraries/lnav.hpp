/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LNAV_HPP__
#define __LNAV_HPP__

#include "pragma/networkdefinitions.h"

namespace Lua
{
	class Interface;
	namespace nav
	{
		DLLNETWORK void register_library(Lua::Interface &lua);
	};
};

#endif
