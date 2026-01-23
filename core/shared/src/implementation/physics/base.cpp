// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.base;

pragma::util::TWeakSharedHandle<pragma::physics::IBase> pragma::physics::IBase::GetHandle() const { return m_handle; }
pragma::util::TSharedHandle<pragma::physics::IBase> pragma::physics::IBase::ClaimOwnership() const { return pragma::util::claim_shared_handle_ownership(m_handle); }

bool pragma::physics::IBase::IsConstraint() const { return false; }
bool pragma::physics::IBase::IsCollisionObject() const { return false; }
bool pragma::physics::IBase::IsController() const { return false; }

luabind::object &pragma::physics::IBase::GetLuaObject(lua::State *lua)
{
	if(!m_luaObj) // Lazy initialization
		InitializeLuaObject(lua);
	return *m_luaObj;
}
const luabind::object &pragma::physics::IBase::GetLuaObject(lua::State *lua) const { return const_cast<IBase *>(this)->GetLuaObject(lua); }
void pragma::physics::IBase::Push(lua::State *l) { m_luaObj->push(l); }
void pragma::physics::IBase::SetUserData(void *userData) const
{
	if(m_userData)
		throw std::logic_error {"User data has already been set!"};
	m_userData = userData;
}
void *pragma::physics::IBase::GetUserData() const { return m_userData; }
void pragma::physics::IBase::SetPhysObj(PhysObj &physObj) { m_physObj = &physObj; }
pragma::physics::PhysObj *pragma::physics::IBase::GetPhysObj() const { return m_physObj; }
void pragma::physics::IBase::Initialize() {}
void pragma::physics::IBase::OnRemove()
{
	m_luaObj = nullptr;
	m_handle = decltype(m_handle) {};
}

void pragma::physics::IBase::InitializeLuaObject(lua::State *lua) { InitializeLuaObject<IBase>(lua); }

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
