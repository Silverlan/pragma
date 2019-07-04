#include "stdafx_shared.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/base.hpp"

util::TWeakSharedHandle<pragma::physics::IBase> pragma::physics::IBase::GetHandle() const {return m_handle;}
util::TSharedHandle<pragma::physics::IBase> pragma::physics::IBase::ClaimOwnership() const {return m_handle;}

bool pragma::physics::IBase::IsConstraint() const {return false;}
bool pragma::physics::IBase::IsCollisionObject() const {return false;}
bool pragma::physics::IBase::IsController() const {return false;}

luabind::object &pragma::physics::IBase::GetLuaObject() {return *m_luaObj;}
const luabind::object &pragma::physics::IBase::GetLuaObject() const {return const_cast<IBase*>(this)->GetLuaObject();}
void pragma::physics::IBase::Push(lua_State *l) {m_luaObj->push(l);}
void pragma::physics::IBase::OnRemove()
{
	m_luaObj = nullptr;
	m_handle = {};
}

void pragma::physics::IBase::InitializeLuaObject(lua_State *lua)
{
	m_luaObj = std::make_unique<luabind::object>(lua,GetHandle());
}

pragma::physics::IBase::IBase(IEnvironment &env)
	: m_physEnv{env}
{}
void pragma::physics::IBase::SetHandle(const util::TWeakSharedHandle<IBase> &handle) {m_handle = handle;}
