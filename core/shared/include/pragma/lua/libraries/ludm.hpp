/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUDM_HPP__
#define __LUDM_HPP__

#include "pragma/networkdefinitions.h"
#include <udm_types.hpp>

namespace udm {struct Array;};
namespace Lua
{
	class Interface;
	namespace udm
	{
		DLLNETWORK void register_library(Lua::Interface &lua);
		DLLNETWORK void set_array_values(lua_State *l,::udm::Array &a,luabind::tableT<void> t,uint32_t tIdx);
		DLLNETWORK bool set_array_value(lua_State *l,::udm::Array &a,uint32_t idx,const luabind::object &o);
		DLLNETWORK void table_to_udm(const Lua::tb<void> &t,::udm::LinkedPropertyWrapper &udm);
		DLLNETWORK luabind::object udm_to_value(lua_State *l,::udm::LinkedPropertyWrapperArg udm);
	};
};

#endif
