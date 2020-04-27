/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LASSET_HPP__
#define __LASSET_HPP__

#include "pragma/networkdefinitions.h"

namespace Lua
{
	class Interface;
	namespace asset
	{
		DLLNETWORK void register_library(Lua::Interface &lua,bool extended);

		DLLNETWORK int32_t exists(lua_State *l);
		DLLNETWORK int32_t find_file(lua_State *l);
		DLLNETWORK int32_t is_loaded(lua_State *l);
	};
};

#endif
