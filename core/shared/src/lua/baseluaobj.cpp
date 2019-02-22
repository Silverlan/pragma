#include "stdafx_shared.h"
#include "pragma/lua/baseluaobj.h"

BaseLuaObj::BaseLuaObj()
	: m_luaObj(nullptr),m_bExternalHandle(false)
{}

BaseLuaObj::~BaseLuaObj()
{
	m_luaObj = nullptr;
}

luabind::object *BaseLuaObj::GetLuaObject() {return m_luaObj.get();}