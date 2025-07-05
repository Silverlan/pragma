// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LNAV_HPP__
#define __LNAV_HPP__

#include "pragma/networkdefinitions.h"

namespace Lua {
	class Interface;
	namespace nav {
		DLLNETWORK void register_library(Lua::Interface &lua);
	};
};

#endif
