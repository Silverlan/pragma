/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/base.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/physics/base_t.hpp"

util::TWeakSharedHandle<pragma::physics::IBase> pragma::physics::IBase::GetHandle() const { return m_handle; }
util::TSharedHandle<pragma::physics::IBase> pragma::physics::IBase::ClaimOwnership() const { return util::claim_shared_handle_ownership(m_handle); }

bool pragma::physics::IBase::IsConstraint() const { return false; }
bool pragma::physics::IBase::IsCollisionObject() const { return false; }
bool pragma::physics::IBase::IsController() const { return false; }

luabind::object &pragma::physics::IBase::GetLuaObject(lua_State *lua)
{
	if(!m_luaObj) // Lazy initialization
		InitializeLuaObject(lua);
	return *m_luaObj;
}
const luabind::object &pragma::physics::IBase::GetLuaObject(lua_State *lua) const { return const_cast<IBase *>(this)->GetLuaObject(lua); }
void pragma::physics::IBase::Push(lua_State *l) { m_luaObj->push(l); }
void pragma::physics::IBase::SetUserData(void *userData) const
{
	if(m_userData)
		throw std::logic_error {"User data has already been set!"};
	m_userData = userData;
}
void *pragma::physics::IBase::GetUserData() const { return m_userData; }
void pragma::physics::IBase::SetPhysObj(PhysObj &physObj) { m_physObj = &physObj; }
PhysObj *pragma::physics::IBase::GetPhysObj() const { return m_physObj; }
void pragma::physics::IBase::Initialize() {}
void pragma::physics::IBase::OnRemove()
{
	m_luaObj = nullptr;
	m_handle = decltype(m_handle) {};
}

void pragma::physics::IBase::InitializeLuaObject(lua_State *lua) { InitializeLuaObject<IBase>(lua); }

pragma::physics::IBase::IBase(IEnvironment &env) : m_physEnv {env} {}
void pragma::physics::IBase::InitializeLuaHandle(const util::TWeakSharedHandle<IBase> &handle) { m_handle = handle; }

bool pragma::physics::IWorldObject::IsSpawned() const { return m_bSpawned; }
void pragma::physics::IWorldObject::Spawn()
{
	if(IsSpawned())
		return;
	m_bSpawned = true;
	AddWorldObject();
	DoSpawn();
}
void pragma::physics::IWorldObject::AddWorldObject()
{
	if(m_bSpawned == false)
		return;
	//RemoveWorldObject();
	DoAddWorldObject();
}
void pragma::physics::IWorldObject::DoSpawn() {}
