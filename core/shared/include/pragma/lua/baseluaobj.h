/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __BASELUAOBJ_H__
#define __BASELUAOBJ_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

class DLLNETWORK BaseLuaObj
{
protected:
	BaseLuaObj();
	virtual ~BaseLuaObj();
	bool m_bExternalHandle; // If true, m_handle will not be deleted
	std::unique_ptr<luabind::object> m_luaObj = nullptr;
public:
	virtual luabind::object *GetLuaObject();
};

template<class THandle>
	class LuaObj
		: public BaseLuaObj
{
protected:
	mutable THandle *m_handle;
	virtual void InitializeHandle()=0;
	template<class TCustomHandle>
		void InitializeLuaObject(lua_State *lua);
	virtual void InitializeLuaObject(lua_State *lua);
public:
	LuaObj();
	virtual ~LuaObj() override;
	THandle GetHandle() const;
};

template<class THandle>
	LuaObj<THandle>::LuaObj()
		: BaseLuaObj()
{}

template<class THandle>
	LuaObj<THandle>::~LuaObj()
{
	m_handle->Invalidate();
	if(m_bExternalHandle == false)
		delete m_handle;
}

template<class THandle>
	THandle LuaObj<THandle>::GetHandle() const {return *m_handle;}

template<class THandle>
	void LuaObj<THandle>::InitializeLuaObject(lua_State *lua) {InitializeLuaObject<THandle>(lua);}

template<class THandle>
	template<class TCustomHandle>
		void LuaObj<THandle>::InitializeLuaObject(lua_State *lua)
{
	if(m_luaObj != nullptr)
		return;
	m_luaObj = std::make_unique<luabind::object>(lua,*(dynamic_cast<TCustomHandle*>(m_handle))); // dynamic_cast required for virtual inheritance
}

#endif