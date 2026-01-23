// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :scripting.lua.object_base;

LuaObjectBase::LuaObjectBase() : m_baseLuaObj(nullptr), m_weakRef(nullptr) {}
LuaObjectBase::LuaObjectBase(const luabind::object &o) { SetLuaObject(o); }
LuaObjectBase::LuaObjectBase(const luabind::weak_ref &ref) { SetLuaObject(ref); }
luabind::object LuaObjectBase::GetLuaObject() const { return (m_baseLuaObj != nullptr) ? *m_baseLuaObj.get() : ((m_weakRef != nullptr) ? Lua::WeakReferenceToObject(*m_weakRef) : luabind::object {}); }
void LuaObjectBase::SetLuaObject(const luabind::object &o)
{
	m_baseLuaObj = pragma::util::make_shared<luabind::object>(o);
	if(o)
		m_weakRef = pragma::util::make_shared<luabind::weak_ref>(Lua::CreateWeakReference(o));
	else
		m_weakRef = nullptr;
}
void LuaObjectBase::SetLuaObject(const luabind::weak_ref &ref)
{
	assert(false); // This function shouldn't be used anymore, it's unsafe!
	m_baseLuaObj = nullptr;
	m_weakRef = pragma::util::make_shared<luabind::weak_ref>(ref);
}

std::string LuaObjectBase::ToString() const { return const_cast<LuaObjectBase *>(this)->CallLuaMember<const char *>("__tostring"); }

void LuaObjectBase::ClearLuaObject()
{
	m_baseLuaObj = nullptr;
	m_weakRef = nullptr;
}

void LuaObjectBase::CallLuaMember(const std::string &name) { CallLuaMember<void>(name); }
