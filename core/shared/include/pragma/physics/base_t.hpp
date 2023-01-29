/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYS_BASE_T_HPP__
#define __PHYS_BASE_T_HPP__

#include "pragma/physics/base.hpp"
#include "pragma/lua/raw_object.hpp"

template<class T>
void pragma::physics::IBase::InitializeLuaObject(lua_State *lua)
{
	auto handle = ClaimOwnership();
	if(handle.IsValid())
		m_luaObj = std::make_unique<luabind::object>(lua, pragma::lua::raw_object_to_luabind_object(lua, util::shared_handle_cast<IBase, T>(handle)));
	else
		m_luaObj = std::make_unique<luabind::object>(lua, pragma::lua::raw_object_to_luabind_object(lua, std::dynamic_pointer_cast<T>(shared_from_this())));
}

#endif
