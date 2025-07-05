// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LAI_HPP__
#define __LAI_HPP__

#include "pragma/networkdefinitions.h"

namespace Lua {
	class Interface;
	namespace ai {
		DLLNETWORK void register_library(Lua::Interface &lua);
	};
};

#endif
