/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LASSET_HPP__
#define __LASSET_HPP__

#include "pragma/networkdefinitions.h"
#include <string>

namespace pragma::asset {enum class Type : uint8_t;};
namespace Lua
{
	class Interface;
	namespace asset
	{
		DLLNETWORK void register_library(Lua::Interface &lua,bool extended);

		DLLNETWORK bool exists(lua_State *l,const std::string &name,pragma::asset::Type type);
		DLLNETWORK luabind::object find_file(lua_State *l,const std::string &name,pragma::asset::Type type);
		DLLNETWORK bool is_loaded(lua_State *l,const std::string &name,pragma::asset::Type type);
		DLLNETWORK void lock_asset_watchers(lua_State *l);
		DLLNETWORK void unlock_asset_watchers(lua_State *l);
	};
};

#endif
