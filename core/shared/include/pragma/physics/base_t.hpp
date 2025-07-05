// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
